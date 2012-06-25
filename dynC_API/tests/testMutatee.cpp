// myMutatee

#include <iostream>
int hi = 0;
int zarray[0];
int array[5] = {1,2,3,4,5};
int array2[5] = {6, 7, 8, 9, 10};
int arrayField = array[2];
struct myStructType{
   int i;
   char *s;
   char *sa[4];
}mystruct = {3, "house", {"how", "now", "brown", "cow"}};
//int i = 232;
int count(int i);

int zomg = 2;

int main(){
   
  int i = 0;
  int r = 0;
  while (i < 10){
    i = count(i);
    ++hi;
    r = hi * 10;
  }
  return 0;
}

void hello(){
/*
   using std::cout;
   using std::endl;
   cout << "hello!" << endl;
*/
}

int count(int i) {
   using std::cout;
   using std::endl;
   if(i % 2 == 0){hello();}
//   cout << "The current count is " << i << endl;
   array[i % 5]++; 
   return i + 1;
}

int count(int i, char *n){
   printf(n);
   return i + 1;
}

int printfWrapper(char *s){
   printf(s);
   return 1;
}

int count(char *s){
   printf("%s\n",s);
   return 1;
}

