#!/usr/bin/env python3

import argparse
import atexit
import base64
from datetime import datetime, timedelta, timezone
import dateutil.parser
import github
import json
import os
import re
from requests import Session
from requests.adapters import HTTPAdapter, Retry
import subprocess
import sys
import tempfile
import time
import urllib.parse
import urllib.request

import sentry_sdk

sentry_sdk.init(traces_sample_rate=0.1)


def _durable_subprocess_run(*args, **kwargs):
    """
    Calls subprocess.run with retries/exponential backoff on failure.
    """

    max_attempts = 5
    for attempt_num in range(max_attempts):
        try:
            return subprocess.run(*args, **kwargs, check=True)
        except subprocess.CalledProcessError as e:
            if attempt_num == max_attempts - 1:
                raise e
            print(
                f"Subprocess failed ({e}), retrying ({attempt_num+1}/{max_attempts})",
                file=sys.stderr,
            )
            time.sleep(2 ** (1 + attempt_num))


def _write_secret(file: str, content: str):
    # Create the file with the oatuh token
    with open(file, "wb") as fd:
        fd.write(b"")
    os.chmod(file, 0o600)
    with open(file, "wb") as fd:
        fd.write(content.encode())


class SpackCIBridge(object):

    def __init__(self, gitlab_repo="", gitlab_host="", gitlab_project="", github_project="",
                 disable_status_post=True, disable_protected_sync=True, disable_tag_sync=True, sync_draft_prs=False,
                 main_branch=None, prereq_checks=[], required_label=""):
        self.gitlab_repo = gitlab_repo
        self.github_project = github_project
        github_token = os.environ.get('GITHUB_TOKEN')
        self.github_repo = "https://{0}@github.com/{1}.git".format(github_token, self.github_project)
        self.py_github = github.Github(auth=github.Auth.Token(github_token))
        self.py_gh_repo = self.py_github.get_repo(self.github_project)

        self.session = Session()
        self.session.mount(
            "https://",
            HTTPAdapter(
                max_retries=Retry(
                    total=5,
                    backoff_factor=2,
                    backoff_jitter=1,
                ),
            ),
        )

        self.merge_msg_regex = re.compile(r"Merge\s+([^\s]+)\s+into\s+([^\s]+)")
        self.unmergeable_shas = []

        self.post_status = not disable_status_post
        self.sync_draft_prs = sync_draft_prs
        self.sync_protected_branches = not disable_protected_sync
        self.sync_tags = not disable_tag_sync
        self.main_branch = main_branch
        self.currently_running_sha = None
        self.latest_tested_main_commit = None

        self.prereq_checks = prereq_checks
        self.required_label = required_label

        dt = datetime.now(timezone.utc) + timedelta(minutes=-60)
        self.time_threshold_brief = urllib.parse.quote_plus(dt.isoformat(timespec="seconds"))

        self.pipeline_api_template = gitlab_host
        self.pipeline_api_template += "/api/v4/projects/"
        self.pipeline_api_template += urllib.parse.quote_plus(gitlab_project)
        self.pipeline_api_template += "/pipelines?ref={0}"

        self.commit_api_template = gitlab_host
        self.commit_api_template += "/api/v4/projects/"
        self.commit_api_template += urllib.parse.quote_plus(gitlab_project)
        self.commit_api_template += "/repository/commits/{0}"

        self.cached_commits = {}

    @atexit.register
    def cleanup():
        """Shutdown ssh-agent upon program termination."""
        if "SSH_AGENT_PID" in os.environ:
            print("    Shutting down ssh-agent({0})".format(os.environ["SSH_AGENT_PID"]))
            _durable_subprocess_run(["ssh-agent", "-k"])

    def setup_git_auth(self, ssh_key_base64, token):
        """Configure authentication for pushing branches to gitlab

        Args:
            ssh_key_base64: Base64 encoded ssh key
            token: OAuth token for gitlab
        """
        if ssh_key_base64:
            self.setup_ssh(ssh_key_base64)
        elif token:
            self.setup_oauth(token)
        else:
            raise Exception("Failed to setup auth for pushing to git remote.")

    def setup_oauth(self, token: str):
        """Configure the origin"""

        url = urllib.parse.urlparse(self.gitlab_repo)
        if not url.scheme.startswith("http"):
            raise Exception(f"Cannot configure oauth for {url.scheme}")

        # Inject the user name and OAuth token into the URL
        if ":" in token:
            url = url._replace(netloc=f"{token}@" + url.netloc)
        else:
            url = url._replace(netloc=f"spackbot:{token}@" + url.netloc)

        _write_secret(".git-credentials", urllib.parse.urlunparse(url) + "\n")

        # Write the credentials to the file
        _durable_subprocess_run(
            [
                "git", "config", "credential.helper", "store --file .git-credentials",
            ],
            stdout=subprocess.DEVNULL,
        )

    def setup_ssh(self, ssh_key_base64):
        print("Starting ssh-agent")
        output = _durable_subprocess_run(["ssh-agent", "-s"], stdout=subprocess.PIPE).stdout

        # Search for PID in output.
        pid_regexp = re.compile(r"SSH_AGENT_PID=([0-9]+)")
        match = pid_regexp.search(output.decode("utf-8"))
        if match is None:
            print("WARNING: could not detect ssh-agent PID.", file=sys.stderr)
            print("ssh-agent will not be killed upon program termination", file=sys.stderr)
        else:
            pid = match.group(1)
            os.environ["SSH_AGENT_PID"] = pid
            self.cleanup_ssh_agent = True

        # Search for socket in output.
        socket_regexp = re.compile(r"SSH_AUTH_SOCK=([^;]+);")
        match = socket_regexp.search(output.decode("utf-8"))
        if match is None:
            print("WARNING: could not detect ssh-agent socket.", file=sys.stderr)
            print("Key will be added to caller's ssh-agent (if any)", file=sys.stderr)
        else:
            socket = match.group(1)
            os.environ["SSH_AUTH_SOCK"] = socket

        # Add the key.
        ssh_key = base64.b64decode(ssh_key_base64)
        ssh_key = ssh_key.replace(b"\r", b"")
        with tempfile.NamedTemporaryFile() as fp:
            fp.write(ssh_key)
            fp.seek(0)
            _durable_subprocess_run(["ssh-add", fp.name])

    def get_commit(self, commit):
        """ Check our cache for a commit on GitHub.
            If we don't have it yet, use the GitHub API to retrieve it."""
        if commit not in self.cached_commits:
            try:
                self.cached_commits[commit] = self.py_gh_repo.get_commit(sha=commit)
            except github.GithubException.GithubException as ghe:
                print(ghe)
                return None
        return self.cached_commits[commit]

    def list_github_prs(self):
        """ Return two dicts of data about open PRs on GitHub:
            one for all open PRs, and one for open PRs that are not up-to-date on GitLab."""
        pr_dict = {}
        pulls = self.py_gh_repo.get_pulls(state="open")
        print("Rate limit after get_pulls(): {}".format(self.py_github.rate_limiting[0]))
        for pull in pulls:
            backlogged = False
            push = True
            if pull.draft and not self.sync_draft_prs:
                print("Skipping draft PR {0} ({1})".format(pull.number, pull.head.ref))
                backlogged = "draft"
                push = False

            pr_string = "pr{0}_{1}".format(pull.number, pull.head.ref)

            if push and pull.updated_at < datetime.now(timezone.utc) + timedelta(minutes=-2880):
                # Skip further analysis of this PR if it hasn't been updated in 48 hours.
                # This helps us avoid wasting our rate limit on PRs with merge conflicts.
                print("Skip pushing stale PR {0}".format(pr_string))
                backlogged = "stale"
                push = False

            if push:
                # Determine if this PR still needs to be pushed to GitLab. This happens in one of two cases:
                # 1) we have never pushed it before
                # 2) we have pushed it before, but the HEAD sha has changed since we pushed it last
                log_args = ["git", "log", "--pretty=%s", "gitlab/{0}".format(pr_string)]
                try:
                    merge_commit_msg = subprocess.run(
                        log_args, check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL).stdout
                    match = self.merge_msg_regex.match(merge_commit_msg[:128].decode("utf-8"))
                    if match and (match.group(1) == pull.head.sha or match.group(2) == pull.head.sha):
                        print("Skip pushing {0} because GitLab already has HEAD {1}".format(pr_string, pull.head.sha))
                        push = False
                except subprocess.CalledProcessError:
                    # This occurs when it's a new PR that hasn't been pushed to GitLab yet.
                    pass
                except UnicodeDecodeError:
                    print(f"Failed to UTF-8 decode commit msg for {pr_string}:\n{merge_commit_msg}")
                    continue

            if push:
                # Check the PRs-to-be-pushed to see if any of them should be considered "backlogged".
                # We currently recognize four types of backlogged PRs:
                # 1) Some required "prerequisite checks" have not yet completed successfully.
                # 2) The PR is missing the required label.
                # 3) The PR is based on a version of the "main branch" that has not yet been tested
                # 4) Draft PRs. Handled earlier in this function.
                if not backlogged and self.prereq_checks:
                    checks_desc = "waiting for {} check to succeed"
                    checks_to_verify = self.prereq_checks.copy()
                    pr_check_runs = self.get_commit(pull.head.sha).get_check_runs()
                    for check in pr_check_runs:
                        if check.name in checks_to_verify:
                            checks_to_verify.remove(check.name)
                            if check.conclusion != "success":
                                backlogged = checks_desc.format(check.name)
                                push = False
                                break
                    if not backlogged and checks_to_verify:
                        backlogged = checks_desc.format(checks_to_verify[0])
                        push = False
                    if backlogged:
                        print("Skip pushing {0} because of {1}".format(pr_string, backlogged))

                if not backlogged and self.required_label:
                    if not any(label.name == self.required_label for label in pull.labels):
                        push = False
                        backlogged = "missing_label"
                        print("Skip pushing {0} because of missing label '{1}'".format(pr_string, self.required_label))

                if not backlogged:
                    check_for_deferral = not any(label.name == 'pipelines:urgent' for label in pull.labels)
                    if check_for_deferral and self.main_branch and pull.base.ref == self.main_branch:
                        # Check if we should defer pushing/testing this PR because it is based on "too new" of a commit
                        # of the main branch.
                        tmp_pr_branch = f"temporary_{pr_string}"
                        _durable_subprocess_run(["git", "fetch", "--depth=2147483647", "github",
                                                f"refs/pull/{pull.number}/head:{tmp_pr_branch}"])
                        # Get the merge base between this PR and the main branch.
                        try:
                            merge_base_sha = _durable_subprocess_run(
                                ["git", "merge-base", tmp_pr_branch, f"github/{self.main_branch}"],
                                stdout=subprocess.PIPE).stdout.strip()
                        except subprocess.CalledProcessError:
                            print(f"'git merge-base {tmp_pr_branch} github/{self.main_branch}' "
                                  "returned non-zero. Skipping")
                            self.unmergeable_shas.append(pull.head.sha)
                            continue

                        repo_head_sha = _durable_subprocess_run(
                            ["git", "rev-parse", tmp_pr_branch],
                            stdout=subprocess.PIPE).stdout.decode("utf-8").strip()

                        if pull.head.sha != repo_head_sha:
                            # If gh repo and api don't agree on what the head sha is, don't
                            # push.  Instead log an error message and backlog the PR.
                            a_sha, r_sha = pull.head.sha[:7], repo_head_sha[:7]
                            print(f"Skip pushing {pr_string} because api says HEAD is {a_sha}, "
                                  f"while repo says HEAD is {r_sha}")
                            backlogged = f"GitHub HEAD shas out of sync (repo={r_sha}, API={a_sha})"
                            push = False
                        # Check if our PR's merge base is an ancestor of the latest tested main branch commit.
                        elif subprocess.run(
                                ["git", "merge-base", "--is-ancestor", merge_base_sha, self.latest_tested_main_commit]
                                ).returncode == 0:
                            print(f"{tmp_pr_branch}'s merge base IS an ancestor of latest_tested_main "
                                  f"{merge_base_sha} vs. {self.latest_tested_main_commit}")
                            try:
                                _durable_subprocess_run(["git", "checkout", self.latest_tested_main_commit])
                                _durable_subprocess_run(["git", "checkout", "-b", pr_string])
                                commit_msg = f"Merge {pull.head.sha} into {self.latest_tested_main_commit}"
                                _durable_subprocess_run(
                                    ["git", "merge", "--no-ff", "-m", commit_msg, tmp_pr_branch],
                                )
                                print(f"Merge succeeded, ready to push {pr_string} to GitLab for CI pipeline testing")
                            except subprocess.CalledProcessError:
                                print(f"Failed to merge PR {pull.number} ({pull.head.ref}) with latest tested "
                                      f"{self.main_branch} ({self.latest_tested_main_commit}). Skipping")
                                self.unmergeable_shas.append(pull.head.sha)
                                try:
                                    _durable_subprocess_run(["git", "merge", "--abort"])
                                except subprocess.CalledProcessError:
                                    pass
                                backlogged = "merge conflicts with {}".format(self.main_branch)
                                push = False
                                continue
                        else:
                            print(f"Skip pushing {pr_string} because its merge base is NOT an ancestor of "
                                  f"latest_tested_main {merge_base_sha} vs. {self.latest_tested_main_commit}")
                            backlogged = "base"
                            push = False
                    else:
                        # If the --main-branch CLI argument wasn't passed, or if this PR doesn't target that branch,
                        # then we will push the merge commit that was automatically created by GitHub to GitLab
                        # where it will kick off a CI pipeline.
                        if not pull.merge_commit_sha:
                            print("PR {0} has merge conflicts. Skipping".format(pull.number))
                            backlogged = "merge conflicts with target branch"
                            push = False
                            continue
                        try:
                            _durable_subprocess_run(["git", "fetch", "--depth=2147483647", "github",
                                                    f"{pull.merge_commit_sha}:{pr_string}"])
                        except subprocess.CalledProcessError:
                            print("Failed to locally checkout PR {0} ({1}). Skipping"
                                  .format(pull.number, pull.merge_commit_sha))
                            backlogged = "GitLab failed to checkout this branch"
                            push = False
                            continue

            pr_dict[pr_string] = {
                'base_sha': pull.base.sha,
                'head_sha': pull.head.sha,
                'push': push,
                'backlogged': backlogged,
            }

        def listify_dict(d):
            pr_strings = sorted(d.keys())
            base_shas = [d[s]['base_sha'] for s in pr_strings]
            head_shas = [d[s]['head_sha'] for s in pr_strings]
            b_logged = [d[s]['backlogged'] for s in pr_strings]
            return {
                "pr_strings": pr_strings,
                "base_shas": base_shas,
                "head_shas": head_shas,
                "backlogged": b_logged,
            }
        all_open_prs = listify_dict(pr_dict)
        filtered_pr_dict = {k: v for (k, v) in pr_dict.items() if v['push']}
        filtered_open_prs = listify_dict(filtered_pr_dict)
        print("All Open PRs:")
        for pr_string in all_open_prs['pr_strings']:
            print("    {0}".format(pr_string))
        print("Filtered Open PRs:")
        for pr_string in filtered_open_prs['pr_strings']:
            print("    {0}".format(pr_string))
        print("Rate limit at the end of list_github_prs(): {}".format(self.py_github.rate_limiting[0]))
        return [all_open_prs, filtered_open_prs]

    def list_github_protected_branches(self):
        """ Return a list of protected branch names from GitHub."""
        branches = self.py_gh_repo.get_branches()
        print("Rate limit after get_branches(): {}".format(self.py_github.rate_limiting[0]))
        protected_branches = [br.name for br in branches if br.protected]
        protected_branches = sorted(protected_branches)
        if self.currently_running_sha:
            print("Skip pushing {0} because it already has a pipeline running ({1})"
                  .format(self.main_branch, self.currently_running_sha))
            protected_branches.remove(self.main_branch)
        print("Protected branches:")
        for protected_branch in protected_branches:
            print("    {0}".format(protected_branch))
        return protected_branches

    def list_github_tags(self):
        """ Return a list of tag names from GitHub."""
        tag_list = self.py_gh_repo.get_tags()
        print("Rate limit after get_tags(): {}".format(self.py_github.rate_limiting[0]))
        tags = sorted([tag.name for tag in tag_list])
        print("Tags:")
        for tag in tags:
            print("    {0}".format(tag))
        return tags

    def setup_git_repo(self):
        """Initialize a bare git repository with two remotes:
        one for GitHub and one for GitLab.
        If main_branch was specified, we also fetch that branch from GitHub.
        """
        _durable_subprocess_run(["git", "init"])
        _durable_subprocess_run(["git", "config", "user.email", "noreply@spack.io"])
        _durable_subprocess_run(["git", "config", "user.name", "spackbot"])
        _durable_subprocess_run(["git", "config", "advice.detachedHead", "false"])
        _durable_subprocess_run(["git", "remote", "add", "github", self.github_repo])
        _durable_subprocess_run(["git", "remote", "add", "gitlab", self.gitlab_repo])

        # Shallow fetch from GitLab.
        self.gitlab_shallow_fetch()

        if self.main_branch:
            _durable_subprocess_run(["git", "fetch", "--depth=2147483647", "github", self.main_branch])

    def get_gitlab_pr_branches(self):
        """Query GitLab for branches that have already been copied over from GitHub PRs.
        Return the string output of `git branch --remotes --list gitlab/pr*`.
        """
        branch_args = ["git", "branch", "--remotes", "--list", "gitlab/pr*"]
        self.gitlab_pr_output = \
            _durable_subprocess_run(branch_args, stdout=subprocess.PIPE).stdout

    def gitlab_shallow_fetch(self):
        """Perform a shallow fetch from GitLab"""
        fetch_args = ["git", "fetch", "-q", "--depth=1", "gitlab"]
        _durable_subprocess_run(fetch_args, stdout=subprocess.PIPE).stdout

    def get_open_refspecs(self, open_prs):
        """Return a list of refspecs to push given a list of open PRs."""
        print("Building initial lists of refspecs to fetch and push")
        pr_strings = open_prs["pr_strings"]
        base_shas = open_prs["base_shas"]
        backlogged = open_prs["backlogged"]
        open_refspecs = []
        for open_pr, base_sha, backlog in zip(pr_strings, base_shas, backlogged):
            open_refspecs.append("{0}:{0}".format(open_pr))
            print("  pushing {0} (based on {1})".format(open_pr, base_sha))
        return open_refspecs

    def update_refspecs_for_protected_branches(self, protected_branches, open_refspecs, fetch_refspecs):
        """Update our refspecs lists for protected branches from GitHub."""
        for protected_branch in protected_branches:
            fetch_refspecs.append("+refs/heads/{0}:refs/remotes/{0}".format(protected_branch))
            open_refspecs.append("refs/heads/{0}:refs/heads/{0}".format(protected_branch))
        return open_refspecs, fetch_refspecs

    def update_refspecs_for_tags(self, tags, open_refspecs, fetch_refspecs):
        """Update our refspecs lists for tags from GitHub."""
        for tag in tags:
            fetch_refspecs.append("+refs/tags/{0}:refs/tags/{0}".format(tag))
            open_refspecs.append("refs/tags/{0}:refs/tags/{0}".format(tag))
        return open_refspecs, fetch_refspecs

    def fetch_github_branches(self, fetch_refspecs):
        """Perform `git fetch` for a given list of refspecs."""
        print("Fetching GitHub refs for open PRs")
        fetch_args = ["git", "fetch", "-q", "--depth=2147483647", "github"] + fetch_refspecs
        _durable_subprocess_run(fetch_args)

    def build_local_branches(self, protected_branches):
        """Create local branches for a list of protected branches."""
        print("Building local branches for protected branches")
        for branch in protected_branches:
            local_branch_name = "{0}".format(branch)
            remote_branch_name = "refs/remotes/{0}".format(branch)
            _durable_subprocess_run(["git", "branch", "-q", local_branch_name, remote_branch_name])

    def make_status_for_pipeline(self, pipeline):
        """Generate POST data to create a GitHub status from a GitLab pipeline
           API response
        """
        post_data = {}
        if "status" not in pipeline:
            return post_data

        if pipeline["status"] == "created":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline has been created"

        elif pipeline["status"] == "waiting_for_resource":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is waiting for resources"

        elif pipeline["status"] == "preparing":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is preparing"

        elif pipeline["status"] == "pending":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is pending"

        elif pipeline["status"] == "running":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is running"

        elif pipeline["status"] == "manual":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is running manually"

        elif pipeline["status"] == "scheduled":
            post_data["state"] = "pending"
            post_data["description"] = "Pipeline is scheduled"

        elif pipeline["status"] == "failed":
            post_data["state"] = "error"
            post_data["description"] = "Pipeline failed"

        elif pipeline["status"] == "canceled":
            # Do not post canceled pipeline status to GitHub, it's confusing to our users.
            # This usually happens when a PR gets force-pushed. The next time the sync script runs
            # it will post a status for the newly force-pushed commit.
            return {}

        elif pipeline["status"] == "skipped":
            post_data["state"] = "failure"
            post_data["description"] = "Pipeline was skipped"

        elif pipeline["status"] == "success":
            post_data["state"] = "success"
            post_data["description"] = "Pipeline succeeded"

        post_data["target_url"] = pipeline["web_url"]
        return post_data

    def dedupe_pipelines(self, api_response):
        """Prune pipelines API response to only include the most recent result for each SHA"""
        pipelines = {}
        for response in api_response:
            sha = response['sha']
            if sha not in pipelines:
                pipelines[sha] = response
            else:
                existing_datetime = dateutil.parser.parse(pipelines[sha]['updated_at'])
                current_datetime = dateutil.parser.parse(response['updated_at'])
                if current_datetime > existing_datetime:
                    pipelines[sha] = response
        return pipelines

    def find_pr_sha(self, tested_sha):
        api_url = self.commit_api_template.format(tested_sha)
        headers = {}
        if "GITLAB_TOKEN" in os.environ:
            headers['PRIVATE-TOKEN'] = os.environ["GITLAB_TOKEN"]
        try:
            response = self.session.get(api_url, headers=headers, timeout=10)
        except OSError:
            print('Failed to fetch commit for tested sha {0}'.format(tested_sha))
            return None

        try:
            tested_commit_info = response.json()
        except json.decoder.JSONDecodeError:
            print('Failed to parse response as json ({0})'.format(response.text()))
            return None

        if 'title' not in tested_commit_info:
            print('Returned commit object missing "Title" field')
            return None

        merge_commit_msg = tested_commit_info['title']
        m = self.merge_msg_regex.match(merge_commit_msg)

        if m is None:
            print('Failed to find pr_sha in merge commit message')
            return None

        return m.group(1)

    def get_pipelines_for_branch(self, branch, time_threshold=None):
        # Use gitlab's API to get pipeline results for the corresponding ref.
        api_url = self.pipeline_api_template.format(
            urllib.parse.quote_plus(branch)
        )

        # Optionally constrain the query with the provided time_threshold
        if time_threshold:
            api_url = "{0}&updated_after={1}".format(api_url, time_threshold)

        headers = {}
        if "GITLAB_TOKEN" in os.environ:
            headers['PRIVATE-TOKEN'] = os.environ["GITLAB_TOKEN"]

        try:
            response = self.session.get(api_url, headers=headers, timeout=10)
        except OSError as inst:
            print("GitLab API request error accessing {0}".format(api_url))
            print(inst)
            return None

        try:
            pipelines = response.json()
        except json.decoder.JSONDecodeError as inst:
            print("Error parsing response to {0}".format(api_url))
            print(inst)
            return None

        return self.dedupe_pipelines(pipelines)

    def post_pipeline_status(self, open_prs, protected_branches):
        print("Rate limit at the beginning of post_pipeline_status(): {}".format(self.py_github.rate_limiting[0]))
        pipeline_branches = []
        backlog_branches = []
        # Split up the open_prs branches into two piles: branches we force-pushed to gitlab
        # and branches we deferred pushing.
        for pr_branch, base_sha, head_sha, backlog in zip(open_prs["pr_strings"],
                                                          open_prs["base_shas"],
                                                          open_prs["head_shas"],
                                                          open_prs["backlogged"]):
            if not backlog:
                pipeline_branches.append(pr_branch)
            else:
                backlog_branches.append((pr_branch, head_sha, backlog))

        pipeline_branches.extend(protected_branches)

        print('Querying pipelines to post status for:')
        for branch in pipeline_branches:
            # Post status to GitHub for each pipeline found.
            pipelines = self.get_pipelines_for_branch(branch, self.time_threshold_brief)
            if not pipelines:
                continue
            for sha, pipeline in pipelines.items():
                post_data = self.make_status_for_pipeline(pipeline)
                if not post_data:
                    continue
                # TODO: associate shas with protected branches, so we do not have to
                # hit an endpoint here, but just use the sha we already know just like
                # we do below for backlogged PR statuses.
                pr_sha = self.find_pr_sha(sha)
                if not pr_sha:
                    print('Could not find github PR sha for tested commit: {0}'.format(sha))
                    print('Using tested commit to post status')
                    pr_sha = sha
                self.create_status_for_commit(pr_sha,
                                              branch,
                                              post_data["state"],
                                              post_data["target_url"],
                                              post_data["description"])

        # Post a status of pending/backlogged for branches we deferred pushing
        print("Posting backlogged status to the following:")
        base_backlog_desc = \
            "This branch's merge-base with {} is newer than the latest commit tested by GitLab".format(self.main_branch)
        for branch, head_sha, reason in backlog_branches:
            if reason == "stale":
                print("Skip posting status for {} because it has not been updated recently".format(branch))
                continue
            elif reason == "missing_label":
                print("Skip posting status for {} because required label is not present".format(branch))
                continue
            elif reason == "base":
                desc = base_backlog_desc
                url = "https://github.com/spack/spack-infrastructure/blob/main/docs/deferred_pipelines.md"
            elif reason == "draft":
                desc = "GitLab CI is disabled for draft PRs"
                url = ""
            else:
                desc = reason
                url = ""
            self.create_status_for_commit(head_sha, branch, "pending", url, desc)

        # Post errors to any PRs that we couldn't merge to latest_tested_main_commit.
        print('Posting unmergeable status to the following:')
        for sha in self.unmergeable_shas:
            print('  {0}'.format(sha))
            self.create_status_for_commit(sha, "", "error", "", f"PR could not be merged with {self.main_branch}")
        print("Rate limit at the end of post_pipeline_status(): {}".format(self.py_github.rate_limiting[0]))

    def create_status_for_commit(self, sha, branch, state, target_url, description):
        context = os.environ.get("GITHUB_STATUS_CONTEXT", "ci/gitlab-ci")
        commit = self.get_commit(sha)
        if commit is None:
            print(f"Unable to find GitHub commit for sha {sha} on branch {branch}")
            return
        existing_statuses = commit.get_combined_status()
        for status in existing_statuses.statuses:
            if (status.context == context and
                    status.state == state and
                    status.description == description and
                    status.target_url == target_url):
                print("Not posting duplicate status to {} / {}".format(branch, sha))
                return
        try:
            status_response = self.get_commit(sha).create_status(
                state=state,
                target_url=target_url,
                description=description,
                context=context
            )
            if status_response.state != state:
                print("Expected CommitStatus state {0}, got {1}".format(
                    state, status_response.state))
        except Exception as e_inst:
            print('Caught exception posting status for {0}/{1}'.format(branch, sha))
            print(e_inst)
        print("  {0} -> {1}".format(branch, sha))

    def sync(self):
        """Synchronize pull requests from GitHub as branches on GitLab."""

        print("Initial rate limit: {}".format(self.py_github.rate_limiting[0]))
        reset_time = datetime.fromtimestamp(
            self.py_github.rate_limiting_resettime, timezone.utc).strftime('%Y-%m-%d %H:%M:%S')
        print("Rate limit will refresh at: {} UTC".format(reset_time))

        # Setup SSH command for communicating with GitLab.
        os.environ["GIT_SSH_COMMAND"] = "ssh -F /dev/null -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no"

        # Work inside a temporary directory that will be deleted when this script terminates.
        with tempfile.TemporaryDirectory() as tmpdirname:
            os.chdir(tmpdirname)

            # Setup the local repo with two remotes.
            self.setup_git_repo()

            if self.main_branch:
                # Find the currently running main branch pipeline, if any, and get the sha.
                # Also get the latest commit on the main branch that has a completed pipeline.
                main_branch_pipelines = self.get_pipelines_for_branch(self.main_branch)
                for sha, pipeline in main_branch_pipelines.items():
                    if self.latest_tested_main_commit is None and \
                            (pipeline['status'] == "success" or pipeline['status'] == "failed"):
                        self.latest_tested_main_commit = sha

                    if self.currently_running_sha is None and pipeline['status'] == "running":
                        self.currently_running_sha = sha

                    if self.latest_tested_main_commit and self.currently_running_sha:
                        break

            print("Latest completed {0} pipeline: {1}".format(self.main_branch, self.latest_tested_main_commit))
            print("Currently running {0} pipeline: {1}".format(self.main_branch, self.currently_running_sha))

            # Retrieve open PRs from GitHub.
            all_open_prs, open_prs = self.list_github_prs()
            # Get refspecs for open PRs and protected branches.
            open_refspecs = self.get_open_refspecs(open_prs)

            fetch_refspecs = []
            # Get protected branches on GitHub.
            protected_branches = []
            if self.sync_protected_branches:
                protected_branches = self.list_github_protected_branches()
                self.update_refspecs_for_protected_branches(protected_branches, open_refspecs, fetch_refspecs)

            # Get tags on GitHub.
            tags = []
            if self.sync_tags:
                tags = self.list_github_tags()
                self.update_refspecs_for_tags(tags, open_refspecs, fetch_refspecs)

            # Sync open GitHub PRs and protected branches to GitLab.
            self.fetch_github_branches(fetch_refspecs)
            self.build_local_branches(protected_branches)
            if open_refspecs:
                print("Syncing to GitLab")
                push_args = ["git", "push", "--porcelain", "-f", "gitlab"] + open_refspecs
                _durable_subprocess_run(push_args)

            # Post pipeline status to GitHub for each open PR, if enabled
            if self.post_status:
                print('Posting pipeline status for open PRs and protected branches')
                self.post_pipeline_status(all_open_prs, protected_branches)


if __name__ == "__main__":
    # Parse command-line arguments.
    parser = argparse.ArgumentParser(description="Sync GitHub PRs to GitLab")
    parser.add_argument("github_project", help="The GitHub project (org/repo or user/repo)")
    parser.add_argument("gitlab_repo", help="Full clone URL for GitLab")
    parser.add_argument("gitlab_host", help="GitLab web host")
    parser.add_argument("gitlab_project", help="GitLab project (org/repo or user/repo)")
    parser.add_argument("--disable-protected-sync", action="store_true", default=False,
                        help="Do not sync protected branches")
    parser.add_argument("--disable-tag-sync", action="store_true", default=False,
                        help="Do not sync tags")
    parser.add_argument("--disable-status-post", action="store_true", default=False,
                        help="Do not post pipeline status to each GitHub PR")
    parser.add_argument("--sync-draft-prs", action="store_true", default=False,
                        help="Copy draft PRs from GitHub to GitLab")
    parser.add_argument("--pr-mirror-bucket", default=None,
                        help="Delete mirrors for closed PRs from the specified S3 bucket")
    parser.add_argument("--main-branch", default=None,
                        help="""If provided, we check if there is a currently running
pipeline for this branch. If so, we defer syncing any subsequent commits in an effort
to not interrupt this pipeline. We also defer pushing any PR branches that are based
on a commit of the main branch that is newer than the latest commit tested by GitLab.""")
    parser.add_argument("--prereq-check", nargs="+", default=False,
                        help="Only push branches that have already passed this GitHub check")
    parser.add_argument("--required-label", default=False,
                        help="Only push branches that have this label on GitHub")

    args = parser.parse_args()

    gl_url = urllib.parse.urlparse(args.gitlab_repo)

    gitlab_ssh_key_base64 = os.getenv("GITLAB_SSH_KEY_BASE64")
    if gl_url.scheme == "ssh" and gitlab_ssh_key_base64 is None:
        raise Exception("Missing SSH key in GITLAB_SSH_KEY_BASE64 with ssh URL for gitlab")

    gitlab_token = os.getenv("GITLAB_TOKEN")
    if gl_url.scheme == "https" and gitlab_token is None:
        raise Exception("Missing OAuth key in GITLAB_TOKEN with https URL for gitlab")

    if "GITHUB_TOKEN" not in os.environ:
        raise Exception("GITHUB_TOKEN environment is not set")

    bridge = SpackCIBridge(gitlab_repo=args.gitlab_repo,
                           gitlab_host=args.gitlab_host,
                           gitlab_project=args.gitlab_project,
                           github_project=args.github_project,
                           disable_status_post=args.disable_status_post,
                           disable_protected_sync=args.disable_protected_sync,
                           disable_tag_sync=args.disable_tag_sync,
                           sync_draft_prs=args.sync_draft_prs,
                           main_branch=args.main_branch,
                           prereq_checks=args.prereq_check,
                           required_label=args.required_label)
    bridge.setup_git_auth(gitlab_ssh_key_base64, gitlab_token)
    bridge.sync()
