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


/* 
 * $Id: w32CONTEXT.h,v 1.4 2006/03/14 22:57:32 legendre Exp $
 */

#ifndef _w32CONTEXT_H
#define _w32CONTEXT_H

#ifdef mips_unknown_ce2_11 //ccw 27 july 2000
// the below definition of CONTEXT allows the MIPS context
// to be passed back up to the NT box and handled correctly.
// I have changed the pdwinnt.C file to use the type 
// w32CONTEXT rather than CONTEXT.  w32CONTEXT is defined
// below, either as mipsCONTEXT or plain old CONTEXT


// FROM WINNT.H 

// The following flags control the contents of the CONTEXT structure.
//
#define mipsCONTEXT_R4000   0x00010000    // r4000 context

#define mipsCONTEXT_CONTROL         (mipsCONTEXT_R4000 | 0x00000001L)
#define mipsCONTEXT_FLOATING_POINT  (mipsCONTEXT_R4000 | 0x00000002L)
#define mipsCONTEXT_INTEGER         (mipsCONTEXT_R4000 | 0x00000004L)

#define mipsCONTEXT_FULL (mipsCONTEXT_CONTROL | mipsCONTEXT_FLOATING_POINT | mipsCONTEXT_INTEGER)

//
// Context Frame
//
//  N.B. This frame must be exactly a multiple of 16 bytes in length.
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  3) it is used to construct a call frame for exception dispatching
//  in user mode, and 4) it is used in the user level thread creation
//  routines.
//
//  The layout of the record conforms to a standard call frame.
//

typedef struct _mipsCONTEXT {

    //
    // This section is always present and is used as an argument build
    // area.
    //

    DWORD Argument[4];

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    DWORD FltF0;
    DWORD FltF1;
    DWORD FltF2;
    DWORD FltF3;
    DWORD FltF4;
    DWORD FltF5;
    DWORD FltF6;
    DWORD FltF7;
    DWORD FltF8;
    DWORD FltF9;
    DWORD FltF10;
    DWORD FltF11;
    DWORD FltF12;
    DWORD FltF13;
    DWORD FltF14;
    DWORD FltF15;
    DWORD FltF16;
    DWORD FltF17;
    DWORD FltF18;
    DWORD FltF19;
    DWORD FltF20;
    DWORD FltF21;
    DWORD FltF22;
    DWORD FltF23;
    DWORD FltF24;
    DWORD FltF25;
    DWORD FltF26;
    DWORD FltF27;
    DWORD FltF28;
    DWORD FltF29;
    DWORD FltF30;
    DWORD FltF31;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_INTEGER.
    //
    // N.B. The registers gp, sp, and ra are defined in this section, but are
    //  considered part of the control context rather than part of the integer
    //  context.
    //
    // N.B. Register zero is not stored in the frame.
    //

    DWORD IntZero;
    DWORD IntAt;
    DWORD IntV0;
    DWORD IntV1;
    DWORD IntA0;
    DWORD IntA1;
    DWORD IntA2;
    DWORD IntA3;
    DWORD IntT0;
    DWORD IntT1;
    DWORD IntT2;
    DWORD IntT3;
    DWORD IntT4;
    DWORD IntT5;
    DWORD IntT6;
    DWORD IntT7;
    DWORD IntS0;
    DWORD IntS1;
    DWORD IntS2;
    DWORD IntS3;
    DWORD IntS4;
    DWORD IntS5;
    DWORD IntS6;
    DWORD IntS7;
    DWORD IntT8;
    DWORD IntT9;
    DWORD IntK0;
    DWORD IntK1;
    DWORD IntGp;
    DWORD IntSp;
    DWORD IntS8;
    DWORD IntRa;
    DWORD IntLo;
    DWORD IntHi;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    DWORD Fsr;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_CONTROL.
    //
    // N.B. The registers gp, sp, and ra are defined in the integer section,
    //   but are considered part of the control context rather than part of
    //   the integer context.
    //

    DWORD Fir;
    DWORD Psr;

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a thread's context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    DWORD ContextFlags;

    DWORD Fill[2];
} mipsCONTEXT, *PmipsCONTEXT;

//end WINNT.H

#define w32CONTEXT mipsCONTEXT
#define w32CONTEXT_CONTROL mipsCONTEXT_CONTROL
#define w32CONTEXT_FLOATING_POINT mipsCONTEXT_FLOATING_POINT
#define w32CONTEXT_INTEGER mipsCONTEXT_INTEGER
#define w32CONTEXT_FULL mipsCONTEXT_FULL

#else
//ccw 27 july 2000 : use standard CONTEXT, for whatever chip winnt determines.
#define w32CONTEXT CONTEXT 

#define w32CONTEXT_CONTROL CONTEXT_CONTROL
#define w32CONTEXT_FLOATING_POINT CONTEXT_FLOATING_POINT
#define w32CONTEXT_INTEGER CONTEXT_INTEGER

#define w32CONTEXT_FULL CONTEXT_FULL

#endif // ifdef mips_unknown_ce2_11


#endif
