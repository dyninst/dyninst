/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: CodeView.C,v 1.12 2002/12/20 07:49:56 jaw Exp $

#include <assert.h>

#include <iostream.h>
#include <iomanip.h>

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/CodeView.h"
#include "dyninstAPI/src/NTTypes.h"


//---------------------------------------------------------------------------
// CodeView methods
//---------------------------------------------------------------------------

bool
CodeView::Parse( void )
{
	DWORD i;


	// verify the CodeView signature
	// currently, we only support the NB11 format
	if( strncmp( pBase, "NB11", 4 ) != 0 )
	{
		// indicate that we do not understand this CodeView format
		return false;
	}

	// obtain access to the subsection directory
	SDHeader* pSDHdr = (SDHeader*)(pBase + *(DWORD*)(pBase + 4));

	// parse the subsections, extracting the information we need
	for( i = 0; i < pSDHdr->cDir; i++ )
	{
		SDEntry* pEntry = 
			(SDEntry*)(((char*)pSDHdr) +
                pSDHdr->cbDirHeader + i * pSDHdr->cbDirEntry);

		switch( pEntry->sst )
		{
		case sstModule:
			ParseModuleSubsection( pEntry );
			break;

		case sstLibraries:
			ParseLibrariesSubsection( pEntry );
			break;

		case sstAlignSym:
			ParseAlignSymSubsection( pEntry );
			break;

		case sstSrcModule:
			ParseSrcModuleSubsection( pEntry );
			break;

		case sstGlobalPub:
		case sstStaticSym:
		case sstGlobalSym:
			ParseSymbolsWithHeaderSubsection( pEntry );
			break;

		default:
			// it is a subsection type we do not care about - skip it
			break;
		}
	}

	return true;
}



void
CodeView::ParseModuleSubsection( SDEntry* pEntry )
{
	// ensure that the modules pdvector contains an element for this module
	// recall that the modules pdvector uses a one-based index
	if( pEntry->iMod >= modules.size() )
	{
		modules.resize( pEntry->iMod + 1 );
	}

	// make a new entry for this module, starting with this module subsection
	modules[pEntry->iMod] =
        Module((ModuleSubsection*)(pBase + pEntry->offset), textId );
}


void
CodeView::ParseLibrariesSubsection( SDEntry* pEntry )
{
	const char* curr = pBase + pEntry->offset;

	while( curr < (pBase + pEntry->offset + pEntry->cb) )
	{
		// add an entry for this library
		libs.push_back(curr);

		// advance to the next string
		curr += (*curr + 1);
	}
}


void
CodeView::ParseSymbolsWithHeaderSubsection( SDEntry* pEntry )
{
	SymHeader* pSymHdr = (SymHeader*)(pBase + pEntry->offset);

	syms.Parse( ((char*)pSymHdr) + sizeof(SymHeader), pSymHdr->cbSymbol );
}


void
CodeView::ParseAlignSymSubsection( SDEntry* pEntry )
{
	// associate this subsection with the correct module
	Module& mod = modules[pEntry->iMod];
	mod.pas = (AlignSymSubsection*)(pBase + pEntry->offset);

	// parse the symbols represented in this section
	mod.syms.Parse( (const char*)mod.pas, pEntry->cb );
}


void
CodeView::ParseSrcModuleSubsection( SDEntry* pEntry )
{
	// associate this subsection with the correct module
	Module& mod = modules[pEntry->iMod];
	mod.psrc = (SrcModuleSubsection*)(pBase + pEntry->offset);
}

#ifdef BPATCH_LIBRARY

#include "LineInformation.h"

/**
 * @param srcModuleTable the pointer to the beginning of the srcModuleSection 
 *		         of the debug information. This is the address where 
 * 			 the line information table can be found in the exec.
 * @param baseAddr	 the information for line numbers is given relative to
 *			 beginning of the module. That is for a line number the
 *			 information that can be obtained is the relative addres
 *			 with respect to module so we pass also the start addres
 *			 of the code in the module.
 * @param lineInformation lineInformation object of ours
 * 
 * !!!!!! IMPORTANT !!!!!!!
 * This function only generates the data structures for source files and
 * the mappings lineNumber <-> lineAddress in the source files. It does not
 * relate the functions to the lines or files yet. Actual functions are related
 * to data structure in CreateTypeAndLineInfo function.
 */

void
CodeView::CreateLineInfo( const char* srcModuleTable, DWORD baseAddr ,
                          LineInformation* lineInformation)
{
	SrcModuleSubsection* psrc = (SrcModuleSubsection*)srcModuleTable;

	//get the number of segments in file to skip some fields
	WORD segInModule = psrc->cSeg;
	WORD fileInModule = psrc->cFile;

	//cerr << "NUMBER OF FILES " << fileInModule << "\n";

	//process each file in the module
	for(WORD fileE=0 ; fileE < fileInModule ; fileE++){

		//get the offset and claculate the address for the file record
		DWORD fileEOffset = 
			*(DWORD*)(srcModuleTable + (1 + fileE)*sizeof(DWORD));
		const char* fileEAddress = srcModuleTable + fileEOffset;
		SrcModuleSubsection::FileInfo* fileI = 
			(SrcModuleSubsection::FileInfo*)fileEAddress;

		//get the number of segments in  this file record
		WORD segInFile = fileI->cSegFile;

		//get the pointer for the file name and then create string
		const char* ptr = 
			fileEAddress + sizeof(DWORD)+ 3*segInFile*sizeof(DWORD);
		LPString currentSourceFile(ptr);

		//cerr << "FILE NAME : " <<  (string)currentSourceFile << "\n";
		lineInformation->insertSourceFileName(string("___tmp___"),
						      (string)currentSourceFile);

		for(WORD segmentE = 0; segmentE < segInFile; segmentE++){
			//calculate the segment table offset and the address
			//for the table
			DWORD segmentEOffset = 
				*(DWORD*)(fileEAddress+ (1+segmentE)*sizeof(DWORD));
			const char* segmentEAddress = 
				srcModuleTable + segmentEOffset;

			//find how many pais exist in the table for line numbers
			WORD pairInSegment = 
				*(WORD*)(segmentEAddress + sizeof(WORD));

			//calculate the starting addresses for parallel arrays
			const char* lineOffsetAddress = 
					segmentEAddress + sizeof(DWORD); 
			const char* lineNumberAddress = 
				lineOffsetAddress + sizeof(DWORD)*pairInSegment;

			//for each pair (number,address) mapping insert to
			//line number data structure
			for(WORD pairE = 0; pairE < pairInSegment; pairE++){
				DWORD lineOffset = *(DWORD*)(lineOffsetAddress + pairE*sizeof(DWORD));
				WORD lineNumber = *(WORD*)(lineNumberAddress + pairE*sizeof(WORD));
				//cerr << "LINE : " << lineNumber << " -- "
				//     << hex << (lineOffset + baseAddr) 
				//     <<  " : " << lineOffset << dec << "\n";
				lineInformation->insertLineAddress(
					string("___tmp___"),
					(string)currentSourceFile,
					lineNumber,lineOffset + baseAddr);
			}
		}
	}
	//since at this point no function info is available we used
	//a temporary function to insert the data but later this
	//function is not going to be used so we delete the entry for
	//this function.

	if(fileInModule)
		lineInformation->deleteFunction(string("___tmp___"));
}

//
// Create type information for a specific module
//
void
CodeView::CreateTypeAndLineInfo( BPatch_module *inpMod , DWORD baseAddr ,
				 LineInformation* lineInformation)
{
        DWORD i;

        // verify the CodeView signature
        // currently, we only support the NB11 format
        if( strncmp( pBase, "NB11", 4 ) != 0 )
        {
                // indicate that we do not understand this CodeView format
                return;
        }

        // obtain access to the subsection directory
        SDHeader* pSDHdr = (SDHeader*)(pBase + *(DWORD*)(pBase + 4));

	//Find inp module name
	char inpModName[1024];
        inpMod->getName(inpModName, 1024);
        char *ptr = strrchr(inpModName, '.');
        if (ptr)
        	*ptr = '\0';

	Module *mod = NULL;
	SDEntry *alignSubSec = NULL;
	int iMod = -1;
	TypesSubSection *pTypeBase = NULL;
	SrcModuleSubsection* lineData = NULL; //to check line info is available

        // parse the subsections, extracting the information we need
        for( i = 0; i < pSDHdr->cDir; i++ )
        {
                SDEntry* pEntry =
                        (SDEntry*)(((char*)pSDHdr) +
                pSDHdr->cbDirHeader + i * pSDHdr->cbDirEntry);

                switch( pEntry->sst )
                {
                case sstModule: 
		{
			ModuleSubsection *pmod = (ModuleSubsection *) 
							(pBase + pEntry->offset);

			LPString lpsName ( ((char*)pmod) + sizeof(ModuleSubsection) +
                			pmod->cSeg * sizeof( ModuleSubsection::SegInfo ) );

                	char modName[1024];
                	strcpy(modName, ((string)lpsName).c_str());
                	ptr = strrchr(modName, '.');
                	if (ptr)
                        	*ptr = '\0';

                	if ( strcmp(modName, inpModName) == 0 ) {
				// We found correct module
				mod = new Module(pmod, textId);
				iMod = pEntry->iMod;
                	}

                        break;
		}

                case sstAlignSym:
        		// check whether this subsection is in the correct module
			if (mod && (iMod == pEntry->iMod) ) {
				mod->pas = (AlignSymSubsection*)(pBase + pEntry->offset);
				alignSubSec = pEntry;
			}
                        break;

                case sstGlobalTypes:
			pTypeBase = (TypesSubSection *) (pBase + pEntry->offset);
                        break;

		case sstSrcModule:
			if(mod && (iMod == pEntry->iMod)){ 
				lineData = (SrcModuleSubsection*)(pBase + pEntry->offset);
				mod->psrc = lineData ;
			}
			break;
                default:
                        // it is a subsection type we do not care about - skip it
                        break;
                }
        }

	if (!mod)
		return; //We could not find the input module

	//if the debug section do not contain the line info section for 
	//the module we return witha warning.
	if(lineData)
		CreateLineInfo((const char*)(mod->psrc),baseAddr,
			       lineInformation);
	else
		cerr << "WARNING : CodeView::CreateTypeAndLineInfo"
		     << " can not create Line Information for : "
		     << inpModName << ".\n";

	mod->syms.CreateTypeInfo( (const char *)mod->pas, alignSubSec->cb,
					pTypeBase, inpMod,lineInformation);
}
#endif // BPATCH_LIBRARY

//---------------------------------------------------------------------------
// CodeView::Symbols methods
//---------------------------------------------------------------------------


void
CodeView::Symbols::Parse( const char* pSymBase, DWORD cb )
{
	// scan the symbols linearly
	SymRecord* curr = (SymRecord*)pSymBase;
	while( ((char*)curr) < (pSymBase + cb) )
	{
		// handle the symbol record
		switch( curr->index )
		{
		case S_LPROC32:
			// add the entry to our list of functions
			lprocs.push_back(( (SymRecordProc*)curr ));
			break;

		case S_GPROC32:
			// add the entry to our list of functions
			gprocs.push_back(( (SymRecordProc*)curr ));
			break;

		case S_LDATA32:
			// add the entry to our list of variables
			lvars.push_back(( (SymRecordData*)curr ));
			break;

		case S_GDATA32:
			// add the entry to our list of variables
			gvars.push_back(( (SymRecordData*)curr ));
			break;

		case S_LABEL32:
			labels.push_back(( (SymRecordLabel*)curr ));
			break;

		case S_BPREL32:
			bprels.push_back(( (SymRecordBPRel*)curr ));
			break;

		case S_THUNK32:
			thunks.push_back(( (SymRecordThunk*)curr ));
			break;

		case S_END:
			// nothing to do for end markers
			break;

		case S_ALIGN:
			// skip padding records
			break;

        case S_PUB32:
            pubs.push_back((SymRecordData*)curr);
            break;

		default:
			// it is a record type that we do not care about
			break;
		}

		// advance to the next symbol
		curr = (SymRecord*)(((char*)curr) + curr->length + sizeof(WORD));
	}
}



CodeView::Symbols&
CodeView::Symbols::operator=( const CodeView::Symbols& syms )
{
	if( &syms != this )
	{
		gprocs = syms.gprocs;		// global functions
		lprocs = syms.lprocs;		// local functions
		gvars = syms.gvars;			// global variables
		lvars = syms.lvars;			// local variables
		bprels = syms.bprels;		// stack variables
		labels = syms.labels;		// labels
		thunks = syms.thunks;		// thunks (code outside of functions)
        pubs = syms.pubs;           // public (catch-all) symbols
	}
	return *this;
}

#ifdef BPATCH_LIBRARY
//
// Creates type information for the symbols in the specified module
//
void
CodeView::Symbols::CreateTypeInfo( const char* pSymBase, DWORD cb, 
                                TypesSubSection *pTypeBase, BPatch_module *mod ,
				LineInformation* lineInformation)
{
	char currFuncName[1024];
	char symName[1024];
	int len;
	int fparam = 0;

	BPatch_function  *fp = NULL;
	BPatch_type *ptrType = NULL;
	BPatch_localVar *locVar = NULL;

	//Initialize required member variables
	char *startAddr = ((char *)pTypeBase->offType) + pTypeBase->cType * sizeof(DWORD);

	// scan the symbols linearly
	SymRecord* curr = (SymRecord*)pSymBase;
	while( ((char*)curr) < (pSymBase + cb) )
	{
		// handle the symbol record
		switch( curr->index )
		{
		case S_LPROC32:
		case S_GPROC32:
			len = (int) ( (SymRecordProc*)curr )->name[0];
			strncpy(currFuncName, &( (SymRecordProc*)curr )->name[1], len);
			currFuncName[len] = '\0';

			//Find function in the module
			fp = mod->findFunction(currFuncName);
			if (fp) {
				lineInformation->insertFunction(
					string(currFuncName),
					(Address)(fp->getBaseAddr()),
					fp->getSize());
				DWORD offset = pTypeBase->offType[((SymRecordProc*)curr )->procType - 0x1000];
				TypeRec *trec = (TypeRec *)(startAddr + offset);
				if (trec->leaf == LF_PROCEDURE)
					fparam = ((ProcTypeRec *)trec)->parms;
				else if (trec->leaf == LF_MFUNCTION) {
					BPatch_type *thisType = ExploreType(mod,
						((MFuncTypeRec *)trec)->thisType,
						pTypeBase, startAddr);
					if (thisType && 
						!strcmp(thisType->getName(), "void"))
						fparam = ((MFuncTypeRec *)trec)->parms;
					else
						fparam = ((MFuncTypeRec *)trec)->parms + 1;
				}

				ptrType = ExploreType(mod, ((ProcTypeRec *)trec)->rvtype, 
								pTypeBase, startAddr);
				if (ptrType)
					fp->setReturnType(ptrType);
			}
			break;

		case S_LDATA32:
		case S_GDATA32:
		case S_PUB32:
			// add the entry to our list of variables
			len = (int) ( (SymRecordData*)curr )->name[0];
			strncpy(symName, &( (SymRecordData*)curr )->name[1], len);
			symName[len] = '\0';
			ptrType = ExploreType(mod, ( (SymRecordData*)curr )->type,
								pTypeBase, startAddr);
			if (ptrType)
				mod->moduleTypes->addGlobalVariable(symName, ptrType);
			break;

		case S_BPREL32:
			len = (int) ( (SymRecordBPRel*)curr )->name[0];
			strncpy(symName, &( (SymRecordBPRel*)curr )->name[1], len);
			symName[len] = '\0';
			if (fp && ( (SymRecordBPRel*)curr )->offset) {
				ptrType = ExploreType(mod, ( (SymRecordBPRel*)curr )->type,
								pTypeBase, startAddr);
				if (ptrType) {
					locVar = new BPatch_localVar(symName, ptrType, -1, 
							((SymRecordBPRel*)curr )->offset);
					if (fparam) {
						fp->funcParameters->addLocalVar(locVar);
						fp->addParam(symName, ptrType, -1, 
							((SymRecordBPRel*)curr )->offset);
						fparam--;
					}
					else {
						fp->localVariables->addLocalVar(locVar);
					}
				}
			}
			break;

		case S_CONSTANT:
		{
			// Constant vars

			char *name;
			if ( ((SymRecordCons *)curr)->value < LF_NUMERIC )
				name = ((SymRecordCons *)curr)->name;
			else
				name = (char *)&((SymRecordCons *)curr)->value +
								sizeof(DWORD);

			len = (int)name[0];
			strncpy(symName, &name[1], len);
			symName[len] = '\0';

			ptrType = ExploreType(mod, ( (SymRecordCons *)curr )->type,
								pTypeBase, startAddr);
			if (ptrType)
				mod->moduleTypes->addGlobalVariable(symName, ptrType);
			break;
		}

		case S_UDT:
			// Typedefs
			break;

		default:
			// it is a record type that we do not care about
			break;
		}

		// advance to the next symbol
		curr = (SymRecord*)(((char*)curr) + curr->length + sizeof(WORD));
	}
}

//
// Mapping from CodeView types to DyninstAPI types
//

BPatch_type *
CodeView::Symbols::CreatePrimitiveType(DWORD index)
{
	BPatch_type *lastType;

	switch(index) {
	case T_NOTYPE: //Regarded as void
 	case T_VOID:
		lastType = new BPatch_type("void", -11, BPatch_built_inType, 0);
		break;
	case T_PVOID:
	case T_PFVOID:
	case T_PHVOID:
	case T_32PVOID:
	case T_32PFVOID:
	case T_64PVOID:
		lastType = new BPatch_type("void", -11, BPatch_built_inType, 0);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
	case T_CHAR:
	case T_RCHAR:
		lastType = new BPatch_type("char", -2, BPatch_built_inType, 1);
		break;
	case T_PCHAR:
	case T_PFCHAR:
	case T_PHCHAR:
	case T_32PCHAR:
	case T_32PFCHAR:
	case T_64PCHAR:
	case T_PRCHAR:
	case T_PFRCHAR:
	case T_PHRCHAR:
	case T_32PRCHAR:
	case T_32PFRCHAR:
	case T_64PRCHAR:
		lastType = new BPatch_type("char", -2, BPatch_built_inType, 1);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
                break;
	case T_UCHAR:
		lastType = new BPatch_type("unsigned char", -5, BPatch_built_inType, 1);
		break;
	case T_PUCHAR:
	case T_PFUCHAR:
	case T_PHUCHAR:
	case T_32PUCHAR:
	case T_32PFUCHAR:
	case T_64PUCHAR:
		lastType = new BPatch_type("unsigned char", -5, BPatch_built_inType, 1);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
                break;
	case T_SHORT:
	case T_INT2:
		lastType = new BPatch_type("short", -3, BPatch_built_inType, 2);
		break;
	case T_PINT2:
	case T_PFINT2:
	case T_PHINT2:
	case T_32PINT2:
	case T_32PFINT2:
	case T_64PINT2:
	case T_PSHORT:
	case T_PFSHORT:
	case T_PHSHORT:
	case T_32PSHORT:
	case T_32PFSHORT:
	case T_64PSHORT:
		lastType = new BPatch_type("short", -3, BPatch_built_inType, 2);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_USHORT:
	case T_UINT2:
		lastType = new BPatch_type("unsigned short", -7, BPatch_built_inType, 2);
		break;
	case T_PUINT2:
	case T_PFUINT2:
	case T_PHUINT2:
	case T_32PUINT2:
	case T_32PFUINT2:
	case T_64PUINT2:
	case T_PUSHORT:
	case T_PFUSHORT:
	case T_PHUSHORT:
	case T_32PUSHORT:
	case T_32PFUSHORT:
	case T_64PUSHORT:
		lastType = new BPatch_type("unsigned short", -7, BPatch_built_inType, 2);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_INT4:
		lastType = new BPatch_type("int", -1, BPatch_built_inType, 4);
		break;
	case T_PINT4:
	case T_PFINT4:
	case T_PHINT4:
	case T_32PINT4:
	case T_32PFINT4:
	case T_64PINT4:
		lastType = new BPatch_type("int", -1, BPatch_built_inType, 4);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_UINT4:
		lastType = new BPatch_type("unsigned int", -8, BPatch_built_inType, 4);
		break;
	case T_PUINT4:
	case T_PFUINT4:
	case T_PHUINT4:
	case T_32PUINT4:
	case T_32PFUINT4:
	case T_64PUINT4:
		lastType = new BPatch_type("unsigned int", -8, BPatch_built_inType, 4);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_LONG:
		lastType = new BPatch_type("long", -4, BPatch_built_inType, sizeof(long));
		break;
	case T_PLONG:
	case T_PFLONG:
	case T_PHLONG:
	case T_32PLONG:
	case T_32PFLONG:
	case T_64PLONG:
		lastType = new BPatch_type("long", -4, BPatch_built_inType, sizeof(long));
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_ULONG:
		lastType = new BPatch_type("unsigned long", -10, BPatch_built_inType, 
								sizeof(unsigned long));
		break;
	case T_PULONG:
	case T_PFULONG:
	case T_PHULONG:
	case T_32PULONG:
	case T_32PFULONG:
	case T_64PULONG:
		lastType = new BPatch_type("unsigned long", -10, BPatch_built_inType, 
								sizeof(unsigned long));
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_REAL32:
		lastType = new BPatch_type("float", -12, BPatch_built_inType, 
									sizeof(float));
		break;
	case T_PREAL32:
	case T_PFREAL32:
	case T_PHREAL32:
	case T_32PREAL32:
	case T_32PFREAL32:
	case T_64PREAL32:
		lastType = new BPatch_type("float", -12, BPatch_built_inType,
									sizeof(float));
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_REAL64:
		lastType = new BPatch_type("double", -13, BPatch_built_inType, 
									sizeof(double));
		break;
	case T_PREAL64:
	case T_PFREAL64:
	case T_PHREAL64:
	case T_32PREAL64:
	case T_32PFREAL64:
	case T_64PREAL64:
		lastType = new BPatch_type("double", -13, BPatch_built_inType, 
									sizeof(double));
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
 	case T_INT8:
 	case T_UINT8:
		lastType = new BPatch_type("long long", -32, BPatch_built_inType, 8);
		break;
	case T_PINT8:
	case T_PUINT8:
	case T_PFINT8:
	case T_PFUINT8:
	case T_PHINT8:
	case T_PHUINT8:
	case T_32PINT8:
	case T_32PUINT8:
	case T_32PFINT8:
	case T_32PFUINT8:
	case T_64PINT8:
	case T_64PUINT8:
		lastType = new BPatch_type("long long", -32, BPatch_built_inType, 8);
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
	case T_REAL80:
		lastType = new BPatch_type("long double", -14, BPatch_built_inType, 
								sizeof(long double));
		break;
	case T_PREAL80:
	case T_PFREAL80:
	case T_PHREAL80:
	case T_32PREAL80:
	case T_32PFREAL80:
	case T_64PREAL80:
		lastType = new BPatch_type("long double", -14, BPatch_built_inType,
								sizeof(long double));
		lastType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
	default:
		lastType = NULL;
	}

	return lastType;
}

BPatch_visibility AccessType(WORD attribute) {
	switch(attribute & 0x3) {
	case 1:
		return BPatch_private;
	case 2:
		return BPatch_protected;
	case 3:
		return BPatch_public;
	}

	return BPatch_visUnknown;
}

void
CodeView::Symbols::FindFields(BPatch_module *mod, BPatch_type *mainType, int count,
				DWORD index, TypesSubSection *pTypeBase, char *startAddr)
{
	BPatch_type *memberType = NULL;
	char fname[1024], *name;
	int foffset;
	int fvalue;

	//Find field list
	DWORD offset = pTypeBase->offType[index - 0x1000];
	TypeRec *trec = (TypeRec *)(startAddr + offset);

	if (trec->leaf != LF_FIELDLIST) {
		printf("Invalid field list entry!\n");
		return;
	}

	// printf("Number of fields: %d\n", count);

	unsigned char *ptr = ((unsigned char *)trec) + sizeof(TypeRec);
	for(int i=0; i<count; ++i) {
		switch( *((WORD *)ptr) ) {
		case LF_MEMBER:
			if ( ((LFMember *)ptr)->offset < LF_NUMERIC ) {
				foffset = ((LFMember *)ptr)->offset;
				name = ((LFMember *)ptr)->name;
			}
			else {
				// Need to find offset
				foffset = -1;
				name = ( (char *) &((LFMember *)ptr)->offset) +
									sizeof(DWORD);
			}
					
			strncpy(fname, &name[1], (int)name[0]);
			fname[(int)name[0]] = '\0';
			memberType = ExploreType(mod, ((LFMember *)ptr)->type, 
								pTypeBase, startAddr);
			if (memberType == NULL)
				memberType = mod->moduleTypes->findType("void");

			mainType->addField(fname, BPatch_scalar, memberType, foffset*8, 
						memberType->getSize(), 
						AccessType(((LFMember *)ptr)->attribute) );

			// printf("Field %d is LF_MEMBER %s\n", i, fname);

			// Move to the next record
			ptr = ((unsigned char *)name) + (int)name[0] + 1;
			break;

		case LF_ENUMERATE:
			if ( ((LFEnumerate *)ptr)->value < LF_NUMERIC ) {
				fvalue = ((LFEnumerate *)ptr)->value;
				name = ((LFEnumerate *)ptr)->name;
			}
			else {
				// Need to find value
				fvalue = -1;
				name = ( (char *) &((LFEnumerate *)ptr)->value) +
									sizeof(DWORD);
			}

			strncpy(fname, &name[1], (int)name[0]);
			fname[(int)name[0]] = '\0';
			mainType->addField(name, BPatch_scalar, fvalue);

			// printf("Field %d is LF_ENUMERATE %s\n", i, fname);

			ptr = ((unsigned char *)name) + (int)name[0] + 1;
			break;

		case LF_BCLASS:
		{
			//Find base class type identifier
			int baseID = ((LFBclass *)ptr)->type;

			//Find offset of the base class
			if ( ((LFBclass *)ptr)->offset < LF_NUMERIC ) {
				foffset = ((LFBclass *)ptr)->offset;
				ptr += sizeof(LFBclass);
			}
			else {
				//Need to calculate offset
				foffset = 0;
				ptr = (unsigned char *)&((LFBclass *)ptr)->offset +
								sizeof(DWORD);
			}
				
			//Find base class
			BPatch_type *baseCl = mod->moduleTypes->findType(baseID);
			if (baseCl && (baseCl->getDataClass() == BPatch_structure) ) {

				//Get field descriptions of the base type
				BPatch_Vector<BPatch_field *> *baseClFields = 
								baseCl->getComponents();
				for(int fieldNum=0; fieldNum < baseClFields->size(); 
									fieldNum++) 
				{
					BPatch_field *field = (*baseClFields)[fieldNum];

					if (field->getVisibility() == BPatch_private)
						continue; //Can not add this member

					mainType->addField(field->getName(),
						field->getTypeDesc(),
						field->getType(),
						foffset*8 + field->getOffset(),
						field->getVisibility());
				}
			}

			// printf("Field %d is LF_BCLASS\n", i);

			break;
		}

		case LF_ONEMETHOD:
			// printf("LF_ONEMETHOD attribute %d\n", ((LFOnemethod *)ptr)->attribute );
			if ( ((LFOnemethod *)ptr)->attribute & 0x10 ) {
				//This is an introducing virtual method.
				name = ((LFOnemethod *)ptr)->name + sizeof(DWORD);
			}
			else
				name = ((LFOnemethod *)ptr)->name;

			strncpy(fname, &name[1], (int)name[0]);
			fname[(int)name[0]] = '\0';
			memberType = ExploreType(mod, ((LFOnemethod *)ptr)->type,
								pTypeBase, startAddr);

			if (memberType == NULL)
				memberType = mod->moduleTypes->findType("void");

			mainType->addField(fname, BPatch_dataMethod, memberType, 0, 0);

			// printf("Field %d is LF_ONEMETHOD %s\n", i, fname);

			ptr = (unsigned char *)name + (int)name[0] + 1;
			break;

		case LF_METHOD:
			strncpy(fname, &((LFMethod *)ptr)->name[1],
						(int)((LFMethod *)ptr)->name[0]);
			fname[(int)((LFMethod *)ptr)->name[0]] = '\0';
			memberType = mod->moduleTypes->findType("void");
			mainType->addField(fname, BPatch_dataMethod, memberType, 0, 0);
			ptr = (unsigned char *)((LFMethod *)ptr)->name + 
						(int)((LFMethod *)ptr)->name[0] + 1;

			// printf("Field %d is LF_METHOD %s\n", i, fname);

			break;

		case LF_STMEMBER:
			//Do not know what to do! Just skip this
			strncpy(fname, &((LFStmember *)ptr)->name[1],
						(int)((LFStmember *)ptr)->name[0]);
			fname[(int)((LFStmember *)ptr)->name[0]] = '\0';

			// printf("Field %d is LF_STMEMBER %s\n", i, fname);

			ptr = (unsigned char *)((LFStmember *)ptr)->name +
					(int)((LFStmember *)ptr)->name[0] + 1;
			break;
		case LF_VBCLASS:
		case LF_IVBCLASS:
			// Do not know what to do! Just skip it
			if ( ((LFVbclass *)ptr)->vbpoff < LF_NUMERIC ) 
				ptr = (unsigned char *)&((LFVbclass *)ptr)->vbpoff +
					sizeof(WORD);
			else
				ptr = (unsigned char *)&((LFVbclass *)ptr)->vbpoff +
					sizeof(DWORD);

			if ( *((WORD *)ptr) < LF_NUMERIC ) 
				ptr += sizeof(WORD);
			else
				ptr += sizeof(DWORD);
			
			// printf("Field %d is LF_LF_VBCLASS or LF_IVBCLASS\n", i);
			break;

		case LF_VFUNCTAB:
			// Do not know what to do! Just skip it.
			// printf("Field %d is LF_VFUNCTAB\n", i);

			ptr += sizeof(LFVfunctab);

			break;

		case LF_FRIENDFCN:
			printf("LF_FRIENDFCN is not handled!\n");
			break;
		case LF_INDEX:
			printf("LF_INDEX is not handled!\n");
			break;
		case LF_NESTTYPE:
			printf("LF_NESTTYPE is not handled!\n");
			break;
		case LF_FRIENDCLS:
			printf("LF_FRIENDCLS is not handled!\n");
			break;
		case LF_VFUNCOFF:
			printf("LF_VFUNCOFF is not handled!\n");
			break;
		case LF_NESTTYPEEX:
			printf("LF_NESTTYPEEX is not handled!\n");
			break;
		case LF_MEMBERMODIFY:
			printf("LF_MEMBERMODIFY is not handled!\n");
			break;
		default:
			// printf("Unknown leaf index in field %d %x\n", i, *((WORD *)ptr));
			return;
		}

		//Check for padding bytes
		while (*ptr > LF_PAD0) 
			ptr += (*ptr - LF_PAD0);
	}
}

BPatch_type *
CodeView::Symbols::ExploreType(BPatch_module *mod, DWORD index, 
                               TypesSubSection *pTypeBase, char *startAddr)
{
	BPatch_type *lastType, *newType = NULL;
	char typeName[1024];

	//First check whether or not this type was created before
	newType = mod->moduleTypes->findType(index);
	if (newType)
		return newType;

	if (index < 0x1000) {
		// This is a primitive type
		newType = CreatePrimitiveType(index);
		//Add type definition
		if (newType) {
			newType->setID(index);
			mod->moduleTypes->addType(newType);
		}
		return newType;
	}


	//Handle non-primitive type
	DWORD offset = pTypeBase->offType[index - 0x1000];
	TypeRec *trec = (TypeRec *)(startAddr + offset);

	switch(trec->leaf) {
	case LF_MODIFIER:
		newType = ExploreType(mod, ((LFModifier *)trec)->index, 
							pTypeBase, startAddr);
		break;
	case LF_POINTER:
		lastType = ExploreType(mod, ((LFPointer *)trec)->type,
							pTypeBase, startAddr);
		if (lastType)
			newType = new BPatch_type("", -1, BPatch_pointer, lastType);
		break;
	case LF_ARRAY:
		lastType = ExploreType(mod, ((LFArray *)trec)->elemtype,
							pTypeBase, startAddr);
		if (lastType) {
			int arrLen;
			char *arrName;

			if ( ((LFArray *)trec)->length < LF_NUMERIC ) {
				arrLen = ((LFArray *)trec)->length;
				arrName = (char *) ((LFArray *)trec)->name;
			}
			else {
				//Need to calculate arrLen
				arrLen = 0;
				arrName = ( (char *) &((LFArray *)trec)->length ) + 
									sizeof(DWORD);
			}
			strncpy(typeName, &arrName[1], (int)arrName[0]);
			typeName[(int)arrName[0]] = '\0';
				
			int high = arrLen / lastType->getSize();
			newType = new BPatch_type(typeName, -1, BPatch_array, lastType, 
							0, high-1);
		}
		break;
	case LF_CLASS:
	case LF_STRUCTURE:
	{
		int classLen;
		char *className;
		if ( ((LFClass *)trec)->length < LF_NUMERIC ) {
			classLen = ((LFClass *)trec)->length;
			className = (char *) ((LFClass *)trec)->name;
		}
		else {
			//Need to calculate classLen
			classLen = 0;
			className = ( (char *) &((LFClass *)trec)->length ) +
									sizeof(DWORD);
		}
		strncpy(typeName, &className[1], (int)className[0]);
		typeName[(int)className[0]] = '\0';
				
		if (trec->leaf == LF_CLASS)
			newType = new BPatch_type(typeName, -1, BPatch_dataTypeClass, classLen);
		else
			newType = new BPatch_type(typeName, -1, BPatch_dataStructure,
									classLen);
		// printf("Name of the structure %s\n", typeName);
		FindFields(mod, newType, ((LFClass *)trec)->count, 
					((LFClass *)trec)->field, pTypeBase, startAddr);
		break;
	}
	case LF_UNION:
	{
		int unionLen;
		char *unionName;
		if ( ((LFUnion *)trec)->length < LF_NUMERIC ) {
			unionLen = ((LFUnion *)trec)->length;
			unionName = (char *) ((LFUnion *)trec)->name;
		}
		else {
			//Need to calculate classLen
			unionLen = 0;
			unionName = ( (char *) &((LFUnion *)trec)->length ) +
									sizeof(DWORD);
		}
		strncpy(typeName, &unionName[1], (int)unionName[0]);
		typeName[(int)unionName[0]] = '\0';
				
		newType = new BPatch_type(typeName, -1, BPatch_union, unionLen);
		FindFields(mod, newType, ((LFClass *)trec)->count,
				((LFClass *)trec)->field, pTypeBase, startAddr);
		break;
	}
	case LF_ENUM:
	{
		strncpy(typeName, &((LFEnum *)trec)->name[1], 
						(int)((LFEnum *)trec)->name[0]);
		typeName[(int)((LFEnum *)trec)->name[0]] = '\0';

		newType = new BPatch_type(typeName, -1, BPatch_enumerated);
		FindFields(mod, newType, ((LFEnum *)trec)->count,
				((LFEnum *)trec)->fList, pTypeBase, startAddr);
		break;
	}
	default:
		break;
	}

	if (!newType)
		return NULL;

	newType->setID(index);
	mod->moduleTypes->addType(newType);
	return(newType);
}
#endif // BPATCH_LIBRARY

//---------------------------------------------------------------------------
// CodeView::Module methods
//---------------------------------------------------------------------------

// a zero-argument constructor is required by the vector class
CodeView::Module::Module( void )
  : pmod( NULL ),
	pas( NULL ),
	psrc( NULL ),
    offsetText( 0 ),
    cbText( 0 )
{
}


CodeView::Module::Module( CodeView::ModuleSubsection* pModule,
                          unsigned int textId )
  : pmod( pModule ),
	pas( NULL ),
	psrc( NULL ),
    offsetText( 0 ),
    cbText( 0 )
{
    // A module may contribute code to several segments (sections)
    // in the image.  Paradyn assumes that the .text section
    // is the only one that matters with respect to code, so
    // we provide the offset of the code from this module
    // in the  section.
	unsigned int i;
	bool found = false;


	// search the module subsection's segInfo array to find
	// the offset for the code from this module in the .text section
	const CodeView::ModuleSubsection::SegInfo* pSects = 
		(const CodeView::ModuleSubsection::SegInfo*)(((const char*)pmod) 
            + sizeof(ModuleSubsection));
	for( i = 0; i < pmod->cSeg; i++ )
	{
		if( pSects[i].seg == textId )
		{
			offsetText = pSects[i].offset;
			cbText = pSects[i].cbSeg;

#ifdef _DEBUG
			// check whether there are other ranges that 
			// this module contributes to in the text segment
			unsigned int j;
			for( j = i + 1; j < pmod->cSeg; j++ )
			{
				if( pSects[j].seg == textId )
				{
					cout << 
                        "warning: multiple contributions to text section"
                        << endl;
				}
			}
#endif // _DEBUG
            break;
		}
	}
}


CodeView::Module&
CodeView::Module::operator=( const CodeView::Module& mod )
{
	if( &mod != this )
	{
		pmod = mod.pmod;			// sstModule subsection
		pas = mod.pas;				// sstAlignSym subsection
		psrc = mod.psrc;			// sstSrcModule subsection
		syms = mod.syms;			// symbols from the sstAlignSym subsection

        offsetText = mod.offsetText;
        cbText = mod.cbText;
	}
	return *this;
}


string
CodeView::Module::GetName( void ) const
{
	LPString lpsName( ((char*)pmod) +
		sizeof(ModuleSubsection) +
		pmod->cSeg * sizeof( ModuleSubsection::SegInfo ) );
	return (string)lpsName;
}


// note that although the sstSrcModule subsection may 
// indicate more than one source file contributed to the code
// in this module, we always only consider the first
// (Paradyn is not designed to handle more than one source
// file per module, and the first source file seems to be
// the "main" source file for the module)
string
CodeView::Module::GetSourceName( void ) const
{
	// verify that we have a source file information for this module
	if( psrc == NULL )
	{
		return "";
	}

	// parse the sstSrcModule subsection to extract the first
	// source file name
	const char* curr = (const char*)psrc;

	//
	// skip over the header...
	//
    // skip over file and segment counts
	assert( psrc->cFile > 0 );
    curr += 2 * sizeof( WORD );

	// ...skip over the baseSrcFile table...
	curr += (psrc->cFile * sizeof( DWORD ));

	// ...skip over the start/end table...
	curr += (psrc->cSeg * 2 * sizeof( DWORD ));

	// ...skip over the seg table...
	curr += (psrc->cSeg * sizeof( WORD ));
	if( (psrc->cSeg % 2) != 0 )
	{
		// skip alignment padding
		curr += sizeof(WORD);
	}

	//
	// we're now pointing at the information for the first source file
	//
    SrcModuleSubsection::FileInfo* fi = (SrcModuleSubsection::FileInfo*)curr;
	curr += sizeof( WORD );

	// skip padding
	curr += sizeof(WORD);

	// skip the line number/address mapping offset table
	curr += (fi->cSegFile * sizeof( DWORD ));

	// skip over the start/end offset table
	curr += (fi->cSegFile * 2 * sizeof( DWORD ));

	// we're now at the source file name
	// length in chars in the first byte, then the source file name chars
	LPString lpsName( curr );
	return lpsName;
}


// A module may contribute code to several segments (sections)
// in the image.  Paradyn assumes that the .text section
// is the only one that matters with respect to code, so
// we provide the offset of the code from this module
// in the  section.
bool
CodeView::Module::GetTextBounds( DWORD& offset, DWORD& cb ) const
{
    bool ret = false;

    if( cbText != 0 )
    {
        offset = offsetText;
        cb = cbText;
        ret = true;
    }
    return ret;
}

