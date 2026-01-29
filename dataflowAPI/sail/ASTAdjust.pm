package ASTAdjust;

use strict;
use warnings;

use Exporter 'import';
use lib "$FindBin::Bin";
use ParserCommon qw(
  XLEN
  WIDTH_BYTES_VAR
);

# Attempt to constant-fold an AST
# Returns:
#   { ok => 1, value => <constant> } if fully foldable
#   { ok => 0 } otherwise
sub try_const_fold {
    my ($curr_node) = @_;
    if ( $curr_node->is_num_node() ) {
        return { ok => 1, value => $curr_node->get_value() };
    }
    elsif ( $curr_node->is_func_node() ) {
        my $func_op   = $curr_node->get_value();
        my $func_args = $curr_node->get_children();

        my %unary_op_lambda = (
            ASTType::ROSE_OP_NEG => sub {
                my ($x) = @_;
                return -$x;
            },
            ASTType::ROSE_OP_BNOT => sub {
                my ($x) = @_;
                return ~$x;
            },
        );
        my %binary_op_lambda = (
            ASTType::ROSE_OP_ADD => sub {
                my ( $x, $y ) = @_;
                return $x + $y;
            },
            ASTType::ROSE_OP_SUB => sub {
                my ( $x, $y ) = @_;
                return $x - $y;
            },
            ASTType::ROSE_OP_SMUL => sub {
                my ( $x, $y ) = @_;
                return $x * $y;
            },
            ASTType::ROSE_OP_SDIV => sub {
                my ( $x, $y ) = @_;
                return $x / $y;
            },
            ASTType::ROSE_OP_SREM => sub {
                my ( $x, $y ) = @_;
                return $x % $y;
            },
            ASTType::ROSE_OP_SHIFT_LEFT => sub {
                my ( $x, $y ) = @_;
                return $x << $y;
            },
            ASTType::ROSE_OP_SHIFT_RIGHT => sub {
                my ( $x, $y ) = @_;
                return ( $x & ( ( 1 << XLEN ) - 1 ) ) >> $y;
            },
            ASTType::ROSE_OP_SHIFT_RIGHT_ARITH => sub {
                my ( $x, $y ) = @_;
                return $x >> $y;
            },
            ASTType::ROSE_OP_ROTATE_LEFT => sub {
                my ( $x, $y ) = @_;
                return ( $x << $y ) | ( $x >> ( XLEN - $y ) );
            },
            ASTType::ROSE_OP_ROTATE_RIGHT => sub {
                my ( $x, $y ) = @_;
                return ( $x >> $y ) | ( $x << ( XLEN - $y ) );
            },
            ASTType::ROSE_OP_BAND => sub {
                my ( $x, $y ) = @_;
                return $x & $y;
            },
            ASTType::ROSE_OP_BOR => sub {
                my ( $x, $y ) = @_;
                return $x | $y;
            },
            ASTType::ROSE_OP_BXOR => sub {
                my ( $x, $y ) = @_;
                return $x ^ $y;
            },
            ASTType::ROSE_OP_EQ => sub {
                my ( $x, $y ) = @_;
                return $x == $y;
            },
            ASTType::ROSE_OP_NEQ => sub {
                my ( $x, $y ) = @_;
                return $x != $y;
            },
            ASTType::ROSE_OP_SIGNED_LT => sub {
                my ( $x, $y ) = @_;
                return $x < $y;
            },
            ASTType::ROSE_OP_SIGNED_GT => sub {
                my ( $x, $y ) = @_;
                return $x > $y;
            },
            ASTType::ROSE_OP_SIGNED_LEQ => sub {
                my ( $x, $y ) = @_;
                return $x <= $y;
            },
            ASTType::ROSE_OP_SIGNED_GEQ => sub {
                my ( $x, $y ) = @_;
                return $x >= $y;
            },
            ASTType::ROSE_OP_UNSIGNED_LT => sub {
                my ( $x, $y ) = @_;
                my $mask = ( 1 << XLEN ) - 1;
                return ( ( $a & $mask ) < ( $b & $mask ) ) ? 1 : 0;
            },
            ASTType::ROSE_OP_UNSIGNED_GT => sub {
                my ( $x, $y ) = @_;
                my $mask = ( 1 << XLEN ) - 1;
                return ( ( $a & $mask ) > ( $b & $mask ) ) ? 1 : 0;
            },
            ASTType::ROSE_OP_UNSIGNED_LEQ => sub {
                my ( $x, $y ) = @_;
                my $mask = ( 1 << XLEN ) - 1;
                return ( ( $a & $mask ) <= ( $b & $mask ) ) ? 1 : 0;
            },
            ASTType::ROSE_OP_UNSIGNED_GEQ => sub {
                my ( $x, $y ) = @_;
                my $mask = ( 1 << XLEN ) - 1;
                return ( ( $a & $mask ) >= ( $b & $mask ) ) ? 1 : 0;
            },
        );

        # Foldable if operand is constant for unary ops
        if ( $curr_node->has_attribute("unary_op") ) {
            my $exp_result = try_const_fold( $func_args->[0] );
            return { ok => 0 } unless $exp_result->{ok};
            return {
                ok    => 1,
                value => $unary_op_lambda{ $curr_node->get_value() }
                  ( $exp_result->{value} )
            };
        }
        # Foldable if both operands are constant for binary ops
        elsif ( $curr_node->has_attribute("binary_op") ) {
            my $lhs_result = try_const_fold( $func_args->[0] );
            my $rhs_result = try_const_fold( $func_args->[1] );
            return { ok => 0 } unless $lhs_result->{ok} && $rhs_result->{ok};
            return {
                ok    => 1,
                value => $binary_op_lambda{ $curr_node->get_value() }
                  ( $lhs_result->{value}, $rhs_result->{value} )
            };

        }
    }
    return { ok => 0 };
}

# Rewrite assignments involving match(...) into EXP_MATCH form.
# This normalizes:
#   X(rd) = match(...)
#   let foo = match(...)
# into a single match expression whose arms contain assignments.
sub adjust_match {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_match( $child, $curr_set );
    }
    if (   $node->is_func_node()
        && $node->is_op(ASTType::ROSE_OP_WRITE_REG)
        && $node->get_nth_child(1)->is_match_node() )
    {
        # X(rd) = match(...)
        my $match_node       = $node->get_nth_child(1);
        my $key_node         = $match_node->get_value();
        my $match_elem_nodes = $match_node->get_children();
        for my $elem_node ( @{$match_elem_nodes} ) {
            my $value_node = $elem_node->get_nth_child(0);
            my $write_reg_node =
              ASTNode->new_func_node( ASTType::ROSE_OP_WRITE_REG,
                [ $node->get_nth_child(0), $value_node ] );
            $elem_node->set_nth_child( 0, $write_reg_node );
        }
        $node->set_type(ASTType::EXP_MATCH);
        $node->set_value($key_node);
        $node->set_children($match_elem_nodes);
    }
    if ( $node->is_assign_node() && $node->get_nth_child(0)->is_match_node() ) {

        # let foo = match(...)
        my $match_node       = $node->get_nth_child(0);
        my $key_node         = $match_node->get_value();
        my $match_elem_nodes = $match_node->get_children();
        for my $elem_node ( @{$match_elem_nodes} ) {
            my $value_node = $elem_node->get_nth_child(0);
            my $var_node   = $node->get_value();
            my $assign_node =
              ASTNode->new_assign_node( $var_node, $value_node );
            $elem_node->set_nth_child( 0, $assign_node );
        }
        $node->set_type(ASTType::EXP_MATCH);
        $node->set_value($key_node);
        $node->set_children($match_elem_nodes);
    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_match( $sibling, $curr_set );
    }
}

# Normalize function operands by inserting implicit arguments

# Also rewrite constant operands as LIT_ID type (literals) where required.
# e.g. d->SignExtend(data, ops->number_(64, 64)) in not valid
# It should be d->SignExtend(data, 64)
sub adjust_func_operands {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_func_operands( $child, $curr_set );
    }
    if ( $node->is_func_node() ) {
        my $func_op   = $node->get_value();
        my $func_args = $node->get_children();

        if ( $node->is_op(ASTType::ROSE_OP_READ_MEM) ) {

            # Add Implicit width
            my $width_bytes_node = ASTNode->new_var_node(WIDTH_BYTES_VAR);
            push @$func_args, $width_bytes_node;
        }
        elsif ($node->is_op(ASTType::ROSE_OP_READ_REG)
            || $node->is_op(ASTType::ROSE_OP_READ_IMM) )
        {
            # Add Implicit width
            my $xlen_node = ASTNode->new_id_node(XLEN);
            my $zero_node = ASTNode->new_id_node(0);
            push @$func_args, $xlen_node;
            push @$func_args, $zero_node;
        }
        elsif ( $func_op eq ASTType::ROSE_OP_SIGN_EXTEND ) {

            # The second operand should be a plain integer
            # It should not be wrapped in ops->number_())
            my $child1 = $node->get_nth_child(1);
            if ( $child1->is_num_node() ) {
                $child1->set_type(ASTType::LIT_ID);
            }
        }
        elsif ( $func_op eq ASTType::ROSE_OP_ZERO_EXTEND ) {

            # The second operand should be a plain integer
            # It should not be wrapped in ops->number_())
            my $child1 = $node->get_nth_child(1);
            if ( $child1->is_num_node() ) {
                $child1->set_type(ASTType::LIT_ID);
            }
        }
        elsif ( $func_op eq ASTType::ROSE_OP_EXTRACT ) {
            my ( undef, $lhs, $rhs ) = @$func_args;
            my $const_fold_lhs = try_const_fold($lhs);
            my $const_fold_rhs = try_const_fold($rhs);
            if ( $const_fold_lhs->{ok} && $const_fold_rhs->{ok} ) {
                my $lhs_node = $node->get_nth_child(1);
                my $rhs_node = $node->get_nth_child(2);
                $lhs_node->set_value( $const_fold_lhs->{value} );
                $lhs_node->set_type(ASTType::LIT_ID);
                $rhs_node->set_value( $const_fold_rhs->{value} );
                $rhs_node->set_type(ASTType::LIT_ID);
            }
        }
        elsif ( $func_op eq ASTType::ROSE_OP_NUMBER ) {
            # The first operand should be a plain integer
            # It should not be wrapped in ops->number_())
            my $child0 = $node->get_nth_child(0);
            if ( $child0->is_num_node() ) {
                $child0->set_type(ASTType::LIT_ID);
            }
        }

    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_func_operands( $sibling, $curr_set );
    }
}

# Replace constant-foldable expressions with numbers.
sub adjust_const_fold {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_const_fold( $child, $curr_set );
    }
    if ( $node->is_func_node() && $node->has_attribute("is_const_foldable") ) {
        my $const_fold_result = try_const_fold($node);
        if ( $const_fold_result->{ok} ) {
            $node->set_type(ASTType::LIT_NUM);
            $node->set_value( $const_fold_result->{value} );
        }
    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_const_fold( $sibling, $curr_set );
    }
}

sub adjust_signedness {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_signedness( $child, $curr_set );
    }
    if ( $node->is_assign_node() ) {
        my $id_node    = $node->get_value();
        my $child_node = $node->get_nth_child(0);

        # Signedness determination
        # Signedness can be determined if SAIL calls signed() or unsigned()
        # In the AST, if a function calls signed() or unsigned(),
        # The result AST will have a "signedness" attribute with value set to 0 or 1

        if (   $child_node->is_var_node()
            && $child_node->has_attribute("signedness") )
        {
            $node->get_value()
              ->set_attribute( "signedness",
                $child_node->get_attribute("signedness") );
        }

        # Signedness can also be determined for the following expression:
        #
        #   foo = if (cond) then signed(bar) else unsigned(baz) end
        #
        elsif ($child_node->is_func_node()
            && $child_node->is_op(ASTType::ROSE_OP_ITE)
            && $child_node->get_nth_child(1)->has_attribute("signedness")
            && $child_node->get_nth_child(2)->has_attribute("signedness") )
        {
            $node->get_value()->set_attribute( "ite_signedness", 1 );
        }
    }

    # Finally, handle all sibling nodes
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_signedness( $sibling, $curr_set );
    }
}

sub adjust_bitvec_assign {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_bitvec_assign( $child, $curr_set );
    }
    if ( $node->is_assign_node() ) {
        my $id_node       = $node->get_value();
        my $children_node = $node->get_children();

        if ( $id_node->is_vec_index_node() ) {
            my $bitvec_node       = $id_node->get_value();
            my ($index_node)      = @{ $id_node->get_children() };
            my ($assign_bit_node) = @{$children_node};

            my $new_value_node =
              ASTBuildHelper::build_bitvec_index_assign( $bitvec_node,
                $index_node, $assign_bit_node );

            $node->set_type(ASTType::EXP_ASSIGN);
            $node->set_value($bitvec_node);
            $node->set_children( [$new_value_node] );
        }
        if ( $id_node->is_vec_range_node() ) {
            my $bitvec_node = $id_node->get_value();
            my ( $beg_index_node, $end_index_node ) =
              @{ $id_node->get_children() };
            my ($assign_bits_node) = @{$children_node};

            my $new_value_node = ASTBuildHelper::build_bitvec_range_assign(
                $bitvec_node,    $beg_index_node,
                $end_index_node, $assign_bits_node
            );

            $node->set_type(ASTType::EXP_ASSIGN);
            $node->set_value($bitvec_node);
            $node->set_children( [$new_value_node] );
        }
    }

    # Finally, handle all sibling nodes
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_bitvec_assign( $sibling, $curr_set );
    }
}

sub change_field_to_id {
    my ( $node, $curr_set ) = @_;

    for my $child ( @{ $node->get_children() } ) {
        change_field_to_id( $child, $curr_set );
    }

    if ( $node->is_field_node() ) {
        my $new_value = "$node->{value}{var}_$node->{value}{field}";
        $node->set_type(ASTType::LIT_VAR);
        $node->set_value($new_value);
    }

    for my $sibling ( @{ $node->get_siblings() } ) {
        change_field_to_id( $sibling, $curr_set );
    }
}

sub change_op_var {
    my ( $node, $curr_set ) = @_;

    for my $child ( @{ $node->get_children() } ) {
        change_op_var( $child, $curr_set );
    }

    if ( $node->is_func_node() ) {
        if (   $node->is_op(ASTType::ROSE_OP_EQ)
            || $node->is_op(ASTType::ROSE_OP_NEQ) )
        {
            my $lhs_node  = $node->get_nth_child(0);
            my $rhs_node  = $node->get_nth_child(1);
            my $lhs_value = $lhs_node->get_value();
            my $rhs_value = $rhs_node->get_value();
            if ( $lhs_value eq "op" && $rhs_value =~ /^[\p{IsUpper}_]+$/ ) {
                $rhs_node->set_value( ParserCommon::rename_op_var($rhs_value) );
            }
            elsif ( $rhs_value eq "op" && $lhs_value =~ /^[\p{IsUpper}_]+$/ ) {
                $lhs_node->set_value( ParserCommon::rename_op_var($lhs_value) );
            }
        }
    }
    if ( $node->is_match_node() ) {
        my $key_node = $node->get_value();
        if ( $key_node->get_value() eq "op" ) {
            my $match_node = $node->get_children();
            for my $match_elem_node ( @{$match_node} ) {
                my $op_node = $match_elem_node->get_value();
                $op_node->set_value(
                    ParserCommon::rename_op_var( $op_node->get_value() ) );
            }
        }
    }

    for my $sibling ( @{ $node->get_siblings() } ) {
        change_op_var( $sibling, $curr_set );
    }
}

sub adjust_mul_div_rem_signedness {
    my ( $node, $curr_set, $lookup ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_mul_div_rem_signedness( $child, $curr_set, $lookup );
    }
    if ( $node->is_func_node() ) {
        my $func_op   = $node->get_value();
        my $func_args = $node->get_children();
        if ( $node->is_op(ASTType::ROSE_OP_SMUL) ) {
            my $lhs_node = $node->get_nth_child(0);
            my $rhs_node = $node->get_nth_child(1);

            # In ROSE, multiplication is split into signedMultiply, unsignedMultiply, and signedUnsignedMultiply
            # Determining which version of multiplication to apply is painful
            # Here, we look for signedness attributes to determine which version of multiply to use
            # The default in SAIL is signed multiply
            my $lhs_signedness = 1;
            my $rhs_signedness = 1;

            # First case, signedness is determined through if and else
            # In this case, we write a C++ switch statement to determine which version of multiply to use
            my $lhs                 = $lhs_node->get_value();
            my $rhs                 = $rhs_node->get_value();
            my $lhs_lookup_ref_node = $lookup->{$lhs}{ref};
            my $rhs_lookup_ref_node = $lookup->{$rhs}{ref};
            my $lhs_lookup_var_node = $lookup->{$lhs}{var};
            my $rhs_lookup_var_node = $lookup->{$rhs}{var};
            if ( defined $lhs_lookup_var_node ) {
                if ( $lhs_lookup_var_node->has_attribute("signedness") ) {
                    $lhs_signedness =
                      $lhs_lookup_var_node->get_attribute("signedness");
                }
                elsif ( $lhs_lookup_var_node->has_attribute("ite_signedness") )
                {
                    $lhs_signedness = ParserCommon::ITE_SIGNEDNESS;
                }
            }
            if ( defined $rhs_lookup_var_node ) {
                if ( $rhs_lookup_var_node->has_attribute("signedness") ) {
                    $rhs_signedness =
                      $rhs_lookup_var_node->get_attribute("signedness");
                }
                elsif ( $rhs_lookup_var_node->has_attribute("ite_signedness") )
                {
                    $rhs_signedness = ParserCommon::ITE_SIGNEDNESS;
                }
            }

            my $lhs_cond = ASTNode->new_num_node($lhs_signedness);
            if ( $lhs_signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $lhs_lookup_ref_node->is_func_node() ) {
                    $lhs_cond = $lhs_lookup_ref_node->get_nth_child(0);
                }
                else {
                    $lhs_cond = ASTNode->new_num_node($lhs_signedness);
                }
            }
            my $rhs_cond = ASTNode->new_num_node($rhs_signedness);
            if ( $rhs_signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $rhs_lookup_ref_node->is_func_node() ) {
                    $rhs_cond = $rhs_lookup_ref_node->get_nth_child(0);
                }
                else {
                    $rhs_cond = ASTNode->new_num_node($rhs_signedness);
                }
            }

            my $ss_mul_node = ASTNode->new_func_node( ASTType::ROSE_OP_SMUL,
                [ $lhs_node, $rhs_node ] );
            my $su_mul_node = ASTNode->new_func_node( ASTType::ROSE_OP_SUMUL,
                [ $lhs_node, $rhs_node ] );
            my $us_mul_node = ASTNode->new_func_node( ASTType::ROSE_OP_SUMUL,
                [ $rhs_node, $lhs_node ] );
            my $uu_mul_node = ASTNode->new_func_node( ASTType::ROSE_OP_UMUL,
                [ $lhs_node, $rhs_node ] );

            if ( !$lhs_cond->is_num_node() && !$rhs_cond->is_num_node() ) {
                my $cond_ss = ASTNode->new_func_node(
                    ASTType::ROSE_OP_BAND,
                    [
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$lhs_cond]
                        ),
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$rhs_cond]
                        ),
                    ]
                );
                my $cond_su = ASTNode->new_func_node(
                    ASTType::ROSE_OP_BAND,
                    [
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$lhs_cond]
                        ),
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL,
                            [
                                ASTNode->new_func_node(
                                    ASTType::ROSE_OP_BAND, [$rhs_cond]
                                )
                            ]
                        ),
                    ]
                );
                my $cond_us = ASTNode->new_func_node(
                    ASTType::ROSE_OP_BAND,
                    [
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$lhs_cond]
                        ),
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL,
                            [
                                ASTNode->new_func_node(
                                    ASTType::ROSE_OP_BAND, [$rhs_cond]
                                )
                            ]
                        ),
                    ]
                );
                my $ite_node1 =
                  ASTNode->new_ite_node( $cond_us, $us_mul_node, $uu_mul_node );
                my $ite_node2 =
                  ASTNode->new_ite_node( $cond_su, $su_mul_node, $ite_node1 );
                $node->set_type(ASTType::EXP_FUNC);
                $node->set_value(ASTType::ROSE_OP_ITE);
                $node->set_children( [ $cond_ss, $ss_mul_node, $ite_node2 ] );
            }
            elsif ( !$lhs_cond->is_num_node() ) {
                my $rhs_signedness = $rhs_cond->get_value();
                if ($rhs_signedness) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_ITE);
                    $node->set_children(
                        [
                            ASTNode->new_func_node(
                                ASTType::ROSE_OP_BOOL, [$lhs_cond]
                            ),
                            $ss_mul_node,
                            $us_mul_node
                        ]
                    );
                }
                else {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_ITE);
                    $node->set_children(
                        [
                            ASTNode->new_func_node(
                                ASTType::ROSE_OP_BOOL, [$lhs_cond]
                            ),
                            $su_mul_node,
                            $uu_mul_node
                        ]
                    );
                }
            }
            elsif ( !$rhs_cond->is_num_node() ) {
                my $lhs_signedness = $lhs_cond->get_value();
                if ($lhs_signedness) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_ITE);
                    $node->set_children(
                        [
                            ASTNode->new_func_node(
                                ASTType::ROSE_OP_BOOL, [$rhs_cond]
                            ),
                            $ss_mul_node,
                            $su_mul_node
                        ]
                    );
                }
                else {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_ITE);
                    $node->set_children(
                        [
                            ASTNode->new_func_node(
                                ASTType::ROSE_OP_BOOL, [$rhs_cond]
                            ),
                            $us_mul_node,
                            $uu_mul_node
                        ]
                    );
                }
            }
            else {
                my $lhs_signedness = $lhs_cond->get_value();
                my $rhs_signedness = $rhs_cond->get_value();
                if ( $lhs_signedness && $rhs_signedness ) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_SMUL);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
                elsif ( $lhs_signedness && !$rhs_signedness ) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_SUMUL);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
                elsif ( !$lhs_signedness && $rhs_signedness ) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_SUMUL);
                    $node->set_children( [ $rhs_node, $lhs_node ] );
                }
                else {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_UMUL);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
            }

        }
        elsif ( $node->is_op(ASTType::ROSE_OP_SDIV) ) {
            my $lhs_node = $node->get_nth_child(0);
            my $rhs_node = $node->get_nth_child(1);

            # In ROSE, division is split into signedDivide and unsignedDivide

            # The default in SAIL is signed divide
            my $signedness = 1;

            # Determining which version of division to apply is painful
            # Here, we look for signedness attributes to determine which version of division to use
            my $lhs_value           = $lhs_node->get_value();
            my $lhs_lookup_ref_node = $lookup->{$lhs_value}->{ref};
            my $lhs_lookup_var_node = $lookup->{$lhs_value}->{var};
            if ( $lhs_lookup_var_node->has_attribute("signedness") ) {
                $signedness = $lhs_lookup_var_node->get_attribute("signedness");
            }
            elsif ( $lhs_lookup_var_node->has_attribute("ite_signedness") ) {
                $signedness = ParserCommon::ITE_SIGNEDNESS;
            }

            my $cond      = ASTNode->new_num_node($signedness);
            my $sdiv_node = ASTNode->new_func_node( ASTType::ROSE_OP_SDIV,
                [ $lhs_node, $rhs_node ] );
            my $udiv_node = ASTNode->new_func_node( ASTType::ROSE_OP_UDIV,
                [ $rhs_node, $lhs_node ] );
            if ( $signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $lhs_lookup_ref_node->is_func_node() ) {
                    $cond = $lhs_lookup_ref_node->get_nth_child(0);
                }
                else {
                    $cond = ASTNode->new_num_node($signedness);
                }
            }
            if ( !$cond->is_num_node() ) {
                $node->set_type(ASTType::EXP_FUNC);
                $node->set_value(ASTType::ROSE_OP_ITE);
                $node->set_children(
                    [
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$cond]
                        ),
                        $sdiv_node,
                        $udiv_node
                    ]
                );
            }
            else {
                if ($signedness) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_SDIV);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
                else {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_UDIV);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
            }
        }
        elsif ( $node->{value} eq ASTType::ROSE_OP_SREM ) {
            my $lhs_node = $node->get_nth_child(0);
            my $rhs_node = $node->get_nth_child(1);

            # In ROSE, division is split into signedDivide and unsignedDivide

            # The default in SAIL is signed divide
            my $signedness = 1;

            # Determining which version of division to apply is painful
            # Here, we look for signedness attributes to determine which version of division to use
            my $lhs_value           = $lhs_node->get_value();
            my $lhs_lookup_ref_node = $lookup->{$lhs_value}->{ref};
            my $lhs_lookup_var_node = $lookup->{$lhs_value}->{var};
            if ( $lhs_lookup_var_node->has_attribute("signedness") ) {
                $signedness = $lhs_lookup_var_node->get_attribute("signedness");
            }
            elsif ( $lhs_lookup_var_node->has_attribute("ite_signedness") ) {
                $signedness = ParserCommon::ITE_SIGNEDNESS;
            }

            my $cond      = ASTNode->new_num_node($signedness);
            my $sdiv_node = ASTNode->new_func_node( ASTType::ROSE_OP_SREM,
                [ $lhs_node, $rhs_node ] );
            my $udiv_node = ASTNode->new_func_node( ASTType::ROSE_OP_UREM,
                [ $rhs_node, $lhs_node ] );
            if ( $signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $lhs_lookup_ref_node->is_func_node() ) {
                    $cond = $lhs_lookup_ref_node->get_nth_child(0);
                }
                else {
                    $cond = ASTNode->new_num_node($signedness);
                }
            }
            if ( !$cond->is_num_node() ) {
                $node->set_type(ASTType::EXP_FUNC);
                $node->set_value(ASTType::ROSE_OP_ITE);
                $node->set_children(
                    [
                        ASTNode->new_func_node(
                            ASTType::ROSE_OP_BOOL, [$cond]
                        ),
                        $sdiv_node,
                        $udiv_node
                    ]
                );

            }
            else {
                if ($signedness) {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_SREM);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
                else {
                    $node->set_type(ASTType::EXP_FUNC);
                    $node->set_value(ASTType::ROSE_OP_UREM);
                    $node->set_children( [ $lhs_node, $rhs_node ] );
                }
            }
        }
    }
    elsif ( $node->is_assign_node() ) {
        my $var_node = $node->get_value();
        my ($value_node) = @{ $node->get_children() };

        my ( @lhs_nodes, @rhs_nodes );
        if ( $var_node->is_tuple_node() && $value_node->is_tuple_node() ) {
            @lhs_nodes = @{ $var_node->get_children() };
            @rhs_nodes = @{ $value_node->get_children() };
        }
        else {
            @lhs_nodes = ($var_node);
            @rhs_nodes = ($value_node);
        }
        for my $i ( 0 .. $#lhs_nodes ) {
            my $var_node   = $lhs_nodes[$i];
            my $value_node = $rhs_nodes[$i];
            my $var = ParserCommon::rename_invalid_id( $var_node->get_value() );
            $lookup->{$var} =
              { is_rose_var => 1, var => $var_node, ref => $value_node };
        }
    }
    elsif ( $node->is_for_node() ) {
        my $i = $node->get_value()->get_value();
        $lookup->{$i} = { is_cpp_var => 1 };
    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_mul_div_rem_signedness( $sibling, $curr_set, $lookup );
    }
}

1;
