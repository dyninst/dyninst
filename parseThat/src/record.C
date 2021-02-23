/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "config.h"
#include "ipc.h"
#include "utils.h"
#include "record.h"
#include "sha1.h"

/* --------------------------------------------------------
 * PRIVATE: Given a binary filename, it will open a pipe to
 * `ldd', parse the output, and return a list of libraries
 */
bool get_libs(strlist *list, const char *prog)
{
    FILE *ldd_in;
    char *buf, *start, *end;

    buf = sprintf_static("ldd %s", prog);
    ldd_in = popen(buf, "r");

    while ( (buf = fgets_static(ldd_in)) != NULL) {
	if (strstr(buf, "needs:")) continue;

	start = strrchr(buf, '>');
	if (!start++) start = buf;
	while (isspace(*start)) ++start;

	end = strrchr(buf, '(');
	if (!end) end = buf + strlen(buf);
	while (start < end && isspace(*(end - 1))) --end;
	*end = '\0';

	strlist_insert(list, start);
    }

    pclose(ldd_in);
    return true;
}

strlist libs_diff(strlist *prev_run, strlist *curr_run)
{
    strlist result = STRLIST_INITIALIZER;
    char buf[STRING_MAX];

    unsigned prev_cnt = 0, curr_cnt = 0;
    char *prev_ptr = strlist_get(prev_run, prev_cnt);
    char *curr_ptr = strlist_get(curr_run, curr_cnt);
    int retval = 0;

    while (prev_ptr || curr_ptr) {
	if (prev_ptr && curr_ptr) {
	    retval = strcmp(prev_ptr, curr_ptr);

	    if (retval == 0) {
		// Libraries match.  Compare file contents.
		char *prev_hash = strlist_get(prev_run, ++prev_cnt);
		char *curr_hash = strlist_get(curr_run, ++curr_cnt);

		if (strcmp(prev_hash, curr_hash) != 0) {
		    snprintf(buf, sizeof(buf), "* %s has changed since last run.", prev_ptr);
		    strlist_push_back(&result, buf);
		}

		prev_ptr = strlist_get(prev_run, ++prev_cnt);
		curr_ptr = strlist_get(curr_run, ++curr_cnt);
		continue;
	    }
	}

	if (!curr_ptr || retval < 0) {
	    // If (there are no more libraries from `ldd`) OR
	    //    (library from history record not found in `ldd` list)

	    snprintf(buf, sizeof(buf), "* Mutatee is no longer dependent on %s.", prev_ptr);
	    strlist_push_back(&result, buf);

	    ++prev_cnt; // Skip digest string.
	    prev_ptr = strlist_get(prev_run, ++prev_cnt);

	} else if (!prev_ptr || retval > 0) {
	    // If (there are no more libraries from the history record) OR
	    //    (library from `ldd` not found in history record list)

	    snprintf(buf, sizeof(buf), "* Mutatee is now dependent on %s.", curr_ptr);
	    strlist_push_back(&result, buf);

	    ++curr_cnt; // Skip digest string.
	    curr_ptr = strlist_get(curr_run, ++curr_cnt);
	}
    }
    return result;
}

/* --------------------------------------------------------
 * PUBLIC: Implementation of public history record functions.
 */

record_t record_init()
{
    record_t result = { false, STRLIST_INITIALIZER, STRLIST_INITIALIZER, {0}, NULL, NULL };
    return result;
}

void record_clear(record_t *record)
{
    record->enabled = false;
    strlist_clear(&record->prog_line);
    strlist_clear(&record->lib_line);
    record->filename[0] = '\0';
    record->fd = NULL;
    record->raw_fd = NULL;
}

bool record_create(record_t *record, const char *prog, int argc, char **argv)
{
    char hash_buf[SHA1_STRING_LEN];
    const char *prog_bin = prog;
    unsigned i;

    record_clear(record);

    if (strrchr(prog, '/'))
	prog_bin = strrchr(prog, '/') + 1;

    if (!sha1_file(prog, hash_buf)) return false;
    strlist_push_back(&record->prog_line, prog_bin);
    strlist_push_back(&record->prog_line, sha1_file(prog, hash_buf));
    for (i = 0; i < (unsigned)argc; ++i)
	strlist_push_back(&record->prog_line, argv[i]);

    strlist lib_list = STRLIST_INITIALIZER;
    get_libs(&lib_list, prog);
    for (i = 0; i < lib_list.count; ++i) {
	const char *lib_path = strlist_get(&lib_list, i);

	if (!sha1_file(lib_path, hash_buf)) continue;
	strlist_push_back(&record->lib_line, lib_path);
	strlist_push_back(&record->lib_line, sha1_file(lib_path, hash_buf));
    }

    record->enabled = true;
    return true;
}

bool record_search(record_t *newRecord)
{
    FILE *index_fd;
    char *index_filename;
    bool found = false, latest = true;

    index_filename = sprintf_static("%s/%s", config.record_dir, HISTORY_RECORD_INDEX_FILE);
    errno = 0;
    index_fd = fopen(index_filename, "r");

    if (errno && errno == ENOENT) {
	index_fd = fopen(index_filename, "w");
	if (!index_fd) {
	    fprintf(config.outfd, "*\n* Error opening history record index %s.  Disabling history record.\n*\n", index_filename);
	    newRecord->enabled = false;
	    return false;
	}
	fclose(index_fd);

    } else if (!index_fd) {
	fprintf(config.outfd, "*\n* Error opening history record index %s.  Disabling history record.\n*\n", index_filename);
        newRecord->enabled = false;
	return false;

    } else {
	// We should use C++ string instead to avoid buffer overflows.
	char *buf;
	while ( (buf = fgets_static(index_fd))) {
	    if (buf[0] == '\t' || buf[0] == '\n') continue;

	    chomp(buf);
	    strlist prog_line = char2strlist(buf);
	    if (strlist_cmp(&prog_line, &newRecord->prog_line)) {

		strlist diff_list = STRLIST_INITIALIZER;
		while (!found && (buf = fgets_static(index_fd)) && buf[0] == '\t') {
		    chomp(buf);
		    strlist lib_line = char2strlist(buf + 1);

		    strncpy(newRecord->filename, strlist_get(&lib_line, 0), sizeof(newRecord->filename) - 1U);
		    strlist_pop_front(&lib_line);

		    if (strlist_cmp(&lib_line, &newRecord->lib_line)) {
			found = true;

		    } else {
			if (latest)
			    diff_list = libs_diff(&lib_line, &newRecord->lib_line);
			latest = false;
		    }
		    strlist_clear(&lib_line);
		}

		if (!found) {
		    fprintf(config.outfd, "* Warning: Current library set unknown.  Differences from latest run include:\n");
		    fprintf(config.outfd, "*\n");
		    for (unsigned i = 0; i < diff_list.count; ++i)
			fprintf(config.outfd, "%s\n", strlist_get(&diff_list, i));

		} else if (!latest)
		    fprintf(config.outfd, "* Warning: Current library signature does not match latest run.\n");

		strlist_clear(&diff_list);
	    }
	    strlist_clear(&prog_line);
	}
	fclose(index_fd);
    }

    if (!found) {
	// Fill storage file information
	const char *prog_file = strlist_get(&newRecord->prog_line, 0);
	if (strrchr(prog_file, '/'))
	    prog_file = strrchr(prog_file, '/') + 1;

	{
		std::string s = std::string(config.record_dir) + "/" + prog_file + "-XXXXXX";
		strncpy(newRecord->filename, s.c_str(), sizeof(newRecord->filename) - 1U);
	}

	int file_desc = mkstemp(newRecord->filename);
	newRecord->fd = fdopen(file_desc, "a");

    } else {
	newRecord->fd = fopen(newRecord->filename, "a");
    }

    char timestr[STRING_MAX];
    time_t timestamp = time(NULL);

    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", localtime(&timestamp));
    fprintf(newRecord->fd, "Log start: %s\n", timestr);
    fprintf(newRecord->fd, "--------------------------------------------------------------------------------\n");

    char raw_filename[ PATH_MAX ];
    strncpy(raw_filename, newRecord->filename, sizeof(raw_filename));
    strncat(raw_filename, ".raw", sizeof(raw_filename) - strlen(raw_filename) - 1);
    newRecord->raw_fd = fopen(raw_filename, "a");

    fprintf(newRecord->raw_fd, "Raw transcript log start: %s\n", timestr);
    fprintf(newRecord->raw_fd, "--------------------------------------------------------------------------------\n");

    return (found && latest);
}

void record_update(record_t *newRecord)
{
    FILE *index_fd;
    FILE *new_fd;
    char index_filename[PATH_MAX];
    char new_index_filename[PATH_MAX];

    strncpy(index_filename, config.record_dir, sizeof(index_filename));
    strncat(index_filename, "/", sizeof(index_filename) - strlen(index_filename) - 1);
    strncat(index_filename, HISTORY_RECORD_INDEX_FILE, sizeof(index_filename) - strlen(index_filename) - 1);
    errno = 0;
    index_fd = fopen(index_filename, "r");
    if (!index_fd) {
	fprintf(stderr, "Could not open %s\n", index_filename);
	return;
    }

    strncpy(new_index_filename, index_filename, sizeof(new_index_filename));
    strncat(new_index_filename, ".update", sizeof(new_index_filename) - strlen(new_index_filename) - 1);
    new_fd = fopen(new_index_filename, "w");
    if (!new_fd) {
	fprintf(stderr, "Could not open %s\n", new_index_filename);
	return;
    }

    char *buf = NULL;
    bool found = false;
    strlist prog_line = STRLIST_INITIALIZER;
    while ( (buf = fgets_static(index_fd))) {
	chomp(buf);

	if (buf[0] == '\t' || buf[0] == '\0') {
	    fprintf(new_fd, "%s\n", buf);
	    buf[0] = '\0';
	    continue;
	}

	prog_line = char2strlist(buf);
	if (strlist_cmp(&prog_line, &newRecord->prog_line)) {
	    fprintf(new_fd, "%s\n", buf);

	    strlist_push_front(&newRecord->lib_line, newRecord->filename);
	    buf = strlist2char(&newRecord->lib_line);
	    fprintf(new_fd, "\t%s\n", buf);
	    strlist_pop_front(&newRecord->lib_line);

	    while ( (buf = fgets_static(index_fd)) && buf[0] == '\t') {
		chomp(buf);

		strlist lib_line = char2strlist(buf);
		strlist_pop_front(&lib_line);

		if (!strlist_cmp(&lib_line, &newRecord->lib_line))
		    fprintf(new_fd, "%s\n", buf);

		strlist_clear(&lib_line);
	    }
	    fprintf(new_fd, "%s", buf);  // NOTE: Do not print "%s\n" here.

	    found = true;
	    break;
	}

	if (strcmp(strlist_get(&newRecord->prog_line, 0),
		   strlist_get(&prog_line, 0)) < 0)
	    break;

	strlist_clear(&prog_line);
	fprintf(new_fd, "%s\n", buf);
	buf[0] = '\0';
    }

    // New binary signature entry.
    if (!found) {
	char *buf2;

	buf2 = strlist2char(&newRecord->prog_line);
	fprintf(new_fd, "%s\n", buf2);

	strlist_push_front(&newRecord->lib_line, newRecord->filename);
	buf2 = strlist2char(&newRecord->lib_line);
	fprintf(new_fd, "\t%s\n\n", buf2);
	strlist_pop_front(&newRecord->lib_line);

	if (buf && buf[0] != '\0')
	    fprintf(new_fd, "%s\n", buf);
    }

    while ( (buf = fgets_static(index_fd)))
	fprintf(new_fd, "%s", buf);  // NOTE: Do not print "%s\n" here.

    fclose(index_fd);
    fclose(new_fd);

    unlink(index_filename);
    rename(new_index_filename, index_filename);
}

void record_close(record_t *newRecord)
{
    if (newRecord->enabled) {
	fprintf(newRecord->fd, "[ Log End ] --------------------------------------------------------------------\n\n");
	fclose(newRecord->fd);

	fprintf(newRecord->raw_fd, "[ Log End ] --------------------------------------------------------------------\n\n");
	fclose(newRecord->raw_fd);

	newRecord->enabled = false;
    }
}
