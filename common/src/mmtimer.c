/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 
#include <sn/mmtimer.h> /* MMTIMER_* */

/* getpid() */
#include <sys/types.h>
#include <unistd.h>              

#include <stdio.h> /* fprintf() */
#include <sys/ioctl.h> /* for ioctl() */
#include <sys/mman.h> /* for mmap() */

/* The mmtimer, once initialized by a call to isMMTimerAvail(),
   is always (mmdev_clicks_per_tick * (*mmdev_timer_addr)). */
static unsigned long mmdev_clicks_per_tick = 0;
static volatile unsigned long * mmdev_timer_addr = NULL;

/* The defined interface in common/h/timing.h is too clumsy, so duplicate code. */
double getLocalCyclesPerSecond() {
	FILE * cpuinfo = fopen( "/proc/cpuinfo", "r" );
	if( cpuinfo == NULL ) { return 0; }
	
	double MHz = 0.0;
	/* Scan /proc/cpuinfo until we find something; assume all CPUs the same. */
	while( ! feof( cpuinfo ) ) {
		char buffer[ 256 ];
		char * result = fgets( buffer, 255, cpuinfo );
		if( result != NULL ) {
			int status = sscanf( result, "cpu MHz : %lf", & MHz );
			if( status == 1 ) { 
				fclose( cpuinfo );
				return ( MHz * 1000000.0 );
				}
			}
		} /* end scanning loop */
		
	fclose( cpuinfo );
	return 0;
	} /* end getLocalCyclesPerSecond() */

int isMMTimerAvail() {
	int fd = 0;
	unsigned long femtosecondsPerTick = 0;
	int mmTimerOffset = 0;
	
	/* Don't do anything twice. */
	if( mmdev_clicks_per_tick != 0 || mmdev_timer_addr != 0 ) { return 1; }
	
	/* Attempt to setup the mmtimer. */
	if( ( fd = open( MMTIMER_FULLNAME, O_RDONLY ) ) == -1 ) {
		return 0;
		}
	if( ( mmTimerOffset = ioctl( fd, MMTIMER_GETOFFSET, 0 ) ) == -ENOSYS ) {
		close( fd );
		return 0;
		}
	 if( ( mmdev_timer_addr = (volatile unsigned long int *)mmap( 0, getpagesize(), PROT_READ, MAP_SHARED, fd, 0 ) ) == NULL ) {
	 	close( fd );
	 	return 0;
		}

	mmdev_timer_addr += mmTimerOffset;
	ioctl( fd, MMTIMER_GETRES, & femtosecondsPerTick );
	mmdev_clicks_per_tick = (unsigned long)((getLocalCyclesPerSecond() * 1.0e-15) * femtosecondsPerTick);
	 
	assert( mmdev_clicks_per_tick != 0 );
	
	///* DEBUG */ fprintf( stderr,	"%d: %lf MHz, "
	//								"microseconds/tick = %lf, "
	//								"clicks_per_tick = %lu\n",
	//								getpid(),
	//								getLocalCyclesPerSecond() / 1.0e6,
	//								femtosecondsPerTick / 1.0e9,
	//								mmdev_clicks_per_tick );

    close( fd );
    return 1;
	} /* end isMMtimerAvail() */
