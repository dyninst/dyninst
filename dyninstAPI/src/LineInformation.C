#include <stdlib.h>

#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/src/util.h"

#ifdef TIMED_PARSE
extern int max_addr_per_line;
extern int max_line_per_addr;
#endif

#ifdef OLD_LINE_INFO

#define FILE_REALLOC_INCREMENT 32
#define FUNCTION_REALLOC_INCREMENT 64
#define TUPLE_REALLOC_INCREMENT 128

#endif

//this function implements binary search and returns the found element.
//besides the found element it returns the next bigger line numbered one
//The found element is the first same values element in the array in existence
//of many same values elements.
//in case it is not found it will RETURN NULL. if the next element
//is not available it will assign NULL to the reference.
//the next contains the entry with the first greater line number
#ifdef OLD_LINE_INFO
tuple** binarySearchLineFirst(tuple element,tuple** array,int howMany,tuple**& next)
{
	int low = 0;
	int high = howMany-1;
	int mid;
	tuple** ret=NULL;
	do{
		mid = (low+high)/2;
		if(element.lineNo < array[mid]->lineNo) 
			high = mid - 1;
		else if(element.lineNo > array[mid]->lineNo)
			low = mid + 1;
		else
			break;
	}while(low <= high);

	//if found then porepare to return and the next available
	if(element.lineNo == array[mid]->lineNo){
		int index = mid;
		for(;(index >= 0)&&(array[index]->lineNo == element.lineNo);index--);
		ret = array+(index+1);
		index = mid;
		for(;(index < howMany)&&(array[index]->lineNo == element.lineNo);index++);
		if(index == howMany)
			next = NULL;
		else
			next = array+index;
	}
	else if(low == howMany)
		next = NULL;
	else
		next = array+low;

	return ret;
}
#endif
#ifdef NOTDEF
//  need a replacement function for binary search that is compat with map struct
tuple *FileLineInformation::findByLine(unsigned short lineNo, tuple **next)
{
  tuple *ret = NULL;
  *next = NULL;
  l2a_iter_t l2a_iter = lineToAddr.find(lineNo);
  if (lineToAddr.end() == l2a_iter) return NULL;  // lineNo not in map

  tuple_list_t *line_addrs = l2a_iter->second;
  if (!line_addrs->size()) {
    cerr << __FILE__ << ":" << __LINE__ << "ERROR: empty set for line " << lineNo << endl;
    return NULL;
  }

  ret = (*line_addrs)[0]; // the smallest address tuple in the list

  //  Not sure if this is functionally equivalent to the code above... but it should behave
  // in the manner that the comment suggests.  Danger!

  // do the same thing with iter++
  il2a_iter++;
  if (l2a_iter == lineToAddr.end())  return ret;  // No next line

  line_addrs = l2a_iter.second;
  if (!line_addrs->size()) {
    cerr << __FILE__ << ":" << __LINE__ << "ERROR: empty set for line " << lineNo << endl;
    return ret;
  }

  *next = (*line_addrs)[0]; // the smallest address tuple in the list
  return ret;
}

#endif

//this function implements binary search and returns the found element.
//besides the found element it returns the previous smaller address entry
//The found element is the first same values element in the array in existence
//of many same values elements.
//in case it is not found it will RETURN NULL. if the next element
//is not available it will assign NULL to the reference.
//the next contains the entry with the previous smaller address
#ifdef OLD_LINE_INFO
tuple** binarySearchAddrFirst(tuple element,tuple** array,int howMany,tuple**& next)
{
	int low = 0;
	int high = howMany-1;
	int mid;
	tuple** ret=NULL;
	do{
		mid = (low+high)/2;
		if(element.codeAddress < array[mid]->codeAddress) 
			high = mid - 1;
		else if(element.codeAddress > array[mid]->codeAddress)
			low = mid + 1;
		else
			break;
	}while(low <= high);

	////if found then porepare to return and the next available
	if(element.codeAddress == array[mid]->codeAddress){
		int index = mid;
		for(;(index>=0)&&(array[index]->codeAddress == element.codeAddress);index--);
		ret = array+(index+1);
		if(index == -1)
			next = NULL;
		else{
			Address addr = array[index]->codeAddress;
			for(;(index>=0) && (array[index]->codeAddress == addr);index--);
			next = array+(index+1);
		}
	}
	else if(high == -1)
		next = NULL;
	else{
		Address addr = array[high]->codeAddress;
		for(;(high >=0) && (array[high]->codeAddress == addr);high--);
		next = array+(high+1);
	}
	return ret;
}
#endif

//constructor
FileLineInformation::FileLineInformation(string fileName)
  :  sourceFileName(fileName),
#ifdef OLD_LINE_INFO
	 tupleCapacity(0),
     size(0),
     lineToAddr(NULL),
     addrToLine(NULL),
     functionNameList(NULL),
     lineInformationList(NULL),
     functionCount(0),
	 functionCapacity(0)
#else
  functionInfoHash(string::hash)
#endif
 
{}

//destructor
FileLineInformation::~FileLineInformation(){


#ifdef OLD_LINE_INFO
	int i;
	for(i=0;i<size;i++)
		delete lineToAddr[i];
	free(lineToAddr);
	free(addrToLine);

	for(i=0;i<functionCount;i++){
		delete lineInformationList[i];
		delete functionNameList[i];
	}
	free(functionNameList);
	free(lineInformationList);

#else

	//  Delete all tuples from just one of our maps (contents are duplicates)
	l2a_iter_t map_iter;
	for (map_iter = lineToAddr.begin(); map_iter != lineToAddr.end(); ++map_iter) {
	  pdvector<tuple *> *line_tuples = map_iter->second;
	  for (unsigned int i = 0; i < line_tuples->size(); i++) {
	    delete (*line_tuples)[i];
	    line_tuples->clear();
	  }
	  delete line_tuples;
	}

	//  Delete sub-maps (but not tuples) from second structure
	a2l_iter_t map_iter2;
	for (map_iter2 = addrToLine.begin(); map_iter2 != addrToLine.end(); ++map_iter2) {
	  pdvector<tuple *> *addr_tuples = map_iter->second;
	  addr_tuples->clear();
	  delete addr_tuples;
	}

	lineToAddr.clear();
	addrToLine.clear();

	dictionary_hash_iter<string, FunctionInfo *> del_iter(functionInfoHash);
	string fName;
	FunctionInfo *fInfo;
	while (del_iter.next(fName, fInfo))
	  delete (fInfo);
	
	functionInfoHash.clear();
#endif
	
}

#ifdef OLD_LINE_INFO
int FileLineInformation::binarySearch(string functionName){

  int low = 0;
  int high = functionCount-1;
  int mid;
  while (low <= high){
    mid = (low+high)/2;
    string entryName = *functionNameList[mid];
    if(functionName < entryName)
      high = mid - 1;
    else if(functionName > entryName)
      low = mid + 1;
    else 
      return (int)mid;
  }

  return -1;
}
#endif

//returns the function info structure
FunctionInfo *FileLineInformation::findFunctionInfo(string functionName)
{
#ifdef OLD_LINE_INFO
	int check = binarySearch(functionName);
	if( check >= 0 )
		return lineInformationList[check];
#else
	if (functionInfoHash.defines(functionName))
	  return functionInfoHash[functionName];

#endif
	return NULL;
}

//inserts function wntry to the mapping from function name to its info
FunctionInfo* FileLineInformation::insertFunction(string functionName){
#ifdef OLD_LINE_INFO
	FunctionInfo* fInfo = findFunctionInfo(functionName);
	if(!fInfo){

		if((unsigned)functionCount == functionCapacity){

			functionCapacity += FUNCTION_REALLOC_INCREMENT;

			if(functionCount == 0){
				functionNameList = (string**)malloc(functionCapacity*sizeof(string*));
				lineInformationList = (FunctionInfo**)malloc(functionCapacity*sizeof(FunctionInfo*));
			}
			else{
				functionNameList = (string**)realloc(functionNameList,
					functionCapacity*sizeof(string*));
				lineInformationList = (FunctionInfo**)realloc(lineInformationList,
					functionCapacity*sizeof(FunctionInfo*));
			}
		}

		unsigned short i = functionCount;
		for(;i > 0;i--){
                        if(functionName < *functionNameList[i-1]){
                                functionNameList[i] = functionNameList[i-1];
                                lineInformationList[i] = lineInformationList[i-1];
                        }
                        else
                                break;
                }
		fInfo = new FunctionInfo();
		lineInformationList[i] = fInfo;
		functionNameList[i] = new string(functionName);
		functionCount++;
	}		
#else
	
	FunctionInfo *fInfo = NULL;
	if (functionInfoHash.defines(functionName)) {
	  fInfo = functionInfoHash[functionName];
	}
	else {
	  fInfo = new FunctionInfo();
	  functionInfoHash[functionName] = fInfo;
	}
#endif

	return fInfo;
}

//returns true if the function has an entry in the mapping
bool FileLineInformation::findFunction(string functionName){
#ifdef OLD_LINE_INFO
	int check = binarySearch(functionName);
	return (check >= 0);
#else
	return functionInfoHash.defines(functionName);
#endif
}

void FileLineInformation::cleanEmptyFunctions(){

#ifdef OLD_LINE_INFO
	int maxFunctionCount = functionCount;
	int shift = 0;

	for(int i=0;i<maxFunctionCount;i++){
		FunctionInfo* funcinfo = lineInformationList[i];
		if(!funcinfo->validInfo){
			delete functionNameList[i];
			delete lineInformationList[i];
			functionCount--;
			shift++;
		}else if(shift > 0){
			functionNameList[i-shift] = functionNameList[i];
			lineInformationList[i-shift] = lineInformationList[i];
		}
	}
	if(!functionCount){
		free(functionNameList);
		free(lineInformationList);
		functionNameList = NULL;
		lineInformationList = NULL;
		functionCapacity = 0;
		return;
	}

	functionNameList =
		(string**)realloc(functionNameList,functionCount*sizeof(string*));
	lineInformationList =
		(FunctionInfo**)realloc(lineInformationList,functionCount*sizeof(FunctionInfo*));
	
	functionCapacity = functionCount;

#else
	dictionary_hash_iter<string, FunctionInfo *> iter(functionInfoHash);
	string fName;
	FunctionInfo *fInfo;
	while (iter.next(fName, fInfo))
	  if (!fInfo->validInfo)
	    functionInfoHash.undef(fName);
#endif
}

#ifndef OLD_LINE_INFO
int FileLineInformation::getFunctionLines(FunctionInfo *fInfo, BPatch_Set<unsigned short> *res)
{
  unsigned short startLine = fInfo->startLinePtr->lineNo; 
  unsigned short endLine = fInfo->endLinePtr->lineNo; 
  l2a_iter_t line_iter;

  if (lineToAddr.end() == (line_iter = lineToAddr.find(startLine))) {
    cerr << "ERROR:  no record of line " << startLine << endl;
    return -1;
  }

  do {
    (*res) += line_iter->first;
    line_iter++;
  } while ((line_iter != lineToAddr.end()) && (line_iter->first <= endLine));

  if ((line_iter == lineToAddr.end()) && ((--line_iter)->first < endLine)) {
    cerr << "ERROR:  reached end of map before end of function was found!" << endl;
    return -1;
  }

  return 0;
}
#endif

//delete the records for function
void FileLineInformation::deleteFunction(string functionName){
#ifdef OLD_LINE_INFO
	int i = binarySearch(functionName);
	if(i < 0)
		return;

	delete functionNameList[i];
	delete lineInformationList[i];

	functionCount--;
	if(!functionCount){
		free(functionNameList);
		free(lineInformationList);
		functionNameList = NULL; 
		lineInformationList = NULL; 
		functionCapacity = 0;
		return;
	}

	for(int j = i;j<functionCount;j++){
		functionNameList[j] = functionNameList[j+1];
		lineInformationList[j] = lineInformationList[j+1];
	}
	functionNameList = 
		(string**)realloc(functionNameList,functionCount*sizeof(string*));
	lineInformationList = 
		(FunctionInfo**)realloc(lineInformationList,functionCount*sizeof(FunctionInfo*));

	functionCapacity = functionCount;

#else
	if (functionInfoHash.defines(functionName)) {
	  FunctionInfo *delInfo = functionInfoHash[functionName];
	  functionInfoHash.undef(functionName);
	  delete delInfo;
	}
	
#endif
}

//updates records for a function
FunctionInfo* FileLineInformation::insertFunction(string functionName,Address baseAddr,
					 Address functionSize)
{
#ifdef OLD_LINE_INFO
	tuple toSearchBegin(0,baseAddr);
	tuple toSearchEnd(0,baseAddr+functionSize-1);

	tuple** next=NULL;
	tuple** retBegin = binarySearchAddrFirst(toSearchBegin,
					addrToLine,size,next);
	if(retBegin){
		unsigned short beginLineIndex = retBegin[0]->linePtr;	
		unsigned short beginAddrIndex = retBegin[0]->addrPtr;	

		tuple** retEnd = binarySearchAddrFirst(toSearchEnd,
					addrToLine,size,next);
		if(!retEnd && !next)
			return NULL;

		unsigned short endLineIndex;
		unsigned short endAddrIndex;
		if(retEnd){
			endLineIndex = retEnd[0]->linePtr;
			endAddrIndex = retEnd[0]->addrPtr;
		}
		else{
			endLineIndex = next[0]->linePtr;
			endAddrIndex = next[0]->addrPtr;
		}

		if((unsigned)functionCount == functionCapacity){

			functionCapacity += FUNCTION_REALLOC_INCREMENT;

			if(functionCount == 0){
				functionNameList = (string**)malloc(functionCapacity*sizeof(string*));
				lineInformationList = (FunctionInfo**)malloc(functionCapacity*sizeof(FunctionInfo*));
			} else {
				functionNameList = (string**)realloc(functionNameList,
						functionCapacity*sizeof(string*));
				lineInformationList = (FunctionInfo**)realloc(lineInformationList,
						functionCapacity*sizeof(FunctionInfo*));
			}
		}

		FunctionInfo* fInfo = new FunctionInfo();
                fInfo->startLinePtr = lineToAddr[beginLineIndex];
                fInfo->endLinePtr = lineToAddr[endLineIndex];
                fInfo->startAddrPtr = addrToLine[beginAddrIndex];
                fInfo->endAddrPtr = addrToLine[endAddrIndex];
                fInfo->validInfo = true;

		unsigned short i = functionCount;
		for(;i > 0;i--){
                        if(functionName < *functionNameList[i-1]){
                                functionNameList[i] = functionNameList[i-1];
                                lineInformationList[i] = lineInformationList[i-1
];
                        }
                        else
                                break;
                }

		lineInformationList[i] = fInfo;
		functionNameList[i] = new string(functionName);
		functionCount++;
		return fInfo;
	}

#else
	// find the start address of this function in our maps
	a2l_iter_t a2l_iter;
	if (addrToLine.end() == (a2l_iter = addrToLine.find(baseAddr))) {
	  cout << __FILE__ << ":" << __LINE__ << ": inserFunction:  baseAddress not found!" << endl;
	  return NULL; // baseAddress not found
	}

	pdvector<tuple *> *base_addr_tuples = a2l_iter->second;
	if (!base_addr_tuples->size()) {
	  cerr << __FILE__ ":" << __LINE__ << ": no tuples for address " << baseAddr << endl;
	  return NULL;  // this should never happen
	}

	tuple *start_tuple = (*base_addr_tuples)[0];

	// find the end address of this function in our maps
	// if end is not represented, find next address greater than end
	//
	// lower_bound is the last node that is NOT LESS than the arg.
	// if the lower bound does not match the arg, we want the node before it,
	// which is less than the arg.

	a2l_iter = addrToLine.find(baseAddr+functionSize -1);
	if (addrToLine.end() == a2l_iter) { // could not find end, try lower_bound (this could be 
	  // constructed better)
	  a2l_iter = addrToLine.lower_bound(baseAddr+functionSize -1);
	  if (a2l_iter == addrToLine.end()) {
	    cerr << __FILE__ ":" << __LINE__ << ": lower_bound failed! " << addrToLine.size() << endl;
	    return NULL;
	  }
	  if (a2l_iter == addrToLine.begin()) {
	    // we are already at the bottom of our range, there are no lesser addresses
	    cerr << __FILE__ ":" << __LINE__ << ": weird result " << baseAddr +functionSize -1<< endl;
	    return NULL;
	  }

	  a2l_iter--;
	  if ((baseAddr +functionSize -1) < a2l_iter->first) {
	    cerr << __FILE__ ":" << __LINE__ << ": weird result " << baseAddr +functionSize -1<< endl;
	  }
	}

	pdvector<tuple *> *end_addr_tuples = a2l_iter->second;
	if (!end_addr_tuples->size()) {
	  cerr << __FILE__ ":" << __LINE__ << ": no tuples for address " 
	       << baseAddr +functionSize -1<< endl;
	  return NULL;  // this should never happen ??
	}

	//  this next line should probably be end_addr_tuples->end()--, or some such???
	//  in theory there should be only one anyways...
	tuple *end_tuple = (*end_addr_tuples)[0];

	FunctionInfo* fInfo = new FunctionInfo();
	fInfo->startLinePtr = start_tuple;
	fInfo->endLinePtr = end_tuple;
	fInfo->startAddrPtr = start_tuple;
	fInfo->endAddrPtr = end_tuple;
	fInfo->validInfo = true;
	// let's do a check here -- not sure if its needed...
	if (functionInfoHash.defines(functionName)) {
	  cerr << __FILE__ ":" << __LINE__ << ": ERROR:  Duplicate insert! " << endl;
	}
	functionInfoHash[functionName] = fInfo;
	return fInfo;
	
#endif
	return NULL;
}
//insert a mapping from line number to address and address to line number
//to the corresponding structures and updates the records for the function
//given
void FileLineInformation::insertLineAddress(string functionName,
					    unsigned short lineNo,
					    Address codeAddress)
{

	FunctionInfo* fInfo = findFunctionInfo(functionName);
	if (NULL == fInfo) {
	  cerr << __FILE__ << ":" << __LINE__ << ":  cannot insert address for "
	       << functionName << ", no record of this function." << endl;
	  return;
	}
	insertLineAddress(fInfo,lineNo,codeAddress);
}

void FileLineInformation::insertLineAddress(FunctionInfo* fInfo,
					    unsigned short lineNo,
					    Address codeAddress)
{
	if(!fInfo)
		return;

#ifdef OLD_LINE_INFO

	/** this is the maximum of the size of tuples */
	if(size > (unsigned short)0xfffe){
		cerr << "WARNING : Up to " << (unsigned short)0xffff
		     << " lines in a file are recorded for line ifnromation"
		     << endl;
		return;
	}

	if((unsigned)size == tupleCapacity){

		tupleCapacity += TUPLE_REALLOC_INCREMENT;

		if(size == 0){
			lineToAddr = (tuple**)malloc(sizeof(tuple*)*tupleCapacity);
			addrToLine = (tuple**)malloc(sizeof(tuple*)*tupleCapacity);
		}else{
			lineToAddr = (tuple**)realloc(lineToAddr,sizeof(tuple*)*tupleCapacity);
			addrToLine = (tuple**)realloc(addrToLine,sizeof(tuple*)*tupleCapacity);
		}
	}

	//since the entries will come in oreder insertion sort is the best sort here
	//to keep the list sorted and using binary search to find the entries.
	//insertion sort comes here

	tuple* newTuple = new tuple(lineNo,codeAddress);

	int index1; 
	for(index1=size-1;index1>=0;index1--)
		if(lineToAddr[index1]->lineNo > lineNo){
			lineToAddr[index1+1] = lineToAddr[index1];
			lineToAddr[index1+1]->linePtr = (unsigned)(index1+1);
		}
		else break;
	index1++;
	lineToAddr[index1] = newTuple;
	lineToAddr[index1]->linePtr = (unsigned)index1; 

	int index2; 
	for(index2=size-1;index2>=0;index2--)
		if(addrToLine[index2]->codeAddress > codeAddress){
			addrToLine[index2+1] = addrToLine[index2];
			addrToLine[index2+1]->addrPtr = (unsigned)(index2+1);
		}
		else break;

	index2++;
	addrToLine[index2] = newTuple;
	addrToLine[index2]->addrPtr = (unsigned)index2;

	size++;

	if(!fInfo->validInfo){
		fInfo->startLinePtr = lineToAddr[index1];
		fInfo->endLinePtr = lineToAddr[index1];
		fInfo->startAddrPtr = addrToLine[index2];
		fInfo->endAddrPtr = addrToLine[index2];
		fInfo->validInfo = true;
		return;
	}
	if(fInfo->startLinePtr->lineNo > lineNo)
		fInfo->startLinePtr = lineToAddr[index1];

	if(fInfo->endLinePtr->lineNo <= lineNo)
		fInfo->endLinePtr = lineToAddr[index1];

	if(fInfo->startAddrPtr->codeAddress > codeAddress)
		fInfo->startAddrPtr = addrToLine[index2];

	if(fInfo->endAddrPtr->codeAddress <= codeAddress)
		fInfo->endAddrPtr = addrToLine[index2];
#else
	tuple *newTuple = new tuple(lineNo,codeAddress);

	// add tuple to both maps

	l2a_iter_t l2a_iter;
	if (lineToAddr.end() == (l2a_iter = lineToAddr.find(lineNo))) {
	  // not in map yet 
	  pdvector<tuple *> *newTupleMap = new pdvector<tuple *>();
	  newTupleMap->push_back(newTuple);

	  if (newTupleMap != (lineToAddr[lineNo] = newTupleMap)) {
	    cerr << __FILE__ << ":" << __LINE__ << ": broken insert!" << endl;
	  }
#ifdef TIMED_PARSE
	  if (newTupleMap->size() > max_addr_per_line) 
	    max_addr_per_line = newTupleMap->size();
#endif
	}
	else {
	  // in map, get map and add tuple 

	  pdvector<tuple *> *tupleMap = l2a_iter->second;
	  assert(tupleMap);
	  tupleMap->push_back(newTuple);

#ifdef TIMED_PARSE
	  if (tupleMap->size() > max_addr_per_line) 
	    max_addr_per_line = tupleMap->size();
#endif
	}

	a2l_iter_t a2l_iter;
	if (addrToLine.end() == (a2l_iter = addrToLine.find(codeAddress))) {
	  // not in map yet


	  pdvector<tuple *> *newTupleMap = new pdvector<tuple *>();
	  newTupleMap->push_back(newTuple);

	  if (newTupleMap != (addrToLine[codeAddress] = newTupleMap)){
	    cerr << __FILE__ << ":" << __LINE__ << ": broken insert!" << endl;
	  }
#ifdef TIMED_PARSE
	  if (newTupleMap->size() > max_line_per_addr) 
	    max_line_per_addr = newTupleMap->size();
#endif
	}
	else {
	  // in map, get list and add tuple 

	  pdvector<tuple *> *tupleMap = a2l_iter->second;
	  assert(tupleMap);
	  tupleMap->push_back(newTuple);


#ifdef TIMED_PARSE
	  if (tupleMap->size() > max_line_per_addr) 
	    max_line_per_addr = tupleMap->size();
#endif
	}

	// update fInfo
	if(!fInfo->validInfo){
	  fInfo->startLinePtr = newTuple;
	  fInfo->endLinePtr = newTuple;
	  fInfo->startAddrPtr = newTuple;
	  fInfo->endAddrPtr = newTuple;
	  fInfo->validInfo = true;
	  return;
	}
	if(fInfo->startLinePtr->lineNo > lineNo)
		fInfo->startLinePtr = newTuple;

	if(fInfo->endLinePtr->lineNo <= lineNo)
		fInfo->endLinePtr = newTuple;

	if(fInfo->startAddrPtr->codeAddress > codeAddress)
		fInfo->startAddrPtr = newTuple;

	if(fInfo->endAddrPtr->codeAddress <= codeAddress)
		fInfo->endAddrPtr = newTuple;
#endif
}
 
//returns true in case of success, false otherwise.
//this method finds the line number corresponding to an address.
//if name of the function is supplied and isFile is false
//then function level search is being applied. Otherwise file level
//search is applied. If there are more than 1 line found than
//the maximum is being returned.
bool FileLineInformation::getLineFromAddr(string name,unsigned short& lineNo,
					  Address codeAddress,bool isFile,
					  bool isExactMatch)
{
	BPatch_Set<unsigned short> lines;
	if(!getLineFromAddr(name,lines,codeAddress,isFile,isExactMatch))
		return false;
	lineNo = lines.maximum();
	return true;
}

//returns true in case of success, false otherwise.
//this method finds the line numbers corresponding to an address.
//if name of the function is supplied and isFile is false
//then function level search is being applied. Otherwise file level
//search is applied. All the line numbers corresponding to the address
//is returned in a set of addresses.
bool FileLineInformation::getLineFromAddr(string name,
				 	  BPatch_Set<unsigned short>& lines,
					  Address codeAddress,bool isFile,
					  bool isExactMatch)
{
#ifdef OLD_LINE_INFO
  tuple** beginPtr;
  tuple** endPtr;
  int howMany;
  
  if(!addrToLine)
    return false;
  if(isFile){
    beginPtr = addrToLine;
    endPtr = (tuple**) (addrToLine+(size-1));
    howMany = size;
  }
  else{
    FunctionInfo* fInfo = findFunctionInfo(name);
    if(!fInfo)
      return false;
    if(!fInfo->validInfo)
      return false;
    beginPtr = (tuple**)(addrToLine+fInfo->startAddrPtr->addrPtr);
    endPtr = (tuple**)(addrToLine+fInfo->endAddrPtr->addrPtr);
    howMany = (fInfo->endAddrPtr->addrPtr - fInfo->startAddrPtr->addrPtr)+1;
  }
  
  if((codeAddress < (*beginPtr)->codeAddress) || 
     ((*endPtr)->codeAddress < codeAddress))
    return false;
  
  tuple toSearch(0,codeAddress);
  tuple** next=NULL;
  tuple** ret = binarySearchAddrFirst(toSearch,beginPtr,howMany,next);
  if(!ret){
    if(isExactMatch)
      return false;
    if(!next)
      return false;
    ret = next;
    codeAddress = (*ret)->codeAddress;
  }
  do{
    lines += (*ret)->lineNo;
    ret++;
  }while((ret <= endPtr) && ((*ret)->codeAddress == codeAddress));
  
  return true;
#else
  FunctionInfo *fInfo = NULL;

  if (!isFile && (!(fInfo = findFunctionInfo(name)) || (!fInfo->validInfo))) {
    cerr << __FILE__ << __LINE__ << ":  Nonxistent/invalid info for "<< name << endl;
    return false;
  }

  // get list of line numbers corresponding to this address
  if (!addrToLine.size()) {
    cerr << "!!!! addrToLine.size() == 0 !!!!" << endl;
    return false;
  }

  a2l_iter_t a2l_iter;
  if (addrToLine.end() == (a2l_iter = addrToLine.lower_bound(codeAddress))) {
    cerr << "!!!! no lower bound in find! !!!!   addrToLine.size() = "<< addrToLine.size() << endl;
    return false;
  }

  Address closest = a2l_iter->first;
  if (codeAddress != closest) {
    if (isExactMatch) return false;
    if (a2l_iter == addrToLine.begin()) return false;

    // lower_bound produces an iterator w/key that is greater than or equal to the search term.
    // in the case of no match, we want the element that is one less than this, so we have to decrement
    a2l_iter--;
    closest = a2l_iter->first;
  }  

  pdvector<tuple *> *addr_tuples = a2l_iter->second;

  if (isFile) 
    // in case of isFile, just grab them all
    for (unsigned int i = 0; i < addr_tuples->size(); ++i)
      lines += (*addr_tuples)[i]->lineNo;
  
  else {
    Address startAddr = fInfo->startAddrPtr->codeAddress;
    Address endAddr = fInfo->endAddrPtr->codeAddress;
    unsigned int i, n_elem = addr_tuples->size();
    int found = 0;

    for (i = 0; i < n_elem; i++) {
      if ((*addr_tuples)[i]->codeAddress == startAddr) {
	found = 1;
	break;
      }
    }

    if (!found) {
      //  startAddr not in map, this is an error
      cerr << "!!!! startAddress not in map !!!!   addr_tuples->size() = "<< addr_tuples->size() << endl;
      return false;
    }

    int end_found = 0;
    Address currentAddress;
    // accumulate following tuples until endAddr is found
    for (; i < n_elem; i++) {
      currentAddress = (*addr_tuples)[i]->codeAddress;
      if (currentAddress > endAddr) break;
      lines += (*addr_tuples)[i]->lineNo;
      if (currentAddress == endAddr)
	end_found = 1;
    }
    if (!end_found)
      cerr << "!!!! startAddress not in map !!!!   addr_tuples->size() = "<< addr_tuples->size() << endl;
  }
#endif
  return true;
}

//method that retuns set of addresses corresponding to a given line number
//the level of the search is file level if isFile flag is set.
//If isFile level is not set, it seraches in the given function
//in case of success it returns true otherwise false
bool FileLineInformation::getAddrFromLine(string name,
					  BPatch_Set<Address>& codeAddress,
					  unsigned short lineNo,bool isFile,
					  bool isExactMatch)
{
#ifdef OLD_LINE_INFO
	tuple** beginPtr;
	tuple** endPtr;
	int howMany;

	if(!lineToAddr)
		return false;
	if(isFile){
		beginPtr = lineToAddr;
		endPtr = (tuple**) (lineToAddr+(size-1));
		howMany = size;
	}
	else{
		FunctionInfo* fInfo = findFunctionInfo(name);
        	if(!fInfo)
			return false;
		if(!fInfo->validInfo)
			return false;
		beginPtr = (tuple**)(lineToAddr+fInfo->startLinePtr->linePtr);
		endPtr = (tuple**)(lineToAddr+fInfo->endLinePtr->linePtr);
		howMany = (fInfo->endLinePtr->linePtr-fInfo->startLinePtr->linePtr)+1;
	}
	if(!isFile && 
	   (lineNo < (*beginPtr)->lineNo) || ((*endPtr)->lineNo < lineNo))
		return false;

	tuple toSearch(lineNo,0);
	tuple** next=NULL;
	tuple** ret = binarySearchLineFirst(toSearch,beginPtr,howMany,next);
	if(!ret){
		if(isExactMatch)
			return false;
		if(!next)
			return false;
		ret = next;
		lineNo = (*ret)->lineNo;
	}
	do{
		codeAddress += (*ret)->codeAddress;
		ret++;
	}while((ret <= endPtr) && ((*ret)->lineNo == lineNo));
#else

  FunctionInfo *fInfo = NULL;

  if (!isFile && (!(fInfo = findFunctionInfo(name)) || (!fInfo->validInfo))) {
    cerr << __FILE__ << __LINE__ << ":  Nonxistent/invalid info for "<< name << endl;
    return false;
  }
  

  if (! lineToAddr.size()) {
    cerr << __FILE__ << __LINE__ << ":  lineToAddr.size() == 0 !!! " << endl;
  }
  // get list of addressess corresponding to this lineNo
  l2a_iter_t l2a_iter;
  if (lineToAddr.end() == (l2a_iter = lineToAddr.lower_bound(lineNo))) {
    // no lower bound
    cerr << __FILE__ << ":" << __LINE__ << ": no lower bound for " << lineNo << endl;
    cerr << __FILE__ << ":" << __LINE__ << ": valid range is from " 
	 << lineToAddr.begin()->first  << " to " << ((lineToAddr.end()--))->first 
	 << endl;
    return false;
  }
  
  unsigned short closest = l2a_iter->first;
  if (closest != lineNo) {
    if (isExactMatch) return false;
    if (l2a_iter == lineToAddr.begin()) return false;
    l2a_iter--;
    closest = l2a_iter->first;
  }

  // map representing all addresses on this line
  pdvector<tuple *> *line_tuple_map = l2a_iter->second;
  
  if (isFile) {
    unsigned int n_elem = line_tuple_map->size();
    // in case of isFile, just grab them all
    for (unsigned int i = 0; i < n_elem; i++)
      codeAddress += (*line_tuple_map)[i]->codeAddress;
  }
  else {
    Address startAddress = fInfo->startLinePtr->codeAddress;
    Address endAddress = fInfo->endAddrPtr->codeAddress;
    unsigned int i, n_elem = line_tuple_map->size();
    int found = 0;
    
    for (i = 0; i < n_elem; i++) {
      if ((*line_tuple_map)[i]->codeAddress == startAddress) {
	found = 1;
	break;
      }
    }

    if (!found) {
      //  startAddr not in map, this is an error
      cerr << "!!!! startAddress not in map !!!!   addr_tuples->size() = "
	   << line_tuple_map->size() << endl;
      return false;
    }

    int end_found = 0;
    Address currentAddress;
    // accumulate following tuples until endAddr is found
    for (; i < n_elem; i++) {
      currentAddress = (*line_tuple_map)[i]->codeAddress;
      if (currentAddress > endAddress) break;
      codeAddress += (*line_tuple_map)[i]->codeAddress;
      if (currentAddress == endAddress)
	end_found = 1;
    }
    if (!end_found)
      cerr << "!!!! startAddress not in map !!!!   addr_tuples->size() = "<< line_tuple_map->size() << endl;
  }
#endif

	return true;
}
#ifdef OLD_LINE_INFO
bool FileLineInformation::getMinMaxAddress(int n,Address& min,Address& max){
	FunctionInfo* fi = lineInformationList[n];
	if(!fi->validInfo)
		return false;
	min = fi->startAddrPtr->codeAddress;
	max = fi->endAddrPtr->codeAddress;
	return true;
}
#endif

unsigned short FileLineInformation::getFunctionCount(){
#ifdef OLD_LINE_INFO
	return functionCount;
#else
	return functionInfoHash.size();
#endif
}

#ifdef OLD_LINE_INFO
string** FileLineInformation::getFunctionNameList(){
	return functionNameList;
}
FunctionInfo** FileLineInformation::getLineInformationList(){
	return lineInformationList;
}
tuple** FileLineInformation::getLineToAddrMap(){
	return lineToAddr;	
}
tuple** FileLineInformation::getAddrToLineMap(){
	return addrToLine;	
}

#endif




//constructor whose argument is the name of the module name
LineInformation::LineInformation(string mName) 
  : 
#ifdef OLD_LINE_INFO
  sourceFileList(NULL),
  lineInformationList(NULL),
  sourceFileCount(0),
  fileCapacity(0),
#else
  fileLineInfoHash(string::hash),  
#endif
  moduleName(mName)

{}


//desctructor
LineInformation::~LineInformation() {
#ifdef OLD_LINE_INFO
	for(int i=0;i<sourceFileCount;i++){
		delete sourceFileList[i];
		delete lineInformationList[i];
	}
	free(sourceFileList);
	free(lineInformationList);
#else
	dictionary_hash_iter<string, FileLineInformation * > del_iter = fileLineInfoHash.begin();
		
	for(;
	    del_iter != fileLineInfoHash.end(); 
	    ++del_iter) {
	  delete (*del_iter);
	}
	fileLineInfoHash.clear();
#endif
}


#ifndef OLD_LINE_INFO
bool LineInformation::getLineAndFile(unsigned long addr,unsigned short& lineNo,
			    char* fileName,int size)
{
  dictionary_hash_iter<string, FileLineInformation * > find_iter(fileLineInfoHash);
  string fileN;
  FileLineInformation *fInfo;
  while(find_iter.next(fileN, fInfo)) {
    if(fInfo->getLineFromAddr(fileN,lineNo,addr,true,false)){
      if(fileN.length() < (unsigned)size)
	size = fileN.length();
      strncpy(fileName,fileN.c_str(),size);
      fileName[size] = '\0';
      return true;
    }
  }
  // cerr << __FILE__ <<__LINE__ <<":  getLineAndFile could not find address: " << addr 
  //     << "in module: " << moduleName <<endl;
  return false;
}

#endif
/*
short unsigned int LineInformation::getSourceFileCount() 
{
#ifdef OLD_LINE_INFO
  return sourceFileCount;
#else
  return fileLineInfoHash.size();
#endif
}
*/
//method to insert entries for the given file name and function name
//it creates the necessary structures and inserts into the maps
void LineInformation::insertSourceFileName(string functionName,string fileName,
					   FileLineInformation** retFileInfo,
					   FunctionInfo** retFuncInfo)
{
  FileLineInformation *fInfo = NULL;
#ifdef OLD_LINE_INFO
  fInfo = getFileLineInformation(fileName);
  if(!fInfo){

	if((unsigned)sourceFileCount == fileCapacity){

		fileCapacity += FILE_REALLOC_INCREMENT;

		if(sourceFileCount == 0){
			sourceFileList = (string**)malloc(fileCapacity*sizeof(string*));
			lineInformationList = (FileLineInformation**)malloc(fileCapacity*sizeof(FileLineInformation*));
		}else{
			sourceFileList = (string**)realloc(sourceFileList,fileCapacity*sizeof(string*));
			lineInformationList = (FileLineInformation**)realloc(lineInformationList,
							  fileCapacity*sizeof(FileLineInformation*));
		}
	}
    
    unsigned short i = sourceFileCount;
    for(;i > 0;i--){
      if(fileName < *sourceFileList[i-1]){
	sourceFileList[i] = sourceFileList[i-1];
	lineInformationList[i] = lineInformationList[i-1];
      }
      else
	break;
    }
    fInfo = new FileLineInformation(fileName);
    lineInformationList[i] = fInfo;
    sourceFileList[i] = new string(fileName);
    sourceFileCount++;
  }
#else
  char *lname = (char *) fileName.c_str();
  char *sname = strrchr(lname, '/');
  string hash_name_str = string(sname ? sname + 1 : lname);

  if (NULL == (fInfo = getFileLineInformation(hash_name_str))) {
    fInfo = new FileLineInformation(fileName);
    fileLineInfoHash[hash_name_str] = fInfo;
  }
#endif


  FunctionInfo* funcInfo = fInfo->insertFunction(functionName);
  if (NULL == funcInfo)
    cerr << "inser function returned NULL!" << endl;
  if(retFileInfo)
    *retFileInfo = fInfo;
  
  if(retFuncInfo)
    *retFuncInfo = funcInfo;
}

//inserts line to address and address to line mapping to the line info
//object of the given filename. The records for function is also updated
void LineInformation::insertLineAddress(string functionName,
					string fileName,
				        unsigned short lineNo,
			                Address codeAddress)
{
	FileLineInformation* fInfo = getFileLineInformation(fileName);
	if(!fInfo) return;
	fInfo->insertLineAddress(functionName,lineNo,codeAddress);
}

bool LineInformation::getAddrFromLine(BPatch_Set<Address>& codeAddress,
				unsigned short lineNo,
				bool isExactMatch)
{
	FileLineInformation* fInfo = getFileLineInformation(moduleName);
	if(!fInfo){ 
		return false;
	}
	return fInfo->getAddrFromLine(moduleName,codeAddress,
				      lineNo,true,isExactMatch);
}

#ifdef OLD_LINE_INFO
int LineInformation::binarySearch(string fileName){

	int low = 0;
	int high = sourceFileCount-1;
	int mid;
	while(low <= high){
		mid = (low+high)/2;
		string entryName = *sourceFileList[mid];
		if(fileName < entryName)
                        high = mid - 1;
                else if(fileName > entryName)
                        low = mid + 1;
                else
			return (int)mid;
	}
	return -1;
}
#endif

//returns the line information for the given filename
FileLineInformation* LineInformation::getFileLineInformation(string fileName){

#ifdef OLD_LINE_INFO
	int check = binarySearch(fileName);
	if(check >= 0)
		return lineInformationList[check];

	for(int i=0;i<sourceFileCount;i++){
		char* name = new char[sourceFileList[i]->length()+1];
		strncpy(name,sourceFileList[i]->c_str(),
			sourceFileList[i]->length());
		name[sourceFileList[i]->length()] = '\0';
		char* p = strrchr(name,'/');
		if(!p) {
			delete[] name;
			continue;
		}
		p++;
		string sname(p);
		if(fileName == sname){
			delete[] name;
			return lineInformationList[i];
		}
		delete[] name;
	}
#else
	if (fileLineInfoHash.defines(fileName))
	  return fileLineInfoHash[fileName];
	/* linear search if not found is no longer necessary since we store both the */
	/* short and long paths (and already did a preemptive strrchr before calling, right?) */
#endif
	return NULL;
} 

#ifdef DEBUG_LINE_INFO
void FileLineInformation::dump(FILE *f, string *modName)
{
  fprintf(f, "FileLineInformation for module: %s\n", modName);
  dumpFunctionInfo(f);
  dumpAddrToLine(f);
  dumpLineToAddr(f);
}
#endif

//method retuns the file line information object which the function belongs to.
//if there is no entry than it retuns NULL
FileLineInformation* 
LineInformation::getFunctionLineInformation(string functionName)
{

#ifdef OLD_LINE_INFO
  for(int i=0;i<sourceFileCount;i++)
    if(lineInformationList[i]->findFunction(functionName))
      return lineInformationList[i];

#else
  dictionary_hash_iter<string, FileLineInformation * > find_iter(fileLineInfoHash);
  string fileN;
  FileLineInformation *fInfo;

  while(find_iter.next(fileN, fInfo)) 
    if (fInfo->findFunction(functionName))
      return fInfo;

#endif

  cerr << __FILE__ << __LINE__ << ":  No info for " << functionName << endl;
  return NULL;
} 

//method that fills the possible file array with the linenumber objects
//where the function code resides
//return false if error occurs. It fills the array with entries 
//as long as the capacity allows and puts a NULL pointer at the end
//of the array
bool
LineInformation::getFunctionLineInformation(string functionName,
					FileLineInformation* possibleFiles[],int capacity)
{
	capacity--;

	if(!possibleFiles || (capacity <= 0))
		return false;

	possibleFiles[capacity] = NULL;
	int entryCount = 0;

#ifdef OLD_LINE_INFO

	for(int i=0;i<sourceFileCount;i++)
		if(lineInformationList[i]->findFunction(functionName)){
			if(entryCount >= capacity)
				return true;
			possibleFiles[entryCount] = lineInformationList[i];
			entryCount++;
		}

	possibleFiles[entryCount] = NULL;
#else

	dictionary_hash_iter<string, FileLineInformation *> iter(fileLineInfoHash);
	string sName;
	FileLineInformation *flInfo;

	while (iter.next(sName, flInfo))
	  if(flInfo->findFunction(functionName)){
	    if(entryCount >= capacity)
	      return true;
	    possibleFiles[entryCount] = flInfo;
	    entryCount++;
	  }
#endif

	if(!entryCount)
		return false;

	return true;
}

//method that returns true if function belongs to this object
//false otherwise
bool LineInformation::findFunction(string functionName){
	bool ret = false;
#ifdef OLD_LINE_INFO
	for(int i=0;!ret && (i<sourceFileCount);i++)
		ret = lineInformationList[i]->findFunction(functionName);
#else
	dictionary_hash_iter<string, FileLineInformation * > find_iter = fileLineInfoHash.begin();
	for (; find_iter != fileLineInfoHash.end(); ++find_iter) {
	  if ((*find_iter)->findFunction(functionName)) {
	    ret = true; break;
	  }
	}
#endif
	return ret;
}

//delete the records for function
void LineInformation::deleteFunction(string functionName){
#ifdef OLD_LINE_INFO
  for(int i=0;i<sourceFileCount;i++)
    lineInformationList[i]->deleteFunction(functionName);
#else
  dictionary_hash_iter<string, FileLineInformation * > find_iter = fileLineInfoHash.begin();
  for (; find_iter != fileLineInfoHash.end(); ++find_iter) 
    (*find_iter)->deleteFunction(functionName);
#endif
}

//delete the records for function
void LineInformation::cleanEmptyFunctions(){
#ifdef OLD_LINE_INFO
	for(int i=0;i<sourceFileCount;i++)
		lineInformationList[i]->cleanEmptyFunctions();
#else
	dictionary_hash_iter<string, FileLineInformation *> iter(fileLineInfoHash);
	string sName;
	FileLineInformation *flInfo;

	while (iter.next(sName, flInfo))
	  flInfo->cleanEmptyFunctions();
#endif
}

//updates records for a function
void LineInformation::insertFunction(string functionName,Address baseAddr,
				     Address functionSize)
{
  if(findFunction(functionName))
    return;
  FunctionInfo* ret = NULL;
#ifdef OLD_LINE_INFO
  for(int i=0;!ret && (i<sourceFileCount);i++)
    ret = lineInformationList[i]->insertFunction(functionName,
						 baseAddr,functionSize);
#else
  dictionary_hash_iter<string, FileLineInformation * > find_iter = fileLineInfoHash.begin();
  for (; find_iter != fileLineInfoHash.end(); ++find_iter) 
    if ((*find_iter)->insertFunction(functionName, baseAddr,functionSize)) break;
#endif
}

#ifdef OLD_LINE_INFO
string** LineInformation::getSourceFileList(){
	return sourceFileList;
}
FileLineInformation** LineInformation::getLineInformationList(){
	return lineInformationList;
}
#endif

unsigned short LineInformation::getSourceFileCount(){
#ifdef OLD_LINE_INFO
  return sourceFileCount;
#else
  return fileLineInfoHash.size();
#endif
}

#ifdef DEBUG_LINE_INFO
void LineInformation::dump(const char *modName)
{

  if (!modName) return NULL;
  // dump info herein to file
  FILE *dumpfile;
  char dumpname[256];

#ifdef OLD_LINE_INFO
  sprintf(dumpname, "/tmp/%s.old", modName);
#else
  sprintf(dumpname, "/tmp/%s.new", modName);
#endif

  if (NULL == (dumpfile = fopen(dumpname, "w"))) {
    assert(0);
  }

 
#ifdef OLD_LINE_INFO
  fprintf(dumpfile, "moduleName: %s, sourceFileCount %d\n", moduleName.c_str(), sourceFileCount);
  for (unsigned int i = 0; i < sourceFileCount; i++) {
    lineInformationList[i]->dump(dumpfile, sourceFileList[i]);
  }
#else
  fprintf(dumpfile, "moduleName: %s, sourceFileCount %d\n", moduleName.c_str(), fileLineInfoHash.size());
  // first need to sort hash elements so dump produces alphabetical module order
  pdvector<string> source_names;
  string aName;
  FileLineInformation *flInfo;
  dictionary_hash_iter<string, FileLineInformation * > iter(fileLineInfoHash);
  while(iter.next(aName, flInfo)) {
    source_names.push_back(aName);
  }
  source_names.sort();

  for (unsigned int i = 0; i < source_names.size(); i++) {
    fileLineInfoHash[source_names[i]]->dump(dumpfile, &source_names[i]);
  }
#endif

  fclose(dumpfile);

}
#endif
ostream& operator<<(ostream& os,FileLineInformation& linfo){

	cerr << "\tLINE TO ADDRESS \t\t ADDRESS TO LINE:\n";
#ifdef OLD_LINE_INFO
	for(int j=0;j<linfo.size;j++){
		os << ostream::dec << j << "\t";
		os << ostream::dec << linfo.lineToAddr[j]->lineNo << " ----> ";
		os << ostream::hex << linfo.lineToAddr[j]->codeAddress 
		   << "\t\t";
		os << ostream::hex << linfo.addrToLine[j]->codeAddress  
		   << " ----> ";
		os << ostream::dec << linfo.addrToLine[j]->lineNo << "\n";
	}

	for(int i=0;i<linfo.functionCount;i++){
		FunctionInfo* funcinfo = linfo.lineInformationList[i];
		os << "FUNCTION LINE : " << *(linfo.functionNameList[i]) << " : " ;
		if(!funcinfo->validInfo)
			continue;
		os << ostream::dec << funcinfo->startLinePtr->lineNo 
		   << " --- ";
		os << ostream::dec << funcinfo->endLinePtr->lineNo << "\t\t";
		os << ostream::hex << funcinfo->startAddrPtr->codeAddress 
		   << " --- ";
		os << ostream::hex << funcinfo->endAddrPtr->codeAddress 
		   << "\n";
	}
#else
	/*
	dictionary_hash_iter<string, functionInfo *> printIter;
	for (printIter = functionInfoHash.begin(); printIter != functionInfoHash.end(); ++printIter) {
	  FunctionInfo* funcinfo = linfo.lineInformationList[i];
	  os << "FUNCTION LINE : " << *(linfo.functionNameList[i]) << " : " ;
	  if(!funcinfo->validInfo)
	    continue;
	  os << ostream::dec << funcinfo->startLinePtr->lineNo 
	     << " --- ";
	  os << ostream::dec << funcinfo->endLinePtr->lineNo << "\t\t";
	  os << ostream::hex << funcinfo->startAddrPtr->codeAddress 
	     << " --- ";
	  os << ostream::hex << funcinfo->endAddrPtr->codeAddress 
	     << "\n";
	*/
#endif
	return os;
}

#ifndef OLD_LINE_INFO
void LineInformation::print() 
{
  cout <<__FILE__ << ":" << __LINE__ << "Line Information summary:" << endl;
  cout << "Module Name:  " << moduleName << ".  Source File Count: " << fileLineInfoHash.size();
  /*
  dictionary_hash_iter<string, FileLineInformation * > iter = fileLineInfoHash.begin();
  for (; iter != fileLineInfoHash.end(); ++iter) {
    FileLineInformation *finfo = *iter;
    cout << "   source: " << finfo->getFileName() << ".  " << finfo->getFunctionCount() << " functions."
	 << "[" << finfo->a2l_map_size() << ", " << finfo->l2a_map_size() << "]" << endl;
  }
  */
}
#endif

ostream& operator<<(ostream& os,LineInformation& linfo){
#ifdef OLDFILE_INFO
	os << "**********************************************\n";
	os << "MODULE : " << linfo.moduleName << "\n";  
	os << "**********************************************\n";
	for(int i=0;i<linfo.sourceFileCount;i++){
		os << "FILE : " << *(linfo.sourceFileList[i]) << "\n";
		os << *(linfo.lineInformationList[i]);
	}
#endif
	return os;
}

/*
ostream& operator<<(ostream& os,FileLineInformation& linfo){
	cerr << "\tLINE TO ADDRESS \t\t ADDRESS TO LINE:\n";
	for(int j=0;j<linfo.size;j++){
		os << dec << j << "\t";
		os << linfo.lineToAddr[j]->lineNo << " ----> ";
		os << hex << linfo.lineToAddr[j]->codeAddress 
		   << "\t\t";
		os << hex << linfo.addrToLine[j]->codeAddress  
		   << " ----> ";
		os << dec << linfo.addrToLine[j]->lineNo << "\n";
	}
	for(int i=0;i<linfo.functionCount;i++){
		FunctionInfo* funcinfo = linfo.lineInformationList[i];
		os << "FUNCTION LINE : " << *(linfo.functionNameList[i]) << " : " ;
		if(!funcinfo->validInfo){
			os << "INVALID LINE INFO" << endl;
			continue;
		}
		os << dec << funcinfo->startLinePtr->lineNo 
		   << " --- ";
		os << funcinfo->endLinePtr->lineNo << "\t\t";
		os << hex << funcinfo->startAddrPtr->codeAddress 
		   << " --- ";
		os << hex << funcinfo->endAddrPtr->codeAddress 
		   << dec << "\n";
	}
	return os;
}

ostream& operator<<(ostream& os,LineInformation& linfo){
	os << "**********************************************\n";
	os << "MODULE : " << linfo.moduleName << "\n";  
	os << "**********************************************\n";
	for(int i=0;i<linfo.sourceFileCount;i++){
		os << "FILE : " << *(linfo.sourceFileList[i]) << "\n";
		os << *(linfo.lineInformationList[i]);
	}
	return os;
}
*/
