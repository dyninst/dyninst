// templatesUIWASHG.C
// templates in the UIthread related to the where axis and search history graph

// seems to be needed for sunos; why I don't know
#ifndef NULL
#define NULL 0
#endif

/* *************************************
 * UIthread WhereAxis stuff
 */

#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#pragma implementation "DictionaryLite.h"
#include "util/src/DictionaryLite.C"

#include "paradyn/src/UIthread/where4tree.C"
#include "paradyn/src/UIthread/rootNode.h"
template class where4tree<whereAxisRootNode>;
template class vector<const whereAxisRootNode *>;
template class vector<where4tree<whereAxisRootNode>::childstruct>;
template class dictionary_lite< resourceHandle, where4tree<whereAxisRootNode> * >;
template class vector<where4tree<whereAxisRootNode> *>;
template class vector<dictionary_lite<resourceHandle,where4tree<whereAxisRootNode>*>::hash_pair>;
template class vector< vector< dictionary_lite<resourceHandle,where4tree<whereAxisRootNode>*>::hash_pair> >;

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

template class dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>;
template class vector<dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>::hash_pair>;
template class vector< vector< dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *>::hash_pair> >;

template class dictionary_lite< unsigned, vector<where4tree<shgRootNode>*> >;
template class vector< vector<dictionary_lite< unsigned, vector<where4tree<shgRootNode>*> > :: hash_pair > >;
template class vector<dictionary_lite< unsigned, vector<where4tree<shgRootNode>*> > :: hash_pair >;
template class vector< vector<where4tree<shgRootNode>*> >;


template class vector<Tk_3DBorder>; // shg consts

