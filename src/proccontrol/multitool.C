
#include "PCProcess.h"
#include "ProcessSet.h"
#include "PlatFeatures.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <set>
#include <vector>


using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

static set<int> ranks;

static void setupOutput()
{
   int fd = open("/g/g0/legendre/tools/dyninst/githead/dyninst/testsuite/ppc64_bgq_ion/output_multitool", O_CREAT | O_WRONLY | O_TRUNC, 0600);
   if (fd == -1) {
      fprintf(stderr, "Failed to create output_multitool\n: %s", strerror(errno));
      return;
   }
   dup2(fd, 1);
   dup2(fd, 2);
   setDebug(true);
   fprintf(stderr, "Initializing tool setup\n");
}

static bool initRanks()
{
   char *jobid_s;
   char rank_dir[64];
   int jobid;

   jobid_s = getenv("BG_JOBID");
   if (!jobid_s) {
      fprintf(stderr, "BG_JOBID is not set!\n");
      return false;
   }
   jobid = atoi(jobid_s);
   
   snprintf(rank_dir, 64, "/jobs/%d/toolctl_rank", jobid);
   DIR *dir = opendir(rank_dir);
   if (!dir) {
      fprintf(stderr, "Could not open %s: %s\n", rank_dir, strerror(errno));
      return false;
   }

   for (struct dirent *dent = readdir(dir); dent; dent = readdir(dir)) {
      int node_id = atoi(dent->d_name);
      ranks.insert(node_id);
   }

   closedir(dir);
   return true;
}

int main(int argc, char *argv[])
{
   bool result;
   int priority = 0;
   char toolname[32];

   setupOutput();

   if (argc > 1) {
      priority = atoi(argv[1]);
   }
   
   snprintf(toolname, 32, "t%d%d\n", getpid() % 100, priority);
   MultiToolControl::setDefaultToolName(toolname);
   MultiToolControl::setDefaultToolPriority(priority);

   result = initRanks();
   if (!result) {
      return -1;
   }

   std::vector<ProcessSet::AttachInfo> ainfo;
   for (set<int>::iterator i = ranks.begin(); i != ranks.end(); i++) {
      ProcessSet::AttachInfo a;
      a.pid = *i;
      a.error_ret = err_none;
      ainfo.push_back(a);
   }
   ProcessSet::ptr pset = ProcessSet::attachProcessSet(ainfo);
   if (!pset || pset->size() != ainfo.size()) {
      fprintf(stderr, "Error attaching to some processes\n");
      return -1;
   }
   
   if (priority) {
      pset->continueProcs();
   }

   while (!pset->allExited()) {
      Process::handleEvents(true);
   }

   return 0;
}

