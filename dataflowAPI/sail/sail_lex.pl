#!/usr/bin/env perl

use strict;
use warnings;

sub tokenize {
    my ($string) = @_;
    my @tokens;
    my $index = 0;
    while ($index < length($string)) {
        my $char = substr($string, $index, 1);
        my $token_str = "";
        my $token_type = "";
        if ($char =~ /[a-zA-Z_\']/) {
            my $ident = $char;
            while ($index + 1 < length($string)) {
                my $next_char = substr($string, $index + 1, 1);
                if ($next_char !~ /[a-zA-Z_]/) {
                    $index++;
                    last;
                }
                $index++;
                $ident .= $next_char;
            }
            if ($index + 1 == length($string)) {
                $index++;
            }

            $token_str = $ident;
            if ($ident eq "undefined") {
                $token_type = "L_UNDEFINED";
            }
            elsif ($ident eq "true" || $ident eq "false") {
                $token_type = "L_BOOL";
            }
            elsif ($ident eq "let") {
                $token_type = "L_LET";
            }
            elsif ($ident eq "match") {
                $token_type = "L_MATCH";
            }
            elsif ($ident eq "if") {
                $token_type = "L_IF";
            }
            elsif ($ident eq "then") {
                $token_type = "L_THEN";
            }
            elsif ($ident eq "else") {
                $token_type = "L_ELSE";
            }
            elsif ($ident eq "assert") {
                $token_type = "L_ASSERT";
            }
            elsif ($ident eq "sizeof") {
                $token_type = "L_SIZEOF";
            }
            else {
                $token_type = "L_IDENT";
            }

        }
        elsif ($char =~ /[0-9]/) {
            $token_type = "L_NUMBER";
            if (substr($string, $index, 2) eq "0b") {
                $token_str .= "0b";
                $index += 2;
                $token_type = "L_BINARY";
            }
            elsif (substr($string, $index, 2) eq "0x") {
                $token_str .= "0x";
                $index += 2;
                $token_type = "L_HEX";
            }
            while ($index + 1 < length($string)) {
                my $next_char = substr($string, $index + 1, 1);
                if ($next_char !~ /[0-9]/) {
                    $index++;
                    last;
                }
                $index++;
                $token_str .= $next_char;
            }
            if ($index + 1 == length($string)) {
                $index++;
            }
        }
        elsif ($char =~ /[\+\-\&\|\^\@\*]/) { # single byte operators and separators
            $token_str .= $char;
            $token_type = "L_OP";
            $index++;
        }
        elsif ($char =~ /[\,]/) {
            $token_str .= ",";
            $token_type = "L_COMMA";
            $index++;
        }
        elsif ($char =~ /[\;]/) {
            $token_str .= ",";
            $token_type = "L_SEMICOLON";
            $index++;
        }
        elsif ($char =~ /[\:]/) {
            $token_str .= ":";
            $token_type = "L_COLON";
            $index++;
        }
        elsif ($char =~ /[\.]/) {
            if (substr($string, $index, 2) eq "..") {
                $token_str .= "..";
                $token_type = "L_RANGE";
                $index += 2;
            }
            else {
                $token_str .= ".";
                $token_type = "L_DOT";
                $index++;
            }
        }
        elsif ($char =~ /[\=]/) {
            if (substr($string, $index, 2) eq "==") {
                $token_str .= "==";
                $token_type = "L_EQUAL";
                $index += 2;
            }
            elsif (substr($string, $index, 2) eq "=>") {
                $token_str .= "=>";
                $token_type = "L_ARROW";
                $index += 2;
            }
            else {
                $token_str .= "=";
                $token_type = "L_ASSIGN";
                $index++;
            }
        }
        elsif ($char =~ /[\!]/) {
            if (substr($string, $index, 2) eq "!=") {
                $token_str .= "!=";
                $token_type = "L_NOT_EQUAL";
                $index += 2;
            }
            else {
                die "Invalid char $char at index $index\n";
            }
        }
        elsif ($char =~ /[\<]/) { # <<, <_s, <_u, <=_s, <=_u
            if (substr($string, $index, 2) eq "<<") {
                $token_str .= "<<";
                $token_type = "L_SHIFT_LEFT";
                $index += 2;
            }
            elsif (substr($string, $index, 2) eq "<_") {
                my $su = substr($string, $index + 2, 1);
                if ($su eq "s") {
                    $token_str .= "<_s";
                    $token_type = "L_SIGNED_LESS_THAN";
                    $index += 3;
                }
                elsif ($su eq "u") {
                    $token_str .= "<_u";
                    $token_type = "L_UNSIGNED_LESS_THAN";
                    $index += 3;
                }
                else {
                    die "Invalid char $char at index $index\n";
                }
            }
            elsif (substr($string, $index, 3) eq "<=_") {
                my $su = substr($string, $index + 3, 1);
                if ($su eq "s") {
                    $token_str .= "<=_s";
                    $token_type = "L_SIGNED_LESS_THAN_EQUAL";
                    $index += 4;
                }
                elsif ($su eq "u") {
                    $token_str .= "<=_u";
                    $token_type = "L_UNSIGNED_LESS_THAN_EQUAL";
                    $index += 4;
                }
                else {
                    die "Invalid char $char at index $index\n";
                }
            }
            else {
                $token_str .= $char;
                $token_type = "L_LESS_THAN";
                $index++;
            }
        }
        elsif ($char =~ /[\>]/) { # >>, >_s, >_u, >=_s, >=_u
            if (substr($string, $index, 2) eq ">>") {
                $token_str .= ">>";
                $token_type = "L_SHIFT_RIGHT";
                $index += 2;
            }
            elsif (substr($string, $index, 2) eq ">_") {
                my $su = substr($string, $index + 2, 1);
                if ($su eq "s") {
                    $token_str .= ">_s";
                    $token_type = "L_SIGNED_GREATER_THAN";
                    $index += 3;
                }
                elsif ($su eq "u") {
                    $token_str .= ">_u";
                    $token_type = "L_UNSIGNED_GREATER_THAN";
                    $index += 3;
                }
                else {
                    die "Invalid char $char at index $index\n";
                }
            }
            elsif (substr($string, $index, 3) eq ">=_") {
                my $su = substr($string, $index + 3, 1);
                if ($su eq "s") {
                    $token_str .= ">=_s";
                    $token_type = "L_SIGNED_GREATER_THAN_EQUAL";
                    $index += 4;
                }
                elsif ($su eq "u") {
                    $token_str .= ">=_u";
                    $token_type = "L_UNSIGNED_GREATER_THAN_EQUAL";
                    $index += 4;
                }
                else {
                    die "Invalid char $char at index $index\n";
                }
            }
            else {
                $token_str .= $char;
                $token_type = "L_GREATER_THAN";
                $index++;
            }
        }
        elsif ($char =~ /[\"]/) {
            my $close_index = $index + 1;
            while ($close_index < length($string) && substr($string, $close_index, 1) ne '"') {
                $close_index++;
            }
            if ($close_index + 1 == length($string)) {
                die "Unmatched quotes at $index\n";
            }
            $token_str .= substr($string, $index, $close_index - $index + 1);
            $token_type = "L_STRING";
            $index += $close_index - $index + 2;
        }
        elsif ($char =~ /[\(\)\[\]\{\}]/) {
            if (substr($string, $index, 2) eq "()") {
                $token_str .= "()";
                $token_type = "L_UNIT";
                $index += 2;
            }
            elsif ($char eq "(") {
                $token_str .= "(";
                $token_type = "L_LEFT_PAREN";
                $index++;
            }
            elsif ($char eq ")") {
                $token_str .= ")";
                $token_type = "L_RIGHT_PAREN";
                $index++;
            }
            elsif ($char eq "[") {
                $token_str .= "[";
                $token_type = "L_LEFT_BRACK";
                $index++;
            }
            elsif ($char eq "]") {
                $token_str .= "]";
                $token_type = "L_RIGHT_BRACK";
                $index++;
            }
            elsif ($char eq "{") {
                $token_str .= "{";
                $token_type = "L_LEFT_BRACE";
                $index++;
            }
            elsif ($char eq "}") {
                $token_str .= "}";
                $token_type = "L_RIGHT_BRACE";
                $index++;
            }
        }
        elsif ($char =~ /[\/]/) { # comments
            my $next_char = substr($string, $index + 1, 1);
            if ($next_char eq "/") {
                $index++;
                while ($index < length($string) && substr($string, $index, 1) ne "\n") {
                    $index++;
                }
                $index++;
            }
            elsif ($next_char eq "*") {
                $index += 2;
                while ($index + 1 < length($string) &&
                    (substr($string, $index, 1) ne "*" || substr($string, $index + 1, 1) ne "/")) {
                    $index++;
                }
                if ($index + 1 == length($string)) {
                    die "Unmatched /* comments at $index\n";
                }
                $index += 2;
            }
            next; # ignore comments
        }
        elsif ($char =~ /[\s]/) {
            $index++;
            next; # ignore spaces
        }
        else {
            die "Invalid char $char at index $index\n";
        }
        print "$index: $token_str, $token_type\n";
        push(@tokens, [$token_str, $token_type]);
    }
    return @tokens;
}

1;
