// Copyright (c) 2013 Peking University.
// Author: Xiaosong Rong (rongxiaosong@gmail.com)
//
/*
 * Change of version 2.0:
 * void print_tree(node *root):
 *      not need the pointer next
 *      use a queue to handle printint by level
 * support Bulk Loading:
 *      node *bulk_load(char **keys, int *values, int n);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_KEY_LEN 15
#define MAX_NUM_VALUE 50

// each key is a string pointer
// num_keys if the number of keys, number of pointers = num_keys + 1
typedef struct node {
    void **pointers;
    char **keys;
    struct node *parent;
    int num_keys;
    bool is_leaf;
} node;

typedef struct record {
    int value;
} record;

typedef struct queue {
    int capacity;
    int front;
    int rear;
    int size;
    node **items;
} queue;

int size = 5;  // number of pointers of each node

void print_tree(node *root);
record *find(node *root, char *key);
node *find_leaf(node *root, char *key);

// Insertion
record *make_new_record(int value);
node *make_new_node();
node *make_new_leaf();
node *make_new_tree(char *key, int value);
node *make_new_root(node *left, node *right, char *key);
node *insert(node *root, char *key, int value);
node *insert_into_parent(node *root, node *left, node *right, char *key);
void insert_into_node(node *nd, node *right, int index, char *key);
node *insert_into_node_after_splitting(node *root, node *nd, node *right,
                                       int index, char *key);
node *insert_into_leaf_after_splitting(node *root, node *leaf, int index,
                                       char *key, record *rec);
void insert_into_leaf(node *leaf, int index, char *key, record *rec);

// Deletion
void destroy_node(node *nd);
void destroy_tree(node *root);
void *remove_entry(node *nd, int index);
node *delete(node *root, char *key);
node *delete_entry(node *root, node *nd, int index);
node *adjust_root(node *root);
int get_node_index(node *nd);
node *coalesce_nodes(node *root, node *nd, node *neighbor, int nd_index);
void distribute_nodes(node *nd, node *neighbor, int nd_index);

// Bulk Loading -- build B+-tree after sorting
node *bulk_load(char **keys, int *values, int n);
int cmp(const void *, const void *);

queue *init_queue(int capacity)
{
    queue *q;
    q = malloc(sizeof(queue));
    q->items = malloc(sizeof(node *) * capacity);
    q->capacity = capacity;
    q->front = q->rear = 0;
    q->size = 0;
    return q;
}

void enqueue(queue *q, node *nd)
{
    if (q->size == q->capacity) {
        fprintf(stderr, "the queue is full !!\n");
        return ;
    }
    q->items[q->rear] = nd;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

node *dequeue(queue *q)
{
    node *nd;
    if (q->size == 0) {
        fprintf(stderr, "the queue is empty !!\n");
        return ;
    }
    nd = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return nd;
}

int get_level(node *root, node *nd)
{
    int level = 0;
    while (nd != root) {
        nd = nd->parent;
        level++;
    }
    return level;
}

void print_tree(node *root)
{
    queue *q;
    node *nd;
    int level, new_level, i;
    if (root == NULL) {
        printf("Empty tree !\n");
        return ;
    }
    q = init_queue(MAX_NUM_VALUE);
    enqueue(q, root);
    level = 0;
    while (q->size > 0) {
        nd = dequeue(q);
        new_level = get_level(root, nd);
        if (new_level > level) {
            printf("\n");
            level = new_level;
        }
        for (i = 0; i < nd->num_keys; i++)
            printf("%s ", nd->keys[i]);
        printf("| ");
        if (!nd->is_leaf) {
            for (i = 0; i <= nd->num_keys; i++)
                enqueue(q, nd->pointers[i]);
        }
    }
    printf("\n");
    free(q->items);
    free(q);
}

record *find(node *root, char *key)
{
    node *leaf;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return NULL;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)
        return NULL;
    return (record *)leaf->pointers[i];
}

node *find_leaf(node *root, char *key)
{
    node *nd;
    int i;
    if (root == NULL)
        return root;
    nd = root;
    while (!nd->is_leaf){
        for (i = 0; i < nd->num_keys && strcmp(nd->keys[i], key) <= 0; i++)
            ;
        nd = (node *)nd->pointers[i];
    }
    return nd;
}

record *make_new_record(int value)
{
    record *rec;
    rec = (record *)malloc(sizeof(record));
    rec->value = value;
    return rec;
}

node *make_new_node()
{
    node *nd;
    nd = (node *)malloc(sizeof(node));
    nd->pointers = malloc(size * sizeof(void *));
    nd->keys = malloc((size - 1) * sizeof(char *));
    nd->parent = NULL;
    nd->num_keys = 0;
    nd->is_leaf = false;
    return nd;
}

node *make_new_leaf()
{
    node *leaf;
    leaf = make_new_node();
    leaf->is_leaf = true;
    return leaf;
}

void destroy_node(node *nd)
{
    free(nd->keys);
    free(nd->pointers);
    free(nd);
}

node *make_new_tree(char *key, int value)
{
    node *root;
    record *rec;
    root = make_new_leaf();
    rec = make_new_record(value);
    root->pointers[0] = rec;
    root->keys[0] = malloc(MAX_KEY_LEN);
    strcpy(root->keys[0], key);
    root->pointers[size-1] = NULL;
    root->num_keys++;
    return root;
}

void destroy_tree(node *root)
{
    int i;
    if (!root->is_leaf) {
        for (i = 0; i < root->num_keys; i++) {
            free(root->keys[i]);
            destroy_tree(root->pointers[i]);
        }
        destroy_tree(root->pointers[i]);
    }
    else {
        for (i = 0; i < root->num_keys; i++) {
            free(root->keys[i]);
            free(root->pointers[i]); // free record
        }
    }
    destroy_node(root);
}

node *make_new_root(node *left, node *right, char *key)
{
    node *root;
    root = make_new_node();
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->keys[0] = malloc(MAX_KEY_LEN);
    strcpy(root->keys[0], key);
    root->num_keys++;
    left->parent = root;
    right->parent = root;
    return root;
}

node *insert(node *root, char *key, int value)
{
    record *rec;
    node *leaf;
    int index, cond;
    leaf = find_leaf(root, key);
    if (!leaf){  // cannot find the leaf, the tree is empty
        return make_new_tree(key, value);
    }
    for (index = 0; index < leaf->num_keys && (cond = strcmp(leaf->keys[index], key)) < 0; index++)
        ;
    if (cond == 0)  // ignore duplicates
        return root;
    rec = make_new_record(value);
    if (leaf->num_keys < size - 1){
        insert_into_leaf(leaf, index, key, rec);
        return root;  // the root remains unchanged
    }
    return insert_into_leaf_after_splitting(root, leaf, index, key, rec);
}

node *insert_into_parent(node *root, node *left, node *right, char *key)
{
    node *parent;
    int index, i;
    parent = left->parent;

    if (parent == NULL){
        return make_new_root(left, right, key);
    }

    for (index = 0; index < parent->num_keys && parent->pointers[index] != left; index++);
        ;
    if (parent->num_keys < size - 1){
        insert_into_node(parent, right, index, key);
        return root;  // the root remains unchanged
    }
    return insert_into_node_after_splitting(root, parent, right, index, key);
}

void insert_into_node(node *nd, node *right, int index, char *key)
{
    int i;
    for (i = nd->num_keys; i > index; i--){
        nd->keys[i] = nd->keys[i-1];
        nd->pointers[i+1] = nd->pointers[i];
    }
    nd->keys[index] = malloc(MAX_KEY_LEN);
    strcpy(nd->keys[index], key);
    nd->pointers[index+1] = right;
    nd->num_keys++;
}

node *insert_into_node_after_splitting(node *root, node *nd, node *right, int index, char *key)
{
    int i, split;
    node **temp_ps, *new_nd, *child;
    char **temp_ks, *new_key;
    temp_ps = malloc((size + 1) * sizeof(node *));
    temp_ks = malloc(size * sizeof(char *));

    for (i = 0; i < size + 1; i++){
        if (i == index + 1)
            temp_ps[i] = right;
        else if (i < index + 1)
            temp_ps[i] = nd->pointers[i];
        else
            temp_ps[i] = nd->pointers[i-1];
    }
    for (i = 0; i < size; i++){
        if (i == index){
            temp_ks[i] = malloc(MAX_KEY_LEN);
            strcpy(temp_ks[i], key);
        }
        else if (i < index)
            temp_ks[i] = nd->keys[i];
        else
            temp_ks[i] = nd->keys[i-1];
    }


    split = size % 2 ? size / 2 + 1 : size / 2;  // split is #pointers
    nd->num_keys = split - 1;
    for (i = 0; i < split - 1; i++){
        nd->pointers[i] = temp_ps[i];
        nd->keys[i] = temp_ks[i];
    }
    nd->pointers[i] = temp_ps[i];  // i == split - 1
    new_key = temp_ks[split - 1];

    new_nd = make_new_node();
    new_nd->num_keys = size - split;
    for (++i; i < size; i++){
        new_nd->pointers[i - split] = temp_ps[i];
        new_nd->keys[i - split] = temp_ks[i];
    }
    new_nd->pointers[i - split] = temp_ps[i];
    new_nd->parent = nd->parent;
    for (i = 0; i <= new_nd->num_keys; i++){  //  #pointers == num_keys + 1
        child = (node *)(new_nd->pointers[i]);
        child->parent = new_nd;
    }

    free(temp_ps);
    free(temp_ks);
    return insert_into_parent(root, nd, new_nd, new_key);
}

void insert_into_leaf(node *leaf, int index, char *key, record *rec)
{
    int i;
    for (i = leaf->num_keys; i > index; i--){
        leaf->keys[i] = leaf->keys[i-1];
        leaf->pointers[i] = leaf->pointers[i-1];
    }
    leaf->keys[index] = malloc(MAX_KEY_LEN);
    strcpy(leaf->keys[index], key);
    leaf->pointers[index] = rec;
    leaf->num_keys++;
}

node *insert_into_leaf_after_splitting(node *root, node *leaf, int index, char *key, record *rec)
{
    node *new_leaf;
    record **temp_ps;
    char **temp_ks, *new_key;
    int i, split;

    temp_ps = malloc(size * sizeof(record *));
    temp_ks = malloc(size * sizeof(char *));
    for (i = 0; i < size; i++){
        if (i == index){
            temp_ps[i] = rec;
            temp_ks[i] = malloc(MAX_KEY_LEN);
            strcpy(temp_ks[i], key);
        }
        else if (i < index){
            temp_ps[i] = leaf->pointers[i];
            temp_ks[i] = leaf->keys[i];
        }
        else{
            temp_ps[i] = leaf->pointers[i-1];
            temp_ks[i] = leaf->keys[i-1];
        }
    }

    split = size / 2;
    leaf->num_keys = split;
    for (i = 0; i < split; i++){
        leaf->pointers[i] = temp_ps[i];
        leaf->keys[i] = temp_ks[i];
    }

    new_leaf = make_new_leaf();
    new_leaf->num_keys = size - split;
    for (; i < size; i++){
        new_leaf->pointers[i - split] = temp_ps[i];
        new_leaf->keys[i - split] = temp_ks[i];
    }

    new_leaf->parent = leaf->parent;
    new_leaf->pointers[size - 1] = leaf->pointers[size - 1];
    leaf->pointers[size - 1] = new_leaf;
    free(temp_ps);
    free(temp_ks);
    new_key = new_leaf->keys[0];
    return insert_into_parent(root, leaf, new_leaf, new_key);
}

node *delete(node *root, char *key)
{
    node *leaf;
    record *rec;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return root;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)  // no such key
        return root;
    rec = (record *)leaf->pointers[i];
    root = delete_entry(root, leaf, i);
    return root;
}

node *delete_entry(node *root, node *nd, int index)
{
    int min_keys, cap, nd_index;
    node *neighbor;

    remove_entry(nd, index);
    if (nd == root)
        return adjust_root(nd);
    min_keys = nd->is_leaf ? size / 2 : (size - 1) / 2;
    if (nd->num_keys >= min_keys) {
        return root;
    }

    nd_index = get_node_index(nd);
    if (nd_index == 0)
        neighbor = nd->parent->pointers[1];  // right neighbor
    else
        neighbor = nd->parent->pointers[nd_index - 1]; // left neighbor

    cap = nd->is_leaf ? size - 1 : size - 2;
    if (neighbor->num_keys + nd->num_keys <= cap)
        return coalesce_nodes(root, nd, neighbor, nd_index);

    distribute_nodes(nd, neighbor, nd_index);
    return root;
}

void distribute_nodes(node *nd, node *neighbor, int nd_index)
{
    int i;
    node *tmp;
    if (nd_index != 0) {
        if (!nd->is_leaf)
            nd->pointers[nd->num_keys + 1] = nd->pointers[nd->num_keys];
        for (i = nd->num_keys; i > 0; i--){  // shift to right by 1
            nd->keys[i] = nd->keys[i - 1];
            nd->pointers[i] = nd->pointers[i - 1];
        }
        if (!nd->is_leaf){
            nd->keys[0] = nd->parent->keys[nd_index - 1];

            nd->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)nd->pointers[0];
            tmp->parent = nd;
            neighbor->pointers[neighbor->num_keys] = NULL;

            nd->parent->keys[nd_index - 1] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
        }
        else {
            nd->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;

            nd->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;

            // nd->parent->keys[nd_index - 1] = nd->keys[0];  //  share the same key with child !!
            strcpy(nd->parent->keys[nd_index - 1], nd->keys[0]);
        }
    }
    else {
        if (!nd->is_leaf){
            nd->keys[nd->num_keys] = nd->parent->keys[0];  // link to father's key
            nd->pointers[nd->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)nd->pointers[nd->num_keys + 1];
            tmp->parent = nd;
            nd->parent->keys[0] = neighbor->keys[0];  //
        }
        else {
            nd->keys[nd->num_keys] = neighbor->keys[0];
            nd->pointers[nd->num_keys] = neighbor->pointers[0];
            // nd->parent->keys[0] = neighbor->keys[1];  // share the same key with chid !!
            strcpy(nd->parent->keys[0], neighbor->keys[1]);
        }
        for (i = 0; i < neighbor->num_keys - 1; i++){
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        neighbor->keys[i] = NULL;
        if (!nd->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        else
            neighbor->pointers[i] = NULL;
    }

    neighbor->num_keys--;
    nd->num_keys++;

}

node *coalesce_nodes(node *root, node *nd, node *neighbor, int nd_index)
{
    int i, j, start, end;
    char *k_prime;
    node *tmp, *parent;

    if (nd_index == 0) {  // make sure neighbor is on the left
        tmp = nd;
        nd = neighbor;
        neighbor = tmp;
        nd_index = 1;
    }
    parent = nd->parent;

    start = neighbor->num_keys;
    if (nd->is_leaf){
        for (i = start, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->pointers[i] = nd->pointers[j];
            nd->keys[j] = NULL;
            nd->pointers[j] = NULL;
        }
        neighbor->num_keys += nd->num_keys;
        neighbor->pointers[size - 1] = nd->pointers[size - 1];
    }
    else {
        neighbor->keys[start] = malloc(MAX_KEY_LEN);
        strcpy(neighbor->keys[start], parent->keys[nd_index - 1]);
        // neighbor->keys[start] = parent->keys[nd_index - 1];
        for (i = start + 1, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->pointers[i] = nd->pointers[j];
        }
        neighbor->pointers[i] = nd->pointers[j];
        neighbor->num_keys += nd->num_keys + 1;

        for (i = 0; i <= neighbor->num_keys; i++){
            tmp = (node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }
    destroy_node(nd);
    return delete_entry(root, parent, nd_index);
}

int get_node_index(node *nd)
{
    node *parent;
    int i;
    parent = nd->parent;
    for (i = 0; i < parent->num_keys && parent->pointers[i] != nd; i++)
        ;
    return i;
}

node *adjust_root(node *root)
{
    node *new_root;
    if (root->num_keys > 0)  // at least two childs
        return root;
    if (!root->is_leaf){  // root has only one child
        new_root = root->pointers[0];
        new_root->parent = NULL;
    }
    else
        new_root = NULL;
    destroy_node(root);
    return new_root;
}

void *remove_entry(node *nd, int index)
{
    int i, index_k;

    if (nd->is_leaf){
        free(nd->keys[index]);
        free(nd->pointers[index]);  // destroy the record
        for (i = index; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->pointers[i] = nd->pointers[i + 1];
        }
        nd->keys[i] = NULL;
        nd->pointers[i] = NULL;
    }
    else{
        index_k = index - 1;  // index_p == index
        free(nd->keys[index_k]);
        for (i = index_k; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->pointers[i + 1] = nd->pointers[i + 2];
        }
        nd->keys[i] = NULL;
        nd->pointers[i + 1] = NULL;
    }
    nd->num_keys--;
}

node *bulk_load(char **keys, int *values, int n)
{
    node *root, *p;
    record *rec;
    int i;
    qsort(keys, n, sizeof(char *), cmp);
    // qsort(keys, n, sizeof(char *), (int (*)(const char **, const char **))cmp);
    root = make_new_tree(keys[0], values[0]);
    for (i = 1; i < n; i++) {
        if (strcmp(keys[i], keys[i-1]) == 0)  // ignore duplicates (key)
            continue;
        p = root;
        while (!p->is_leaf) {
            p = p->pointers[p->num_keys];  // right most child
        } // p is the right most child
        rec = make_new_record(values[i]);
        if (p->num_keys < size - 1)
            insert_into_leaf(p, p->num_keys, keys[i], rec);
        else
            root = insert_into_leaf_after_splitting(root, p, p->num_keys, keys[i], rec);
    }
    return root;
}

int cmp(const void *p, const void *q)
{
    //keys' type is (char **), each item is a (char *)
    return strcmp(*(char **)p, *(char **)q);
}

// ** for testing
node *test_insert(node *root)
{
    char *key;
    int i, value;
    key = malloc(MAX_KEY_LEN);
    for (i = 0; i < MAX_NUM_VALUE; i++) {
        scanf("%s%d", key, &value);
        if (strcmp(key, "exit") == 0)
            break;
        root = insert(root, key, value);
        printf("After insert %s: \n", key);
        print_tree(root);
    }
    return root;
}

void test_find(node *root)
{
    char *key;
    record *r;
    key = malloc(MAX_KEY_LEN);
    while (1) {
        scanf("%s", key);
        if (strcmp(key, "exit") == 0)
            break;
        r = find(root, key);
        if (r == NULL)
            printf("( %s ) Not found!!\n", key);
        else
            printf("Record of %s: %d\n", key, r->value);
    }
}

node *test_delete(node *root)
{
    char *key;
    key = malloc(MAX_KEY_LEN);
    while (1) {
        scanf("%s", key);
        if (strcmp(key, "exit") == 0)
            break;
        root = delete(root, key);
        printf("After delete %s: \n", key);
        print_tree(root);
    }
    return root;
}

void test_bulk_load(void)
{
    int i, n;
    node *root;
    char **keys;
    int *values;
    keys = malloc(sizeof(char *) * MAX_NUM_VALUE);
    values = malloc(sizeof(int) * MAX_NUM_VALUE);
    for (i = 0; i < MAX_NUM_VALUE; i++) {
        keys[i] = malloc(MAX_KEY_LEN);
        if (scanf("%s%d", keys[i], &values[i]) == EOF) {
            free(keys[i]);
            break;
        }
    }
    n = i;
    root = bulk_load(keys, values, n);
    print_tree(root);
    destroy_tree(root);

    for (i = 0; i < n; i++) {
        free(keys[i]);
    }
    free(keys);
    free(values);
}
// ** end of testing

main(int argc, char *argv[])
{
    node *root = NULL;

    // test_bulk_load();
    root = test_insert(root);
    test_find(root);
    test_delete(root);
}
