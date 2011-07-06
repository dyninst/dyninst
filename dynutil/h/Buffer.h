#if !defined(_BUFFER_H_)
#define (_BUFFER_H_)

// A smart buffer that supports adding pretty much arbitrary
// units of data directly; avoids the GET_PTR/SET_PTR nonsense
// in codeGen.


namespace Dyninst {
class Buffer {
  public:
   Buffer();

   void add(char);
   void add(unsigned char);
   void add(short);
   void add(unsigned short);
   void add(int);
   void add(unsigned int);
   void add(long);
   void add(unsigned long);
   void add(char *buf, int size);

   char *ptr();
   unsigned index();
   unsigned size();
   void setIndex(unsigned);
};
};

      
