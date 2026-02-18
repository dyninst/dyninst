package ParserCommon;

use strict;
use warnings;

use Exporter 'import';
our @EXPORT_OK = qw(
  rename_invalid_id
  rename_op_var
  INDENT
  XLEN
  LOG2_XLEN
  TEMP_VAR
  WIDTH_BYTES_VAR
  ITE_SIGNEDNESS
);

use constant {
    INDENT          => 4,
    XLEN            => 64,
    LOG2_XLEN       => 6,
    TEMP_VAR        => "tmp",
    WIDTH_BYTES_VAR => "width",
    ITE_SIGNEDNESS  => -1,
};

sub rename_invalid_id {
    my ($id) = @_;
    $id =~ tr/'/_/;
    $id =~ tr/./_/;
    return $id;
}

sub rename_op_var {
    my ($id) = @_;
    return rename_invalid_id("rose_riscv64_op_\L$id");
}

1;
