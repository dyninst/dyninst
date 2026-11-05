package PrintROSE;

use strict;
use warnings;

use Exporter 'import';
use lib "$FindBin::Bin";
use ASTBuild;
use ASTNode;
use ASTAdjust;
use ParserCommon qw(INDENT XLEN);

our @EXPORT_OK = qw(
  print_indent
  print_load_init_code
  print_store_init_code
  print_mul_init_code
  print_div_init_code
  print_rem_init_code
);

sub print_rose_code {
    my ( $self, $body, $args, $curr_set ) = @_;

    # Parse function body
    # Generate ROSE C++ code

    my %lookup    = ();
    my $body_node = ASTBuild->parse_ast( $body, $curr_set );
    ASTAdjust::change_field_to_id( $body_node, $curr_set, {} );
    ASTAdjust::change_op_var( $body_node, $curr_set, {} );
    ASTAdjust::adjust_bitvec_assign( $body_node, $curr_set );
    ASTAdjust::adjust_match( $body_node, $curr_set );
    ASTAdjust::adjust_const_fold( $body_node, $curr_set );
    ASTAdjust::adjust_signedness( $body_node, $curr_set );
    ASTAdjust::adjust_func_operands( $body_node, $curr_set );
    ASTAdjust::adjust_mul_div_rem_signedness( $body_node, $curr_set, {} );

    print_indent( "struct IP_$curr_set : P {\n",                     0 );
    print_indent( "void p(D d, Ops ops, I insn, A args, B raw) {\n", 1 );

    if ( my $vars = $ParserConfig::ARGS_PER_SUBSET{$curr_set} ) {
        for my $var ( keys %$vars ) {
            $vars->{$var}{init_code}();
        }
    }
    else {
        die "Error: $curr_set does not contain ROSE args to SAIL args mapping. Consider adding the mapping to ParserConfig.pm.";
    }
    print("\n");

    print_all_declare( $body_node, $curr_set, {} );
    print_indent( "", 2 );
    print_ast( $body_node, $curr_set, \%lookup, 2 );
    print(";\n");
    print_indent( "return;\n", 2 );
    print_indent( "}\n",       1 );
    print_indent( "};\n\n",    0 );
}

sub print_func {
    my ( $func_name, $func_args, $curr_set, $lookup, $indent_lvl ) = @_;
    print($func_name);
    print("(");
    for my $i ( 0 .. $#$func_args ) {
        print_ast( $func_args->[$i], $curr_set, $lookup, $indent_lvl );
        print ", " if $i != $#$func_args;
    }
    print(")");
}

sub print_indent {
    my ( $code, $indent_lvl ) = @_;
    print( " " x ( INDENT * $indent_lvl ) . $code );
}

sub print_declare {
    my ( $var_name, $indent_lvl ) = @_;
    $var_name = ParserCommon::rename_invalid_id($var_name);
    print_indent( "BaseSemantics::SValuePtr $var_name;\n", $indent_lvl );
}

sub print_declare_if_undeclared {
    my ( $var_name, $lookup, $indent_lvl ) = @_;
    if ( !exists $lookup->{$var_name} ) {
        print_declare( $var_name, $indent_lvl );
    }
}

sub print_assign {
    my ( $lhs_node, $rhs_node, $curr_set, $lookup, $indent_lvl ) = @_;
    my ( @lhs_nodes, @rhs_nodes );
    if ( $lhs_node->is_tuple_node() && $rhs_node->is_tuple_node() ) {
        @lhs_nodes = @{ $lhs_node->get_children() };
        @rhs_nodes = @{ $rhs_node->get_children() };
    }
    else {
        @lhs_nodes = ($lhs_node);
        @rhs_nodes = ($rhs_node);
    }
    for my $i ( 0 .. $#lhs_nodes ) {
        my $var_node   = $lhs_nodes[$i];
        my $value_node = $rhs_nodes[$i];
        my $var = ParserCommon::rename_invalid_id( $var_node->get_value() );
        print("$var = ");
        print_ast( $value_node, $curr_set, $lookup, $indent_lvl );
        $lookup->{$var} = { "var" => $var_node, "ref" => $value_node };
        if ($i != $#lhs_nodes) {
            print(";\n");
            print_indent("", $indent_lvl);
        }
    }
}

sub print_switch {
    my ( $match_node, $match_exprs, $curr_set, $lookup, $indent_lvl ) = @_;
    my $match_var = $match_node->get_value();
    print("switch (");
    print_ast( $match_node, $curr_set, $lookup, $indent_lvl );
    print(") {\n");
    for my $i ( 0 .. $#$match_exprs ) {
        my $child_node = $match_exprs->[$i];
        if ( $child_node->is_match_op_node() ) {
            my $key_node   = $child_node->get_value();
            my $value_node = $child_node->get_nth_child(0);
            if (   $match_node->is_var_node()
                && $match_node->get_value() eq "op" )
            {
                # change op to riscv64_rose_..._op
                my $key = $key_node->get_value();
                if ( $curr_set eq "AMO" ) {

                   # In Capstone, AQ and RL are treated as separate instructions
                   # Thus, we need to add all these cases back
                    print_indent( "case ${key}_w: \n",       $indent_lvl + 1 );
                    print_indent( "case ${key}_w_aq: \n",    $indent_lvl + 1 );
                    print_indent( "case ${key}_w_aq_rl: \n", $indent_lvl + 1 );
                    print_indent( "case ${key}_w_rl: \n",    $indent_lvl + 1 );
                    print_indent( "case ${key}_d: \n",       $indent_lvl + 1 );
                    print_indent( "case ${key}_d_aq: \n",    $indent_lvl + 1 );
                    print_indent( "case ${key}_d_aq_rl: \n", $indent_lvl + 1 );
                    print_indent( "case ${key}_d_rl: \n",    $indent_lvl + 1 );
                }
                else {
                    print_indent( "case $key: \n", $indent_lvl + 1 );
                }
            }
            else {
                print_indent( "case ", $indent_lvl + 1 );
                print_ast( $key_node, $curr_set, $lookup, $indent_lvl );
                print(": {\n");
            }
            if ( $value_node->is_block_node() ) {
                print_ast( $value_node, $curr_set, $lookup, $indent_lvl + 2 );
                print("\n");
                print_indent( "break;\n", $indent_lvl + 2 );
                print_indent( "}\n",      $indent_lvl + 1 );
            }
            else {
                print_indent( "", $indent_lvl + 2 );
                print_ast( $value_node, $curr_set, $lookup, $indent_lvl + 2 );
                print(";\n");
                print_indent( "break;\n", $indent_lvl + 2 );
                print_indent( "}\n",      $indent_lvl + 1 );
            }
        }
    }
    print_indent( "default:\n",   $indent_lvl + 1 );
    print_indent( "assert(0);\n", $indent_lvl + 2 );
    print_indent( "break;\n",     $indent_lvl + 2 );
    print_indent( "}",            $indent_lvl );
}

sub build_expr {
    my ( $curr_node, $parent_prec, $is_right ) = @_;
    my %CPP_OP_PREC = (
        ASTType::ROSE_OP_BOR         => 3,
        ASTType::ROSE_OP_BXOR        => 4,
        ASTType::ROSE_OP_BAND        => 5,
        ASTType::ROSE_OP_SHIFT_LEFT  => 8,
        ASTType::ROSE_OP_SHIFT_RIGHT => 8,
        ASTType::ROSE_OP_ADD         => 9,
        ASTType::ROSE_OP_SUB         => 9,
        ASTType::ROSE_OP_SMUL        => 10,
        ASTType::ROSE_OP_SDIV        => 10,
        ASTType::ROSE_OP_SREM        => 10,
        ASTType::ROSE_OP_BNOT        => 11,
    );
    my %CPP_OP_STR = (
        ASTType::ROSE_OP_BOR         => "|",
        ASTType::ROSE_OP_BXOR        => "^",
        ASTType::ROSE_OP_BAND        => "&",
        ASTType::ROSE_OP_SHIFT_LEFT  => "<<",
        ASTType::ROSE_OP_SHIFT_RIGHT => ">>",
        ASTType::ROSE_OP_ADD         => "+",
        ASTType::ROSE_OP_SUB         => "-",
        ASTType::ROSE_OP_SMUL        => "*",
        ASTType::ROSE_OP_SDIV        => "/",
        ASTType::ROSE_OP_SREM        => "%",
        ASTType::ROSE_OP_BNOT        => "~",
    );
    my %NON_ASSOC = map { $_ => 1 } qw(- / % << >>);
    if ( !$curr_node->is_func_node() ) {
        return $curr_node->get_value();
    }
    else {
        my $const_fold_result = ASTAdjust::try_const_fold($curr_node);
        if ( $const_fold_result->{ok} ) {
            return $const_fold_result->{value};
        }
        my $func_op   = $curr_node->get_value();
        my $func_args = $curr_node->get_children();
        my $prec      = $CPP_OP_PREC{$func_op};

        # Unary operator
        if ( scalar @$func_args == 1 ) {
            my $child_str  = build_expr( $func_args->[0], $prec, 0 );
            my $need_paren = $prec < $parent_prec;
            return $CPP_OP_STR{$func_op}
              . ( $need_paren ? "($child_str)" : $child_str );
        }

        # Binary operator
        elsif ( scalar @$func_args == 2 ) {
            my $lhs_str    = build_expr( $func_args->[0], $prec, 0 );
            my $rhs_str    = build_expr( $func_args->[1], $prec, 1 );
            my $need_paren = ( $prec < $parent_prec )
              || ( $is_right && $NON_ASSOC{$func_op} && $prec == $parent_prec );

            my $child_str = "$lhs_str $CPP_OP_STR{$func_op} $rhs_str";
            return $need_paren ? "($child_str)" : $child_str;
        }
    }
}

sub print_raw {
    my ( $curr_node, $parent_prec, $is_right ) = @_;
    print( build_expr( $curr_node, 0, 0 ) );
}

sub print_all_declare {
    my ( $node, $curr_set, $lookup ) = @_;
    for my $child ( @{ $node->get_children() } ) {
        print_all_declare( $child, $curr_set, $lookup );
    }
    if ( $node->is_assign_node() ) {
        my $var_node   = $node->get_value();
        my $value_node = $node->get_nth_child(0);
        my $var_name   = $var_node->get_value();
        print_declare_if_undeclared( $var_name, $lookup, 2 );
        $lookup->{$var_name} = { "var" => $var_node, "ref" => $value_node };
    }
    for my $sibling ( @{ $node->get_siblings() } ) {
        print_all_declare( $sibling, $curr_set, $lookup );
    }
}

sub print_ast {
    my ( $node, $curr_set, $lookup, $indent_lvl ) = @_;
    if ( $node->is_null_node() ) {
        return;
    }
    elsif ( $node->is_id_node() ) {
        print( $node->get_value() );
    }
    elsif ( $node->is_num_node() ) {
        my $value = $node->get_value();
        print("ops->number_(@{[XLEN]}, $value)");
    }
    elsif ( $node->is_bool_node() ) {
        my $value = $node->get_value();
        print("ops->boolean_($value)");
    }
    elsif ( $node->is_var_node() ) {
        my $value = $node->get_value();

        # For plain C++ variables, we need to wrap it in ops->number_() to change it into SValuePtr
        if (
            (
                exists $ParserConfig::ARGS_PER_SUBSET{$curr_set}->{$value}
                && $ParserConfig::ARGS_PER_SUBSET{$curr_set}
                ->{$value}{need_number}
            )
            || (   exists $lookup->{$value}
                && exists $lookup->{$value}{is_cpp_var}
                && $lookup->{$value}{is_cpp_var} )
          )
        {
            print("ops->number_(@{[XLEN]}, $value)");
        }
        else {
            print($value);
        }
    }
    elsif ( $node->is_func_node() ) {
        my $func_op       = $node->get_value();
        my $func_args     = $node->get_children();
        my %func_dispatch = (
            ASTType::ROSE_OP_ADD          => 'ops->add',
            ASTType::ROSE_OP_BAND         => 'ops->and_',
            ASTType::ROSE_OP_BNOT         => 'ops->invert',
            ASTType::ROSE_OP_BOR          => 'ops->or_',
            ASTType::ROSE_OP_BXOR         => 'ops->xor_',
            ASTType::ROSE_OP_BOOL         => 'ops->boolean_',
            ASTType::ROSE_OP_BREV8        => 'd->Brev8',
            ASTType::ROSE_OP_CLZ          => 'd->CountLeadingZeroBits',
            ASTType::ROSE_OP_CMUL         => 'd->CarrylessMultiply',
            ASTType::ROSE_OP_CMULR        => 'd->CarrylessMultiplyReverse',
            ASTType::ROSE_OP_CONCAT       => 'ops->concat',
            ASTType::ROSE_OP_CTZ          => 'd->CountTrailingZeroBits',
            ASTType::ROSE_OP_EADDR        => 'd->effectiveAddress',
            ASTType::ROSE_OP_EXTRACT      => 'ops->extract',
            ASTType::ROSE_OP_ITE          => 'ops->ite',
            ASTType::ROSE_OP_EQ           => 'ops->isEqual',
            ASTType::ROSE_OP_NEQ          => 'ops->isNotEqual',
            ASTType::ROSE_OP_SIGNED_GEQ   => 'ops->isSignedGreaterThanOrEqual',
            ASTType::ROSE_OP_SIGNED_GT    => 'ops->isSignedGreaterThan',
            ASTType::ROSE_OP_SIGNED_LEQ   => 'ops->isSignedLessThanOrEqual',
            ASTType::ROSE_OP_SIGNED_LT    => 'ops->isSignedLessThan',
            ASTType::ROSE_OP_UNSIGNED_GEQ => 'ops->isUnsignedGreaterThanOrEqual',
            ASTType::ROSE_OP_UNSIGNED_GT  => 'ops->isUnsignedGreaterThan',
            ASTType::ROSE_OP_UNSIGNED_LEQ => 'ops->isUnsignedLessThanOrEqual',
            ASTType::ROSE_OP_UNSIGNED_LT  => 'ops->isUnsignedLessThan',
            ASTType::ROSE_OP_IS_ZERO      => 'd->CountLeadingZeroBits',
            ASTType::ROSE_OP_READ_MEM     => 'd->readMemory',
            ASTType::ROSE_OP_WRITE_MEM    => 'd->writeMemory',
            ASTType::ROSE_OP_NEG          => 'ops->negate',
            ASTType::ROSE_OP_NUMBER       => 'ops->number_',
            ASTType::ROSE_OP_POPC         => 'd->PopCount',
            ASTType::ROSE_OP_READ_IMM     => 'd->read',
            ASTType::ROSE_OP_READ_REG     => 'd->read',
            ASTType::ROSE_OP_REV8         => 'd->Rev8',
            ASTType::ROSE_OP_ROTATE_LEFT  => 'ops->rotateLeft',
            ASTType::ROSE_OP_ROTATE_RIGHT => 'ops->rotateRight',
            ASTType::ROSE_OP_SDIV         => 'ops->signedDivide',
            ASTType::ROSE_OP_SHIFT_LEFT   => 'ops->shiftLeft',
            ASTType::ROSE_OP_SHIFT_RIGHT_ARITH => 'ops->shiftRightArithmetic',
            ASTType::ROSE_OP_SHIFT_RIGHT       => 'ops->shiftRight',
            ASTType::ROSE_OP_SIGN_EXTEND       => 'd->SignExtend',
            ASTType::ROSE_OP_SMUL              => 'ops->signedMultiply',
            ASTType::ROSE_OP_SREM              => 'ops->signedModulo',
            ASTType::ROSE_OP_SUB               => 'ops->subtract',
            ASTType::ROSE_OP_SUMUL             => 'ops->signedUnsignedMultiply',
            ASTType::ROSE_OP_UDIV              => 'ops->unsignedDivide',
            ASTType::ROSE_OP_UMUL              => 'ops->unsignedMultiply',
            ASTType::ROSE_OP_UREM              => 'ops->unsignedModulo',
            ASTType::ROSE_OP_WRITE_REG         => 'd->write',
            ASTType::ROSE_OP_ZERO_EXTEND       => 'd->ZeroExtend',
        );

        if ( my $func_name = $func_dispatch{$func_op} ) {
            print_func( $func_name, $func_args, $curr_set, $lookup,
                $indent_lvl );
        }

        # Handle special functions
        # Print the function all at once
        elsif ( $node->is_op(ASTType::ROSE_OP_GET_PC) ) {
            print("PC");
        }
        elsif ( $node->is_op(ASTType::ROSE_OP_GET_NEXT_PC) ) {
            print("ops->add(PC, ops->number_(@{[XLEN]}, insn->get_size()))");
        }
        elsif ( $node->is_op(ASTType::ROSE_OP_JUMP) ) {
            print_func( "d->BranchTo", $func_args, $curr_set, $lookup,
                $indent_lvl );
        }
    }
    elsif ( $node->is_block_node() ) {
        my $children_node = $node->get_children();
        for my $i ( 0 .. $#$children_node ) {
            my $child_node = $children_node->[$i];
            print_ast( $child_node, $curr_set, $lookup, $indent_lvl );
        }
    }
    elsif ( $node->is_match_node() ) {
        my $key         = $node->get_value();
        my $match_cases = $node->get_children();
        print_switch( $key, $match_cases, $curr_set, $lookup, $indent_lvl );
    }
    elsif ( $node->is_assign_node() ) {
        my $id_node       = $node->get_value();
        my $children_node = $node->get_children();

        print_assign( $id_node, $children_node->[0], $curr_set, $lookup,
            $indent_lvl );
    }
    elsif ( $node->is_hex_node() ) {
        my $hex_str   = $node->get_value();
        my $hex_value = "0x$hex_str";
        my $len       = length($hex_str) * 4;
        print("ops->number_($len, $hex_value)");
    }
    elsif ( $node->is_bin_node() ) {
        my $bin_str   = $node->get_value();
        my $hex_value = sprintf( "%x", oct("0b$bin_str") );
        my $len       = length($bin_str);
        print("ops->number_($len, $hex_value)");
    }
    elsif ( $node->is_return_node() ) {
        print("return;\n");
    }
    elsif ( $node->is_convert_node() ) {
        my $conv_insn = $node->get_value()->{insn};
        my $conv_type = $node->get_value()->{type};
        if ( $conv_insn =~ /^LOAD$/ ) {
            my $is_unsigned =
              ( $ArgsXlat::conv_args_tuple->[0]->[3]->{E_lit}->[0] ) eq
              "L_true";
            my $width_bytes =
              $ArgsXlat::conv_args_tuple->[0]->[4]->{E_lit}->[0]->{L_num};
            if ( !$is_unsigned ) {
                if ( $width_bytes eq "1" ) {
                    $conv_insn = "LB";
                }
                elsif ( $width_bytes eq "2" ) {
                    $conv_insn = "LH";
                }
                elsif ( $width_bytes eq "4" ) {
                    $conv_insn = "LW";
                }
                elsif ( $width_bytes eq "8" ) {
                    $conv_insn = "LD";
                }
            }
            else {
                if ( $width_bytes eq "1" ) {
                    $conv_insn = "LBU";
                }
                elsif ( $width_bytes eq "2" ) {
                    $conv_insn = "LHU";
                }
                elsif ( $width_bytes eq "4" ) {
                    $conv_insn = "LWU";
                }
            }
        }
        elsif ( $conv_insn =~ /STORE/ ) {
            my $width_bytes =
              $ParserConfig::conv_args_tuple->[0]->[3]->{E_lit}->[0]->{L_num};
            if ( $width_bytes eq "1" ) {
                $conv_insn = "SB";
            }
            elsif ( $width_bytes eq "2" ) {
                $conv_insn = "SH";
            }
            elsif ( $width_bytes eq "4" ) {
                $conv_insn = "SW";
            }
            elsif ( $width_bytes eq "8" ) {
                $conv_insn = "SD";
            }
        }
        my $orig_insn_mnemonic = "\"$conv_insn\"";
        my $orig_rose_insn     = "rose_riscv64_op_$conv_insn";

        print( "SgAsmRiscv64Instruction new_insn{0, $orig_insn_mnemonic, $orig_rose_insn};\n" );
        print_indent( "auto conv = Riscv64::IP_$conv_type\{\};\n", 2 );
        print_indent( "conv.p(d, ops, &new_insn, new_args, raw)",  2 );
    }
    elsif ( $node->is_for_node() ) {
        my $i = $node->get_value()->get_value();
        $lookup->{$i} = { is_cpp_var => 1 };
        my $ord = $node->get_attribute("ord");
        my ( $init_node, $fini_node, $step_node, $block_node ) =
          @{ $node->get_children() };
        if ( $ord eq "Ord_inc" ) {
            print("for ($i = ");
            print_raw($init_node);
            print("; $i <= ");
            print_raw($fini_node);
            print("; $i += ");
            print_raw($step_node);
            print(") {\n");
            print_indent( "", $indent_lvl + 1 );
            print_ast( $block_node, $curr_set, $lookup, $indent_lvl + 1 );
            print(";\n");
            print_indent( "};\n", $indent_lvl );
            print_indent( "",     $indent_lvl );
        }
        elsif ( $ord eq "Ord_dec" ) {
            print("for ($i = ");
            print_raw($init_node);
            print("; $i >= ");
            print_raw($fini_node);
            print("; $i -= ");
            print_raw($step_node);
            print(") {\n");
            print_indent( "", $indent_lvl + 1 );
            print_ast( $block_node, $curr_set, $lookup, $indent_lvl + 1 );
            print(";\n");
            print_indent( "};\n", $indent_lvl );
            print_indent( "",     $indent_lvl );
        }
    }
    elsif ( $node->is_while_node() ) {
        my ( $cond_node, $block_node ) = @{ $node->get_children() };
        print("while (");
        print_ast( $cond_node, $curr_set, $lookup, $indent_lvl );
        print(") {\n");
        print_indent( "", $indent_lvl + 1 );
        print_ast( $block_node, $curr_set, $lookup, $indent_lvl + 1 );
        print(";\n");
        print_indent( "}\n", $indent_lvl );

    }
    elsif ( $node->is_field_node() ) {
        print( $node->get_value()->{var} );
        print("_");
        print( $node->get_value()->{field} );
    }

    # Finally, handle all sibling nodes
    for my $sibling ( @{ $node->get_siblings() } ) {
        print(";\n");
        print_indent( "", $indent_lvl );
        print_ast( $sibling, $curr_set, $lookup, $indent_lvl );
    }
}

sub print_load_init_code {
    my ($indent_lvl) = @_;
    print_indent( "switch (insn->get_kind()) {\n", $indent_lvl );
    print_indent( "case rose_riscv64_op_lb:\n",    $indent_lvl + 1 );
    print_indent( "width = 1;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_lbu:\n",   $indent_lvl + 1 );
    print_indent( "width = 1;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 0;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_lh:\n",    $indent_lvl + 1 );
    print_indent( "width = 2;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_lhu:\n",   $indent_lvl + 1 );
    print_indent( "width = 2;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 0;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_lw:\n",    $indent_lvl + 1 );
    print_indent( "width = 4;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_lwu:\n",   $indent_lvl + 1 );
    print_indent( "width = 4;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 0;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_ld:\n",    $indent_lvl + 1 );
    print_indent( "width = 8;\n",                  $indent_lvl + 2 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "default:\n",                    $indent_lvl + 1 );
    print_indent( "assert(0);\n",                  $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "}\n",                           $indent_lvl );
}

sub print_store_init_code {
    my ($indent_lvl) = @_;
    print_indent( "switch (insn->get_kind()) {\n", $indent_lvl );
    print_indent( "case rose_riscv64_op_sb:\n",    $indent_lvl + 1 );
    print_indent( "width = 1;\n",                  $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_sh:\n",    $indent_lvl + 1 );
    print_indent( "width = 2;\n",                  $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_sw:\n",    $indent_lvl + 1 );
    print_indent( "width = 4;\n",                  $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_sd:\n",    $indent_lvl + 1 );
    print_indent( "width = 8;\n",                  $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "default:\n",                    $indent_lvl + 1 );
    print_indent( "assert(0);\n",                  $indent_lvl + 2 );
    print_indent( "}\n",                           $indent_lvl );
}

sub print_mul_init_code {
    my ($indent_lvl) = @_;
    print_indent( "switch (insn->get_kind()) {\n",  $indent_lvl );
    print_indent( "case rose_riscv64_op_mul:\n",    $indent_lvl + 1 );
    print_indent( "mul_op_signed_rs1 = 1;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_signed_rs2 = 1;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_high = 0;\n",             $indent_lvl + 2 );
    print_indent( "break;\n",                       $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_mulh:\n",   $indent_lvl + 1 );
    print_indent( "mul_op_signed_rs1 = 1;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_signed_rs2 = 1;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_high = 1;\n",             $indent_lvl + 2 );
    print_indent( "break;\n",                       $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_mulhsu:\n", $indent_lvl + 1 );
    print_indent( "mul_op_signed_rs1 = 1;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_signed_rs2 = 0;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_high = 1;\n",             $indent_lvl + 2 );
    print_indent( "break;\n",                       $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_mulhu:\n",  $indent_lvl + 1 );
    print_indent( "mul_op_signed_rs1 = 0;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_signed_rs2 = 0;\n",       $indent_lvl + 2 );
    print_indent( "mul_op_high = 1;\n",             $indent_lvl + 2 );
    print_indent( "break;\n",                       $indent_lvl + 2 );
    print_indent( "default:\n",                     $indent_lvl + 1 );
    print_indent( "assert(0);\n",                   $indent_lvl + 2 );
    print_indent( "}\n",                            $indent_lvl );
}

sub print_div_init_code {
    my ($indent_lvl) = @_;
    print_indent( "switch (insn->get_kind()) {\n", $indent_lvl );
    print_indent( "case rose_riscv64_op_div:\n",   $indent_lvl + 1 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_divu:\n",  $indent_lvl + 1 );
    print_indent( "is_unsigned = 0;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "default:\n",                    $indent_lvl + 1 );
    print_indent( "assert(0);\n",                  $indent_lvl + 2 );
    print_indent( "}\n",                           $indent_lvl );
}

sub print_rem_init_code {
    my ($indent_lvl) = @_;
    print_indent( "int is_unsigned;\n",            $indent_lvl );
    print_indent( "switch (insn->get_kind()) {\n", $indent_lvl );
    print_indent( "case rose_riscv64_op_rem:\n",   $indent_lvl + 1 );
    print_indent( "is_unsigned = 1;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "case rose_riscv64_op_remu:\n",  $indent_lvl + 1 );
    print_indent( "is_unsigned = 0;\n",            $indent_lvl + 2 );
    print_indent( "break;\n",                      $indent_lvl + 2 );
    print_indent( "default:\n",                    $indent_lvl + 1 );
    print_indent( "assert(0);\n",                  $indent_lvl + 2 );
    print_indent( "}\n",                           $indent_lvl );
}

1;
