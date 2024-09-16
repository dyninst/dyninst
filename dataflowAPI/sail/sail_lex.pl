#!/usr/bin/env perl

use strict;
use warnings;

sub tokenize {
    my ($string) = @_;
    my @tokens;
    my $index = 0;
    while (length($string) > 0) {
        my $token_str = "";
        my $token_type = "";

        # Note that the regex order is important!
        my %regexes = (
            "L_UNDEFINED" => qr/^(undefined)/,
            "L_TRUE" => qr/^(true)/,
            "L_FALSE" => qr/^(false)/,
            "L_LET" => qr/^(let)/,
            "L_MATCH" => qr/^(match)/,
            "L_IF" => qr/^(if)/,
            "L_THEN" => qr/^(then)/,
            "L_ELSE" => qr/^(else)/,
            "L_ASSERT" => qr/^(assert)/,
            "L_SIZEOF" => qr/^(sizeof)/,
            "L_IDENT" => qr/^([0-9a-zA-Z_\']+)/,

            "L_BIN" => qr/^(0b[0-1_]+)/,
            "L_HEX" => qr/^(0x[A-Fa-f0-9_]+)/,
            "L_POWER" => qr/^(2\s*\^)/,

            "L_STRING" => qr/^"([^"\\]|\\.)*"/,

            "L_ADD" => qr/^(\+)/,
            "L_SUB" => qr/^(\-)/,
            "L_AND" => qr/^(\&)/,
            "L_OR" => qr/^(\|)/,
            "L_XOR" => qr/^(\^)/,
            "L_AT" => qr/^(\@)/,
            "L_MUL" => qr/^(\*)/,

            "L_COMMA" => qr/^(\,)/,
            "L_COLON" => qr/^(\:)/,
            "L_SEMICOLON" => qr/^(\;)/,

            "L_RANGE" => qr/^(\.\.)/,
            "L_DOT" => qr/^(\.)/,

            "L_EQUAL" => qr/^(\=\=)/,
            "L_ARROW" => qr/^(\=\>)/,
            "L_ASSIGN" => qr/^(\=)/,

            "L_NOT_EQUAL" => qr/^(\!\=)/,

            "L_SHIFT_LEFT" => qr/^(\<\<)/,
            "L_LESS_THAN" => qr/^(\<)/,
            "L_LESS_THAN_EQUALS" => qr/^(\<\=)/,
            "L_LESS_THAN_SIGNED" => qr/^(\<_s)/,
            "L_LESS_THAN_UNSIGNED" => qr/^(\<_u)/,
            "L_LESS_THAN_EQUALS_SIGNED" => qr/^(\<\=_s)/,
            "L_LESS_THAN_EQUALS_UNSIGNED" => qr/^(\<\=_u)/,

            "L_SHIFT_RIGHT" => qr/^(\>\>)/,
            "L_GREATER_THAN" => qr/^(\>)/,
            "L_GREATER_THAN_EQUALS" => qr/^(\>\=)/,
            "L_GREATER_THAN_SIGNED" => qr/^(\>_s)/,
            "L_GREATER_THAN_UNSIGNED" => qr/^(\>_u)/,
            "L_GREATER_THAN_EQUALS_SIGNED" => qr/^(\>\=_s)/,
            "L_GREATER_THAN_EQUALS_UNSIGNED" => qr/^(\>\=_u)/,

            "L_UNIT" => qr/^(\(\))/,
            "L_LEFT_PAREN" => qr/^(\()/,
            "L_RIGHT_PAREN" => qr/^(\))/,
            "L_LEFT_BRACK" => qr/^(\[)/,
            "L_RIGHT_BRACK" => qr/^(\])/,
            "L_LEFT_BRACE" => qr/^(\{)/,
            "L_RIGHT_BRACE" => qr/^(\})/,

            # Comments and spaces
            "L_SINGLE_LINE_COMMENT" => qr/^\/\/.*\n/,
            "L_DOUBLE_LINE_COMMENT" => qr/^\/\*.*\*\//,
            "L_SPACE" => qr/^\s+/,
        );

        my $match = 0;
        while (my ($token_type, $regex) = each %regexes) {
            if ($string =~ /$regex/gms) {
                $match = 1;
                $string =~ s/$regex//s;
                if (defined $1) {
                    push(@tokens, [$1, $token_type]);
                    print "$1, $token_type\n";
                    last;
                }
            }
        }
        if (!$match) {
            die "Unexpected token\n";
        }

    }
    return @tokens;
}

1;
