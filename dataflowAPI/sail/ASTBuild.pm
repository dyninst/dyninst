package ASTBuild;

use strict;
use warnings;

use Data::Dumper;

use Exporter 'import';
use lib "$FindBin::Bin";
use ASTBuildFunc qw(@FN_DISPATCH @ERR_DISPATCH);
use ASTType      qw(
  ROSE_OP_CONCAT
  ROSE_OP_EADDR
  ROSE_OP_EXTRACT
  ROSE_OP_JUMP
  ROSE_OP_READ_IMM
  ROSE_OP_READ_MEM
  ROSE_OP_READ_REG
  ROSE_OP_SIGNED_GEQ
  ROSE_OP_SIGNED_GT
  ROSE_OP_SIGNED_LEQ
  ROSE_OP_SIGNED_LT
  ROSE_OP_UNSIGNED_GEQ
  ROSE_OP_UNSIGNED_GT
  ROSE_OP_UNSIGNED_LEQ
  ROSE_OP_UNSIGNED_LT
  ROSE_OP_WRITE_MEM
  ROSE_OP_WRITE_REG
);
use ParserConfig;
use ParserCommon qw(
  TEMP_VAR
  WIDTH_BYTES_VAR
);

sub parse_ast {
    my ( $self, $ast, $curr_set ) = @_;
    return $self->do_exp( $ast, $curr_set );
}

sub do_pat {
    my ( $self, $ast, $curr_set ) = @_;

    if ( my $papp = $ast->{P_app} ) {
        my ( $id_ast, $args_ast ) = @$papp;
        my $id   = $id_ast->{Id};
        my @args = map { $self->do_pat( $_, $curr_set ) } @$args_ast;
        return ASTNode->new_app_node( $id, \@args );
    }

    if ( my $plit = $ast->{P_lit} ) {
        return ASTNode->new_lit_node( $plit->[0] );
    }

    if ( my $pid = $ast->{P_id} ) {
        return ASTNode->new_var_node( $pid->[0]{Id} );
    }

    if ( my $ptuple = $ast->{P_tuple} ) {
        my @elems = map { $self->do_pat( $_, $curr_set ) } @{ $ptuple->[0] };
        return ASTNode->new_tuple_node( \@elems );
    }

    if ( my $ptype = $ast->{P_typ} ) {

        # Ignore type annotation
        my ( $p_type, $p_id ) = @$ptype;
        return $self->do_pat( $p_id, $curr_set );
    }

    if ( my $pvar = $ast->{P_var} ) {
        return ASTNode->new_var_node( $pvar->[0]{P_id}[0]{Id} );
    }

    if ( my $pvec = $ast->{P_vector} ) {
        my @elems = map { $self->do_pat( $_, $curr_set ) } @{ $pvec->[0] };
        return ASTNode->new_vec_node( \@elems );
    }

    if ( my $pvec = $ast->{P_vector_concat} ) {
        my ( $p_id_ast, $p_concat_ast ) = @$pvec;
        my $p_id_node     = $self->do_pat( $p_id_ast, $curr_set );
        my $p_concat_node = $self->do_pat( $p_id_ast, $curr_set );
        return ASTNode->new_func_node( ROSE_OP_CONCAT,
            [ $p_id_node, $p_concat_node ] );
    }

    if ( my $pvec = $ast->{P_vector_subrange} ) {
        my ( $p_id_ast, $p_beg_ast, $p_end_ast ) = @$pvec;
        my $p_id_node  = $self->do_pat( $p_id_ast,  $curr_set );
        my $p_beg_node = $self->do_pat( $p_beg_ast, $curr_set );
        my $p_end_node = $self->do_pat( $p_end_ast, $curr_set );
        return ASTNode->new_func_node( ROSE_OP_EXTRACT,
            [ $p_id_node, $p_beg_node, $p_end_node ] );
    }

    if ( exists $ast->{P_wild} ) {
        return ASTNode->new_wild_node;
    }

    die "Unknown pattern AST: " . Dumper($ast);
}

sub do_pexp {
    my ( $self, $ast, $curr_set ) = @_;

    if ( my $pexp = $ast->{Pat_exp} ) {
        my ( $key_ast, $val_ast ) = @$pexp;
        return [
            $self->do_pat( $key_ast, $curr_set ),
            $self->do_exp( $val_ast, $curr_set ),
        ];
    }

    die "Unknown paired pattern AST" . Dumper($ast);
}

sub do_varbind {
    my ( $self, $var_ast, $exp_ast, $curr_set ) = @_;

    # Ignore redundant assignments in C-extensions
    return ASTNode->new_null_node()
      if $curr_set =~ /^C_/;

    # Process id and expression
    my $var_node = $self->do_lexp( $var_ast, $curr_set );
    my $exp_node = $self->do_exp( $exp_ast, $curr_set );

    $var_node->is_var_node() or die "Unexpected error: id is not variable";

    # Skip if either side evaluates to null
    return ASTNode->new_null_node()
      if $var_node->is_null_node || $exp_node->is_null_node;

    # Skip if assign to itself
    return ASTNode->new_null_node()
      if $exp_node->is_var_node
      && $var_node->get_value() eq $exp_node->get_value();

    return ASTNode->new_assign_node( $var_node, $exp_node );
}

sub do_letbind {
    my ( $self, $ast, $curr_set ) = @_;
    my ( $pat_ast, $exp_ast ) = @{ $ast->{LB_val} };

    # Ignore redundant assignments in C-extensions
    return ASTNode->new_null_node()
      if $curr_set =~ /^C_/;

    # Process pattern and expression
    my $id_node  = $self->do_pat( $pat_ast, $curr_set );
    my $exp_node = $self->do_exp( $exp_ast, $curr_set );

    # Skip if either side evaluates to null
    return ASTNode->new_null_node()
      if $id_node->is_null_node || $exp_node->is_null_node;

    # Skip if assign to itself
    return ASTNode->new_null_node
      if $exp_node->is_var_node
      && $id_node->get_value() eq $exp_node->get_value();

    return ASTNode->new_assign_node( $id_node, $exp_node );
}

sub do_lexp {
    my ( $self, $ast, $curr_set ) = @_;

    if ( my $id = $ast->{LE_id} ) {
        return ASTNode->new_var_node( $id->[0]{Id} );
    }

    # TODO Fix this
    if ( my $deref = $ast->{LE_deref} ) {
        my ($expr_ast) = @$deref;
        my $expr_node = $self->do_exp( $expr_ast, $curr_set );
        return ASTNode->new_func_node( ROSE_OP_READ_MEM, [$expr_node] );
    }

    if ( my $app = $ast->{LE_app} ) {
        my ( $id_ast, $args_ast ) = @$app;
        my $id   = $id_ast->{Id};
        my @args = map { $self->do_pat( $_, $curr_set ) } @$args_ast;
        return ASTNode->new_app_node( $id, \@args );
    }

    if ( my $typed = $ast->{LE_typ} ) {

        # Ignore type
        return ASTNode->new_var_node( $typed->[1]{Id} );
    }

    if ( my $tuple = $ast->{LE_tuple} ) {
        my @elems = map { $self->do_pat( $_, $curr_set ) } @{ $tuple->[0] };
        return ASTNode->new_tuple_node( \@elems );
    }

    if ( my $vec = $ast->{LE_vector} ) {
        my ( $vec_ast, $index_ast ) = @$vec;
        my $vec_node   = $self->do_lexp( $vec_ast, $curr_set );
        my $index_node = $self->do_exp( $index_ast, $curr_set );
        return ASTNode->new_vec_index_node( $vec_node, $index_node );
    }

    if ( my $range = $ast->{LE_vector_range} ) {
        my ( $vec_ast, $start_ast, $end_ast ) = @$range;
        my $vec_node   = $self->do_lexp( $vec_ast, $curr_set );
        my $start_node = $self->do_exp( $start_ast, $curr_set );
        my $end_node   = $self->do_exp( $end_ast,   $curr_set );
        return ASTNode->new_vec_range_node( $vec_node, $start_node, $end_node );
    }

    if ( my $vec = $ast->{LE_vector_concat} ) {
        my ( $vec_ast, $concat_ast ) = @$vec;
        my $vec_node    = $self->do_lexp( $vec_ast, $curr_set );
        my $concat_node = $self->do_exp( $concat_ast, $curr_set );
        return ASTNode->new_vec_concat_node( $vec_node, $concat_node );
    }

    if ( my $field = $ast->{LE_field} ) {
        my ( $field_ast, $id_ast ) = @$field;
        my $var   = $field_ast->{LE_id}[0]{Id};
        my $field = $id_ast->{Id};
        return ASTNode->new_field_node( $var, $field );
    }

    die "Unknown l-value expression AST" . Dumper($ast);
}

sub do_exp {
    my ( $self, $ast, $curr_set ) = @_;

    if ( my $lit_ast = $ast->{E_lit} ) {

        my $val_ast = $lit_ast->[0];

        # The value could either be a string or a has like { L_num: {...} }

        if ( ref($val_ast) eq "HASH" ) {
            if ( exists $val_ast->{L_num} ) {
                my $num = $val_ast->{L_num};
                return ASTNode->new_num_node($num);
            }

            if ( exists $val_ast->{L_hex} ) {
                my $hex = $val_ast->{L_hex};
                return ASTNode->new_hex_node($hex);
            }

            if ( exists $val_ast->{L_bin} ) {
                my $bin = $val_ast->{L_bin};
                return ASTNode->new_bin_node($bin);
            }

            if ( exists $val_ast->{L_string} ) {
                my $str = $val_ast->{L_string};
                return ASTNode->new_str_node($str);
            }
        }
        else {
            if ( $val_ast =~ /^L_(true|one)$/ ) {
                return ASTNode->new_bool_node(1);
            }

            if ( $val_ast =~ /^L_(false|zero)$/ ) {
                return ASTNode->new_bool_node(0);

            }

            if ( $val_ast =~ /^L_unit$/ ) {
                return ASTNode->new_unit_node();
            }
        }

        return ASTNode->new_null_node();
    }
    if ( my $id_ast = $ast->{E_id} ) {
        my $id = $id_ast->[0]{Id};

        # Ignore RETIRE_* pseudo-values
        return ASTNode->new_null_node()
          if $id eq "RETIRE_SUCCESS" || $id eq "RETIRE_FAIL";

        $id = ParserCommon::rename_invalid_id($id);

        return ASTNode->new_func_node( ROSE_OP_READ_IMM,
            [ ASTNode->new_var_node($id) ] )
          if ( exists $ParserConfig::ARGS_PER_SUBSET{$curr_set}
            && exists $ParserConfig::ARGS_PER_SUBSET{$curr_set}{$id}
            && $ParserConfig::ARGS_PER_SUBSET{$curr_set}{$id}{need_read} );

        return ASTNode->new_func_node( ROSE_OP_READ_IMM,
            [ ASTNode->new_var_node($id) ] )
          if ( exists $ParserConfig::ARGS_PER_SUBSET{$curr_set}
            && exists $ParserConfig::ARGS_PER_SUBSET{$curr_set}{$id}
            && $ParserConfig::ARGS_PER_SUBSET{$curr_set}{$id}{need_read} );

        if ( exists $ParserConfig::GLOBAL_VARS{$id} ) {
            my $value = $ParserConfig::GLOBAL_VARS{$id}{value};
            return ASTNode->new_num_node($value);
        }

        # Default: variable
        return ASTNode->new_var_node($id);
    }

    if ( my $fld = $ast->{E_field} ) {
        my ( $var_ast, $field_ast ) = @$fld;

        my $var   = $var_ast->{E_id}[0]{Id};
        my $field = $field_ast->{Id};

        return ASTNode->new_field_node( $var, $field );
    }

    if ( my $blk = $ast->{E_block} ) {
        my ($exps_ast) = @$blk;

        my @nodes =
          map { $self->do_exp( $_, $curr_set ) }
          grep { defined $_ } @$exps_ast;

        @nodes = grep { !$_->is_null_node } @nodes;

        return ASTNode->new_block_node( \@nodes );
    }

    if ( my $assign = $ast->{E_assign} ) {
        my ( $var_ast, $exp_ast ) = @$assign;

        my $lhs = $self->do_lexp( $var_ast, $curr_set );
        my $rhs = $self->do_exp( $exp_ast, $curr_set );

        return ASTNode->new_assign_node( $lhs, $rhs );
    }

    if ( my $assert_ast = $ast->{E_assert} ) {

        # We ignore all asserts
        return ASTNode->new_null_node();
    }

    if ( my $let = $ast->{E_let} ) {
        my ( $letbind_ast, $exp_ast ) = @$let;

        my $let_node  = $self->do_letbind( $letbind_ast, $curr_set );
        my $next_node = $self->do_exp( $exp_ast, $curr_set );

        return $next_node if $let_node->is_null_node;
        return $let_node  if $next_node->is_null_node;

        $let_node->append_sibling($next_node);
        return $let_node;
    }

    if ( my $var = $ast->{E_var} ) {
        my ( $var_ast, $bind_ast, $exp_ast ) = @$var;

        my $let_node  = $self->do_varbind( $var_ast, $bind_ast, $curr_set );
        my $next_node = $self->do_exp( $exp_ast, $curr_set );

        return $next_node if $let_node->is_null_node;
        return $let_node  if $next_node->is_null_node;

        $let_node->append_sibling($next_node);
        return $let_node;
    }

    if ( my $for = $ast->{E_for} ) {
        my ( $i_ast, $init_ast, $fini_ast, $step_ast, $ord, $block_ast ) =
          @$for;

        my $i     = ASTNode->new_var_node( $i_ast->{Id} );
        my $init  = $self->do_exp( $init_ast,  $curr_set );
        my $fini  = $self->do_exp( $fini_ast,  $curr_set );
        my $step  = $self->do_exp( $step_ast,  $curr_set );
        my $block = $self->do_exp( $block_ast, $curr_set );

        return ASTNode->new_for_node( $i, $init, $fini, $step, $ord, $block );
    }

    if ( my $loop_ast = $ast->{E_loop} ) {

        my ( $loop_type, undef, $cond_ast, $block_ast ) = @$loop_ast;

        my $cond_node;

        if ( $loop_type eq "While" ) {
            $cond_node = $self->do_exp( $cond_ast, $curr_set );
        }
        elsif ( $loop_type eq "Until" ) {
            my $exp_node = $self->do_exp( $cond_ast, $curr_set );
            return ASTBuildHelper::build_boolean_not($exp_node);
        }
        else {
            die "Unexpected SAIL loop type: $loop_type";
        }

        my $block_node = $self->do_exp( $block_ast, $curr_set );

        return ASTNode->new_while_node( $cond_node, $block_node );
    }

    if ( my $app = $ast->{E_app} ) {
        my ( $id_ast, $exps_ast ) = @$app;

        # Boolean operators:  And_Bool / Or_Bool (id_ast is an array of ["And_Bool"] or ["Or_Bool"])
        if ( ref $id_ast eq "ARRAY" ) {
            my $bool_op = $id_ast->[0];

            my ( $lhs_ast, $rhs_ast ) = @$exps_ast;
            my $lhs = $self->do_exp( $lhs_ast, $curr_set );
            my $rhs = $self->do_exp( $rhs_ast, $curr_set );

            return ASTBuildHelper::build_boolean_and( $lhs, $rhs )
              if $bool_op eq "And_Bool";

            return ASTBuildHelper::build_boolean_or( $lhs, $rhs )
              if $bool_op eq "Or_Bool";

            die "Unexpected boolean operator: $bool_op";
        }

        # Normal function application (id_ast is object with {Id => "fn_id", ...})
        if ( my $fn_id = $id_ast->{Id} ) {

            for my $dispatch (@FN_DISPATCH) {
                my ( $re, $handler ) = @$dispatch;
                if ( $fn_id =~ $re ) {
                    return $handler->( $self, $exps_ast, $curr_set );
                }
            }

            # This is for instruction conversion
            if ( grep( /^$fn_id$/, @ParserConfig::supported_riscv_subsets) ) {
                return $fn_id;
            }

            for my $dispatch (@ERR_DISPATCH) {
                my ( $re, $handler ) = @$dispatch;
                if ( $fn_id =~ $re ) {
                    $handler->( $fn_id );
                    exit 1;
                }
            }

            # Unexpected functions, probably newly added extension
            die "$fn_id does not map to ROSE C++. Consider implementing it.";
            exit 1;
        }

        # Operators, mostly comparison operators
        if ( my $operator = $id_ast->{Operator} ) {

            # Map SAIL operators to ROSE operators
            my %op_map = (
                '<_s'  => ROSE_OP_SIGNED_LT,
                '<=_s' => ROSE_OP_SIGNED_LEQ,
                '>_s'  => ROSE_OP_SIGNED_GT,
                '>=_s' => ROSE_OP_SIGNED_GEQ,
                '<_u'  => ROSE_OP_UNSIGNED_LT,
                '<=_u' => ROSE_OP_UNSIGNED_LEQ,
                '>_u'  => ROSE_OP_UNSIGNED_GT,
                '>=_u' => ROSE_OP_UNSIGNED_GEQ,
            );
            if ( my $rose_op = $op_map{$operator} ) {
                my ( $lhs_ast, $rhs_ast ) = @$exps_ast;
                my $lhs_node = $self->do_exp( $lhs_ast, $curr_set );
                my $rhs_node = $self->do_exp( $rhs_ast, $curr_set );
                return ASTNode->new_func_node( $rose_op,
                    [ $lhs_node, $rhs_node ] );
            }
            else {
                die "Unknown operator in E_app: $operator";
            }
        }
    }
    elsif ( exists $ast->{E_match} ) {
        my ( $id_ast, $match_list_ast ) = @{ $ast->{E_match} };
        my $id_node = $self->do_exp( $id_ast, $curr_set );
        my @match_list_node;
        if ( exists $id_ast->{E_app} ) {
            my $app_ast = $id_ast->{E_app};
            my $func    = $app_ast->[0]->{Id};
            if ( $func eq "vmem_read" ) {

                # offset and width are already handled by capstone
                my $source_node =
                  $self->do_exp( $app_ast->[1]->[0], $curr_set );

                # In sail, memory read is done using vmem_read
                # In ROSE, it needs to be broken into two steps:
                # 1. Get effective address
                # 2. Read from the address

                # First, we look for where the read value is stored
                # look for Ok pattern and ignore Err pattern
                my @ok_asts =
                  grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq "Ok"
                  } @$match_list_ast;

                # Handle Ok(result), where result stores the value read
                my ( $ok_node, $next_node ) =
                  @{ $self->do_pexp( $ok_asts[0], $curr_set ) };
                my $read_result_node = $ok_node->get_nth_child(0);

                # Now, let's deal with the effective address
                # Dyninst combines read register and offset into the effective address
                # This is different from how SAIL handles memory read
                # SAIL use the function vmem_read(rs1, offset, width, Read(Data), al, rl, res)

                # The effective address is stored in a temporary variable in ROSE
                # Read from the effective address will be handled in the printer
                my $ea_target      = TEMP_VAR;
                my $ea_target_node = ASTNode->new_var_node($ea_target);

                my $ea_node =
                  ASTNode->new_func_node( ROSE_OP_EADDR, [$source_node] );

                # Now, we construct the assignment nodes

                my $ea_assign_node =
                  ASTNode->new_assign_node( $ea_target_node, $ea_node );
                my $mem_read_node =
                  ASTNode->new_func_node( ROSE_OP_READ_MEM, [$ea_target_node] );
                my $mem_read_assign_node =
                  ASTNode->new_assign_node( $read_result_node, $mem_read_node );
                $ea_assign_node->append_sibling($mem_read_assign_node);

                # Finally, construct the next block node in Ok(result)
                $ea_assign_node->append_sibling($next_node);

                return $ea_assign_node;
            }
            if ( $func eq "vmem_write" ) {
                my $source_node =
                  $self->do_exp( $app_ast->[1]->[0], $curr_set );

                # In sail, memory write is done using vmem_write
                # In ROSE, it needs to be broken into two steps:
                # 1. Get effective address
                # 2. Write from the address

                # The effective address is stored in a temporary variable in ROSE
                # Write to the effective address will be handled in the printer

                my $ea_target      = TEMP_VAR;
                my $ea_target_node = ASTNode->new_var_node($ea_target);

                my $ea_node =
                  ASTNode->new_func_node( ROSE_OP_EADDR, [$source_node] );
                my $ea_assign_node =
                  ASTNode->new_assign_node( $ea_target_node, $ea_node );

                # Now, we construct the write address node
                my $write_data_node =
                  $self->do_exp( $app_ast->[1]->[3], $curr_set );
                my $impl_width_bytes = ASTNode->new_var_node(WIDTH_BYTES_VAR);
                my $mem_write_node = ASTNode->new_func_node( ROSE_OP_WRITE_MEM,
                    [ $ea_target_node, $impl_width_bytes, $write_data_node ] );

                # Ignore Ok and Err pattern
                # They are related to whether the reservation failed
                # This cannot be possibly determined in ROSE

                $ea_assign_node->append_sibling($mem_write_node);
                return $ea_assign_node;
            }
            if ( $func eq "mem_read" ) {

                # offset and width are already handled by capstone
                my $addr_node = $self->do_exp( $app_ast->[1]->[1], $curr_set );

                # In sail, memory read is done using vmem_read
                # In ROSE, it needs to be broken into two steps:
                # 1. Get effective address
                # 2. Read from the address

                # First, we look for where the read value is stored
                # look for Ok pattern and ignore Err pattern
                my @ok_asts =
                  grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq "Ok"
                  } @$match_list_ast;

                my ( $ok_node, $next_node ) =
                  @{ $self->do_pexp( $ok_asts[0], $curr_set ) };
                my $read_result_node = $ok_node->get_nth_child(0);

                # Now, we construct the assignment nodes

                my $mem_read_node =
                  ASTNode->new_func_node( ROSE_OP_READ_MEM, [$addr_node] );
                my $mem_read_assign_node =
                  ASTNode->new_assign_node( $read_result_node, $mem_read_node );

                # Finally, construct the next block node in Ok(result)
                $mem_read_assign_node->append_sibling($next_node);

                return $mem_read_assign_node;
            }
            if ( $func eq "mem_write_value" ) {

                # offset and width are already handled by capstone
                my $addr_node = $self->do_exp( $app_ast->[1]->[0], $curr_set );

                # In sail, memory read is done using vmem_read
                # In ROSE, it needs to be broken into two steps:
                # 1. Get effective address
                # 2. Read from the address

                # First, we look for where the read value is stored
                # look for Ok pattern and ignore Err pattern
                my @ok_asts = grep {

                         exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq "Ok"
                      && $_->{Pat_exp}->[0]->{P_app}->[1]->[0]->{P_lit}->[0] eq
                      "L_true"
                } @$match_list_ast;

                # Now, we construct the write node

                my $write_data_node =
                  $self->do_exp( $app_ast->[1]->[2], $curr_set );
                my $impl_width_bytes = ASTNode->new_var_node(WIDTH_BYTES_VAR);
                my $mem_write_node = ASTNode->new_func_node( ROSE_OP_WRITE_MEM,
                    [ $addr_node, $impl_width_bytes, $write_data_node ] );

                # Finally, construct the next block node in Ok(result)
                my $next_ast  = $ok_asts[0]->{Pat_exp}->[1];
                my $next_node = $self->do_exp( $next_ast, $curr_set );

                $mem_write_node->append_sibling($next_node);

                return $mem_write_node;
            }
            if ( $func eq "mem_write_ea" ) {

                # Accept Ok(...)
                my @ok_asts =
                  grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq "Ok"
                  } @$match_list_ast;

                my $ok_ast    = $ok_asts[0]->{Pat_exp}->[1];
                my $next_node = $self->do_exp( $ok_ast, $curr_set );
                return $next_node;
            }
            if ( $func eq "ext_data_get_addr" ) {

                # Accept Ext_DataAddr_OK
                my @ok_asts = grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq
                      "Ext_DataAddr_OK"
                } @$match_list_ast;

                my $source_node =
                  $self->do_exp( $app_ast->[1]->[0], $curr_set );
                my $ea_node =
                  ASTNode->new_func_node( ROSE_OP_EADDR, [$source_node] );

                my $ea_target =
                  $ok_asts[0]->{Pat_exp}->[0]->{P_app}->[1]->[0]->{P_id}->[0]
                  ->{Id};
                my $ea_target_node = ASTNode->new_var_node($ea_target);
                my $ea_assign_node =
                  ASTNode->new_assign_node( $ea_target_node, $ea_node );

                my $ok_ast  = $ok_asts[0]->{Pat_exp}->[1];
                my $ok_asts = $self->do_exp( $ok_ast, $curr_set );

                $ea_assign_node->append_sibling($ok_asts);
                return $ea_assign_node;
            }

            # Ignore translateAddr
            # ROSE's effectiveAddress already handles that
            if ( $func eq "translateAddr" ) {
                my $src_id;
                if ( exists $app_ast->[1]->[0]->{E_id} ) {
                    $src_id = $app_ast->[1]->[0]->{E_id}->[0]->{Id};
                }
                elsif ( exists $app_ast->[1]->[0]->{E_app} ) {
                    $src_id =
                      $app_ast->[1]->[0]->{E_app}->[1]->[0]->{E_id}->[0]->{Id};
                }
                else {
                    die "Fatal error: unrecognized translateAddr AST";
                    exit 1;
                }
                my $src_id_node = ASTNode->new_var_node($src_id);

                # Accept Ok(...)
                my @ok_asts =
                  grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq "Ok"
                  } @$match_list_ast;

                my $ok_tuple =
                  $ok_asts[0]->{Pat_exp}->[0]->{P_app}->[1]->[0]->{P_tuple};
                my $target_id      = $ok_tuple->[0]->[0]->{P_id}->[0]->{Id};
                my $target_id_node = ASTNode->new_var_node($target_id);
                my $assign_node =
                  ASTNode->new_assign_node( $target_id_node, $src_id_node );

                my $ok_ast    = $ok_asts[0]->{Pat_exp}->[1];
                my $next_node = $self->do_exp( $ok_ast, $curr_set );
                $assign_node->append_sibling($next_node);
                return $assign_node;
            }
            # Ignore control flow check
            # Return whatever is needed to be done next
            if ( $func eq "ext_control_check_pc" ) {
                my @ok_asts = grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq
                      "Ext_ControlAddr_OK"
                } @$match_list_ast;
                my ( $ok_node, $next_node ) =
                  @{ $self->do_pexp( $ok_asts[0], $curr_set ) };
                return $next_node;
            }
            # Same, but also return the destination to jump to
            if ( $func eq "jump_to" ) {
                my @ok_asts = grep {
                    exists $_->{Pat_exp}->[0]->{P_app}
                      && $_->{Pat_exp}->[0]->{P_app}->[0]->{Id} eq
                      "Retire_Success"
                } @$match_list_ast;
                my ( $ok_node, $next_node ) =
                  @{ $self->do_pexp( $ok_asts[0], $curr_set ) };

                $id_node->append_sibling($next_node);
                return $id_node;
            }

            # Some(...) None(...) pattern
            # We only care about values in Some(...)
            my ( $match_value_node1, $match_expr_node1 ) =
              @{ $self->do_pexp( $match_list_ast->[0], $curr_set ) };
            my ( $match_value_node2, $match_expr_node2 ) =
              @{ $self->do_pexp( $match_list_ast->[1], $curr_set ) };
            if (   $match_value_node1->is_app_node()
                && $match_value_node2->is_app_node() )
            {
                # match ... None ... Some ...
                # match ... Ok ... Err ...
                if (   $match_value_node1->get_value() eq "Some"
                    && $match_value_node2->get_value() eq "None" )
                {
                    my $some_node = $match_value_node1->get_nth_child(0);
                    my $expr_node =
                      $self->do_exp( $app_ast->[1]->[0], $curr_set );
                    my $assign_node =
                      ASTNode->new_assign_node( $some_node, $expr_node );
                    $assign_node->append_sibling($match_expr_node1);
                    return $assign_node;
                }
                elsif ($match_value_node1->get_value() eq "None"
                    && $match_value_node2->get_value() eq "Some" )
                {
                    my $some_node = $match_value_node2->get_nth_child(0);
                    my $expr_node =
                      $self->do_exp( $app_ast->[1]->[0], $curr_set );
                    my $assign_node =
                      ASTNode->new_assign_node( $some_node, $expr_node );
                    $assign_node->append_sibling($match_expr_node2);
                    return $assign_node;
                }
            }
        }
        @match_list_node = map {
            my ( $key, $value ) = @{ $self->do_pexp( $_, $curr_set ) };
            ASTNode->new_match_op_node( $key, $value );
        } @$match_list_ast;
        return ASTNode->new_match_node( $id_node, \@match_list_node );
    }
    elsif ( my $iff = $ast->{E_if} ) {
        my ( $cond_ast, $if_ast, $else_ast ) = @$iff;

        my $cond_node = $self->do_exp( $cond_ast, $curr_set );
        my $if_node   = $self->do_exp( $if_ast,   $curr_set );
        my $else_node = $self->do_exp( $else_ast, $curr_set );

        # If both the if or else node is an exception
        # We return a NULL node
        if ( $if_node->is_exception_node && $else_node->is_exception_node ) {
            return ASTNode->new_null_node;
        }

        # If the if statement is an exception but the else is not
        # Or if the else statement is an exception but the if is not
        # We collapse the if-then-else
        elsif ( $if_node->is_exception_node ) {
            return $else_node;
        }
        elsif ( $else_node->is_exception_node ) {
            return $if_node;
        }

        # Special case: In SAIL, it is possible to write
        #   if (cond) jumps_to(foo)
        # In C++, this is impossible. It must write
        #   d->branchTo(ops->ite(cond, foo, PC))
        # So we need to append the default jump target PC back
        if (   $if_node->is_func_node
            && $if_node->is_op(ROSE_OP_JUMP)
            && ( $else_node->is_null_node || $else_node->is_return_node ) )
        {
            $else_node = ASTNode->new_func_node( ROSE_OP_JUMP,
                [ ASTNode->new_var_node("PC") ] );
        }

        # Special case: In SAIL, it is possible to write
        #   if (cond) X(rd) = foo
        # In C++, this is impossible. It must write
        #   d->write(ops->ite(cond, foo, d->read(rd)))
        # So we need to append the default value d->read(rd)
        if (   $if_node->is_func_node
            && $if_node->is_op(ROSE_OP_WRITE_REG)
            && ( $else_node->is_null_node || $else_node->is_return_node ) )
        {
            my $reg = $if_node->get_nth_child(0);
            $else_node = ASTNode->new_func_node( ROSE_OP_WRITE_REG,
                [ $reg, ASTNode->new_func_node( ROSE_OP_READ_REG, [$reg] ) ] );
        }

        #
        # Merge two writes to same register
        #   if (cond) X(rd) = foo else X(rd) = bar
        # becomes
        #   d->write(rd, ops->ite(cond, foo, bar))
        #
        if (   $if_node->is_func_node
            && $if_node->is_op(ASTType::ROSE_OP_WRITE_REG)
            && $else_node->is_func_node
            && $else_node->is_op(ASTType::ROSE_OP_WRITE_REG)
            && $if_node->get_nth_child(0)->is_var_node
            && $else_node->get_nth_child(0)->is_var_node
            && $if_node->get_nth_child(0)->get_value() eq
            $else_node->get_nth_child(0)->get_value() )
        {
            my $reg = $if_node->get_nth_child(0);

            my $ite = ASTNode->new_ite_node(
                $cond_node,
                $if_node->get_nth_child(0),
                $else_node->get_nth_child(0)
            );

            return ASTNode->new_func_node( ASTType::ROSE_OP_WRITE_REG,
                [ $reg, $ite ] );

        }

        # Merge two jumps
        #   if (cond) jump_to(foo) else jump_to(bar)
        # becomes
        #   d->branchTo(ops->ite(cond, foo, bar))
        if (   $if_node->is_func_node
            && $if_node->is_op(ASTType::ROSE_OP_JUMP)
            && $else_node->is_func_node
            && $else_node->is_op(ASTType::ROSE_OP_JUMP) )
        {
            my $ite = ASTNode->new_ite_node(
                $cond_node,
                $if_node->get_nth_child(0),
                $else_node->get_nth_child(0)
            );

            return ASTNode->new_func_node( ASTType::ROSE_OP_JUMP, [$ite] );

        }

        # Generic if-then-else node
        return ASTNode->new_ite_node( $cond_node, $if_node, $else_node );
    }
    return ASTNode->new_null_node();
}

1;
