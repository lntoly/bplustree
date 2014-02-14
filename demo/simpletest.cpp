#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bplustree.h"

/* First message to the user.
 */
void usage_1( void )
{
	printf("To build a B+ tree of a different order, start again and enter the order\n");
	printf("as an integer argument:  simpletest <order>  ");
	printf("(%d <= order <= %d).\n", BPTREE_MIN_ORDER, BPTREE_MAX_ORDER);
	printf("the order followed by the key length:\n"
			"simpletest <order> <key_length> .\n");
}

/* Second message to the user.
 */
void usage_2( void )
{
	printf("Enter any of the following commands after the prompt > :\n");
	printf("\ti <k>  -- Insert <k> (an integer) as both key and value).\n");
	printf("\tf <k>  -- Find the value under key <k>.\n");
	printf("\tp <k> -- Print the path from the root to key k and its associated value.\n");
	printf("\td <k>  -- Delete key <k> and its associated value.\n");
	printf("\tx -- Destroy the whole tree.  Start again with an empty tree of the same order.\n");
	printf("\tt -- Print the B+ tree.\n");
	printf("\tl -- Print the keys of the leaves (bottom row of the tree).\n");
	printf("\tv -- Toggle output of pointer addresses (\"verbose\") in tree and leaves.\n");
	printf("\tq -- Quit. (Or use Ctl-D.)\n");
	printf("\t? -- Print this help message.\n");
}

/* Brief usage note.
 */
void usage_3( void )
{
	printf("Usage: ./simpletest [<order>]\n");
	printf("\twhere %d <= order <= %d .\n", BPTREE_MIN_ORDER, BPTREE_MAX_ORDER);
}

int main(int argc, char ** argv)
{
	char instruction;
	int input;
	int order = BPTREE_DEFAULT_ORDER, key_length = BPTREE_DEFAULT_KEY_LENGTH;
	bool verbose_output = false;
	
	if (argc > 1) {
		order = atoi(argv[1]);
		if (order < BPTREE_MIN_ORDER || order > BPTREE_MAX_ORDER) {
			fprintf(stderr, "Invalid order: %d .\n\n", order);
			usage_3();
			exit(EXIT_FAILURE);
		}
	}
	
	if (argc > 2) {
		key_length = atoi(argv[1]);
	}
	
	usage_1();
	usage_2();
	
	BPlusTree bptree(order, key_length);
	
	printf("> ");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'd':
			scanf("%d", &input);
			bptree.Delete(input);
			bptree.PrintBPTree();
			break;
		case 'i':
			scanf("%d", &input);
			bptree.Insert(input, input);
			bptree.PrintBPTree();
			break;
		case 'f':
		case 'p':
			scanf("%d", &input);
			bptree.FindAndPrint(input, instruction == 'p');
			break;
		case 'l':
			bptree.PrintBPTreeLeaves();
			break;
		case 'q':
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		case 't':
			bptree.PrintBPTree();
			break;
		case 'v':
			verbose_output = !verbose_output;
			bptree.SetVerbose(verbose_output);
			break;
		case 'x':
			bptree.DestroyBPTree();
			bptree.PrintBPTree();
			break;
		default:
			usage_2();
			break;
		}
		while (getchar() != (int)'\n');
		printf("> ");
	}
	printf("\n");
	
	return EXIT_SUCCESS;
}


