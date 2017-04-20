#include <stdlib.h>
#include <stdio.h>

int main() {
	int a = 1;
	int b = 0;

	// trigger the divide error
	printf("%d \n", a/b);

	return 0;
}
