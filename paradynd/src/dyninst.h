/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * dyninst.h - exported interface to instrumentation.
 *
 * $Log: dyninst.h,v $
 * Revision 1.11  1994/09/22 01:50:54  markc
 * reorganized, temporary
 *
 * Revision 1.10  1994/08/08  20:13:34  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.9  1994/06/27  21:28:08  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.8  1994/05/18  00:52:26  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.7  1994/05/16  22:31:49  hollings
 * added way to request unique resource name.
 *
 * Revision 1.6  1994/04/09  18:34:52  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 * Revision 1.5  1994/03/31  01:48:32  markc
 * Added default args to addProcess definition.
 *
 * Revision 1.4  1994/03/24  16:41:58  hollings
 * Moved sample aggregation to lib/util (so paradyn could use it).
 *
 * Revision 1.3  1994/03/20  01:53:04  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.2  1994/02/01  18:46:50  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:17  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.5  1994/01/26  06:57:07  hollings
 * new header location
 *
 * Revision 1.4  1993/12/13  20:02:23  hollings
 * added applicationDefined
 *
 * Revision 1.3  1993/08/26  18:22:24  hollings
 * added cost model
 *
 * Revision 1.2  1993/07/01  17:03:17  hollings
 * added debugger calls and process control calls
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "rtinst/h/trace.h"
#include "util/h/stringDecl.h"
#include "util/h/stringPool.h"

extern stringPool pool;

/* time */
typedef double timeStamp;		

#endif
