
/*
 * $Log: metricExt.h,v $
 * Revision 1.2  1994/07/07 13:10:43  markc
 * Turned off debugging printfs.
 *
 * Revision 1.1  1994/07/07  03:25:32  markc
 * Configuration language parser.
 *
 */

#ifndef _METRIC_EXT_H
#define _METRIC_EXT_H

extern int metMain();

extern int metDoTunable ();
extern int metDoProcess ();
extern int metDoDaemon  ();
extern int metDoVisi ();

#endif
