
/** beginning of the pragma */

/** the initiation of the template objects for
  * the code coverage different than the dynsint library.
  */

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "BPatch_Set.h"
#include "BPatch_Set.h"
/** end of the pragma */

#include "common/h/Types.h"
#include "common/h/String.h"

class BPatch_function;
class FunctionCoverage;
class BPFunctionList;

/** beginning of the template class initialization */
template struct comparison<BPatch_function*>;
template class BPatch_Set<BPatch_function*>;

template struct comparison<FunctionCoverage*>;
template class BPatch_Set<FunctionCoverage*>;

template class pdvector<BPFunctionList*>;
template class dictionary_hash<string,BPFunctionList*>;
template class pdvector<dictionary_hash<string,BPFunctionList*>::entry>;

/** end of the template class initialization */
