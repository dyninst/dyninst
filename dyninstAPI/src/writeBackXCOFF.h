
/* 	this file contains the class writeBackXCOFF
	which is used to handle the editing of
	XCOFF files for save the world
*/

//typedef unsigned int Address;

#include "XCOFFlib.h"
#include "process.h"



class writeBackXCOFF {


	private:

	XCOFF newFile;
	XCOFF oldFile;

	scnhdr *newSections;
	int numberSections;
	int maxSections;
	char *textTramps;
	unsigned int textTrampsAddr;
	unsigned int textTrampsSize;

	process *mutateeProcess;

	bool debugFlag;	

	int loadFile(XCOFF* file);

	void parseXCOFF(XCOFF *file);

	void updateFilehdr(int offSet);

	int updateScnhdrs(int offSet, int newTextSize);

	struct scnhdr *addSectionHeader(char *newFileCurrent, char* name,
		unsigned int paddr, unsigned int vaddr, int sectionSize, int flags);

	void  copySectionData(int offSet);

	void copyRelocationData(int offSet);

	void copyLineNumberData(int offSet);

	void* copySymbolTableData(int offSet);

	void addNewSectionData(char *newFileCurrent, char* data,int sectionSize, struct scnhdr *newHdr);

//	void expandMoveTextSection(char* newFileStart, int start, int sectionsize, int newTextFilePtr);

	public:
	writeBackXCOFF(char* oldFileName, char* newFileName, bool debugOutputFlag = false,int numbScns=10);
	~writeBackXCOFF();

	bool createXCOFF(); //does all the final accounting to prepare for output
	int addSection(char *name, unsigned int paddr, unsigned int vaddr, 
		int sectionSize, char*data, int flags = 0x0020); //adds a section

	bool outputXCOFF(); //actually does the write
	bool setHeapAddr(unsigned int heapAddr); //sets the address of _end

	void registerProcess(process *p){ mutateeProcess = p;};		
	void attachToText(Address addr, unsigned int size, char* data);
	

};


