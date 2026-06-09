package ASTBuildHelper;

use strict;
use warnings;

use Exporter 'import';
use lib "$FindBin::Bin";
use ASTType qw(
  ROSE_OP_ADD
  ROSE_OP_SUB
  ROSE_OP_BOOL
  ROSE_OP_CONCAT
  ROSE_OP_EXTRACT
  ROSE_OP_BAND
  ROSE_OP_BOR
  ROSE_OP_BNOT
);
use ParserCommon qw(
  XLEN
);

our @EXPORT_OK = qw(
  build_boolean_not
  build_boolean_and
  build_boolean_or
  build_bitvec_index_assign
  build_bitvec_range_assign
);

sub build_boolean_not {
    my ($exp_node) = @_;
    return ASTNode->new_func_node( ROSE_OP_BNOT,
        [ ASTNode->new_func_node( ROSE_OP_BOOL, [$exp_node] ), ] );
}

sub build_boolean_and {
    my ( $exp1_node, $exp2_node ) = @_;
    return ASTNode->new_func_node(
        ROSE_OP_BAND,
        [
            ASTNode->new_func_node( ROSE_OP_BOOL, [$exp1_node] ),
            ASTNode->new_func_node( ROSE_OP_BOOL, [$exp2_node] ),
        ]
    );
}

sub build_boolean_or {
    my ( $exp1_node, $exp2_node ) = @_;
    return ASTNode->new_func_node(
        ROSE_OP_BOR,
        [
            ASTNode->new_func_node( ROSE_OP_BOOL, [$exp1_node] ),
            ASTNode->new_func_node( ROSE_OP_BOOL, [$exp2_node] ),
        ]
    );
}

sub build_bitvec_index_assign {
    my ( $bitvec_node, $idx_node, $value_node ) = @_;
    return ASTNode->new_func_node(
        ROSE_OP_CONCAT,
        [
            ASTNode->new_func_node(
                ROSE_OP_EXTRACT,
                [
                    $bitvec_node,
                    ASTNode->new_num_node(0),
                    ASTNode->new_func_node(
                        ROSE_OP_SUB, [ $idx_node, ASTNode->new_num_node(1) ]
                    )
                ]
            ),
            ASTNode->new_func_node(
                ROSE_OP_CONCAT,
                [
                    $value_node,
                    ASTNode->new_func_node(
                        ROSE_OP_EXTRACT,
                        [
                            $bitvec_node,
                            ASTNode->new_func_node(
                                ROSE_OP_ADD,
                                [ $idx_node, ASTNode->new_num_node(1) ]
                            ),
                            ASTNode->new_num_node(XLEN)
                        ]
                    )
                ]
            )
        ]
    );
}

sub build_bitvec_range_assign {
    my ( $bitvec_node, $beg_node, $end_node, $value_node ) = @_;
    return ASTNode->new_func_node(
        ROSE_OP_CONCAT,
        [
            ASTNode->new_func_node(
                ROSE_OP_EXTRACT,
                [
                    $bitvec_node,
                    ASTNode->new_num_node(0),
                    ASTNode->new_func_node(
                        ROSE_OP_SUB, [ $beg_node, ASTNode->new_num_node(1) ]
                    )
                ]
            ),
            ASTNode->new_func_node(
                ROSE_OP_CONCAT,
                [
                    $value_node,
                    ASTNode->new_func_node(
                        ROSE_OP_EXTRACT,
                        [
                            $bitvec_node,
                            ASTNode->new_func_node(
                                ROSE_OP_ADD,
                                [ $end_node, ASTNode->new_num_node(1) ]
                            ),
                            ASTNode->new_num_node(XLEN)
                        ]
                    )
                ]
            )
        ]
    );
}

1;
