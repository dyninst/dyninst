Class FuncExtent
----------------

Function Extents are used internally for accounting and lookup purposes.
They may be useful for users who wish to precisely identify the ranges
of the address space spanned by functions (functions are often
discontiguous, particularly on architectures with variable length
instruction sets).

=========== =========== ===============================
Method name Return type Method description
=========== =========== ===============================
func        Function \* Function linked to this extent.
start       Address     Start of the extent.
end         Address     End of the extent (exclusive).
=========== =========== ===============================
