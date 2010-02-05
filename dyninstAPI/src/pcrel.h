#if !defined(_PCREL_H_)
#define _PCREL_H_

class pcRelRegion {
 public:
   friend class codeGen;
   codeGen *gen;
   instruction orig_instruc;
   unsigned cur_offset;
   unsigned cur_size;
   pcRelRegion(const instruction &i);
   virtual unsigned apply(Address addr) = 0;
   virtual unsigned maxSize() = 0;
   virtual bool canPreApply();
   virtual ~pcRelRegion();
}; 


class pcRelJump : public pcRelRegion {
private:
   Address addr_targ;
   patchTarget *targ;
    bool copy_prefixes_;

   Address get_target();
public:
   pcRelJump(patchTarget *t, const instruction &i, bool copyPrefixes = true);
   pcRelJump(Address target, const instruction &i, bool copyPrefixes = true);
   virtual unsigned apply(Address addr);
   virtual unsigned maxSize();        
   virtual bool canPreApply();
   virtual ~pcRelJump();
};

class pcRelJCC : public pcRelRegion {
private:
   Address addr_targ;
   patchTarget *targ;

   Address get_target();
public:
   pcRelJCC(patchTarget *t, const instruction &i);
   pcRelJCC(Address target, const instruction &i);
   virtual unsigned apply(Address addr);
   virtual unsigned maxSize();        
   virtual bool canPreApply();
   virtual ~pcRelJCC();
};

class pcRelCall: public pcRelRegion {
private:
   Address targ_addr;
   patchTarget *targ;

   Address get_target();
public:
   pcRelCall(patchTarget *t, const instruction &i);
   pcRelCall(Address targ_addr, const instruction &i);

   virtual unsigned apply(Address addr);
   virtual unsigned maxSize();        
   virtual bool canPreApply();
   ~pcRelCall();
};

class pcRelData : public pcRelRegion {
private:
   Address data_addr;
public:
   pcRelData(Address a, const instruction &i);
   virtual unsigned apply(Address addr);
   virtual unsigned maxSize();        
   virtual bool canPreApply();
};

#endif
