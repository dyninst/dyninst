// Two functions that share a loop: loop_entrant jumps into the loop of
// loop_owner, so the loop's head block belongs to both functions. Hand-written
// assembly does this, for example OpenSSL's libcrypto.
//
// In loop_owner's flow graph the head block has an incoming edge whose source
// block is only in loop_entrant. No instrumentation point can be made for that
// edge, since a point's edge must have both ends in the point's function.

extern "C" int loop_owner(int);
extern "C" int loop_entrant(int);

asm(R"(
  .text
  .globl loop_owner
  .type loop_owner, @function
loop_owner:
  xorl %eax, %eax
.Lloop_head:
  addl $1, %eax
  cmpl %edi, %eax
  jl .Lloop_head
  ret
  .size loop_owner, .-loop_owner

  .globl loop_entrant
  .type loop_entrant, @function
loop_entrant:
  movl $1, %eax
  jmp .Lloop_head
  .size loop_entrant, .-loop_entrant
)");

int main(int argc, char **) { return loop_owner(argc) + loop_entrant(argc); }
