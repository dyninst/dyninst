
#ifndef KLUDGES_H
#define KLUDGES_H

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
 */

typedef int (*xdr_rd_func)(const void *, char *, const int);
typedef int (*xdr_wr_func)(const void *, const char *, const int);

#if defined(sparc_sun_sunos4_1_3)
#include "util/h/sunosHeaders.h"

#elif defined(sparc_sun_solaris2_4)
#include "util/h/solarisHeaders.h"

#elif defined(i386_unknown_netbsd1_0)
#include "util/h/netbsdHeaders.h"

#elif defined(hppa1_1_hp_hpux)
#include "util/h/hpuxHeaders.h"

#elif defined(sparc_tmc_cmost7_3)
#include "util/h/sunosHeaders.h"

#endif  /* architecture specific */

#endif /* KLUDGES_H */

