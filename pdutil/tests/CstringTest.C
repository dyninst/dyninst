

/*
 * Code to test the Cstring ADT
 * Has no purify errors.
 * Presents examples of use of class Cstring
 *
 * Verified output is in cstring_out
 *
 * $Log: CstringTest.C,v $
 * Revision 1.1  1994/08/17 18:26:10  markc
 * Code and sample output to test new classes.
 *
 * 
 */ 


#pragma implementation "cstring.h"
#include "../h/cstring.h"

void test()
{
  Cstring one, two, three("happy");
  Cstring *p1, *p2, *p3;
  
  p1 = new Cstring("sad");
  one = *p1;
  p2 = new Cstring(one);
  *p2 = "p2";

  if (three == "happy")
    cout << "three == happy\n";

  if (one == (*p1))
    cout << "one == p1\n";

  if (one == (*p2))
    cout << "one == p2\n";

  if (one < (*p2))
    cout << one << " is < " << (*p2) << endl;
  else
    cout << one << " is not < " << (*p2) << endl;

  if (one < "zippy")
    cout << one << " is < " << "zippy" << endl;
  else
    cout << one << " is not < " << "zippy" << endl;

  if (one < "sade")
    cout << one << " is < " << "sade" << endl;
  else
    cout << one << " is not < " << "sade" << endl;

  if (one < "sa")
    cout << one << " is < " << "sa" << endl;
  else
    cout << one << " is not < " << "sa" << endl;

  if (one < "sad")
    cout << one << " is < " << "sad" << endl;
  else
    cout << one << " is not < " << "sad" << endl;

  if (one > (*p2))
    cout << one << " is > " << (*p2) << endl;
  else
    cout << one << " is not > " << (*p2) << endl;

  if (one > "zippy")
    cout << one << " is > " << "zippy" << endl;
  else
    cout << one << " is not > " << "zippy" << endl;

  if (one > "sade")
    cout << one << " is > " << "sade" << endl;
  else
    cout << one << " is not > " << "sade" << endl;

  if (one > "sad")
    cout << one << " is > " << "sad" << endl;
  else
    cout << one << " is not > " << "sad" << endl;  

  cout << "one is " << one << endl;
  cout << "two is " << two << endl;
  cout << "three is " << three << endl;
  cout << "p1 is " << (*p1) << endl;
  cout << "p2 is " << (*p2) << endl;

  cout << "p2 is " << p2->get() << endl;
  
  Cstring *p4 = new Cstring(1, p2->get_copy());
  cout << "p4 is " << (*p4) << endl;

  p4->use(p4->get_copy());
  cout << "p4 is " << (*p4) << endl;

  cout << one << " plus " << two << " = " << (one + two) << endl;
  cout << one << " plus " << one << " = " << (one + one) << endl;

  delete p1;
  delete p2;
  delete p4;
}

main()
{
  test();
}
