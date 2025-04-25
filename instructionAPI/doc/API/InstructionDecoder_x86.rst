.. _sec:instructionDecoder:

InstructionDecoder_x86 Class
----------------------------

The class decodes instructions, given a buffer of bytes and a length,
and constructs an Instruction. The will, by default, be constructed to
decode machine language on the platform on which it has been compiled.
The buffer will be treated as if there is an instruction stream starting
at the beginning of the buffer. objects are given a buffer from which to
decode at construction. Calls to will proceed to decode instructions
sequentially from that buffer until its end is reached. At that point,
all subsequent calls to will return an invalid Instruction object. An
object may alternately be constructed without designating a buffer, and
the buffer may be specified at the time is called. This method of use
may be more convenient for users who are decoding non-contiguous
instructions.
