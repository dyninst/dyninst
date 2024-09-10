#!/usr/bin/env perl

use strict;
use warnings;

sub parse_block {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref}[0];
    my ($la1_token_str, $la1_token_type) = @{$tokens_ref}[1];
    my $ast;

    if ($la1_token_type eq "L_LET") {
        shift(@$tokens_ref);
        my $letbind_ast = parse_letbind($tokens_ref);
        ($la1_token_str, $la1_token_type) = @{$tokens_ref}[1];
        if ($la1_token_type eq "L_SEMICOLON") {
            shift(@$tokens_ref);
        }

    }
}

sub parse_exp {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref}[0];
    my ($la1_token_str, $la1_token_type);
    if (scalar(@$tokens_ref) > 1) {
        ($la1_token_str, $la1_token_type) = @{$tokens_ref}[1];
    }

    if ($la1_token_type eq "L_LEFT_BRACE") {
        shift(@$tokens_ref);
        my $block_ast = parse_block($tokens_ref);
        ($token_str, $token_type) = @{$tokens_ref}[0];
        if ($token_type ne "L_RIGHT_BRACE") {
            die "parse_exp: unexpected token $token_str";
        }
        shift(@$tokens_ref);
        return $block_ast;
    }
    elsif ($la1_token_type eq "L_IF") {
        shift(@$tokens_ref);
        my $if_ast = parse_exp($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref}[0];
        if ($token_type ne "L_THEN") {
            die "parse_exp: unexpected token $token_str";
        }
        shift(@$tokens_ref);
        my $then_ast = parse_exp($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref}[0];
        if ($token_type eq "L_ELSE") {
            my $else_ast = parse_exp($tokens_ref);

            $then_ast->_right_node = $else_ast;
            $then_ast->_parent_node = $if_ast;
            $else_ast->_left_node = $then_ast;
            $else_ast->_parent_node = $if_ast;
            $if_ast->_child_nodes = ($then_ast, $else_ast);
        }
        else {
            $then_ast->_parent_node = $if_ast;
            $if_ast->_child_nodes = ($then_ast);
        }
    }
    elsif ($la1_token_type eq "L_MATCH") {
        shift(@$tokens_ref);
        my $match_ast = parse_exp($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref}[0];
        if ($token_type ne "L_LEFT_BRACE") {
            die "parse_exp: unexpected token $token_str";
        }
        shift(@$tokens_ref);

        my $case_list = parse_caselist($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref}[0];
        if ($token_type ne "L_RIGHT_BRACE") {
            die "parse_exp: unexpected token $token_str";
        }
    }
}

1;
