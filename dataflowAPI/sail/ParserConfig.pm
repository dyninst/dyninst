package ParserConfig;

use strict;
use warnings;

use Tie::IxHash;

use Exporter 'import';
use lib "$FindBin::Bin";

use PrintROSE qw(
  print_indent
  print_load_init_code
  print_store_init_code
  print_mul_init_code
  print_div_init_code
  print_rem_init_code
);

our @EXPORT_OK = qw(
  $supported_riscv_subsets
  $supported_riscv_exts
  %ARGS_PER_SUBSET
  %GLOBAL_VARS
  %C_INSN_NEW_INSN
);

# Instruction subsets supported currently
our @supported_riscv_subsets = (
    "UTYPE",   "BTYPE",    "ITYPE",      "JAL",
    "JALR",    "SHIFTIOP", "RTYPE",      "LOAD",
    "STORE",   "ADDIW",    "RTYPEW",     "SHIFTIWOP",
    "LOADRES", "STORECON", "AMO",        "MUL",
    "DIV",     "REM",      "MULW",       "DIVW",
    "REMW",    "C_NOP",    "C_ADDI4SPN", "C_LW",
    "C_LD",    "C_SW",     "C_SD",       "C_ADDI",
    "C_JAL",   "C_ADDIW",  "C_LI",       "C_ADDI16SP",
    "C_LUI",   "C_SRLI",   "C_SRAI",     "C_ANDI",
    "C_SUB",   "C_XOR",    "C_OR",       "C_AND",
    "C_SUBW",  "C_ADDW",   "C_J",        "C_BEQZ",
    "C_BNEZ",  "C_SLLI",   "C_LWSP",     "C_LDSP",
    "C_SWSP",  "C_SDSP",   "C_JR",       "C_JALR",
    "C_MV",    "C_ADD",
);

# Extensions supported currently
our @supported_riscv_exts = (
    "Ext_M", "Ext_A", "Ext_F", "Ext_D", "Ext_Zca", "Ext_Zcf",
    "Ext_Zcd",
);

sub ordered_hash {
    my @kv = @_;
    my %h;
    tie %h, 'Tie::IxHash';
    %h = @kv;
    return \%h;
}

our %ARGS_PER_SUBSET = %{
    ordered_hash(

        UTYPE => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[1];\n", 2 );
                },
                need_read => 1,
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        BTYPE => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[2];\n", 2 );
                },
                need_read => 1,
            },
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[1];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[0];\n", 2 );
                },
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        ITYPE => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[2];\n", 2 );
                },
                need_read => 1,
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        JAL => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[1];\n", 2 );
                },
                need_read => 1,
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        JALR => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rd = args[0];\n", 2 );
                },
                need_read => 1,
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rd = args[0];\n", 2 );
                },
            },
        ),

        SHIFTIOP => ordered_hash(
            shamt => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[2];\n", 2 );
                },
                need_read => 1,
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        RTYPE => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        LOAD => ordered_hash(
            imm => { init_code => sub { } },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            is_unsigned => {
                init_code   => sub { print_indent( "int is_unsigned;\n", 2 ); },
                need_number => 1,
            },
            width => {
                init_code => sub {
                    print_indent( "size_t width;\n", 2 );
                    print_load_init_code(2);
                },
                need_number => 1,
            },
        ),

        STORE => ordered_hash(
            imm => { init_code => sub { } },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[0];\n", 2 );
                },
            },
            width => {
                init_code => sub {
                    print_indent( "size_t width;\n", 2 );
                    print_store_init_code(2);
                },
                need_number => 1,
            },
        ),

        LOADRES => ordered_hash(
            aq  => { init_code => sub { } },
            rl  => { init_code => sub { } },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            width => {
                init_code => sub {
                    print_indent( "size_t width = lrsc_width_str(insn)\n", 2 );
                },
                need_number => 1,
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        STORECON => ordered_hash(
            aq  => { init_code => sub { } },
            rl  => { init_code => sub { } },
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            width => {
                init_code => sub {
                    print_indent( "size_t width = lrsc_width_str(insn)\n", 2 );
                },
                need_number => 1,
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        ADDIW => ordered_hash(
            imm => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[2];\n", 2 );
                },
                need_read => 1,
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        RTYPEW => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        SHIFTIWOP => ordered_hash(
            shamt => {
                init_code => sub {
                    print_indent( "SgAsmExpression *imm = args[2];\n", 2 );
                },
                need_read => 1,
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
        ),

        AMO => ordered_hash(
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
            aq  => { init_code => sub { } },
            rl  => { init_code => sub { } },
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[1];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[2];\n", 2 );
                },
            },
            width => {
                init_code => sub {
                    print_indent( "size_t width = lrsc_width_str(insn)\n", 2 );
                },
                need_number => 1,
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        MUL => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            op => {
                init_code => sub {
                    print_indent(
                        "enum Riscv64InstructionKind op = insn->get_kind();\n",
                        2
                    );
                },
            },
            mul_op_signed_rs1 => {
                init_code =>
                  sub { print_indent( "bool mul_op_signed_rs1;\n", 2 ); },
                need_number => 1,
            },
            mul_op_signed_rs2 => {
                init_code =>
                  sub { print_indent( "bool mul_op_signed_rs2;\n", 2 ); },
                need_number => 1,
            },
            mul_op_high => {
                init_code => sub {
                    print_indent( "bool mul_op_high;\n", 2 );
                    print_mul_init_code(2);
                },
                need_number => 1,
            },
        ),

        DIV => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            is_unsigned => {
                init_code => sub {
                    print_indent( "bool is_unsigned;\n", 2 );
                    print_div_init_code(2);
                },
                need_number => 1,
            },
        ),

        REM => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            is_unsigned => {
                init_code => sub {
                    print_indent( "bool is_unsigned;\n", 2 );
                    print_rem_init_code(2);
                },
                need_number => 1,
            },
        ),

        MULW => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
        ),

        DIVW => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            is_unsigned => {
                init_code => sub {
                    print_indent( "bool is_unsigned;\n", 2 );
                    print_div_init_code(2);
                },
                need_number => 1,
            },
        ),

        REMW => ordered_hash(
            rs2 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs2 = args[2];\n", 2 );
                },
            },
            rs1 => {
                init_code => sub {
                    print_indent( "SgAsmExpression *rs1 = args[1];\n", 2 );
                },
            },
            rd => {
                init_code =>
                  sub { print_indent( "SgAsmExpression *rd = args[0];\n", 2 ); }
                ,
            },
            is_unsigned => {
                init_code => sub {
                    print_indent( "bool is_unsigned;\n", 2 );
                    print_rem_init_code(2);
                },
                need_number => 1,
            },
        ),

    )
};

our %GLOBAL_VARS = (
    xlen => {
        value => 64
    },
    log2_xlen => {
        value => 6
    },
);

# Capstone C-Extension index to Sail C-Extension index
use constant {
    CS_SAIL_IDX   => 'index',
    CS_SAIL_FIXED => 'fixed',
    REG_ZERO      => 0,
    REG_RA        => 1,
    REG_SP        => 2,
};

# Compressed instruction index to base instruction index
# -1 means that the instruction is mapped to a single instruction group
our %C_INSN_NEW_INSN = (
    C_NOP      => -1,
    C_ADDI4SPN =>  3,
    C_LW       => -1,
    C_LD       => -1,
    C_SW       => -1,
    C_SD       => -1,
    C_ADDI     =>  3,
    C_JAL      => -1,
    C_ADDIW    => -1,
    C_LI       =>  3,
    C_ADDI16SP =>  3,
    C_LUI      =>  2,
    C_SRLI     =>  3,
    C_SRAI     =>  3,
    C_ANDI     =>  3,
    C_SUB      =>  3,
    C_XOR      =>  3,
    C_OR       =>  3,
    C_AND      =>  3,
    C_SUBW     =>  3,
    C_ADDW     =>  3,
    C_J        => -1,
    C_BEQZ     =>  3,
    C_BNEZ     =>  3,
    C_SLLI     =>  3,
    C_LWSP     => -1,
    C_LDSP     => -1,
    C_SWSP     => -1,
    C_SDSP     => -1,
    C_JR       => -1,
    C_JALR     => -1,
    C_MV       =>  3,
    C_ADD      =>  3,
);

1;
