#include <stdio.h>

int findSQ(int inp) {
	int res = inp * inp;

	return res;
}

void fcn1(float par) {
	float dummy = par / 2;
}

double fcn2() {
	return 3.0;
}

int fcn3(int inp) {
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
