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

// $Id: CodeView.C,v 1.5 2000/07/28 17:20:40 pcroth Exp $

#include <assert.h>

#include <iostream.h>
#include <iomanip.h>

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/CodeView.h"


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
	// ensure that the modules vector contains an element for this module
	// recall that the modules vector uses a one-based index
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
		libs += curr;

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
			lprocs += ( (SymRecordProc*)curr );
			break;

		case S_GPROC32:
			// add the entry to our list of functions
			gprocs += ( (SymRecordProc*)curr );
			break;

		case S_LDATA32:
			// add the entry to our list of variables
			lvars += ( (SymRecordData*)curr );
			break;

		case S_GDATA32:
			// add the entry to our list of variables
			gvars += ( (SymRecordData*)curr );
			break;

		case S_LABEL32:
			labels += ( (SymRecordLabel*)curr );
			break;

		case S_BPREL32:
			bprels += ( (SymRecordBPRel*)curr );
			break;

		case S_THUNK32:
			thunks += ( (SymRecordThunk*)curr );
			break;

		case S_END:
			// nothing to do for end markers
			break;

		case S_ALIGN:
			// skip padding records
			break;

        case S_PUB32:
            pubs += (SymRecordData*)curr;
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
