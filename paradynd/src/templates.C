/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.11  1994/11/10 18:58:20  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.10  1994/11/09  18:40:42  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.9  1994/11/02  11:18:03  markc
 * Instantiated new classes here.
 *
 * Revision 1.8  1994/09/22  02:27:18  markc
 * Gave type names to typedefs
 *
 * Revision 1.7  1994/09/20  18:18:33  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.6  1994/08/08  20:13:48  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/08/02  18:25:09  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.4  1994/07/26  20:02:09  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.3  1994/07/20  23:23:44  hollings
 * added insn generated metric.
 *
 * Revision 1.2  1994/06/27  18:57:17  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/05/30  19:51:17  hollings
 * added code to support external templates.
 *
 *
 */

#pragma implementation "Pair.h"
#include "util/h/Pair.h"

#pragma implementation "Vector.h"
#include "util/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "util/h/tunableConst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "util/h/Object.h"

typedef vector<heapItem*> a1;
typedef vector<process*> a2;
typedef vector<pdFunction*> a3;
typedef vector<heapItem*> a4;
typedef vector<resourcePredicate*> a5;
typedef vector<Symbol> a6;

typedef dictionary_hash<string, internalMetric*> z1;
typedef dictionary_hash<metricDefinitionNode*, internalMetric*> z2;
typedef dictionary_hash<metricDefinitionNode*, kludgeInternalMetric*> z3;
typedef dictionary_hash<string, kludgeInternalMetric*> z4;

typedef dictionary_hash <int, dataReqNode*> lb;
typedef dictionary_hash <Address, Symbol*> la;
typedef dictionary_hash <instPoint*, point*> l0;
typedef dictionary_hash <string, unsigned> l1;
typedef dictionary_hash <string, internalMetric*> l2;
typedef dictionary_hash <string, resource*> l3;
typedef dictionary_hash <string, image*> l4;
typedef dictionary_hash <instPoint*, unsigned> l5;
typedef dictionary_hash <string, pdFunction*> l6;
typedef dictionary_hash <unsigned, pdFunction*> l7;
typedef dictionary_hash <string, vector<pdFunction*>*> l8;
typedef dictionary_hash <string, internalSym*> l9;
typedef dictionary_hash <string, module *> l10;
typedef dictionary_hash <unsigned, metricDefinitionNode*> l11;
typedef dictionary_hash <unsigned, heapItem*> l12;
typedef dictionary_hash <int, process*> l13;

typedef dictionary_hash_iter<string, internalMetric*> zi1;
typedef dictionary_hash_iter<metricDefinitionNode*, internalMetric*> zi2;
typedef dictionary_hash_iter<metricDefinitionNode*, kludgeInternalMetric*> zi3;
typedef dictionary_hash_iter<string, kludgeInternalMetric*> zi4;

typedef dictionary_hash_iter <Address, Symbol*> ila;
typedef dictionary_hash_iter <instPoint*, point*> il0;
typedef dictionary_hash_iter <string, unsigned> il1;
typedef dictionary_hash_iter <string, internalMetric*> il2;
typedef dictionary_hash_iter <string, resource*> il3;
typedef dictionary_hash_iter <string, image*> il4;
typedef dictionary_hash_iter <instPoint*, unsigned> il5;
typedef dictionary_hash_iter <string, pdFunction*> il6;
typedef dictionary_hash_iter <unsigned, pdFunction*> il7;
typedef dictionary_hash_iter <string, vector<pdFunction*>*> il8;
typedef dictionary_hash_iter <string, internalSym*> il9;
typedef dictionary_hash_iter <string, module *> il10;
typedef dictionary_hash_iter <unsigned, metricDefinitionNode*> il11;
typedef dictionary_hash_iter <unsigned, heapItem*> il12;
typedef dictionary_hash_iter <int, process*> il13;
