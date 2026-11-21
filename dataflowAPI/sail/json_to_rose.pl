#!/usr/bin/env perl

use strict;
use warnings;

use Carp::Assert;
use File::Slurp qw(read_file);
use FindBin;
use Getopt::Long qw(GetOptions);
use JSON qw(decode_json);
use Pod::Usage;

use lib "$FindBin::Bin";
use ParserConfig;
use PrintROSE;

sub load_json_ast {
    my ($file) = @_;
    -f $file or die "Error: JSON file '$file' not found\n";
    my $json_str = read_file($file);
    my $json     = decode_json($json_str);
    return $json->{"ast"} // die "Error: 'ast' field missing in JSON\n";
}

sub find_execute_for_set {
    my ($ast, $insn_set) = @_;

    foreach my $def (@$ast) {
        next unless exists $def->{DEF_fundef};

        my $functions = $def->{DEF_fundef}->{FD_function}->[2];
        foreach my $function (@$functions) {
            my $clause = $function->{"FCL_funcl"} or next;

            my $name = $clause->[0]->{"Id"} // "";
            next unless $name eq "execute";

            my $pat_exp = $clause->[1]->{Pat_exp};
            my ($head, $body) = @$pat_exp;

            my $fun_name = $head->{P_app}[0]->{Id} // "";
            next unless $fun_name eq $insn_set;

            my $args = $head->{P_app}[1]->[0]->{P_tuple}->[0];
            return ($body, $args);
        }
    }
    die "Error: instruction set '$insn_set' not found or unsupported\n";
}

sub main {

    my $insn_set;
    my $json_file = "sail_semantics.json";

    GetOptions(
        "insn-set=s"  => \$insn_set,
        "json-file=s" => \$json_file,
        "help"        => sub { pod2usage(1) },
        "man"         => sub { pod2usage(-verbose => 2) },
    ) or pod2usage(2);

    pod2usage(2) unless $insn_set;

    my $ast = load_json_ast($json_file);
    my ($body, $args) = find_execute_for_set($ast, $insn_set);

    PrintROSE->print_rose_code($body, $args, $insn_set);

    exit 0;
}

main();

__END__

=head1 NAME

json_to_rose.pl - Convert Sail semantics JSON into ROSE code

=head1 SYNOPSIS

json_to_rose.pl --insn-set=<INSTRUCTION_SET> [--json-file=<FILE>]

=head1 DESCRIPTION

This script reads a Sail semantics JSON file (default: F<sail_semantics.json>),
searches for the C<execute> function inside the specified instruction set,
and prints the corresponding ROSE code.

=head1 OPTIONS

=over 4

=item B<--insn-set>=I<SET>

Required. The instruction set name to extract (e.g., C<UTYPE>).

=item B<--json-file>=I<FILE>

Optional. Path to the Sail JSON file. Defaults to F<sail_semantics.json>.

=item B<--help>

Print this help message.

=item B<--man>

Show full documentation.

=back

=head1 AUTHOR

Angus He

=cut

