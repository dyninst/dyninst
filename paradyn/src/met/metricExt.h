
/*
 * $Log: metricExt.h,v $
 * Revision 1.4  1995/05/18 10:58:36  markc
 * mdl
 *
 * Revision 1.3  1994/08/22  15:53:33  markc
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

#include "util/h/String.h"
#include "paradyn/src/DMthread/DMinternals.h"

extern bool metMain(applicationContext *appCon, string &file);
extern bool mdl_init();
extern bool mdl_send(dynRPCUser *du);
extern void mdl_destroy();
extern bool mdl_apply();

#endif
