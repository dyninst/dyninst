// Dummy Function
int call35_2() {
}  

// call35_1 is the template for which the platform specific call35 
// functions used in test35 are based. call35_1_sparc_solaris.s, 
// call35_1_x86_solaris.s,  and call35_1_x86_linux.s were all compiled
// versions of this function, where they each have instructions added to 
// force relocation of the function.
int call35_1() {
  int localVariable35_1 = 1;
  int localVariable35_2 = 1; 
  int total35_1 = 0;
  int total35_2 = 2; 

  call35_2();

  total35_1 = localVariable35_1 * localVariable35_2;

  call35_2();

  call35_2();

  return total35_2;
}

