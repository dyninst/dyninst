
/*
 *  Code to test the KList ADT
 *  Test KList<Cstring> and KList<int> 
 *  No purify errors
 *  Present uses of functions
 *
 *  Verified output is in klist_out
 *
 *  The log for this test is in klist_out
 *
 */


/*
 * $Log: KListTest.C,v $
 * Revision 1.1  1994/08/17 18:26:11  markc
 * Code and sample output to test new classes.
 *
 */

#include <iostream.h>

#pragma implementation "klist.h"
#include "../h/klist.h"

#pragma implementation "cstring.h"
#include "../h/cstring.h"

#pragma implementation "keylist.h"
#include "../h/keylist.h"

int compare_cstring (const Cstring &a, const Cstring &b)
{
  return (a == b);
}

int add1(int &i)
{
  return (i+1);
}

Cstring addTag(Cstring &item)
{
  return(item + "happy");
}

void printTimesTwo (int &i)
{
  cout << i << " * 2 = " << (2*i) << endl;
}

int printUntilZippy(const Cstring &i1, const Cstring &i2)
{
  cout << i1 << endl;
  return (i1 == i2);
}

void test()
{
  KList<int> iList, i2List;
  KList<Cstring> cList, c2List, *pcList;

  cout << " ilist empty = " << iList.empty() << endl;
  cout << " clist empty = " << cList.empty() << endl;

  cout << "Printing iList " << iList << endl;
  cout << "Printing cList " << cList << endl;

  iList.append(4);
  iList.prepend(3);
  iList.append(5);

  cList.append("sad");
  cList.prepend("happy");
  cList.append("zippy");

  cout << "Printing iList " << iList << endl;
  cout << "Printing cList " << cList << endl;

  c2List.destroy();
  c2List.appendUnique("orange");
  c2List.prependUnique("apple");
  cout << "Printing c2List " << c2List << endl;
  
  c2List += cList;
  i2List += iList;
  cout << "These lists should be equal\n";
  cout << "Printing iList " << iList << endl;
  cout << "Printing i2List " << iList << endl;

  cout << "Printing c2List " << c2List << endl;
  c2List.appendUnique("orange");
  cout << "Printing c2List " << c2List << endl;

  pcList = new KList<Cstring>(c2List);

  cout << "Printing pcList " << *pcList << endl;
  delete (pcList);

  cList = c2List;
  cout << "Printing c2List " << c2List << endl;
  cout << "Printing cList " << cList << endl;

  cout << "Printing iList (before 3) " << iList << endl;
  iList.remove(3);
  cout << "Printing iList (removed 3) " << iList << endl;

  cout << "Printing cList (before removing apple) " << cList << endl;
  cList.remove("apple");
  cout << "Printing cList (after removing apple) " << cList << endl;

  Cstring a("orange");
  cout << "Printing cList (before removing orange) " << cList << endl;
  int fd1;
  cList.find(a, fd1, 1, &compare_cstring);
  cout << "Printing cList (after removing orange) " << cList << endl;

  iList.append(5);
  iList.append(1);
  cout << "Printing iList " << iList << endl;
  iList = iList.pure_map(&add1);
  cout << "iList after pure_map(&add1) " << iList << endl;
  iList.modify(&add1);
  cout << "iList after modify(&add1) " << iList << endl;

  cout << "cList before modify(&addTag) " << cList << endl;
  cList.modify(&addTag);
  cout << "cList after modify(&addTag) " << cList << endl;

  cout << " count of " << iList << " is " << iList.count()
    << " the list is " << (iList.empty() ? " not " : " ") 
      << " empty " << endl;
  cout << " count of " << cList << " is " << cList.count()
    << " the list is " << (cList.empty() ? " not " : " ")
      << " empty " << endl;

  cout << " iList.map(&printTimesTwo) " << endl;
  iList.map(&printTimesTwo);

  cout << " cList.mapUntil(&printUntilZippy) " << endl;
  cList.mapUntil("zippy", &printUntilZippy);

  Cstring findem("sadhappy");
  cout << " Search " << cList << " for " << findem << endl;
  int fd;
  cout << "cList.find('sadhappy') = " << cList.find(findem, fd) << endl;

  cout << "cdr (" << cList << " ) = " << cList.cdr() << endl;
  cout << "cdr (" << iList << " ) = " << iList.cdr() << endl;

  int valid=1;

  while (valid) {
    cout << " car ( " << cList << " ) = " << cList.car(valid) << endl;
  }
  valid = 1;
  while (valid) {
    cout << " car ( " << iList << " ) = " << iList.car(valid) << endl;
  }

  Cstring p1("me");
  cout << " p1 = " << p1 << endl;
  p1 = p1;
  cout << " p1 = " << p1 << endl;
  p1 = p1 + p1;
  cout << " p1 = " << p1 << endl;
  KList<int> l1, l2;
  l1.append(1);
  l1.append(2);
  l1.append(3);
  l2 = l1;
  cout << "l1 = " << l1 << endl;
  l1 = l1;
  cout << "l1 = l1 --> " << l1 << endl;
  l1 += l1;
  cout << "l1 += l1 --> " << l1 << endl;
  l1 = l2;
  cout << "l1 reset " << l1 << endl;
  l1 += l2;
  cout << "l2 = " << l2 << endl;
  cout << "l1 += l2 --> " << l1 << endl;
}

void testk()
{
  KeyList<int> ilist;
  int i1=1, i2=2, i3=3, i4=4, i5=5, i6=6, i7=7, i8=8;

  ilist.appendUnique(i1, (void*) i1);
  ilist.appendUnique(i2, (void*) i2);
  ilist.appendUnique(i3, (void*) i3);
  ilist.appendUnique(i4, (void*) i4);
  ilist.appendUnique(i5, (void*) i5);
  ilist.appendUnique(i6, (void*) i6);
  ilist.appendUnique(i7, (void*) i7);
  ilist.appendUnique(i8, (void*) i8);

  int v;
  cout << "ilist = " << ilist << endl;
  cout << " ilist.find(" << i1 << ") =" << ilist.find((void*) i1, v) << endl;
  cout << " ilist.find(" << i2 << ") =" << ilist.find((void*) i2, v) << endl;
  cout << " ilist.find(" << i3 << ") =" << ilist.find((void*) i3, v) << endl;
  cout << " ilist.find(" << i4 << ") =" << ilist.find((void*) i4, v) << endl;
  cout << " ilist.find(" << i5 << ") =" << ilist.find((void*) i5, v) << endl;
  cout << " ilist.find(" << i6 << ") =" << ilist.find((void*) i6, v) << endl;
  cout << " ilist.find(" << i7 << ") =" << ilist.find((void*) i7, v) << endl;
  cout << " ilist.find(" << i8 << ") =" << ilist.find((void*) i8, v) << endl;

  ilist.reverse();
  cout << "ilist.reverse() = " << ilist << endl;
  ilist.destroy();
  ilist.reverse();
  
  KeyList<Cstring> slist;
  slist.appendUnique("happy1", (void*) i1);
  slist.appendUnique("happy2", (void*) i2);
  slist.appendUnique("happy3", (void*) i3);
  slist.appendUnique("happy4", (void*) i4);
  cout << "slist = " << slist << endl;
  cout << "slist.find(" << i1 << ") = " << slist.find((void*) i1, v) << endl;
  slist.reverse();
  cout << "slist.reverse() = " << slist << endl;
}

void testh()
{
  KHTable<int> i1(15);
  int j1=1, j2=2, j3=3, j4=4, j5=5, j6=6, j7=7, j8=8, j56=56;

  i1.destroy();
  i1.add(j1, (void*) j1);
  i1.add(j2, (void*) j2);
  i1.add(j3, (void*) j3);
  i1.add(j4, (void*) j4);
  i1.add(j5, (void*) j5);
  i1.add(j6, (void*) j6);
  i1.add(j7, (void*) j7);
  i1.add(j8, (void*) j8);
  i1.add(j56, (void*) j56);
  
  cout << "table= " << i1 << endl;
  i1.destroy();
}


main()
{
  test();
  testk();
  testh();
}
