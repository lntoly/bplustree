#include "bplustree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

BPlusTree::BPlusTree( int nOrder/* = BPTREE_DEFAULT_ORDER*/, int nKeyLength/* = BPTREE_DEFAULT_KEY_LENGTH*/ )
{
    order = nOrder;
    if(order < BPTREE_MIN_ORDER)
        order = BPTREE_MIN_ORDER;
    if(order > BPTREE_MAX_ORDER)
        order = BPTREE_MAX_ORDER;
    key_length = nKeyLength + 1;
    if(key_length < 0)
        key_length = BPTREE_DEFAULT_KEY_LENGTH + 1;
    verbose_output = false;
    queue = NULL;
    root_node = NULL;
}

BPlusTree::~BPlusTree()
{
    DestroyBPTree();
}

node * BPlusTree::Insert( char * key, int value )
{
    root_node = Insert(root_node, key, value);
    return root_node;
}

node * BPlusTree::Delete( char * key )
{
    root_node = Delete(root_node, key);
    return root_node;
}

record * BPlusTree::Find( char * key, bool verbose )
{
    record * r = Find(root_node, key, verbose);
    return r;
}

node * BPlusTree::Insert( int key, int value )
{
    char * key_str = (char *)malloc(key_length);
    memset(key_str, 0, key_length);
    char * bak_str = (char *)malloc(key_length);
    memset(bak_str, 0, key_length);
    sprintf(bak_str, "%d", key);
    int i = 0;
    for(i = 0; i < key_length; i++)
    {
        key_str[i] = '0';
    }
    key_str[key_length-1] = '\0';
    for(i = 0; i < strlen(bak_str); i++)
    {
        key_str[key_length-2-i] = bak_str[strlen(bak_str)-1-i];
    }
    node * p = Insert(key_str, value);
    free(key_str);
    free(bak_str);
    return p;
}

node * BPlusTree::Delete( int key )
{
    char * key_str = (char *)malloc(key_length);
    memset(key_str, 0, key_length);
    char * bak_str = (char *)malloc(key_length);
    memset(bak_str, 0, key_length);
    sprintf(bak_str, "%d", key);
    int i = 0;
    for(i = 0; i < key_length; i++)
    {
        key_str[i] = '0';
    }
    key_str[key_length-1] = '\0';
    for(i = 0; i < strlen(bak_str); i++)
    {
        key_str[key_length-2-i] = bak_str[strlen(bak_str)-1-i];
    }
    node * p = Delete(key_str);
    free(key_str);
    free(bak_str);
    return p;
}

record * BPlusTree::Find( int key, bool verbose )
{
    char * key_str = (char *)malloc(key_length);
    memset(key_str, 0, key_length);
    char * bak_str = (char *)malloc(key_length);
    memset(bak_str, 0, key_length);
    sprintf(bak_str, "%d", key);
    int i = 0;
    for(i = 0; i < key_length; i++)
    {
        key_str[i] = '0';
    }
    key_str[key_length-1] = '\0';
    for(i = 0; i < strlen(bak_str); i++)
    {
        key_str[key_length-2-i] = bak_str[strlen(bak_str)-1-i];
    }
    record * p = Find(key_str, verbose);
    free(key_str);
    free(bak_str);
    return p;
}

void BPlusTree::DestroyBPTree()
{
    root_node = destroy_tree(root_node);
}

void BPlusTree::PrintBPTree()
{
    print_tree(root_node);
}

void BPlusTree::PrintBPTreeLeaves()
{
    print_leaves(root_node);
}

void BPlusTree::FindAndPrint( char * key, bool verbose )
{
    find_and_print(root_node, key, verbose);
}

void BPlusTree::FindAndPrint( int key, bool verbose )
{
    char * key_str = (char *)malloc(key_length);
    memset(key_str, 0, key_length);
    char * bak_str = (char *)malloc(key_length);
    memset(bak_str, 0, key_length);
    sprintf(bak_str, "%d", key);
    int i = 0;
    for(i = 0; i < key_length; i++)
    {
        key_str[i] = '0';
    }
    key_str[key_length-1] = '\0';
    for(i = 0; i < strlen(bak_str); i++)
    {
        key_str[key_length-2-i] = bak_str[strlen(bak_str)-1-i];
    }
    find_and_print(root_node, key_str, verbose);
    free(key_str);
    free(bak_str);
}

void BPlusTree::SetVerbose( bool verbose )
{
    verbose_output = verbose;
}

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
node * BPlusTree::Insert( node * root, char * key, int value )
{
    record * pointer;
    node * leaf;

    /* The current implementation ignores
     * duplicates.
     */
    if (Find(root, key, false) != NULL)
        return root;

    /* Create a new record for the
     * value.
     */
    pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */
    if (root == NULL) 
        return start_new_tree(key, pointer);

    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    leaf = find_leaf(root, key, false);

    /* Case: leaf has room for key and pointer.
     */
    if (leaf->num_keys < order - 1) {
        leaf = insert_into_leaf(leaf, key, pointer);
        return root;
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(root, leaf, key, pointer);
}

/* Master deletion function.
 */
node * BPlusTree::Delete(node * root, char * key)
{
    node * key_leaf;
    record * key_record;

    key_record = Find(root, key, false);
    key_leaf = find_leaf(root, key, false);
    if (key_record != NULL && key_leaf != NULL) {
        root = delete_entry(root, key_leaf, key, key_record);
        free(key_record);
    }
    return root;
}

/* Finds and returns the record to which
 * a key refers.
 */
record * BPlusTree::Find( node * root, char * key, bool verbose )
{
    int i = 0;
    node * c = find_leaf( root, key, verbose );
    if (c == NULL) return NULL;
    for (i = 0; i < c->num_keys; i++)
        if (strcmp(c->keys[i], key) == 0) break;
    if (i == c->num_keys) 
        return NULL;
    else
        return (record *)c->pointers[i];
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
void BPlusTree::enqueue( node * new_node )
{
    node * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
node * BPlusTree::dequeue( void )
{
    node * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}

/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int BPlusTree::height( node * root )
{
    int h = 0;
    node * c = root;
    while (!c->is_leaf) {
        c = (node *)(c->pointers[0]);
        h++;
    }
    return h;
}

/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int BPlusTree::path_to_root( node * root, node * child )
{
    int length = 0;
    node * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}

/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void BPlusTree::print_leaves( node * root )
{
    int i;
    node * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = (node *)(c->pointers[0]);
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            printf("%s ", c->keys[i]);
        }
        if (verbose_output)
            printf("%lx ", (unsigned long)c->pointers[order - 1]);
        if (c->pointers[order - 1] != NULL) {
            printf(" | ");
            c = (node *)(c->pointers[order - 1]);
        }
        else
            break;
    }
    printf("\n");
}

/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void BPlusTree::print_tree( node * root )
{
    node * n = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while( queue != NULL ) {
        n = dequeue();
        if (n->parent != NULL && n == n->parent->pointers[0]) {
            new_rank = path_to_root( root, n );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output) 
            printf("(%lx)", (unsigned long)n);
        for (i = 0; i < n->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)n->pointers[i]);
            printf("%s ", n->keys[i]);
        }
        if (!n->is_leaf)
            for (i = 0; i <= n->num_keys; i++)
                enqueue((node *)(n->pointers[i]));
        if (verbose_output) {
            if (n->is_leaf) 
                printf("%lx ", (unsigned long)n->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long)n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}

/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void BPlusTree::find_and_print( node * root, char * key, bool verbose )
{
    record * r = Find(root, key, verbose);
    if (r == NULL)
        printf("Record not found under key %s.\n", key);
    else 
        printf("Record at %lx -- key %s, value %d.\n",
                (unsigned long)r, key, r->value);
}

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
node * BPlusTree::find_leaf( node * root, char * key, bool verbose )
{
    int i = 0;
    node * c = root;
    if (c == NULL) {
        if (verbose) 
            printf("Empty tree.\n");
        return c;
    }
    while (!c->is_leaf) {
        if (verbose) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                printf("%s ", c->keys[i]);
            printf("%s] ", c->keys[i]);
        }
        i = 0;
        while (i < c->num_keys) {
            if (strcmp(key, c->keys[i]) >= 0) i++;
            else break;
        }
        if (verbose)
            printf("%d ->\n", i);
        c = (node *)c->pointers[i];
    }
    if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->num_keys - 1; i++)
            printf("%s ", c->keys[i]);
        printf("%s] ->\n", c->keys[i]);
    }
    return c;
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int BPlusTree::cut( int length )
{
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}

/* Creates a new record to hold the value
 * to which a key refers.
 */
record * BPlusTree::make_record(int value)
{
    record * new_record = (record *)malloc(sizeof(record));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->value = value;
    }
    return new_record;
}

/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
node * BPlusTree::make_node( void )
{
    node * new_node;
    new_node = (node *)malloc(sizeof(node));
    if (new_node == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }
    new_node->keys = (char **)malloc( (order - 1) * sizeof(char *) );
    if (new_node->keys == NULL) {
        perror("New node keys array.");
        exit(EXIT_FAILURE);
    }
    new_node->pointers = (void **)malloc( order * sizeof(void *) );
    if (new_node->pointers == NULL) {
        perror("New node pointers array.");
        exit(EXIT_FAILURE);
    }
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = NULL;
    new_node->next = NULL;
    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
node * BPlusTree::make_leaf( void )
{
    node * leaf = make_node();
    leaf->is_leaf = true;
    return leaf;
}

/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int BPlusTree::get_left_index(node * parent, node * left)
{
    int left_index = 0;
    while (left_index <= parent->num_keys && 
            parent->pointers[left_index] != left)
        left_index++;
    return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
node * BPlusTree::insert_into_leaf( node * leaf, char * key, record * pointer )
{
    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && strcmp(leaf->keys[insertion_point], key) < 0)
        insertion_point++;

    for (i = leaf->num_keys; i > insertion_point; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertion_point] = (char *)malloc(key_length);
    memset(leaf->keys[insertion_point], 0, key_length);
    strncpy(leaf->keys[insertion_point], key, key_length);
    leaf->pointers[insertion_point] = pointer;
    leaf->num_keys++;
    return leaf;
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
node * BPlusTree::insert_into_leaf_after_splitting( node * root, node * leaf, char * key, record * pointer )
{
    node * new_leaf;
    char ** temp_keys, * new_key;
    void ** temp_pointers;
    int insertion_index, split, i, j;

    new_leaf = make_leaf();

    temp_keys = (char **)malloc( order * sizeof(char *) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    temp_pointers = (void **)malloc( order * sizeof(void *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < order - 1 && strcmp(leaf->keys[insertion_index], key) < 0)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = (char *)malloc(key_length);
    memset(temp_keys[insertion_index], 0, key_length);
    strncpy(temp_keys[insertion_index], key, key_length);
    temp_pointers[insertion_index] = pointer;

    leaf->num_keys = 0;

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->pointers[i] = temp_pointers[i];
        leaf->keys[i] = temp_keys[i];
        leaf->num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->pointers[j] = temp_pointers[i];
        new_leaf->keys[j] = temp_keys[i];
        new_leaf->num_keys++;
    }

    free(temp_pointers);
    free(temp_keys);

    new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    leaf->pointers[order - 1] = new_leaf;

    for (i = leaf->num_keys; i < order - 1; i++)
        leaf->pointers[i] = NULL;
    for (i = new_leaf->num_keys; i < order - 1; i++)
        new_leaf->pointers[i] = NULL;

    new_leaf->parent = leaf->parent;
    new_key = new_leaf->keys[0];

    return insert_into_parent(root, leaf, new_key, new_leaf);
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
node * BPlusTree::insert_into_node( node * root, node * n, int left_index, char * key, node * right )
{
    int i;

    for (i = n->num_keys; i > left_index; i--) {
        n->pointers[i + 1] = n->pointers[i];
        n->keys[i] = n->keys[i - 1];
    }
    n->pointers[left_index + 1] = right;
    n->keys[left_index] = (char *)malloc(key_length);
    memset(n->keys[left_index], 0, key_length);
    strncpy(n->keys[left_index], key, key_length);
    n->num_keys++;
    return root;
}

/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
node * BPlusTree::insert_into_node_after_splitting( node * root, node * old_node, int left_index, char * key, node * right )
{
    int i, j, split;
    node * new_node, * child;
    char ** temp_keys, * k_prime;
    node ** temp_pointers;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    temp_pointers = (node **)malloc( (order + 1) * sizeof(node *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    temp_keys = (char **)malloc( order * sizeof(char *) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = (node *)(old_node->pointers[i]);
    }

    for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] = old_node->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = (char *)malloc(key_length);
    memset(temp_keys[left_index], 0, key_length);
    strncpy(temp_keys[left_index], key, key_length);

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(order);
    new_node = make_node();
    old_node->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_node->pointers[i] = temp_pointers[i];
        old_node->keys[i] = temp_keys[i];
        old_node->num_keys++;
    }
    old_node->pointers[i] = temp_pointers[i];
    k_prime = temp_keys[split - 1];
    for (++i, j = 0; i < order; i++, j++) {
        new_node->pointers[j] = temp_pointers[i];
        new_node->keys[j] = temp_keys[i];
        new_node->num_keys++;
    }
    new_node->pointers[j] = temp_pointers[i];
    free(temp_pointers);
    free(temp_keys);
    new_node->parent = old_node->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        child = (node *)(new_node->pointers[i]);
        child->parent = new_node;
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    return insert_into_parent(root, old_node, k_prime, new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
node * BPlusTree::insert_into_parent( node * root, node * left, char * key, node * right )
{
    int left_index;
    node * parent;

    parent = left->parent;

    /* Case: new root. */

    if (parent == NULL)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node. 
     */

    if (parent->num_keys < order - 1)
        return insert_into_node(root, parent, left_index, key, right);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(root, parent, left_index, key, right);
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
node * BPlusTree::insert_into_new_root( node * left, char * key, node * right )
{
    node * root = make_node();
    root->keys[0] = (char *)malloc(key_length);
    memset(root->keys[0], 0, key_length);
    strncpy(root->keys[0], key, key_length);
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}

/* First insertion:
 * start a new tree.
 */
node * BPlusTree::start_new_tree( char * key, record * pointer )
{
    node * root = make_leaf();
    root->keys[0] = (char *)malloc(key_length);
    memset(root->keys[0], 0, key_length);
    strncpy(root->keys[0], key, key_length);
    root->pointers[0] = pointer;
    root->pointers[order - 1] = NULL;
    root->parent = NULL;
    root->num_keys++;
    return root;
}

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int BPlusTree::get_neighbor_index( node * n )
{
    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}

node * BPlusTree::adjust_root( node * root )
{
    node * new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->num_keys > 0)
        return root;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (!root->is_leaf) {
        new_root = (node *)(root->pointers[0]);
        new_root->parent = NULL;
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = NULL;

    free(root->keys);
    free(root->pointers);
    free(root);

    return new_root;
}

/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
node * BPlusTree::coalesce_nodes( node * root, node * n, node * neighbor, int neighbor_index, char * k_prime )
{
    int i, j, neighbor_insertion_index, n_start, n_end;
    char * new_k_prime;
    node * tmp;
    bool split;

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = neighbor->num_keys;

    /*
     * Nonleaf nodes may sometimes need to remain split,
     * if the insertion of k_prime would cause the resulting
     * single coalesced node to exceed the limit order - 1.
     * The variable split is always false for leaf nodes
     * and only sometimes set to true for nonleaf nodes.
     */

    split = false;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * If there is room in the neighbor, append
     * all pointers and keys from the neighbor.
     * Otherwise, append only cut(order) - 2 keys and
     * cut(order) - 1 pointers.
     */

    if (!n->is_leaf) {

        /* Append k_prime.
         */

        neighbor->keys[neighbor_insertion_index] = (char *)malloc(key_length);
        memset(neighbor->keys[neighbor_insertion_index], 0, key_length);
        strncpy(neighbor->keys[neighbor_insertion_index], k_prime, key_length);
        neighbor->num_keys++;


        /* Case (default):  there is room for all of n's keys and pointers
         * in the neighbor after appending k_prime.
         */

        n_end = n->num_keys;

        /* Case (special): k cannot fit with all the other keys and pointers
         * into one coalesced node.
         */
        n_start = 0; // Only used in this special case.
        if (n->num_keys + neighbor->num_keys >= order) {
            split = true;
            n_end = cut(order) - 2;
        }

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
            n_start++;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        neighbor->pointers[i] = n->pointers[j];

        /* If the nodes are still split, remove the first key from
         * n.
         */
        if (split) {
            new_k_prime = n->keys[n_start];
            for (i = 0, j = n_start + 1; i < n->num_keys; i++, j++) {
                n->keys[i] = n->keys[j];
                n->pointers[i] = n->pointers[j];
            }
            n->pointers[i] = n->pointers[j];
            n->num_keys--;
        }

        /* All children must now point up to the same parent.
         */

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[order - 1] = n->pointers[order - 1];
    }

    if (!split) {
        root = delete_entry(root, n->parent, k_prime, n);
        free(n->keys);
        free(n->pointers);
        free(n); 
    }
    else
        for (i = 0; i < n->parent->num_keys; i++)
            if (n->parent->pointers[i + 1] == n) {
                n->parent->keys[i] = new_k_prime;
                break;
            }

    return root;
}

/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
node * BPlusTree::redistribute_nodes( node * root, node * n, node * neighbor, int neighbor_index, int k_prime_index, char * k_prime )
{
    int i;
    node * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
            strncpy(n->parent->keys[k_prime_index], n->keys[0], key_length);
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            strncpy(n->parent->keys[k_prime_index], neighbor->keys[1], key_length);
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        else
            neighbor->pointers[i] = NULL;
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}

/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
node * BPlusTree::delete_entry( node * root, node * n, char * key, void * pointer )
{
    int min_keys;
    node * neighbor;
    int neighbor_index;
    int k_prime_index;
    char * k_prime;
    int capacity;

    // Remove key and pointer from node.

    n = remove_entry_from_node(n, key, (node *)pointer);

    /* Case:  deletion from the root. 
     */

    if (n == root) 
        return adjust_root(root);


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    min_keys = n->is_leaf ? cut(order - 1) : cut(order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (n->num_keys >= min_keys)
        return root;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = n->parent->keys[k_prime_index];
    neighbor = neighbor_index == -1 ? (node *)(n->parent->pointers[1]) : 
        (node *)(n->parent->pointers[neighbor_index]);

    capacity = n->is_leaf ? order : order - 1;

    /* Coalescence. */

    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);

    /* Redistribution. */

    else
        return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
}

node * BPlusTree::remove_entry_from_node( node * n, char * key, node * pointer )
{
    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    while (strcmp(n->keys[i], key) != 0)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];


    // One key fewer.
    n->num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->is_leaf)
        for (i = n->num_keys; i < order - 1; i++)
            n->pointers[i] = NULL;
    else
        for (i = n->num_keys + 1; i < order; i++)
            n->pointers[i] = NULL;

    return n;
}

node * BPlusTree::destroy_tree( node * root )
{
    if(root == NULL)
        return NULL;
    destroy_tree_nodes(root);
    return NULL;
}

void BPlusTree::destroy_tree_nodes( node * root )
{
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++) {
            free(root->pointers[i]);
            free(root->keys[i]);
        }
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_nodes((node *)(root->pointers[i]));
    free(root->pointers);
    free(root->keys);
    free(root);
}


