
/*
 *
 * Code to test functions from libUtil & class List from list.h
 *        
 * This code has been purified
 *
 * Verified output is in util_out
 *
 * $Log: utilTest.C,v $
 * Revision 1.1  1994/08/17 18:26:16  markc
 * Code and sample output to test new classes.
 *
 *
 */
#include <string.h>
#include "../h/rpcUtil.h"

#pragma implementation "list.h"
#include "../h/list.h"

#pragma implementation "cstring.h"
#include "../h/cstring.h"

void dump_res(char **res)
{
  int i=0;

  cout << "DUMP STRING:";
  while (res[i]) {
    cout << "%s,", res[i];
    delete [] res[i];
    i++;
  }
  cout << endl;
  delete [] res;
}

void test_list()
{
  List<int> ilist;
  List<char*> slist;
  char *s1 = strdup("happy");
  char *s2 = strdup("sad");

  cout << " ilist = " << ilist << endl;
  cout << " ilist is " << (ilist.empty() ? " " : " NOT ") << " empty " << endl;
  cout << " slist = " << slist << endl;
  cout << " slist is " << (slist.empty() ? " " : " NOT ") << " empty " << endl;

  ilist.add(1);
  ilist.add(2);
  ilist.add(3);
  
  slist.add(s1);
  slist.add(s2);
  
  cout << " ilist = " << ilist << endl;
  cout << " ilist is " << (ilist.empty() ? " " : " NOT ") << " empty " << endl;
  cout << " slist = " << slist << endl;
  cout << " slist is " << (slist.empty() ? " " : " NOT ") << " empty " << endl;

  ilist.removeAll();

  List<char*> walk;
  char *temp;
  for (walk=slist; temp = *walk; walk++)
    delete [] temp;
  slist.removeAll();

  HTable<char*> hashEm;
  HTable<Cstring> hashC;

  Cstring c1("c1"), c2("c2"), c3("c4"), c4("a1"), c5("a2"), c6("c3");

  char *a1= strdup("a1");
  char *a3= strdup("a3");
  char *a2= strdup("a2");
  char *b3= strdup("b3");
  char *aa= strdup("aa");
  char *ba= strdup("ba");

  hashEm.add(a1, (void*) a1);
  hashEm.add(a2, (void*) a2);
  hashEm.add(a3, (void*) a3);
  hashEm.add(b3, (void*) b3);
  hashEm.add(aa, (void*) aa);
  hashEm.add(ba, (void*) ba);

  cout << "hashEm = " << hashEm << endl;

  hashC.add(c1, (void*) c1.get());
  hashC.add(c2, (void*) c2.get());
  hashC.add(c3, (void*) c3.get());
  hashC.add(c4, (void*) c4.get());
  hashC.add(c5, (void*) c5.get());
  hashC.add(c6, (void*) c6.get());

  cout << "hashC = " << hashC << endl;

  hashC.destroy();

  HTable<char *> wht;

  char *tm;
  for (wht=hashEm; tm=*wht; wht++)
    delete [] tm;
  hashEm.destroy();
}


main()
{
  int ct;
  char **answer;
  char *a1; 
  char *a2; 
  char *a3; 
  char *a4; 
  char *a5; 
  char *a6; 
  char *a7; 
  char *a8; 
  char *a9; 
  char *a10;
  char *a11; 
  char *a12;
  a1 = strdup("           ");
  a2 = strdup("   mark");
  a3 = strdup("mark  ");
  a4 = strdup("mark ");
  a5 = strdup("   mark ");
  a6 = strdup(" mark mark");
  a7 = strdup(" mark  mark");
  a8 = strdup(" mark mark ");
  a9 = strdup(" mark  mark  ");
  a10 = strdup("mark mark ");
  a11 = strdup("  mark1  mark2 mark3   mark4");
  a12 = strdup("  mark1  mark2 mark3    mark4 mark5    mark6 mark7 mark8     mark9           mark10    ");
   

  cout << "String is: " << a1 << endl;
  answer = RPCgetArg(ct, a1);
  dump_res(answer);

  cout << "String is: " << a2 << endl;
  answer = RPCgetArg(ct, a2);
  dump_res(answer);

  cout << "String is: " << a3 << endl;
  answer = RPCgetArg(ct, a3);
  dump_res(answer);

  cout << "String is: " << a4 << endl;
  answer = RPCgetArg(ct, a4);
  dump_res(answer);

  cout << "String is: " << a5 << endl;
  answer = RPCgetArg(ct, a5);
  dump_res(answer);

  cout << "String is: " << a6 << endl;
  answer = RPCgetArg(ct, a6);
  dump_res(answer);

  cout << "String is: " << a7 << endl;
  answer = RPCgetArg(ct, a7);
  dump_res(answer);

  cout << "String is: " << a8 << endl;
  answer = RPCgetArg(ct, a8);
  dump_res(answer);

  cout << "String is: " << a9 << endl;
  answer = RPCgetArg(ct, a9);
  dump_res(answer);

  cout << "String is: " << a10 << endl;
  answer = RPCgetArg(ct, a10);
  dump_res(answer);

  cout << "String is: " << a11 << endl;
  answer = RPCgetArg(ct, a11);
  dump_res(answer);

  cout << "String is: " << a12 << endl;
  answer = RPCgetArg(ct, a12);
  dump_res(answer);

  delete [] a1;
  delete [] a2;
  delete [] a3;
  delete [] a4;
  delete [] a5;
  delete [] a6;
  delete [] a7;
  delete [] a8;
  delete [] a9;
  delete [] a10;
  delete [] a11;
  delete [] a12;

  test_list();
}
