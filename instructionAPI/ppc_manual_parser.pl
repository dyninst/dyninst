#!/usr/bin/perl

use HTML::LinkExtor;
use HTML::Parser;
use LWP::UserAgent;
use URI::URL;
use strict;

my $url = "http://publibn.boulder.ibm.com/doc_link/en_US/a_doc_lib/aixassem/alangref/frame9_toc.htm";

my %unique_links;
sub callback {
    my ($tag, %attr) = @_;
    return if $tag ne 'a';
    foreach my $v (values %attr)
    {
	$unique_links{$v} = 1 unless $v =~ /javascript|appendix|instr/;
    }

}
my $ua = LWP::UserAgent->new;

my $p = HTML::LinkExtor->new(\&callback);

my $res = $ua->request(HTTP::Request->new(GET => $url),
		    sub { $p->parse($_[0])});

my $base = $res->base;


my @links = keys %unique_links;
for (@links) { s/\#.*$//; }
@links = map {$_ = url($_, $base)->abs; } (@links);

my $column = 0;
my $mnemonic;
my %mnemonic_info;
my $suffix = undef;

sub set_mnemonic
{
    my ($self, $text) = @_;
    ($mnemonic) = ($text =~ /(\S*)\s+/);
    $self->handler(text => "");
}

sub handle_text
{
    my($self, $text) = @_;
    chomp $text;
    return unless($text =~ /^[\w|\d|\/|\s]+$/);
    $text =~ s/^A$/RA/;
    $text =~ s/^B$/RB/;
    $text =~ s/^S$/RS/;
    $text =~ s/^T$/RT/;
    $text =~ s/^SIMM$/SI/;
    $text =~ s/^SI\/D$/SI/;
    $text =~ s/^SI\/UI$/SI/;
    $text =~ s/^sh$/SH/;
    $text =~ s/^me$/ME/;
    $text =~ s/^mb$/MB/;
    $text =~ s/^ds$/DS/;
    $mnemonic_info{$mnemonic} .= "$text ";
    $self->handler(text => "");
}


sub handle_table_entries
{
    my($self, $tag, $attr) = @_;
    if($tag =~ /td/i)
    {
	if($attr->{'headers'} =~ /COL2/i)
	{
	    $self->handler(text => \&handle_text, "self, dtext");
	}
    }
}

sub restore_default_handler
{
    my ($self, $tag) = @_;
    if($tag =~ /table/i)
    {
	$self->handler(start => \&check_table_headers, "self, tagname, attr");
    }
}


sub check_for_value
{
    my($self, $text) = @_;
    if($text =~ /Value/)
    {
	$self->handler(start => \&handle_table_entries, "self, tagname, attr");
	$self->handler(end => \&restore_default_handler, "self, tagname");
	if(defined $suffix)
	{
	    $mnemonic .= $suffix;
	}
	else
	{
	    $suffix = "s";
	}
    }
}

sub check_table_headers
{
    my($self, $tag, $attr) = @_;
    if($tag =~ /th/i)
    {
	if($attr->{'id'} =~ /COL2/i)
	{
	    $self->handler(text => \&check_for_value, "self, dtext");
	}
    }
    if($tag =~ /h2/i)
    {
	$self->handler(text => \&set_mnemonic, "self, dtext");
    }
}


my $instructionParser = HTML::Parser->new(api_version => 3,
				       handlers => [start => [\&check_table_headers, "self, tagname, attr"],
						    ],
				       marked_sections => 1);

while (my $url = shift(@links))
{
    $mnemonic = undef;
    $suffix = undef;
    $instructionParser->handler(start => \&check_table_headers, "self, tagname, attr");
    $res = $ua->request(HTTP::Request->new(GET => $url),
		    sub { $instructionParser->parse($_[0])});

}


my @main_opcode_array;
my %extended_opcode_array;
my %mnemonics;

foreach my $k(keys %mnemonic_info)
{
    my @vals = split(/\s+/, $mnemonic_info{$k});
    my $primary_opcode = shift @vals;
    unshift(@vals, $k);
    if(exists $main_opcode_array[$primary_opcode])
    {
	my $i = $#vals;
	my $extended_opcode;
	while($i > 0 and not $extended_opcode)
	{
	    --$i;
	    $extended_opcode = $vals[$i] if ($vals[$i] =~ /^\d+$/);
	}
	if(defined $extended_opcode)
	{
	    if($main_opcode_array[$primary_opcode][0] ne "extended")
	    {
		my $tmp_extended_opcode;
		my $aref = $main_opcode_array[$primary_opcode];
		my @tmp_vals = @$aref;
		$i = $#tmp_vals;
		while($i > 0 and not $tmp_extended_opcode)
		{
		    --$i;
		    $tmp_extended_opcode = $tmp_vals[$i] if ($tmp_vals[$i] =~ /^\d+$/);
		}
		print "@tmp_vals\n" if $tmp_vals[0] =~ /D/;
		$extended_opcode_array{$primary_opcode}{$tmp_extended_opcode} = [@tmp_vals];
		$main_opcode_array[$primary_opcode] = ["extended"];
	    }
	    print "@vals\n" if $vals[0] =~ /D/;
	    $extended_opcode_array{$primary_opcode}{$extended_opcode} = [@vals];
	    $mnemonics{$vals[0]} = 1;

	}
    }
    else
    {
	$main_opcode_array[$primary_opcode] = [@vals];
	$mnemonics{$vals[0]} = 1;
    }
}

my %size_to_result_type = (
			   b => 'u8',
			   h => 'u16',
			   w => 'u32',
			   d => 'u64',
			   q => 'u64',
			   fs => 'sp_float',
			   fd => 'dp_float',
			   fiw => 'u32',
			   fq => 'dbl128',
			   mw => 'u32',
			   sw => 'u8',
			   scb => 'u8');

sub print_table_entry
{
    my ($primary_opcode, @data) = @_;
    my @opcode_data;
    map {push (@opcode_data, $_) unless /^[\/\d]*$/; } @data;
    my $mnem = shift @opcode_data;
    if(!$mnem)
    {
	$mnem = "INVALID";
    }
    my ($load, $width, $update, $index) = $mnem =~ /^(l|st)((?:f|m|s|sc|fi)?\w?)(?:z|ar|a|br|c|i)?(u?)(x?)\.?$/;

    for my $i(0..$#opcode_data)
    {
	if($opcode_data[$i] eq "D" and $i < $#opcode_data)
	{
	    $opcode_data[$i] = "RT";
	}
	if($load and $opcode_data[$i] eq "RA")
	{
	    my $load_operand = $load . $update . $index;
	    $load_operand =~ tr/[a-z]/[A-Z]/;
	    splice @opcode_data, $i, 2, "$load_operand<$size_to_result_type{$width}>";
	}
    }

    my $operands = "operandSpec()";
    my $next_table = "NULL";
    if($mnem eq "extended")
    {
	$next_table = "fn(extended_op_$primary_opcode)";
    }
    if(@opcode_data)
    {
	$operands = "list_of(fn(";
	$operands .= join("))(fn(", @opcode_data);
	$operands .= "))";
    }
    my $enum = $mnem;
    $enum =~ s/\./_rc/g;
    print "power_op_$enum, \"$mnem\", $next_table, $operands";

}

print "enum powerOpcodes {\n";
for my $mnem(keys %mnemonics)
{
    $mnem =~ s/\./_rc/g;
    print "\tpower_op_$mnem,\n";
}
print "};\n";

print "power_entry power_entry::main_opcode_table\[\] = {\n";
for my $i(0..$#main_opcode_array)
{
    print "\t{";
    print_table_entry($i, @{$main_opcode_array[$i]});
    print "},\n";
}
print "};\n";

foreach my $primary(sort {$a <=> $b} keys %extended_opcode_array)
{
    print "std::map<unsigned int,power_entry> power_entry::extended_op_$primary = map_list_of\n";
    foreach my $extended(sort {$a <=> $b} keys %{$extended_opcode_array{$primary}})
    {
	print "\t($extended, power_entry(";
	print_table_entry($primary, @{$extended_opcode_array{$primary}{$extended}});
	print "))\n";
    }
    print ";\n"
}
