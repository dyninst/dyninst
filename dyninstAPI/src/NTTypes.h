/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __NT_TYPES_H_
#define __NT_TYPES_H_

enum LeafIndex {
	// Leaf indices for type records that can be referenced from symbols:
	LF_MODIFIER 		= 0x1001,
	LF_POINTER 		= 0x1002,
	LF_ARRAY 		= 0x1003,
	LF_CLASS 		= 0x1004,
	LF_STRUCTURE 		= 0x1005,
	LF_UNION 		= 0x1006,
	LF_ENUM 		= 0x1007,
	LF_PROCEDURE 		= 0x1008,
	LF_MFUNCTION 		= 0x1009,
	LF_VTSHAPE		= 0x000a,
	LF_COBOL0		= 0x100a,
	LF_COBOL1		= 0x000c,
	LF_BARRAY		= 0x100b,
	LF_LABEL		= 0x000e,
	LF_NULL			= 0x000f,
	LF_NOTTRAN		= 0x0010,
	LF_DIMARRAY		= 0x100c,
	LF_VFTPATH		= 0x100d,
	LF_PRECOMP		= 0x100e,
	LF_ENDPRECOMP		= 0x0014,
	LF_OEM			= 0x100f,
	LF_TYPESERVER		= 0x0016,

	// Leaf indices for type records that can be referenced from 
	// other type records:
	LF_SKIP 		= 0x1200,
	LF_ARGLIST 		= 0x1201,
	LF_DEFARG 		= 0x1202,
	LF_FIELDLIST 		= 0x1203,
	LF_DERIVED 		= 0x1204,
	LF_BITFIELD 		= 0x1205,
	LF_METHODLIST 		= 0x1206,
	LF_DIMCONU 		= 0x1207,
	LF_DIMCONLU 		= 0x1208,
	LF_DIMVARU 		= 0x1209,
	LF_DIMVARLU 		= 0x120a,
	LF_REFSYM 		= 0x020c,

	// Leaf indices for fields of complex lists:
	LF_BCLASS 		= 0x1400,
	LF_VBCLASS 		= 0x1401,
	LF_IVBCLASS 		= 0x1402,
	LF_ENUMERATE 		= 0x0403,
	LF_FRIENDFCN 		= 0x1403,
	LF_INDEX 		= 0x1404,
	LF_MEMBER 		= 0x1405,
	LF_STMEMBER 		= 0x1406,
	LF_METHOD 		= 0x1407,
	LF_NESTTYPE 		= 0x1408,
	LF_VFUNCTAB 		= 0x1409,
	LF_FRIENDCLS 		= 0x140a,
	LF_ONEMETHOD 		= 0x140b,
	LF_VFUNCOFF 		= 0x140c,
	LF_NESTTYPEEX 		= 0x140d,
	LF_MEMBERMODIFY 	= 0x140e,
			
	// Leaf indices for numeric fields of symbols and type records:
	LF_NUMERIC 		= 0x8000,
	LF_CHAR 		= 0x8000,
	LF_SHORT 		= 0x8001,
	LF_USHORT 		= 0x8002,
	LF_LONG 		= 0x8003,
	LF_ULONG 		= 0x8004,
	LF_REAL32 		= 0x8005,
	LF_REAL64 		= 0x8006,
	LF_REAL80 		= 0x8007,
	LF_REAL128 		= 0x8008,
	LF_QUADWORD 		= 0x8009,
	LF_UQUADWORD 		= 0x800a,
	LF_REAL48 		= 0x800b,
	LF_COMPLEX32 		= 0x800c,
	LF_COMPLEX64 		= 0x800d,
	LF_COMPLEX80 		= 0x800e,
	LF_COMPLEX128 		= 0x800f,
	LF_VARSTRING 		= 0x8010,

	// Alignment values
	LF_PAD0			= 0xf0,
	LF_PAD1			= 0xf1,
	LF_PAD2			= 0xf2,
	LF_PAD3			= 0xf3,
	LF_PAD4			= 0xf4,
	LF_PAD5			= 0xf5,
	LF_PAD6			= 0xf60,
	LF_PAD7			= 0xf7,
	LF_PAD8			= 0xf8,
	LF_PAD9			= 0xf9,
	LF_PAD10		= 0xfa,
	LF_PAD11		= 0xfb,
	LF_PAD12		= 0xfc,
	LF_PAD13		= 0xfc,
	LF_PAD14		= 0xfe,
	LF_PAD15		= 0xff
};

enum PrimitiveTypes
{
	T_NOTYPE 		= 0x0000, // Uncharacterized type (no type)
	T_ABS 			= 0x0001, // Absolute symbol
	T_SEGMENT 		= 0x0002, // Segment type
	T_VOID 			= 0x0003, // Void
	T_PVOID 		= 0x0103, // Near pointer to void
	T_PFVOID 		= 0x0203, // Far pointer to void
	T_PHVOID 		= 0x0303, // Huge pointer to void
	T_32PVOID 		= 0x0403, // 32 bit near pointer to void
	T_32PFVOID 		= 0x0503, // 32 bit far pointer to void
	T_64PVOID 		= 0x0603, // 64 bit pointer to void
	T_CURRENCY 		= 0x0004, // Basic 8 byte currency value
	T_NBASICSTR 		= 0x0005, // Near Basic string
	T_FBASICSTR 		= 0x0006, // Far Basic string
	T_NOTTRANS 		= 0x0007, // Untranslated type record from CV 3.x format
	T_BIT 			= 0x0060, // Bit
	T_PASCHAR 		= 0x0061, // Pascal CHAR
	
	// Character types
	T_CHAR 			= 0x0010, // 8-bit signed 
	T_UCHAR 		= 0x0020, // 8-bit unsigned
	T_PCHAR 		= 0x0110, // Near pointer to 8-bit signed
	T_PUCHAR 		= 0x0120, // Near pointer to 8-bit unsigned
	T_PFCHAR 		= 0x0210, // Far pointer to 8-bit signed
	T_PFUCHAR 		= 0x0220, // Far pointer to 8-bit unsigned
	T_PHCHAR 		= 0x0310, // Huge pointer to 8-bit signed
	T_PHUCHAR 		= 0x0320, // Huge pointer to 8-bit unsigned
	T_32PCHAR 		= 0x0410, // 16:32 near pointer to 8-bit signed
	T_32PUCHAR 		= 0x0420, // 16:32 near pointer to 8-bit unsigned
	T_32PFCHAR 		= 0x0510, // 16:32 far pointer to 8-bit signed
	T_32PFUCHAR 		= 0x0520, // 16:32 far pointer to 8-bit unsigned
	T_64PCHAR 		= 0x0610, // 64 bit pointer to 8 bit signed
	T_64PUCHAR 		= 0x0620, // 64 bit pointer to 8 bit unsigned
	
	// Really a character types
	T_RCHAR 		= 0x0070, // real char
	T_PRCHAR 		= 0x0170, // near pointer to a real char
	T_PFRCHAR 		= 0x0270, // far pointer to a real char
	T_PHRCHAR 		= 0x0370, // huge pointer to a real char
	T_32PRCHAR 		= 0x0470, // 16:32 near pointer to a real char
	T_32PFRCHAR 		= 0x0570, // 16:32 far pointer to a real char
	T_64PRCHAR 		= 0x0670, // 64 bit pointer to a real char
	
	// Wide character types
	T_WCHAR 		= 0x0071, // wide char
	T_PWCHAR 		= 0x0171, // near pointer to a wide char
	T_PFWCHAR 		= 0x0271, // far pointer to a wide char
	T_PHWCHAR 		= 0x0371, // huge pointer to a wide char
	T_32PWCHAR 		= 0x0471, // 16:32 near pointer to a wide char
	T_32PFWCHAR 		= 0x0571, // 16:32 far pointer to a wide char
	T_64PWCHAR 		= 0x0671, // 64 bit pointer to a wide char
	
	// Really 16 bit integer types
	T_INT2 			= 0x0072, // really 16 bit signed int
	T_UINT2 		= 0x0073, // really 16 bit unsigned int
	T_PINT2 		= 0x0172, // near pointer to 16 bit signed int
	T_PUINT2 		= 0x0173, // near pointer to 16 bit unsigned int
	T_PFINT2 		= 0x0272, // far pointer to 16 bit signed int
	T_PFUINT2 		= 0x0273, // far pointer to 16 bit unsigned int
	T_PHINT2 		= 0x0372, // huge pointer to 16 bit signed int
	T_PHUINT2 		= 0x0373, // huge pointer to 16 bit unsigned int
	T_32PINT2 		= 0x0472, // 16:32 near pointer to 16 bit signed int
	T_32PUINT2 		= 0x0473, // 16:32 near pointer to 16 bit unsigned int
	T_32PFINT2 		= 0x0572, // 16:32 far pointer to 16 bit signed int
	T_32PFUINT2 		= 0x0573, // 16:32 far pointer to 16 bit unsigned int
	T_64PINT2 		= 0x0672, // 64 bit pointer to 16 bit signed int
	T_64PUINT2 		= 0x0673, // 64 bit pointer to 16 bit unsigned int
	
	// 16-bit short types
	T_SHORT 		= 0x0011, // 16-bit signed 
	T_USHORT 		= 0x0021, // 16-bit unsigned
	T_PSHORT 		= 0x0111, // Near pointer to 16-bit signed
	T_PUSHORT 		= 0x0121, // Near pointer to 16-bit unsigned
	T_PFSHORT 		= 0x0211, // Far pointer to 16-bit signed
	T_PFUSHORT 		= 0x0221, // Far pointer to 16-bit unsigned
	T_PHSHORT 		= 0x0311, // Huge pointer to 16-bit signed
	T_PHUSHORT 		= 0x0321, // Huge pointer to 16-bit unsigned
	T_32PSHORT 		= 0x0411, // 16:32 near pointer to 16 bit signed
	T_32PUSHORT 		= 0x0421, // 16:32 near pointer to 16 bit unsigned
	T_32PFSHORT 		= 0x0511, // 16:32 far pointer to 16 bit signed
	T_32PFUSHORT 		= 0x0521, // 16:32 far pointer to 16 bit unsigned
	T_64PSHORT 		= 0x0611, // 64 bit pointer to 16 bit signed
	T_64PUSHORT 		= 0x0621, // 64 bit pointer to 16 bit unsigned
	
	// Really 32 bit integer types
	T_INT4 			= 0x0074, // really 32 bit signed int
	T_UINT4 		= 0x0075, // really 32 bit unsigned int
	T_PINT4 		= 0x0174, // near pointer to 32 bit signed int
	T_PUINT4 		= 0x0175, // near pointer to 32 bit unsigned int
	T_PFINT4 		= 0x0274, // far pointer to 32 bit signed int
	T_PFUINT4 		= 0x0275, // far pointer to 32 bit unsigned int
	T_PHINT4 		= 0x0374, // huge pointer to 32 bit signed int
	T_PHUINT4 		= 0x0375, // huge pointer to 32 bit unsigned int
	T_32PINT4 		= 0x0474, // 16:32 near pointer to 32 bit signed int
	T_32PUINT4 		= 0x0475, // 16:32 near pointer to 32 bit unsigned int
	T_32PFINT4 		= 0x0574, // 16:32 far pointer to 32 bit signed int
	T_32PFUINT4 		= 0x0575, // 16:32 far pointer to 32 bit unsigned int
	T_64PINT4 		= 0x0674, // 64 bit pointer to 32 bit signed int
	T_64PUINT4 		= 0x0675, // 64 bit pointer to 32 bit unsigned int

	// 32-bit long types
	T_LONG 			= 0x0012, // 32-bit signed 
	T_ULONG 		= 0x0022, // 32-bit unsigned
	T_PLONG 		= 0x0112, // Near pointer to 32-bit signed
	T_PULONG 		= 0x0122, // Near pointer to 32-bit unsigned
	T_PFLONG 		= 0x0212, // Far pointer to 32-bit signed
	T_PFULONG 		= 0x0222, // Far pointer to 32-bit unsigned
	T_PHLONG 		= 0x0312, // Huge pointer to 32-bit signed
	T_PHULONG 		= 0x0322, // Huge pointer to 32-bit unsigned
	T_32PLONG 		= 0x0412, // 16:32 near pointer to 32 bit signed 
	T_32PULONG 		= 0x0422, // 16:32 near pointer to 32 bit unsigned 
	T_32PFLONG 		= 0x0512, // 16:32 far pointer to 32 bit signed 
	T_32PFULONG 		= 0x0522, // 16:32 far pointer to 32 bit unsigned
	T_64PLONG 		= 0x0612, // 64 bit pointer to 32 bit signed 
	T_64PULONG 		= 0x0622, // 64 bit pointer to 32 bit unsigned
	
	// Really 64-bit integer types
	T_INT8 			= 0x0076, // 64-bit signed int
	T_UINT8 		= 0x0077, // 64-bit unsigned int
	T_PINT8 		= 0x0176, // Near pointer to 64-bit signed int
	T_PUINT8 		= 0x0177, // Near pointer to 64-bit unsigned int
	T_PFINT8 		= 0x0276, // Far pointer to 64-bit signed int
	T_PFUINT8 		= 0x0277, // Far pointer to 64-bit unsigned int
	T_PHINT8 		= 0x0376, // Huge pointer to 64-bit signed int
	T_PHUINT8 		= 0x0377, // Huge pointer to 64-bit unsigned int
	T_32PINT8 		= 0x0476, // 16:32 near pointer to 64 bit signed int
	T_32PUINT8 		= 0x0477, // 16:32 near pointer to 64 bit unsigned int
	T_32PFINT8 		= 0x0576, // 16:32 far pointer to 64 bit signed int
	T_32PFUINT8 		= 0x0577, // 16:32 far pointer to 64 bit unsigned int
	T_64PINT8 		= 0x0676, // 64 bit pointer to 64 bit signed int
	T_64PUINT8 		= 0x0677, // 64 bit pointer to 64 bit unsigned int
	
	// 64-bit integral types
	T_QUAD 			= 0x0013, // 64-bit signed 
	T_UQUAD 		= 0x0023, // 64-bit unsigned
	T_PQUAD 		= 0x0113, // Near pointer to 64-bit signed
	T_PUQUAD 		= 0x0123, // Near pointer to 64-bit unsigned
	T_PFQUAD 		= 0x0213, // Far pointer to 64-bit signed
	T_PFUQUAD 		= 0x0223, // Far pointer to 64-bit unsigned
	T_PHQUAD 		= 0x0313, // Huge pointer to 64-bit signed
	T_PHUQUAD 		= 0x0323, // Huge pointer to 64-bit unsigned
	T_32PQUAD 		= 0x0413, // 16:32 near pointer to 64 bit signed 
	T_32PUQUAD 		= 0x0423, // 16:32 near pointer to 64 bit unsigned 
	T_32PFQUAD 		= 0x0513, // 16:32 far pointer to 64 bit signed 
	T_32PFUQUAD 		= 0x0523, // 16:32 far pointer to 64 bit unsigned 
	T_64PQUAD 		= 0x0613, // 64 bit pointer to 64 bit signed 
	T_64PUQUAD 		= 0x0623, // 64 bit pointer to 64 bit unsigned 
	
	// 32-bit real types
	T_REAL32 		= 0x0040, // 32-bit real 
	T_PREAL32 		= 0x0140, // Near pointer to 32-bit real
	T_PFREAL32 		= 0x0240, // Far pointer to 32-bit real
	T_PHREAL32 		= 0x0340, // Huge pointer to 32-bit real
	T_32PREAL32 		= 0x0440, // 16:32 near pointer to 32 bit real
	T_32PFREAL32 		= 0x0540, // 16:32 far pointer to 32 bit real
	T_64PREAL32 		= 0x0640, // 64 pointer to 32 bit real
	
	// 48-bit real types
	T_REAL48 		= 0x0044, // 48-bit real 
	T_PREAL48 		= 0x0144, // Near pointer to 48-bit real
	T_PFREAL48 		= 0x0244, // Far pointer to 48-bit real
	T_PHREAL48 		= 0x0344, // Huge pointer to 48-bit real
	T_32PREAL48 		= 0x0444, // 16:32 near pointer to 48 bit real
	T_32PFREAL48 		= 0x0544, // 16:32 far pointer to 48 bit real
	T_64PREAL48 		= 0x0644, // 64 bit pointer to 48 bit real
	
	// 64-bit real types
	T_REAL64 		= 0x0041, // 64-bit real 
	T_PREAL64 		= 0x0141, // Near pointer to 64-bit real
	T_PFREAL64 		= 0x0241, // Far pointer to 64-bit real
	T_PHREAL64 		= 0x0341, // Huge pointer to 64-bit real
	T_32PREAL64 		= 0x0441, // 16:32 near pointer to 64 bit real
	T_32PFREAL64 		= 0x0541, // 16:32 far pointer to 64 bit real
	T_64PREAL64 		= 0x0641, // 64 bit pointer to 64 bit real
	
	// 80-bit real types
	T_REAL80 		= 0x0042, // 80-bit real 
	T_PREAL80 		= 0x0142, // Near pointer to 80-bit real
	T_PFREAL80 		= 0x0242, // Far pointer to 80-bit real
	T_PHREAL80 		= 0x0342, // Huge pointer to 80-bit real
	T_32PREAL80 		= 0x0442, // 16:32 near pointer to 80 bit real
	T_32PFREAL80 		= 0x0542, // 16:32 far pointer to 80 bit real
	T_64PREAL80 		= 0x0642, // 64 bit pointer to 80 bit real
	
	// 128-bit real types
	T_REAL128 		= 0x0043, // 128-bit real 
	T_PREAL128 		= 0x0143, // Near pointer to 128-bit real
	T_PFREAL128 		= 0x0243, // Far pointer to 128-bit real
	T_PHREAL128 		= 0x0343, // Huge pointer to 128-bit real
	T_32PREAL128 		= 0x0443, // 16:32 near pointer to 128 bit real
	T_32PFREAL128 		= 0x0543, // 16:32 far pointer to 128 bit real
	T_64PREAL128 		= 0x0643, // 64 bit pointer to 128 bit real
	
	// 32-bit complex types
	T_CPLX32 		= 0x0050, // 32-bit complex 
	T_PCPLX32 		= 0x0150, // Near pointer to 32-bit complex
	T_PFCPLX32 		= 0x0250, // Far pointer to 32-bit complex
	T_PHCPLX32 		= 0x0350, // Huge pointer to 32-bit complex
	T_32PCPLX32 		= 0x0450, // 16:32 near pointer to 32 bit complex
	T_32PFCPLX32 		= 0x0550, // 16:32 far pointer to 32 bit complex
	T_64PCPLX32 		= 0x0650, // 64 bit pointer to 32 bit complex
	
	// 64-bit complex types
	T_CPLX64 		= 0x0051, // 64-bit complex 
	T_PCPLX64 		= 0x0151, // Near pointer to 64-bit complex
	T_PFCPLX64 		= 0x0251, // Far pointer to 64-bit complex
	T_PHCPLX64 		= 0x0351, // Huge pointer to 64-bit complex
	T_32PCPLX64 		= 0x0451, // 16:32 near pointer to 64 bit complex
	T_32PFCPLX64 		= 0x0551, // 16:32 far pointer to 64 bit complex
	T_64PCPLX64 		= 0x0651, // 64 bit pointer to 64 bit complex
	
	// 80-bit complex types
	T_CPLX80 		= 0x0052, // 80-bit complex 
	T_PCPLX80 		= 0x0152, // Near pointer to 80-bit complex
	T_PFCPLX80 		= 0x0252, // Far pointer to 80-bit complex
	T_PHCPLX80 		= 0x0352, // Huge pointer to 80-bit complex
	T_32PCPLX80 		= 0x0452, // 16:32 near pointer to 80 bit complex
	T_32PFCPLX80 		= 0x0552, // 16:32 far pointer to 80 bit complex
	T_64PCPLX80 		= 0x0652, // 64 bit pointer to 80 bit complex
	
	// 128-bit complex types
	T_CPLX128 		= 0x0053, // 128-bit complex 
	T_PCPLX128 		= 0x0153, // Near pointer to 128-bit complex
	T_PFCPLX128 		= 0x0253, // Far pointer to 128-bit complex
	T_PHCPLX128 		= 0x0353, // Huge pointer to 128-bit real
	T_32PCPLX128 		= 0x0453, // 16:32 near pointer to 128 bit complex
	T_32PFCPLX128 		= 0x0553, // 16:32 far pointer to 128 bit complex
	T_64PCPLX128 		= 0x0653, // 64 bit pointer to 128 bit complex
	
	// Boolean types
	T_BOOL08 		= 0x0030, // 8-bit Boolean
	T_BOOL16 		= 0x0031, // 16-bit Boolean
	T_BOOL32 		= 0x0032, // 32-bit Boolean
	T_BOOL64 		= 0x0033, // 64-bit Boolean
	T_PBOOL08 		= 0x0130, // Near pointer to 8-bit Boolean
	T_PBOOL16 		= 0x0131, // Near pointer to 16-bit Boolean
	T_PBOOL32 		= 0x0132, // Near pointer to 32-bit Boolean
	T_PBOOL64 		= 0x0133, // Near pointer to 64-bit Boolean
	T_PFBOOL08 		= 0x0230, // Far pointer to 8-bit Boolean
	T_PFBOOL16 		= 0x0231, // Far pointer to 16-bit Boolean
	T_PFBOOL32 		= 0x0232, // Far pointer to 32-bit Boolean
	T_PFBOOL64 		= 0x0233, // Far pointer to 64-bit Boolean
	T_PHBOOL08 		= 0x0330, // Huge pointer to 8-bit Boolean
	T_PHBOOL16 		= 0x0331, // Huge pointer to 16-bit Boolean
	T_PHBOOL32 		= 0x0332, // Huge pointer to 32-bit Boolean
	T_PHBOOL64 		= 0x0333, // Huge pointer to 64-bit Boolean
	T_32PBOOL08 		= 0x0430, // 16:32 near pointer to 8 bit boolean
	T_32PFBOOL08 		= 0x0530, // 16:32 far pointer to 8 bit boolean
	T_32PBOOL16 		= 0x0431, // 16:32 near pointer to 16 bit boolean
	T_32PFBOOL16 		= 0x0531, // 16:32 far pointer to 16 bit boolean
	T_32PBOOL32 		= 0x0432, // 16:32 near pointer to 32 bit boolean
	T_32PFBOOL32 		= 0x0532, // 16:32 far pointer to 32 bit boolean
	T_32PBOOL64 		= 0x0433, // 16:32 near pointer to 64-bit Boolean
	T_32PFBOOL64 		= 0x0533, // 16:32 far pointer to 64-bit Boolean
	T_64PBOOL08 		= 0x0630, // 64 bit pointer to 8 bit boolean
	T_64PBOOL16 		= 0x0631, // 64 bit pointer to 16 bit boolean
	T_64PBOOL32 		= 0x0632, // 64 bit pointer to 32 bit boolean
	T_64PBOOL64 		= 0x0633  // 64 bit pointer to 64-bit Boolean
};

#endif //__NT_TYPES_H_
