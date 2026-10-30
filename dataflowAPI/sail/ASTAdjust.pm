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

# An attribute of a SAIL argument in ParserConfig, without creating entries
sub arg_attribute {
    my ( $curr_set, $name, $attribute ) = @_;
    my $args = $ParserConfig::ARGS_PER_SUBSET{$curr_set};
    return undef unless $args && exists $args->{$name};
    return $args->{$name}{$attribute};
}

# The value of a condition that is known when the code is generated, or
# undef: a comparison of op with an op Capstone does not decode, a bound of
# an argument whose largest value is known, and boolean operators over them.
sub known_bool {
    my ( $node, $curr_set ) = @_;
    return $node->get_value() ? 1 : 0 if $node->is_bool_node();
    return undef unless $node->is_func_node();
    my @kids = @{ $node->get_children() };
    my @known = map { known_bool( $_, $curr_set ) } @kids;

    if ( $node->is_op(ASTType::ROSE_OP_BOOL) ) {
        return $known[0];
    }
    if ( $node->has_attribute("boolean") ) {
        if ( $node->is_op(ASTType::ROSE_OP_BNOT) ) {
            return defined $known[0] ? !$known[0] + 0 : undef;
        }
        my $absorbing = $node->is_op(ASTType::ROSE_OP_BAND) ? 0 : 1;
        return $absorbing if grep { defined $_ && $_ == $absorbing } @known;
        return 1 - $absorbing unless grep { !defined $_ } @known;
        return undef;
    }
    if ( $node->is_op(ASTType::ROSE_OP_EQ) || $node->is_op(ASTType::ROSE_OP_NEQ) ) {
        my ( $var, $other ) =
          $kids[0]->is_var_node() && $kids[0]->get_value() eq "op"
          ? @kids
          : reverse @kids;
        return undef
          unless $var->is_var_node()
          && $var->get_value() eq "op"
          && grep { $_ eq $other->get_value() }
          @{ arg_attribute( $curr_set, "op", "unsupported" ) // [] };
        return $node->is_op(ASTType::ROSE_OP_EQ) ? 0 : 1;
    }
    if (   $node->is_op(ASTType::ROSE_OP_SIGNED_LEQ)
        || $node->is_op(ASTType::ROSE_OP_SIGNED_LT) )
    {
        my ( $var, $bound ) = @kids;
        my $max = $var->is_var_node()
          && arg_attribute( $curr_set, $var->get_value(), "max_value" );
        return undef unless $max && $bound->is_num_node();
        my $holds =
          $node->is_op(ASTType::ROSE_OP_SIGNED_LEQ)
          ? $max <= $bound->get_value()
          : $max < $bound->get_value();
        return $holds ? 1 : undef;
    }
    return undef;
}

# Leave out what cannot happen: the match arms of unsupported ops, and the
# arm of an if or ite whose condition is known.
sub adjust_known_conditions {
    my ( $node, $curr_set ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        adjust_known_conditions( $child, $curr_set );
    }
    if ( $node->is_match_node() && $node->get_value()->get_value() eq "op" ) {
        my %unsupported = map { $_ => 1 }
          @{ arg_attribute( $curr_set, "op", "unsupported" ) // [] };
        $node->set_children(
            [
                grep { !$unsupported{ $_->get_value()->get_value() } }
                  @{ $node->get_children() }
            ]
        );
    }
    if ( $node->is_ite_node() ) {
        my $known = known_bool( $node->get_nth_child(0), $curr_set );
        if ( defined $known ) {
            my $arm      = $node->get_nth_child( $known ? 1 : 2 );
            my $siblings = $node->get_siblings();
            %$node = %$arm;
            $node->{siblings} = [ @{ $arm->get_siblings() }, @$siblings ];
        }
    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        adjust_known_conditions( $sibling, $curr_set );
    }
}

# A node is a C++ expression if it is built from numbers, the need_number
# arguments (plain C++ variables) and C++ operators over them.
sub is_cpp_expr {
    my ( $node, $curr_set ) = @_;
    return 1 if $node->is_num_node() || $node->is_id_node();
    if ( $node->is_var_node() ) {
        return arg_attribute( $curr_set, $node->get_value(), "need_number" );
    }
    return 0
      unless $node->is_func_node()
      && ( $node->is_op(ASTType::ROSE_OP_BOOL) || PrintROSE::cpp_op($node) );
    for my $child ( @{ $node->get_children() } ) {
        return 0 unless is_cpp_expr( $child, $curr_set );
    }
    return 1;
}

# Turn an operand that ROSE takes as a plain integer into a C++ expression,
# e.g. ops->extract(x, 0, width * 8) instead of ops->number_(64, width) ...
sub make_cpp_operand {
    my ( $node, $curr_set ) = @_;
    is_cpp_expr( $node, $curr_set )
      or die "Error: $curr_set: operand is not a C++ integer expression";
    my $expr = PrintROSE::build_expr( $node, 0, 0 );
    $node->set_type(ASTType::LIT_ID);
    $node->set_value($expr);
    $node->set_children( [] );
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

            # Add Implicit width, a plain integer
            my $width_bytes_node = ASTNode->new_id_node(WIDTH_BYTES_VAR);
            push @$func_args, $width_bytes_node;
        }
        elsif ( $node->is_op(ASTType::ROSE_OP_READ_REG) ) {
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

            # The bounds are plain integers
            make_cpp_operand( $func_args->[1], $curr_set );
            make_cpp_operand( $func_args->[2], $curr_set );
        }
        elsif ( $node->is_op(ASTType::ROSE_OP_WRITE_MEM) ) {

            # The width is a plain integer
            make_cpp_operand( $func_args->[1], $curr_set );
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
            my ( $high_index_node, $low_index_node ) =
              @{ $id_node->get_children() };
            my ($assign_bits_node) = @{$children_node};

            my $new_value_node = ASTBuildHelper::build_bitvec_range_assign(
                $bitvec_node,    $high_index_node,
                $low_index_node, $assign_bits_node
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

# For a variable set by
#   let x = if cond then signed(...) else unsigned(...)
# (or the arms the other way round), the condition under which x is signed.
sub signed_cond {
    my ($ite_node) = @_;
    my $cond_node = $ite_node->get_nth_child(0);
    return $ite_node->get_nth_child(1)->get_attribute("signedness")
      ? $cond_node
      : ASTBuildHelper::build_boolean_not($cond_node);
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
                    $lhs_cond = signed_cond($lhs_lookup_ref_node);
                }
                else {
                    $lhs_cond = ASTNode->new_num_node($lhs_signedness);
                }
            }
            my $rhs_cond = ASTNode->new_num_node($rhs_signedness);
            if ( $rhs_signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $rhs_lookup_ref_node->is_func_node() ) {
                    $rhs_cond = signed_cond($rhs_lookup_ref_node);
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
                my $cond_ss =
                  ASTBuildHelper::build_boolean_and( $lhs_cond, $rhs_cond );
                my $cond_su = ASTBuildHelper::build_boolean_and( $lhs_cond,
                    ASTBuildHelper::build_boolean_not($rhs_cond) );
                my $cond_us = ASTBuildHelper::build_boolean_and(
                    ASTBuildHelper::build_boolean_not($lhs_cond), $rhs_cond );
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
                [ $lhs_node, $rhs_node ] );
            if ( $signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $lhs_lookup_ref_node->is_func_node() ) {
                    $cond = signed_cond($lhs_lookup_ref_node);
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
                [ $lhs_node, $rhs_node ] );
            if ( $signedness == ParserCommon::ITE_SIGNEDNESS ) {
                if ( $lhs_lookup_ref_node->is_func_node() ) {
                    $cond = signed_cond($lhs_lookup_ref_node);
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
