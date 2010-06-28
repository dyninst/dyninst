/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"

#include "CodeObject.h"
#include "CFG.h"
#include "Parser.h"
#include "debug.h"

#include "instructionAPI/h/Register.h"
#include "symEval/h/SymEval.h"
#include "symEval/src/SymEvalPolicy.h"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "boost/tuple/tuple_io.hpp"
#include "dynutil/h/dyntypes.h"
#include "dynutil/h/dyn_regs.h"
#include <stdarg.h>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

namespace {
    // initialization help
    static inline CFGFactory * __fact_init(CFGFactory * fact) {
        if(fact) return fact;
        return new CFGFactory();
    }
    static inline ParseCallback * __pcb_init(ParseCallback * cb) {
        if(cb) return cb;
        return new ParseCallback();
    }
}

CodeObject::CodeObject(CodeSource *cs, CFGFactory *fact, ParseCallback * cb) :
    _cs(cs),
    _fact(__fact_init(fact)),
    _pcb(__pcb_init(cb)),
    parser(new Parser(*this,*_fact,*_pcb) ),
    owns_factory(fact == NULL),
    owns_pcb(cb == NULL),
    flist(parser->sorted_funcs)
{
    process_hints(); // if any
}

void
CodeObject::process_hints()
{
    Function * f = NULL;
    const vector<Hint> & hints = cs()->hints();
    vector<Hint>::const_iterator hit;

    for(hit = hints.begin();hit!=hints.end();++hit) {
        CodeRegion * cr = (*hit)._reg;
        if(!cs()->regionsOverlap())
            f = parser->factory().mkfunc(
                (*hit)._addr,HINT,(*hit)._name,this,cr,cs());
        else
            f = parser->factory().mkfunc(
                (*hit)._addr,HINT,(*hit)._name,this,cr,cr);
        if(f) {
            parsing_printf("[%s] adding hint %lx\n",FILE__,f->addr());
            parser->add_hint(f);
        }
    }
}

CodeObject::~CodeObject() {
    if(owns_factory)
        delete _fact;
    if(owns_pcb)
        delete _pcb;
    if(parser)
        delete parser;
}

Function *
CodeObject::findFuncByEntry(CodeRegion * cr, Address entry)
{
    return parser->findFuncByEntry(cr,entry);
}

int
CodeObject::findFuncs(CodeRegion * cr, Address addr, set<Function*> & funcs)
{
    return parser->findFuncs(cr,addr,funcs);
}

Function *
CodeObject::findFuncByName(CodeRegion * cr, const std::string name )
{
    return parser->findFuncByName(cr,name);
}

Block *
CodeObject::findBlockByEntry(CodeRegion * cr, Address addr)
{
    return parser->findBlockByEntry(cr, addr);
}

int
CodeObject::findBlocks(CodeRegion * cr, Address addr, set<Block*> & blocks)
{
    return parser->findBlocks(cr,addr,blocks);
}

void
CodeObject::parse() {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse();
}

void
CodeObject::parse(Address target, bool recursive) {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse_at(target,recursive,HINT);
}

void
CodeObject::parseGaps(CodeRegion *cr) {
    if(!parser) {
        fprintf(stderr,"FATAL: internal parser undefined\n");
        return;
    }
    parser->parse_gap_heuristic(cr);
}

void
CodeObject::add_edge(Block * src, Block * trg, EdgeTypeEnum et)
{
    parser->link(src,trg,et,false);
}

void
CodeObject::finalize() {
    parser->finalize();
}

/* added for library fingerprinting */

string printidTuple(idTuple id);
string printCSV(idTuple id);

void makeTrapVector(set< vector<int> > & traps, int num, ...) {
    vector<int> trap;
    va_list args;
    va_start(args, num);

    for (int i = 0; i < num; i++) {
        trap.push_back(va_arg(args, int));
    }
    va_end(args);
    
    traps.insert(trap);
}

set<string> makeCallSet(int num, ...) {
    set<string> calls;
    va_list args;
    va_start(args, num);

    for (int i = 0; i < num; i++) {
        calls.insert(va_arg(args, char *));
    }
    va_end(args);

    return calls;
}

void CodeObject::addMapping(set< vector<int> > traps, set<string> calls, string name) {
    libraryIDMappings.insert(make_pair(boost::make_tuple("",traps,calls), name));
    syscallFuncs.insert(name);
}

/* Based on traps made in this function (and params passed, calls) identify function */
bool CodeObject::buildLibraryIDMappings() 
{

    /* process system call param #s */
    FILE * sysinfo;
    if ( (sysinfo = fopen("/p/paradyn/development/jacobson/Dyninst/fingerprinting/libc/glibc-2.5/params.txt", "r")) == NULL) {
        return false;
    }

    FILE * unistd;

    char cur[128];
    char buf[128];
    while (!feof(sysinfo)) {
        if (fgets(cur, 128, sysinfo) == NULL) break;
        
        char * sys;
        int sysnum;
        int numparams;
        char * delim = ",";
        sys = strtok(cur, delim);
        numparams = atoi(strtok(NULL, delim));

        stringstream sysname;
        sysname << "__NR_";
        sysname << sys;
        
        //cout << sysname.str() << " " << numparams << endl;

        if ( (unistd = fopen("/usr/include/asm/unistd.h", "r")) == NULL) {
            return false;
        }
        bool foundSyscall = false;
        while (!foundSyscall && !feof(unistd)) {
            if (fgets(buf, 128, unistd) == NULL) break;

            if (strstr(buf, sysname.str().c_str()) != NULL) {
                //cout << buf << endl;
                /*char * tmp = strchr(buf, ' ');
                char * tmp2 = strchr(tmp, ' ');
                char * tmp3 = strrchr(tmp, ' ');
                cout << tmp3 << endl;*/
                char tab = '\t';
                char * tmp = strrchr(buf, tab);
                if (!tmp)
                    tmp = strrchr(buf, ' ');
                if (!tmp)
                    break;
                //cout << tmp << endl;
                sscanf(tmp, "%d", &sysnum);
                //cout << sysnum << endl;
                syscallParams.insert(make_pair(sysnum,numparams)); 
                foundSyscall = true;
                break;
            }
        }
        fclose(unistd);
    }

    fclose(sysinfo);

    syscallFuncs.insert("_L_lock_80");
    syscallFuncs.insert("___xstat64");
    syscallFuncs.insert("__access");
    syscallFuncs.insert("__alloc_dir");
    syscallFuncs.insert("__bind");
    syscallFuncs.insert("__brk");
    syscallFuncs.insert("__call_pselect6");
    syscallFuncs.insert("__chdir");
    syscallFuncs.insert("__chmod");
    syscallFuncs.insert("__chown");
    syscallFuncs.insert("__clone");
    syscallFuncs.insert("__close_nocancel");
    syscallFuncs.insert("__dup");
    syscallFuncs.insert("__dup2");
    syscallFuncs.insert("__exit_thread");
    syscallFuncs.insert("__fchdir");
    syscallFuncs.insert("__fchmod");
    syscallFuncs.insert("__fchown");
    syscallFuncs.insert("__fcntl_nocancel");
    syscallFuncs.insert("__flock");
    syscallFuncs.insert("__fstatfs64");
    syscallFuncs.insert("__ftruncate64");
    syscallFuncs.insert("__futimes");
    syscallFuncs.insert("__fxstat");
    syscallFuncs.insert("__fxstat64");
    syscallFuncs.insert("__fxstatat");
    syscallFuncs.insert("__fxstatat64");
    syscallFuncs.insert("__gai_sigqueue");
    syscallFuncs.insert("__gconv_load_cache");
    syscallFuncs.insert("__getcwd");
    syscallFuncs.insert("__getdents");
    syscallFuncs.insert("__getdents64");
    syscallFuncs.insert("__getegid");
    syscallFuncs.insert("__geteuid");
    syscallFuncs.insert("__getgid");
    syscallFuncs.insert("__getitimer");
    syscallFuncs.insert("__getpgid");
    syscallFuncs.insert("__getpid");
    syscallFuncs.insert("__getppid");
    syscallFuncs.insert("__getrlimit");
    syscallFuncs.insert("__getrusage");
    syscallFuncs.insert("__getsockopt");
    syscallFuncs.insert("__getuid");
    syscallFuncs.insert("__ioctl");
    syscallFuncs.insert("__kill");
    syscallFuncs.insert("__lchown");
    syscallFuncs.insert("__libc_accept");
    syscallFuncs.insert("__libc_close");
    syscallFuncs.insert("__libc_creat");
    syscallFuncs.insert("__libc_fork");
    syscallFuncs.insert("__libc_fsync");
    syscallFuncs.insert("__libc_lseek");
    syscallFuncs.insert("__libc_message");
    syscallFuncs.insert("__libc_msgrcv");
    syscallFuncs.insert("__libc_msgsnd");
    syscallFuncs.insert("__libc_open");
    syscallFuncs.insert("__libc_read");
    syscallFuncs.insert("__libc_recv");
    syscallFuncs.insert("__libc_recvfrom");
    syscallFuncs.insert("__libc_send");
    syscallFuncs.insert("__libc_setup_tls");
    syscallFuncs.insert("__libc_sigaction");
    syscallFuncs.insert("__libc_wait");
    syscallFuncs.insert("__libc_write");
    syscallFuncs.insert("__link");
    syscallFuncs.insert("__lll_mutex_lock_wait");
    syscallFuncs.insert("__lll_mutex_unlock_wake");
    syscallFuncs.insert("__llseek");
    syscallFuncs.insert("__lxstat");
    syscallFuncs.insert("__lxstat64");
    syscallFuncs.insert("__mkdir");
    syscallFuncs.insert("__mmap");
    syscallFuncs.insert("__mmap64");
    syscallFuncs.insert("__modify_ldt");
    syscallFuncs.insert("__mount");
    syscallFuncs.insert("__mremap");
    syscallFuncs.insert("__nanosleep");
    syscallFuncs.insert("__netlink_close");
    syscallFuncs.insert("__new_msgctl");
    syscallFuncs.insert("__new_semctl");
    syscallFuncs.insert("__new_shmctl");
    syscallFuncs.insert("__nscd_getai");
    syscallFuncs.insert("__nscd_getgrouplist");
    syscallFuncs.insert("__nscd_open_socket");
    syscallFuncs.insert("__open_catalog");
    syscallFuncs.insert("__openat64_nocancel");
    syscallFuncs.insert("__openat_nocancel");
    syscallFuncs.insert("__personality");
    syscallFuncs.insert("__pipe");
    syscallFuncs.insert("__prctl");
    syscallFuncs.insert("__read_chk");
    syscallFuncs.insert("__readahead");
    syscallFuncs.insert("__readlink_chk");
    syscallFuncs.insert("__remap_file_pages");
    syscallFuncs.insert("__res_iclose");
    syscallFuncs.insert("__rmdir");
    syscallFuncs.insert("__sched_getaffinity_new");
    syscallFuncs.insert("__sched_rr_get_interval");
    syscallFuncs.insert("__sched_yield");
    syscallFuncs.insert("__select");
    syscallFuncs.insert("__sendmsg");
    syscallFuncs.insert("__sendto");
    syscallFuncs.insert("__setgid");
    syscallFuncs.insert("__setitimer");
    syscallFuncs.insert("__setresgid");
    syscallFuncs.insert("__setresuid");
    syscallFuncs.insert("__setrlimit");
    syscallFuncs.insert("__setsid");
    syscallFuncs.insert("__setuid");
    syscallFuncs.insert("__sigprocmask");
    syscallFuncs.insert("__sigqueue");
    syscallFuncs.insert("__sleep");
    syscallFuncs.insert("__socketpair");
    syscallFuncs.insert("__statfs");
    syscallFuncs.insert("__swapcontext");
    syscallFuncs.insert("__swapoff");
    syscallFuncs.insert("__swapon");
    syscallFuncs.insert("__symlink");
    syscallFuncs.insert("__sysctl");
    syscallFuncs.insert("__tcgetattr");
    syscallFuncs.insert("__times");
    syscallFuncs.insert("__umask");
    syscallFuncs.insert("__umount");
    syscallFuncs.insert("__umount2");
    syscallFuncs.insert("__uname");
    syscallFuncs.insert("__unlink");
    syscallFuncs.insert("__utimes");
    syscallFuncs.insert("__vfork");
    syscallFuncs.insert("__wait4");
    syscallFuncs.insert("__waitpid");
    syscallFuncs.insert("__xmknod");
    syscallFuncs.insert("__xmknodat");
    syscallFuncs.insert("__xstat");
    syscallFuncs.insert("_dl_debug_vdprintf");
    syscallFuncs.insert("_dl_get_origin");
    syscallFuncs.insert("_dl_map_object_from_fd");
    syscallFuncs.insert("_dl_sysinfo_int80");
    syscallFuncs.insert("_exit");
    syscallFuncs.insert("_nl_load_domain");
    syscallFuncs.insert("_nl_load_locale");
    syscallFuncs.insert("_nl_load_locale_from_archive");
    syscallFuncs.insert("acct");
    syscallFuncs.insert("adjtimex");
    syscallFuncs.insert("alarm");
    syscallFuncs.insert("bdflush");
    syscallFuncs.insert("cancel_handler");
    syscallFuncs.insert("capget");
    syscallFuncs.insert("capset");
    syscallFuncs.insert("chroot");
    syscallFuncs.insert("connect");
    syscallFuncs.insert("create_module");
    syscallFuncs.insert("daemon");
    syscallFuncs.insert("delete_module");
    syscallFuncs.insert("do_pread");
    syscallFuncs.insert("do_pread64");
    syscallFuncs.insert("do_pwrite");
    syscallFuncs.insert("do_pwrite64");
    syscallFuncs.insert("do_readv");
    syscallFuncs.insert("do_sigsuspend");
    syscallFuncs.insert("do_sigtimedwait");
    syscallFuncs.insert("do_sigwait");
    syscallFuncs.insert("do_sigwaitinfo");
    syscallFuncs.insert("do_waitid");
    syscallFuncs.insert("do_writev");
    syscallFuncs.insert("endutent_file");
    syscallFuncs.insert("epoll_create");
    syscallFuncs.insert("epoll_ctl");
    syscallFuncs.insert("epoll_wait");
    syscallFuncs.insert("execve");
    syscallFuncs.insert("faccessat");
    syscallFuncs.insert("fchmodat");
    syscallFuncs.insert("fchownat");
    syscallFuncs.insert("fdatasync");
    syscallFuncs.insert("fgetxattr");
    syscallFuncs.insert("flistxattr");
    syscallFuncs.insert("fremovexattr");
    syscallFuncs.insert("fsetxattr");
    syscallFuncs.insert("fstatfs");
    syscallFuncs.insert("ftruncate");
    syscallFuncs.insert("ftw_dir");
    syscallFuncs.insert("ftw_dir");
    syscallFuncs.insert("futimesat");
    syscallFuncs.insert("get_kernel_syms");
    syscallFuncs.insert("getaddrinfo");
    syscallFuncs.insert("getcontext");
    syscallFuncs.insert("getgroups");
    syscallFuncs.insert("gethostid");
    syscallFuncs.insert("getloadavg");
    syscallFuncs.insert("getmsg");
    syscallFuncs.insert("getpeername");
    syscallFuncs.insert("getpgrp");
    syscallFuncs.insert("getpmsg");
    syscallFuncs.insert("getpriority");
    syscallFuncs.insert("getresgid");
    syscallFuncs.insert("getresuid");
    syscallFuncs.insert("getsid");
    syscallFuncs.insert("getsockname");
    syscallFuncs.insert("gettimeofday");
    syscallFuncs.insert("getxattr");
    syscallFuncs.insert("herror");
    syscallFuncs.insert("if_indextoname");
    syscallFuncs.insert("if_nameindex_ioctl");
    syscallFuncs.insert("if_nametoindex");
    syscallFuncs.insert("init_module");
    syscallFuncs.insert("inotify_add_watch");
    syscallFuncs.insert("inotify_init");
    syscallFuncs.insert("inotify_rm_watch");
    syscallFuncs.insert("ioperm");
    syscallFuncs.insert("iopl");
    syscallFuncs.insert("klogctl");
    syscallFuncs.insert("lgetxattr");
    syscallFuncs.insert("linkat");
    syscallFuncs.insert("listen");
    syscallFuncs.insert("listxattr");
    syscallFuncs.insert("llistxattr");
    syscallFuncs.insert("lockf64");
    syscallFuncs.insert("loser_poll");
    syscallFuncs.insert("lremovexattr");
    syscallFuncs.insert("lsetxattr");
    syscallFuncs.insert("madvise");
    syscallFuncs.insert("make_request");
    syscallFuncs.insert("mincore");
    syscallFuncs.insert("mkdirat");
    syscallFuncs.insert("mlock");
    syscallFuncs.insert("mlockall");
    syscallFuncs.insert("mprotect");
    syscallFuncs.insert("msgget");
    syscallFuncs.insert("msync");
    syscallFuncs.insert("munlock");
    syscallFuncs.insert("munlockall");
    syscallFuncs.insert("munmap");
    syscallFuncs.insert("nfsservctl");
    syscallFuncs.insert("nscd_getgr_r");
    syscallFuncs.insert("nscd_gethst_r");
    syscallFuncs.insert("nscd_getpw_r");
    syscallFuncs.insert("open64");
    syscallFuncs.insert("open_socket");
    syscallFuncs.insert("opendir");
    syscallFuncs.insert("pivot_root");
    syscallFuncs.insert("posix_fadvise");
    syscallFuncs.insert("posix_fadvise64@@GLIBC_2.3.3");
    syscallFuncs.insert("posix_fadvise64@GLIBC_2.2");
    syscallFuncs.insert("posix_madvise");
    syscallFuncs.insert("ppoll");
    syscallFuncs.insert("ptrace");
    syscallFuncs.insert("putmsg");
    syscallFuncs.insert("putpmsg");
    syscallFuncs.insert("query_module");
    syscallFuncs.insert("quotactl");
    syscallFuncs.insert("raise");
    syscallFuncs.insert("readlink");
    syscallFuncs.insert("readlinkat");
    syscallFuncs.insert("reboot");
    syscallFuncs.insert("recvmsg");
    syscallFuncs.insert("removexattr");
    syscallFuncs.insert("rename");
    syscallFuncs.insert("renameat");
    syscallFuncs.insert("sched_get_priority_max");
    syscallFuncs.insert("sched_get_priority_min");
    syscallFuncs.insert("sched_getparam");
    syscallFuncs.insert("sched_getscheduler");
    syscallFuncs.insert("sched_setaffinity");
    syscallFuncs.insert("sched_setparam");
    syscallFuncs.insert("sched_setscheduler");
    syscallFuncs.insert("semget");
    syscallFuncs.insert("semop");
    syscallFuncs.insert("semtimedop");
    syscallFuncs.insert("sendfile");
    syscallFuncs.insert("sendfile64");
    syscallFuncs.insert("setcontext");
    syscallFuncs.insert("setdomainname");
    syscallFuncs.insert("setfsgid");
    syscallFuncs.insert("setfsuid");
    syscallFuncs.insert("setgroups");
    syscallFuncs.insert("sethostid");
    syscallFuncs.insert("sethostname");
    syscallFuncs.insert("setpgid");
    syscallFuncs.insert("setpriority");
    syscallFuncs.insert("setregid");
    syscallFuncs.insert("setreuid");
    syscallFuncs.insert("setsockopt");
    syscallFuncs.insert("settimeofday");
    syscallFuncs.insert("setutent_file");
    syscallFuncs.insert("setxattr");
    syscallFuncs.insert("shmat");
    syscallFuncs.insert("shmdt");
    syscallFuncs.insert("shmget");
    syscallFuncs.insert("shutdown");
    syscallFuncs.insert("sigaltstack");
    syscallFuncs.insert("sigpending");
    syscallFuncs.insert("socket");
    syscallFuncs.insert("splice");
    syscallFuncs.insert("statfs64");
    syscallFuncs.insert("stime");
    syscallFuncs.insert("symlinkat");
    syscallFuncs.insert("sync");
    syscallFuncs.insert("syscall");
    syscallFuncs.insert("sysconf");
    syscallFuncs.insert("sysinfo");
    syscallFuncs.insert("tcdrain");
    syscallFuncs.insert("tcsetattr");
    syscallFuncs.insert("tee");
    syscallFuncs.insert("time");
    syscallFuncs.insert("truncate");
    syscallFuncs.insert("truncate64");
    syscallFuncs.insert("unlinkat");
    syscallFuncs.insert("unshare");
    syscallFuncs.insert("updwtmp_file");
    syscallFuncs.insert("uselib");
    syscallFuncs.insert("ustat");
    syscallFuncs.insert("utime");
    syscallFuncs.insert("vhangup");
    syscallFuncs.insert("vmsplice");
    syscallFuncs.insert("write_gmon");
    
    set< vector<int> > traps;

    char* mappingType = getenv("MAPPING_TYPE");
    if (!mappingType) {
        mappingType="tpc";
    } else {
        cout << "mapping type = " << mappingType << endl;
    }

    if (!strcmp(mappingType,"tpc")) {
        //if (getObject()->isStaticBinary()) 

        traps.clear(); makeTrapVector(traps, 1, -1);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__libc_fork");
        traps.clear(); makeTrapVector(traps, 1, 11);addMapping(traps, makeCallSet(0), "execve");
        traps.clear(); makeTrapVector(traps, 1, 110);addMapping(traps, makeCallSet(0), "iopl");
        traps.clear(); makeTrapVector(traps, 1, 122);addMapping(traps, makeCallSet(0), "__uname");
        traps.clear(); makeTrapVector(traps, 1, 123);addMapping(traps, makeCallSet(0), "__modify_ldt");
        traps.clear(); makeTrapVector(traps, 1, 127);addMapping(traps, makeCallSet(0), "create_module");
        traps.clear(); makeTrapVector(traps, 1, 130);addMapping(traps, makeCallSet(0), "get_kernel_syms");
        traps.clear(); makeTrapVector(traps, 1, 140);addMapping(traps, makeCallSet(0), "__llseek");
        traps.clear(); makeTrapVector(traps, 1, 142);addMapping(traps, makeCallSet(0), "__select");
        traps.clear(); makeTrapVector(traps, 1, 149);addMapping(traps, makeCallSet(0), "__sysctl");
        traps.clear(); makeTrapVector(traps, 1, 167);addMapping(traps, makeCallSet(0), "query_module");
        traps.clear(); makeTrapVector(traps, 1, 186);addMapping(traps, makeCallSet(0), "sigaltstack");
        traps.clear(); makeTrapVector(traps, 1, 2);makeTrapVector(traps, 1, 190);addMapping(traps, makeCallSet(0), "__vfork");
        traps.clear(); makeTrapVector(traps, 1, 243);addMapping(traps, makeCallSet(0), "__libc_setup_tls");
        traps.clear(); makeTrapVector(traps, 1, 243);addMapping(traps, makeCallSet(7, "__libc_lseek","__fxstat64","mprotect","__mmap","__libc_close","__libc_read","munmap"), "_dl_map_object_from_fd");
        traps.clear(); makeTrapVector(traps, 1, 52);addMapping(traps, makeCallSet(0), "__umount2");
        traps.clear(); makeTrapVector(traps, 1, 67);makeTrapVector(traps, 1, 174);addMapping(traps, makeCallSet(0), "__libc_sigaction");
        traps.clear(); makeTrapVector(traps, 1, 72);makeTrapVector(traps, 1, 179);addMapping(traps, makeCallSet(0), "do_sigsuspend");
        traps.clear(); makeTrapVector(traps, 1, 76);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__getrlimit");
        traps.clear(); makeTrapVector(traps, 1, 90);addMapping(traps, makeCallSet(0), "__mmap");
        traps.clear(); makeTrapVector(traps, 1, 90);makeTrapVector(traps, 1, 192);addMapping(traps, makeCallSet(1, "setsockopt"), "__mmap64");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);addMapping(traps, makeCallSet(0), "__exit_thread");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);makeTrapVector(traps, 2, 20,-1);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__clone");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);makeTrapVector(traps, 2, 252,-1);addMapping(traps, makeCallSet(0), "_exit");
        traps.clear(); makeTrapVector(traps, 2, 10,-1);addMapping(traps, makeCallSet(0), "__unlink");
        traps.clear(); makeTrapVector(traps, 2, 10,-1);makeTrapVector(traps, 2, 40,-1);makeTrapVector(traps, 4, 301,-1,-1,-1);addMapping(traps, makeCallSet(0), "unlinkat");
        traps.clear(); makeTrapVector(traps, 2, 111,-1);addMapping(traps, makeCallSet(0), "vhangup");
        traps.clear(); makeTrapVector(traps, 2, 115,-1);addMapping(traps, makeCallSet(0), "__swapoff");
        traps.clear(); makeTrapVector(traps, 2, 116,-1);addMapping(traps, makeCallSet(0), "sysinfo");
        traps.clear(); makeTrapVector(traps, 2, 118,-1);addMapping(traps, makeCallSet(0), "__libc_fsync");
        traps.clear(); makeTrapVector(traps, 2, 12,-1);addMapping(traps, makeCallSet(0), "__chdir");
        traps.clear(); makeTrapVector(traps, 2, 124,-1);addMapping(traps, makeCallSet(0), "adjtimex");
        traps.clear(); makeTrapVector(traps, 2, 13,-1);addMapping(traps, makeCallSet(0), "time");
        traps.clear(); makeTrapVector(traps, 2, 132,-1);addMapping(traps, makeCallSet(0), "__getpgid");
        traps.clear(); makeTrapVector(traps, 2, 133,-1);addMapping(traps, makeCallSet(0), "__fchdir");
        traps.clear(); makeTrapVector(traps, 2, 136,-1);addMapping(traps, makeCallSet(0), "__personality");
        traps.clear(); makeTrapVector(traps, 2, 138,-1);makeTrapVector(traps, 1, 215);addMapping(traps, makeCallSet(0), "setfsuid");
        traps.clear(); makeTrapVector(traps, 2, 139,-1);makeTrapVector(traps, 1, 216);addMapping(traps, makeCallSet(0), "setfsgid");
        traps.clear(); makeTrapVector(traps, 2, 147,-1);addMapping(traps, makeCallSet(0), "getsid");
        traps.clear(); makeTrapVector(traps, 2, 148,-1);addMapping(traps, makeCallSet(0), "fdatasync");
        traps.clear(); makeTrapVector(traps, 2, 152,-1);addMapping(traps, makeCallSet(0), "mlockall");
        traps.clear(); makeTrapVector(traps, 2, 153,-1);addMapping(traps, makeCallSet(0), "munlockall");
        traps.clear(); makeTrapVector(traps, 2, 157,-1);addMapping(traps, makeCallSet(0), "sched_getscheduler");
        traps.clear(); makeTrapVector(traps, 2, 158,-1);addMapping(traps, makeCallSet(0), "__sched_yield");
        traps.clear(); makeTrapVector(traps, 2, 159,-1);addMapping(traps, makeCallSet(0), "sched_get_priority_max");
        traps.clear(); makeTrapVector(traps, 2, 160,-1);addMapping(traps, makeCallSet(0), "sched_get_priority_min");
        traps.clear(); makeTrapVector(traps, 2, 20,-1);addMapping(traps, makeCallSet(0), "__getpid");
        traps.clear(); makeTrapVector(traps, 2, 224,-1);makeTrapVector(traps, 3, 238,-1,-1);makeTrapVector(traps, 4, 270,-1,-1,-1);addMapping(traps, makeCallSet(0), "raise");
        traps.clear(); makeTrapVector(traps, 2, 23,-1);makeTrapVector(traps, 1, 213);addMapping(traps, makeCallSet(0), "__setuid");
        traps.clear(); makeTrapVector(traps, 2, 24,-1);makeTrapVector(traps, 1, 199);addMapping(traps, makeCallSet(0), "__getuid");
        traps.clear(); makeTrapVector(traps, 2, 25,-1);addMapping(traps, makeCallSet(0), "stime");
        traps.clear(); makeTrapVector(traps, 2, 254,-1);addMapping(traps, makeCallSet(0), "epoll_create");
        traps.clear(); makeTrapVector(traps, 2, 27,-1);addMapping(traps, makeCallSet(0), "alarm");
        traps.clear(); makeTrapVector(traps, 2, 29,-1);addMapping(traps, makeCallSet(2, "__sigprocmask","__nanosleep"), "__sleep");
        traps.clear(); makeTrapVector(traps, 2, 291,-1);addMapping(traps, makeCallSet(0), "inotify_init");
        traps.clear(); makeTrapVector(traps, 2, 310,-1);addMapping(traps, makeCallSet(0), "unshare");
        traps.clear(); makeTrapVector(traps, 2, 36,-1);addMapping(traps, makeCallSet(0), "sync");
        traps.clear(); makeTrapVector(traps, 2, 40,-1);addMapping(traps, makeCallSet(0), "__rmdir");
        traps.clear(); makeTrapVector(traps, 2, 41,-1);addMapping(traps, makeCallSet(0), "__dup");
        traps.clear(); makeTrapVector(traps, 2, 42,-1);addMapping(traps, makeCallSet(0), "__pipe");
        traps.clear(); makeTrapVector(traps, 2, 43,-1);addMapping(traps, makeCallSet(0), "__times");
        traps.clear(); makeTrapVector(traps, 2, 45,-1);addMapping(traps, makeCallSet(0), "__brk");
        traps.clear(); makeTrapVector(traps, 2, 46,-1);makeTrapVector(traps, 1, 214);addMapping(traps, makeCallSet(0), "__setgid");
        traps.clear(); makeTrapVector(traps, 2, 47,-1);makeTrapVector(traps, 1, 200);addMapping(traps, makeCallSet(0), "__getgid");
        traps.clear(); makeTrapVector(traps, 2, 49,-1);makeTrapVector(traps, 1, 201);addMapping(traps, makeCallSet(0), "__geteuid");
        traps.clear(); makeTrapVector(traps, 2, 50,-1);makeTrapVector(traps, 1, 202);addMapping(traps, makeCallSet(0), "__getegid");
        traps.clear(); makeTrapVector(traps, 2, 51,-1);addMapping(traps, makeCallSet(0), "acct");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(1, "__libc_open"), "sethostid");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(4, "__chdir", "__fchdir", "__openat64_nocancel", "opendir"), "ftw_dir");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(2, "connect","socket"), "open_socket");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(2, "open_socket","__libc_read"), "__nscd_open_socket");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(3, "__sendto","time","recvmsg"), "make_request");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(4, "__alloc_dir","__fxstat64","___xstat64","__libc_open"), "opendir");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(4, "__llseek","__access","__libc_open","__fcntl_nocancel"), "setutent_file");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(4, "connect","getsockname","___xstat64","socket"), "getaddrinfo");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(5, "__llseek","__ftruncate64","alarm","__libc_open","__fcntl_nocancel"), "updwtmp_file");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(5, "sysconf","__fxstat64","__mmap64","munmap","__libc_open"), "_nl_load_locale_from_archive");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(7, "__lll_mutex_unlock_wake","__fxstat64","__lll_mutex_lock_wait","__mmap","__libc_read","munmap","__libc_open"), "_nl_load_domain");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);addMapping(traps, makeCallSet(8, "__setsid","__chdir","__fxstat64","_exit","__libc_close","__dup2","__libc_fork","__libc_open"), "daemon");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 1, 266);addMapping(traps, makeCallSet(2, "__libc_read","__libc_open"), "sysconf");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 4, 146,-1,-1,-1);addMapping(traps, makeCallSet(2, "__libc_read","__libc_open"), "__libc_message");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 4, 146,-1,-1,-1);makeTrapVector(traps, 4, 146,-1,-1,2);makeTrapVector(traps, 4, 146,-1,-1,3);makeTrapVector(traps, 4, 146,-1,-1,64);addMapping(traps, makeCallSet(2, "__getpid","__libc_open"), "write_gmon");
        traps.clear(); makeTrapVector(traps, 2, 60,-1);addMapping(traps, makeCallSet(0), "__umask");
        traps.clear(); makeTrapVector(traps, 2, 61,-1);addMapping(traps, makeCallSet(0), "chroot");
        traps.clear(); makeTrapVector(traps, 2, 64,-1);addMapping(traps, makeCallSet(0), "__getppid");
        traps.clear(); makeTrapVector(traps, 2, 65,-1);addMapping(traps, makeCallSet(0), "getpgrp");
        traps.clear(); makeTrapVector(traps, 2, 66,-1);addMapping(traps, makeCallSet(0), "__setsid");
        traps.clear(); makeTrapVector(traps, 2, 73,-1);makeTrapVector(traps, 3, 176,-1,8);addMapping(traps, makeCallSet(0), "sigpending");
        traps.clear(); makeTrapVector(traps, 2, 86,-1);addMapping(traps, makeCallSet(0), "uselib");
        traps.clear(); makeTrapVector(traps, 3, 100,-1,-1);addMapping(traps, makeCallSet(0), "fstatfs");
        traps.clear(); makeTrapVector(traps, 3, 105,-1,-1);addMapping(traps, makeCallSet(0), "__getitimer");
        traps.clear(); makeTrapVector(traps, 3, 121,-1,-1);addMapping(traps, makeCallSet(0), "setdomainname");
        traps.clear(); makeTrapVector(traps, 3, 129,-1,-1);addMapping(traps, makeCallSet(0), "delete_module");
        traps.clear(); makeTrapVector(traps, 3, 134,-1,-1);addMapping(traps, makeCallSet(0), "bdflush");
        traps.clear(); makeTrapVector(traps, 3, 143,-1,-1);addMapping(traps, makeCallSet(0), "__flock");
        traps.clear(); makeTrapVector(traps, 3, 15,-1,-1);addMapping(traps, makeCallSet(0), "__chmod");
        traps.clear(); makeTrapVector(traps, 3, 15,-1,-1);makeTrapVector(traps, 4, 306,-1,-1,-1);addMapping(traps, makeCallSet(0), "fchmodat");
        traps.clear(); makeTrapVector(traps, 3, 150,-1,-1);addMapping(traps, makeCallSet(0), "mlock");
        traps.clear(); makeTrapVector(traps, 3, 151,-1,-1);addMapping(traps, makeCallSet(0), "munlock");
        traps.clear(); makeTrapVector(traps, 3, 154,-1,-1);addMapping(traps, makeCallSet(0), "sched_setparam");
        traps.clear(); makeTrapVector(traps, 3, 155,-1,-1);addMapping(traps, makeCallSet(0), "sched_getparam");
        traps.clear(); makeTrapVector(traps, 3, 161,-1,-1);addMapping(traps, makeCallSet(0), "__sched_rr_get_interval");
        traps.clear(); makeTrapVector(traps, 3, 162,-1,-1);addMapping(traps, makeCallSet(0), "__nanosleep");
        traps.clear(); makeTrapVector(traps, 3, 183,-1,-1);addMapping(traps, makeCallSet(1, "readlink"), "__getcwd");
        traps.clear(); makeTrapVector(traps, 3, 184,-1,-1);addMapping(traps, makeCallSet(0), "capget");
        traps.clear(); makeTrapVector(traps, 3, 185,-1,-1);addMapping(traps, makeCallSet(0), "capset");
        traps.clear(); makeTrapVector(traps, 3, 193,-1,-1);addMapping(traps, makeCallSet(1, "truncate"), "truncate64");
        traps.clear(); makeTrapVector(traps, 3, 194,-1,-1);addMapping(traps, makeCallSet(1, "ftruncate"), "__ftruncate64");
        traps.clear(); makeTrapVector(traps, 3, 217,-1,-1);addMapping(traps, makeCallSet(0), "pivot_root");
        traps.clear(); makeTrapVector(traps, 3, 22,-1,-1);addMapping(traps, makeCallSet(0), "__umount");
        traps.clear(); makeTrapVector(traps, 3, 235,-1,-1);addMapping(traps, makeCallSet(0), "removexattr");
        traps.clear(); makeTrapVector(traps, 3, 236,-1,-1);addMapping(traps, makeCallSet(0), "lremovexattr");
        traps.clear(); makeTrapVector(traps, 3, 237,-1,-1);addMapping(traps, makeCallSet(0), "fremovexattr");
        traps.clear(); makeTrapVector(traps, 3, 293,-1,-1);addMapping(traps, makeCallSet(0), "inotify_rm_watch");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);addMapping(traps, makeCallSet(0), "utime");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 3, 271,-1,-1);addMapping(traps, makeCallSet(0), "__utimes");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 3, 271,-1,-1);makeTrapVector(traps, 4, 299,-1,-1,-1);addMapping(traps, makeCallSet(1, "__futimes"), "futimesat");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 4, 55,-1,1,-1);makeTrapVector(traps, 3, 271,-1,-1);addMapping(traps, makeCallSet(0), "__futimes");
        traps.clear(); makeTrapVector(traps, 3, 33,-1,-1);addMapping(traps, makeCallSet(0), "__access");
        traps.clear(); makeTrapVector(traps, 3, 33,-1,-1);makeTrapVector(traps, 4, 307,-1,-1,-1);addMapping(traps, makeCallSet(5, "__getegid","__geteuid","__fxstatat64","__getuid","__getgid"), "faccessat");
        traps.clear(); makeTrapVector(traps, 3, 37,-1,-1);addMapping(traps, makeCallSet(0), "__kill");
        traps.clear(); makeTrapVector(traps, 3, 37,-1,9);addMapping(traps, makeCallSet(3, "__lll_mutex_unlock_wake","__waitpid","__lll_mutex_lock_wait"), "cancel_handler");
        traps.clear(); makeTrapVector(traps, 3, 38,-1,-1);addMapping(traps, makeCallSet(0), "rename");
        traps.clear(); makeTrapVector(traps, 3, 38,-1,-1);makeTrapVector(traps, 5, 302,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "renameat");
        traps.clear(); makeTrapVector(traps, 3, 39,-1,-1);addMapping(traps, makeCallSet(0), "__mkdir");
        traps.clear(); makeTrapVector(traps, 3, 39,-1,-1);makeTrapVector(traps, 4, 296,-1,-1,-1);addMapping(traps, makeCallSet(0), "mkdirat");
        traps.clear(); makeTrapVector(traps, 3, 57,-1,-1);addMapping(traps, makeCallSet(0), "setpgid");
        traps.clear(); makeTrapVector(traps, 3, 62,-1,-1);addMapping(traps, makeCallSet(0), "ustat");
        traps.clear(); makeTrapVector(traps, 3, 63,-1,-1);addMapping(traps, makeCallSet(0), "__dup2");
        traps.clear(); makeTrapVector(traps, 3, 70,-1,-1);makeTrapVector(traps, 1, 203);addMapping(traps, makeCallSet(0), "setreuid");
        traps.clear(); makeTrapVector(traps, 3, 71,-1,-1);makeTrapVector(traps, 1, 204);addMapping(traps, makeCallSet(0), "setregid");
        traps.clear(); makeTrapVector(traps, 3, 74,-1,-1);addMapping(traps, makeCallSet(0), "sethostname");
        traps.clear(); makeTrapVector(traps, 3, 75,-1,-1);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__setrlimit");
        traps.clear(); makeTrapVector(traps, 3, 77,-1,-1);addMapping(traps, makeCallSet(0), "__getrusage");
        traps.clear(); makeTrapVector(traps, 3, 78,-1,-1);addMapping(traps, makeCallSet(0), "gettimeofday");
        traps.clear(); makeTrapVector(traps, 3, 79,-1,-1);addMapping(traps, makeCallSet(0), "settimeofday");
        traps.clear(); makeTrapVector(traps, 3, 8,-1,-1);addMapping(traps, makeCallSet(0), "__libc_creat");
        traps.clear(); makeTrapVector(traps, 3, 80,-1,-1);makeTrapVector(traps, 1, 205);addMapping(traps, makeCallSet(1, "sysconf"), "getgroups");
        traps.clear(); makeTrapVector(traps, 3, 81,-1,-1);makeTrapVector(traps, 1, 206);addMapping(traps, makeCallSet(1, "sysconf"), "setgroups");
        traps.clear(); makeTrapVector(traps, 3, 83,-1,-1);addMapping(traps, makeCallSet(0), "__symlink");
        traps.clear(); makeTrapVector(traps, 3, 83,-1,-1);makeTrapVector(traps, 4, 304,-1,-1,-1);addMapping(traps, makeCallSet(0), "symlinkat");
        traps.clear(); makeTrapVector(traps, 3, 87,-1,-1);addMapping(traps, makeCallSet(0), "__swapon");
        traps.clear(); makeTrapVector(traps, 3, 9,-1,-1);addMapping(traps, makeCallSet(0), "__link");
        traps.clear(); makeTrapVector(traps, 3, 9,-1,-1);makeTrapVector(traps, 6, 303,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "linkat");
        traps.clear(); makeTrapVector(traps, 3, 91,-1,-1);addMapping(traps, makeCallSet(0), "munmap");
        traps.clear(); makeTrapVector(traps, 3, 92,-1,-1);addMapping(traps, makeCallSet(0), "truncate");
        traps.clear(); makeTrapVector(traps, 3, 93,-1,-1);addMapping(traps, makeCallSet(0), "ftruncate");
        traps.clear(); makeTrapVector(traps, 3, 94,-1,-1);addMapping(traps, makeCallSet(0), "__fchmod");
        traps.clear(); makeTrapVector(traps, 3, 96,-1,-1);addMapping(traps, makeCallSet(0), "getpriority");
        traps.clear(); makeTrapVector(traps, 3, 99,-1,-1);addMapping(traps, makeCallSet(0), "__statfs");
        traps.clear(); makeTrapVector(traps, 4, 101,-1,-1,-1);addMapping(traps, makeCallSet(0), "ioperm");
        traps.clear(); makeTrapVector(traps, 4, 102,1,-1,-1);addMapping(traps, makeCallSet(0), "socket");
        traps.clear(); makeTrapVector(traps, 4, 102,10,-1,-1);addMapping(traps, makeCallSet(0), "__libc_recv");
        traps.clear(); makeTrapVector(traps, 4, 102,11,-1,-1);addMapping(traps, makeCallSet(0), "__sendto");
        traps.clear(); makeTrapVector(traps, 4, 102,12,-1,-1);addMapping(traps, makeCallSet(0), "__libc_recvfrom");
        traps.clear(); makeTrapVector(traps, 4, 102,13,-1,-1);addMapping(traps, makeCallSet(0), "shutdown");
        traps.clear(); makeTrapVector(traps, 4, 102,14,-1,-1);addMapping(traps, makeCallSet(0), "setsockopt");
        traps.clear(); makeTrapVector(traps, 4, 102,15,-1,-1);addMapping(traps, makeCallSet(0), "__getsockopt");
        traps.clear(); makeTrapVector(traps, 4, 102,16,-1,-1);addMapping(traps, makeCallSet(0), "__sendmsg");
        traps.clear(); makeTrapVector(traps, 4, 102,17,-1,-1);addMapping(traps, makeCallSet(0), "recvmsg");
        traps.clear(); makeTrapVector(traps, 4, 102,2,-1,-1);addMapping(traps, makeCallSet(0), "__bind");
        traps.clear(); makeTrapVector(traps, 4, 102,3,-1,-1);addMapping(traps, makeCallSet(0), "connect");
        traps.clear(); makeTrapVector(traps, 4, 102,4,-1,-1);addMapping(traps, makeCallSet(0), "listen");
        traps.clear(); makeTrapVector(traps, 4, 102,5,-1,-1);addMapping(traps, makeCallSet(0), "__libc_accept");
        traps.clear(); makeTrapVector(traps, 4, 102,6,-1,-1);addMapping(traps, makeCallSet(0), "getsockname");
        traps.clear(); makeTrapVector(traps, 4, 102,7,-1,-1);addMapping(traps, makeCallSet(0), "getpeername");
        traps.clear(); makeTrapVector(traps, 4, 102,8,-1,-1);addMapping(traps, makeCallSet(0), "__socketpair");
        traps.clear(); makeTrapVector(traps, 4, 102,9,-1,-1);addMapping(traps, makeCallSet(0), "__libc_send");
        traps.clear(); makeTrapVector(traps, 4, 103,-1,-1,-1);addMapping(traps, makeCallSet(0), "klogctl");
        traps.clear(); makeTrapVector(traps, 4, 104,-1,-1,-1);addMapping(traps, makeCallSet(0), "__setitimer");
        traps.clear(); makeTrapVector(traps, 4, 125,-1,-1,-1);addMapping(traps, makeCallSet(0), "mprotect");
        traps.clear(); makeTrapVector(traps, 4, 126,-1,-1,-1);makeTrapVector(traps, 5, 175,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__sigprocmask");
        traps.clear(); makeTrapVector(traps, 4, 126,0,-1,-1);addMapping(traps, makeCallSet(0), "getcontext");
        traps.clear(); makeTrapVector(traps, 4, 128,-1,-1,-1);addMapping(traps, makeCallSet(0), "init_module");
        traps.clear(); makeTrapVector(traps, 4, 14,-1,-1,-1);addMapping(traps, makeCallSet(0), "__xmknod");
        traps.clear(); makeTrapVector(traps, 4, 14,-1,-1,-1);makeTrapVector(traps, 5, 297,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__xmknodat");
        traps.clear(); makeTrapVector(traps, 4, 144,-1,-1,-1);addMapping(traps, makeCallSet(0), "msync");
        traps.clear(); makeTrapVector(traps, 4, 145,-1,-1,-1);addMapping(traps, makeCallSet(0), "do_readv");
        traps.clear(); makeTrapVector(traps, 4, 146,-1,-1,-1);addMapping(traps, makeCallSet(0), "do_writev");
        traps.clear(); makeTrapVector(traps, 4, 146,-1,-1,-1);addMapping(traps, makeCallSet(1, "__getpid"), "_dl_debug_vdprintf");
        traps.clear(); makeTrapVector(traps, 4, 146,2,-1,-1);addMapping(traps, makeCallSet(0), "herror");
        traps.clear(); makeTrapVector(traps, 4, 156,-1,-1,-1);addMapping(traps, makeCallSet(0), "sched_setscheduler");
        traps.clear(); makeTrapVector(traps, 4, 16,-1,-1,-1);makeTrapVector(traps, 1, 198);addMapping(traps, makeCallSet(0), "__lchown");
        traps.clear(); makeTrapVector(traps, 4, 164,-1,-1,-1);makeTrapVector(traps, 1, 208);addMapping(traps, makeCallSet(0), "__setresuid");
        traps.clear(); makeTrapVector(traps, 4, 165,-1,-1,-1);makeTrapVector(traps, 1, 209);addMapping(traps, makeCallSet(0), "getresuid");
        traps.clear(); makeTrapVector(traps, 4, 168,-1,-1,-1);addMapping(traps, makeCallSet(0), "loser_poll");
        traps.clear(); makeTrapVector(traps, 4, 169,-1,-1,-1);addMapping(traps, makeCallSet(0), "nfsservctl");
        traps.clear(); makeTrapVector(traps, 4, 170,-1,-1,-1);makeTrapVector(traps, 1, 210);addMapping(traps, makeCallSet(0), "__setresgid");
        traps.clear(); makeTrapVector(traps, 4, 171,-1,-1,-1);makeTrapVector(traps, 1, 211);addMapping(traps, makeCallSet(0), "getresgid");
        traps.clear(); makeTrapVector(traps, 4, 178,-1,-1,-1);addMapping(traps, makeCallSet(1, "__getuid"), "__gai_sigqueue");
        traps.clear(); makeTrapVector(traps, 4, 178,-1,-1,-1);addMapping(traps, makeCallSet(2, "__getpid","__getuid"), "__sigqueue");
        traps.clear(); makeTrapVector(traps, 4, 182,-1,-1,-1);makeTrapVector(traps, 1, 212);addMapping(traps, makeCallSet(1, "__lchown"), "__chown");
        traps.clear(); makeTrapVector(traps, 4, 19,-1,-1,-1);addMapping(traps, makeCallSet(0), "__libc_lseek");
        traps.clear(); makeTrapVector(traps, 4, 218,-1,-1,-1);addMapping(traps, makeCallSet(0), "mincore");
        traps.clear(); makeTrapVector(traps, 4, 221,-1,-1,-1);makeTrapVector(traps, 4, 221,-1,12,-1);addMapping(traps, makeCallSet(1, "__getpid"), "lockf64");
        traps.clear(); makeTrapVector(traps, 4, 225,-1,-1,-1);addMapping(traps, makeCallSet(0), "__readahead");
        traps.clear(); makeTrapVector(traps, 4, 232,-1,-1,-1);addMapping(traps, makeCallSet(0), "listxattr");
        traps.clear(); makeTrapVector(traps, 4, 233,-1,-1,-1);addMapping(traps, makeCallSet(0), "llistxattr");
        traps.clear(); makeTrapVector(traps, 4, 234,-1,-1,-1);addMapping(traps, makeCallSet(0), "flistxattr");
        traps.clear(); makeTrapVector(traps, 4, 241,-1,-1,-1);makeTrapVector(traps, 4, 242,-1,-1,-1);addMapping(traps, makeCallSet(1, "__getpid"), "sched_setaffinity");
        traps.clear(); makeTrapVector(traps, 4, 242,-1,-1,-1);addMapping(traps, makeCallSet(0), "__sched_getaffinity_new");
        traps.clear(); makeTrapVector(traps, 4, 268,-1,84,-1);addMapping(traps, makeCallSet(1, "__statfs"), "statfs64");
        traps.clear(); makeTrapVector(traps, 4, 269,-1,84,-1);addMapping(traps, makeCallSet(1, "fstatfs"), "__fstatfs64");
        traps.clear(); makeTrapVector(traps, 4, 292,-1,-1,-1);addMapping(traps, makeCallSet(0), "inotify_add_watch");
        traps.clear(); makeTrapVector(traps, 4, 4,-1,-1,-1);addMapping(traps, makeCallSet(0), "__libc_write");
        traps.clear(); makeTrapVector(traps, 4, 54,-1,21505,-1);addMapping(traps, makeCallSet(0), "__tcgetattr");
        traps.clear(); makeTrapVector(traps, 4, 54,-1,21513,1);addMapping(traps, makeCallSet(0), "tcdrain");
        traps.clear(); makeTrapVector(traps, 4, 55,-1,-1,-1);makeTrapVector(traps, 4, 55,-1,5,-1);makeTrapVector(traps, 4, 221,-1,-1,-1);addMapping(traps, makeCallSet(0), "__fcntl_nocancel");
        traps.clear(); makeTrapVector(traps, 4, 7,-1,-1,-1);addMapping(traps, makeCallSet(0), "__waitpid");
        traps.clear(); makeTrapVector(traps, 4, 85,-1,-1,-1);makeTrapVector(traps, 5, 305,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "readlinkat");
        traps.clear(); makeTrapVector(traps, 4, 85,135522375,-1,4096);addMapping(traps, makeCallSet(0), "_dl_get_origin");
        traps.clear(); makeTrapVector(traps, 4, 95,-1,-1,-1);makeTrapVector(traps, 1, 207);addMapping(traps, makeCallSet(0), "__fchown");
        traps.clear(); makeTrapVector(traps, 4, 97,-1,-1,-1);addMapping(traps, makeCallSet(0), "setpriority");
        traps.clear(); makeTrapVector(traps, 5, 131,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "quotactl");
        traps.clear(); makeTrapVector(traps, 5, 180,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__libc_lseek","__libc_read"), "do_pread");
        traps.clear(); makeTrapVector(traps, 5, 180,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__llseek","__libc_read"), "do_pread64");
        traps.clear(); makeTrapVector(traps, 5, 181,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__libc_lseek","__libc_write"), "do_pwrite");
        traps.clear(); makeTrapVector(traps, 5, 181,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__llseek","__libc_write"), "do_pwrite64");
        traps.clear(); makeTrapVector(traps, 5, 187,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "sendfile");
        traps.clear(); makeTrapVector(traps, 5, 229,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "getxattr");
        traps.clear(); makeTrapVector(traps, 5, 230,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "lgetxattr");
        traps.clear(); makeTrapVector(traps, 5, 231,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "fgetxattr");
        traps.clear(); makeTrapVector(traps, 5, 239,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "sendfile64");
        traps.clear(); makeTrapVector(traps, 5, 250,-1,-1,-1,-1);makeTrapVector(traps, 5, 272,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "posix_fadvise64@@GLIBC_2.3.3");
        traps.clear(); makeTrapVector(traps, 5, 255,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "epoll_ctl");
        traps.clear(); makeTrapVector(traps, 5, 256,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "epoll_wait");
        traps.clear(); makeTrapVector(traps, 5, 26,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "ptrace");
        traps.clear(); makeTrapVector(traps, 5, 315,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "tee");
        traps.clear(); makeTrapVector(traps, 5, 316,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "vmsplice");
        traps.clear(); makeTrapVector(traps, 5, 88,-18751827,672274793,-1,-1);addMapping(traps, makeCallSet(0), "reboot");
        traps.clear(); makeTrapVector(traps, 6, 163,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__mremap");
        traps.clear(); makeTrapVector(traps, 6, 172,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__prctl");
        traps.clear(); makeTrapVector(traps, 6, 21,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__mount");
        traps.clear(); makeTrapVector(traps, 6, 226,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "setxattr");
        traps.clear(); makeTrapVector(traps, 6, 227,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "lsetxattr");
        traps.clear(); makeTrapVector(traps, 6, 228,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "fsetxattr");
        traps.clear(); makeTrapVector(traps, 6, 257,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__remap_file_pages");
        traps.clear(); makeTrapVector(traps, 6, 284,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(1, "__waitpid"), "do_waitid");
        traps.clear(); makeTrapVector(traps, 6, 298,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__lchown","__chown"), "fchownat");
        traps.clear(); makeTrapVector(traps, 6, 309,-1,-1,-1,8,-1);addMapping(traps, makeCallSet(1, "__sigprocmask"), "ppoll");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(2, "__lll_mutex_unlock_wake","__lll_mutex_lock_wait"), "_L_lock_80");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,-1,2,-1,-1,-1);addMapping(traps, makeCallSet(0), "__lll_mutex_lock_wait");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,1,1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__lll_mutex_unlock_wake");
        traps.clear(); makeTrapVector(traps, 7, 308,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__call_pselect6");
        traps.clear(); makeTrapVector(traps, 7, 313,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "splice");
    } else if (!strcmp(mappingType, "tc")) {
        cout << "making map for TC" << endl;
        traps.clear(); makeTrapVector(traps,1,-1);makeTrapVector(traps,1,120);addMapping(traps, makeCallSet(0), "__libc_fork");
        traps.clear(); makeTrapVector(traps,1,1);addMapping(traps, makeCallSet(0), "__exit_thread");
        traps.clear(); makeTrapVector(traps,1,1);makeTrapVector(traps,1,20);makeTrapVector(traps,1,120);addMapping(traps, makeCallSet(0), "__clone");
        traps.clear(); makeTrapVector(traps,1,1);makeTrapVector(traps,1,252);addMapping(traps, makeCallSet(0), "_exit");
        traps.clear(); makeTrapVector(traps,1,10);addMapping(traps, makeCallSet(0), "__unlink");
        traps.clear(); makeTrapVector(traps,1,10);makeTrapVector(traps,1,40);makeTrapVector(traps,1,301);addMapping(traps, makeCallSet(0), "unlinkat");
        traps.clear(); makeTrapVector(traps,1,100);addMapping(traps, makeCallSet(0), "fstatfs");
        traps.clear(); makeTrapVector(traps,1,101);addMapping(traps, makeCallSet(0), "ioperm");
        traps.clear(); makeTrapVector(traps,1,103);addMapping(traps, makeCallSet(0), "klogctl");
        traps.clear(); makeTrapVector(traps,1,104);addMapping(traps, makeCallSet(0), "__setitimer");
        traps.clear(); makeTrapVector(traps,1,105);addMapping(traps, makeCallSet(0), "__getitimer");
        traps.clear(); makeTrapVector(traps,1,11);addMapping(traps, makeCallSet(0), "execve");
        traps.clear(); makeTrapVector(traps,1,110);addMapping(traps, makeCallSet(0), "iopl");
        traps.clear(); makeTrapVector(traps,1,111);addMapping(traps, makeCallSet(0), "vhangup");
        traps.clear(); makeTrapVector(traps,1,115);addMapping(traps, makeCallSet(0), "__swapoff");
        traps.clear(); makeTrapVector(traps,1,116);addMapping(traps, makeCallSet(0), "sysinfo");
        traps.clear(); makeTrapVector(traps,1,118);addMapping(traps, makeCallSet(0), "__libc_fsync");
        traps.clear(); makeTrapVector(traps,1,12);addMapping(traps, makeCallSet(0), "__chdir");
        traps.clear(); makeTrapVector(traps,1,121);addMapping(traps, makeCallSet(0), "setdomainname");
        traps.clear(); makeTrapVector(traps,1,122);addMapping(traps, makeCallSet(0), "__uname");
        traps.clear(); makeTrapVector(traps,1,123);addMapping(traps, makeCallSet(0), "__modify_ldt");
        traps.clear(); makeTrapVector(traps,1,124);addMapping(traps, makeCallSet(0), "adjtimex");
        traps.clear(); makeTrapVector(traps,1,125);addMapping(traps, makeCallSet(0), "mprotect");
        traps.clear(); makeTrapVector(traps,1,126);makeTrapVector(traps,1,175);addMapping(traps, makeCallSet(0), "__sigprocmask");
        traps.clear(); makeTrapVector(traps,1,127);addMapping(traps, makeCallSet(0), "create_module");
        traps.clear(); makeTrapVector(traps,1,128);addMapping(traps, makeCallSet(0), "init_module");
        traps.clear(); makeTrapVector(traps,1,129);addMapping(traps, makeCallSet(0), "delete_module");
        traps.clear(); makeTrapVector(traps,1,13);addMapping(traps, makeCallSet(0), "time");
        traps.clear(); makeTrapVector(traps,1,130);addMapping(traps, makeCallSet(0), "get_kernel_syms");
        traps.clear(); makeTrapVector(traps,1,131);addMapping(traps, makeCallSet(0), "quotactl");
        traps.clear(); makeTrapVector(traps,1,132);addMapping(traps, makeCallSet(0), "__getpgid");
        traps.clear(); makeTrapVector(traps,1,133);addMapping(traps, makeCallSet(0), "__fchdir");
        traps.clear(); makeTrapVector(traps,1,134);addMapping(traps, makeCallSet(0), "bdflush");
        traps.clear(); makeTrapVector(traps,1,136);addMapping(traps, makeCallSet(0), "__personality");
        traps.clear(); makeTrapVector(traps,1,138);makeTrapVector(traps,1,215);addMapping(traps, makeCallSet(0), "setfsuid");
        traps.clear(); makeTrapVector(traps,1,139);makeTrapVector(traps,1,216);addMapping(traps, makeCallSet(0), "setfsgid");
        traps.clear(); makeTrapVector(traps,1,14);addMapping(traps, makeCallSet(0), "__xmknod");
        traps.clear(); makeTrapVector(traps,1,14);makeTrapVector(traps,1,297);addMapping(traps, makeCallSet(0), "__xmknodat");
        traps.clear(); makeTrapVector(traps,1,140);addMapping(traps, makeCallSet(0), "__llseek");
        traps.clear(); makeTrapVector(traps,1,142);addMapping(traps, makeCallSet(0), "__select");
        traps.clear(); makeTrapVector(traps,1,143);addMapping(traps, makeCallSet(0), "__flock");
        traps.clear(); makeTrapVector(traps,1,144);addMapping(traps, makeCallSet(0), "msync");
        traps.clear(); makeTrapVector(traps,1,145);addMapping(traps, makeCallSet(0), "do_readv");
        traps.clear(); makeTrapVector(traps,1,146);addMapping(traps, makeCallSet(1, "__getpid"), "_dl_debug_vdprintf");
        traps.clear(); makeTrapVector(traps,1,147);addMapping(traps, makeCallSet(0), "getsid");
        traps.clear(); makeTrapVector(traps,1,148);addMapping(traps, makeCallSet(0), "fdatasync");
        traps.clear(); makeTrapVector(traps,1,149);addMapping(traps, makeCallSet(0), "__sysctl");
        traps.clear(); makeTrapVector(traps,1,15);addMapping(traps, makeCallSet(0), "__chmod");
        traps.clear(); makeTrapVector(traps,1,15);makeTrapVector(traps,1,306);addMapping(traps, makeCallSet(0), "fchmodat");
        traps.clear(); makeTrapVector(traps,1,150);addMapping(traps, makeCallSet(0), "mlock");
        traps.clear(); makeTrapVector(traps,1,151);addMapping(traps, makeCallSet(0), "munlock");
        traps.clear(); makeTrapVector(traps,1,152);addMapping(traps, makeCallSet(0), "mlockall");
        traps.clear(); makeTrapVector(traps,1,153);addMapping(traps, makeCallSet(0), "munlockall");
        traps.clear(); makeTrapVector(traps,1,154);addMapping(traps, makeCallSet(0), "sched_setparam");
        traps.clear(); makeTrapVector(traps,1,155);addMapping(traps, makeCallSet(0), "sched_getparam");
        traps.clear(); makeTrapVector(traps,1,156);addMapping(traps, makeCallSet(0), "sched_setscheduler");
        traps.clear(); makeTrapVector(traps,1,157);addMapping(traps, makeCallSet(0), "sched_getscheduler");
        traps.clear(); makeTrapVector(traps,1,158);addMapping(traps, makeCallSet(0), "__sched_yield");
        traps.clear(); makeTrapVector(traps,1,159);addMapping(traps, makeCallSet(0), "sched_get_priority_max");
        traps.clear(); makeTrapVector(traps,1,16);makeTrapVector(traps,1,198);addMapping(traps, makeCallSet(0), "__lchown");
        traps.clear(); makeTrapVector(traps,1,160);addMapping(traps, makeCallSet(0), "sched_get_priority_min");
        traps.clear(); makeTrapVector(traps,1,161);addMapping(traps, makeCallSet(0), "__sched_rr_get_interval");
        traps.clear(); makeTrapVector(traps,1,162);addMapping(traps, makeCallSet(0), "__nanosleep");
        traps.clear(); makeTrapVector(traps,1,163);addMapping(traps, makeCallSet(0), "__mremap");
        traps.clear(); makeTrapVector(traps,1,164);makeTrapVector(traps,1,208);addMapping(traps, makeCallSet(0), "__setresuid");
        traps.clear(); makeTrapVector(traps,1,165);makeTrapVector(traps,1,209);addMapping(traps, makeCallSet(0), "getresuid");
        traps.clear(); makeTrapVector(traps,1,167);addMapping(traps, makeCallSet(0), "query_module");
        traps.clear(); makeTrapVector(traps,1,168);addMapping(traps, makeCallSet(0), "loser_poll");
        traps.clear(); makeTrapVector(traps,1,169);addMapping(traps, makeCallSet(0), "nfsservctl");
        traps.clear(); makeTrapVector(traps,1,170);makeTrapVector(traps,1,210);addMapping(traps, makeCallSet(0), "__setresgid");
        traps.clear(); makeTrapVector(traps,1,171);makeTrapVector(traps,1,211);addMapping(traps, makeCallSet(0), "getresgid");
        traps.clear(); makeTrapVector(traps,1,172);addMapping(traps, makeCallSet(0), "__prctl");
        traps.clear(); makeTrapVector(traps,1,178);addMapping(traps, makeCallSet(1, "__getuid"), "__gai_sigqueue");
        traps.clear(); makeTrapVector(traps,1,178);addMapping(traps, makeCallSet(2, "__getpid","__getuid"), "__sigqueue");
        traps.clear(); makeTrapVector(traps,1,180);addMapping(traps, makeCallSet(2, "__libc_lseek","__libc_read"), "do_pread");
        traps.clear(); makeTrapVector(traps,1,180);addMapping(traps, makeCallSet(2, "__libc_read","__llseek"), "do_pread64");
        traps.clear(); makeTrapVector(traps,1,181);addMapping(traps, makeCallSet(2, "__libc_lseek","__libc_write"), "do_pwrite");
        traps.clear(); makeTrapVector(traps,1,181);addMapping(traps, makeCallSet(2, "__libc_write","__llseek"), "do_pwrite64");
        traps.clear(); makeTrapVector(traps,1,182);makeTrapVector(traps,1,212);addMapping(traps, makeCallSet(1, "__lchown"), "__chown");
        traps.clear(); makeTrapVector(traps,1,183);addMapping(traps, makeCallSet(1, "readlink"), "__getcwd");
        traps.clear(); makeTrapVector(traps,1,184);addMapping(traps, makeCallSet(0), "capget");
        traps.clear(); makeTrapVector(traps,1,185);addMapping(traps, makeCallSet(0), "capset");
        traps.clear(); makeTrapVector(traps,1,186);addMapping(traps, makeCallSet(0), "sigaltstack");
        traps.clear(); makeTrapVector(traps,1,187);addMapping(traps, makeCallSet(0), "sendfile");
        traps.clear(); makeTrapVector(traps,1,19);addMapping(traps, makeCallSet(0), "__libc_lseek");
        traps.clear(); makeTrapVector(traps,1,193);addMapping(traps, makeCallSet(1, "truncate"), "truncate64");
        traps.clear(); makeTrapVector(traps,1,194);addMapping(traps, makeCallSet(1, "ftruncate"), "__ftruncate64");
        traps.clear(); makeTrapVector(traps,1,2);makeTrapVector(traps,1,190);addMapping(traps, makeCallSet(0), "__vfork");
        traps.clear(); makeTrapVector(traps,1,20);addMapping(traps, makeCallSet(0), "__getpid");
        traps.clear(); makeTrapVector(traps,1,21);addMapping(traps, makeCallSet(0), "__mount");
        traps.clear(); makeTrapVector(traps,1,217);addMapping(traps, makeCallSet(0), "pivot_root");
        traps.clear(); makeTrapVector(traps,1,218);addMapping(traps, makeCallSet(0), "mincore");
        traps.clear(); makeTrapVector(traps,1,22);addMapping(traps, makeCallSet(0), "__umount");
        traps.clear(); makeTrapVector(traps,1,221);addMapping(traps, makeCallSet(1, "__getpid"), "lockf64");
        traps.clear(); makeTrapVector(traps,1,224);makeTrapVector(traps,1,238);makeTrapVector(traps,1,270);addMapping(traps, makeCallSet(0), "raise");
        traps.clear(); makeTrapVector(traps,1,225);addMapping(traps, makeCallSet(0), "__readahead");
        traps.clear(); makeTrapVector(traps,1,226);addMapping(traps, makeCallSet(0), "setxattr");
        traps.clear(); makeTrapVector(traps,1,227);addMapping(traps, makeCallSet(0), "lsetxattr");
        traps.clear(); makeTrapVector(traps,1,228);addMapping(traps, makeCallSet(0), "fsetxattr");
        traps.clear(); makeTrapVector(traps,1,229);addMapping(traps, makeCallSet(0), "getxattr");
        traps.clear(); makeTrapVector(traps,1,23);makeTrapVector(traps,1,213);addMapping(traps, makeCallSet(0), "__setuid");
        traps.clear(); makeTrapVector(traps,1,230);addMapping(traps, makeCallSet(0), "lgetxattr");
        traps.clear(); makeTrapVector(traps,1,231);addMapping(traps, makeCallSet(0), "fgetxattr");
        traps.clear(); makeTrapVector(traps,1,232);addMapping(traps, makeCallSet(0), "listxattr");
        traps.clear(); makeTrapVector(traps,1,233);addMapping(traps, makeCallSet(0), "llistxattr");
        traps.clear(); makeTrapVector(traps,1,234);addMapping(traps, makeCallSet(0), "flistxattr");
        traps.clear(); makeTrapVector(traps,1,235);addMapping(traps, makeCallSet(0), "removexattr");
        traps.clear(); makeTrapVector(traps,1,236);addMapping(traps, makeCallSet(0), "lremovexattr");
        traps.clear(); makeTrapVector(traps,1,237);addMapping(traps, makeCallSet(0), "fremovexattr");
        traps.clear(); makeTrapVector(traps,1,239);addMapping(traps, makeCallSet(0), "sendfile64");
        traps.clear(); makeTrapVector(traps,1,24);makeTrapVector(traps,1,199);addMapping(traps, makeCallSet(0), "__getuid");
        traps.clear(); makeTrapVector(traps,1,240);addMapping(traps, makeCallSet(2, "__lll_mutex_lock_wait","__lll_mutex_unlock_wake"), "_L_lock_80");
        traps.clear(); makeTrapVector(traps,1,241);makeTrapVector(traps,1,242);addMapping(traps, makeCallSet(1, "__getpid"), "sched_setaffinity");
        traps.clear(); makeTrapVector(traps,1,242);addMapping(traps, makeCallSet(0), "__sched_getaffinity_new");
        traps.clear(); makeTrapVector(traps,1,243);addMapping(traps, makeCallSet(0), "__libc_setup_tls");
        traps.clear(); makeTrapVector(traps,1,243);addMapping(traps, makeCallSet(7, "__fxstat64","__libc_close","__libc_lseek","__libc_read","__mmap","mprotect","munmap"), "_dl_map_object_from_fd");
        traps.clear(); makeTrapVector(traps,1,25);addMapping(traps, makeCallSet(0), "stime");
        traps.clear(); makeTrapVector(traps,1,250);makeTrapVector(traps,1,272);addMapping(traps, makeCallSet(0), "posix_fadvise64@@GLIBC_2.3.3");
        traps.clear(); makeTrapVector(traps,1,254);addMapping(traps, makeCallSet(0), "epoll_create");
        traps.clear(); makeTrapVector(traps,1,255);addMapping(traps, makeCallSet(0), "epoll_ctl");
        traps.clear(); makeTrapVector(traps,1,256);addMapping(traps, makeCallSet(0), "epoll_wait");
        traps.clear(); makeTrapVector(traps,1,257);addMapping(traps, makeCallSet(0), "__remap_file_pages");
        traps.clear(); makeTrapVector(traps,1,26);addMapping(traps, makeCallSet(0), "ptrace");
        traps.clear(); makeTrapVector(traps,1,268);addMapping(traps, makeCallSet(1, "__statfs"), "statfs64");
        traps.clear(); makeTrapVector(traps,1,269);addMapping(traps, makeCallSet(1, "fstatfs"), "__fstatfs64");
        traps.clear(); makeTrapVector(traps,1,27);addMapping(traps, makeCallSet(0), "alarm");
        traps.clear(); makeTrapVector(traps,1,284);addMapping(traps, makeCallSet(1, "__waitpid"), "do_waitid");
        traps.clear(); makeTrapVector(traps,1,29);addMapping(traps, makeCallSet(2, "__nanosleep","__sigprocmask"), "__sleep");
        traps.clear(); makeTrapVector(traps,1,291);addMapping(traps, makeCallSet(0), "inotify_init");
        traps.clear(); makeTrapVector(traps,1,292);addMapping(traps, makeCallSet(0), "inotify_add_watch");
        traps.clear(); makeTrapVector(traps,1,293);addMapping(traps, makeCallSet(0), "inotify_rm_watch");
        traps.clear(); makeTrapVector(traps,1,298);addMapping(traps, makeCallSet(2, "__chown","__lchown"), "fchownat");
        traps.clear(); makeTrapVector(traps,1,30);addMapping(traps, makeCallSet(0), "utime");
        traps.clear(); makeTrapVector(traps,1,30);makeTrapVector(traps,1,271);addMapping(traps, makeCallSet(0), "__utimes");
        traps.clear(); makeTrapVector(traps,1,30);makeTrapVector(traps,1,271);makeTrapVector(traps,1,299);addMapping(traps, makeCallSet(1, "__futimes"), "futimesat");
        traps.clear(); makeTrapVector(traps,1,30);makeTrapVector(traps,1,55);makeTrapVector(traps,1,271);addMapping(traps, makeCallSet(0), "__futimes");
        traps.clear(); makeTrapVector(traps,1,308);addMapping(traps, makeCallSet(0), "__call_pselect6");
        traps.clear(); makeTrapVector(traps,1,309);addMapping(traps, makeCallSet(1, "__sigprocmask"), "ppoll");
        traps.clear(); makeTrapVector(traps,1,310);addMapping(traps, makeCallSet(0), "unshare");
        traps.clear(); makeTrapVector(traps,1,313);addMapping(traps, makeCallSet(0), "splice");
        traps.clear(); makeTrapVector(traps,1,315);addMapping(traps, makeCallSet(0), "tee");
        traps.clear(); makeTrapVector(traps,1,316);addMapping(traps, makeCallSet(0), "vmsplice");
        traps.clear(); makeTrapVector(traps,1,33);addMapping(traps, makeCallSet(0), "__access");
        traps.clear(); makeTrapVector(traps,1,33);makeTrapVector(traps,1,307);addMapping(traps, makeCallSet(5, "__fxstatat64","__getegid","__geteuid","__getgid","__getuid"), "faccessat");
        traps.clear(); makeTrapVector(traps,1,36);addMapping(traps, makeCallSet(0), "sync");
        traps.clear(); makeTrapVector(traps,1,37);addMapping(traps, makeCallSet(0), "__kill");
        traps.clear(); makeTrapVector(traps,1,37);addMapping(traps, makeCallSet(3, "__lll_mutex_lock_wait","__lll_mutex_unlock_wake","__waitpid"), "cancel_handler");
        traps.clear(); makeTrapVector(traps,1,38);addMapping(traps, makeCallSet(0), "rename");
        traps.clear(); makeTrapVector(traps,1,38);makeTrapVector(traps,1,302);addMapping(traps, makeCallSet(0), "renameat");
        traps.clear(); makeTrapVector(traps,1,39);addMapping(traps, makeCallSet(0), "__mkdir");
        traps.clear(); makeTrapVector(traps,1,39);makeTrapVector(traps,1,296);addMapping(traps, makeCallSet(0), "mkdirat");
        traps.clear(); makeTrapVector(traps,1,4);addMapping(traps, makeCallSet(0), "__libc_write");
        traps.clear(); makeTrapVector(traps,1,40);addMapping(traps, makeCallSet(0), "__rmdir");
        traps.clear(); makeTrapVector(traps,1,41);addMapping(traps, makeCallSet(0), "__dup");
        traps.clear(); makeTrapVector(traps,1,42);addMapping(traps, makeCallSet(0), "__pipe");
        traps.clear(); makeTrapVector(traps,1,43);addMapping(traps, makeCallSet(0), "__times");
        traps.clear(); makeTrapVector(traps,1,45);addMapping(traps, makeCallSet(0), "__brk");
        traps.clear(); makeTrapVector(traps,1,46);makeTrapVector(traps,1,214);addMapping(traps, makeCallSet(0), "__setgid");
        traps.clear(); makeTrapVector(traps,1,47);makeTrapVector(traps,1,200);addMapping(traps, makeCallSet(0), "__getgid");
        traps.clear(); makeTrapVector(traps,1,49);makeTrapVector(traps,1,201);addMapping(traps, makeCallSet(0), "__geteuid");
        traps.clear(); makeTrapVector(traps,1,50);makeTrapVector(traps,1,202);addMapping(traps, makeCallSet(0), "__getegid");
        traps.clear(); makeTrapVector(traps,1,51);addMapping(traps, makeCallSet(0), "acct");
        traps.clear(); makeTrapVector(traps,1,52);addMapping(traps, makeCallSet(0), "__umount2");
        traps.clear(); makeTrapVector(traps,1,55);makeTrapVector(traps,1,221);addMapping(traps, makeCallSet(0), "__fcntl_nocancel");
        traps.clear(); makeTrapVector(traps,1,57);addMapping(traps, makeCallSet(0), "setpgid");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(4, "__chdir", "__fchdir", "__openat64_nocancel", "opendir"), "ftw_dir");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(1, "__libc_open"), "sethostid");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(2, "__libc_read","open_socket"), "__nscd_open_socket");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(2, "connect","socket"), "open_socket");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(3, "__sendto","recvmsg","time"), "make_request");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(4, "___xstat64","__alloc_dir","__fxstat64","__libc_open"), "opendir");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(4, "___xstat64","connect","getsockname","socket"), "getaddrinfo");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(4, "__access","__fcntl_nocancel","__libc_open","__llseek"), "setutent_file");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(5, "__fcntl_nocancel","__ftruncate64","__libc_open","__llseek","alarm"), "updwtmp_file");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(5, "__fxstat64","__libc_open","__mmap64","munmap","sysconf"), "_nl_load_locale_from_archive");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(7, "__fxstat64","__libc_open","__libc_read","__lll_mutex_lock_wait","__lll_mutex_unlock_wake","__mmap","munmap"), "_nl_load_domain");
        traps.clear(); makeTrapVector(traps,1,6);addMapping(traps, makeCallSet(8, "__chdir","__dup2","__fxstat64","__libc_close","__libc_fork","__libc_open","__setsid","_exit"), "daemon");
        traps.clear(); makeTrapVector(traps,1,6);makeTrapVector(traps,1,146);addMapping(traps, makeCallSet(2, "__getpid","__libc_open"), "write_gmon");
        traps.clear(); makeTrapVector(traps,1,6);makeTrapVector(traps,1,146);addMapping(traps, makeCallSet(2, "__libc_open","__libc_read"), "__libc_message");
        traps.clear(); makeTrapVector(traps,1,6);makeTrapVector(traps,1,266);addMapping(traps, makeCallSet(2, "__libc_open","__libc_read"), "sysconf");
        traps.clear(); makeTrapVector(traps,1,60);addMapping(traps, makeCallSet(0), "__umask");
        traps.clear(); makeTrapVector(traps,1,61);addMapping(traps, makeCallSet(0), "chroot");
        traps.clear(); makeTrapVector(traps,1,62);addMapping(traps, makeCallSet(0), "ustat");
        traps.clear(); makeTrapVector(traps,1,63);addMapping(traps, makeCallSet(0), "__dup2");
        traps.clear(); makeTrapVector(traps,1,64);addMapping(traps, makeCallSet(0), "__getppid");
        traps.clear(); makeTrapVector(traps,1,65);addMapping(traps, makeCallSet(0), "getpgrp");
        traps.clear(); makeTrapVector(traps,1,66);addMapping(traps, makeCallSet(0), "__setsid");
        traps.clear(); makeTrapVector(traps,1,67);makeTrapVector(traps,1,174);addMapping(traps, makeCallSet(0), "__libc_sigaction");
        traps.clear(); makeTrapVector(traps,1,7);addMapping(traps, makeCallSet(0), "__waitpid");
        traps.clear(); makeTrapVector(traps,1,70);makeTrapVector(traps,1,203);addMapping(traps, makeCallSet(0), "setreuid");
        traps.clear(); makeTrapVector(traps,1,71);makeTrapVector(traps,1,204);addMapping(traps, makeCallSet(0), "setregid");
        traps.clear(); makeTrapVector(traps,1,72);makeTrapVector(traps,1,179);addMapping(traps, makeCallSet(0), "do_sigsuspend");
        traps.clear(); makeTrapVector(traps,1,73);makeTrapVector(traps,1,176);addMapping(traps, makeCallSet(0), "sigpending");
        traps.clear(); makeTrapVector(traps,1,74);addMapping(traps, makeCallSet(0), "sethostname");
        traps.clear(); makeTrapVector(traps,1,75);makeTrapVector(traps,1,191);addMapping(traps, makeCallSet(0), "__setrlimit");
        traps.clear(); makeTrapVector(traps,1,76);makeTrapVector(traps,1,191);addMapping(traps, makeCallSet(0), "__getrlimit");
        traps.clear(); makeTrapVector(traps,1,77);addMapping(traps, makeCallSet(0), "__getrusage");
        traps.clear(); makeTrapVector(traps,1,78);addMapping(traps, makeCallSet(0), "gettimeofday");
        traps.clear(); makeTrapVector(traps,1,79);addMapping(traps, makeCallSet(0), "settimeofday");
        traps.clear(); makeTrapVector(traps,1,8);addMapping(traps, makeCallSet(0), "__libc_creat");
        traps.clear(); makeTrapVector(traps,1,80);makeTrapVector(traps,1,205);addMapping(traps, makeCallSet(1, "sysconf"), "getgroups");
        traps.clear(); makeTrapVector(traps,1,81);makeTrapVector(traps,1,206);addMapping(traps, makeCallSet(1, "sysconf"), "setgroups");
        traps.clear(); makeTrapVector(traps,1,83);addMapping(traps, makeCallSet(0), "__symlink");
        traps.clear(); makeTrapVector(traps,1,83);makeTrapVector(traps,1,304);addMapping(traps, makeCallSet(0), "symlinkat");
        traps.clear(); makeTrapVector(traps,1,85);makeTrapVector(traps,1,305);addMapping(traps, makeCallSet(0), "readlinkat");
        traps.clear(); makeTrapVector(traps,1,86);addMapping(traps, makeCallSet(0), "uselib");
        traps.clear(); makeTrapVector(traps,1,87);addMapping(traps, makeCallSet(0), "__swapon");
        traps.clear(); makeTrapVector(traps,1,88);addMapping(traps, makeCallSet(0), "reboot");
        traps.clear(); makeTrapVector(traps,1,9);addMapping(traps, makeCallSet(0), "__link");
        traps.clear(); makeTrapVector(traps,1,9);makeTrapVector(traps,1,303);addMapping(traps, makeCallSet(0), "linkat");
        traps.clear(); makeTrapVector(traps,1,90);addMapping(traps, makeCallSet(0), "__mmap");
        traps.clear(); makeTrapVector(traps,1,90);makeTrapVector(traps,1,192);addMapping(traps, makeCallSet(1, "setsockopt"), "__mmap64");
        traps.clear(); makeTrapVector(traps,1,91);addMapping(traps, makeCallSet(0), "munmap");
        traps.clear(); makeTrapVector(traps,1,92);addMapping(traps, makeCallSet(0), "truncate");
        traps.clear(); makeTrapVector(traps,1,93);addMapping(traps, makeCallSet(0), "ftruncate");
        traps.clear(); makeTrapVector(traps,1,94);addMapping(traps, makeCallSet(0), "__fchmod");
        traps.clear(); makeTrapVector(traps,1,95);makeTrapVector(traps,1,207);addMapping(traps, makeCallSet(0), "__fchown");
        traps.clear(); makeTrapVector(traps,1,96);addMapping(traps, makeCallSet(0), "getpriority");
        traps.clear(); makeTrapVector(traps,1,97);addMapping(traps, makeCallSet(0), "setpriority");
        traps.clear(); makeTrapVector(traps,1,99);addMapping(traps, makeCallSet(0), "__statfs");
    } else if (!strcmp(mappingType, "tp")) {

        traps.clear(); makeTrapVector(traps, 1, -1);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__libc_fork");
        traps.clear(); makeTrapVector(traps, 1, 11);addMapping(traps, makeCallSet(0), "execve");
        traps.clear(); makeTrapVector(traps, 1, 110);addMapping(traps, makeCallSet(0), "iopl");
        traps.clear(); makeTrapVector(traps, 1, 122);addMapping(traps, makeCallSet(0), "__uname");
        traps.clear(); makeTrapVector(traps, 1, 123);addMapping(traps, makeCallSet(0), "__modify_ldt");
        traps.clear(); makeTrapVector(traps, 1, 127);addMapping(traps, makeCallSet(0), "create_module");
        traps.clear(); makeTrapVector(traps, 1, 130);addMapping(traps, makeCallSet(0), "get_kernel_syms");
        traps.clear(); makeTrapVector(traps, 1, 140);addMapping(traps, makeCallSet(0), "__llseek");
        traps.clear(); makeTrapVector(traps, 1, 142);addMapping(traps, makeCallSet(0), "__select");
        traps.clear(); makeTrapVector(traps, 1, 149);addMapping(traps, makeCallSet(0), "__sysctl");
        traps.clear(); makeTrapVector(traps, 1, 167);addMapping(traps, makeCallSet(0), "query_module");
        traps.clear(); makeTrapVector(traps, 1, 186);addMapping(traps, makeCallSet(0), "sigaltstack");
        traps.clear(); makeTrapVector(traps, 1, 2);makeTrapVector(traps, 1, 190);addMapping(traps, makeCallSet(0), "__vfork");
        traps.clear(); makeTrapVector(traps, 1, 52);addMapping(traps, makeCallSet(0), "__umount2");
        traps.clear(); makeTrapVector(traps, 1, 67);makeTrapVector(traps, 1, 174);addMapping(traps, makeCallSet(0), "__libc_sigaction");
        traps.clear(); makeTrapVector(traps, 1, 72);makeTrapVector(traps, 1, 179);addMapping(traps, makeCallSet(0), "do_sigsuspend");
        traps.clear(); makeTrapVector(traps, 1, 76);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__getrlimit");
        traps.clear(); makeTrapVector(traps, 1, 90);addMapping(traps, makeCallSet(0), "__mmap");
        traps.clear(); makeTrapVector(traps, 1, 90);makeTrapVector(traps, 1, 192);addMapping(traps, makeCallSet(0), "__mmap64");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);addMapping(traps, makeCallSet(0), "__exit_thread");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);makeTrapVector(traps, 2, 20,-1);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__clone");
        traps.clear(); makeTrapVector(traps, 2, 1,-1);makeTrapVector(traps, 2, 252,-1);addMapping(traps, makeCallSet(0), "_exit");
        traps.clear(); makeTrapVector(traps, 2, 10,-1);addMapping(traps, makeCallSet(0), "__unlink");
        traps.clear(); makeTrapVector(traps, 2, 10,-1);makeTrapVector(traps, 2, 40,-1);makeTrapVector(traps, 4, 301,-1,-1,-1);addMapping(traps, makeCallSet(0), "unlinkat");
        traps.clear(); makeTrapVector(traps, 2, 111,-1);addMapping(traps, makeCallSet(0), "vhangup");
        traps.clear(); makeTrapVector(traps, 2, 115,-1);addMapping(traps, makeCallSet(0), "__swapoff");
        traps.clear(); makeTrapVector(traps, 2, 116,-1);addMapping(traps, makeCallSet(0), "sysinfo");
        traps.clear(); makeTrapVector(traps, 2, 118,-1);addMapping(traps, makeCallSet(0), "__libc_fsync");
        traps.clear(); makeTrapVector(traps, 2, 12,-1);addMapping(traps, makeCallSet(0), "__chdir");
        traps.clear(); makeTrapVector(traps, 2, 124,-1);addMapping(traps, makeCallSet(0), "adjtimex");
        traps.clear(); makeTrapVector(traps, 2, 13,-1);addMapping(traps, makeCallSet(0), "time");
        traps.clear(); makeTrapVector(traps, 2, 132,-1);addMapping(traps, makeCallSet(0), "__getpgid");
        traps.clear(); makeTrapVector(traps, 2, 133,-1);addMapping(traps, makeCallSet(0), "__fchdir");
        traps.clear(); makeTrapVector(traps, 2, 136,-1);addMapping(traps, makeCallSet(0), "__personality");
        traps.clear(); makeTrapVector(traps, 2, 138,-1);makeTrapVector(traps, 1, 215);addMapping(traps, makeCallSet(0), "setfsuid");
        traps.clear(); makeTrapVector(traps, 2, 139,-1);makeTrapVector(traps, 1, 216);addMapping(traps, makeCallSet(0), "setfsgid");
        traps.clear(); makeTrapVector(traps, 2, 147,-1);addMapping(traps, makeCallSet(0), "getsid");
        traps.clear(); makeTrapVector(traps, 2, 148,-1);addMapping(traps, makeCallSet(0), "fdatasync");
        traps.clear(); makeTrapVector(traps, 2, 152,-1);addMapping(traps, makeCallSet(0), "mlockall");
        traps.clear(); makeTrapVector(traps, 2, 153,-1);addMapping(traps, makeCallSet(0), "munlockall");
        traps.clear(); makeTrapVector(traps, 2, 157,-1);addMapping(traps, makeCallSet(0), "sched_getscheduler");
        traps.clear(); makeTrapVector(traps, 2, 158,-1);addMapping(traps, makeCallSet(0), "__sched_yield");
        traps.clear(); makeTrapVector(traps, 2, 159,-1);addMapping(traps, makeCallSet(0), "sched_get_priority_max");
        traps.clear(); makeTrapVector(traps, 2, 160,-1);addMapping(traps, makeCallSet(0), "sched_get_priority_min");
        traps.clear(); makeTrapVector(traps, 2, 20,-1);addMapping(traps, makeCallSet(0), "__getpid");
        traps.clear(); makeTrapVector(traps, 2, 224,-1);makeTrapVector(traps, 3, 238,-1,-1);makeTrapVector(traps, 4, 270,-1,-1,-1);addMapping(traps, makeCallSet(0), "raise");
        traps.clear(); makeTrapVector(traps, 2, 23,-1);makeTrapVector(traps, 1, 213);addMapping(traps, makeCallSet(0), "__setuid");
        traps.clear(); makeTrapVector(traps, 2, 24,-1);makeTrapVector(traps, 1, 199);addMapping(traps, makeCallSet(0), "__getuid");
        traps.clear(); makeTrapVector(traps, 2, 25,-1);addMapping(traps, makeCallSet(0), "stime");
        traps.clear(); makeTrapVector(traps, 2, 254,-1);addMapping(traps, makeCallSet(0), "epoll_create");
        traps.clear(); makeTrapVector(traps, 2, 27,-1);addMapping(traps, makeCallSet(0), "alarm");
        traps.clear(); makeTrapVector(traps, 2, 29,-1);addMapping(traps, makeCallSet(0), "__sleep");
        traps.clear(); makeTrapVector(traps, 2, 291,-1);addMapping(traps, makeCallSet(0), "inotify_init");
        traps.clear(); makeTrapVector(traps, 2, 310,-1);addMapping(traps, makeCallSet(0), "unshare");
        traps.clear(); makeTrapVector(traps, 2, 36,-1);addMapping(traps, makeCallSet(0), "sync");
        traps.clear(); makeTrapVector(traps, 2, 40,-1);addMapping(traps, makeCallSet(0), "__rmdir");
        traps.clear(); makeTrapVector(traps, 2, 41,-1);addMapping(traps, makeCallSet(0), "__dup");
        traps.clear(); makeTrapVector(traps, 2, 42,-1);addMapping(traps, makeCallSet(0), "__pipe");
        traps.clear(); makeTrapVector(traps, 2, 43,-1);addMapping(traps, makeCallSet(0), "__times");
        traps.clear(); makeTrapVector(traps, 2, 45,-1);addMapping(traps, makeCallSet(0), "__brk");
        traps.clear(); makeTrapVector(traps, 2, 46,-1);makeTrapVector(traps, 1, 214);addMapping(traps, makeCallSet(0), "__setgid");
        traps.clear(); makeTrapVector(traps, 2, 47,-1);makeTrapVector(traps, 1, 200);addMapping(traps, makeCallSet(0), "__getgid");
        traps.clear(); makeTrapVector(traps, 2, 49,-1);makeTrapVector(traps, 1, 201);addMapping(traps, makeCallSet(0), "__geteuid");
        traps.clear(); makeTrapVector(traps, 2, 50,-1);makeTrapVector(traps, 1, 202);addMapping(traps, makeCallSet(0), "__getegid");
        traps.clear(); makeTrapVector(traps, 2, 51,-1);addMapping(traps, makeCallSet(0), "acct");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 1, 266);addMapping(traps, makeCallSet(0), "sysconf");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 4, 146,-1,-1,-1);addMapping(traps, makeCallSet(0), "__libc_message");
        traps.clear(); makeTrapVector(traps, 2, 6,-1);makeTrapVector(traps, 4, 146,-1,-1,-1);makeTrapVector(traps, 4, 146,-1,-1,2);makeTrapVector(traps, 4, 146,-1,-1,3);makeTrapVector(traps, 4, 146,-1,-1,64);addMapping(traps, makeCallSet(0), "write_gmon");
        traps.clear(); makeTrapVector(traps, 2, 60,-1);addMapping(traps, makeCallSet(0), "__umask");
        traps.clear(); makeTrapVector(traps, 2, 61,-1);addMapping(traps, makeCallSet(0), "chroot");
        traps.clear(); makeTrapVector(traps, 2, 64,-1);addMapping(traps, makeCallSet(0), "__getppid");
        traps.clear(); makeTrapVector(traps, 2, 65,-1);addMapping(traps, makeCallSet(0), "getpgrp");
        traps.clear(); makeTrapVector(traps, 2, 66,-1);addMapping(traps, makeCallSet(0), "__setsid");
        traps.clear(); makeTrapVector(traps, 2, 73,-1);makeTrapVector(traps, 3, 176,-1,8);addMapping(traps, makeCallSet(0), "sigpending");
        traps.clear(); makeTrapVector(traps, 2, 86,-1);addMapping(traps, makeCallSet(0), "uselib");
        traps.clear(); makeTrapVector(traps, 3, 100,-1,-1);addMapping(traps, makeCallSet(0), "fstatfs");
        traps.clear(); makeTrapVector(traps, 3, 105,-1,-1);addMapping(traps, makeCallSet(0), "__getitimer");
        traps.clear(); makeTrapVector(traps, 3, 121,-1,-1);addMapping(traps, makeCallSet(0), "setdomainname");
        traps.clear(); makeTrapVector(traps, 3, 129,-1,-1);addMapping(traps, makeCallSet(0), "delete_module");
        traps.clear(); makeTrapVector(traps, 3, 134,-1,-1);addMapping(traps, makeCallSet(0), "bdflush");
        traps.clear(); makeTrapVector(traps, 3, 143,-1,-1);addMapping(traps, makeCallSet(0), "__flock");
        traps.clear(); makeTrapVector(traps, 3, 15,-1,-1);addMapping(traps, makeCallSet(0), "__chmod");
        traps.clear(); makeTrapVector(traps, 3, 15,-1,-1);makeTrapVector(traps, 4, 306,-1,-1,-1);addMapping(traps, makeCallSet(0), "fchmodat");
        traps.clear(); makeTrapVector(traps, 3, 150,-1,-1);addMapping(traps, makeCallSet(0), "mlock");
        traps.clear(); makeTrapVector(traps, 3, 151,-1,-1);addMapping(traps, makeCallSet(0), "munlock");
        traps.clear(); makeTrapVector(traps, 3, 154,-1,-1);addMapping(traps, makeCallSet(0), "sched_setparam");
        traps.clear(); makeTrapVector(traps, 3, 155,-1,-1);addMapping(traps, makeCallSet(0), "sched_getparam");
        traps.clear(); makeTrapVector(traps, 3, 161,-1,-1);addMapping(traps, makeCallSet(0), "__sched_rr_get_interval");
        traps.clear(); makeTrapVector(traps, 3, 162,-1,-1);addMapping(traps, makeCallSet(0), "__nanosleep");
        traps.clear(); makeTrapVector(traps, 3, 183,-1,-1);addMapping(traps, makeCallSet(0), "__getcwd");
        traps.clear(); makeTrapVector(traps, 3, 184,-1,-1);addMapping(traps, makeCallSet(0), "capget");
        traps.clear(); makeTrapVector(traps, 3, 185,-1,-1);addMapping(traps, makeCallSet(0), "capset");
        traps.clear(); makeTrapVector(traps, 3, 193,-1,-1);addMapping(traps, makeCallSet(0), "truncate64");
        traps.clear(); makeTrapVector(traps, 3, 194,-1,-1);addMapping(traps, makeCallSet(0), "__ftruncate64");
        traps.clear(); makeTrapVector(traps, 3, 217,-1,-1);addMapping(traps, makeCallSet(0), "pivot_root");
        traps.clear(); makeTrapVector(traps, 3, 22,-1,-1);addMapping(traps, makeCallSet(0), "__umount");
        traps.clear(); makeTrapVector(traps, 3, 235,-1,-1);addMapping(traps, makeCallSet(0), "removexattr");
        traps.clear(); makeTrapVector(traps, 3, 236,-1,-1);addMapping(traps, makeCallSet(0), "lremovexattr");
        traps.clear(); makeTrapVector(traps, 3, 237,-1,-1);addMapping(traps, makeCallSet(0), "fremovexattr");
        traps.clear(); makeTrapVector(traps, 3, 293,-1,-1);addMapping(traps, makeCallSet(0), "inotify_rm_watch");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);addMapping(traps, makeCallSet(0), "utime");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 3, 271,-1,-1);addMapping(traps, makeCallSet(0), "__utimes");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 3, 271,-1,-1);makeTrapVector(traps, 4, 299,-1,-1,-1);addMapping(traps, makeCallSet(0), "futimesat");
        traps.clear(); makeTrapVector(traps, 3, 30,-1,-1);makeTrapVector(traps, 4, 55,-1,1,-1);makeTrapVector(traps, 3, 271,-1,-1);addMapping(traps, makeCallSet(0), "__futimes");
        traps.clear(); makeTrapVector(traps, 3, 33,-1,-1);addMapping(traps, makeCallSet(0), "__access");
        traps.clear(); makeTrapVector(traps, 3, 33,-1,-1);makeTrapVector(traps, 4, 307,-1,-1,-1);addMapping(traps, makeCallSet(0), "faccessat");
        traps.clear(); makeTrapVector(traps, 3, 37,-1,-1);addMapping(traps, makeCallSet(0), "__kill");
        traps.clear(); makeTrapVector(traps, 3, 37,-1,9);addMapping(traps, makeCallSet(0), "cancel_handler");
        traps.clear(); makeTrapVector(traps, 3, 38,-1,-1);addMapping(traps, makeCallSet(0), "rename");
        traps.clear(); makeTrapVector(traps, 3, 38,-1,-1);makeTrapVector(traps, 5, 302,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "renameat");
        traps.clear(); makeTrapVector(traps, 3, 39,-1,-1);addMapping(traps, makeCallSet(0), "__mkdir");
        traps.clear(); makeTrapVector(traps, 3, 39,-1,-1);makeTrapVector(traps, 4, 296,-1,-1,-1);addMapping(traps, makeCallSet(0), "mkdirat");
        traps.clear(); makeTrapVector(traps, 3, 57,-1,-1);addMapping(traps, makeCallSet(0), "setpgid");
        traps.clear(); makeTrapVector(traps, 3, 62,-1,-1);addMapping(traps, makeCallSet(0), "ustat");
        traps.clear(); makeTrapVector(traps, 3, 63,-1,-1);addMapping(traps, makeCallSet(0), "__dup2");
        traps.clear(); makeTrapVector(traps, 3, 70,-1,-1);makeTrapVector(traps, 1, 203);addMapping(traps, makeCallSet(0), "setreuid");
        traps.clear(); makeTrapVector(traps, 3, 71,-1,-1);makeTrapVector(traps, 1, 204);addMapping(traps, makeCallSet(0), "setregid");
        traps.clear(); makeTrapVector(traps, 3, 74,-1,-1);addMapping(traps, makeCallSet(0), "sethostname");
        traps.clear(); makeTrapVector(traps, 3, 75,-1,-1);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__setrlimit");
        traps.clear(); makeTrapVector(traps, 3, 77,-1,-1);addMapping(traps, makeCallSet(0), "__getrusage");
        traps.clear(); makeTrapVector(traps, 3, 78,-1,-1);addMapping(traps, makeCallSet(0), "gettimeofday");
        traps.clear(); makeTrapVector(traps, 3, 79,-1,-1);addMapping(traps, makeCallSet(0), "settimeofday");
        traps.clear(); makeTrapVector(traps, 3, 8,-1,-1);addMapping(traps, makeCallSet(0), "__libc_creat");
        traps.clear(); makeTrapVector(traps, 3, 80,-1,-1);makeTrapVector(traps, 1, 205);addMapping(traps, makeCallSet(0), "getgroups");
        traps.clear(); makeTrapVector(traps, 3, 81,-1,-1);makeTrapVector(traps, 1, 206);addMapping(traps, makeCallSet(0), "setgroups");
        traps.clear(); makeTrapVector(traps, 3, 83,-1,-1);addMapping(traps, makeCallSet(0), "__symlink");
        traps.clear(); makeTrapVector(traps, 3, 83,-1,-1);makeTrapVector(traps, 4, 304,-1,-1,-1);addMapping(traps, makeCallSet(0), "symlinkat");
        traps.clear(); makeTrapVector(traps, 3, 87,-1,-1);addMapping(traps, makeCallSet(0), "__swapon");
        traps.clear(); makeTrapVector(traps, 3, 9,-1,-1);addMapping(traps, makeCallSet(0), "__link");
        traps.clear(); makeTrapVector(traps, 3, 9,-1,-1);makeTrapVector(traps, 6, 303,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "linkat");
        traps.clear(); makeTrapVector(traps, 3, 91,-1,-1);addMapping(traps, makeCallSet(0), "munmap");
        traps.clear(); makeTrapVector(traps, 3, 92,-1,-1);addMapping(traps, makeCallSet(0), "truncate");
        traps.clear(); makeTrapVector(traps, 3, 93,-1,-1);addMapping(traps, makeCallSet(0), "ftruncate");
        traps.clear(); makeTrapVector(traps, 3, 94,-1,-1);addMapping(traps, makeCallSet(0), "__fchmod");
        traps.clear(); makeTrapVector(traps, 3, 96,-1,-1);addMapping(traps, makeCallSet(0), "getpriority");
        traps.clear(); makeTrapVector(traps, 3, 99,-1,-1);addMapping(traps, makeCallSet(0), "__statfs");
        traps.clear(); makeTrapVector(traps, 4, 101,-1,-1,-1);addMapping(traps, makeCallSet(0), "ioperm");
        traps.clear(); makeTrapVector(traps, 4, 102,1,-1,-1);addMapping(traps, makeCallSet(0), "socket");
        traps.clear(); makeTrapVector(traps, 4, 102,10,-1,-1);addMapping(traps, makeCallSet(0), "__libc_recv");
        traps.clear(); makeTrapVector(traps, 4, 102,11,-1,-1);addMapping(traps, makeCallSet(0), "__sendto");
        traps.clear(); makeTrapVector(traps, 4, 102,12,-1,-1);addMapping(traps, makeCallSet(0), "__libc_recvfrom");
        traps.clear(); makeTrapVector(traps, 4, 102,13,-1,-1);addMapping(traps, makeCallSet(0), "shutdown");
        traps.clear(); makeTrapVector(traps, 4, 102,14,-1,-1);addMapping(traps, makeCallSet(0), "setsockopt");
        traps.clear(); makeTrapVector(traps, 4, 102,15,-1,-1);addMapping(traps, makeCallSet(0), "__getsockopt");
        traps.clear(); makeTrapVector(traps, 4, 102,16,-1,-1);addMapping(traps, makeCallSet(0), "__sendmsg");
        traps.clear(); makeTrapVector(traps, 4, 102,17,-1,-1);addMapping(traps, makeCallSet(0), "recvmsg");
        traps.clear(); makeTrapVector(traps, 4, 102,2,-1,-1);addMapping(traps, makeCallSet(0), "__bind");
        traps.clear(); makeTrapVector(traps, 4, 102,3,-1,-1);addMapping(traps, makeCallSet(0), "connect");
        traps.clear(); makeTrapVector(traps, 4, 102,4,-1,-1);addMapping(traps, makeCallSet(0), "listen");
        traps.clear(); makeTrapVector(traps, 4, 102,5,-1,-1);addMapping(traps, makeCallSet(0), "__libc_accept");
        traps.clear(); makeTrapVector(traps, 4, 102,6,-1,-1);addMapping(traps, makeCallSet(0), "getsockname");
        traps.clear(); makeTrapVector(traps, 4, 102,7,-1,-1);addMapping(traps, makeCallSet(0), "getpeername");
        traps.clear(); makeTrapVector(traps, 4, 102,8,-1,-1);addMapping(traps, makeCallSet(0), "__socketpair");
        traps.clear(); makeTrapVector(traps, 4, 102,9,-1,-1);addMapping(traps, makeCallSet(0), "__libc_send");
        traps.clear(); makeTrapVector(traps, 4, 103,-1,-1,-1);addMapping(traps, makeCallSet(0), "klogctl");
        traps.clear(); makeTrapVector(traps, 4, 104,-1,-1,-1);addMapping(traps, makeCallSet(0), "__setitimer");
        traps.clear(); makeTrapVector(traps, 4, 125,-1,-1,-1);addMapping(traps, makeCallSet(0), "mprotect");
        traps.clear(); makeTrapVector(traps, 4, 126,-1,-1,-1);makeTrapVector(traps, 5, 175,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__sigprocmask");
        traps.clear(); makeTrapVector(traps, 4, 126,0,-1,-1);addMapping(traps, makeCallSet(0), "getcontext");
        traps.clear(); makeTrapVector(traps, 4, 128,-1,-1,-1);addMapping(traps, makeCallSet(0), "init_module");
        traps.clear(); makeTrapVector(traps, 4, 14,-1,-1,-1);addMapping(traps, makeCallSet(0), "__xmknod");
        traps.clear(); makeTrapVector(traps, 4, 14,-1,-1,-1);makeTrapVector(traps, 5, 297,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__xmknodat");
        traps.clear(); makeTrapVector(traps, 4, 144,-1,-1,-1);addMapping(traps, makeCallSet(0), "msync");
        traps.clear(); makeTrapVector(traps, 4, 145,-1,-1,-1);addMapping(traps, makeCallSet(0), "do_readv");
        traps.clear(); makeTrapVector(traps, 4, 146,2,-1,-1);addMapping(traps, makeCallSet(0), "herror");
        traps.clear(); makeTrapVector(traps, 4, 156,-1,-1,-1);addMapping(traps, makeCallSet(0), "sched_setscheduler");
        traps.clear(); makeTrapVector(traps, 4, 16,-1,-1,-1);makeTrapVector(traps, 1, 198);addMapping(traps, makeCallSet(0), "__lchown");
        traps.clear(); makeTrapVector(traps, 4, 164,-1,-1,-1);makeTrapVector(traps, 1, 208);addMapping(traps, makeCallSet(0), "__setresuid");
        traps.clear(); makeTrapVector(traps, 4, 165,-1,-1,-1);makeTrapVector(traps, 1, 209);addMapping(traps, makeCallSet(0), "getresuid");
        traps.clear(); makeTrapVector(traps, 4, 168,-1,-1,-1);addMapping(traps, makeCallSet(0), "loser_poll");
        traps.clear(); makeTrapVector(traps, 4, 169,-1,-1,-1);addMapping(traps, makeCallSet(0), "nfsservctl");
        traps.clear(); makeTrapVector(traps, 4, 170,-1,-1,-1);makeTrapVector(traps, 1, 210);addMapping(traps, makeCallSet(0), "__setresgid");
        traps.clear(); makeTrapVector(traps, 4, 171,-1,-1,-1);makeTrapVector(traps, 1, 211);addMapping(traps, makeCallSet(0), "getresgid");
        traps.clear(); makeTrapVector(traps, 4, 182,-1,-1,-1);makeTrapVector(traps, 1, 212);addMapping(traps, makeCallSet(0), "__chown");
        traps.clear(); makeTrapVector(traps, 4, 19,-1,-1,-1);addMapping(traps, makeCallSet(0), "__libc_lseek");
        traps.clear(); makeTrapVector(traps, 4, 218,-1,-1,-1);addMapping(traps, makeCallSet(0), "mincore");
        traps.clear(); makeTrapVector(traps, 4, 221,-1,-1,-1);makeTrapVector(traps, 4, 221,-1,12,-1);addMapping(traps, makeCallSet(0), "lockf64");
        traps.clear(); makeTrapVector(traps, 4, 225,-1,-1,-1);addMapping(traps, makeCallSet(0), "__readahead");
        traps.clear(); makeTrapVector(traps, 4, 232,-1,-1,-1);addMapping(traps, makeCallSet(0), "listxattr");
        traps.clear(); makeTrapVector(traps, 4, 233,-1,-1,-1);addMapping(traps, makeCallSet(0), "llistxattr");
        traps.clear(); makeTrapVector(traps, 4, 234,-1,-1,-1);addMapping(traps, makeCallSet(0), "flistxattr");
        traps.clear(); makeTrapVector(traps, 4, 241,-1,-1,-1);makeTrapVector(traps, 4, 242,-1,-1,-1);addMapping(traps, makeCallSet(0), "sched_setaffinity");
        traps.clear(); makeTrapVector(traps, 4, 241,-1,-1,-1);makeTrapVector(traps, 4, 242,-1,-1,-1);addMapping(traps, makeCallSet(0), "sched_setaffinity"); traps.clear(); makeTrapVector(traps, 4, 242,-1,-1,-1);addMapping(traps, makeCallSet(0), "__sched_getaffinity_new");
        traps.clear(); makeTrapVector(traps, 4, 268,-1,84,-1);addMapping(traps, makeCallSet(0), "statfs64");
        traps.clear(); makeTrapVector(traps, 4, 269,-1,84,-1);addMapping(traps, makeCallSet(0), "__fstatfs64");
        traps.clear(); makeTrapVector(traps, 4, 292,-1,-1,-1);addMapping(traps, makeCallSet(0), "inotify_add_watch");
        traps.clear(); makeTrapVector(traps, 4, 4,-1,-1,-1);addMapping(traps, makeCallSet(0), "__libc_write");
        traps.clear(); makeTrapVector(traps, 4, 54,-1,21505,-1);addMapping(traps, makeCallSet(0), "__tcgetattr");
        traps.clear(); makeTrapVector(traps, 4, 54,-1,21513,1);addMapping(traps, makeCallSet(0), "tcdrain");
        traps.clear(); makeTrapVector(traps, 4, 55,-1,-1,-1);makeTrapVector(traps, 4, 55,-1,5,-1);makeTrapVector(traps, 4, 221,-1,-1,-1);addMapping(traps, makeCallSet(0), "__fcntl_nocancel");
        traps.clear(); makeTrapVector(traps, 4, 7,-1,-1,-1);addMapping(traps, makeCallSet(0), "__waitpid");
        traps.clear(); makeTrapVector(traps, 4, 85,-1,-1,-1);makeTrapVector(traps, 5, 305,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "readlinkat");
        traps.clear(); makeTrapVector(traps, 4, 85,135522375,-1,4096);addMapping(traps, makeCallSet(0), "_dl_get_origin");
        traps.clear(); makeTrapVector(traps, 4, 95,-1,-1,-1);makeTrapVector(traps, 1, 207);addMapping(traps, makeCallSet(0), "__fchown");
        traps.clear(); makeTrapVector(traps, 4, 97,-1,-1,-1);addMapping(traps, makeCallSet(0), "setpriority");
        traps.clear(); makeTrapVector(traps, 5, 131,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "quotactl");
        traps.clear(); makeTrapVector(traps, 5, 187,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "sendfile");
        traps.clear(); makeTrapVector(traps, 5, 229,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "getxattr");
        traps.clear(); makeTrapVector(traps, 5, 230,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "lgetxattr");
        traps.clear(); makeTrapVector(traps, 5, 231,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "fgetxattr");
        traps.clear(); makeTrapVector(traps, 5, 239,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "sendfile64");
        traps.clear(); makeTrapVector(traps, 5, 250,-1,-1,-1,-1);makeTrapVector(traps, 5, 272,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "posix_fadvise64@@GLIBC_2.3.3");
        traps.clear(); makeTrapVector(traps, 5, 255,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "epoll_ctl");
        traps.clear(); makeTrapVector(traps, 5, 256,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "epoll_wait");
        traps.clear(); makeTrapVector(traps, 5, 26,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "ptrace");
        traps.clear(); makeTrapVector(traps, 5, 315,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "tee");
        traps.clear(); makeTrapVector(traps, 5, 316,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "vmsplice");
        traps.clear(); makeTrapVector(traps, 5, 88,-18751827,672274793,-1,-1);addMapping(traps, makeCallSet(0), "reboot");
        traps.clear(); makeTrapVector(traps, 6, 163,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__mremap");
        traps.clear(); makeTrapVector(traps, 6, 172,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__prctl");
        traps.clear(); makeTrapVector(traps, 6, 21,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__mount");
        traps.clear(); makeTrapVector(traps, 6, 226,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "setxattr");
        traps.clear(); makeTrapVector(traps, 6, 227,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "lsetxattr");
        traps.clear(); makeTrapVector(traps, 6, 228,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "fsetxattr");
        traps.clear(); makeTrapVector(traps, 6, 257,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__remap_file_pages");
        traps.clear(); makeTrapVector(traps, 6, 284,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "do_waitid");
        traps.clear(); makeTrapVector(traps, 6, 298,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "fchownat");
        traps.clear(); makeTrapVector(traps, 6, 309,-1,-1,-1,8,-1);addMapping(traps, makeCallSet(0), "ppoll");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "_L_lock_80");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,-1,2,-1,-1,-1);addMapping(traps, makeCallSet(0), "__lll_mutex_lock_wait");
        traps.clear(); makeTrapVector(traps, 7, 240,-1,1,1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__lll_mutex_unlock_wake");
        traps.clear(); makeTrapVector(traps, 7, 308,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "__call_pselect6");
        traps.clear(); makeTrapVector(traps, 7, 313,-1,-1,-1,-1,-1,-1);addMapping(traps, makeCallSet(0), "splice");
    } else {

traps.clear(); makeTrapVector(traps, 1, -1);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__libc_fork");
traps.clear(); makeTrapVector(traps, 1, 1);addMapping(traps, makeCallSet(0), "__exit_thread");
traps.clear(); makeTrapVector(traps, 1, 1);makeTrapVector(traps, 1, 20);makeTrapVector(traps, 1, 120);addMapping(traps, makeCallSet(0), "__clone");
traps.clear(); makeTrapVector(traps, 1, 1);makeTrapVector(traps, 1, 252);addMapping(traps, makeCallSet(0), "_exit");
traps.clear(); makeTrapVector(traps, 1, 10);addMapping(traps, makeCallSet(0), "__unlink");
traps.clear(); makeTrapVector(traps, 1, 10);makeTrapVector(traps, 1, 40);makeTrapVector(traps, 1, 301);addMapping(traps, makeCallSet(0), "unlinkat");
traps.clear(); makeTrapVector(traps, 1, 100);addMapping(traps, makeCallSet(0), "fstatfs");
traps.clear(); makeTrapVector(traps, 1, 101);addMapping(traps, makeCallSet(0), "ioperm");
traps.clear(); makeTrapVector(traps, 1, 103);addMapping(traps, makeCallSet(0), "klogctl");
traps.clear(); makeTrapVector(traps, 1, 104);addMapping(traps, makeCallSet(0), "__setitimer");
traps.clear(); makeTrapVector(traps, 1, 105);addMapping(traps, makeCallSet(0), "__getitimer");
traps.clear(); makeTrapVector(traps, 1, 11);addMapping(traps, makeCallSet(0), "execve");
traps.clear(); makeTrapVector(traps, 1, 110);addMapping(traps, makeCallSet(0), "iopl");
traps.clear(); makeTrapVector(traps, 1, 111);addMapping(traps, makeCallSet(0), "vhangup");
traps.clear(); makeTrapVector(traps, 1, 115);addMapping(traps, makeCallSet(0), "__swapoff");
traps.clear(); makeTrapVector(traps, 1, 116);addMapping(traps, makeCallSet(0), "sysinfo");
traps.clear(); makeTrapVector(traps, 1, 118);addMapping(traps, makeCallSet(0), "__libc_fsync");
traps.clear(); makeTrapVector(traps, 1, 12);addMapping(traps, makeCallSet(0), "__chdir");
traps.clear(); makeTrapVector(traps, 1, 121);addMapping(traps, makeCallSet(0), "setdomainname");
traps.clear(); makeTrapVector(traps, 1, 122);addMapping(traps, makeCallSet(0), "__uname");
traps.clear(); makeTrapVector(traps, 1, 123);addMapping(traps, makeCallSet(0), "__modify_ldt");
traps.clear(); makeTrapVector(traps, 1, 124);addMapping(traps, makeCallSet(0), "adjtimex");
traps.clear(); makeTrapVector(traps, 1, 125);addMapping(traps, makeCallSet(0), "mprotect");
traps.clear(); makeTrapVector(traps, 1, 126);makeTrapVector(traps, 1, 175);addMapping(traps, makeCallSet(0), "__sigprocmask");
traps.clear(); makeTrapVector(traps, 1, 127);addMapping(traps, makeCallSet(0), "create_module");
traps.clear(); makeTrapVector(traps, 1, 128);addMapping(traps, makeCallSet(0), "init_module");
traps.clear(); makeTrapVector(traps, 1, 129);addMapping(traps, makeCallSet(0), "delete_module");
traps.clear(); makeTrapVector(traps, 1, 13);addMapping(traps, makeCallSet(0), "time");
traps.clear(); makeTrapVector(traps, 1, 130);addMapping(traps, makeCallSet(0), "get_kernel_syms");
traps.clear(); makeTrapVector(traps, 1, 131);addMapping(traps, makeCallSet(0), "quotactl");
traps.clear(); makeTrapVector(traps, 1, 132);addMapping(traps, makeCallSet(0), "__getpgid");
traps.clear(); makeTrapVector(traps, 1, 133);addMapping(traps, makeCallSet(0), "__fchdir");
traps.clear(); makeTrapVector(traps, 1, 134);addMapping(traps, makeCallSet(0), "bdflush");
traps.clear(); makeTrapVector(traps, 1, 136);addMapping(traps, makeCallSet(0), "__personality");
traps.clear(); makeTrapVector(traps, 1, 138);makeTrapVector(traps, 1, 215);addMapping(traps, makeCallSet(0), "setfsuid");
traps.clear(); makeTrapVector(traps, 1, 139);makeTrapVector(traps, 1, 216);addMapping(traps, makeCallSet(0), "setfsgid");
traps.clear(); makeTrapVector(traps, 1, 14);addMapping(traps, makeCallSet(0), "__xmknod");
traps.clear(); makeTrapVector(traps, 1, 14);makeTrapVector(traps, 1, 297);addMapping(traps, makeCallSet(0), "__xmknodat");
traps.clear(); makeTrapVector(traps, 1, 140);addMapping(traps, makeCallSet(0), "__llseek");
traps.clear(); makeTrapVector(traps, 1, 142);addMapping(traps, makeCallSet(0), "__select");
traps.clear(); makeTrapVector(traps, 1, 143);addMapping(traps, makeCallSet(0), "__flock");
traps.clear(); makeTrapVector(traps, 1, 144);addMapping(traps, makeCallSet(0), "msync");
traps.clear(); makeTrapVector(traps, 1, 145);addMapping(traps, makeCallSet(0), "do_readv");
traps.clear(); makeTrapVector(traps, 1, 147);addMapping(traps, makeCallSet(0), "getsid");
traps.clear(); makeTrapVector(traps, 1, 148);addMapping(traps, makeCallSet(0), "fdatasync");
traps.clear(); makeTrapVector(traps, 1, 149);addMapping(traps, makeCallSet(0), "__sysctl");
traps.clear(); makeTrapVector(traps, 1, 15);addMapping(traps, makeCallSet(0), "__chmod");
traps.clear(); makeTrapVector(traps, 1, 15);makeTrapVector(traps, 1, 306);addMapping(traps, makeCallSet(0), "fchmodat");
traps.clear(); makeTrapVector(traps, 1, 150);addMapping(traps, makeCallSet(0), "mlock");
traps.clear(); makeTrapVector(traps, 1, 151);addMapping(traps, makeCallSet(0), "munlock");
traps.clear(); makeTrapVector(traps, 1, 152);addMapping(traps, makeCallSet(0), "mlockall");
traps.clear(); makeTrapVector(traps, 1, 153);addMapping(traps, makeCallSet(0), "munlockall");
traps.clear(); makeTrapVector(traps, 1, 154);addMapping(traps, makeCallSet(0), "sched_setparam");
traps.clear(); makeTrapVector(traps, 1, 155);addMapping(traps, makeCallSet(0), "sched_getparam");
traps.clear(); makeTrapVector(traps, 1, 156);addMapping(traps, makeCallSet(0), "sched_setscheduler");
traps.clear(); makeTrapVector(traps, 1, 157);addMapping(traps, makeCallSet(0), "sched_getscheduler");
traps.clear(); makeTrapVector(traps, 1, 158);addMapping(traps, makeCallSet(0), "__sched_yield");
traps.clear(); makeTrapVector(traps, 1, 159);addMapping(traps, makeCallSet(0), "sched_get_priority_max");
traps.clear(); makeTrapVector(traps, 1, 16);makeTrapVector(traps, 1, 198);addMapping(traps, makeCallSet(0), "__lchown");
traps.clear(); makeTrapVector(traps, 1, 160);addMapping(traps, makeCallSet(0), "sched_get_priority_min");
traps.clear(); makeTrapVector(traps, 1, 161);addMapping(traps, makeCallSet(0), "__sched_rr_get_interval");
traps.clear(); makeTrapVector(traps, 1, 162);addMapping(traps, makeCallSet(0), "__nanosleep");
traps.clear(); makeTrapVector(traps, 1, 163);addMapping(traps, makeCallSet(0), "__mremap");
traps.clear(); makeTrapVector(traps, 1, 164);makeTrapVector(traps, 1, 208);addMapping(traps, makeCallSet(0), "__setresuid");
traps.clear(); makeTrapVector(traps, 1, 165);makeTrapVector(traps, 1, 209);addMapping(traps, makeCallSet(0), "getresuid");
traps.clear(); makeTrapVector(traps, 1, 167);addMapping(traps, makeCallSet(0), "query_module");
traps.clear(); makeTrapVector(traps, 1, 168);addMapping(traps, makeCallSet(0), "loser_poll");
traps.clear(); makeTrapVector(traps, 1, 169);addMapping(traps, makeCallSet(0), "nfsservctl");
traps.clear(); makeTrapVector(traps, 1, 170);makeTrapVector(traps, 1, 210);addMapping(traps, makeCallSet(0), "__setresgid");
traps.clear(); makeTrapVector(traps, 1, 171);makeTrapVector(traps, 1, 211);addMapping(traps, makeCallSet(0), "getresgid");
traps.clear(); makeTrapVector(traps, 1, 172);addMapping(traps, makeCallSet(0), "__prctl");
traps.clear(); makeTrapVector(traps, 1, 182);makeTrapVector(traps, 1, 212);addMapping(traps, makeCallSet(0), "__chown");
traps.clear(); makeTrapVector(traps, 1, 183);addMapping(traps, makeCallSet(0), "__getcwd");
traps.clear(); makeTrapVector(traps, 1, 184);addMapping(traps, makeCallSet(0), "capget");
traps.clear(); makeTrapVector(traps, 1, 185);addMapping(traps, makeCallSet(0), "capset");
traps.clear(); makeTrapVector(traps, 1, 186);addMapping(traps, makeCallSet(0), "sigaltstack");
traps.clear(); makeTrapVector(traps, 1, 187);addMapping(traps, makeCallSet(0), "sendfile");
traps.clear(); makeTrapVector(traps, 1, 19);addMapping(traps, makeCallSet(0), "__libc_lseek");
traps.clear(); makeTrapVector(traps, 1, 193);addMapping(traps, makeCallSet(0), "truncate64");
traps.clear(); makeTrapVector(traps, 1, 194);addMapping(traps, makeCallSet(0), "__ftruncate64");
traps.clear(); makeTrapVector(traps, 1, 2);makeTrapVector(traps, 1, 190);addMapping(traps, makeCallSet(0), "__vfork");
traps.clear(); makeTrapVector(traps, 1, 20);addMapping(traps, makeCallSet(0), "__getpid");
traps.clear(); makeTrapVector(traps, 1, 21);addMapping(traps, makeCallSet(0), "__mount");
traps.clear(); makeTrapVector(traps, 1, 217);addMapping(traps, makeCallSet(0), "pivot_root");
traps.clear(); makeTrapVector(traps, 1, 218);addMapping(traps, makeCallSet(0), "mincore");
traps.clear(); makeTrapVector(traps, 1, 22);addMapping(traps, makeCallSet(0), "__umount");
traps.clear(); makeTrapVector(traps, 1, 221);addMapping(traps, makeCallSet(0), "lockf64"); traps.clear(); makeTrapVector(traps, 1, 55);makeTrapVector(traps, 1, 221);addMapping(traps, makeCallSet(0), "__fcntl_nocancel");
traps.clear(); makeTrapVector(traps, 1, 224);makeTrapVector(traps, 1, 238);makeTrapVector(traps, 1, 270);addMapping(traps, makeCallSet(0), "raise");
traps.clear(); makeTrapVector(traps, 1, 225);addMapping(traps, makeCallSet(0), "__readahead");
traps.clear(); makeTrapVector(traps, 1, 226);addMapping(traps, makeCallSet(0), "setxattr");
traps.clear(); makeTrapVector(traps, 1, 227);addMapping(traps, makeCallSet(0), "lsetxattr");
traps.clear(); makeTrapVector(traps, 1, 228);addMapping(traps, makeCallSet(0), "fsetxattr");
traps.clear(); makeTrapVector(traps, 1, 229);addMapping(traps, makeCallSet(0), "getxattr");
traps.clear(); makeTrapVector(traps, 1, 23);makeTrapVector(traps, 1, 213);addMapping(traps, makeCallSet(0), "__setuid");
traps.clear(); makeTrapVector(traps, 1, 230);addMapping(traps, makeCallSet(0), "lgetxattr");
traps.clear(); makeTrapVector(traps, 1, 231);addMapping(traps, makeCallSet(0), "fgetxattr");
traps.clear(); makeTrapVector(traps, 1, 232);addMapping(traps, makeCallSet(0), "listxattr");
traps.clear(); makeTrapVector(traps, 1, 233);addMapping(traps, makeCallSet(0), "llistxattr");
traps.clear(); makeTrapVector(traps, 1, 234);addMapping(traps, makeCallSet(0), "flistxattr");
traps.clear(); makeTrapVector(traps, 1, 235);addMapping(traps, makeCallSet(0), "removexattr");
traps.clear(); makeTrapVector(traps, 1, 236);addMapping(traps, makeCallSet(0), "lremovexattr");
traps.clear(); makeTrapVector(traps, 1, 237);addMapping(traps, makeCallSet(0), "fremovexattr");
traps.clear(); makeTrapVector(traps, 1, 239);addMapping(traps, makeCallSet(0), "sendfile64");
traps.clear(); makeTrapVector(traps, 1, 24);makeTrapVector(traps, 1, 199);addMapping(traps, makeCallSet(0), "__getuid");
traps.clear(); makeTrapVector(traps, 1, 241);makeTrapVector(traps, 1, 242);addMapping(traps, makeCallSet(0), "sched_setaffinity");
traps.clear(); makeTrapVector(traps, 1, 241);makeTrapVector(traps, 1, 242);addMapping(traps, makeCallSet(0), "sched_setaffinity"); traps.clear(); makeTrapVector(traps, 1, 242);addMapping(traps, makeCallSet(0), "__sched_getaffinity_new");
traps.clear(); makeTrapVector(traps, 1, 25);addMapping(traps, makeCallSet(0), "stime");
traps.clear(); makeTrapVector(traps, 1, 250);makeTrapVector(traps, 1, 272);addMapping(traps, makeCallSet(0), "posix_fadvise64@@GLIBC_2.3.3");
traps.clear(); makeTrapVector(traps, 1, 254);addMapping(traps, makeCallSet(0), "epoll_create");
traps.clear(); makeTrapVector(traps, 1, 255);addMapping(traps, makeCallSet(0), "epoll_ctl");
traps.clear(); makeTrapVector(traps, 1, 256);addMapping(traps, makeCallSet(0), "epoll_wait");
traps.clear(); makeTrapVector(traps, 1, 257);addMapping(traps, makeCallSet(0), "__remap_file_pages");
traps.clear(); makeTrapVector(traps, 1, 26);addMapping(traps, makeCallSet(0), "ptrace");
traps.clear(); makeTrapVector(traps, 1, 268);addMapping(traps, makeCallSet(0), "statfs64");
traps.clear(); makeTrapVector(traps, 1, 269);addMapping(traps, makeCallSet(0), "__fstatfs64");
traps.clear(); makeTrapVector(traps, 1, 27);addMapping(traps, makeCallSet(0), "alarm");
traps.clear(); makeTrapVector(traps, 1, 284);addMapping(traps, makeCallSet(0), "do_waitid");
traps.clear(); makeTrapVector(traps, 1, 29);addMapping(traps, makeCallSet(0), "__sleep");
traps.clear(); makeTrapVector(traps, 1, 291);addMapping(traps, makeCallSet(0), "inotify_init");
traps.clear(); makeTrapVector(traps, 1, 292);addMapping(traps, makeCallSet(0), "inotify_add_watch");
traps.clear(); makeTrapVector(traps, 1, 293);addMapping(traps, makeCallSet(0), "inotify_rm_watch");
traps.clear(); makeTrapVector(traps, 1, 298);addMapping(traps, makeCallSet(0), "fchownat");
traps.clear(); makeTrapVector(traps, 1, 30);addMapping(traps, makeCallSet(0), "utime");
traps.clear(); makeTrapVector(traps, 1, 30);makeTrapVector(traps, 1, 271);addMapping(traps, makeCallSet(0), "__utimes");
traps.clear(); makeTrapVector(traps, 1, 30);makeTrapVector(traps, 1, 271);makeTrapVector(traps, 1, 299);addMapping(traps, makeCallSet(0), "futimesat");
traps.clear(); makeTrapVector(traps, 1, 30);makeTrapVector(traps, 1, 55);makeTrapVector(traps, 1, 271);addMapping(traps, makeCallSet(0), "__futimes");
traps.clear(); makeTrapVector(traps, 1, 308);addMapping(traps, makeCallSet(0), "__call_pselect6");
traps.clear(); makeTrapVector(traps, 1, 309);addMapping(traps, makeCallSet(0), "ppoll");
traps.clear(); makeTrapVector(traps, 1, 310);addMapping(traps, makeCallSet(0), "unshare");
traps.clear(); makeTrapVector(traps, 1, 313);addMapping(traps, makeCallSet(0), "splice");
traps.clear(); makeTrapVector(traps, 1, 315);addMapping(traps, makeCallSet(0), "tee");
traps.clear(); makeTrapVector(traps, 1, 316);addMapping(traps, makeCallSet(0), "vmsplice");
traps.clear(); makeTrapVector(traps, 1, 33);addMapping(traps, makeCallSet(0), "__access");
traps.clear(); makeTrapVector(traps, 1, 33);makeTrapVector(traps, 1, 307);addMapping(traps, makeCallSet(0), "faccessat");
traps.clear(); makeTrapVector(traps, 1, 36);addMapping(traps, makeCallSet(0), "sync");
traps.clear(); makeTrapVector(traps, 1, 38);addMapping(traps, makeCallSet(0), "rename");
traps.clear(); makeTrapVector(traps, 1, 38);makeTrapVector(traps, 1, 302);addMapping(traps, makeCallSet(0), "renameat");
traps.clear(); makeTrapVector(traps, 1, 39);addMapping(traps, makeCallSet(0), "__mkdir");
traps.clear(); makeTrapVector(traps, 1, 39);makeTrapVector(traps, 1, 296);addMapping(traps, makeCallSet(0), "mkdirat");
traps.clear(); makeTrapVector(traps, 1, 4);addMapping(traps, makeCallSet(0), "__libc_write");
traps.clear(); makeTrapVector(traps, 1, 40);addMapping(traps, makeCallSet(0), "__rmdir");
traps.clear(); makeTrapVector(traps, 1, 41);addMapping(traps, makeCallSet(0), "__dup");
traps.clear(); makeTrapVector(traps, 1, 42);addMapping(traps, makeCallSet(0), "__pipe");
traps.clear(); makeTrapVector(traps, 1, 43);addMapping(traps, makeCallSet(0), "__times");
traps.clear(); makeTrapVector(traps, 1, 45);addMapping(traps, makeCallSet(0), "__brk");
traps.clear(); makeTrapVector(traps, 1, 46);makeTrapVector(traps, 1, 214);addMapping(traps, makeCallSet(0), "__setgid");
traps.clear(); makeTrapVector(traps, 1, 47);makeTrapVector(traps, 1, 200);addMapping(traps, makeCallSet(0), "__getgid");
traps.clear(); makeTrapVector(traps, 1, 49);makeTrapVector(traps, 1, 201);addMapping(traps, makeCallSet(0), "__geteuid");
traps.clear(); makeTrapVector(traps, 1, 50);makeTrapVector(traps, 1, 202);addMapping(traps, makeCallSet(0), "__getegid");
traps.clear(); makeTrapVector(traps, 1, 51);addMapping(traps, makeCallSet(0), "acct");
traps.clear(); makeTrapVector(traps, 1, 52);addMapping(traps, makeCallSet(0), "__umount2");
traps.clear(); makeTrapVector(traps, 1, 55);makeTrapVector(traps, 1, 221);addMapping(traps, makeCallSet(0), "__fcntl_nocancel");
traps.clear(); makeTrapVector(traps, 1, 57);addMapping(traps, makeCallSet(0), "setpgid");
traps.clear(); makeTrapVector(traps, 1, 6);makeTrapVector(traps, 1, 266);addMapping(traps, makeCallSet(0), "sysconf");
traps.clear(); makeTrapVector(traps, 1, 60);addMapping(traps, makeCallSet(0), "__umask");
traps.clear(); makeTrapVector(traps, 1, 61);addMapping(traps, makeCallSet(0), "chroot");
traps.clear(); makeTrapVector(traps, 1, 62);addMapping(traps, makeCallSet(0), "ustat");
traps.clear(); makeTrapVector(traps, 1, 63);addMapping(traps, makeCallSet(0), "__dup2");
traps.clear(); makeTrapVector(traps, 1, 64);addMapping(traps, makeCallSet(0), "__getppid");
traps.clear(); makeTrapVector(traps, 1, 65);addMapping(traps, makeCallSet(0), "getpgrp");
traps.clear(); makeTrapVector(traps, 1, 66);addMapping(traps, makeCallSet(0), "__setsid");
traps.clear(); makeTrapVector(traps, 1, 67);makeTrapVector(traps, 1, 174);addMapping(traps, makeCallSet(0), "__libc_sigaction");
traps.clear(); makeTrapVector(traps, 1, 7);addMapping(traps, makeCallSet(0), "__waitpid");
traps.clear(); makeTrapVector(traps, 1, 70);makeTrapVector(traps, 1, 203);addMapping(traps, makeCallSet(0), "setreuid");
traps.clear(); makeTrapVector(traps, 1, 71);makeTrapVector(traps, 1, 204);addMapping(traps, makeCallSet(0), "setregid");
traps.clear(); makeTrapVector(traps, 1, 72);makeTrapVector(traps, 1, 179);addMapping(traps, makeCallSet(0), "do_sigsuspend");
traps.clear(); makeTrapVector(traps, 1, 73);makeTrapVector(traps, 1, 176);addMapping(traps, makeCallSet(0), "sigpending");
traps.clear(); makeTrapVector(traps, 1, 74);addMapping(traps, makeCallSet(0), "sethostname");
traps.clear(); makeTrapVector(traps, 1, 75);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__setrlimit");
traps.clear(); makeTrapVector(traps, 1, 76);makeTrapVector(traps, 1, 191);addMapping(traps, makeCallSet(0), "__getrlimit");
traps.clear(); makeTrapVector(traps, 1, 77);addMapping(traps, makeCallSet(0), "__getrusage");
traps.clear(); makeTrapVector(traps, 1, 78);addMapping(traps, makeCallSet(0), "gettimeofday");
traps.clear(); makeTrapVector(traps, 1, 79);addMapping(traps, makeCallSet(0), "settimeofday");
traps.clear(); makeTrapVector(traps, 1, 8);addMapping(traps, makeCallSet(0), "__libc_creat");
traps.clear(); makeTrapVector(traps, 1, 80);makeTrapVector(traps, 1, 205);addMapping(traps, makeCallSet(0), "getgroups");
traps.clear(); makeTrapVector(traps, 1, 81);makeTrapVector(traps, 1, 206);addMapping(traps, makeCallSet(0), "setgroups");
traps.clear(); makeTrapVector(traps, 1, 83);addMapping(traps, makeCallSet(0), "__symlink");
traps.clear(); makeTrapVector(traps, 1, 83);makeTrapVector(traps, 1, 304);addMapping(traps, makeCallSet(0), "symlinkat");
traps.clear(); makeTrapVector(traps, 1, 85);makeTrapVector(traps, 1, 305);addMapping(traps, makeCallSet(0), "readlinkat");
traps.clear(); makeTrapVector(traps, 1, 86);addMapping(traps, makeCallSet(0), "uselib");
traps.clear(); makeTrapVector(traps, 1, 87);addMapping(traps, makeCallSet(0), "__swapon");
traps.clear(); makeTrapVector(traps, 1, 88);addMapping(traps, makeCallSet(0), "reboot");
traps.clear(); makeTrapVector(traps, 1, 9);addMapping(traps, makeCallSet(0), "__link");
traps.clear(); makeTrapVector(traps, 1, 9);makeTrapVector(traps, 1, 303);addMapping(traps, makeCallSet(0), "linkat");
traps.clear(); makeTrapVector(traps, 1, 90);addMapping(traps, makeCallSet(0), "__mmap");
traps.clear(); makeTrapVector(traps, 1, 90);makeTrapVector(traps, 1, 192);addMapping(traps, makeCallSet(0), "__mmap64");
traps.clear(); makeTrapVector(traps, 1, 91);addMapping(traps, makeCallSet(0), "munmap");
traps.clear(); makeTrapVector(traps, 1, 92);addMapping(traps, makeCallSet(0), "truncate");
traps.clear(); makeTrapVector(traps, 1, 93);addMapping(traps, makeCallSet(0), "ftruncate");
traps.clear(); makeTrapVector(traps, 1, 94);addMapping(traps, makeCallSet(0), "__fchmod");
traps.clear(); makeTrapVector(traps, 1, 95);makeTrapVector(traps, 1, 207);addMapping(traps, makeCallSet(0), "__fchown");
traps.clear(); makeTrapVector(traps, 1, 96);addMapping(traps, makeCallSet(0), "getpriority");
traps.clear(); makeTrapVector(traps, 1, 97);addMapping(traps, makeCallSet(0), "setpriority");
traps.clear(); makeTrapVector(traps, 1, 99);addMapping(traps, makeCallSet(0), "__statfs");
    }

    return true;    
}



