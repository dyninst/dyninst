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


/*
 * Kludges to handle broken system includes and such...
 *
 * $Log: kludges.h,v $
 * Revision 1.1  1994/07/14 17:41:56  jcargill
 * Added kludges file to clean up compilation warnings.
 *
 */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

typedef void (*Sa_Handler)();

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

