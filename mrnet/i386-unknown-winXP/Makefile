#
# Copyright (c) 1996-2003 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#
# $Id: Makefile,v 1.2 2003/11/21 17:57:54 pcroth Exp $
#

!include ..\\..\\nmake.config


!ifdef NO_OPT_FLAG
PROJCFG    = Debug
!else
PROJCFG    = Release
!endif

BUILDFLAGS  = /USEENV


XPLAT_LIB       = xplat\\libxplat.lib
XPLAT_INCDIR    = ..\\mrnet\\xplat\\include\\xplat
MRNET_LIB       = libmrnet\\libmrnet.lib
MRNET_INCDIR    = ..\\mrnet\\h
TOPGEN_EXE      = topgen\\topgen.exe

INSTDIR = ..\\..\\..\\$(PLATFORM)
IBINDIR = $(INSTDIR)\\bin
ILIBDIR = $(INSTDIR)\\lib
IINCDIR = $(INSTDIR)\\include
XPLAT_IINCDIR = $(INSTDIR)\\include\\xplat
MRNET_IINCDIR = $(INSTDIR)\\include\\mrnet



all: $(XPLAT_LIB) $(MRNET_LIB) $(TOPGEN_EXE)



#
# install targets
#
# note that we explicitly use the MS copy command for predictability
# and to avoid the portability problems between cp and copy (e.g.,
# different ability to handle forward vs. backslashes).
#

install:    xplat-install libmrnet-install topgen-install

xplat-install:  $(XPLAT_LIB) $(ILIBDIR) $(XPLAT_IINCDIR)
	copy $(XPLAT_LIB) $(ILIBDIR)\\
    copy $(XPLAT_INCDIR)\\*.h $(XPLAT_IINCDIR)\\

libmrnet-install: $(MRNET_LIB) $(ILIBDIR) $(MRNET_IINCDIR)
	copy $(MRNET_LIB) $(ILIBDIR)\\
    copy $(MRNET_INCDIR)\\*.h $(MRNET_IINCDIR)\\

topgen-install: $(TOPGEN_EXE) $(IBINDIR)
	copy $(TOPGEN_EXE) $(IBINDIR)\\


#
# directory creation targets
#
# Note that we do not rely on the directory-making command's
# ability to create intermediate directories.  Cygwin's mkdir does
# not handle paths with backslashes if -p is used.  The mkdir built in
# to Windows' cmd.exe shell does not create intermediate directories
# unless command extensions are turned on.
# 

$(INSTDIR):
	-md $@

$(ILIBDIR) $(IBINDIR) $(IINCDIR): $(INSTDIR)
	-md $@

$(XPLAT_IINCDIR): $(IINCDIR)
	-md $@

$(MRNET_IINCDIR): $(IINCDIR)
	-md $@
	

!ifdef USE_VC7
!include Makefile.vc7
!else
!include Makefile.vc6
!endif

