/* 
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski
 *     Jeff Hollingsworth, and Bruce Irvin. All rights reserved.
 *
 * This software is furnished under the condition that it may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.  The name of the principals
 * may not be used in any advertising or publicity related to this
 * software without specific, written prior authorization.
 * Any use of this software must include the above copyright notice.
 *
 */

/*
 * Configuration for installing IPS.
 *
 * Revision 1.7  90/12/07  12:55:43  rbi
 * Added a #define for ultrix40
 * 
 * Revision 1.6  90/11/28  15:51:11  rbi
 * Upgrade to Ultrix 4.0
 * 
 * Revision 1.5  90/10/11  18:01:45  hollings
 * Added ATSCC_VERSION flag.
 * 
 * Revision 1.4  90/10/11  13:58:17  hollings
 * Ready for release.
 * 
 * Revision 1.3  90/05/28  13:03:47  hollings
 * More comments.
 * 
 * Revision 1.2  90/04/20  13:07:35  hollings
 * Changed Sequent to use X11R4.
 * 
 * Revision 1.1  90/03/21  15:47:17  hollings
 * Initial revision
 * 
 *
 */

/*
 * Should the Slave use the Sequent parallel programming library.
#define SEQ_SLAVE
 */

/*
 * Is Suns' External Data Protocol Available.  Both the master and all slaves
 *   must be made with this flag the same.  
 *
 * WARNING: IPS has not been tested without this option for several years!
 *
 */
#define USEXDR		

/*
 * Location of various files.
 *
 */

/*
 * IPS_DIR is the location where ips will be installed.
 *   (See installation guide).
 *
 */
#define IPS_DIR			"/usr/public/ips"

/*
 * Define the default location for the slave analyst.  This can be overriden
 *   via the ips*slave resource.
 *
 */
#define DEF_SLAVE               "/usr/public/ips/slave"

/*
 * Define the default location for the message of the day.  The resource
 *   ips*motd overrides this.
 *
 */
#define DEF_MOTD                "/var/scratch/lib/ips/motd"

/*
 * Define the default location for the syscyl file.  The resources
 *   ips*sysctl overrides this.
 *
 */
#define DEF_SYSCTL              "/usr/public/ips/sysctl"

/*
 * size of trace buffers in memory.
 *
 */
#ifndef _SEQUENT_
#define TBSIZE	0x100000
#else
#define TBSIZE	0x10000
#endif

/*
 * APPLICATION_DEFAULT_DIR - Place to look for application defaults 
 *   file.  This only needs to be defined if the application default
 *   directory is not in a standard place.
 */
#define APPLICATION_DEFAULT_DIR	"/var/scratch/lib/app-defaults"


/*
 * Which version of the Sequent ATS C compiler is being used.
 *
 */
#define ATSCC_VERSION	"1.0"

/*
 * Which X version.  Supported values:
 *     1104	- X11 release 4
 *     1103	- X11 release 3
 *
 */
#define	X_VERSION	1104

/*
 * DO NOT EDIT BEYOND HERE!
 *   The rest of these macros are derived from earlier macros.
 */
#if defined(sequent) || defined(_SEQUENT_)
#define SEQUENT
#endif

#if defined(_SEQUENT_) || defined(CRAY)
#define SYSV
#endif

#if defined(_SEQUENT_)
#define DYNIX_PTX
#endif

#if defined(sequent)
#define DYNIX
#endif

#if defined(sequent) && defined(SLAVE_SIDE) && !defined(SEQ_SLAVE)
#define PARALLEL_SLAVE
#else
/* define this so we know one of the two is defined */
#define SEQ_SLAVE
#define shared 
#endif

#if !defined(SLAVE_SIDE)
#define shared
#endif

/*
 * Determine the ultrix version.
 *  The SYS_startcpu system call should be defined only 
 *  in versions of ultrix that are 4.0 or newer.
 */
#ifdef ultrix
#include <sys/syscall.h>
#ifdef SYS_startcpu
#define ultrix40
#else
#define ultrix30
#endif /* SYS_startcpu */
#endif /* ultrix */

#if (X_VERSION >= 1104)
#define ATHENA_DIR <X11/Xaw
#define XAW_BC
#else
#define ATHENA_DIR <X11
#endif
