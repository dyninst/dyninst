class nopVisitor : public Visitor
{
  public:
   nopVisitor() : foundReg(false), foundImm(false), foundBin(false), isNop(true) {}
   virtual ~nopVisitor() {}
   
   bool foundReg;
   bool foundImm;
   bool foundBin;
   bool isNop;
   
   virtual void visit(BinaryFunction*)
   {
      if (foundBin) isNop = false;
      if (!foundImm) isNop = false;
      if (!foundReg) isNop = false;
      foundBin = true;
   }
   virtual void visit(Immediate *imm)
   {
      if (imm != 0) isNop = false;
      foundImm = true;
   }
   virtual void visit(RegisterAST *)
   {
      foundReg = true;
   }
   virtual void visit(Dereference *)
   {
      isNop = false;
   }
};

