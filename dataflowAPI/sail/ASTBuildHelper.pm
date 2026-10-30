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
  build_extract
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

# SAIL bit ranges name the top bit (inclusive); ROSE's extract takes an
# exclusive end.  Build extract(value, low, high + 1).
sub build_extract {
    my ( $value_node, $low_node, $high_node ) = @_;
    my $end_node;

    # v[n - 1 .. 0] is common; end it at n rather than n - 1 + 1
    if (   $high_node->is_func_node()
        && $high_node->is_op(ROSE_OP_SUB)
        && $high_node->get_nth_child(1)->is_num_node()
        && $high_node->get_nth_child(1)->get_value() == 1 )
    {
        $end_node = $high_node->get_nth_child(0);
    }
    else {
        $end_node = ASTNode->new_func_node( ROSE_OP_ADD,
            [ $high_node, ASTNode->new_num_node(1) ] );
        $end_node->set_attribute( "binary_op",         1 );
        $end_node->set_attribute( "is_const_foldable", 1 );
    }
    return ASTNode->new_func_node( ROSE_OP_EXTRACT,
        [ $value_node, $low_node, $end_node ] );
}

sub build_bitvec_index_assign {
    my ( $bitvec_node, $idx_node, $value_node ) = @_;
    return build_bitvec_range_assign( $bitvec_node, $idx_node, $idx_node,
        $value_node );
}

# bitvec[high .. low] = value, as concat(bitvec[low-1 .. 0], value,
# bitvec[XLEN-1 .. high+1]); ROSE's concat puts its first operand low.
sub build_bitvec_range_assign {
    my ( $bitvec_node, $high_node, $low_node, $value_node ) = @_;
    my $above_node = ASTNode->new_func_node( ROSE_OP_ADD,
        [ $high_node, ASTNode->new_num_node(1) ] );
    $above_node->set_attribute( "binary_op",         1 );
    $above_node->set_attribute( "is_const_foldable", 1 );
    return ASTNode->new_func_node(
        ROSE_OP_CONCAT,
        [
            ASTNode->new_func_node(
                ROSE_OP_EXTRACT,
                [ $bitvec_node, ASTNode->new_num_node(0), $low_node ]
            ),
            ASTNode->new_func_node(
                ROSE_OP_CONCAT,
                [
                    $value_node,
                    ASTNode->new_func_node(
                        ROSE_OP_EXTRACT,
                        [
                            $bitvec_node, $above_node,
                            ASTNode->new_num_node(XLEN)
                        ]
                    )
                ]
            )
        ]
    );
}

1;
