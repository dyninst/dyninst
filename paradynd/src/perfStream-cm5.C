/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, \
  Karen Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/perfStream-cm5.C,v 1.1 1994/07/14 14:45:52 jcargill Exp $";
#endif



/*
 * File containing the CM5 specific traceHandling stuff.  Will we
 * still need this after changing to synchronous sampling of the nodes?
 *
 * $Log: perfStream-cm5.C,v $
 * Revision 1.1  1994/07/14 14:45:52  jcargill
 * Added new file for dynRPC functions, and a default (null) function for
 * processArchDependentTraceStream, and the cm5 version.
 *
 */

#include <cm/cmmd.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "rtinst/src/traceio.h"
#include "primitives.h"
#include "util.h"


extern time64 firstRecordTime;
extern Boolean firstSampleReceived;
extern int CMMD_enabled;

extern void createResource(traceHeader *header, struct _newresource *res);


void processArchDependentTraceStream()
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;
    int bufStart;		/* current starting point */

    static char buffer[TRACE_BUF_SIZE];	/* buffer for data */
    static int bufEnd = 0;	/* last valid data in buffer */

    /* Check for node I/O */
    while (CMMD_poll_for_services() == 1)
	;	/* TEMPORARY:    XXXXXX */


    /*  */
    while (1) {
	/* Check whether there's any data that we need to worry about... */
	/* Otherwise, cut out before CMMD calls block! */
	if (!CMMD_enabled)
	    return;
	if (!CMMD_msg_pending (CMMD_ANY_NODE, CMMD_ANY_TAG))
	    return;

//	printf ("CMMD Message appears to be pending...\n");

	/* Read the pending CM5 network traffic. */
	CMMD_receive_block (CMMD_ANY_NODE, CMMD_ANY_TAG, 
			    &buffer[bufEnd], TRACE_BUF_SIZE); 
	ret = CMMD_bytes_received();

	/* Process the trace...  */
//	printf ("Got %d trace bytes from one of the nodes...\n", ret);

	if (ret < 0) {
	    perror("CMMD_receive_block error");
	    exit(-2);
	} else if (ret == 0) {
	    /* end of file */
	    printf("got ZERO length message from node\n");
	    //	curr->traceLink = -1;
	    //	curr->status = exited;
	    return;
	}

	bufEnd += ret;

	bufStart = 0;
	while (bufStart < bufEnd) {
	    if (bufEnd - bufStart < (sizeof(traceStream) + sizeof(header))) {
		break;
	    }

	    if (bufStart % WORDSIZE != 0) /* Word alignment check */
		break;		/* this will re-align by shifting */

	    memcpy(&sid, &buffer[bufStart], sizeof(traceStream));
	    bufStart += sizeof(traceStream);

	    memcpy(&header, &buffer[bufStart], sizeof(header));
	    bufStart += sizeof(header);

	    if (header.length % WORDSIZE != 0)
		printf("Warning: non-aligned length (%d) received on traceStream.  Type=%d\n", header.length, header.type);
	    
	    if (bufEnd - bufStart < header.length) {
		/* the whole record isn't here yet */
		bufStart -= sizeof(traceStream) + sizeof(header);
		break;
	    }

	    recordData = &buffer[bufStart];
	    bufStart +=  header.length;

	    /*
	     * convert header to time based on first record.
	     *
	     */
	    if (!firstRecordTime) {
		double st;

		firstRecordTime = header.wall;
		st = firstRecordTime/1000000.0;
		printf("started at %f\n", st);
	    }
	    header.wall -= firstRecordTime;

	    switch (header.type) {

	      case TR_NEW_RESOURCE:
		createResource(&header, (struct _newresource *) recordData);
		printf ("New Resource received!\n");
		break;

	      case TR_SAMPLE:
		processSample(&header, (traceSample *) recordData);
		firstSampleReceived = True;

//		printf ("Received sample!\n");
		break;

	      case TR_EXIT:
		//  Need to look up curr...
		    //	    curr->status = exited;
		break;

	      case TR_NODE_PRINT:
		logLine((char *) recordData);
		break;

	      case -1:		/* leavings from non-reporting nodes */
//		printf ("Aggregation leavings...\n");
		break;		/* after hardware aggregation on the cm5 */

	      default:
		printf("got record type %d on sid %d\n", header.type, sid);
	    }
	}

	/* copy those bits we have to the base */
	memcpy(buffer, &buffer[bufStart], bufEnd - bufStart);
	bufEnd = bufEnd - bufStart;
    }
}
