#include <stdio.h>

int findSQ(int inp) {
	int res = inp * inp;

	return res;
}

int AddTwo(int inp) {
	return inp+2;
}

int main() {
	int val = 2;

	val = findSQ(val);
	val = findSQ(val);
	val = findSQ(val);

	printf("Value %d\n", val);
	return 0;
}
