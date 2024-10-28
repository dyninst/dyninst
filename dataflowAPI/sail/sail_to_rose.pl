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
# Curret processing instruction subset
#
my $curr_set;

#
# We are interested in the following riscv instruction subsets
#
my $riscv_subsets = Set::Scalar->new(
    "UTYPE",
    "RISCV_JAL",
    "BTYPE",
    "ITYPE",
    "SHIFTIOP",
    "RTYPE",
    "LOAD",
    "STORE",
    "ADDIW",
    "RTYPEW",
    "SHIFTW",
    "SHIFTIWOP",
    "LOADRES",
    "STORECON",
    "AMO",
    "C_NOP",
    "C_ADDI4SPN",
    "C_LW",
    "C_LD",
    "C_SW",
    "C_SD",
    "C_ADDI",
    "C_JAL",
    "C_ADDIW",
    "C_LI",
    "C_ADDI16SP",
    "C_LUI",
    "C_SRLI",
    "C_SRAI",
    "C_ANDI",
    "C_SUB",
    "C_XOR",
    "C_OR",
    "C_AND",
    "C_SUBW",
    "C_ADDW",
    "C_J",
    "C_BEQZ",
    "C_BNEZ",
    "C_SLLI",
    "C_LWSP",
    "C_LDSP",
    "C_SWSP",
    "C_SDSP",
    "C_JR",
    "C_JALR",
    "C_MV",
    "C_ADD",
    );

#
# Map Capstone argument index to rose argument index
#
my %sail_rose_args_map = (
    "UTYPE" => {
        "rd" => 0,
        "imm" => 1,
    },
    "RISCV_JAL" => {
        "rd" => 0,
        "imm" => 1,
    },
    "RISCV_JALR" => {
        "rd" => 0,
        "rs1" => 1,
        "imm" => 2,
    },
    "BTYPE" => {
        "rs1" => 0,
        "rs2" => 1,
        "imm" => 2,
    },
    "ITYPE" => {
        "rd" => 0,
        "rs1" => 1,
        "imm" => 2,
    },
    "SHIFTIOP" => {
        "rd" => 0,
        "rs1" => 1,
        "shamt" => 2,
    },
    "RTYPE" => {
        "rd" => 0,
        "rs1" => 1,
        "rs2" => 2,
    },
    "ADDIW" => {
        "rd" => 0,
        "rs1" => 1,
        "imm" => 2,
    },
    "RTYPEW" => {
        "rd" => 0,
        "rs1" => 1,
        "rs2" => 2,
    },
    "SHIFTIWOP" => {
        "rd" => 0,
        "rs1" => 1,
        "shamt" => 2,
    },
    "LOADRES" => {
        "rd" => 0,
        "rs1" => 1,
    },
    "STORECON" => {
        "rd" => 0,
        "rs1" => 1,
        "rs2" => 2,
    },
    "AMO" => {
        "rd" => 0,
        "rs2" => 1,
        "rs1" => 2,
    },
);

#
# Capstone cext index <-> Sail cext index
# -n (n \in [0, 31]) means that it will use hardcoded register X_{n-1}
# -32 means PC
#
my %c_insn_new_arg = (
    "C_NOP" => [],
    "C_ADDI4SPN" => [0, 1, 2],
    "C_LW" => [0, 1],
    "C_LD" => [0, 1],
    "C_SW" => [0, 1],
    "C_SD" => [0, 1],
    "C_ADDI" => [0, 0, 1],
    "C_JAL" => [-2, 0],
    "C_ADDIW" => [0, 0, 1],
    "C_LI" => [0, -1, 1],
    "C_ADDI16SP" => [-3, -3, 1],
    "C_LUI" => [0, 1],
    "C_SRLI" => [0, 0, 1],
    "C_SRAI" => [0, 0, 1],
    "C_ANDI" => [0, 0, 1],
    "C_SUB" => [0, 0, 1],
    "C_XOR" => [0, 0, 1],
    "C_OR" => [0, 0, 1],
    "C_AND" => [0, 0, 1],
    "C_SUBW" => [0, 0, 1],
    "C_ADDW" => [0, 0, 1],
    "C_J" => [-1, 0],
    "C_BEQZ" => [0, -1, 1],
    "C_BNEZ" => [0, -1, 1],
    "C_SLLI" => [0, 0, 1],
    "C_LWSP" => [0, 1],
    "C_LDSP" => [0, 1],
    "C_SWSP" => [0, 1],
    "C_SDSP" => [0, 1],
    # technically, the third argument should be scalar zero.
    # However, it does not matter here because d->read accepts both scalars and registers
    "C_JR" => [-1, 0, -1],
    "C_JALR" => [-2, 0, -1],
    "C_MV" => [0, -1, 1],
    "C_ADD" => [0, 0, 1],
);

#
# cext <-> base instruction index
# -1 means that the instruction is mapped to a single instruction group
#
my %c_insn_new_insn = (
    "C_NOP" => -1,
    "C_ADDI4SPN" => 3,
    "C_LW" => -1,
    "C_LD" => -1,
    "C_SW" => -1,
    "C_SD" => -1,
    "C_ADDI" => 3,
    "C_JAL" => -1,
    "C_ADDIW" => -1,
    "C_LI" => 3,
    "C_ADDI16SP" => 3,
    "C_LUI" => 2,
    "C_SRLI" => 3,
    "C_SRAI" => 3,
    "C_ANDI" => 3,
    "C_SUB" => 3,
    "C_XOR" => 3,
    "C_OR" => 3,
    "C_AND" => 3,
    "C_SUBW" => 3,
    "C_ADDW" => 3,
    "C_J" => -1,
    "C_BEQZ" => 3,
    "C_BNEZ" => 3,
    "C_SLLI" => 3,
    "C_LWSP" => -1,
    "C_LDSP" => -1,
    "C_SWSP" => -1,
    "C_SDSP" => -1,
    "C_JR" => -1,
    "C_JALR" => -1,
    "C_MV" => 3,
    "C_ADD" => 3,
);

#
# instruction "execute" functions
#
foreach my $def (@$riscv_ast) {
    if (exists $def->{"DEF_fundef"}) {
        my $functions = $def->{"DEF_fundef"}->{"FD_function"}->[2];
        foreach my $function (@$functions) {
            my $function_list = $function->{"FCL_funcl"};
            my $function_name = $function_list->[0]->{"Id"} // "";

            # We only care about "execute" functions
            next unless $function_name eq "execute";

            my $function_exp = $function_list->[1]->{"Pat_exp"};
            my ($function_head, $function_body) = @$function_exp;

            # Parse function head
            my $function_id_name = $function_head->{"P_app"}[0]->{"Id"} // "";

            next unless $riscv_subsets->has($function_id_name);

            $curr_set = $function_id_name;

            my $function_args = $function_head->{"P_app"}[1]->[0]->{"P_tuple"}->[0];

            print "struct IP_$curr_set : P {\n";
            print "void p(D d, Ops ops, I insn, A args, B raw) {\n";

            # Generate ROSE C++ definition code
            if ($curr_set =~ /^LOAD$/) {
                print "SgAsmExpression *rd = args[0];\n";
                print "enum Riscv64InstructionKind op = insn->get_kind();\n";
                print "BaseSemantics::SValuePtr read_addr = d->effectiveAddress(args[1]);\n";
                print  <<EOF
size_t width_bytes;
int is_unsigned;
switch (insn->get_kind()) {
    case rose_riscv64_op_lb:
        width_bytes = 1;
        is_unsigned = 1;
        break;
    case rose_riscv64_op_lbu:
        width_bytes = 1;
        is_unsigned = 0;
        break;
    case rose_riscv64_op_lh:
        width_bytes = 2;
        is_unsigned = 1;
        break;
    case rose_riscv64_op_lhu:
        width_bytes = 2;
        is_unsigned = 0;
        break;
    case rose_riscv64_op_lw:
        width_bytes = 4;
        is_unsigned = 1;
        break;
    case rose_riscv64_op_lwu:
        width_bytes = 4;
        is_unsigned = 0;
        break;
    case rose_riscv64_op_ld:
        width_bytes = 8;
        is_unsigned = 1;
        break;
    default:
        assert(0 && "Invalid RISC-V instruction kind");
        break;
}
EOF
            }
            elsif ($curr_set =~ /^STORE$/) {
                print "SgAsmExpression *rs2 = args[0];\n";
                print "enum Riscv64InstructionKind op = insn->get_kind();\n";
                print "BaseSemantics::SValuePtr write_addr = d->effectiveAddress(args[1]);\n";
                print <<EOF
size_t width_bytes;
switch (insn->get_kind()) {
    case rose_riscv64_op_sb:
        width_bytes = 1;
        break;
    case rose_riscv64_op_sh:
        width_bytes = 2;
        break;
    case rose_riscv64_op_sw:
        width_bytes = 4;
        break;
    case rose_riscv64_op_sd:
        width_bytes = 8;
        break;
    default:
        assert(0 && "Invalid RISC-V instruction kind");
        break;
}
EOF
            }
            elsif ($curr_set =~ /^C_.*$/) {
                my $implicit_regs = Set::Scalar->new();
                foreach my $arg_idx (@{$c_insn_new_arg{$curr_set}}) {
                    if ($arg_idx < 0 && !$implicit_regs->has($arg_idx)) {
                        my $reg_idx = -$arg_idx - 1;
                        print "SgAsmDirectRegisterExpression dre$reg_idx\{d->findRegister(\"x$reg_idx\", 64)\};\n";
                        $implicit_regs->insert($arg_idx);
                    }
                }
                my $new_args = join(", ", map {
                    $_ < 0 ? "&dre" . (-$_ - 1) : "args[$_]"
                } @{$c_insn_new_arg{$curr_set}});
                print "SgAsmExpressionPtrList new_args{$new_args};\n";
            }
            else {
                foreach my $function_arg (@$function_args) {
                    my $arg_id = $function_arg->{"P_id"}->[0]->{"Id"};
                    my $arg_idx = $sail_rose_args_map{$curr_set}->{$arg_id};
                    print "SgAsmExpression *$arg_id = args[$arg_idx];\n" if $arg_id =~ /(imm|rs\d*|rd|shamt)/;
                }
                if ($curr_set =~ /^LOADRES|STORECON|AMO$/) {
                    print "size_t width_bytes = lrsc_width_str(insn);\n" 
                }
                if ($curr_set =~ /^LOADRES$/) {
                    print "BaseSemantics::SValuePtr read_addr = d->effectiveAddress(rs1);\n" 
                }
                if ($curr_set =~ /^STORECON$/) {
                    print "BaseSemantics::SValuePtr write_addr = d->effectiveAddress(rs1);\n" 
                }
                if ($curr_set =~ /^AMO$/) {
                    print "bool is_unsigned = amo_signed_str(insn);\n" ;
                    print "BaseSemantics::SValuePtr addr = d->effectiveAddress(rs1);\n";
                    print "BaseSemantics::SValuePtr read_addr = addr;\n";
                    print "BaseSemantics::SValuePtr write_addr = addr;\n";
                }
                print "enum Riscv64InstructionKind op = insn->get_kind();\n";
            }

            # Parse function body
            # Generate ROSE C++ code

            # Ignore NOP instructions
            if ($curr_set !~ /^C_NOP$/) {
                my $function_body_rose = do_exp($function_body);
                print $function_body_rose;
            }
            print "\n}\n};\n\n";
        }
    }
}

sub do_pat {
    my ($ast) = @_;
    if (exists $ast->{"P_typ"}) {
        # Ignore type annotation
        my (undef, $p_id) = @{$ast->{"P_typ"}};
        return $p_id->{"P_id"}->[0]->{"Id"};
    }
    elsif (exists $ast->{"P_id"}) {
        return $ast->{"P_id"}->[0]->{"Id"};
    }
    elsif (exists $ast->{"P_app"}) {
        my $app = $ast->{"P_app"};
        my $id = $app->[0]->{"Id"};
        my $arg = join(",", map {do_pat($_)} @{$app->[1]});
        return "$id($arg)";
    }
    elsif (exists $ast->{"P_tuple"}) {
        my $tuple = $ast->{"P_tuple"}->[0];
        return "[" . join(",", map {do_pat($_)} @$tuple) . "]";
    }
    elsif (exists $ast->{"P_wild"}) {
        return "...";
    }
}

sub do_pexp {
    my ($ast) = @_;
    if (exists $ast->{"Pat_exp"}) {
        my ($key_ast, $val_ast) = @{$ast->{"Pat_exp"}};
        return (do_pat($key_ast), do_exp($val_ast));
    }
}

sub do_letbind {
    my ($ast) = @_;
    my ($pat_ast, $exp_ast) = @{$ast->{"LB_val"}};
    my $id = do_pat($pat_ast);
    my $exp = do_exp($exp_ast);

    # Ignore certain redundant assignments
    if ($curr_set !~ /^C_.*$/
        && ($curr_set !~ /^LOAD|STORE$/ || ($id !~ /^width_bytes|offset$/))
        && ($curr_set !~ /STORE|AMO/ || $exp !~ /^%mem_write_ea%$/)) {
        my $rose_code = "BaseSemantics::SValuePtr $id";
        if (exists $exp_ast->{"E_match"}) {
            # the placeholder for E_match will be "<match>"
            $exp =~ s/<match>/$id/gm;
            $rose_code = "$rose_code;\n$exp";
        }
        elsif ($rose_code ne "" && $exp ne "") {
            $rose_code = "$rose_code = $exp";
        }
        else {
            $rose_code = "";
        }
        return $rose_code;
    }
    else {
        return "";
    }
}

sub do_exp {
    my ($ast) = @_;
    if (exists $ast->{"E_block"}) {
        my ($exps_ast) = @{$ast->{"E_block"}};
        foreach my $exp (@$exps_ast) {
            my $code = do_exp($exp);
        }
        return join("\n", map {
            # Ignore RETIRE_SUCCESS and RETIRE_FAIL
            my $code = do_exp($_) // "";
            $code if $code !~ /^RETIRE_(SUCCESS|FAIL)$/;
        } @$exps_ast);
    }
    elsif (exists $ast->{"E_let"}) {
        my ($letbind_ast, $exp_ast) = @{$ast->{"E_let"}};
        my $rose_code = do_letbind($letbind_ast);
        my $next_code = do_exp($exp_ast);
        return $rose_code ? ($next_code ? "$rose_code;\n$next_code" : $rose_code) : $next_code;
    }
    elsif (exists $ast->{"E_app"}) {
        my ($id_ast, $exps_ast) = @{$ast->{"E_app"}};
        if (exists $id_ast->{"Id"}) {
            my $function_name = $id_ast->{"Id"};
            if ($function_name eq "sign_extend") {
                my ($se_num_ast, $se_exp_ast) = @{$exps_ast};
                my $se_num = do_exp($se_num_ast);
                my $se_exp = rose_tr(do_exp($se_exp_ast));
                return "d->SignExtend($se_exp, $se_num)";
            }
            elsif ($function_name eq "bitvector_concat") {
                # We use shift instead of concat
                my ($concat_id_ast, $concat_bit_ast) = @{$exps_ast};
                my $concat_id = do_exp($concat_id_ast);
                my $concat_bit_hex = do_exp($concat_bit_ast);
                my $rose_code = "";
                # If the shift length is a scalar, apply ops->shiftLeft and ops->add
                if ($concat_bit_hex =~ /^0x[0-9a-fA-F]+/) {
                    $concat_bit_hex =~ s/0x//g;
                    my $concat_bit_len = length($concat_bit_hex) * 4;
                    my $concat_bit_num = sprintf("%b", $concat_bit_hex);

                    my $rose_tr_concat = rose_tr($concat_id);
                    my $rose_number_concat_bit_len = rose_number($concat_bit_len);
                    $rose_code = "ops->shiftLeft($rose_tr_concat, $rose_number_concat_bit_len)";
                    if ($concat_bit_num != 0) {
                        my $rose_number_concat_bit_num = rose_number($concat_bit_num);
                        $rose_code = "ops->add($rose_code, $rose_number_concat_bit_num)";
                    }
                }
                # Otherwise, apply regular ops->concat
                else {
                    $rose_code = "ops->concat($concat_id_ast, $concat_bit_ast)";
                }
                return $rose_code;
            }
            # Arithmetics, boolean logic operators, comparison operators, and sign/zero extensions
            elsif ($function_name eq "zero_extend") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = do_exp($exp1_ast);
                my $exp2 = do_exp($exp2_ast);
                return "d->ZeroExtend($exp2, $exp1)";
            }
            elsif ($function_name eq "sign_extend") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = do_exp($exp1_ast);
                my $exp2 = do_exp($exp2_ast);
                return "d->SignExtend($exp2, $exp1)";
            }
            elsif ($function_name eq "extend_value") {
                my ($is_unsigned_ast, $result) = @{$exps_ast};
                my $is_unsigned = do_exp($is_unsigned_ast);
                my $res = do_exp($result);
                return "($is_unsigned ? d->SignExtend($res, 64) : d->ZeroExtend($res, 64))";
            }
            elsif ($function_name eq "subrange_bits") {
                my ($exp1_ast, $exp2_ast, $exp3_ast) = @{$exps_ast};
                my $exp1 = do_exp($exp1_ast);
                my $exp2 = do_exp($exp2_ast);
                my $exp3 = do_exp($exp3_ast);
                return "ops->extract($exp1, $exp3, $exp2)";
            }
            elsif ($function_name eq "add_bits") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->add($exp1, $exp2)";
            }
            elsif ($function_name eq "sub_vec") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->subtract($exp1, $exp2)";
            }
            elsif ($function_name eq "shift_bits_left") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->shiftLeft($exp1, $exp2)";
            }
            elsif ($function_name eq "shift_bits_right") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));

                return "ops->shiftRight($exp1, $exp2)";
            }
            elsif ($function_name =~ /shift_right_arith(32|64)/) {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));

                return "ops->shiftRightArithmetic($exp1, $exp2)";
            }
            elsif ($function_name eq "and_vec") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));

                return "ops->and_($exp1, $exp2)";
            }
            elsif ($function_name eq "or_vec") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));

                return "ops->or_($exp1, $exp2)";
            }
            elsif ($function_name eq "xor_vec") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));

                return "ops->xor_($exp1, $exp2)";
            }
            elsif ($function_name eq "bool_to_bits") {
                # In C, bool is the same as int, so do nothing
                my ($exp_ast) = @{$exps_ast};
                return do_exp($exp_ast);
            }
            elsif ($function_name eq "eq_bits") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isEqual($exp1, $exp2)";
            }
            elsif ($function_name eq "neq_bits") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isNotEqual($exp1, $exp2)";
            }
            elsif ($function_name eq "neq_bits") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isNotEqual($exp1, $exp2)";
            }
            elsif ($function_name eq "add_atom") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "($exp1 + $exp2)";
            }
            elsif ($function_name eq "sub_atom") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "($exp1 - $exp2)";
            }
            elsif ($function_name eq "mult_atom") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "($exp1 * $exp2)";
            }
            # Program counter related functions
            elsif ($function_name eq "get_arch_pc") {
                return "ops->number_(64, insn->get_address())";
            }
            elsif ($function_name eq "get_next_pc") {
                return "(ops->number_(64, insn->get_address() + insn->get_size()))";
            }
            elsif ($function_name eq "set_next_pc") {
                my ($id_ast) = @{$exps_ast};
                my $target = do_exp($id_ast);
                return "d->BranchTo($target);";
            }
            # Register read/write functions
            elsif ($function_name eq "rX_bits") {
                my ($id_ast) = @{$exps_ast};
                my $id = do_exp($id_ast);
                return "d->read($id, 64)";
            }
            elsif ($function_name eq "wX_bits") {
                my ($id_ast, $exp_ast) = @{$exps_ast};
                my $id = do_exp($id_ast);
                my $exp = do_exp($exp_ast);

                my $rose_id = rose_tr($id);
                my $rose_exp = rose_tr($exp);
                return "d->write($rose_id, $rose_exp);";
            }
            # The remaining functions cannot be mapped to ROSE directly.
            # They should be handled by their parent node
            # We use  %.*% to indicate that these functions should be handled by their parent
            elsif ($function_name eq "ext_control_check_pc") {
                my ($target_ast) = @{$exps_ast};
                my $target_id = do_exp($target_ast);
                return "%ext_control_check_pc%($target_id)";
            }
            elsif ($function_name eq "ext_data_get_addr") {
                my ($target_ast) = @{$exps_ast};
                my $target_id = do_exp($target_ast);
                return "%ext_data_get_addr%($target_id)";
            }
            elsif ($function_name eq "check_misaligned") {
                my ($vaddr_ast, $width_ast) = @{$exps_ast};
                my $vaddr_id = do_exp($vaddr_ast);
                my $width_id = do_exp($width_ast);
                return "%check_misaligned%($vaddr_id,$width_id)";
            }
            elsif ($function_name eq "translateAddr") {
                my ($vaddr_ast, $data_ast) = @{$exps_ast};
                my $vaddr_id = do_exp($vaddr_ast);
                my $data_id = do_exp($data_ast);
                return "%translateAddr%($vaddr_id,$data_id)";
            }
            elsif ($function_name eq "Read") {
                my ($data_ast) = @{$exps_ast};
                my $data_id = do_exp($data_ast);
                return "%Read%($data_id);";
            }
            elsif ($function_name eq "MemValue") {
                my ($result_ast) = @{$exps_ast};
                my $result_id = do_exp($result_ast);
                return "%MemValue%($result_id);";
            }
            elsif ($function_name eq "MemException") {
                my ($e_ast) = @{$exps_ast};
                my $e_id = do_exp($e_ast);
                return "%MemException%($e_id);";
            }
            elsif ($function_name =~ /mem_(read|write_ea|write_value)/) {
                return "%$function_name%";
            }
            # For cext instructions, they are mapped to the corresponding base instruction.
            # Thus, we should hunt for their corresponding `execute` functions
            elsif ($function_name eq "execute" && $curr_set =~ /^C_.*$/) {
                my ($exp) = @{$exps_ast};
                my $conv_type = $exp->{"E_app"}->[0]->{"Id"};
                my $conv_insn_index = $c_insn_new_insn{$curr_set};
                my $conv_args_tuple = $exp->{"E_app"}->[1]->[0]->{"E_tuple"};
                my $conv_insn = $conv_insn_index != -1
                    ? $conv_args_tuple->[0]->[$conv_insn_index]->{"E_id"}->[0]->{"Id"}
                    : $conv_type;
                $conv_insn = $conv_insn =~ /^RISCV_(.*)$/ ? $1 : $conv_insn;
                if ($conv_insn =~ /^LOAD$/) {
                    my $is_unsigned = ($conv_args_tuple->[0]->[5]->{"E_lit"}->[0]) eq "L_true";
                    my $width_bytes = $conv_args_tuple->[0]->[4]->{"E_id"}->[0]->{"Id"} // "";
                    if (!$is_unsigned) {
                        if ($width_bytes eq "BYTE") {
                            $conv_insn = "LB";
                        }
                        elsif ($width_bytes eq "HALF") {
                            $conv_insn = "LH";
                        }
                        elsif ($width_bytes eq "WORD") {
                            $conv_insn = "LW";
                        }
                        elsif ($width_bytes eq "DOUBLE") {
                            $conv_insn = "LD";
                        }
                    }
                    else {
                        if ($width_bytes eq "BYTE") {
                            $conv_insn = "LBU";
                        }
                        elsif ($width_bytes eq "HALF") {
                            $conv_insn = "LHU";
                        }
                        elsif ($width_bytes eq "WORD") {
                            $conv_insn = "LWU";
                        }
                    }
                }
                elsif ($conv_insn =~ /STORE/) {
                    my $width_bytes = $conv_args_tuple->[0]->[3]->{"E_id"}->[0]->{"Id"} // "";
                    if ($width_bytes eq "BYTE") {
                        $conv_insn = "SB";
                    }
                    elsif ($width_bytes eq "HALF") {
                        $conv_insn = "SH";
                    }
                    elsif ($width_bytes eq "WORD") {
                        $conv_insn = "SW";
                    }
                    elsif ($width_bytes eq "DOUBLE") {
                        $conv_insn = "SD";
                    }
                }
                $conv_insn = lc($conv_insn) =~ tr/./_/r;
                my $orig_insn_mnemonic = "\"$conv_insn\"";
                my $orig_rose_insn = "rose_riscv64_op_$conv_insn";

                my $rose_code = "SgAsmRiscv64Instruction new_insn{0, $orig_insn_mnemonic, $orig_rose_insn};\n";
                $rose_code .= "auto conv = Riscv64::IP_$conv_type\{\};\n";
                $rose_code .= "conv.p(d, ops, &new_insn, new_args, raw);";
                return $rose_code;
            }
            elsif ($riscv_subsets->has($function_name)) {
                return $function_name;
            }
        }
        # Operators, mostly comparison operators
        elsif (exists $id_ast->{"Operator"}) {
            my $operator = $id_ast->{"Operator"};
            if ($operator eq "<_s") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isSignedLessThan($exp1, $exp2)";
            }
            elsif ($operator eq "<=_s") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isSignedLessThanOrEqual($exp1, $exp2)";
            }
            elsif ($operator eq ">_s") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isSignedGreaterThan($exp1, $exp2)";
            }
            elsif ($operator eq ">=_s") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isSignedGreaterThanOrEqual($exp1, $exp2)";
            }
            elsif ($operator eq "<_u") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isUnsignedLessThan($exp1, $exp2)";
            }
            elsif ($operator eq "<=_u") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isUnsignedLessThanOrEqual($exp1, $exp2)";
            }
            elsif ($operator eq ">_u") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isUnsignedGreaterThan($exp1, $exp2)";
            }
            elsif ($operator eq ">=_u") {
                my ($exp1_ast, $exp2_ast) = @{$exps_ast};
                my $exp1 = rose_tr(do_exp($exp1_ast));
                my $exp2 = rose_tr(do_exp($exp2_ast));
                return "ops->isUnsignedGreaterThanOrEqual($exp1, $exp2)";
            }
        }
    }
    elsif (exists $ast->{"E_match"}) {
        my ($id_ast, $match_list_ast) = @{$ast->{"E_match"}};
        my $id = do_exp($id_ast);
        if ($id eq "op") {
            my $rose_case_code = "";
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ /^RISCV_\w+$/) {
                    $key =~ s/RISCV_(\w+)/rose_riscv64_op_\L$1/l;
                    $key =~ tr/./_/;
                }
                elsif ($key =~ /^AMO\w+$/) {
                    $key =~ s/(AMO\w+)/rose_riscv64_op_\L$1/l;
                    $key =~ tr/./_/;
                }
                if ($curr_set eq "AMO") {
                    $rose_case_code .= "case ${key}_w:\n";
                    $rose_case_code .= "case ${key}_w_aq:\n";
                    $rose_case_code .= "case ${key}_w_aq_rl:\n";
                    $rose_case_code .= "case ${key}_w_rl:\n";
                    $rose_case_code .= "case ${key}_d:\n";
                    $rose_case_code .= "case ${key}_d_aq:\n";
                    $rose_case_code .= "case ${key}_d_aq_rl:\n";
                    $rose_case_code .= "case ${key}_d_rl:\n";
                    $rose_case_code .= "<match> = $val;\nbreak;\n";
                }
                else {
                    $rose_case_code = "${rose_case_code}case $key:\n";
                    $rose_case_code .= "<match> = $val;\n";
                    $rose_case_code .= "break;\n";
                }
            }
            my $default_code = "default:\n";
            $default_code .= "assert(0 && \"Invalid RISC-V instruction kind\");\n";

            my $rose_code = "switch ($id) {\n";
            $rose_code .= "$rose_case_code$default_code}";
            return $rose_code;
        }
        # Functions not handlable by do_exp
        elsif (my ($old_var) = ($id =~ /^%ext_control_check_pc%\((.*)\)$/)) {
            my $rose_case_code = "";
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);

                # Ext_ControlAddr_Error/Ext_ControlAddr_OK pair
                if ($key =~ /Ext_ControlAddr_Error\((.*)\)/) {
                    # Ignore
                }
                elsif (my ($new_var) = ($key =~ /Ext_ControlAddr_OK\((.*)\)/)) {
                    return "BaseSemantics::SValuePtr $new_var = $old_var;\n$val";
                }
            }
        }
        elsif ($id =~ /^%ext_data_get_addr%\((.*)\)$/) {
            my $rose_case_code = "";
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ "Ext_DataAddr_Error(.*)") {
                    # Ignore
                }
                elsif ($key =~ "Ext_DataAddr_OK(.*)") {
                    return $val;
                }
            }
        }
        elsif ($id =~ /^%translateAddr%(.*)$/) {
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ "TR_Failure(.*)") {
                    # Ignore
                }
                elsif ($key =~ "TR_Address(.*)") {
                    return $val;
                }
            }
        }
        elsif ($id =~ /^%mem_read%$/) {
            my $rose_code = "BaseSemantics::SValuePtr read_data = d->readMemory(read_addr, width_bytes);\n";
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ "MemException(.*)") {
                    # Ignore
                }
                elsif (my ($id) = ($key =~ "MemValue\((.*)\)")) {
                    $id =~ s/\(|\)//g;
                    return "$rose_code\nBaseSemantics::SValuePtr $id = read_data;\n$val";
                }
            }
            return $rose_code;
        }
        elsif ($id =~ /^eares$/) {
            foreach my $match_ast (@$match_list_ast) {
                my ($key, $val) = do_pexp($match_ast);
                if ($key =~ "MemException(.*)") {
                    # Ignore
                }
                elsif ($key =~ "MemValue(.*)") {
                    return $val;
                }
            }
        }
        elsif ($id =~ /%mem_write_value%/) {
            return "d->writeMemory(write_addr, width_bytes, rs2_val);";
        }
    }
    elsif (exists $ast->{"E_if"}) {
        my ($cond_ast, $if_ast, $then_ast) = @{$ast->{"E_if"}};
        my $cond_code = do_exp($cond_ast);
        my $if_code = do_exp($if_ast);
        my $then_code = do_exp($then_ast);
        # If "if then" are used to handle error checks, we ignore it. Otherwise, we handle it
        # FIXME: This is a hacky way to deal with if-then-else. Need improvement

        # Jump instructions uses the variable "taken"
        if ($cond_code =~ /^taken$/) {
            # ops->ite is extremely difficult to write
            # I just parse the string result of $if and $then to meet the format
            # I don't know if there is a better way of doing it
            my (undef, $if) = ($if_code =~ /^(.*);\n(.*)$/m);
            return "BaseSemantics::SValuePtr target = ops->ite(taken, t, PC);\n$if";
        }
        # Branch instructions uses the variable "loaded"
        elsif ($if_code =~ /^loaded$/ || $then_code =~ /^loaded$/) {
            return "ops->ite($cond_code, $if_code, $then_code)";
        }
        else {
            return $then_code;
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

# Translate immediate and registers to d->read(...)

sub rose_tr {
    my ($id) = @_;
    if ($id =~ /^(imm|rs\d*|shamt)$/) {
        return "d->read($id, 64)";
    }
    return $id;
}

# Translate scalar to ops->number_(...)

sub rose_number {
    my ($id) = @_;
    return "ops->number_(64, $id)";
}
