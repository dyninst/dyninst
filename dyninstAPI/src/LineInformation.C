#include <stdlib.h>

#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/src/util.h"


//this function implements binary search and returns the found element.
//besides the found element it returns the next bigger line numbered one
//The found element is the first same values element in the array in existence
//of many same values elements.
//in case it is not found it will RETURN NULL. if the next element
//is not available it will assign NULL to the reference.
//the next contains the entry with the first greater line number
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
//this function implements binary search and returns the found element.
//besides the found element it returns the previous smaller address entry
//The found element is the first same values element in the array in existence
//of many same values elements.
//in case it is not found it will RETURN NULL. if the next element
//is not available it will assign NULL to the reference.
//the next contains the entry with the previous smaller address
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

//constructor
FileLineInformation::FileLineInformation(string fileName)
	: sourceFileName(fileName),
	  size(0),
	  lineToAddr(NULL),
	  addrToLine(NULL),
	  functionCount(0),
	  functionNameList(NULL),
	  lineInformationList(NULL),
	  latestSearchResult(-1) {}

//destructor
FileLineInformation::~FileLineInformation(){
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
}

int FileLineInformation::binarySearch(string functionName){
	/*
	if((latestSearchResult >= 0) && (latestSearchResult < functionCount) &&
	   (functionName == *functionNameList[latestSearchResult]))
		return latestSearchResult;
	*/

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
		else {
			latestSearchResult = mid;
			return (int)mid;
		}
	}
	latestSearchResult = -1;
	return -1;
}

//returns the function info structure
FunctionInfo *FileLineInformation::findFunctionInfo(string functionName){
	int check = binarySearch(functionName);
	if( check >= 0 )
		return lineInformationList[check];
	return NULL;
}

//inserts function wntry to the mapping from function name to its info
void FileLineInformation::insertFunction(string functionName){
	FunctionInfo* fInfo = findFunctionInfo(functionName);
	if(!fInfo){
		functionNameList = (string**)(functionCount ? 
			realloc(functionNameList,(functionCount+1)*sizeof(string*)) : 
			malloc(sizeof(string*)));
		lineInformationList = (FunctionInfo**)(functionCount ? 
			realloc(lineInformationList,(functionCount+1)*sizeof(FunctionInfo*)) : 
			malloc(sizeof(FunctionInfo*)));

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
		functionNameList[i] = new string(functionName);
		lineInformationList[i] = new FunctionInfo();
		functionCount++;
	}	
}

//returns true if the function has an entry in the mapping
bool FileLineInformation::findFunction(string functionName){
	int check = binarySearch(functionName);
	return (check >= 0);
}

//delete the records for function
void FileLineInformation::deleteFunction(string functionName){
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
}

//updates records for a function
bool FileLineInformation::insertFunction(string functionName,Address baseAddr,
					 Address functionSize)
{
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
			return false;

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

		functionNameList = (string**)(functionCount ?
			realloc(functionNameList,(functionCount+1)*sizeof(string*)) :
               		malloc(sizeof(string*)));
		lineInformationList = (FunctionInfo**)(functionCount ?
			realloc(lineInformationList,(functionCount+1)*sizeof(FunctionInfo*)) :
			malloc(sizeof(FunctionInfo*)));


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
		return true;
	}
	return false;
}

//insert a mapping from line number to address and address to line number
//to the corresponding structures and updates the records for the function
//given
void FileLineInformation::insertLineAddress(string functionName,
					    unsigned short lineNo,
					    Address codeAddress)
{

	FunctionInfo* fInfo = findFunctionInfo(functionName);
	if(!fInfo){
		cerr << "FATAL ERROR : Something wrong \n";
		return;
	}

	lineToAddr = !lineToAddr ? ((tuple**)malloc(sizeof(tuple*))) :
		     ((tuple**)realloc(lineToAddr,sizeof(tuple*)*(size+1)));
	addrToLine = !addrToLine ? ((tuple**)malloc(sizeof(tuple*))) :
		     ((tuple**)realloc(addrToLine,sizeof(tuple*)*(size+1)));

	//since the entries will come in oreder insertion sort is the best sort here
	//to keep the list sorted and using binary search to find the entries.
	//insertion sort comes here

	tuple* newTuple = new tuple(lineNo,codeAddress);

	short index1; 
	for(index1=size-1;index1>=0;index1--)
		if(lineToAddr[index1]->lineNo > lineNo){
			lineToAddr[index1+1] = lineToAddr[index1];
			lineToAddr[index1+1]->linePtr = (unsigned)(index1+1);
		}
		else break;
	index1++;
	lineToAddr[index1] = newTuple;
	lineToAddr[index1]->linePtr = (unsigned)index1; 

	short index2; 
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

	return true;
}

bool FileLineInformation::getMinMaxAddress(int n,Address& min,Address& max){
	FunctionInfo* fi = lineInformationList[n];
	if(!fi->validInfo)
		return false;
	min = fi->startAddrPtr->codeAddress;
	max = fi->endAddrPtr->codeAddress;
	return true;
}

unsigned short FileLineInformation::getFunctionCount(){
	return functionCount;
}

string** FileLineInformation::getFunctionNameList(){
	return functionNameList;
}

tuple** FileLineInformation::getLineToAddrMap(){
	return lineToAddr;	
}
tuple** FileLineInformation::getAddrToLineMap(){
	return addrToLine;	
}
FunctionInfo** FileLineInformation::getLineInformationList(){
	return lineInformationList;
}

//constructor whose argument is the name of the module name
LineInformation::LineInformation(string mName) 
		: moduleName(mName),
		  sourceFileCount(0),
		  sourceFileList(NULL),
		  lineInformationList(NULL),
	  	  latestSearchResult(-1){}


//desctructor
LineInformation::~LineInformation() {
	for(int i=0;i<sourceFileCount;i++){
		delete sourceFileList[i];
		delete lineInformationList[i];
	}
	free(sourceFileList);
	free(lineInformationList);
}

//method to insert entries for the given file name and function name
//it creates the necessary structures and inserts into the maps
void LineInformation::insertSourceFileName(string functionName,string fileName)
{
	FileLineInformation* fInfo = getFileLineInformation(fileName);
	if(!fInfo){
		sourceFileList = (string**)(sourceFileCount ? 
			realloc(sourceFileList,(sourceFileCount+1)*sizeof(string*)) : 
			malloc(sizeof(string*)));
		lineInformationList = (FileLineInformation**)(sourceFileCount ? 
			realloc(lineInformationList,(sourceFileCount+1)*sizeof(FileLineInformation*)) : 
			malloc(sizeof(FileLineInformation*)));

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
	fInfo->insertFunction(functionName);
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

//returns line number corresponding to the the given address
//if isFile is set the search is file level otherwsie function level.
bool LineInformation::getLineFromAddr(string name,
			    	      unsigned short& lineNo,
				      Address codeAddress,
				      bool isFile,
				      bool isExactMatch)
{
	FileLineInformation* fInfo = NULL;
	if(isFile) 
		fInfo = getFileLineInformation(name);
	else
		fInfo = getFunctionLineInformation(name);
		
	if(!fInfo) return false;
	
	return fInfo->getLineFromAddr(name,lineNo,codeAddress,isFile,isExactMatch);
}

//returns line number corresponding to the the given address
//if isFile is set the search is file level otherwsie function level.
bool LineInformation::getLineFromAddr(string name,
			    	      BPatch_Set<unsigned short>& lines,
				      Address codeAddress,
				      bool isFile,
				      bool isExactMatch)
{
	FileLineInformation* fInfo = NULL;
	if(isFile) 
		fInfo = getFileLineInformation(name);
	else
		fInfo = getFunctionLineInformation(name);
		
	if(!fInfo) return false;
	
	return fInfo->getLineFromAddr(name,lines,codeAddress,isFile,isExactMatch);
}

//returns address corresponding to the the given line 
//if isFile is set the search is file level otherwsie function level.
bool LineInformation::getAddrFromLine(string name,
			    	      BPatch_Set<Address>& codeAddress,
				      unsigned short lineNo,
				      bool isFile,
				      bool isExactMatch)
{
	FileLineInformation* fInfo = NULL;
	if(isFile) 
		fInfo = getFileLineInformation(name);
	else
		fInfo = getFunctionLineInformation(name);
		
	if(!fInfo) return false;

	return fInfo->getAddrFromLine(name,codeAddress,lineNo,isFile,isExactMatch);
}

bool LineInformation::getAddrFromLine(BPatch_Set<Address>& codeAddress,
				      unsigned short lineNo,
				      bool isExactMatch)
{
	FileLineInformation* fInfo = getFileLineInformation(moduleName);
	if(!fInfo){ 
#ifdef DEBUG_LINE
                cerr << "Module " << moduleName << " is not source file name\n";
#endif
		return false;
	}
	return fInfo->getAddrFromLine(moduleName,codeAddress,
				      lineNo,true,isExactMatch);
}

int LineInformation::binarySearch(string fileName){
	/*
	if((latestSearchResult >= 0) && (latestSearchResult < sourceFileCount) &&
	   (fileName == *sourceFileList[latestSearchResult]))
		return latestSearchResult;
	*/

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
                else{
			latestSearchResult = mid;
			return (int)mid;
		}
	}
	latestSearchResult = -1;
	return -1;
}

//returns the line information for the given filename
FileLineInformation* LineInformation::getFileLineInformation(string fileName){

	int check = binarySearch(fileName);
	if(check >= 0)
		return lineInformationList[check];

	for(int i=0;i<sourceFileCount;i++){
		char* name = new char[sourceFileList[i]->length()+1];
		strncpy(name,sourceFileList[i]->string_of(),
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
	return NULL;
} 

//method retuns the file line information object which the function belongs to.
//if there is no entry than it retuns NULL
FileLineInformation* 
LineInformation::getFunctionLineInformation(string functionName){
	for(int i=0;i<sourceFileCount;i++)
		if(lineInformationList[i]->findFunction(functionName))
			return lineInformationList[i];
	return NULL;
} 

//method that returns true if function belongs to this object
//false otherwise
bool LineInformation::findFunction(string functionName){
	bool ret = false;
	for(int i=0;!ret && (i<sourceFileCount);i++)
		ret = lineInformationList[i]->findFunction(functionName);
	return ret;
}

//delete the records for function
void LineInformation::deleteFunction(string functionName){
	for(int i=0;i<sourceFileCount;i++)
		lineInformationList[i]->deleteFunction(functionName);
}

//updates records for a function
void LineInformation::insertFunction(string functionName,Address baseAddr,
				     Address functionSize)
{
	if(findFunction(functionName))
		return;
	bool ret = false;
	for(int i=0;!ret && (i<sourceFileCount);i++)
		ret = lineInformationList[i]->insertFunction(functionName,
						       baseAddr,functionSize);
}

string** LineInformation::getSourceFileList(){
	return sourceFileList;
}

unsigned short LineInformation::getSourceFileCount(){
	return sourceFileCount;
}

FileLineInformation** LineInformation::getLineInformationList(){
	return lineInformationList;
}

ostream& operator<<(ostream& os,FileLineInformation& linfo){
	cerr << "\tLINE TO ADDRESS \t\t ADDRESS TO LINE:\n";
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

