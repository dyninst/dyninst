#include <stdlib.h>
#include <stdio.h>

#include "symtabAPI/h/Dyn_Symtab.h"

int main()
{
	string s = "/lib/tls/libc.so.6";
	bool flag;
	int i;
	Dyn_Symtab *ps = new Dyn_Symtab();
	for(i=0;i<30;i++)
	{
		bool err = ps->openFile(s,ps);
		cout << i << endl;
	}	
	Dyn_Section ret;
	assert((ps->findSection(".text", ret))==true);
	assert(ret.getSecName() == ".text");
	assert((ps->findSection(".data", ret))==true);
	assert(ret.getSecName() == ".data");
	delete ps;
}
