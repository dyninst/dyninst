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

// $Id: CodeView.h,v 1.7 2001/08/01 15:39:54 chadd Exp $

//
// This file contains the declaration of the CodeView class.
// The CodeView class also declares several sub-classes and 
// sub-structs that describe the format of many CodeView 
// constructs.
//
// As is commonly the case in Microsoft programming, the 
// CodeView data layout involves variable-length constructs.
// (For example, many CodeView subsections involve
// variable-length arrays.)  C++ does not provide syntax
// capable of describing such situations, and so many of
// the sub-struct declarations below are incomplete.
//
// Also, this CodeView class is not complete in that it
// does not provide a window into all of the CodeView 
// information.  It is intended, at this time, to provide
// enough support for the functionality needed by Paradyn/DyninstAPI.
// The implementation should be flexible enough, however, to add
// additional support as needed.
//
// For these two reasons, this file should not be used as a 
// definition of the CodeView symbol format.  See the CodeView
// specification from MSDN for such purposes.
//
#ifndef CODEVIEW_H
#define CODEVIEW_H

#ifdef mips_unknown_ce2_11 
#include <windows.h> //ccw 7 apr 2001
#endif

#ifdef BPATCH_LIBRARY
#include "BPatch.h"
#include "BPatch_module.h"
#include "dyninstAPI/src/BPatch_collections.h"
#endif

//
// LPString
//
// An LPString is a length-prefixed string.
// The length is contained in a byte, followed by the
// characters of the string.  Unlike a C string, there
// is no null termination.
//
class LPString
{
private:
	const char* data;

public:
	LPString( const char* str = NULL ) : data( str ) {}

	unsigned char GetLength( void ) const   { return *data; }
	const char*	GetChars( void ) const      { return (data + 1); }

	operator string( void ) const           { return string( data + 1,
                                                *(unsigned char*)data ); }
};



//
// CodeView
//
// A CodeView object knows how to parse debug 
// information in CodeView format.
//
class CodeView
{
public:
    // types of subsections
	enum SubsectionType
	{
		sstModule		= 0x120,
		sstTypes		= 0x121,
		sstPublic		= 0x122,
		sstPublicSym	= 0x123,
		sstSymbols		= 0x124,
		sstAlignSym		= 0x125,
		sstSrcLnSeg		= 0x126,
		sstSrcModule	= 0x127,
		sstLibraries	= 0x128,
		sstGlobalSym	= 0x129,
		sstGlobalPub	= 0x12a,
		sstGlobalTypes	= 0x12b,
		sstMPC			= 0x12c,
		sstSegMap		= 0x12d,
		sstSegName		= 0x12e,
		sstPreComp		= 0x12f,
		sstOffsetMap16	= 0x131,
		sstOffsetMap32	= 0x132,
		sstFileIndex	= 0x133,
		sstStaticSym	= 0x134
	};

    // types of symbol records
	enum SymbolType
	{
		S_COMPILE		= 0x0001,
		S_SSEARCH		= 0x0005,
		S_END			= 0x0006,
		S_SKIP			= 0x0007,
		S_CVRESERVE		= 0x0008,
		S_OBJNAME		= 0x0009,
		S_ENDARG		= 0x000a,
		S_COBOLUDT		= 0x000b,
		S_MANYREG		= 0x000c,
		S_RETURN		= 0x000d,
		S_ENTRYTHIS		= 0x000e,
		S_REGISTER		= 0x1001,
		S_CONSTANT		= 0x1002,
		S_UDT			= 0x1003,
		S_COBOLUDT_2	= 0x1004,
		S_MANYREG_2		= 0x1005,
		S_BPREL32		= 0x1006,
		S_LDATA32		= 0x1007,
		S_GDATA32		= 0x1008,
		S_PUB32			= 0x1009,
		S_LPROC32		= 0x100a,
		S_GPROC32		= 0x100b,
		S_THUNK32		= 0x0206,
		S_BLOCK32		= 0x0207,
		S_WITH32		= 0x0208,
		S_LABEL32		= 0x0209,
		S_CEXMODEL32	= 0x020a,
		S_VFTTABLE32	= 0x100c,
		S_REGREL32		= 0x100d,
		S_LTHREAD32		= 0x100e,
		S_GTHREAD32		= 0x100f,
		S_LPROCMIPS		= 0x1010,
		S_GPROCMIPS		= 0x1011,
		S_PROCREF		= 0x0400,
		S_DATAREF		= 0x0401,
		S_ALIGN			= 0x0402
	};

    // format of subsection directory header
	struct SDHeader
	{
		WORD	cbDirHeader;		// length of subsection directory header
		WORD	cbDirEntry;			// length of each directory entry
		DWORD	cDir;				// number of directory entries
		DWORD	IfoNextDir;			// offset from IfaBase to next directory
                                    // (currently unused)
		DWORD	flags;				// (currently unused)
	};

    // format of subsection directory entries
	struct SDEntry
	{
		WORD	sst;				// type of the subsection
		WORD	iMod;				// index of associated module
		DWORD	offset;				// offset from IfaBase of subsection
		DWORD	cb;					// size of subsection in bytes
	};

    // format of symbol subsection header
	struct SymHeader
	{
		WORD	symhash;			// index of symbol hash function
		WORD	addrhash;			// index of address hash function
		DWORD	cbSymbol;			// size (bytes) of symbol table
		DWORD	cbSymHash;			// size (bytes) of symbol hash table
		DWORD	cbAddrHash;			// size (bytes) of address hash table
	};


    // format of common fields for symbol records
	struct SymRecord
	{
		WORD	length;			// length of record, excluding length field
		WORD	index;			// type of symbol record
	};

    // format of PROC (function) symbol records
	struct SymRecordProc
	{
		SymRecord	base;		// fields to all SymRecords
		DWORD		pParent;	// parent lexical scope
		DWORD		pEnd;		// end of lexical scope
		DWORD		pNext;		// next lexical scope
		DWORD		procLength;	// size of the procedure (bytes)
		DWORD		debugStart;	// offset in bytes from proc start to point
                                //      where stack frame has been set up
		DWORD		debugEnd;	// offset in bytes from proc start to point
                                //      where procedure is ready to return
		DWORD		procType;	// type of procedure type record
		DWORD		offset;		// offset portion of proc address
		WORD		segment;	// segment (PE section) part of proc address
		char		flags;		// flags
								//    fpo       :1 true if function has frame
                                //                  pointer omitted
								//    interrupt :1 true if function is
                                //                  interrupt routine
								//    return    :1 true if function performs
                                //                  far return
								//    never     :1 true if function never
                                //                  returns
								//    unused    :4
		char		name[1];	// length-prefixed name of procedure
	};

    // format of PROCREF symbol records
    // (A PROCREF references a PROC symbol record
    // elsewhere in the CodeView information)
	struct SymRecordProcRef
	{
		SymRecord	base;		// fields common to all SymRecords
		DWORD		checksum;	// checksum of referenced symbol name
		DWORD		offset;		// offset of procedure symbol record
                                // from the module subsection (?)
		WORD		module;		// index of the module that contains the symbol
	};

    // format of DATA symbol records
	struct SymRecordData
	{
		SymRecord	base;		// fields common to all SymRecords
		DWORD		type;		// type of variable
		DWORD		offset;		// offset portion of variable address
		WORD		segment;	// segment (section) part of variable address
		char		name[1];	// length-prefixed name of variable
	};

    // format of BPREL symbol records
    // (BPREL records contain information about
    // variables contained on the stack)
	struct SymRecordBPRel
	{
		SymRecord	base;		// fields common to all SymRecords
		DWORD		offset;		// offset from BP of variable
		DWORD		type;		// type of variable
		char		name[1];	// length-prefixed name of variable
	};

    // format of LABEL symbol records
    // LABEL records are sometimes used to 
    // represent functions.
	struct SymRecordLabel
	{
		SymRecord	base;		// fields common to all SymRecords
		DWORD		offset;		// offset portion of label address
		WORD		segment;	// segment (PE section) part of label address
		char		flags;		// flags
								//    fpo       :1 true if function has
                                //                  frame pointer omitted
								//    interrupt :1 true if function is
                                //                  interrupt routine
								//    return    :1 true if function performs
                                //                  far return
								//    never     :1 true if function never
                                //                  returns
								//    unused    :4
		char		name[1];	// length-prefixed name of procedure
	};

    // format of THUNK symbol records
    // A THUNK is a piece of code outside a function
	struct SymRecordThunk
	{
		SymRecord	base;		// fields common to all SymRecords
		DWORD		pParent;	// parent lexical scope
		DWORD		pEnd;		// end of lexical scope
		DWORD		pNext;		// next lexical scope
		DWORD		offset;		// offset portion of thunk address
		WORD		segment;	// segment (PE section) part of thunk address
		WORD		thunkLength;	// length of thunk code (in bytes)
		char		ordinal;	// ordinal specifying type of thunk
								// 0: notype
								// 1: adjustor
								// 2: vcall
								// 3: pcode
		char		name[1];	// length-prefixed name of procedure
		// variant field.       if ordinal is:
		//                      notype => there is no variant field
		//                      adjustor => WORD offset from this pointer,
        //                          then length-prefixed name of function
        //                          to call
		//                      vcall => WORD displacement into virtual table
		//                      pcode => segment:offset of pcode entry point
	};

	// Format of constant variables
	struct SymRecordCons
	{
		SymRecord	base;
		DWORD		type;		// Type of symbol or containing enum
		WORD		value;		// Numeric leaf containing the value 
						// of symbol
		char		name[1];	// Length-prefixed name of symbol
	};

	// Format of typedefs
	struct SymRecordTypeDef
	{
		SymRecord	base;
		DWORD		type;		// Type of symbol
		char		name[1];	// Length-prefixed name of 
						// the user defined type
	};

    // format of Module subsection
	struct ModuleSubsection
	{
		struct SegInfo
		{
			WORD	seg;			// segment described
			WORD	pad;			// padding - must be zero
			DWORD	offset;			// offset in segment to where code starts
			DWORD	cbSeg;			// number of bytes of code in the segment
		};

		WORD	ovlNumber;		// overlay number
		WORD	iLib;			// library index if linked from library
		WORD	cSeg;			// number of SegInfo structs in subsection
		char	style[2];		// debugging style (should be "CV")

		// SegInfo	sinfo[];	// array of segment information structs
		// char		name[1];	// length-prefixed name of module
	};

    // format of AlignSym subsection
	struct AlignSymSubsection
	{
		// sstAlignSym subsections have no header.
		// The subsection consists of a stream of variable-length
        // SymRecord* entries
	};

    // format of SrcModule subsection
	struct SrcModuleSubsection
	{
        struct FileInfo
        {
            WORD    cSegFile;   // number of segments that receive code
                                // from this source file
            WORD    pad;
            // DWORD    baseSrcLen[cSegFile];   // array of offsets for line/
                                                // address mapping table
            // DWORD    startEnd[cSegFile][2];  // start/end offsets of
                                                // code for this source file
            // char     name[1];                // length-prefixed name of file
        };

        WORD    cFile;          // number of files that contributed
                                // code/data to this module
        WORD    cSeg;           // number of segments that received
                                // code/data from this module
        // DWORD    baseSrcFile[cFile];     // array of offsets into
                                            // source file array
        // DWORD    startEnd[cSeg][2];      // start/end offsets of
                                            // code or data for each segment

        // WORD     seg[cSeg];              // segment indices for segments
                                            // that receive code from this
                                            // module
        // WORD     pad[0-2];               // padding to maintain 4-byte
                                            // alignment

        // FileInfo fileInfo[];             // array of variable-length
                                            // records describing source
                                            // files
	};

	//
	// Type record definitions
	//
	struct TypesSubSection {
		char flags[4];
		DWORD cType; //Number of Types
		DWORD offType[1]; // Type offsets
	};

	struct TypeRec {
		WORD length;
		WORD leaf;
	};

	struct ProcTypeRec {
		TypeRec trec;
		DWORD rvtype; 		// Return value type
		char call;		// Calling convention of the procedure
		char reserved;
		WORD  parms;		// Number of parameters
		DWORD arglist;		// Type index of argument list type record
	};

	struct MFuncTypeRec {
		TypeRec trec;
		DWORD rvtype;		// Return value type
		DWORD classType;	// Type index of the containing class of 
					// the function
		DWORD thisType;		// Type index of the this parameter of 
					// the member function
		char call;		// Calling convention of the procedure
		char res;		// Reserved. Must be emitted as zero
		WORD parms;		// Number of parameters
		DWORD arglist;		// Type index of argument list type record
		DWORD thisadjust;	// Logical this adjustor for the method
	};

	struct LFModifier {
		TypeRec trec;
		DWORD index;		// type index of the modified type
		WORD attribute;	
	};

	struct LFPointer {
		TypeRec trec;
		DWORD type;		// Type index of object pointed to
		DWORD attribute;	// Ordinal specifying mode of pointer
	};

	struct LFArray {
		TypeRec trec;
		DWORD elemtype;		// Type index of each array element
		DWORD idxtype;		// Type index of indexing variable 
		WORD length;		// Length of array in bytes
		char name[1];
	};

	struct LFClass {
		TypeRec trec;
		WORD count;		// Number of elements in the class or structure
		WORD property;		// Property bit field
		DWORD field;		// Type index of the field list for this class
		DWORD dList;		// Type index of the derivation list
		DWORD vshape;		// Type index of the virtual function table 
					// shape descriptor
		WORD length;		// Numeric leaf specifying size in bytes of 
					// the structure
		char name[1];
	};

	struct LFUnion {
		TypeRec trec;
		WORD count;		// Number of fields in the union
		WORD property;		// Property bit field
		DWORD field;		// Type index of the field list
		WORD length;		// Numeric leaf specifying size in bytes of 
					// the union
		char name[1];
	};

	struct LFEnum {
		TypeRec trec;
		WORD count;		// Number of fields in the union
		WORD property;		// Property bit field
		DWORD type;		// Underlying type of enum
		DWORD fList;		// Type index of field list
		char name[1];		// Length-prefixed name of enum
	};

	struct LFMfunc {
		TypeRec trec;
		DWORD rvtype;		// Type index of the value returned by 
					// the procedure
		DWORD classidx;		// Type index of the containing class of 
					// the function
		DWORD thisidx;		// Type index of the this parameter of 
					// the member function. 
		char call;		// Calling convention of the procedure
		char res;		// Reserved. Must be emitted as zero
		WORD params;		// Number of parameters
		DWORD arglist;		// List of parameter specifiers
		DWORD thisadjust;	// Logical this adjustor for the method
	};

	struct LFDimArray {
		TypeRec trec;
		DWORD utype;		// Underlying type of the array
		DWORD diminfo;		// Index of the type record containing 
					// the dimension information
		char name[1];		// Length-prefixed name of the array
	};

	struct LFMember {
		WORD leaf;
		WORD attribute;		// Member attribute bit field
		DWORD type;		// Index to type record for field
		WORD offset;		// Numeric leaf specifying the offset of field 
					// in the structure
		char name[1];		// Length-prefixed name of the member field
	};

	struct LFIndex {
		WORD leaf;
		WORD pad0;		// Two bytes of padding for native alignment 
					// on type index to follow, must be 0
		DWORD index;		// Type index. 
	};

	struct LFEnumerate {
		WORD leaf;
		WORD attribute;		// Member attribute bit field 
		WORD value;		// Numeric leaf specifying the value of enumerate
		char name[1];		// Length-prefixed name of the member field.
	};

	struct LFMethod {
		WORD leaf;
		WORD count;		// Number of occurrences of function within 
					// the class. 
		DWORD mlist;		// Type index of method list
		char name[1];		// Length-prefixed name of method
	};

	struct LFBclass {
		WORD leaf;
		WORD attribute;		// attribute Member attribute bit field
		DWORD type;		// Index to type record of the class
		WORD offset;		// Offset of subobject that represents 
					// the base class within the structure
	};

	struct LFOnemethod {
		WORD leaf;
		WORD attribute;		// Method attribute
		DWORD type;		// Type index of method
		char name[1];		// Length prefixed name of method
	};

	struct LFStmember {
		WORD leaf;
		WORD attribute;		// Member attribute bit field
		DWORD type;		// Index to type record for field
		char name[1];		// Length-prefixed name of the member field
	};

	struct LFVbclass {
		WORD leaf;
		WORD attribute;		// Member attribute bit field
		DWORD btype;		// Index to type record of the direct or 
					// indirect virtual base class
		DWORD vbtype;		// Type index of the virtual base pointer 
					// for this base
		WORD vbpoff;		// Numeric leaf specifying the offset of 
					// the virtual base pointer from the address 
					// point of the class for this virtual base
	};

	struct LFVfunctab {
		WORD leaf;
		WORD pad0;		// Two bytes of padding for native alignment 
					// on type index to follow, must be 0
		DWORD type;		// Index to the pointer record describing 
					// the pointer
	};

    // object that encapsulates the various types of symbols
	class Symbols
	{
	public:
        // accessors
		const vector<SymRecordProc*>&
            GetGlobalFunctions( void ) const    { return gprocs; }
		const vector<SymRecordProc*>&
            GetLocalFunctions( void ) const     { return lprocs; }
		const vector<SymRecordData*>&
            GetGlobalVariables( void ) const	{ return gvars; }
		const vector<SymRecordData*>&
            GetLocalVariables( void ) const		{ return lvars; }
		const vector<SymRecordBPRel*>&
            GetStackVariables( void ) const		{ return bprels; }
		const vector<SymRecordLabel*>&
            GetLabels( void ) const				{ return labels; }
		const vector<SymRecordThunk*>&
            GetThunks( void ) const				{ return thunks; }
        const vector<SymRecordData*>&
            GetPublics( void ) const            { return pubs; }

        // operations
		void	Parse( const char* pSymBase, DWORD cb );
		Symbols& operator=( const Symbols& syms );
#ifdef BPATCH_LIBRARY
		void CreateTypeInfo( const char* pSymBase, DWORD cb, 
                               TypesSubSection *pTypeBase, BPatch_module *mod ,
			       LineInformation* lineInformation);
#endif

	private:
		vector<SymRecordProc*> gprocs;		// global functions
		vector<SymRecordProc*> lprocs;		// local functions
		vector<SymRecordData*> gvars;		// global variables
		vector<SymRecordData*> lvars;		// local variables
		vector<SymRecordBPRel*> bprels;	    // stack variables
		vector<SymRecordLabel*> labels;	    // labels
		vector<SymRecordThunk*> thunks;	    // thunks (non-function code)
        vector<SymRecordData*> pubs;        // public (catch-all) symbols

		//Below are used to create DyninstAPI types for the symbols

	// Operations
#ifdef BPATCH_LIBRARY
		BPatch_type *ExploreType(BPatch_module *, DWORD, 
						TypesSubSection *, char *);
		BPatch_type *CreatePrimitiveType(DWORD index);
		void FindFields(BPatch_module *mod, BPatch_type *mainType, int cnt,
				DWORD index, TypesSubSection *pTypeBase, char *startAddr);
#endif

        friend class CodeView;
	};


    // object that represents a CodeView module,
    // by tying together the relevant subsections
    // associated with that module
	class Module
	{
	public:
		Module( void );
		Module( ModuleSubsection* pModule, unsigned int textId );

		string GetName( void ) const;
		WORD GetLibraryIndex( void ) const		{ return pmod->iLib; }
		const Symbols& GetSymbols( void ) const		{ return syms; }
		string GetSourceName( void ) const;
		bool    GetTextBounds( DWORD& offset, DWORD& cb ) const;

		Module& operator=( const Module& mod );

	private:
		ModuleSubsection* pmod;		// sstModule subsection
		AlignSymSubsection* pas;	// sstAlignSym subsection
		SrcModuleSubsection* psrc;	// sstSrcModule subsection
		Symbols syms;				// symbols from the sstAlignSym subsection
        DWORD offsetText;           // offset of code in text section
        DWORD cbText;               // size of code in text section

		friend class CodeView;
	};


    // constructors
	CodeView( const char* pSymbols, unsigned int textSectionId )
	  : pBase( pSymbols ),
		textId( textSectionId )
	{}

    // accessors
	const vector<Module>&		GetModules( void ) const	{ return modules; }
	const vector<LPString>&		GetLibraries( void ) const	{ return libs; }
    const Symbols&              GetSymbols( void ) const    { return syms; }

    // operations
	bool Parse( void );
#ifdef BPATCH_LIBRARY
	//this method parses debug infor and generates type and line info
	void CreateTypeAndLineInfo( BPatch_module *inpMod , DWORD baseAddr ,
                                    LineInformation* lineInformation);

	//method is called by former to create the mappings (address,line)
        //for source files. It does not include the specific fucntion info
	//which is later done by former.
	void CreateLineInfo(const char* srcMod, DWORD baseAddr ,
                            LineInformation* lineInformation);
#endif

private:
	const char* pBase;			// location of CodeView symbols
	vector<Module> modules;		// modules represented in the executable
	vector<LPString> libs;		// libraries used to build this executable
    Symbols syms;               // symbols not associated with a module
    unsigned int textId;        // section number for .text section

	void	ParseModuleSubsection( SDEntry* pEntry );
	void	ParseLibrariesSubsection( SDEntry* pEntry );
	void	ParseAlignSymSubsection( SDEntry* pEntry );
	void	ParseSrcModuleSubsection( SDEntry* pEntry );
	void	ParseSymbolsWithHeaderSubsection( SDEntry* pEntry );
};

#endif // CODEVIEW_H
