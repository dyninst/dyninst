
/*
 * $Log: metricExt.h,v $
 * Revision 1.5  1995/06/02 20:49:03  newhall
 * made code compatable with new DM interface
 * fixed problem with force option in visiDef
 * fixed hpux link errors
 *
 * Revision 1.4  1995/05/18  10:58:36  markc
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
#include "paradyn/src/DMthread/DMinclude.h"
#include "dyninstRPC.xdr.CLNT.h"

extern bool metMain(string &file);
extern bool mdl_init();
extern bool mdl_send(dynRPCUser *du);
extern void mdl_destroy();
extern bool mdl_apply();

#endif
