#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline char* sub_str(char* dest, char* src, int s_idx, int e_idx) {
	dest = (char*) malloc(sizeof(char) * (e_idx - s_idx + 1));
	strncpy(dest, (src + sizeof(char) * s_idx), (e_idx - s_idx + 1));
	return dest;
}

static inline char* asm_sub_str(char* dest, char* src, int s_idx, int e_idx);

int main(int argc, char** argv) {
	int ret = -1;
	int s_idx = 1;
	int e_idx = 3;
	char* dest;
	char* dest_2;

	if (strlen(argv[1]) < (e_idx + 1)) {
		printf("invalid input string length (must be at least %d characters) \n", e_idx + 1);
		return ret;
	}

	dest = sub_str(dest, argv[1], s_idx, e_idx);
	printf("original str: %s, s_idx: %d, e_idx: %d \n", argv[1], s_idx, e_idx);
	printf("substring from C function: %s \n", dest);

	dest_2 = (char*) malloc(sizeof(char) * e_idx);
	dest_2 = asm_sub_str(dest_2, argv[1], s_idx, e_idx);
	printf("substring from asm function: %s \n", dest_2);
	ret = strcmp(dest, dest_2);
	if (ret == 0) {
		printf("the 2 substrings are the same \n");
	} else {
		printf("the 2 substrings are different \n");
	} 
	return ret;
}

char* asm_sub_str(char* dest, char* src, int s_idx, int e_idx) {
	int tsrc, tdst;
	char* src_2;
	
	src_2 = src + sizeof(char) * s_idx;
	asm volatile(
		"1: \tlodsb\n\t"
		"stosb\n\t"
		"testb %%al, %%al\n\t"
		"jne 1b"
		: "=&S" (tsrc), "=&D" (tdst)
		: "0" (src_2), "1" (dest)
		: "memory"
	);

	dest[(e_idx - s_idx) + 1] = '\0';
	return dest;
}
