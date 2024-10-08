#!/usr/bin/env perl

use strict;
use warnings;

use JSON;
use File::Slurp;
use Set::Scalar;
use Data::Dumper;
use Carp::Assert;

my $riscv_ast_json_file = "riscv_ast.json";
my $riscv_ast_json_str = read_file($riscv_ast_json_file);
my $riscv_ast_json = decode_json($riscv_ast_json_str);
my $riscv_ast = $riscv_ast_json->{"ast"};

#
# We are interested in the following riscv instruction subsets
#

my $riscv_subsets = Set::Scalar->new("UTYPE");

#
# We are insterested in
# 1. instruction "encdec" mappings
# 2. instruction "execute" functions
#

#
# instruction "encdec" mappings
#
my %sail_rose_args_map;
foreach my $def (@$riscv_ast) {
    next unless exists $def->{"DEF_mapdef"};

    my $mapping_def = $def->{"DEF_mapdef"}->{"MD_mapping"};
    my $mapping_id = $mapping_def->[0]->{"Id"};
    # We only care about "encdec" mappings
    
    next unless defined $mapping_id && $mapping_id eq "encdec";

    my $mappings = $mapping_def->[2];
    foreach my $mapping (@$mappings) {
        my $mapping_app = $mapping->{"MCL_bidir"}->[0]->{"MPat_pat"}->[0]->{"MP_app"};
        my $mapping_id = $mapping_app->[0]->{"Id"};

        next unless defined $mapping_id && $riscv_subsets->has($mapping_id);

        my $mapping_args = $mapping_app->[1];

        die if !defined $mapping_args;

        my %index_arg_hash;
        for (my $i = 0; $i < scalar(@$mapping_args); $i++) {
            my $mp_id = $mapping_args->[$i]->{"MP_id"};
            if (defined $mp_id) {
                my $id = $mp_id->[0]->{"Id"};
                $index_arg_hash{$id} = $i;
            }
        }

        @sail_rose_args_map{keys %index_arg_hash} = values %index_arg_hash;
    }
}

#
# Map Capstone argument index to rose argument index
#
%sail_rose_args_map = (
    "UTYPE" => {
        "imm" => 0,
        "rd" => 1,
        "op" => 2,
    },
);

#
# instruction "execute" functions
#
foreach my $def (@$riscv_ast) {
    if (exists $def->{"DEF_fundef"}) {
        my $functions = $def->{"DEF_fundef"}->{"FD_function"}->[2];
        foreach my $function (@$functions) {
            my $function_list = $function->{"FCL_funcl"};
            my $function_name = $function_list->[0]->{"Id"};

            # We only care about "execute" functions
            next unless (defined $function_name && $function_name eq "execute");

            my $function_exp = $function_list->[1]->{"Pat_exp"};
            my $function_head = $function_exp->[0];
            my $function_body = $function_exp->[1];

            # Parse function head
            my $function_id_name = $function_head->{"P_app"}[0]->{"Id"};

            next unless (defined $function_id_name && $riscv_subsets->has($function_id_name));

            my $function_args = $function_head->{"P_app"}[1]->[0]->{"P_tuple"}->[0];

            # Generate ROSE C++ definition code
            foreach my $function_arg (@$function_args) {
                my $arg_id = $function_arg->{"P_id"}->[0]->{"Id"};
                my $arg_idx = $sail_rose_args_map{$function_id_name}->{$arg_id};
                if ($arg_id =~ /(imm|rs\d*|)/) {
                    print "SgAsmExpression *$arg_id = args[$arg_idx]\n";
                }
            }
            print "enum Riscv64InstructionKind op = insn->get_kind();\n";

            # Parse function body
            # Generate ROSE C++ code
            do_exp($function_body);
        }
    }
}

sub do_pat {
    my ($ast) = @_;
    if (exists $ast->{"P_typ"}) {
        # ignore type annotation
        my $p_typ = $ast->{"P_typ"};
        my (undef, $p_id) = @{$p_typ};
        my $id = $p_id->{"P_id"}->[0]->{"Id"};
        return $id;
    }
    elsif (exists $ast->{"P_id"}) {
        my $id = $ast->{"P_id"}->[0]->{"Id"};
        return $id;
    }
}

sub do_pexp {
    my ($ast) = @_;
    if (exists $ast->{"Pat_exp"}) {
        my ($key_ast, $val_ast) = @{$ast->{"Pat_exp"}};
        my $key = do_pat($key_ast);
        my $val = do_exp($val_ast);
        return ($key, $val);
    }
}

sub do_letbind {
    my ($ast) = @_;
    my ($pat_ast, $exp_ast) = @{$ast->{"LB_val"}};
    my $id = do_pat($pat_ast);
    my $exp = do_exp($exp_ast);

    my $rose_code = "BaseSemantics::SValuePtr $id";
    if (exists $exp_ast->{"E_match"}) {
        # the placeholder for E_match will be "<match>"
        $exp =~ s/<match>/$id/gm;
        $rose_code = "$rose_code;\n$exp";
    }
    else {
        $rose_code = "$rose_code = $exp";
    }
}

sub do_exp {
    my ($ast) = @_;
    if (exists $ast->{"E_block"}) {
        my ($exps_ast) = @{$ast->{"E_block"}};
        foreach my $exp (@$exps_ast) {
            my $last_code = do_exp($exp);
            print $last_code if $last_code !~ /RETIRE_(SUCCESS|FAIL)/;
        }
    }
    elsif (exists $ast->{"E_let"}) {
        my ($letbind_ast, $exp_ast) = @{$ast->{"E_let"}};
        my $rose_code = do_letbind($letbind_ast);
        print "$rose_code;\n";
        my $next_code = do_exp($exp_ast);
        return $next_code;
    }
    elsif (exists $ast->{"E_app"}) {
        my ($id_ast, $exps_ast) = @{$ast->{"E_app"}};
        my $function_name = $id_ast->{"Id"};
        if ($function_name eq "sign_extend") {
            my ($se_num_ast, $se_exp_ast) = @{$exps_ast};
            my $se_num = do_exp($se_num_ast);
            my $se_exp = do_exp($se_exp_ast);
            die "se_num undefined" if !defined $se_num;
            die "se_exp undefined" if !defined $se_exp;
            return "d->SignExtend($se_exp, $se_num)";
        }
        elsif ($function_name eq "bitvector_concat") {
            # We use shift instead of concat
            my ($concat_id_ast, $concat_bit_ast) = @{$exps_ast};
            my $concat_id = do_exp($concat_id_ast);
            my $concat_bit_hex = do_exp($concat_bit_ast);
            $concat_bit_hex =~ s/0x//g;

            my $concat_bit_len = length($concat_bit_hex) * 4;
            my $concat_bit_num = sprintf("%b", $concat_bit_hex);

            my $rose_tr_concat = rose_tr($concat_id);
            my $rose_number_concat_bit_len = rose_number($concat_bit_len);
            my $rose_code = "ops->shiftLeft($rose_tr_concat, $rose_number_concat_bit_len)";
            if ($concat_bit_num != 0) {
                my $rose_number_concat_bit_num = rose_number($concat_bit_num);
                $rose_code = "ops->add($rose_code, $rose_number_concat_bit_num)";
            }
            return $rose_code;
        }
        elsif ($function_name eq "add_bits") {
            my ($exp1_ast, $exp2_ast) = @{$exps_ast};
            my $exp1 = do_exp($exp1_ast);
            my $exp2 = do_exp($exp2_ast);

            my $rose_exp1 = rose_tr($exp1);
            my $rose_exp2 = rose_tr($exp2);
            return "ops->add($exp1, $exp2)";
        }
        elsif ($function_name eq "get_arch_pc") {
            return "ops->number_(64, insn->get_address())";
        }
        elsif ($function_name eq "wX_bits") {
            my ($id_ast, $exp_ast) = @{$exps_ast};
            my $id = do_exp($id_ast);
            my $exp = do_exp($exp_ast);

            my $rose_id = rose_tr($id);
            my $rose_exp = rose_tr($exp);
            return "d->write($rose_id, $rose_exp)";
        }
    }
    elsif (exists $ast->{"E_match"}) {
        my ($id_ast, $match_list_ast) = @{$ast->{"E_match"}};
        my $id = do_exp($id_ast);
        if ($id eq "op") {
            my $rose_case_code = "";
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ /RISCV_\w+/) {
                    $key =~ s/RISCV_(\w+)/rose_roscv64_op_\L$1/l;
                    $key =~ tr/./_/;
                }
                $rose_case_code = "${rose_case_code}case $key:\n<match> = $val;\n";
            }
            return "switch ($id) {\n$rose_case_code}";
        }
        else {
            # TODO switch statement other than op
        }
    }
    elsif (exists $ast->{"E_id"}) {
        my ($id_ast) = @{$ast->{"E_id"}};
        return $id_ast->{"Id"};
    }
    elsif (exists $ast->{"E_lit"}) {
        my ($lit_ast) = @{$ast->{"E_lit"}};
        if (exists $lit_ast->{"L_num"}) {
            return $lit_ast->{"L_num"};
        }
        elsif (exists $lit_ast->{"L_hex"}) {
            return "0x" . $lit_ast->{"L_hex"};
        }
    }
}

sub rose_tr {
    my ($id) = @_;
    if ($id =~ /^(imm|rs|rs\d*)$/) {
        return "d->read($id, 64)";
    }
    return $id;
}

sub rose_number {
    my ($id) = @_;
    return "ops->number_(64, $id)";
}
