#if !defined (local_var_h_)
#define local_var_h_

#include <vector>
#include "Symbol.h"
#include "Symtab.h"
#include "Type.h"
#include "Function.h"

#include "frame.h"
#include "procstate.h"
#include "walker.h"

using namespace Dyninst;
using namespace SymtabAPI;
using namespace Stackwalker;

class LVReader : public MemRegReader
{
 private:
   ProcessState *proc;
   int current_depth;
   std::vector<Frame> *swalk;
   Dyninst::THR_ID thrd;

   bool isFrameRegister(MachRegister reg)
   {
      return (reg == x86::EBP || reg == MachRegFrameBase);
   }

   bool isStackRegister(MachRegister reg)
   {
      return (reg == x86::ESP);
   }
 public:

   LVReader(ProcessState *p, int f, std::vector<Frame> *s, Dyninst::THR_ID t) :
      proc(p),
      current_depth(f),
      swalk(s),
      thrd(t)
   {
   }

   virtual bool ReadMem(Address addr, void *buffer, unsigned size)
   {
      return proc->readMem(buffer, addr, size);
   }


   virtual bool GetReg(MachRegister reg, MachRegisterVal &val)
   {
      Frame &f = (*swalk)[current_depth];
      if (isFrameRegister(reg)) {
         val = static_cast<MachRegisterVal>(f.getFP());
         return true;
      }
      if (isStackRegister(reg)) {
         val = static_cast<MachRegisterVal>(f.getSP());
         return true;
      }
      if (reg == MachRegReturn) {
         val = static_cast<MachRegisterVal>(f.getRA());
         return true;
      }

      if (!current_depth) {
         return proc->getRegValue(reg, thrd, val);
      }

      current_depth--;
      Frame &g = (*swalk)[current_depth];
      Offset offset;
      void *symtab_v;
      std::string lib;
      g.getLibOffset(lib, offset, symtab_v);
      Symtab *symtab = (Symtab *)(symtab_v);
      if (!symtab)
         return false;
      
      bool result = symtab->getRegValueAtFrame(offset, reg, val, this);
      current_depth++;
      return result;
   }

   virtual ~LVReader() {}
};


/**
 * Given a StackwalkerAPI frame, return the SymtabAPI function
 * that created the frame.
 **/
static Dyninst::SymtabAPI::Function *getFunctionForFrame(Frame f)
{
   Offset offset;
   void *symtab_v;
   std::string lib_name;
   bool result = f.getLibOffset(lib_name, offset, symtab_v);
   if (!result || !symtab_v)
      return NULL;
   Symtab *symtab = (Symtab *) symtab_v;
   
   Function *func;
   result = symtab->getContainingFunction(offset, func);
   if (!result)
      return NULL;
   return func;
}

/**
 * Given a frame in a stackwalk, and a local variable, get the value
 * of that local variable in the frame.
 *
 * 'localVar' is the variable that we're getting the value of.
 * 'swalk' is a stackwalk from StackwalkerAPI
 * 'frame' is an index into swalk and notes the frame that we'll be reading
 *   the variable from.  localVar should be part of the frame defined by
 *   swalk[frame]
 * 'out_buffer' is a buffer where we will write the value of the local variable.
 * out_buffer_size should be the size of out_buffer, used to prevent buffer overflows
 *
 * getLocalVariableValue will return one of the following on success or error
 **/
static int glvv_Success = 0;
static int glvv_EParam = -1;
static int glvv_EOutOfScope = -2;
static int glvv_EBufferSize = -3;
static int glvv_EUnknown = -4;

static int getLocalVariableValue(localVar *var, 
                                 std::vector<Frame> &swalk, unsigned frame,
                                 void *out_buffer, unsigned out_buffer_size)
{
   bool result;

   if (!var || frame < 0 || frame >= swalk.size() || !out_buffer) {
      return glvv_EParam;
   }

   /**
    * Find the SymtabAPI object for this frame
    * Find the offset for this frame's RA()
    **/
   std::string lib_name;
   Offset offset;
   void *symtab_v;
   swalk[frame].getLibOffset(lib_name, offset, symtab_v);
   THR_ID thrd = swalk[frame].getThread();
   ProcessState *proc = swalk[frame].getWalker()->getProcessState();

   /**
    * Find the variable location that is valid at this point.
    **/
   bool deref;
   std::vector<VariableLocation> &locs = var->getLocationLists();
   std::vector<VariableLocation>::iterator i;
   for (i = locs.begin(); i != locs.end(); i++) {
      if (i->lowPC <= offset && offset < i->hiPC) {
         break;
      }
   }
   if (i == locs.end()) {
      return glvv_EOutOfScope;
   }
   VariableLocation &loc = *i;
   
   /**
    * Interpret the variable location
    **/
   Address var_addr = 0;
   deref = (loc.refClass == storageRef);
   switch (loc.stClass) { 
      case storageAddr:
         var_addr = loc.frameOffset;
         break;
      case storageReg:
      case storageRegOffset: {
         MachRegisterVal reg_value;
         MachRegister reg = loc.reg;
         if (loc.stClass == storageRegOffset && loc.reg == -1) {
            reg = MachRegFrameBase;
         }
         
         LVReader r(proc, frame, &swalk, thrd);
         result = r.GetReg(reg, reg_value);
         
         if (loc.stClass == storageReg) {
            var_addr = reg_value;
         }
         else {
            deref = true;
            var_addr = reg_value + loc.frameOffset;
         }
         break;
      }
   }

   /** 
    * Get the size of the variable
    **/
   unsigned size = out_buffer_size;
   Type *var_type = var->getType();
   if (var_type) {
      size = var_type->getSize();
   }
   if (size > out_buffer_size) {
      return glvv_EBufferSize;
   }

   /**
    * Read the resulting value
    **/
   if (deref) {
      result = proc->readMem(out_buffer, var_addr, size);
      if (!result) {
         return glvv_EUnknown;
      }
      return glvv_Success;
   }

   if (size > sizeof(var_addr)) {
      //Value stored in register, but larger than a register?
      return glvv_EBufferSize;
   }
   memcpy(out_buffer, &var_addr, size);

   return glvv_Success;
}

#endif
