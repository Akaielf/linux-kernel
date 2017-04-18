// copyright 2017: Kai Chen, Hsieh, NJIT ID: 31396044, for CS 610 project
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
	int value;
	int freq;
	bool is_leaf;
	struct node* left;
	struct node* right;
};

struct code {
	int value;
	int len;
};

void create_leaf_6044(int value, int freq, struct node *leaf) {
	leaf->value = value;
	leaf->freq = freq;
	leaf->is_leaf = true;
	leaf->left = NULL;
	leaf->right = NULL;
}

void create_internal_node_6044(struct node *new_node, struct node *left_child, struct node *right_child) {
	// the parent node frequency is the sum up the children nodes
	new_node->freq = left_child->freq + right_child->freq;
	new_node->is_leaf = false;
	new_node->left = left_child;
	new_node->right = right_child;
}

int insert_node_6044(struct node *new_node, struct node *heap[256], int heap_size) {
	if (heap_size >= 256) {
		printf("Stack overflow on the min heap \n");
		return heap_size;
	}

	heap[heap_size] = new_node;
	
	// might need to check overflow condition
	return heap_size + 1;
}

struct node* extract_heap_6044(struct node *heap[256], int last) {
	// varibles used for recording the index of parent, left and right child
	int parent, left, right, index;
	struct node *ret;
	struct node *min_child;
	struct node *temp;

	ret = heap[0];
	heap[0] = heap[last];

	parent = 0;
	left = 1;
	while (left <= last) {
		right = left + 1;
		if (right <= last && heap[right]->freq < heap[left]->freq) {
			// keep the min child data to swap position later
			min_child = heap[right];
			index = right;
		} else {
			min_child = heap[left];
			index = left;
		}
		// in this case we don't need to adjust the heap content
		if (heap[parent]->freq <= min_child->freq) {
			break;
		} else {
			temp = heap[parent];
			heap[parent] = min_child;
			heap[index] = temp;
			parent = index;
			left = parent * 2 + 1;
		}
	}

	return ret;
}

// adjust the heap content to main the min heap property
void adjust_heap_6044(struct node *heap[256], int last) {
	int i = last;
	struct node *temp;

	// if there is only 1 node in the heap, no need to adjust
	if (last == 0) {
		return;
	}

	// if the new node has smaller frequency than its parent, swap their positions
	while (i != 0 && heap[i]->freq < heap[(i - 1)/2]->freq) {
		temp = heap[(i - 1)/2];
		heap[(i - 1)/2] = heap[i];
		heap[i] = temp;
		i = (i - 1)/2;
	}
}

// this function is only for debug
void display_heap(struct node *heap[256], int heap_size) {
	int i;

	printf("Current heap content: \n");
	for(i = 0; i < heap_size; i++) {
		printf("%d ", heap[i]->freq);
		if (i % 5 == 0 && i != 0) {
			printf("\n");
		}
	}
	printf("\n");
}

struct node* build_huffman_tree_6044(struct node *heap[256], int heap_size) {
	struct node* left;
	struct node* right;
	struct node* new_node;

	printf("Start building the Huffman binary tree \n");
	// if the heap only has 1 node, it would not enter this loop
	while(heap_size != 1) {
		left = extract_heap_6044(heap, --heap_size);
		// debug logs
		/*printf("Pop freq: %d ", left->freq);
		if (left->is_leaf) {
			printf("value: %d", left->value);
		}
		printf("\n");*/
		right = extract_heap_6044(heap, --heap_size);
		/*printf("Pop freq: %d ", right->freq);
		if (right->is_leaf) {
			printf("value: %d", right->value);
		}
		printf("\n");*/
		new_node = (struct node*)malloc(sizeof(struct node));
		create_internal_node_6044(new_node, left, right);
		heap_size = insert_node_6044(new_node, heap, heap_size);
		adjust_heap_6044(heap, heap_size - 1);
	}

	//printf("Finish building Huffman binary tree, heap size: %d \n", heap_size);
	return heap[0];
}

// careful about the tree that has only 1 node, in this case we would encode the root as one bit '0'
void dfs_6044(struct node *curr_node, struct code code[256], int bitmap, int len) {
	// bitmap = -1 means this is the first time executing, set the initial value of bitmap with 0
	if (bitmap == -1) {
		bitmap = 0;
	}

	if (curr_node->is_leaf) {
		//printf("freq: %d (leaf) code: %d len: %d \n", curr_node->freq, bitmap, len);
		code[curr_node->value].value = bitmap;
		code[curr_node->value].len = len;
		return;
	} else {
		bitmap = bitmap << 1;
		//printf("freq: %d code: %d len: %d \n", curr_node->freq, bitmap, len);
		dfs_6044(curr_node->left, code, bitmap + 1, len + 1);
		dfs_6044(curr_node->right, code, bitmap, len + 1);
	}
}

void decode_6044(char *in_buff, long infile_size, char *outfile, long outfile_size, struct node *root) {
	int i, index, loc, value, len, remain;
	int bit_offset;
	size_t ret;
	char in_byte, out_byte;
	char *out_buff;
	FILE *in_fp;
	FILE *out_fp;
	struct node *curr_node;

	out_buff = (char*)calloc(outfile_size, sizeof(char));
	if (!out_buff) {
		printf("Out of memory, please check the file size again \n");
		exit(1);
	}

	curr_node = root;
	index = 0;
	for(i = 0; i < infile_size; i++) {
		in_byte = in_buff[i] & 0xff;
		bit_offset = 7;
		while(bit_offset >= 0) {
			// if the bit value is 1, choose left child path, else take right child path
			if(in_byte & (1 << bit_offset)) {
				curr_node = curr_node->left;
			} else {
				curr_node = curr_node->right;
			}
			if(curr_node->is_leaf) {
				out_buff[index] = curr_node->value & 0xff;
				//printf("decoded 1 symbol: %d \n", out_buff[index]);
				index++;
				if(index == outfile_size) {
					// finished decoding all the symbols
					break;
				}
				// return to the root node to decode next symbol
				curr_node = root;
			}
			bit_offset--;
		}
		if(index == outfile_size) {
			break;
		}
	}

	out_fp = fopen(outfile, "wb");
	fwrite(out_buff, sizeof(char), outfile_size, out_fp);
	fclose(out_fp);
	free(out_buff);
}

int main(int argc, char **argv) {
	int i, index, symbol_nr, val, freq;
	FILE *fp;
	long size, curr_pos, outfile_size;
	char *buff;
	char *meta_str;
	char *outfile;
	char *cmd;
	size_t ret, len;
	// record the frequency of byte values
	int count[256] = {0};
	struct code code_list[256] = {-1};
	struct node *node_list[256];
	struct node *new_node;
	struct node *root;
	int heap_size = 0;

	if (argc <= 1) {
		printf("Invalid input arguments \n");
		return 0;
	} else {
		printf("Input argument: %s \n", argv[1]);
	}	

	printf("Algorithms project Huffman coding \n");
	fp = fopen(argv[1], "rb");
	// check if the file pointer is valid
	if (!fp) {
		printf("Invalid input file \n");
		exit(1);
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	// rewind the file pointer to the begining
	rewind(fp);
	
	// allocate the space for output file name string
	outfile = (char*)malloc(sizeof(char) * (strlen(argv[1])));
	sprintf(outfile, "%s", argv[1]);
	if (strlen(outfile) > 4) {
		// cut the last 4 characters of file name (remove ".huf")
		*(outfile + sizeof(char) * (strlen(outfile) - 4)) = '\0';
	}

	// if the input file size is 0, no need to encode/decode
	if (size == 0) {
		cmd = (char*)malloc(sizeof(char) * (strlen(outfile) + 7));
		sprintf(cmd, "touch %s", outfile);
		system(cmd);
		sprintf(cmd, "rm %s", argv[1]);
		system(cmd);
		return 0;
	}
	
	meta_str = NULL;
	len = 0;
	getline(&meta_str, &len, fp);
	outfile_size = atol(strtok(meta_str, " "));
	symbol_nr = atoi(strtok(NULL, " "));
	printf("%ld bytes after decoded, %d symbols \n", outfile_size, symbol_nr);

	for(i = 0; i < symbol_nr; i++) {
		meta_str = NULL;
		len = 0;
		getline(&meta_str, &len, fp);
		val = atoi(strtok(meta_str, " "));
		freq = atoi(strtok(NULL, " "));
		//printf("symbol: %d detected, freq %d \n", val, freq);
		count[val] = freq;
	}
	
	curr_pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	// calculate the remaining size of the file
	size = ftell(fp) - curr_pos;
	fseek(fp, curr_pos, SEEK_SET);

	buff = (char*)malloc(sizeof(char) * size);
	if (!buff) {
		printf("Out of memory, please check the file size again \n");
		exit(1);
	}

	ret = fread(buff, 1, size, fp);
	if (ret != size) {
		printf("fread() error \n");
		exit(1);
	}
	printf("The input file has been successfully loaded, total %d bytes \n", ret);

	// add nodes into the heap
	for(i = 0; i < 256; i++) {
		// no need to decode symbols that never appear
		if(count[i] == 0) {
			continue;
		}
		new_node = (struct node*)malloc(sizeof(struct node));
		create_leaf_6044(i, count[i], new_node);
		heap_size = insert_node_6044(new_node, node_list, heap_size);
		adjust_heap_6044(node_list, heap_size - 1);
	}
	//display_heap(node_list, heap_size);

	root = build_huffman_tree_6044(node_list, heap_size);
	printf("Creating Huffman codes \n");
	// create Huffman codes
	dfs_6044(root, code_list, -1, 0);
	
	printf("Decoding file \n");
	decode_6044(buff, size, outfile, outfile_size, root);

	fclose(fp);
	free(buff);
	cmd = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 4));
	sprintf(cmd, "rm %s", argv[1]);
	system(cmd);

	return 0;
}
