
/*
 * $Log: metricExt.h,v $
 * Revision 1.3  1994/08/22 15:53:33  markc
 * Config language version 2.
 *
 * Revision 1.2  1994/07/07  13:10:43  markc
 * Turned off debugging printfs.
 *
 * Revision 1.1  1994/07/07  03:25:32  markc
 * Configuration language parser.
 *
 */

#ifndef _METRIC_EXT_H
#define _METRIC_EXT_H

extern int metMain(char *userFile);

extern int metDoTunable ();
extern int metDoProcess ();
extern int metDoDaemon  ();
extern int metDoVisi ();

#endif
