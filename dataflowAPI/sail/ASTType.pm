package ASTType;

use strict;
use warnings;

use Exporter 'import';

our @EXPORT_OK = qw(
  EXP_APP
  EXP_ASSIGN
  EXP_BLOCK
  EXP_CONVERT
  EXP_EXCEPTION
  EXP_FIELD
  EXP_FOR
  EXP_FUNC
  EXP_LIT
  EXP_MATCH
  EXP_MATCH_OP
  EXP_RETURN
  EXP_TUPLE
  EXP_VAR
  EXP_WHILE
  EXP_WILD
  LIT_BIN
  LIT_BOOL
  LIT_CONS
  LIT_HEX
  LIT_ID
  LIT_LIST
  LIT_NULL
  LIT_NUM
  LIT_STR
  LIT_STR_APPEND
  LIT_UNIT
  LIT_VAR
  LIT_VEC
  LIT_VEC_CONCAT
  LIT_VEC_INDEX
  LIT_VEC_RANGE
  ROSE_OP_ADD
  ROSE_OP_BAND
  ROSE_OP_BNOT
  ROSE_OP_BOR
  ROSE_OP_BXOR
  ROSE_OP_BOOL
  ROSE_OP_BREV8
  ROSE_OP_CLZ
  ROSE_OP_CMUL
  ROSE_OP_CMULR
  ROSE_OP_CONCAT
  ROSE_OP_CTZ
  ROSE_OP_EADDR
  ROSE_OP_EXTRACT
  ROSE_OP_GET_NEXT_PC
  ROSE_OP_GET_PC
  ROSE_OP_IDENT
  ROSE_OP_ITE
  ROSE_OP_EQ
  ROSE_OP_NEQ
  ROSE_OP_SIGNED_GT
  ROSE_OP_SIGNED_GEQ
  ROSE_OP_SIGNED_LT
  ROSE_OP_SIGNED_LEQ
  ROSE_OP_UNSIGNED_GT
  ROSE_OP_UNSIGNED_GEQ
  ROSE_OP_UNSIGNED_LT
  ROSE_OP_UNSIGNED_LEQ
  ROSE_OP_IS_ZERO
  ROSE_OP_JUMP
  ROSE_OP_LOG2
  ROSE_OP_READ_MEM
  ROSE_OP_WRITE_MEM
  ROSE_OP_WRITE_MEM_EA
  ROSE_OP_WRITE_MEM_VALUE
  ROSE_OP_NEG
  ROSE_OP_NOOP
  ROSE_OP_NUMBER
  ROSE_OP_POPC
  ROSE_OP_READ
  ROSE_OP_READ_IMM
  ROSE_OP_READ_REG
  ROSE_OP_REV8
  ROSE_OP_ROTATE_LEFT
  ROSE_OP_ROTATE_RIGHT
  ROSE_OP_SDIV
  ROSE_OP_SHIFT_LEFT
  ROSE_OP_SHIFT_RIGHT
  ROSE_OP_SHIFT_RIGHT_ARITH
  ROSE_OP_SIGN_EXTEND
  ROSE_OP_SMUL
  ROSE_OP_SREM
  ROSE_OP_SUB
  ROSE_OP_UDIV
  ROSE_OP_UREM
  ROSE_OP_WRITE_REG
  ROSE_OP_ZERO_EXTEND
);

use constant {
    LIT_NULL       => 'Null',
    LIT_VAR        => 'Variable',
    LIT_ID         => 'Identity',
    LIT_NUM        => 'Decimal Number',
    LIT_HEX        => 'Hex Number',
    LIT_BIN        => 'Binary Number',
    LIT_STR        => 'String',
    LIT_BOOL       => 'Boolean',
    LIT_UNIT       => 'Unit',
    LIT_VEC        => 'Vector',
    LIT_VEC_INDEX  => 'Vector Index',
    LIT_VEC_RANGE  => 'Vector Range',
    LIT_VEC_CONCAT => 'Vector Range',
    LIT_LIST       => 'List',
    LIT_CONS       => 'Construct',
    LIT_STR_APPEND => 'String Append'
};

use constant {
    EXP_VAR       => 'Variable',
    EXP_LIT       => 'Literal',
    EXP_FUNC      => 'Function',
    EXP_TUPLE     => 'Tuple',
    EXP_WILD      => 'Wildcard',
    EXP_BLOCK     => 'Block',
    EXP_FOR       => 'For Loop',
    EXP_WHILE     => 'While Loop',
    EXP_ASSIGN    => 'Assignment',
    EXP_MATCH     => 'Match',
    EXP_MATCH_OP  => 'Match Operation',
    EXP_APP       => 'Application',
    EXP_RETURN    => 'Return',
    EXP_FIELD     => 'Field Access',
    EXP_CONVERT   => 'Convert',
    EXP_EXCEPTION => 'Exception',
};

use constant {
    ROSE_OP_NOOP              => 'No Operation',
    ROSE_OP_IDENT             => 'Identity Function',
    ROSE_OP_SIGN_EXTEND       => 'Sign Extend',
    ROSE_OP_ZERO_EXTEND       => 'Zero Extend',
    ROSE_OP_READ_REG          => 'Read Register',
    ROSE_OP_READ_IMM          => 'Read Immediate',
    ROSE_OP_WRITE_REG         => 'Write Register',
    ROSE_OP_NUMBER            => 'To Number',
    ROSE_OP_BOOL              => 'To Boolean',
    ROSE_OP_ADD               => 'Add',
    ROSE_OP_SUB               => 'Subtract',
    ROSE_OP_SMUL              => 'Multiply',
    ROSE_OP_UMUL              => 'Unsigned Multiply',
    ROSE_OP_SUMUL             => 'Unsigned Multiply',
    ROSE_OP_SDIV              => 'Signed Divide',
    ROSE_OP_UDIV              => 'Unsigned Divide',
    ROSE_OP_SREM              => 'Signed Modulo',
    ROSE_OP_UREM              => 'Unsigned Modulo',
    ROSE_OP_NEG               => 'Negate',
    ROSE_OP_ITE               => 'If Then Else',
    ROSE_OP_EXTRACT           => 'Extract Bits',
    ROSE_OP_SHIFT_LEFT        => 'Shift Left',
    ROSE_OP_SHIFT_RIGHT       => 'Shift Right',
    ROSE_OP_SHIFT_RIGHT_ARITH => 'Shift Right Arithmetic',
    ROSE_OP_ROTATE_LEFT       => 'Rotate Left',
    ROSE_OP_ROTATE_RIGHT      => 'Rotate Right',
    ROSE_OP_BAND              => 'Bitwise And',
    ROSE_OP_BOR               => 'Bitwise Or',
    ROSE_OP_BXOR              => 'Bitwise Xor',
    ROSE_OP_BNOT              => 'Bitwise Not',
    ROSE_OP_EQ                => 'Is Equal',
    ROSE_OP_NEQ               => 'Is Not Equal',
    ROSE_OP_SIGNED_LT         => 'Is Signed Less Than',
    ROSE_OP_SIGNED_GT         => 'Is Signed Greater Than',
    ROSE_OP_SIGNED_LEQ        => 'Is Signed Less Than Equal',
    ROSE_OP_SIGNED_GEQ        => 'Is Signed Greater Than Equal',
    ROSE_OP_UNSIGNED_LT       => 'Is Unsigned Less Than',
    ROSE_OP_UNSIGNED_GT       => 'Is Unsigned Greater Than',
    ROSE_OP_UNSIGNED_LEQ      => 'Is Unsigned Less Than Equal',
    ROSE_OP_UNSIGNED_GEQ      => 'Is Unsigned Greater Than Equal',
    ROSE_OP_IS_ZERO           => 'Is Zero',
    ROSE_OP_CONCAT            => 'Concatenate',
    ROSE_OP_GET_PC            => 'Get Program Counter',
    ROSE_OP_GET_NEXT_PC       => 'Get Next Program Counter',
    ROSE_OP_JUMP              => 'Jump',
    ROSE_OP_EADDR             => 'Get Effective Address',
    ROSE_OP_READ_MEM          => 'Read Memory',
    ROSE_OP_WRITE_MEM         => 'Write Memory',
    ROSE_OP_WRITE_MEM_VALUE   => 'Write Memory Value',
    ROSE_OP_WRITE_MEM_EA      => 'Write Memory Effective Address',
    ROSE_OP_CLZ               => 'Count Leading Zero',
    ROSE_OP_CTZ               => 'Count Trailing Zero',
    ROSE_OP_CMUL              => 'Carryless Multiplication',
    ROSE_OP_CMULR             => 'Carryless Multiplication Reverse',
    ROSE_OP_POPC              => 'Popcount',
    ROSE_OP_REV8              => 'Reverse Bytes Wise',
    ROSE_OP_BREV8             => 'Reverse Bits In Bytes',
    ROSE_OP_LOG2              => 'Log2',
};

1;
