#if !defined( __test_recovery_h )
#define __test_recovery_h 1

#include <string>
#include <set>
#include <list>

#include "mrnet/MRNet.h"

using namespace std;

void proc_ArgList( int iargc,
                   char ** iargv,
                   string &obenchmark_type,
                   uint32_t &oduration,
                   string &obackend_exe,
                   string &oso_file,
                   string &ohosts,
                   string &ohosts_format,
                   uint32_t &oprocs_per_node,
                   uint32_t &onfailures,
                   uint32_t &ofailure_frequency,
                   uint32_t &onorphans,
                   uint32_t &oextra_internal_procs,
                   uint32_t &ojob_id );

bool get_HostListFromFile( string &ihostfile,
                           list<string> & ohost_list );

bool get_HostListFromSLURMFormattedString( string &inodes_str,
                                           list< string > & orpocess_list );

int run_ThroughputTestFE( Network * inetwork,
                          unsigned int inwaves,
                          unsigned int insamples,
                          const char * iso_filename );

int run_ThroughputTestBE( Network * inetwork,
                          unsigned int inwaves,
                          unsigned int imax_val );


bool validate_Output( set<uint32_t> &ieq_class,
                      uint32_t inbackends,
                      uint32_t inwaves,
                      uint32_t imax_val,
                      set<uint32_t> &oeq_class );


#endif /* __test_recovery_h */
