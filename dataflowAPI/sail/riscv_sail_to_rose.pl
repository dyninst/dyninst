#!/usr/bin/env perl

# This is an extremely dumb parser that turns SAIL into ROSE semantics.
# Note: Not all lexical, syntax rules for the SAIL language are implemented.
# Only the minimal set of rules are implemented.

# Manual: https://alasdair.github.io/manual.html#_the_sail_grammar

use strict;
use warnings;

use File::Slurp;
use Carp::Assert;

require './sail_lex.pl';
require './sail_syntax.pl';
require './sail_ast.pl';

my @sail_model_files = ("riscv_insts_base.sail");
my $execute_pattern = qr/function\s+clause\s+execute\s+\(?(.*?)\((.*?)\)\)?\s*=\s*(\{[^{}]*(?:[^{}]*|(?3))[^{}]*\})/;
my $code_line_pattern = qr/(.*?);/;

foreach my $sail_model_file (@sail_model_files) {
    my $sail_model = read_file($sail_model_file);
    # find all "function clause execute" in the sail model file
    while ($sail_model =~ m/$execute_pattern/gs) {

        # Get the instruction type
        my $insn_type = uc($1);

        # The structures of arguments are pretty simple, just use basic regex to parse it
        # There's no point of using a lexer to parse it
        my @insn_type_args = split(/\s*,\s*/, $2);

        # The instruction code is more complex, we should perform lexical and syntax analysis
        # Semantic analysis is not done because, y know, the SAIL source code is already semantically correct :)
        my $insn_code = $3;
        my @tokens = tokenize($insn_code);
        build_ast(@tokens);
        my $z = <STDIN>;
    }
}

# function\s+clause\s+execute\s+\(?(.*?)\((.*?)\)\)?\s*=\s*(\{[^{}]*(?:[^{}]*|(?3))[^{}]*\})
