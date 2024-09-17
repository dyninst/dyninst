#!/usr/bin/env perl

use strict;
use warnings;

# Manual: https://alasdair.github.io/manual.html#_the_sail_grammar

sub parse_atomic_exp {
    my ($tokens_ref) = @_;

    my ($token_str, $token_type) = @{$tokens_ref->[0]};

    my ($la1_token_str, $la1_token_type) = ("", "L_EPSILON");
    if (scalar(@$tokens_ref) > 1) {
        ($la1_token_str, $la1_token_type) = @{$tokens_ref->[1]};
    }

    if ($la1_token_type eq "L_IDENT") {
        #
        # <id>
        #
        my $id_ast = parse_id($tokens_ref);

        if (scalar(@$tokens_ref) == 0) {
            return $id_ast;
        }

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_LEFT_PAREN") {
            #
            # (
            #
            shift(@$tokens_ref);

            ($token_str, $token_type) = @{$tokens_ref->[0]};
            if ($token_type eq "L_RIGHT_PAREN") {
                #
                # )
                #
                shift(@$tokens_ref);

                my $func_ast = ASTNode->new("<FUNC>", "AST_FUNC", undef, undef, undef, ());
                $id_ast->_parent_node = $func_ast;
                push(@{$func_ast->_child_nodes}, $id_ast);

                return $func_ast;
            }
            else {
                #
                # <pat_list>
                #
                my $pat_list_ast = parse_pat_list($tokens_ref);

                #
                # )
                #
                shift(@$tokens_ref);

                my $func_ast = ASTNode->new("<FUNC>", "AST_FUNC", undef, undef, undef, ());

                $id_ast->_parent_node = $func_ast;
                push(@{$func_ast->_child_nodes}, $id_ast);

                my $curr = $pat_list_ast;
                while (defined $curr) {
                    $curr->_parent_node = $func_ast;
                    push(@{$func_ast->_child_nodes}, $curr);
                    $curr = $curr->_right_node;
                }

                return $func_ast;
            }
        }
        else {
            return $id_ast;
        }
    }
    elsif ($la1_token_type eq "L_LEFT_PAREN") {
        my $tuple_ast = ASTNode->new("<TUPLE>", "AST_TUPLE", undef, undef, undef, ());

        #
        # (
        #
        shift(@$tokens_ref);

        #
        # <pat>
        #

        my $pat_ast = parse_pat($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_RIGHT_PAREN") {
            #
            # )
            #
            shift(@$tokens_ref);

            $pat_ast->_parent_node = $tuple_ast;
            push(@{$tuple_ast->_child_nodes}, $pat_ast);

            return $tuple_ast;
        }
        elsif ($token_type eq "L_COMMA") {
            #
            # ,
            #
            shift(@$tokens_ref);

            #
            # <pat_list>
            #

            my $pat_list_ast = parse_pat_list($tokens_ref);

            $pat_ast->_right_node = $pat_list_ast;
            $pat_list_ast->_left_node = $pat_ast;

            my $cur_node = $pat_ast;
            while (defined $cur_node) {
                $cur_node->_parent_node = $tuple_ast;
                push(@{$tuple_ast->_child_nodes}, $cur_node);
                $cur_node = $cur_node->_right_node;
            }

            #
            # )
            #
            shift(@$tokens_ref);

            return $tuple_ast;
        }
    }
    elsif ($la1_token_type eq "L_BOOL" || $la1_token_type eq "L_UNIT" || $la1_token_type eq "L_NUMBER"
        || $la1_token_type eq "L_UNDEFINED" || $la1_token_type eq "L_BIN" || $la1_token_type eq "L_HEX"
        || $la1_token_type eq "L_STRING") {
        #
        # <lit>
        #
        my $lit_ast = parse_lit($tokens_ref);
    }
    else {
        my $atomic_exp_ast = parse_atomic_exp($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_DOT") {
            my $dot_ast = ASTNode->new("<DOT>", "AST_DOT", undef, undef, undef, ());

            #
            # .
            #
            shift(@$tokens_ref);

            #
            # <id>
            #
            my $id_ast = parse_id($tokens_ref);

            ($token_str, $token_type) = @{$tokens_ref->[0]};
            if ($token_type eq "L_LEFT_PAREN") {
                #
                # (
                #
                shift(@$tokens_ref);

                ($token_str, $token_type) = @{$tokens_ref->[0]};
                if ($token_type eq "L_RIGHT_PAREN") {
                    #
                    # )
                    #
                    shift(@$tokens_ref);


                    $dot_ast->_parent_node = $atomic_exp_ast;
                    $id_ast->_parent_node = $dot_ast;
                    push(@{$atomic_exp_ast->_child_nodes}, $dot_ast);
                    push(@{$dot_ast->_child_nodes}, $id_ast);

                    my $func_ast = ASTNode->new("<FUNC>", "AST_FUNC", undef, undef, undef, ());
                    $atomic_exp_ast->_parent_node = $func_ast;
                    push(@{$func_ast->_child_nodes}, $atomic_exp_ast);

                    return $func_ast;
                }
                else {
                    #
                    # <pat_list>
                    #
                    my $pat_list_ast = parse_pat_list($tokens_ref);

                    #
                    # )
                    #
                    shift(@$tokens_ref);

                    $dot_ast->_parent_node = $atomic_exp_ast;
                    $id_ast->_parent_node = $dot_ast;
                    push(@{$atomic_exp_ast->_child_nodes}, $dot_ast);
                    push(@{$dot_ast->_child_nodes}, $id_ast);

                    my $func_ast = ASTNode->new("<FUNC>", "AST_FUNC", undef, undef, undef, ());

                    $atomic_exp_ast->_parent_node = $func_ast;
                    push(@{$func_ast->_child_nodes}, $atomic_exp_ast);

                    my $curr = $pat_list_ast;
                    while (defined $curr) {
                        $curr->_parent_node = $func_ast;
                        push(@{$func_ast->_child_nodes}, $curr);
                        $curr = $curr->_right_node;
                    }

                    return $func_ast;
                }
            }
            else {
                $dot_ast->_parent_node = $atomic_exp_ast;
                $id_ast->_parent_node = $dot_ast;
                push(@{$atomic_exp_ast->_child_nodes}, $dot_ast);
                push(@{$dot_ast->_child_nodes}, $id_ast);

                return $atomic_exp_ast;
            }
        }
        elsif ($token_type eq "L_LEFT_BRACK") {
            #
            # [
            #
            shift(@$tokens_ref);

            #
            # <exp>
            #
            my $exp1_ast = parse_exp($tokens_ref);

            if ($token_type eq "L_RANGE") {
                #
                # ..
                #
                shift(@$tokens_ref);

                #
                # NUMBER
                #
                my $exp2_ast = parse_exp($tokens_ref);

                #
                # ]
                #
                shift(@$tokens_ref);

                my $range_ast = ASTNode->new("<RANGE>", "AST_RANGE", undef, undef, undef, ());
                $atomic_exp_ast->_right_node = $exp1_ast;
                $exp1_ast->_left_node = $atomic_exp_ast;
                $exp1_ast->_right_node = $exp2_ast;
                $exp2_ast->_left_node = $exp1_ast;
                $atomic_exp_ast->_parent_node = $exp1_ast->_parent_node = $exp2_ast->_parent_node = $range_ast;
                push(@{$range_ast->_child_nodes}, $atomic_exp_ast);
                push(@{$range_ast->_child_nodes}, $exp1_ast);
                push(@{$range_ast->_child_nodes}, $exp2_ast);

                return $range_ast;
            }
            else {
                #
                # ]
                #
                shift(@$tokens_ref);

                my $index_ast = ASTNode->new("<INDEX>", "AST_INDEX", undef, undef, undef, ());
                $atomic_exp_ast->_right_node = $exp1_ast;
                $exp1_ast->_left_node = $atomic_exp_ast;
                $atomic_exp_ast->_parent_node = $index_ast;
                $exp1_ast->_parent_node = $index_ast;
                push(@{$index_ast->_child_nodes}, $atomic_exp_ast);
                push(@{$index_ast->_child_nodes}, $exp1_ast);

                return $index_ast;
            }
        }
    }
}

sub parse_atomic_pat {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref->[0]};
    my ($la1_token_str, $la1_token_type) = ("", "L_EPSILON");
    if (scalar(@$tokens_ref) > 1) {
        ($la1_token_str, $la1_token_type) = @{$tokens_ref->[1]};
    }

    if ($la1_token_type eq "L_IDENT") {
        #
        # <id>
        #
        my $id_ast = parse_id($tokens_ref);

        if (scalar(@$tokens_ref) == 0) {
            return $id_ast;
        }

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_LEFT_PAREN") {
            #
            # (
            #
            shift(@$tokens_ref);

            ($token_str, $token_type) = @{$tokens_ref->[0]};
            if ($token_type eq "L_RIGHT_PAREN") {
                #
                # )
                #
                shift(@$tokens_ref);

                $id_ast->_type = "AST_FUNC";

                return $id_ast;
            }
            else {
                #
                # <pat_list>
                #
                my $pat_list_ast = parse_pat_list($tokens_ref);

                my $curr = $pat_list_ast;
                while (defined $curr) {
                    $curr->_parent_node = $id_ast;
                    push(@{$id_ast->_child_nodes}, $curr);
                    $curr = $curr->_right_node;
                }

                #
                # )
                #
                shift(@$tokens_ref);

                $id_ast->_type = "AST_FUNC";

                return $id_ast;
            }
        }
        elsif ($token_type eq "L_LEFT_BRACK") {
            #
            # [
            #
            shift(@$tokens_ref);

            #
            # NUMBER
            #
            ($token_str, $token_type) = @{$tokens_ref->[0]};
            shift(@$tokens_ref);
            my $number1_ast = ASTNode->new($token_str, "AST_NUMBER", undef, undef, undef, ());

            ($token_str, $token_type) = @{$tokens_ref->[0]};
            if ($token_type eq "L_RANGE") {
                #
                # ..
                #
                shift(@$tokens_ref);

                #
                # NUMBER
                #
                ($token_str, $token_type) = @{$tokens_ref->[0]};
                shift(@$tokens_ref);
                my $number2_ast = ASTNode->new($token_str, "AST_NUMBER", undef, undef, undef, ());

                #
                # ]
                #
                shift(@$tokens_ref);

                my $range_ast = ASTNode->new("<RANGE>", "AST_RANGE", undef, undef, undef, ());
                $id_ast->_right_node = $number1_ast;
                $number1_ast->_left_node = $id_ast;
                $number1_ast->_right_node = $number2_ast;
                $number2_ast->_left_node = $number1_ast;
                $id_ast->_parent_node = $number1_ast->_parent_node = $number2_ast->_parent_node = $range_ast;
                push(@{$range_ast->_child_nodes}, $id_ast);
                push(@{$range_ast->_child_nodes}, $number1_ast);
                push(@{$range_ast->_child_nodes}, $number2_ast);

                return $range_ast;
            }
            else {
                #
                # ]
                #
                shift(@$tokens_ref);

                my $index_ast = ASTNode->new("<INDEX>", "AST_INDEX", undef, undef, undef, ());
                $id_ast->_right_node = $number1_ast;
                $number1_ast->_left_node = $id_ast;
                $id_ast->_parent_node = $index_ast;
                $number1_ast->_parent_node = $index_ast;
                push(@{$index_ast->_child_nodes}, $id_ast);
                push(@{$index_ast->_child_nodes}, $number1_ast);

                return $index_ast;
            }
        }
        else {
            return $id_ast;
        }
    }
    elsif ($la1_token_type eq "L_LEFT_PAREN") {
        my $tuple_ast = ASTNode->new("<TUPLE>", "AST_TUPLE", undef, undef, undef, ());

        #
        # (
        #
        shift(@$tokens_ref);

        #
        # <pat>
        #

        my $pat_ast = parse_pat($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_RIGHT_PAREN") {
            #
            # )
            #
            shift(@$tokens_ref);

            $pat_ast->_parent_node = $tuple_ast;
            push(@{$tuple_ast->_child_nodes}, $pat_ast);

            return $tuple_ast;
        }
        elsif ($token_type eq "L_COMMA") {
            #
            # ,
            #
            shift(@$tokens_ref);

            #
            # <pat_list>
            #

            my $pat_list_ast = parse_pat_list($tokens_ref);

            $pat_ast->_right_node = $pat_list_ast;
            $pat_list_ast->_left_node = $pat_ast;

            my $cur_node = $pat_ast;
            while (defined $cur_node) {
                $cur_node->_parent_node = $tuple_ast;
                push(@{$tuple_ast->_child_nodes}, $cur_node);
                $cur_node = $cur_node->_right_node;
            }

            #
            # )
            #
            shift(@$tokens_ref);

            return $tuple_ast;
        }
    }
    elsif ($la1_token_type eq "L_BOOL" || $la1_token_type eq "L_UNIT" || $la1_token_type eq "L_NUMBER"
        || $la1_token_type eq "L_UNDEFINED" || $la1_token_type eq "L_BIN" || $la1_token_type eq "L_HEX"
        || $la1_token_type eq "L_STRING") {
        #
        # <lit>
        #
        my $lit_ast = parse_lit($tokens_ref);
    }
}

sub parse_block {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref->[0]};
    my ($la1_token_str, $la1_token_type) = ("", "L_EPSILON");
    if (scalar(@$tokens_ref) > 1) {
        ($la1_token_str, $la1_token_type) = @{$tokens_ref->[1]};
    }

    #
    # let <letbind> [;]
    # let <letbind> ; <block>
    #
    if ($la1_token_type eq "L_LET") {
        #
        # let
        #
        shift(@$tokens_ref);

        #
        # <letbind>
        #
        my $letbind_ast = parse_letbind($tokens_ref);

        #
        # ;
        # [;]
        #
        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_SEMICOLON") {
            shift(@$tokens_ref);
        }

        #
        # <block>
        #
        if (scalar(@$tokens_ref) > 0) {
            my $block_ast = parse_block($tokens_ref);
            $letbind_ast->_right_node = $block_ast;
            $block_ast->_left_node = $letbind_ast;
        }

        return $letbind_ast;
    }
    # <exp> is pretty hard to do look ahead.
    # However, we're confident that the sail source code is correct, so we can safely assume that
    # anything that doesn't satisfy the previous lookahead is an <exp> expression
    else {
        #
        # <exp> ; <block>
        #
        my $exp_ast = parse_exp($tokens_ref);
        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type ne "L_SEMICOLON") {
            die "parse_block: expected ';'";
        }
        shift(@$tokens_ref);

        my $block_ast = parse_block($tokens_ref);

        $exp_ast->_right_node = $block_ast;
        $block_ast->_left_node = $exp_ast;

        return $exp_ast;
    }
}

sub parse_case {
    #
    # <pat> => <exp>
    # <pat> if <exp> => <exp>
    #

    my ($tokens_ref) = @_;
    my ($token_str, $token_type);
    my $pat_ast = parse_pat($tokens_ref);


    ($token_str, $token_type) = @{$tokens_ref->[0]};

    #
    # <pat> => <exp>
    #
    if ($token_type eq "L_MATCH_ARROW") {
        #
        # =>
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $exp_ast = parse_exp($tokens_ref);

        $exp_ast->_parent_node = $pat_ast;
        $pat_ast->_child_nodes = ($exp_ast);
    }

    #
    # <pat> if <exp> => <exp>
    #
    elsif ($token_type eq "L_IF") {
        #
        # if
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $exp1_ast = parse_exp($tokens_ref);

        #
        # =>
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $exp2_ast = parse_exp($tokens_ref);

        $exp1_ast->_parent_node = $pat_ast;
        $pat_ast->_child_nodes = ($exp1_ast);
        $exp2_ast->_parent_node = $exp1_ast;
        $exp1_ast->_child_nodes = ($exp2_ast);
    }

    return $pat_ast;
}

sub parse_case_list {
    my ($tokens_ref) = @_;

    #
    # <case>
    # <case> ,
    # <case> , <case_list>
    #
    my $case_ast = parse_case($tokens_ref);

    #
    # ,
    #
    if (scalar(@$tokens_ref) > 0) {
        shift(@$tokens_ref);

        if (scalar(@$tokens_ref) > 0) {
            my $case_list_ast = parse_case_list($tokens_ref);

            $case_ast->_right_node = $case_list_ast;
            $case_list_ast->_left_node = $case_ast;
        }
    }
    return $case_ast;
}

sub parse_exp {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref->[0]};
    my ($la1_token_str, $la1_token_type) = ("", "L_EPSILON");
    if (scalar(@$tokens_ref) > 1) {
        ($la1_token_str, $la1_token_type) = @{$tokens_ref->[1]};
    }

    #
    # { <block> }
    #
    if ($la1_token_type eq "L_LEFT_BRACE") {
        #
        # {
        #
        shift(@$tokens_ref);

        #
        # <block>
        #
        my $block_ast = parse_block($tokens_ref);

        #
        # }
        #
        shift(@$tokens_ref);

        return $block_ast;
    }
    #
    # if <exp> then <exp> else <exp>
    # if <exp> then <exp>
    #
    elsif ($la1_token_type eq "L_IF") {
        #
        # if
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $if_ast = parse_exp($tokens_ref);

        #
        # then
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $then_ast = parse_exp($tokens_ref);

        ($token_str, $token_type) = @{$tokens_ref->[0]};
        if ($token_type eq "L_ELSE") {
            #
            # else
            #
            shift(@$tokens_ref);

            #
            # <exp>
            #
            my $else_ast = parse_exp($tokens_ref);

            my $if_then_else_ast = ASTNode->new("<IF_THEN_ELSE>", "AST_IF_THEN_ELSE", undef, undef, undef, ());

            $then_ast->_right_node = $else_ast;
            $then_ast->_parent_node = $if_ast;
            $else_ast->_left_node = $then_ast;
            $else_ast->_parent_node = $if_ast;
            $if_ast->_parent_node = $if_then_else_ast;
            push(@{$if_then_else_ast->_child_nodes}, $if_ast);
            push(@{$if_ast->_child_nodes}, $then_ast);
            push(@{$if_ast->_child_nodes}, $else_ast);

            return $if_then_else_ast;
        }
        else {
            my $if_then_ast = ASTNode->new("<IF_THEN>", "AST_IF_THEN", undef, undef, undef, ());

            $if_ast->_parent_node = $if_then_ast;
            $then_ast->_parent_node = $if_ast;
            push(@{$if_then_ast->_child_nodes}, $if_ast);
            push(@{$if_ast->_child_nodes}, $then_ast);

            return $if_then_ast;
        }
    }
    # match <exp> { <case_list> }
    elsif ($la1_token_type eq "L_MATCH") {
        #
        # match
        #
        shift(@$tokens_ref);

        #
        # <exp>
        #
        my $match_ast = parse_exp($tokens_ref);

        #
        # {
        #
        shift(@$tokens_ref);

        #
        # <case_list>
        #
        my $case_list_ast = parse_case_list($tokens_ref);

        #
        # }
        #
        shift(@$tokens_ref);

        my $match_case_ast = ASTNode->new("<MATCH>", "AST_MATCH", undef, undef, undef, ());
        $case_list_ast->_parent_node = $match_ast;
        $match_ast->_parent_node = $match_case_ast;
        push(@{$match_case_ast->_child_nodes}, $match_ast);
        push(@{$match_ast->_child_nodes}, $case_list_ast);

        return $match_case_ast;
    }
    else {
        my $exp0_ast = parse_exp0($tokens_ref);
    }
}

sub parse_exp0 {
    my ($tokens_ref) = @_;

    my $prefix_op_ast = parse_prefix_op($tokens_ref);
    my $atomic_exp_ast = parse_atomic_exp($tokens_ref);
}

sub parse_id {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref->[0]};

    #
    # <id>
    #
    shift(@$tokens_ref);

    # The default type is set to variable (AST_VAR)
    # You should set it to other types elsewhere if the context does not match
    # e.g. in parse_atomic_pat, the type is set to function (AST_FUNC) if the context matches "var()"
    my $id_ast = ASTNode->new($token_str, "AST_VAR", undef, undef, undef, ());

    return $id_ast;
}

sub parse_letbind {
    my ($tokens_ref) = @_;

    #
    # <letbind> ::= <pat> = <exp>
    #

    # <pat>
    my $pat_ast = parse_pat($tokens_ref);

    # =
    shift(@$tokens_ref);

    # <exp>
    my $exp_ast = parse_exp($tokens_ref);

    my $assign_ast = ASTNode->new("=", "AST_ASSIGN", undef, undef, undef, ($pat_ast, $exp_ast));
    $pat_ast->_parent_node = $exp_ast->_parent_node = $assign_ast;
    $pat_ast->_right_node = $exp_ast;
    $exp_ast->_left_node = $pat_ast;

    return $assign_ast;
}

sub parse_lit {
    my ($tokens_ref) = @_;
    my ($token_str, $token_type) = @{$tokens_ref->[0]};

    #
    # true
    # false
    #
    if ($token_type eq "L_BOOL") {
        shift(@$tokens_ref);
        return ASTNode->new($token_str, "AST_BOOL", undef, undef, undef, ());
    }

    #
    # ()
    #
    elsif ($token_type eq "L_UNIT") {
        shift(@$tokens_ref);
        return ASTNode->new("()", "AST_UNIT", undef, undef, undef, ());
    }

    #
    # NUMBER
    #
    elsif ($token_type eq "L_NUMBER") {
        shift(@$tokens_ref);
        return ASTNode->new($token_str, "AST_NUMBER", undef, undef, undef, ());
    }

    #
    # undefined
    #
    elsif ($token_type eq "L_UNDEFINED") {
        shift(@$tokens_ref);
        return ASTNode->new("undefined", "AST_UNDEFINED", undef, undef, undef, ());
    }

    #
    # BINARY_LITERAL
    #
    elsif ($token_type eq "L_BIN") {
        shift(@$tokens_ref);
        return ASTNode->new(oct($token_str), "AST_BIN", undef, undef, undef, ());
    }

    #
    # HEXADECIMAL_LITERAL
    #
    elsif ($token_type eq "L_HEX") {
        shift(@$tokens_ref);
        return ASTNode->new(oct($token_str), "AST_HEX", undef, undef, undef, ());
    }

    #
    # STRING_LITERAL
    #
    elsif ($token_type eq "L_STRING") {
        shift(@$tokens_ref);
        $token_str =~ s/^"//;
        $token_str =~ s/"$//;
        return ASTNode->new($token_str, "AST_STRING", undef, undef, undef, ());
    }
}

sub parse_pat {
    my ($tokens_ref) = @_;

    #
    # <pat1>
    #

    my $pat1_ast = parse_pat1($tokens_ref);

    return $pat1_ast;
}

sub parse_pat1 {
    my ($tokens_ref) = @_;

    my $atomic_pat_ast = parse_atomic_pat($tokens_ref);

    return $atomic_pat_ast;
}

sub parse_pat_list {
    my ($tokens_ref) = @_;

    my $pat_ast = parse_pat($tokens_ref);

    if (scalar(@$tokens_ref) == 0) {
        return $pat_ast;
    }

    my ($token_str, $token_type) = @{$tokens_ref->[0]};
    if ($token_type eq "L_COMMA") {
        #
        # ,
        #
        shift(@$tokens_ref);
    }

    if (scalar(@$tokens_ref) == 0) {
        return $pat_ast;
    }

    #
    # <pat_list>
    #
    my $pat_list_ast = parse_pat_list($tokens_ref);

    $pat_ast->_right_node = $pat_list_ast;
    $pat_list_ast->_left_node = $pat_ast;

    return $pat_ast;
}

sub parse_prefix_op {
    my ($tokens_ref) = @_;

    if (scalar(@$tokens_ref) == 0) {
        #
        # | epsilon
        #
        return;
    }

    my ($token_str, $token_type) = @{$tokens_ref->[0]};

    if ($token_type eq "L_POWER") {
        #
        # | 2^
        #
        shift(@$tokens_ref);

        return ASTNode->new("<POWER>", "AST_POWER", undef, undef, undef, ());
    }
    elsif ($token_type eq "L_SUB") {
        #
        # | -
        #
        shift(@$tokens_ref);

        return ASTNode->new("<NEG>", "AST_NEG", undef, undef, undef, ());
    }
    elsif ($token_type eq "L_ADD") {
        #
        # | +
        #
        shift(@$tokens_ref);

        return ASTNode->new("<POS>", "AST_POS", undef, undef, undef, ());
    }
}

1;
