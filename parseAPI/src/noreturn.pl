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
#  Co-author: Tim Haines
#  University of Wisconsin-Madison
#
#----------------------------------------------------------------------

use strict;
use warnings;
use Pod::Usage;
use Getopt::Long qw(GetOptions);

my $dyninst = undef;
my $verbose = undef;
my $help    = undef;

GetOptions(
	'd|dyninst' => \$dyninst,
	'v|verbose' => \$verbose,
	'h|help'    => \$help
) or pod2usage( -exitval => 2 );

pod2usage( -exitval => 0, -verbose => 99 ) if $help;

# Make sure the user specified at least one file to parse
if(!@ARGV) {
	print "No files provided!\n";
	pod2usage(-exitval => 1);
}

# DWARF debug_info entry names
my %DW = (
	'ABBREV'    => "Abbrev Number",
	'LINK_NAME' => "DW_AT_linkage_name",
	'NAME'      => "DW_AT_name",
	'NORET'     => "DW_AT_noreturn",
	'SUBPROG'   => "DW_TAG_subprogram"
);

# The name attributes to parse for each non-returning function
my @name_attrs = @DW{'NAME','LINK_NAME'};

for my $file (@ARGV) {
	if(!-f $file) {
		print "'$file' is not a regular file; skipping";
		next;
	}
	
	print "\nProcessing '$file'...\n";

	# Map function names to their address in the file (and vice versa)
	my ($Addr2Name, $Name2Addr) = &get_names($file);

	# Find all the DWARF subprogram entries for the file
	my @dwarf_entries = &parse_dwarf($file);
	
	# Find the non-returning functions
	my @no_return_funcs = grep { /$DW{'NORET'}/ } @dwarf_entries;
	
	# Parse the DWARF name attributes
	my @funcs = map { &parse_item($_, \@name_attrs) } @no_return_funcs;
	
	# Match the DWARF names with their addresses
	map { &match($_, \@name_attrs, $Addr2Name, $Name2Addr) } @funcs;
	
	# Display the _unique_ function names
	my %uniq = ();
	for my $f (@funcs) {
		if($verbose) {
			print "\n", '-'x60, "\n";
			print "${$f->{'item'}}\n";
			
			# Inlined functions have no address
			if($f->{'addr'}) {
				print "Found function(s) at address $f->{'addr'}:\n";
			}
		}

		for my $n (@{$f->{'names'}}) {
			next if exists $uniq{$n};
			$uniq{$n}++;
			if ($dyninst) {
				print "        (\"$n\", true)\n";
			} elsif ($verbose) {
				print "\t$n\n";
			} else {
				print "$n\n";
			}
		}
	}
	if($verbose) {
		print '='x60, "\n";
	}
}

#-----------------------------------------------------------------------
#
#  Use nm to create two mappings: address to name(s) and name to address
#
#----------------------------------------------------------------------
# A typical output of nm looks something like this:
#
#	0000000000201020 B __bss_start
#	0000000000201020 b completed.7932
#	                 w __cxa_finalize
#----------------------------------------------------------------------
sub get_names {
	my $file = $_[0];
	
	my %Addr2Name = ();
	my %Name2Addr = ();

	open(my $fdNM, "nm $file |") or die "unable to run 'nm $file'";

	while(my $line = <$fdNM>) {
		my ($addr, $name) = (split /[\s]+/, $line)[0,-1];

		if($addr =~ /^0/) {
			push @{$Addr2Name{$addr}}, $name;
			$Name2Addr{$name} = $addr;
		}
	}
	
	return (\%Addr2Name, \%Name2Addr);
}

#----------------------------------------------------------------------
#
#  Parse the dwarf info to find subprogram items.
#  An item begins with the DW_TAG_subprogram and ends with the next
#  Abbrev Number line.
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
sub parse_dwarf {
	my $file = $_[0];
	
	open(my $fdDWARF, "objdump -Wi $file |") or die "unable to run 'objdump $file'";
	
	my @items = ();

  	OUTER_LOOP:
  	while(my $line = <$fdDWARF>) {

		# skip ahead until DW_TAG_subprogram
		while($line !~ /$DW{'SUBPROG'}/) {
			$line = <$fdDWARF>;
			if(!$line) {
				last OUTER_LOOP;
			}
		}
		
		# Save the Abbrev line as the first line of the next item
		my $item = $line;		

		# Combine lines until the next 'Abbrev Number'
		while($line = <$fdDWARF>) {
			if($line =~ /$DW{'ABBREV'}/) {
				# Save the current item
				push @items, $item;
				redo OUTER_LOOP;
			}
			$item .= $line;
		}
	}
	
	return @items;
}

#----------------------------------------------------------------------
#
#  Parse one noreturn subprogram item and pick out the given attributes.
#
#----------------------------------------------------------------------
sub parse_item {
	my ($item, $name_attrs) = @_;
	
	my %res = ();
	for my $attr (@{$name_attrs}) {
		if($item =~ /$attr.*$/m ) {
			$res{$attr} = (split /[\s]+/, $&)[-1];
		}
	}
	$res{'item'} = \$item;
	
	return \%res;
}

#----------------------------------------------------------------------
#
#  Match the DWARF names with the addresses parsed from the binary,
#  including any aliases
#
#----------------------------------------------------------------------
sub match {
	my ($item, $name_attrs, $Addr2Name, $Name2Addr) = @_;
	
	for my $attr (@{$name_attrs}) {
		# Not every function has both attribute names
		next unless defined $item->{$attr};
		my $name = $item->{$attr};
		
		# Skip functions without an address
		next unless exists $Name2Addr->{$name};
		my $addr = $Name2Addr->{$name};

		foreach my $nm (@{$Addr2Name->{$addr}}) {
			# strip @GLIBC_x.y.z, if exists
			$nm =~ s/[@]glibc.*$//i;
			push @{$item->{'names'}}, $nm;
		}
		$item->{'addr'} = $addr;
	}
}

__END__

=head1 DESCRIPTION

Scan a binary file (libc.so, libstdc++.so, etc) and report function names
that have the DWARF attribute DW_AT_noreturn.

To use, first build glibc (direct, via autotools) and gcc (via spack) with
debug/dwarf info (-g) and run this script on various libraries. e.g.,

    libc.so
    libpthread.so
    libstdc++.so
    libgfortran.so
    libgomp.so

=head1 SYNOPSIS

noreturn.pl [options] file.so [file2.so...]

  Options:
    -d  format output for dyninst CodeSource.C
    -v  verbose output
    -h  show detailed help message
=cut
