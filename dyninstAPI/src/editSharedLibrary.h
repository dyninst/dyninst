/* $Id: editSharedLibrary.h,v 1.1 2005/03/18 04:34:57 chadd Exp $ */
#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf.h>
#include  <stdlib.h>
#include  <string.h>
#include <unistd.h>

class editSharedLibrary {

	private:

		Elf *oldElf;
		int fd;

	public:

		bool removeBSSfromSharedLibrary(char* libName);

	private:
		bool	removeBSS();


};
// vim:ts=5:
