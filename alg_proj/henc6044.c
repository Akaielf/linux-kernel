// copyright 2017, Kai Chen, Hsieh, NJIT 31396044
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
	for (i = 0; i < heap_size; i++) {
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
	while (heap_size != 1) {
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

void encode_6044(char *buff, long size, char *outfile, long outfile_size, struct code code_list[256], int count[256], int symbol_nr) {
	int i, index, loc, value, len, remain;
	int bit_offset;
	char out_byte;
	char *meta_str;
	char *out_buff;
	FILE *fp;

	bit_offset = 0;
	// allocate output buffer and initialize with zeros
	out_buff = (char*)calloc(outfile_size, sizeof(char));
	if(!out_buff) {
		printf("Out of memory \n");
		exit(1);
	}
	for (i = 0; i < size; i++) {
		index = (int)(*(buff + i) & 0xFF);
		//printf("encoding one character: %d \n", index);
		value = code_list[index].value;
		len = code_list[index].len;
		//printf("code: %d, len: %d \n", value, len);
		//printf("%d content: %d code: %d len: %d \n", i, index, value, len);
		while (len > 0) {
			// assume the shortest code length is at least 1
			loc = (bit_offset + len - 1) / 8;
			remain = (bit_offset + len) % 8;
			//printf("remain = %d \n", remain);
			if (remain != 0) {
				out_byte = (value << (8 - remain)) & 0xff;
				value = value >> remain;
				len = len - remain;
			} else {
				out_byte = value & 0xff;
				value = value >> 8;
				len = len - 8;
			}
			out_buff[loc] = out_buff[loc] | out_byte;
		}
		bit_offset = bit_offset + code_list[index].len;
		//printf("bit_offset = %d \n", bit_offset);
	}

	fp = fopen(outfile, "wb");
	meta_str = (char*)malloc(sizeof(char) * 32);
	// the symbol_nr indicate how many following lines for recording symbols and frequency 
	sprintf(meta_str, "%ld %d\n", size, symbol_nr);
	fwrite(meta_str, sizeof(char), strlen(meta_str), fp);	
	// can do better here
	for (i = 0; i < 256; i++) {
		if (count[i] > 0) {
			meta_str = (char*)malloc(sizeof(char) * 32);
			sprintf(meta_str, "%d %d\n", i, count[i]);
			fwrite(meta_str, sizeof(char), strlen(meta_str), fp);
		}
	}
	fwrite(out_buff, sizeof(char), outfile_size, fp);
	fclose(fp);
	free(out_buff);
}

int main(int argc, char **argv) {
	int i, index;
	FILE *fp;
	long size, outfile_size;
	char *buff;
	char *outfile;
	char *cmd;
	size_t ret;
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
	outfile = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 5));
	sprintf(outfile, "%s.huf", argv[1]);

	// if the input file size is 0, no need to encode/decode
	if (size == 0) {
		cmd = (char*)malloc(sizeof(char) * (strlen(outfile) + 7));
		sprintf(cmd, "touch %s", outfile);
		system(cmd);
		sprintf(cmd, "rm %s", argv[1]);
		system(cmd);
		return 0;
	}

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

	for (i=0; i < size; i++) {
		index = (int)(*(buff + i) & 0xFF);
		if (index >= 0 && index <= 255) {
			count[index]++;
		} else {
			printf("Invalid byte value: %d \n", index);
			return 0;
		}	
	}

	// add nodes into the heap
	for (i = 0; i < 256; i++) {
		// no need to encode those symbols that never appear
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
	int sum = 0;
	for (i = 0; i < 256; i++) {
		sum = sum + code_list[i].len * count[i];
	}
	
	printf("Encoded bytes: %d \n", sum / 8);
	outfile_size = sum / 8;
	// extra byte to contain the remaining information that smaller than 1 byte
	if (sum % 8 != 0) {
		outfile_size = outfile_size + 1;
	}
	printf("Encoded file size: %d \n", outfile_size);
	encode_6044(buff, size, outfile, outfile_size, code_list, count, heap_size);

	fclose(fp);
	free(buff);
	cmd = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 4));
	sprintf(cmd, "rm %s", argv[1]);
	system(cmd);

	return 0;
}
