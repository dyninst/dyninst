#!/usr/bin/env perl
#
#  Copyright (c) 2020, Rice University.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
#  * Neither the name of Rice University (RICE) nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
#  This software is provided by RICE and contributors "as is" and any
#  express or implied warranties, including, but not limited to, the
#  implied warranties of merchantability and fitness for a particular
#  purpose are disclaimed. In no event shall RICE or contributors be
#  liable for any direct, indirect, incidental, special, exemplary, or
#  consequential damages (including, but not limited to, procurement of
#  substitute goods or services; loss of use, data, or profits; or
#  business interruption) however caused and on any theory of liability,
#  whether in contract, strict liability, or tort (including negligence
#  or otherwise) arising in any way out of the use of this software, even
#  if advised of the possibility of such damage.
#
#  Mark W. Krentel
#  Rice University
#  May 2020
#
#----------------------------------------------------------------------
#
#  This script scans a binary file (libc.so, libstdc++.so, etc) and
#  reports function names that have attribute DW_AT_noreturn.
#
#  To use: build glibc (direct, via autotools) and gcc (via spack)
#  with debug/dwarf info (-g) and run this script on various
#  libraries.
#
#    libc.so
#    libpthread.so
#    libstdc++.so
#    libgfortran.so
#    libgomp.so
#
#  Usage:  ./noreturn.pl  [-d | -v]  file.so
#
#    -d  format output for dyninst CodeSource.C
#    -v  verbose output
#
#----------------------------------------------------------------------
#
#  We read the dwarf info with 'objdump -Wi' and look for
#  DW_TAG_subprogram items with attribute DW_AT_noreturn.  Compare the
#  names and linkage names with the output from nm.  For example:
#
#  <2><15ed3b>: Abbrev Number: 29 (DW_TAG_subprogram)
#     <15ed3c>   DW_AT_external    : 1
#     <15ed3c>   DW_AT_name        : (indirect string, offset: 0x1e602): __throw_bad_alloc
#     <15ed40>   DW_AT_decl_file   : 1
#     <15ed41>   DW_AT_decl_line   : 53
#     <15ed42>   DW_AT_linkage_name: (indirect string, offset: 0x1f838): _ZSt17__throw_bad_allocv
#     <15ed46>   DW_AT_noreturn    : 1
#     <15ed46>   DW_AT_declaration : 1
#  <2><15ed46>: Abbrev Number: 29 (DW_TAG_subprogram)
#  ...
#
#  Note: Some functions have multiple names and the dwarf info
#  normally exists for only one name.  So, when looking up the
#  attribute names, we treat one name as referring to all names with
#  the same address.  For example:
#
#    00000000000c91b0 W _Exit
#    00000000000c91b0 t __GI__exit
#    00000000000c91b0 T _exit
#
#----------------------------------------------------------------------

$DW_ABBREV =     "Abbrev Number";
$DW_LINK_NAME =  "DW_AT_linkage_name";
$DW_NAME =       "DW_AT_name";
$DW_NORET =      "DW_AT_noreturn";
$DW_SUBPROG =    "DW_TAG_subprogram";

%Addr2Name = ();
%Name2Addr = ();

%NORET = ();

$dyninst = 0;
$verbose = 0;

#----------------------------------------------------------------------
#
#  Print one name in short, verbose or dyninst format.  Include alias
#  names with the same address and skip duplicates.
#
sub do_name {
    local $name = $_[0];
    local $addr = $Name2Addr{$name};

    if ( ! $addr ) {
	return;
    }

    # multiple names separated by spaces
    @list = split /[\s]+/, $Addr2Name{$addr};

    foreach $nm ( @list ) {
	#
	# strip @GLIBC_x.y.z, if exists
	#
	$nm =~ s/[@]glibc.*$//i;

	if ( ! $NORET{$nm} ) {
	    if ( $dyninst ) {
		print "        (\"$nm\", true)\n";
	    }
	    elsif ( $verbose ) {
		print "noret:  $nm\n";
	    }
	    else {
		print "$nm\n";
	    }
	    $NORET{$nm} = 1;
	}
    }
}

#----------------------------------------------------------------------
#
#  Parse one noreturn subprogram item and pick out the name and
#  linkage name attributes.
#
sub do_item {
    local $item = $_[0];
    local $name;

    if ( $item !~ /$DW_NORET/ ) {
	return;
    }

    if ( $verbose ) {
	print "\n------------------------------------------------------------\n";
	print "$item\n";
    }

    #
    # check DW_AT_name, if exists
    #
    if ( $item =~ /$DW_NAME.*$/m ) {
	@list = split /[\s]+/, $&;
	$name = $list[$#list];
	do_name $name;
    }

    #
    # check DW_AT_linkage_name
    #
    if ( $item =~ /$DW_LINK_NAME.*$/m ) {
	@list = split /[\s]+/, $&;
	$name = $list[$#list];
	do_name $name;
    }
}

#----------------------------------------------------------------------
#
#  Parse the dwarf info and call do_item() on each subprogram item.
#  An item begins with the DW_TAG_subprogram and ends with the next
#  Abbrev Number line.  (see above)
#
sub parse_dwarf {
    local $file = $_[0];
    local $endfile = 0;
    local ($item, $next_item);

    if ( ! open(DWARF, "objdump -Wi $file |") ) {
	die "unable to run objdump: $file";
    }

    $item = "";
    OUTER_LOOP: while ( 1 ) {
	#
	# skip ahead until DW_TAG_subprogram
	#
	while ( $item !~ /$DW_SUBPROG/ ) {
	    $item = <DWARF>;
	    if ( ! $item ) {
		last OUTER_LOOP;
	    }
	}

	#
	# combine lines until the next Abbrev Number.  save the Abbrev
	# line as the first line of the next item.
	#
	$next_item = "";
	while ( 1 ) {
	    if ( $_ = <DWARF> ) {
		if ( $_ =~ /$DW_ABBREV/ ) {
		    $next_item = $_;
		    last;
		}
		else {
		    $item = $item . $_;
		}
	    }
	    else {
		$endfile = 1;
		last;
	    }
	}
	do_item $item;

	if ( $endfile ) {
	    last;
	}
	$item = $next_item;
    }

    close DWARF;
}

#----------------------------------------------------------------------
#
#  Use nm to create two mappings, address to name(s) and name to
#  address.  The addr2name list is a multi-valued string, separated by
#  spaces.
#
sub get_names {
    local $file = $_[0];
    local ($line, $addr, $name);

    if ( ! open(NM, "nm $file |") ) {
        die "unable to run nm: $file";
    }

    while ( $line = <NM> ) {
        @list = split /[\s]+/, $line;
	$addr = $list[0];
        $name = $list[$#list];

	if ( $addr =~ /^0/ ) {
	    $Addr2Name{$addr} =
		( $Addr2Name{$addr} ) ? "$Addr2Name{$addr} $name" : $name;
	    $Name2Addr{$name} = $addr;
	}
    }

    close NM;
}

#----------------------------------------------------------------------
#
#  Command-line args:  [-d | -v]  file.so
#
while ( $ARGV[0] =~ /^-/ ) {
    if ( $ARGV[0] eq '-d' ) {
	$dyninst = 1;
	shift;
    }
    elsif ( $ARGV[0] eq '-v' ) {
	$verbose = 1;
	shift;
    }
    else {
	die "unknown option:  $ARGV[0]";
    }
}

$file = $ARGV[0];

if ( $file eq '' ) {
    die "missing file";
}
if ( ! -f $file ) {
    die "not a regular file: $file";
}

get_names $file;
parse_dwarf $file;
