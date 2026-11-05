package ASTBuildFunc;

use strict;
use warnings;

use Exporter 'import';
use ASTBuildHelper;
use lib "$FindBin::Bin";
use ParserCommon qw(
  XLEN
);

use ASTType qw(
  ROSE_OP_SIGN_EXTEND
  ROSE_OP_ZERO_EXTEND
  ROSE_OP_READ_REG
  ROSE_OP_WRITE_REG
  ROSE_OP_NUMBER
  ROSE_OP_BOOL
  ROSE_OP_ADD
  ROSE_OP_SUB
  ROSE_OP_SMUL
  ROSE_OP_SDIV
  ROSE_OP_SREM
  ROSE_OP_NEG
  ROSE_OP_EXTRACT
  ROSE_OP_SHIFT_LEFT
  ROSE_OP_SHIFT_RIGHT
  ROSE_OP_SHIFT_RIGHT_ARITH
  ROSE_OP_ROTATE_LEFT
  ROSE_OP_ROTATE_RIGHT
  ROSE_OP_BAND
  ROSE_OP_BOR
  ROSE_OP_BXOR
  ROSE_OP_BNOT
  ROSE_OP_EQ
  ROSE_OP_NEQ
  ROSE_OP_SIGNED_LT
  ROSE_OP_SIGNED_GT
  ROSE_OP_SIGNED_LEQ
  ROSE_OP_SIGNED_GEQ
  ROSE_OP_UNSIGNED_GT
  ROSE_OP_CONCAT
  ROSE_OP_GET_PC
  ROSE_OP_GET_NEXT_PC
  ROSE_OP_JUMP
  ROSE_OP_WRITE_MEM_VALUE
  ROSE_OP_WRITE_MEM_EA
  ROSE_OP_CLZ
  ROSE_OP_CTZ
  ROSE_OP_CMUL
  ROSE_OP_CMULR
  ROSE_OP_POPC
  ROSE_OP_REV8
  ROSE_OP_BREV8
  ROSE_OP_LOG2
);

our @EXPORT_OK = qw(
  @FN_DISPATCH
  @ERR_DISPATCH
);

sub unary_op_tmpl {
    my ( $self, $rose_func, $exps_ast, $curr_set ) = @_;
    my @exp_nodes     = map { $self->do_exp( $_, $curr_set ) } @$exps_ast;
    my $unary_op_node = ASTNode->new_func_node( $rose_func, \@exp_nodes );
    $unary_op_node->set_attribute( "unary_op",          1 );
    $unary_op_node->set_attribute( "is_const_foldable", 1 );
    return $unary_op_node;
}

sub binary_op_tmpl {
    my ( $self, $rose_func, $exps_ast, $curr_set ) = @_;
    my @exp_nodes      = map { $self->do_exp( $_, $curr_set ) } @$exps_ast;
    my $binary_op_node = ASTNode->new_func_node( $rose_func, \@exp_nodes );
    $binary_op_node->set_attribute( "binary_op",         1 );
    $binary_op_node->set_attribute( "is_const_foldable", 1 );
    return $binary_op_node;
}

sub unary_op {
    my ($rose_func) = @_;
    return sub {
        my ( $self, $exps_ast, $curr_set ) = @_;
        return unary_op_tmpl( $self, $rose_func, $exps_ast, $curr_set );
    };
}

sub binary_op {
    my ($rose_func) = @_;
    return sub {
        my ( $self, $exps_ast, $curr_set ) = @_;
        return binary_op_tmpl( $self, $rose_func, $exps_ast, $curr_set );
    };
}

our @FN_DISPATCH = (

    # Unary Operator
    [ qr/^not_(bit(s)?|vec|atom)$/,    unary_op(ROSE_OP_BNOT) ],
    [ qr/^negate_(bit(s)?|vec|atom)$/, unary_op(ROSE_OP_NEG) ],

    # Binary Operator
    [ qr/^add_(bit(s)?|vec|atom)$/,                   binary_op(ROSE_OP_ADD) ],
    [ qr/^sub_(bit(s)?|vec|atom)$/,                   binary_op(ROSE_OP_SUB) ],
    [ qr/^mult_atom$/,                                binary_op(ROSE_OP_SMUL) ],
    [ qr/^(quot(_positive)?_round_zero)|(ediv_int)$/, binary_op(ROSE_OP_SDIV) ],
    [ qr/^rem(_positive)?_round_zero$/,               binary_op(ROSE_OP_SREM) ],
    [ qr/^shift(l|_bits_left)$/,        binary_op(ROSE_OP_SHIFT_LEFT) ],
    [ qr/^shift(r|_bits_right)$/,       binary_op(ROSE_OP_SHIFT_RIGHT) ],
    [ qr/^shift(_bits)?_right_arith$/,  binary_op(ROSE_OP_SHIFT_RIGHT_ARITH) ],
    [ qr/^rotate(l|_bits_left)$/,       binary_op(ROSE_OP_ROTATE_LEFT) ],
    [ qr/^rotate(r|_bits_right)$/,      binary_op(ROSE_OP_ROTATE_RIGHT) ],
    [ qr/^and_vec$/,                    binary_op(ROSE_OP_BAND) ],
    [ qr/^or_vec$/,                     binary_op(ROSE_OP_BOR) ],
    [ qr/^xor_vec$/,                    binary_op(ROSE_OP_BXOR) ],
    [ qr/^eq_(bit(s)?|int|anything)$/,  binary_op(ROSE_OP_EQ) ],
    [ qr/^neq_(bit(s)?|int|anything)$/, binary_op(ROSE_OP_NEQ) ],
    [ qr/^gt_int$/,                     binary_op(ROSE_OP_SIGNED_GT) ],
    [ qr/^lt_int$/,                     binary_op(ROSE_OP_SIGNED_LT) ],
    [ qr/^gteq_int$/,                   binary_op(ROSE_OP_SIGNED_GEQ) ],
    [ qr/^lteq_int$/,                   binary_op(ROSE_OP_SIGNED_LEQ) ],

    # Identity / constants

    [
        qr/^__id$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return $self->do_exp( $exps_ast->[0], $curr_set );
        }
    ],

    [
        qr/^zeros$/,
        sub {
            return ASTNode->new_num_node(0);
        }
    ],

    [
        qr/^ones$/,
        sub {
            return ASTNode->new_num_node(1);
        }
    ],

    # Other arithmetic operations

    [
        qr/^pow2$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_SHIFT_LEFT,
                [
                    ASTNode->new_num_node(1),
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                ]
            );
        }
    ],

    [
        qr/^log2$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_LOG2,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^min_int$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $exp1_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $exp2_node = $self->do_exp( $exps_ast->[1], $curr_set );
            return ASTNode->new_ite_node(
                ASTNode->new_func_node(
                    ROSE_OP_SIGNED_LT, [ $exp1_node, $exp2_node ]
                ),
                $exp1_node,
                $exp2_node
            );
        }
    ],

    [
        qr/^max_int$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $exp1_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $exp2_node = $self->do_exp( $exps_ast->[1], $curr_set );
            return ASTNode->new_ite_node(
                ASTNode->new_func_node(
                    ROSE_OP_SIGNED_GT, [ $exp1_node, $exp2_node ]
                ),
                $exp1_node,
                $exp2_node
            );
        }
    ],

    # Signedness annotations

    [
        qr/^signed$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $n = $self->do_exp( $exps_ast->[0], $curr_set );
            $n->set_attribute( "signedness", 1 );
            return $n;
        }
    ],

    [
        qr/^unsigned$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $n = $self->do_exp( $exps_ast->[0], $curr_set );
            $n->set_attribute( "signedness", 0 );
            return $n;
        }
    ],

    # Extensions

    [
        qr/^sign_extend$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_SIGN_EXTEND,
                [
                    $self->do_exp( $exps_ast->[1], $curr_set ),
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                ]
            );
        }
    ],

    [
        qr/^zero_extend$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_ZERO_EXTEND,
                [
                    $self->do_exp( $exps_ast->[1], $curr_set ),
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                ]
            );
        }
    ],

    # Boolean ops

    [
        qr/^not(_bool)?$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTBuildHelper::build_boolean_not(
                $self->do_exp( $exps_ast->[0], $curr_set ) );
        }
    ],

    [
        qr/^and(_bool)?$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTBuildHelper::build_boolean_and(
                $self->do_exp( $exps_ast->[0], $curr_set ),
                $self->do_exp( $exps_ast->[1], $curr_set ),
            );
        }
    ],

    [
        qr/^or(_bool)?$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTBuildHelper::build_boolean_or(
                $self->do_exp( $exps_ast->[0], $curr_set ),
                $self->do_exp( $exps_ast->[1], $curr_set ),
            );
        }
    ],

    # Bit operations

    [
        qr/^rev8$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_REV8,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^brev8$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_BREV8,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^count_ones$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_POPC,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^count_leading_zeros$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_CLZ,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^count_trailing_zeros$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_CTZ,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^carryless_mul$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_CMUL,
                [
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                    $self->do_exp( $exps_ast->[1], $curr_set )
                ]
            );
        }
    ],

    [
        qr/^carryless_mulr$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_CMULR,
                [
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                    $self->do_exp( $exps_ast->[1], $curr_set )
                ]
            );
        }
    ],

    [
        qr/^extend_value$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $id  = $self->do_exp( $exps_ast->[0], $curr_set );
            my $exp = $self->do_exp( $exps_ast->[1], $curr_set );
            return ASTNode->new_ite_node(
                $id,
                ASTNode->new_func_node(
                    ROSE_OP_SIGN_EXTEND, [ $exp, ASTNode->new_num_node(XLEN) ]
                ),
                ASTNode->new_func_node(
                    ROSE_OP_ZERO_EXTEND, [ $exp, ASTNode->new_num_node(XLEN) ]
                ),
            );
        }
    ],

    # Conversions

    [
        qr/^to_bits$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return $self->do_exp( $exps_ast->[1], $curr_set );
        }
    ],

    [
        qr/^(to_bits_truncate|trunc)$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_EXTRACT,
                [
                    $self->do_exp( $exps_ast->[1], $curr_set ),
                    ASTNode->new_num_node(0),
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                ]
            );
        }
    ],

    [
        qr/^bool_to_bit(s)?$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_NUMBER,
                [
                    ASTNode->new_num_node(XLEN),
                    $self->do_exp( $exps_ast->[0], $curr_set )
                ]
            );
        }
    ],

    [
        qr/^bit_to_bool$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_BOOL,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^bits_of_virtaddr$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my ($id_ast) = @{$exps_ast};
            return $self->do_exp( $id_ast, $curr_set );
        }
    ],

    [
        qr/^subrange_bits$/,
        sub {
            my ( $self, $exps_ast, $curr_set )             = @_;
            my ( $value_ast, $high_bit_ast, $low_bit_ast ) = @{$exps_ast};
            my $value_node    = $self->do_exp( $value_ast,    $curr_set );
            my $high_bit_node = $self->do_exp( $high_bit_ast, $curr_set );
            my $low_bit_node  = $self->do_exp( $low_bit_ast,  $curr_set );
            return ASTNode->new_func_node( ROSE_OP_EXTRACT,
                [ $value_node, $low_bit_node, $high_bit_node ] );

        }
    ],
    [
        qr/bitvector_concat/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my ( $concat_id_ast, $concat_bit_ast ) = @{$exps_ast};
            my $concat_id_node = $self->do_exp( $concat_id_ast, $curr_set );
            my $concat_bit_hex_node =
              $self->do_exp( $concat_bit_ast, $curr_set );
            my $concat_bit_hex = $concat_bit_hex_node->get_value();
            my $concat_bit_len = length($concat_bit_hex) * 4;
            my $concat_bit_num = sprintf( "%x", $concat_bit_hex );

            # Get the bit length of value
            my $concat_bit_len_node = ASTNode->new_num_node($concat_bit_len);
            my $shift_left_node = ASTNode->new_func_node( ROSE_OP_SHIFT_LEFT,
                [ $concat_id_node, $concat_bit_len_node ] );
            return $concat_bit_num
              ? ASTNode->new_func_node( ROSE_OP_ADD,
                [ $shift_left_node, ASTNode->new_num_node($concat_bit_num) ] )
              : $shift_left_node;

        }
    ],
    [
        qr/^bitvector_access$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            my $value_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $bit_node   = $self->do_exp( $exps_ast->[1], $curr_set );

            return ASTNode->new_func_node( ROSE_OP_EXTRACT,
                [ $value_node, $bit_node, $bit_node ] );
        }
    ],

    [
        qr/^bitvector_update$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;

            my $value_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $bit_node   = $self->do_exp( $exps_ast->[1], $curr_set );
            my $bit        = $bit_node->get_value();

            if ( $bit == 0 ) {
                return ASTNode->new_func_node(
                    ROSE_OP_CONCAT,
                    [
                        ASTNode->new_func_node(
                            ROSE_OP_EXTRACT,
                            [
                                $value_node,
                                ASTNode->new_num_node(1),
                                ASTNode->new_num_node( XLEN - 1 ),
                            ]
                        ),
                        $bit_node
                    ]
                );
            }
            elsif ( $bit == XLEN - 1 ) {
                return ASTNode->new_func_node(
                    ROSE_OP_CONCAT,
                    [
                        $bit_node,
                        ASTNode->new_func_node(
                            ROSE_OP_EXTRACT,
                            [
                                $value_node,
                                ASTNode->new_num_node(0),
                                ASTNode->new_num_node( XLEN - 2 ),
                            ]
                        )
                    ]
                );
            }
            else {
                return ASTNode->new_func_node(
                    ROSE_OP_CONCAT,
                    [
                        ASTNode->new_func_node(
                            ROSE_OP_EXTRACT,
                            [
                                $value_node,
                                ASTNode->new_num_node( $bit + 1 ),
                                ASTNode->new_num_node( XLEN - 1 ),
                            ]
                        ),
                        ASTNode->new_func_node(
                            ROSE_OP_CONCAT,
                            [
                                $bit_node,
                                ASTNode->new_func_node(
                                    ROSE_OP_EXTRACT,
                                    [
                                        $value_node,
                                        ASTNode->new_num_node(0),
                                        ASTNode->new_num_node( $bit - 1 ),
                                    ]
                                )
                            ]
                        )
                    ]
                );
            }
        }
    ],

    [
        qr/^signed_saturation$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;

            my $len_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $exp_node = $self->do_exp( $exps_ast->[1], $curr_set );

            my $min_value_node = ASTNode->new_func_node(
                ROSE_OP_NEG,
                [
                    ASTNode->new_func_node(
                        ROSE_OP_SHIFT_LEFT,
                        [
                            ASTNode->new_num_node(1),
                            ASTNode->new_func_node(
                                ROSE_OP_SUB,
                                [ $len_node, ASTNode->new_num_node(1) ]
                            )
                        ]
                    ),
                    ASTNode->new_num_node(1)
                ]
            );

            my $max_value_node = ASTNode->new_func_node(
                ROSE_OP_SUB,
                [
                    ASTNode->new_func_node(
                        ROSE_OP_SHIFT_LEFT,
                        [
                            ASTNode->new_num_node(1),
                            ASTNode->new_func_node(
                                ROSE_OP_SUB,
                                [ $len_node, ASTNode->new_num_node(1) ]
                            )
                        ]
                    ),
                    ASTNode->new_num_node(1)
                ]
            );

            return ASTNode->new_ite_node(
                ASTNode->new_func_node(
                    ROSE_OP_SIGNED_GT, [ $exp_node, $max_value_node ]
                ),
                $max_value_node,
                ASTNode->new_ite_node(
                    ASTNode->new_func_node(
                        ROSE_OP_SIGNED_LT, [ $exp_node, $min_value_node ]
                    ),
                    $min_value_node,
                    $exp_node
                )
            );
        }
    ],

    [
        qr/^unsigned_saturation$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;

            my $len_node = $self->do_exp( $exps_ast->[0], $curr_set );
            my $exp_node = $self->do_exp( $exps_ast->[1], $curr_set );

            my $max_value_node = ASTNode->new_func_node(
                ROSE_OP_SUB,
                [
                    ASTNode->new_func_node(
                        ROSE_OP_SHIFT_LEFT,
                        [ ASTNode->new_num_node(1), $len_node ]
                    ),
                    ASTNode->new_num_node(1)
                ]
            );

            return ASTNode->new_ite_node(
                ASTNode->new_func_node(
                    ROSE_OP_UNSIGNED_GT, [ $exp_node, $max_value_node ]
                ),
                $max_value_node,
                $exp_node
            );
        }
    ],

    # PC / control flow

    [ qr/^get_arch_pc$/, sub { ASTNode->new_func_node(ROSE_OP_GET_PC) } ],
    [ qr/^get_next_pc$/, sub { ASTNode->new_func_node(ROSE_OP_GET_NEXT_PC) } ],

    [
        qr/^(set_next_pc|jump_to)$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_JUMP,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    # Registers

    [
        qr/^rX(_pair)?_(S|bits)$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node( ROSE_OP_READ_REG,
                [ $self->do_exp( $exps_ast->[0], $curr_set ) ] );
        }
    ],

    [
        qr/^wX(_pair)?_(S|bits)$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;
            return ASTNode->new_func_node(
                ROSE_OP_WRITE_REG,
                [
                    $self->do_exp( $exps_ast->[0], $curr_set ),
                    $self->do_exp( $exps_ast->[1], $curr_set ),
                ]
            );
        }
    ],

    # Memory

    [
        qr/^mem_write_value$/,
        sub {
            return ASTNode->new_func_node(ROSE_OP_WRITE_MEM_VALUE);
        }
    ],

    [
        qr/^mem_write_ea$/,
        sub {
            return ASTNode->new_func_node(ROSE_OP_WRITE_MEM_EA);
        }
    ],

    # Misc

    [
        qr/^execute$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;

            my ($exp) = @$exps_ast;

            my $conv_type      = $exp->{E_app}[0]{Id};
            my $new_insn_index = $ParserConfig::C_INSN_NEW_INSN{$curr_set};

            my $tuple_elems = $exp->{E_app}[1][0]{E_tuple};

            my $conv_insn =
                $new_insn_index != -1
              ? $tuple_elems->[0][$new_insn_index]{E_id}[0]{Id}
              : $conv_type;

            $conv_insn =~ s/^RISCV_//;
            $conv_insn = lc($conv_insn) =~ tr/./_/r;

            return ASTNode->new_convert_node( $conv_insn, $conv_type );
        }
    ],

    [
        qr/^currentlyEnabled$/,
        sub {
            my ( $self, $exps_ast, $curr_set ) = @_;

            my ($id_ast) = @$exps_ast;
            my $id_node = $self->do_exp( $id_ast, $curr_set );
            my $id = $id_node->get_value();

            if ( grep( /^$id$/, @ParserConfig::supported_riscv_exts ) ) {
                return ASTNode->new_bool_node(1);
            }
            return ASTNode->new_bool_node(0);
        }
    ],

    # Functions already handled in E_match

    [ qr/^(v?)mem_(read|write)$/, sub { ASTNode->new_null_node() } ],
    [ qr/^ext_control_check_pc$/, sub { ASTNode->new_null_node() } ],
    [ qr/^ext_data_get_addr$/,    sub { ASTNode->new_null_node() } ],
    [ qr/^is_aligned_(p|v)addr$/, sub { ASTNode->new_null_node() } ],
    [ qr/^translateAddr$/,        sub { ASTNode->new_null_node() } ],
    [ qr/^creg2reg_idx$/,         sub { ASTNode->new_null_node() } ],
    [ qr/^Some$/,                 sub { ASTNode->new_null_node() } ],
    [ qr/^None$/,                 sub { ASTNode->new_null_node() } ],

    # Unintersting functions

    [ qr/^print$/,                      sub { ASTNode->new_null_node() } ],
    [ qr/^update_elp_state|reset_elp$/, sub { ASTNode->new_null_node() } ],
    [ qr/^bool_bits_forward$/,          sub { ASTNode->new_null_node() } ],
    [ qr/^cancel_reservation$/,         sub { ASTNode->new_null_node() } ],
    [ qr/^Retire_Success$/,             sub { ASTNode->new_null_node() } ],

    # Exceptions

    [ qr/^internal_error$/,             sub { ASTNode->new_exception_node() } ],
    [ qr/^Illegal_Instruction$/,        sub { ASTNode->new_exception_node() } ],
    [ qr/^(.*)_Exception$/,             sub { ASTNode->new_exception_node() } ],
    [ qr/^E_SAMO_(.*)$/,                sub { ASTNode->new_exception_node() } ],
    [ qr/^Ext_DataAddr_Check_Failure$/, sub { ASTNode->new_exception_node() } ],

);

our @ERR_DISPATCH = (

    # Floating Points
    [
        qr/^(r|w)F(_or_X)?(_(D|H|S)|_bits|_BF16)?$/,
        sub {
            my ($fn_id) = @_;
            die
            "Error: Floating points register read/write function $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^riscv_f(16|32|64).*$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^accrue_fflags$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^riscv_(u?i(32|64)ToF(16|32|64))$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^f_is_((Q|S)?NAN|(pos|neg)_(inf|norm|subnorm|zero))_(D|H|S)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(fp_(add|class|div|eq|ge|gt|le|lt|max|min|mul|muladd|mulsub|nmuladd|nmulsub|sub|widen))$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(fle|fmake|fsplit|negate)_(BF16|D|H|S)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^select_instr_or_fcsr_rm$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^encdec_rounding_mode_forwards$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^get_scalar(_fp)?$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^bf16_to_f32(_set_flags)?$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^fcvtmod_helper$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^cfregidx_to_fregidx$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Floating points operation $fn_id not supported in ROSE C++";
        },
    ],
    [
        qr/^nan_box|canonical_NaN_(D|H|S)$/,
        sub {
            my ($fn_id) = @_;
            die
            "Error: Floating points Nan operation $fn_id not supported in ROSE C++";
        }
    ],

    # Vectors
    [
        qr/^((read|write)_vreg(_seg)?)|((r|w)V(_bits)?)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector registers read/write function $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^plain_vector_(access|update)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^vector_(init|length)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^recip7$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^fixed_rounding_incr$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^init_masked_(result(_cmp|_carry)?|source)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(read|write)_vmask(_carry)?$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^illegal_fp_(normal|variable|width|vd_masked|vd_unmasked)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^illegal_(load|normal|reduction|store|variable_width)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^illegal_(vd_masked|vd_unmasked|widening_reduction)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(set|assert)_vstart$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^get_(sew|sew_pow|sew_bytes|lmul_pow|num_elem)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^get_(slice_int|shift_amount|vtype_vta)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^get_((start|end)_element)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(get|write)_velem_(oct_vec|quad|quad_vec)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^vlewidth_pow_forwards$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^_get_Vtype_(reserved|vill|vlmul|vma|vsew|vta)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^to_bits_unsafe$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^Mk_Vtype$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^handle_illegal_vtype$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^execute_vsetvl_type$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^process_(rfvv_single|clean_inval|rfvv_widening_reduction)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^process_((vl|vs)(re|seg|segff|sseg|xseg)|vm)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^(read|write)_single_element$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^valid_reg_overlap$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        },
    ],

    [
        qr/^fixed_rounding_incr$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        }
    ],
    [
        qr/^vtype_vta$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector operations $fn_id not supported in ROSE C++";
        }
    ],

    # Vector Cryptography

    [
        qr/^zvk(.*)|vrev8$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Vector K extension operation $fn_id not supported in ROSE C++";
        }
    ],

    # System
    [
        qr/^E_(U|S|M)_EnvCall$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Ecall functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^Trap$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Trap functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^_get_Mstatus_(TSR|TVM|TW)|(read|do)CSR|_get_Fcsr_FRM$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Control status functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^effective_fence_set|is_fiom_activ|sail_barrier$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Fence function $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^Enter_Wait$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Wait for interruption functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^CLT_(M|S)RET|Ext_XRET_Priv_Failure|ext_check_xret_priv$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Return from trap functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^make_landing_pad_exception$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Landing pad functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^(add_to|flush|lookup|match|reset)_TLB(_Entry)?|translate_TLB_(hit|miss)$/,
        sub {
            my ($fn_id) = @_;
            die "Error: TLB functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^(trap)(_callback|Cause_bits_forwards|Cause_is_interrupt|Cause_to_str|_handler|VectorMode_of_bits)|Trap$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Trap functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^exception_handler$/,
        sub {
            my ($fn_id) = @_;
            die
            "Error: Exception handler functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^E_breakpoint$/,
        sub {
            my ($fn_id) = @_;
            die "Error: Breakpoint functions $fn_id not supported in ROSE C++";
        }
    ],

    [
        qr/^cbo_clean_flush_enabled|cbop_priv_check|cbo_zero_enabled|process_clean_inval$/,
        sub {
            my ($fn_id) = @_;
            die
            "Error: Cache-Block management functions $fn_id not supported in ROSE C++";
        }
    ]
);

1;
