//
// UI and tunable constant templates
//


#pragma implementation "list.h"
#include "util/h/list.h"
#include "util/h/String.h"
#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#pragma implementation "Queue.h"
#include "util/h/Queue.h"
#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"
#pragma implementation "DictionaryLite.h"
#include "util/src/DictionaryLite.C"


/* ******************************
 * TCthread stuff
 */
#include "paradyn/src/TCthread/tunableConst.h"

template class vector<tunableBooleanConstant>;
template class vector<tunableFloatConstant>;
template class dictionary_lite<string, tunableBooleanConstant>;
template class dictionary_lite<string, tunableFloatConstant>;
template class pair<string, tunableBooleanConstant>;
template class pair<string, tunableFloatConstant>;
template class vector< dictionary_lite<string, tunableBooleanConstant> :: hash_pair >;
template class vector< dictionary_lite<string, tunableFloatConstant> :: hash_pair >;
template class vector< vector< dictionary_lite<string, tunableBooleanConstant>::hash_pair > >;
template class vector< vector< dictionary_lite<string, tunableFloatConstant>::hash_pair > >;


/* *************************************
 * UIthread stuff
 */
#include "VM.thread.h"
#include "../src/UIthread/UIglobals.h"

template class List<metricInstInfo *>;
template class ListItem<metricInstInfo *>;
template class vector<VM_activeVisiInfo>;
template class vector<string*>;
template class dictionary<unsigned,string*>;
template class dictionary_hash<unsigned, string*>;
template class vector< pair<unsigned,string *> >;
template class pair<unsigned, string*>;
template class vector< vector<dictionary_hash<unsigned, string *>::hash_pair> >;
template class vector< dictionary_hash<unsigned, string *>::hash_pair >;

/* *************************************
 * UIthread WhereAxis stuff
 */

#include "paradyn/src/UIthread/where4tree.C"
#include "paradyn/src/UIthread/rootNode.h"
template class where4tree<whereAxisRootNode>;
template class vector<const whereAxisRootNode *>;
template class vector<where4tree<whereAxisRootNode>::childstruct>;
template class dictionary_lite< resourceHandle, where4tree<whereAxisRootNode> * >;
template class vector<where4tree<whereAxisRootNode> *>;
template class vector<dictionary_lite<resourceHandle,where4tree<whereAxisRootNode>*>::hash_pair>;
template class vector< vector< dictionary_lite<resourceHandle,where4tree<whereAxisRootNode>*>::hash_pair> >;
template class pair<resourceHandle, where4tree<whereAxisRootNode>* >;

#include "paradyn/src/UIthread/graphicalPath.C"
template class whereNodeGraphicalPath<whereAxisRootNode>;

#include "paradyn/src/UIthread/simpSeq.C"
template class simpSeq<unsigned>;

#include "paradyn/src/UIthread/abstractions.h"
template class vector<abstractions::whereAxisStruct>;

/* *************************************
 * UIthread Search History Graph stuff
 */

#include "paradyn/src/UIthread/shgRootNode.h"
template class where4tree<shgRootNode>;
template class vector<where4tree<shgRootNode>::childstruct>;
template class whereNodeGraphicalPath<shgRootNode>;
template class vector<const shgRootNode *>;

#include "paradyn/src/UIthread/shgPhases.h"
template class vector<shgPhases::shgStruct>;

template class dictionary_lite<unsigned, where4tree<shgRootNode> *>;
template class vector<where4tree<shgRootNode> *>;
template class vector<dictionary_lite<unsigned, where4tree<shgRootNode> *>::hash_pair>;
template class vector< vector< dictionary_lite<unsigned, where4tree<shgRootNode> *>::hash_pair> >;
template class pair<unsigned, where4tree<shgRootNode> *>;

template class dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>;
template class vector<dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>::hash_pair>;
template class vector< vector< dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>::hash_pair> >;
template class pair<where4tree<shgRootNode> *, where4tree<shgRootNode> *>;

template class vector<Tk_3DBorder>; // shg consts

/* *************************************
 * UIthread Logo Stuff
 */

#include "paradyn/src/UIthread/pdLogo.h"
template class vector<pdLogo *>;
template class dictionary_lite<string, pdLogo *>;
template class pair<string, pdLogo *>;
template class vector<dictionary_lite<string, pdLogo *>::hash_pair>;
template class vector< vector<dictionary_lite<string,pdLogo*>::hash_pair> >;

template class dictionary_lite<string, pdLogo::logoStruct>;
template class vector<pdLogo::logoStruct>;
template class vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair>;
template class vector< vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair> >;
template class pair<string, pdLogo::logoStruct>;

/* *************************************
 * UIthread Misc Stuff
 */

#include "paradyn/src/UIthread/minmax.C"
template int min(int,int);
template int max(int,int);
template void ipmin(int &, int);
template void ipmax(int &, int);
template float min(float,float);
template float max(float,float);
