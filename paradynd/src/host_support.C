/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * $Id: host_support.C,v 1.14 2000/07/28 17:22:11 pcroth Exp $
 *    code that enables paradyndPVM to function as the hoster process
 *    in the PVM environment, which means that it will be responsible for 
 *    starting new PVM daemons.
 */

/*
 * Most of the code in this file was copied from the pvm distribution and
 * slightly modified.
 */

/*
 *         PVM version 3.3:  Parallel Virtual Machine System
 *               University of Tennessee, Knoxville TN.
 *           Oak Ridge National Laboratory, Oak Ridge TN.
 *                   Emory University, Atlanta GA.
 *      Authors:  A. L. Beguelin, J. J. Dongarra, G. A. Geist,
 *    W. C. Jiang, R. J. Manchek, B. K. Moore, and V. S. Sunderam
 *                   (C) 1992 All Rights Reserved
 *
 *                              NOTICE
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted
 * provided that the above copyright notice appear in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * Neither the Institutions (Emory University, Oak Ridge National
 * Laboratory, and University of Tennessee) nor the Authors make any
 * representations about the suitability of this software for any
 * purpose.  This software is provided ``as is'' without express or
 * implied warranty.
 *
 * PVM version 3 was funded in part by the U.S. Department of Energy,
 * the National Science Foundation and the State of Tennessee.
 */

#include "common/h/headers.h"
#include <pwd.h>
#include <pvm3.h>
#include <pvmsdpro.h>
#include <ctype.h>

extern "C" {
// conflicts with an earlier declaration in g++2.7.0, even though this is the
// correct prototype (sigh)
//  extern int gettimeofday (struct timeval *TP, struct timezone *TZP);

// temp hack; stuff like this should probably move to common/h/{PLATFORM}headers.h
// anyway.
#if defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(mips_sgi_irix6_4)
  extern void endservent(void);
  extern void endpwent(); 
#elif !defined(rs6000_ibm_aix4_1)
  extern int endservent(void);
  extern void endpwent(); 
#endif

// This function have moved to common/h/{PLATFORM}headers.h
// extern int endservent();

extern int getdtablesize();
}


extern void PDYN_goodbye(const char*);
extern char *PDYN_daemon();
extern char **PDYN_daemon_args();

/* if > 1, uses parallel startup strategy */
#ifndef	RSHNPLL
#define	RSHNPLL	5
#endif

#define	CINDEX(s,c)	strchr(s,c)

#ifndef	RSHCOMMAND
#define	RSHCOMMAND	"rsh"
#endif

#ifndef	RSHTIMEOUT
#define	RSHTIMEOUT	60
#endif

struct slot {
  struct slot *s_link, *s_rlink;	/* free/active list */
  struct hst *s_hst;		/* host table entry */
  struct timeval s_bail;		/* timeout time */
  int s_rfd, s_wfd, s_efd;	/* slave stdin/out/err */
  char s_buf[256];		/* config reply line */
  int s_len;			/* length of s_buf */
};

#define	LISTPUTAFTER(o,n,f,r)	{ (n)->f=(o)->f; (n)->r=o; (o)->f->r=n; (o)->f=n; }
#define	LISTPUTBEFORE(o,n,f,r)	{ (n)->r=(o)->r; (n)->f=o; (o)->r->f=n; (o)->r=n; }
#define	LISTDELETE(e,f,r)	{ (e)->f->r=(e)->r; (e)->r->f=(e)->f; (e)->r=(e)->f=0; }

#define	TALLOC(n,t,g)	(t*)malloc((n)*sizeof(t))
#define	FREE(p)	free((char *)p)
#define	STRALLOC(s)			strcpy(TALLOC(strlen(s)+1,char,"str"),s)


static struct slot slots[RSHNPLL+2];	/* state var/context for each slot */
static struct slot *slfree;	/* free list of slots */

static int pl_startup(int num, struct hst **hostlist);
static int phase1(struct slot *sp);
static int close_slot(struct slot *sp);

#ifndef	max
#define	max(a,b)	((a)>(b)?(a):(b))
#endif

#ifndef	min
#define	min(a,b)	((a)<(b)?(a):(b))
#endif

#define	TVCLEAR(tvp)	((tvp)->tv_sec = (tvp)->tv_usec = 0)

#define	TVISSET(tvp)	((tvp)->tv_sec || (tvp)->tv_usec)

#define	TVXLTY(xtv, ytv) \
((xtv)->tv_sec < (ytv)->tv_sec || \
 ((xtv)->tv_sec == (ytv)->tv_sec && (xtv)->tv_usec < (ytv)->tv_usec))

#define	TVXADDY(ztv, xtv, ytv)	\
if (((ztv)->tv_usec = (xtv)->tv_usec + (ytv)->tv_usec) < 1000000) {	\
  (ztv)->tv_sec = (xtv)->tv_sec + (ytv)->tv_sec;	\
} else {	\
  (ztv)->tv_usec -= 1000000;	\
  (ztv)->tv_sec = (xtv)->tv_sec + (ytv)->tv_sec + 1;	\
}

#define	TVXSUBY(ztv, xtv, ytv)	\
if ((xtv)->tv_usec >= (ytv)->tv_usec) {	\
    (ztv)->tv_sec = (xtv)->tv_sec - (ytv)->tv_sec;	\
    (ztv)->tv_usec = (xtv)->tv_usec - (ytv)->tv_usec;	\
} else {	\
    (ztv)->tv_sec = (xtv)->tv_sec - (ytv)->tv_sec - 1;	\
    (ztv)->tv_usec = (xtv)->tv_usec + 1000000 - (ytv)->tv_usec;	\
}

struct hst {
  int h_tid;
  char *h_name;
  char *h_login;
  char *h_sopts;
  int h_flag;
  char *h_cmd;
  char *h_result;
};

#define	HST_PASSWORD	1            /* ask for a password */
#define	HST_MANUAL	2	     /* do manual startup */

static int debugmask = 0;
char *username = 0;

bool
PDYN_hostDelete()
{
  int tid;
  // tid is the tid of the exited pvmd
  if (pvm_upkint(&tid, 1, 1) < 0) return false;

  // TODO send message to paradyn informing of paradynd exit
  return true;
}

bool
PDYN_reg_as_hoster()
{
  struct passwd *pe;
  int uid;

  if ((uid = getuid()) == -1) { 
    PDYN_goodbye("Register hoster: can't getuid()\n");
    return false;
  }

  if ( (pe = getpwuid(uid)) != 0 )
    username = STRALLOC(pe->pw_name);
  else {
    PDYN_goodbye("Register hoster: can't getuid()\n");
    return false;
  }

  endpwent();
  pvm_reg_hoster();
  return true;
}

bool PDYN_hoster()
{
  int tid;
  int num;
  int i;
  struct hst **hostlist=0;
  struct hst *hp;
  char *p;
  char sopts[64];
  char lognam[256];
  char cmd[512];
  int fromtid;

  /*
   * unpack the startup message
   */

  pvm_bufinfo(pvm_getrbuf(), (int *)0, (int *)0, &fromtid);
  pvm_unpackf("%d", &num);
  fprintf(stderr, "hoster() %d to start\n", num);

  if (num > 0)
    {
      hostlist = TALLOC(num, struct hst *, "xxx");
      for (i = 0; i < num; i++)
	{
	  hp = TALLOC(1, struct hst, "xxx");
	  hostlist[i] = hp;
	  hp->h_flag = 0;
	  hp->h_result = 0;
	  if (pvm_unpackf("%d %s %s %s", &hp->h_tid, sopts,
			  lognam, cmd)) {
	    PDYN_goodbye("hoster() bad message format\n");
	    return false;
	  }
	  hp->h_sopts = STRALLOC(sopts);
	  hp->h_login = STRALLOC(lognam);
	  hp->h_cmd = STRALLOC(cmd);
	  fprintf(stderr, "%d. t%x %s so=\"%s\"\n", i,
		  hp->h_tid, hp->h_login, hp->h_sopts);

	  if ( (p = CINDEX(hp->h_login, '@')) != 0 )
	    {
	      hp->h_name = STRALLOC(p + 1);
	      *p = 0;
	      p = STRALLOC(hp->h_login);
	      FREE(hp->h_login);
	      hp->h_login = p;
	    }
	  else
	    {
	      hp->h_name = hp->h_login;
	      hp->h_login = 0;
	    }
	  if (!strcmp(hp->h_sopts, "pw"))
	    hp->h_flag |= HST_PASSWORD;
	  if (!strcmp(hp->h_sopts, "ms"))
	    hp->h_flag |= HST_MANUAL;
	}
    }
  
  /*
   * do it
   */
  
#if     RSHNPLL > 1
  pl_startup(num, hostlist);
#else   /*RSHNPLL > 1*/
  for (i = 0; i < num; i++) {
    PDYN_goodbye("not implemented\n");
    return false;
  }
    /* slave_exec(hostlist[i]); */
#endif  /*RSHNPLL > 1*/

  /*
   * send results back to pvmd
   */

  pvm_packf("%+ %d", PvmDataFoo, num);
  for (i = 0; i < num; i++)
    {
      pvm_packf("%d", hostlist[i]->h_tid);
      pvm_packf("%s", hostlist[i]->h_result 
		? hostlist[i]->h_result : "PvmDSysErr");
    }

  /*
     printf("hoster() sending back host table\n");
     */
  pvm_setmwid(pvm_getsbuf(), pvm_getmwid(pvm_getrbuf()));
  pvm_send(fromtid, SM_STHOSTACK);

  /* start paradyndPVM */
  for (i=0; i<num; i++)
    {
      if (pvm_spawn(PDYN_daemon(), PDYN_daemon_args(), 1, hostlist[i]->h_name,
		1, &tid) != 1) {
	PDYN_goodbye("Can't start paradyndPVM on host\n");
	return false;
      }
    }
  
  return true;
}


#if RSHNPLL > 1

/********************************************
 *  this is the new (parallel) startup code  *
 *                                           *
 ********************************************/

int
close_slot(struct slot *sp)
{
  if (sp->s_wfd != -1)
    (void)close(sp->s_wfd);
  if (sp->s_rfd != -1)
    (void)close(sp->s_rfd);
  if (sp->s_efd != -1)
    (void)close(sp->s_efd);
  LISTDELETE(sp, s_link, s_rlink);
  LISTPUTBEFORE(slfree, sp, s_link, s_rlink);
  return 0;
}

int
pl_startup(int num, struct hst **hostlist)
{
  int nxth = 0;		        /* next host in list to start */
  struct slot *slact;           /* active list of slots */
  struct hst *hp;
  struct slot *sp, *sp2;
  struct timeval tnow;
  struct timeval tout;
  fd_set rfds;
  int nfds;
  int i;
  int n;
  char *p;
  char ebuf[256];  	/* for reading stderr */

  /* init slot free list */
  
  slfree = &slots[RSHNPLL+1];
  slfree->s_link = slfree->s_rlink = slfree;
  slact = &slots[RSHNPLL];
  slact->s_link = slact->s_rlink = slact;
  for (i = RSHNPLL; i-- > 0; ) {
    LISTPUTAFTER(slfree, &slots[i], s_link, s_rlink);
  }

  /* keep at this until all hosts in table are completed  */
  for (; ; )
    {
      /* if empty slots, start on new hosts */
      for (; ; ) 
	{
	  /* find a host for slot */
	  if (slfree->s_link != slfree && nxth < num)
	    hp = hostlist[nxth++];
	  else
	    break;

	  sp = slfree->s_link;
	  LISTDELETE(sp, s_link, s_rlink);
	  sp->s_hst = hp;
	  sp->s_len = 0;
	  if (debugmask)
	    fprintf(stderr, "pl_startup() trying %s\n", hp->h_name);

	  phase1(sp);
	  if (hp->h_result)
	    {
	      /* error or fully started (manual startup) */
	      LISTPUTBEFORE(slfree, sp, s_link, s_rlink);
	    }
	  else
	    {
	      /* partially started */

	      LISTPUTBEFORE(slact, sp, s_link, s_rlink);
	      gettimeofday(&sp->s_bail, (struct timezone*)0);
	      tout.tv_sec = RSHTIMEOUT;
	      tout.tv_usec = 0;
	      TVXADDY(&sp->s_bail, &sp->s_bail, &tout);
	    }
	}

      /* if no hosts in progress, we are finished */

      if (slact->s_link == slact)
	break;

      /*
       * until next timeout, get output from any slot
       */

      FD_ZERO(&rfds);
      nfds = 0;
      TVCLEAR(&tout);
      gettimeofday(&tnow, (struct timezone*)0);
      for (sp = slact->s_link; sp != slact; sp = sp->s_link)
	{
	  if (TVXLTY(&sp->s_bail, &tnow))
	    {
	      fprintf(stderr, "pl_startup() %s timed out after %d secs\n",
		      sp->s_hst->h_name, RSHTIMEOUT);
	      sp->s_hst->h_result = STRALLOC("PvmCantStart");
	      sp2 = sp->s_rlink;
	      close_slot(sp);
	      sp = sp2;
	      continue;
	    }

	  if (!TVISSET(&tout) || TVXLTY(&sp->s_bail, &tout))
	    tout = sp->s_bail;
	  if (sp->s_rfd >= 0)
	    FD_SET(sp->s_rfd, &rfds);
	  if (sp->s_rfd > nfds)
	    nfds = sp->s_rfd;
	  if (sp->s_efd >= 0)
	    FD_SET(sp->s_efd, &rfds);
	  if (sp->s_efd > nfds)
	    nfds = sp->s_efd;
	}

      if (slact->s_link == slact)
	break;

      nfds++;

      if (TVXLTY(&tnow, &tout))
	{
	  TVXSUBY(&tout, &tout, &tnow);
	} 
      else
	{
	  TVCLEAR(&tout);
	}
      if (debugmask)
	{
	  fprintf(stderr, "pl_startup() select timeout is %lu.%06lu\n",
		  tout.tv_sec, tout.tv_usec);
	}
      if ((n = P_select(nfds, &rfds, 0, 0, &tout)) == -1)
	{
	  if (errno != EINTR) {
	    PDYN_goodbye("work() select");
	    return false;
	  }
	}
      if (debugmask)
	{
	  (void)fprintf(stderr, "pl_startup() select returns %d\n", n);
	}
      if (n < 1)
	{
	  if (n == -1 && errno != EINTR) {
	    PDYN_goodbye("pl_startup() select");
	    return false;
	  }
	  continue;
	}

      /*
       * check for response on stdout or stderr of any slave.
       */

      for (sp = slact->s_link; sp != slact; sp = sp->s_link) {

	/*
	 * stdout ready.  get complete line then scan config info from it.
	 */
	if (sp->s_rfd >= 0 && FD_ISSET(sp->s_rfd, &rfds))
	  {
	    n = read(sp->s_rfd, sp->s_buf + sp->s_len,
		     sizeof(sp->s_buf) - sp->s_len);
	    if (n > 0)
	      {
		sp->s_len += n;
		if (sp->s_len >= sizeof(sp->s_buf))
		  {
		    fprintf(stderr, "pl_startup() pvmd@%s: big read\n",
			    sp->s_hst->h_name);
		    sp->s_hst->h_result = STRALLOC("PvmCantStart");
		  }
		sp->s_buf[sp->s_len] = 0;
		if ( (p = CINDEX(sp->s_buf + sp->s_len - n, '\n')) != 0 )
		  {
		    if (debugmask)
		      {
			fprintf(stderr, "pvmd@%s: %s",
				sp->s_hst->h_name, sp->s_buf);
		      }
		    *p = 0;
		    sp->s_hst->h_result = STRALLOC(sp->s_buf);
		  }

	      }
	    else
	      {
		if (n)
		  {
		    fprintf(stderr, "pl_startup() pvmd@%s\n",
			    sp->s_hst->h_name);
		    perror("");
		  }
		else
		  {
		    fprintf(stderr, "pl_startup() pvmd@%s: EOF\n",
			    sp->s_hst->h_name);
		  }
		sp->s_hst->h_result = STRALLOC("PvmCantStart");
	      }
	    if (sp->s_hst->h_result)
	      {
		sp2 = sp->s_rlink;
		close_slot(sp);
		sp = sp2;
		continue;
	      }
	  }

	/*
	 * response on stderr.  log prefixed by remote's host name.
	 */
	if (sp->s_efd >= 0 && FD_ISSET(sp->s_efd, &rfds))
	  {
	    if ((n = read(sp->s_efd, ebuf, sizeof(ebuf)-1)) > 0)
	      {
		char *p = ebuf, c;

		ebuf[n] = 0;
		fprintf(stderr, "pvmd@%s: ", sp->s_hst->h_name);
		/*
		   q = pvmtxt + strlen(pvmtxt);
		   */
		while ( (c = *p++ & 0x7f) != 0 )
		  {
		    if (isprint(c))
		      /*
		       *q++ = c;
		       */
		      fputc(c, stderr);

		    else
		      {
			/*
			 *q++ = '^';
			 *q++ = (c + '@') & 0x7f;
			 */
			fputc('^', stderr);
			fputc((c + '@') & 0x7f, stderr);
		      }
		  }
		/*
		 *q++ = '\n';
		 *q = 0;
		 */
		fputc('\n', stderr);

	      }
	    else
	      {
		(void)close(sp->s_efd);
		sp->s_efd = -1;
	      }
	  }
      }
    }
  return 0;
}

int
phase1(struct slot *sp)
{
  struct hst *hp;
  char *hn;
  char *av[16];			/* for rsh args */
  int ac;
  char buf[512];
  int pid = -1;			/* pid of rsh */
  char *p;

#ifndef NOREXEC
  struct servent *se;
  static u_short execport = 0;

  if (!execport)
    {
      if (!(se = P_getservbyname("exec", "tcp")))
	{
	  fprintf(stderr, "phase1() can't getservbyname(): %s\n", "exec"); {
	    PDYN_goodbye("getserverbyname\n");
	    return 0;
	  }
	}
      execport = se->s_port;
      P_endservent();
    }
#endif

  hp = sp->s_hst;
  hn = hp->h_name;
  sp->s_rfd = sp->s_wfd = sp->s_efd = -1;

  /*
   * XXX manual startup hack... this is if we can't use rexec or rsh
   */

  if (hp->h_flag & HST_MANUAL)
    {
      fprintf(stderr, "*** Manual startup ***\n");
      fprintf(stderr, "Login to \"%s\" and type:\n", hn);
      fprintf(stderr, "%s\n", hp->h_cmd);

      /* get version */

      fprintf(stderr, "Type response: ");
      fflush(stderr);
      if (!(fgets(buf, sizeof(buf), stdin)))
	{
	  fprintf(stderr, "host %s read error\n", hn);
	  goto oops;
	}
      p = buf + strlen(buf) - 1;
      if (*p == '\n')
	*p = 0;
      hp->h_result = STRALLOC(buf);
      fprintf(stderr, "Thanks\n");
      fflush(stderr);
      return 0;
    }

  /*
   * XXX end manual startup hack
   */

  if (!(hp->h_flag & HST_PASSWORD))
    {		/* use rsh to start */
      int wpfd[2], rpfd[2], epfd[2];
      int i;

      if (debugmask)
	{
	  fprintf(stderr, "phase1() trying rsh to %s\n", hn);
	}

      /* fork an rsh to startup the slave pvmd */

#ifdef	IMA_TITN
      if (socketpair(AF_UNIX, SOCK_STREAM, 0, wpfd) == -1
	  || socketpair(AF_UNIX, SOCK_STREAM, 0, rpfd) == -1
	  || socketpair(AF_UNIX, SOCK_STREAM, 0, epfd) == -1)
	{
	  pvm_error("phase1() socketpair");
	  goto oops;
	}
#else
      if (pipe(wpfd) == -1 || pipe(rpfd) == -1 || pipe(epfd) == -1)
	{
	  pvm_perror("phase1() pipe");
	  goto oops;
	}
#endif

      if (debugmask)
	{
	  fprintf(stderr, "phase1() pipes: %d %d %d %d %d %d\n",
		  wpfd[0], wpfd[1], rpfd[0], rpfd[1], epfd[0], epfd[1]);
	}

      if ((pid = fork()) == -1) {
	PDYN_goodbye("phase1() fork");
	return 0;
      }
      if (!pid)
	{
	  (void)dup2(wpfd[0], 0);
	  (void)dup2(rpfd[1], 1);
	  (void)dup2(epfd[1], 2);
	  for (i = getdtablesize(); --i > 2; )
	    (void)close(i);
	  ac = 0;
	  av[ac++] = strdup(RSHCOMMAND);
	  av[ac++] = hn;
	  if (hp->h_login)
	    {
	      av[ac++] = strdup("-l");
	      av[ac++] = hp->h_login;
	    }
	  av[ac++] = hp->h_cmd;
	  av[ac++] = 0;
	  if (debugmask)
	    {
	      for (ac = 0; av[ac]; ac++)
		fprintf(stderr, "av[%d]=\"%s\" ", ac, av[ac]);
	      fputc('\n', stderr);
	    }
	  execvp(av[0], av);
	  fputs("phase1() execvp failed\n", stderr);
	  fflush(stderr);
	  _exit(1);
	}
      (void)close(wpfd[0]);
      (void)close(rpfd[1]);
      (void)close(epfd[1]);
      sp->s_wfd = wpfd[1];
      sp->s_rfd = rpfd[0];
      sp->s_efd = epfd[0];

    } 
  else
    {		/* use rexec to start */
#ifdef NOREXEC
      fprintf(stderr, "slconfg() sorry, no rexec()\n");
      goto oops;
#else
      if (debugmask)
	{
	  fprintf(stderr, "phase1() rexec \"%s\"\n", hp->h_cmd);
	}
      if ((sp->s_wfd = sp->s_rfd = P_rexec(&hn, execport,
					 (hp->h_login ? hp->h_login : username),
					 (char*)0, hp->h_cmd, &sp->s_efd))
	  == -1)
	{
	  fprintf(stderr, "phase1() rexec failed for host %s\n", hn);
	  goto oops;
	}
#endif
    }
  return 0;

 oops:
  hp->h_result = STRALLOC("PvmCantStart");
 oops2:
  if (sp->s_wfd != -1)
    close(sp->s_wfd);
  if (sp->s_rfd != -1)
    close(sp->s_rfd);
  if (sp->s_efd != -1)
    close(sp->s_efd);
  sp->s_wfd = sp->s_rfd = sp->s_efd = -1;
  return 1;
}

#endif	/*RSHNPLL > 1*/



