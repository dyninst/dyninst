
/*
 * This file is only used to create the metric tester (metTester) 
 * executable.
 */

/*
 * $Log: metTemplates.C,v $
 * Revision 1.2  1994/08/22 15:53:31  markc
 * Config language version 2.
 *
 */


#pragma implementation "list.h"
#include "util/h/list.h"

#include "../met/metParse.h" 

class stringList;
class daemonMet;
class processMet;
class visiMet;
class tunableMet;

typedef List<stringList*>;
typedef List<daemonMet*>;
typedef List<processMet*>;
typedef List<visiMet*>;
typedef List<tunableMet*>; 
