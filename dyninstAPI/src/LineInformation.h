#ifndef _LineInformation_h_
#define _LineInformation_h_

#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "BPatch_Set.h"

typedef struct tuple {
	unsigned short lineNo;
	Address codeAddress;
	unsigned short linePtr;
	unsigned short addrPtr;

	tuple(unsigned short l,Address a): lineNo(l),codeAddress(a) {}

} tuple;

typedef struct FunctionInfo {
	bool validInfo;
	tuple* startLinePtr;
	tuple* endLinePtr;
	tuple* startAddrPtr;
	tuple* endAddrPtr;

	FunctionInfo() : validInfo(false) {}
} FunctionInfo;

//the following class declarations keep the line information for modules.

/** this class contains array of tuples for mapping from line number to address
  * and for address to line number. These two arrays are sorted in increasing order
  * besides keeping the arrays it contains a mapping from function name to FunctionInfo
  * structure to keep track of the starting and ending points of the information for a 
  * function. This class represents the line information for a file in a module,
  * and functions declared in that file.
  */
class FileLineInformation{
	friend class BPatch_thread;
	friend class LineInformation;
	friend ostream& operator<<(ostream&,FileLineInformation&);

	/** structure to keep the mapping from line number to adress
	  * or vice versa
	  */
public:
private:
	/** structure to keep the line information for each function in the module
	  */

	/** name of the source file this class represents */
	string sourceFileName;

	/** number of entries in the mapping array */
	unsigned short size;

	/** array of tuple structure sorted according to lineNo field */
	tuple** lineToAddr;

	/** array of tuple structure sorted according to codeAddress field */
	tuple** addrToLine;

	/** a mapping from function name to the structure FunctionInfo */
	unsigned short functionCount;
	string** functionNameList;
	FunctionInfo** lineInformationList;
	
public:
	/** constructor
	  * @param fileName name of the source file this object is created */
	FileLineInformation(string fileName);


	string getFileName() { return sourceFileName; }

	/** destructor */
	~FileLineInformation();

	/** method that returns function info for a given function 
	  * @param functionName name of the function
	  */
	FunctionInfo* findFunctionInfo(string functionName);

	/** method that inserts a function entry to the map 
	  * @param functionName function name to insert 
	  */
	void insertFunction(string functionName);

	/** method that checks the mapping for existence of the function */
	bool findFunction(string functionName);

	/** method that inserts a line information entry to the array of
	  * tuples and updates the info for function given.
	  * @param functionName name of the function to update info for
	  * @param lineNo line number information
	  * @param codeAddress address corresponding to the line number
	  */
	void insertLineAddress(string functionName,
			       unsigned short lineNo,Address codeAddress);

	/** method that finds the line number corresponding to a given
	  * address. In case line number is not found it returns 
	  * false, otherwise true. This function searches in two scopes.
	  * if isFile, then the line number info is searched in file level,
	  * otherwise in function level. If there are more than 1 line info 
	  * for the address then the maximum is taken
	  * @param name name of the function
	  * @param lineNo line number to return
	  * @param codeAddress address given to search
	  * @param isFile flag that defines the scope of search
	  * @param isExactMatch flag defining whether exact match is searched
	  */
	bool getLineFromAddr(string name,unsigned short& lineNo,
			     Address codeAddress,bool isFile=false,
			     bool isExactMatch=true);

	/** as is in previous explanation except set of lines is returned
	  * instead of a single line number */
	bool getLineFromAddr(string name,BPatch_Set<unsigned short>& lines,
			     Address codeAddress,bool isFile=false,
			     bool isExactMatch=true);

	/** method that returns set of addresses for a given line number
	  * the search can be done in file level or in function level 
  	  * if the address is found in the map it return true
	  * otherwise it returns false.
	  * @param name name of the function
	  * @param codeAddress address set to return
	  * @param lineNo line number togiven to search 
	  * @param isFile flag that defines the scope of search
	  * @param isExactMatch flag defining whether exact match is searched
	  */
	bool getAddrFromLine(string name,BPatch_Set<Address>& codeAddress,
			     unsigned short lineNo,bool isFile=false,
			     bool isExactMatch=true);

	/** tempoprary method to be deleted in commit */
	void print();
};

/** class which contains a mapping from file names to the line information
  * object for those file names.
  */
class LineInformation {
private:
	friend class BPatch_thread;
	friend ostream& operator<<(ostream&,LineInformation&);

	/** name of the module this object belongs to */
	string moduleName;

	/** mapping from source file name to line info object for that
	  * file. If the module has more than 1 source file than it 
	  * will contain entry for each source file. The file names are
	  * inserted as given by stab info ( it may be full path or only
	  * file name).
	  */
	unsigned short sourceFileCount;
	string** sourceFileList;
	FileLineInformation** lineInformationList;

public:

	/** constructor 
	  * @param mName module name
	  */
	LineInformation(string mName) ;
	
	/** destructor */
	~LineInformation(); 

	/** method to insert an entry to the line info for a file.
	  * inserts an entry to the mapping from file name to line
	  * info object for the given function. The latest one always replaces the
	  * previous ones.
	  * @param functionName function name to insert
	  * @param fileName file name which the function belongs */
	void insertSourceFileName(string functionName,string fileName);

	/** method that inserts a line information entry to corresponding
	  * file line information object for the given file name and
	  * function.
	  * @param functionName name of the function
	  * @param fileName name of the source file function belongs
	  * @param lineNo line number information
	  * @param codeAddress address corresponding to the line number
	  */
	void insertLineAddress(string functionName,string fileName,
			       unsigned short lineNo,Address codeAddress);

	/** method that finds the line number corresponding to a given
	  * address. In case line number is not found it returns 
	  * false, otherwise true. This function searches in two scopes.
	  * if isFile, then the line number info is searched in file level,
	  * otherwise in function level. If there are more than 1 line info 
	  * for the address then the maximum is taken
	  * @param name name of the function/file
	  * @param lineNo line number to return
	  * @param codeAddress address given to search
	  * @param isFile flag that defines the scope of search
	  * @param isExactMatch flag defining whether exact match is searched
	  */
	bool getLineFromAddr(string name,unsigned short& lineNo,
			     Address codeAddress,bool isFile=false,
			     bool isExactMatch=true);

	/** the same as previous except it returns all corresponding 
	  * lines for the given address */
	bool getLineFromAddr(string name,BPatch_Set<unsigned short>& lines,
			     Address codeAddress,bool isFile=false,
			     bool isExactMatch=true);

	/** method that returns set of addresses for a given line number
	  * the search can be done in file level or in function level 
  	  * if the address is found in the map it return true
	  * otherwise it returns false.
	  * @param name name of the function or file
	  * @param codeAddress address set to return
	  * @param lineNo line number togiven to search 
	  * @param isFile flag that defines the scope of search
	  * @param isExactMatch flag defining whether exact match is searched
	  */
	bool getAddrFromLine(string name,BPatch_Set<Address>& codeAddress,
			     unsigned short lineNo,bool isFile=false,
			     bool isExactMatch=true);

	/** method that returns set of addresses corresponding to a line number
	  * for the file which is name of the module
	  * In failure it returns false
	  */
	bool getAddrFromLine(BPatch_Set<Address>& codeAddress,unsigned short lineNo,
			     bool isExactMatch=true);

	/** method that returns the line info object for a given file name 
  	  * It returns NULL if the entry does not exist.
	  * @param fileName name of the source file
	  */
	FileLineInformation* getFileLineInformation(string fileName);

	/** method that returns the line info object for the file 
	  * for a given function name
	  * @param functionName name of the function being looked for
	  */
	FileLineInformation* getFunctionLineInformation(string functionName);

	/** method that checks the existence of a function in the line info object */
	bool findFunction(string functionName);


	/** tempoprary method to be deleted in commit */
	void print();
};

#endif /*_LineInformation_h_*/
