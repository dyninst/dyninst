#include <stdio.h>
#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>

#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_collections.h"
#include "LineInformation.h"

#define NOTYPENAME ""

// XXXX
// This is a hack to deal with the fact that the gnu compiler uses a differnt 
//   style of array records in the coff debug format.  It should be removed
//   once we understand what/why the difference is. - mehmet & jkh 3/10/00
//
extern bool GCC_COMPILED; //True if mutater is compiled with a GNU compiler

class AuxWrap {
  AUXU _auxInfo;
  LDFILE *_ldptr;
  int _index;
  bool _error;

public:
  AuxWrap() { _ldptr = NULL; _index = -1; _error = false; }
  AuxWrap(LDFILE *ldptr, int index) { 
	_ldptr = ldptr; 
	GetAuxInfo(index); 
  }
  pAUXU GetAuxInfo(int index) {
	pAUXU ptr = ldgetaux(_ldptr, index);
	if (!ptr) {
		_error = true;
		return NULL;
	}

	_error = false;
	_index = index;
	_auxInfo = *ptr;
	return &_auxInfo;
  }
  LDFILE *Ldptr() const { return _ldptr; }
  bool GetError() const { return _error; }
  pAUXU GetAuxInfo() { return &_auxInfo; }
  pAUXU operator->() { return &_auxInfo; }
  AuxWrap& operator++() {
	if (_index != -1)
		GetAuxInfo(_index + 1);

	return *this;
  }
  AuxWrap& operator++(int) {
	if (_index != -1)
		GetAuxInfo(_index + 1);

	return *this;
  }
  AuxWrap& operator--() {
	if (_index != -1)
		GetAuxInfo(_index - 1);

	return *this;
  }
  AuxWrap& operator--(int) {
	if (_index != -1)
		GetAuxInfo(_index - 1);

	return *this;
  }
};
  
BPatch_type *ExploreType(BPatch_module *mod, LDFILE *ldptr, char *name, int index, 
					int st_type, bool typeDef = false);

int GetRndxIndex(AuxWrap& auxInfo) {
	return auxInfo->rndx.index;
	// return auxInfo->rndx.rfd;
}

void FindRange(AuxWrap& auxInfo, char*& low, char*& high, bool range_64) {
  char tmp[100];

  sprintf(tmp,"%d",auxInfo->dnLow);
  auxInfo++; //Move forward
  low = new char[strlen(tmp) + 1];
  strcpy(low, tmp);

  //Get the low and high range values
  if (range_64) {
	// Low and high ranges consist of 2 records
	// XXX - Need more work
	++auxInfo;
  }

  sprintf(tmp,"%d",auxInfo->dnHigh);
  auxInfo++; //Move forward
  high = new char[strlen(tmp) + 1];
  strcpy(high, tmp);

  if (range_64) {
     	// Low and high ranges consist of 2 records
	// XXX - Need more work
	++auxInfo;
  }
}

BPatch_type *HandleTQ(char *name, unsigned int tq, 
				BPatch_type *prevType, AuxWrap& auxInfo) {
  if (tq == tqNil)
	return NULL; //Nothing to do

  int low, high;
  BPatch_type *newType = NULL;

  switch(tq) {
  case tqRef:
	newType = new BPatch_type(name, -1, BPatch_referance, prevType);
	break;

  case tqPtr:
  case tqFar: //Ptr type qualifier
	newType = new BPatch_type(name, -1, BPatch_pointer, prevType);
	break;

  case tqArray:
  case tqArray_64: //Array handling
	//Skip Rndx and Get the low bound
	auxInfo++; //Move forward
	if (GCC_COMPILED)
		auxInfo++; //An extra move forward
	low = auxInfo->dnLow;
	if (tq == tqArray_64)
		auxInfo++; //Ignore high order bits
	auxInfo++; //Move forward
	high = auxInfo->dnHigh;
	auxInfo++; //Move forward
	if (tq == tqArray_64)
		auxInfo++; //Ignore high order bits

	auxInfo++; //Move forward, skip width of the element type
	newType = new BPatch_type(name, -1, BPatch_array, prevType, low, high);
	break;
  }

  return newType;
}

void FindFields(BPatch_module *mod, BPatch_type *mainType, AuxWrap& auxInfo) {
//Find the member vars and functions of the mainType (Can be either class, union or struct)

  SYMR symbol;
  char *name;
  int offset = 0;
  BPatch_type *memberType = NULL;
  LDFILE *ldptr = auxInfo.Ldptr();
  int index = GetRndxIndex(auxInfo);
  
  auxInfo++; //Move forward

  if (ldtbread(ldptr, index++, &symbol) != SUCCESS)
	return;

  if ( !(symbol.st == stBlock && symbol.sc == scInfo) ) {
/*
  if ( symbol.st != stBlock ) {
  fprintf(stderr, "Invalid block begining-> symbol.st = %d, symbol.sc = %d\n", 
							 symbol.st, symbol.sc);
*/
	return; //Invalid symbol is reached
	// index--;
  }

  //Find out all the member types
  while(1) {
  	if (ldtbread(ldptr, index++, &symbol) != SUCCESS)
		return;

	if (symbol.st == stEnd)
		break;

	name = ldgetname(ldptr, &symbol);
	if (mainType->getDataClass() == BPatch_enumerated)
		mainType->addField(name, BPatch_scalar, symbol.value);
	else {
		memberType = ExploreType(mod, ldptr, NOTYPENAME, symbol.index, symbol.st);
		if (memberType) {
			mainType->addField(name, BPatch_scalar, 
						memberType, offset, memberType->getSize());
			offset += memberType->getSize() * 8;
		}
	}
  } 
}

BPatch_type *ExploreType(BPatch_module *mod, LDFILE *ldptr, char *name, int index, 
					int st_type, bool typeDef=false) {
  SYMR symbol;
  char *typeName;
  BPatch_type *lastType, *newType = NULL;
  int width = 0;

  AuxWrap auxInfo(ldptr, index);
  if (auxInfo.GetError())
	return NULL; //Invalid index or ldptr is sent

  if (st_type == stProc || st_type == stStaticProc) {
	auxInfo++; //Ignore isym, move to next aux record
	index++;
  }

  newType = mod->moduleTypes->findType(index);
  if (!typeDef && newType)
	return newType; //This type is already inserted

  //Store the main TIR
  AuxWrap mainTir = auxInfo;

  auxInfo++; //Move forward

  if (mainTir->ti.fBitfield) {
	//Next TIR contains width info
	width = (auxInfo->width) / 8;
	auxInfo++; //Move forward
  }

  switch(mainTir->ti.bt) {
	case btEnum:
	case btClass:
	case btStruct:
	case btUnion:
		//Find the name
		if ( !typeDef && (ldtbread(ldptr, GetRndxIndex(auxInfo), &symbol) == SUCCESS) )
			typeName = ldgetname(ldptr, &symbol);
		else
			typeName = name;

		switch(mainTir->ti.bt) {
		case btEnum:
			lastType = new BPatch_type(typeName, -1, BPatch_enumerated);
			break;
		case btClass:
			lastType = new BPatch_type(typeName, -1, BPatch_class, width);
			break;
		case btStruct:
			lastType = new BPatch_type(typeName, -1, BPatch_structure, width);
			break;
		case btUnion:
			lastType = new BPatch_type(typeName, -1, BPatch_union, width);
			break;
		}

		//Field parsing
		FindFields(mod, lastType, auxInfo);
		break;

	case btRange:
	case btRange_64: {
		char *low, *high;
		FindRange(auxInfo, low, high, auxInfo->ti.bt == btRange_64);
		lastType = new BPatch_type(name, -1, BPatchSymTypeRange, low, high);
		break;
	}

	case btTypedef:
		if (ldtbread(ldptr, GetRndxIndex(auxInfo), &symbol) == SUCCESS)
			typeName = ldgetname(ldptr, &symbol);
		else
			typeName = name;

		lastType = ExploreType(mod, ldptr, typeName, symbol.index, -1);
		auxInfo++; //Move forward
		break;

	case btIndirect:
		//Rndx points to an entry in the aux symbol table that contains a TIR
		lastType = ExploreType(mod, ldptr, name, GetRndxIndex(auxInfo), -1);
		auxInfo++; //Move forward
		break;

	case btSet:
	case btProc:
	// case btDecimal: COBOL: packed/unpacked decimal
	// case btFixedBin: COBOL: Fixed binary
	case btAdr32:
 	case btComplex:
 	case btDComplex:
  	case btFixedDec:
 	case btFloatDec:
 	case btString:
 	case btBit:
 	case btPicture:
 	case btPtrMem:
 	// case btScaledBin: COBOL: Scaled Binary
 	case btVptr:
 	// case btArrayDesc: FORTRAN 90: Array descriptor
 	case btAdr64:
 	// case btAdr: Same as btAdr64
 	case btChecksum:
		//Nothing to do!
		lastType = NULL;
		break;

	case btNil: //Regarded as void
 	case btVoid:
		if (typeDef)
			lastType = new BPatch_type(name, -11, BPatch_built_inType, 0);
		else 
			lastType = new BPatch_type("void", -11, BPatch_built_inType, 0);
		break;
	case btChar:
		if (typeDef)
			lastType = new BPatch_type(name, -2, BPatch_built_inType, 1);
		else
			lastType = new BPatch_type("char", -2, BPatch_built_inType, 1);
		break;
	case btUChar:
		if (typeDef)
			lastType = new BPatch_type(name, -5, BPatch_built_inType, 1);
		else
			lastType = new BPatch_type("unsigned char", -5, BPatch_built_inType, 1);
		break;
	case btShort:
		if (typeDef)
			lastType = new BPatch_type(name, -3, BPatch_built_inType, 2);
		else
			lastType = new BPatch_type("short", -3, BPatch_built_inType, 2);
		break;
 	case btUShort:
		if (typeDef)
			lastType = new BPatch_type(name, -7, BPatch_built_inType, 2);
		else
			lastType = new BPatch_type("unsigned short", -7, BPatch_built_inType, 2);
		break;
 	case btInt32:
 	// case btInt: Same as btInt32
		if (typeDef)
			lastType = new BPatch_type(name, -1, BPatch_built_inType, 4);
		else
			lastType = new BPatch_type("int", -1, BPatch_built_inType, 4);
		break;
 	case btUInt32:
 	// case btUInt: Same as btUInt32
		if (typeDef)
			lastType = new BPatch_type(name, -8, BPatch_built_inType, 4);
		else
			lastType = new BPatch_type("unsigned int", -8, BPatch_built_inType, 4);
		break;
 	case btLong32:
		if (typeDef)
			lastType = new BPatch_type(name, -4, BPatch_built_inType, sizeof(long));
		else
			lastType = new BPatch_type("long", -4, BPatch_built_inType, sizeof(long));
		break;
 	case btULong32:
		if (typeDef)
			lastType = new BPatch_type(name, -10, BPatch_built_inType, 
									sizeof(unsigned long));
		else
			lastType = new BPatch_type("unsigned long", -10, BPatch_built_inType, 
									sizeof(unsigned long));
		break;
 	case btFloat:
		if (typeDef)
			lastType = new BPatch_type(name, -12, BPatch_built_inType, sizeof(float));
		else
			lastType = new BPatch_type("float", -12, BPatch_built_inType, 
									sizeof(float));
		break;
 	case btDouble:
		if (typeDef)
			lastType = new BPatch_type(name, -13, BPatch_built_inType, 
									sizeof(double));
		else
			lastType = new BPatch_type("double", -13, BPatch_built_inType, 
									sizeof(double));
		break;
 	case btLong64:
 	// case btLong: Same as btLong64
 	case btLongLong64:
 	// case btLongLong: Same as btLongLong64
 	case btInt64:
		if (typeDef)
			lastType = new BPatch_type(name, -32, BPatch_built_inType, 8);
		else
			lastType = new BPatch_type("long long", -32, BPatch_built_inType, 8);
		break;
 	case btULong64:
 	// case btULong: Same as btULong64
 	case btULongLong64:
 	// case btULongLong: Same as btULongLong64
 	case btUInt64:
		if (typeDef)
			lastType = new BPatch_type(name, -33, BPatch_built_inType, 8);
		else
			lastType = new BPatch_type("unsigned long long", -33, 
								BPatch_built_inType, 8);
		break;
 	case btLDouble:
		if (typeDef)
			lastType = new BPatch_type(name, -14, BPatch_built_inType, 
								sizeof(long double));
		else
			lastType = new BPatch_type("long double", -14, BPatch_built_inType, 
								sizeof(long double));
		break;
 	case btInt8:
 	case btUInt8:
		if (typeDef)
			lastType = new BPatch_type(name, -27, BPatch_built_inType, 1);
		else
			lastType = new BPatch_type("integer*1", -27, BPatch_built_inType, 1);
		break;

	default:
		//Never to reach here!
		lastType = NULL;
		break;
  } //switch

  if (!lastType)
	return NULL; //Something wrong!

  //Handle type qualifiers
  newType = lastType;

  while(1) {
  	for(int tq=0; newType != NULL && tq < itqMax; tq++) {
		switch(tq) {
		case 0:
			newType = HandleTQ(name, mainTir->ti.tq0, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		case 1:
			newType = HandleTQ(name, mainTir->ti.tq1, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		case 2:
			newType = HandleTQ(name, mainTir->ti.tq2, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		case 3:
			newType = HandleTQ(name, mainTir->ti.tq3, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		case 4:
			newType = HandleTQ(name, mainTir->ti.tq4, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		case 5:
			newType = HandleTQ(name, mainTir->ti.tq5, lastType, auxInfo);
			if (newType)
				lastType = newType;
			break;
		}
  	}

	//Is this enough?
	if (mainTir->ti.continued)
		mainTir = auxInfo; //Type qualifiers continue in the next TIR
	else
		break;
  }

  newType = mod->moduleTypes->findType(index + 10000);
  if (!typeDef || !newType)
  	lastType->setID(index + 10000);

  //Add type definition
  mod->moduleTypes->addType(lastType);

  return lastType;
}

int GetOffset(LDFILE *ldptr, char *name, int varType, int value) {
  SYMR symbol;
  PDR pdr;
  int fcnIndex = 0;
  int res;

  while ((res = ldgetpd(ldptr, fcnIndex++, &pdr)) == SUCCESS) {
	if (ldtbread(ldptr, pdr.isym, &symbol) == FAILURE) 
		continue;

	string funcName = ldgetname(ldptr, &symbol);
	if (funcName == name)
		break;
  }

  if (res != SUCCESS) {
	cout << "Warning: Function " << name << " is not found in the procedure table!" << "\n";
	return -1;
  }

  if (varType == stLocal)
	return (pdr.frameoffset - pdr.localoff + value);
  else
	return (pdr.frameoffset - 48 + value);
}

void FindLineInfo(BPatch_module *mod, LDFILE *ldptr, 
			const string& modName, LineInformation*& lineInformation)
{

  pCHDRR symbolTable = SYMTAB(ldptr);
  char *ptr;
  int i, j;

  BPatch_function  *fp;

  for(i=0;i<symbolTable->cfd;i++){
	/*get the relevant file header */
	pCFDR fileDesc = symbolTable->pcfd+i;

	/* look at the file name if it the same name as the module keep it */
	char* csf = NULL;
	if(fileDesc->pss && (fileDesc->pfd->rss != -1))
		csf = fileDesc->pss+fileDesc->pfd->rss;
	else continue;

	/* as always being done get rid of the full path. To me we should have kept it*/
	ptr = strrchr(csf, '/');	
	if(ptr)
		ptr++;

	string* currentSourceFile = new string(ptr ? ptr : csf);
	extern string* processDirectories(string*);
	currentSourceFile = processDirectories(currentSourceFile);

	/* if it is the same name with the module name then parse lines */
	if(*currentSourceFile == modName){
		/* for each procedure entry look at the information */
		for(j=0;j<fileDesc->pfd->cpd;j++){
			pPDR procedureDesc = fileDesc->ppd+j;

			/* no name is available for proc */
			if(!fileDesc->psym || !fileDesc->pfd->csym ||
			   (procedureDesc->isym == -1))
				continue;
			
			/* get the name of the procedure if available */
			pSYMR procSym = fileDesc->psym+procedureDesc->isym;
			string currentFunctionName(fileDesc->pss + procSym->iss);
			lineInformation->insertSourceFileName(currentFunctionName,
							      *currentSourceFile);

			/* no line information for the filedesc is available */
			if(!fileDesc->pfd->cline || !fileDesc->pline || 
			   (procedureDesc->iline == -1))
				continue;

			/* get the base address of the procedure */
			fp = mod->findFunction(currentFunctionName.string_of());
			if(!fp) continue;

			unsigned long currentFunctionBase = (unsigned long)(fp->getBaseAddr());

			int linerIndex = 0;
			unsigned long instByteCount = 0;
			
			/* find the right entry in the liner array */
			for(linerIndex = procedureDesc->iline;
			    linerIndex < fileDesc->pfd->cline; linerIndex++)
				if(fileDesc->pline[linerIndex] == procedureDesc->lnLow)
					break;

			int currentLine = -1;
			/* while the line belongs to this function insert it */
			for(;linerIndex < fileDesc->pfd->cline; linerIndex++){

				if((fileDesc->pline[linerIndex] > procedureDesc->lnHigh)||
				   (fileDesc->pline[linerIndex] < procedureDesc->lnLow))
					break;
				
				if(currentLine != fileDesc->pline[linerIndex]){
					currentLine = fileDesc->pline[linerIndex];

					lineInformation->insertLineAddress(
						currentFunctionName,
						*currentSourceFile,
						fileDesc->pline[linerIndex],
						instByteCount + currentFunctionBase);
				}

				instByteCount += sizeof(unsigned);
			}
		}
	}
  }
}

//Main function to parse stab strings
extern char *parseStabString(BPatch_module *, int linenum, char *str, 
	  int fPtr, BPatch_type *commonBlock = NULL);
extern char *current_func_name;

//Stab definitions (from gdb)
#define CODE_MASK 0x8F300
#define ECOFF_IS_STAB(sym) (((sym)->index & 0xFFF00) == CODE_MASK)
#define ECOFF_MARK_STAB(code) ((code)+CODE_MASK)
#define ECOFF_UNMARK_STAB(code) ((code)-CODE_MASK)
#define STABS_SYMBOL "@stabs"

//Main fcn to construct type information
void parseCoff(BPatch_module *mod, char *exeName, const string& modName,
	       LineInformation* lineInformation)
{
  char *ptr, name[4096]; //Symbol name
  char curr_fname[256];
  LDFILE *ldptr = NULL;
  long index=0;
  SYMR symbol;
  
  BPatch_function  *fp;
  BPatch_type *ptrType = NULL;
  BPatch_localVar *locVar = NULL;

  if ( !(ldptr = ldopen(exeName, ldptr)) )
	return;

  if (LDSWAP(ldptr))
	return; //Bytes are swapped

  string module = "DEFAULT_MODULE";
  int moduleEndIdx = -1;

  //Find the boundries
  while (ldtbread(ldptr, index++, &symbol) == SUCCESS) {
        ptr = ldgetname(ldptr, &symbol);

	if (!ptr) 
		continue;

	if (strlen(ptr) >= sizeof(name)) {
	    printf("name overflow: %s\n", name);
	    abort();
	} 

	strcpy(name, ptr);

	if (symbol.st == stFile) {
	    //Disregard the full path name...
	    ptr = strrchr(name, '/');
	    if (ptr)
		ptr++;
	    else
		ptr = name;

	    module = ptr;
	    moduleEndIdx = symbol.index;

	    if (module == modName) 
		break;
	}
	else if (symbol.st == stEnd) {
	    if (index == moduleEndIdx)
	    	module = "DEFAULT_MODULE";
	}
	else if (module == "DEFAULT_MODULE") {
	    break; //We are in the global var section
	}
  }

  if (module != modName) {
	ldclose(ldptr);
	return; //We did not find the correct module
  }

  /* get the symbol table of the executable to access fields */
  FindLineInfo(mod, ldptr, modName, lineInformation);

  bool stabFormat = false; //Flag to check debug format
  //Check whether the first symbol is @stabs or not?
  if (ldtbread(ldptr, index, &symbol) != SUCCESS) {
        ldclose(ldptr);
        return;
  }

  ptr = ldgetname(ldptr, &symbol);
  if (ptr && !strcmp(ptr, "@stabs")) {
        stabFormat = true;
        index++; //Move to next record
  }

  current_func_name = NULL;

  //Process the symbol table
  while (ldtbread(ldptr, index++, &symbol) == SUCCESS) {
        ptr = ldgetname(ldptr, &symbol);

	if (!ptr)
		continue;

	if (strlen(ptr) >= sizeof(name)) {
	    printf("name overflow: %s\n", name);
	    abort();
	} 

	strcpy(name, ptr);

	if (stabFormat && ECOFF_IS_STAB (&symbol)) {
		while (name[strlen(name)-1] == '\\') {
			if (ldtbread(ldptr, index++, &symbol) != SUCCESS) {
				ldclose(ldptr);
				return;
			}
			ptr = ldgetname(ldptr, &symbol);
			name[strlen(name)-1] = '\0';
			strcat(name, ptr);
		}

		int typeCode = ECOFF_UNMARK_STAB (symbol.index);
		if (typeCode == 138)
			typeCode = 128; //Why?
		else if (typeCode == 42)
			typeCode = 32; //Why?
		else if (typeCode == 46)
			typeCode = 36; //Why?
		else if (typeCode == 170)
			typeCode = 160; //Why?

		switch (typeCode) {
		case 0x64:  // N_SO
		case 0x84:  // N_SOL
			current_func_name = NULL; // reset for next object file
			continue;
			
		case 32:    // Global symbols -- N_GYSM
		case 36:    // functions and text segments -- N_FUN
		case 128:   // typedefs and variables -- N_LSYM
		case 160:   // parameter variable -- N_PSYM
			int value = symbol.value;
			if ( ((typeCode == 128) || (typeCode == 160)) && current_func_name ) {
				int varType = stLocal;
				char *p = strchr(name, ':');
				if (!p)
					continue; //Not stab format!
				p++;
				if (*p == 'p')
					varType = stParam;
				value = GetOffset(ldptr, current_func_name, varType, value);
			}

			char *temp = parseStabString(mod, 0, name, value);
			if (*temp) {
				//Error parsing the stabstr, return should be \0
				fprintf(stderr, "Stab string parsing ERROR!! More to parse: %s\n", temp);
			}
			break;
		}
		continue;
	}
	else if (stabFormat || ECOFF_IS_STAB (&symbol)) 
		continue;

	switch(symbol.st) {
	case stFile:
		ldclose(ldptr);
		return; //Module symbols have finished

	case stProc:
        case stStaticProc:
		if (symbol.sc == scText) {
			strcpy(curr_fname, name);
			fp = mod->findFunction(curr_fname);
			if (fp) {
				ptrType = ExploreType(mod, ldptr, NOTYPENAME, symbol.index, symbol.st);
				if (ptrType)
					fp->setReturnType(ptrType);
			}
		}
              	break;

        case stGlobal:
	case stConstant:
	case stStatic:
		switch(symbol.sc) {
		case scData:
		case scSData:
		case scBss:
		case scSBss:
		case scRData:
		case scRConst:
		case scTlsData:
		case scTlsBss:
			ptrType = ExploreType(mod, ldptr, NOTYPENAME, symbol.index, symbol.st);
			if (ptrType)
				mod->moduleTypes->addGlobalVariable(name, ptrType);
			break;
		}
		break;

        case stLocal:
	case stParam:
		switch(symbol.sc) {
		case scAbs:
		case scVar: 
		case scRegister:
		case scVarRegister:

			if (curr_fname) {
				int value;

				switch(symbol.sc) {
				case scAbs:
				case scVar: 
					value = GetOffset(ldptr, curr_fname,
								symbol.st, symbol.value);
					break;
				case scRegister:
				case scVarRegister:
					value = symbol.value;
					break;
				}

				fp = mod->findFunction(curr_fname);
				if (fp && value != -1) {
					ptrType = ExploreType(mod, ldptr, NOTYPENAME, 
								symbol.index, symbol.st);
					if (ptrType) {
						//How do we get linenum
						locVar = new BPatch_localVar(name, ptrType, 
									-1, value, symbol.sc);
						if (symbol.st == stParam) {
							fp->funcParameters->addLocalVar(locVar);
							fp->addParam(name, ptrType, 
									-1, value, symbol.sc);
						}
						else
							fp->localVariables->addLocalVar(locVar);
					}
				}
			}
			break;

		case scData:
		case scSData:
		case scBss:
		case scSBss:
		case scRConst:
		case scRData:
			if (symbol.st == stParam)
				; //Parameter is static var. Don't know what to do
			break;
		}
		case scUnallocated:
			break; //Nothing to do

		break;

	case stTypedef:
	case stBase: //Base class
	case stTag: //C++ class, structure or union
		if (symbol.sc == scInfo)
			ptrType = ExploreType(mod, ldptr, name, symbol.index, symbol.st, true);
		break;

	default:
		break;

	} //switch
  } //while

  //Close the file
  ldclose(ldptr);
}
