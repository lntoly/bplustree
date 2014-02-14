/*!	@brief B+ Tree Implementation
 *
 *	@file bplustree.h
 *	@author	xucp
 *	@date 2014-02-14
 *	@version 1.0
 *
 *	Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 */
#ifndef _BPLUSTREE_HEADER
#define _BPLUSTREE_HEADER

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4251 4996)
#endif

#ifdef WIN32
#	define BPTREE_OS_WINDOWS
#endif

#ifdef WIN32
#	ifdef BPTREE_INTERFACE_EXPORT
#		define BPTREE_INTERFACE_API __declspec(dllexport)
#	elif defined(BPTREE_INTERFACE_IMPORT)
#		define BPTREE_INTERFACE_API __declspec(dllimport)
#	else
#		define BPTREE_INTERFACE_API
#	endif
#else
#	define BPTREE_INTERFACE_API
#endif

#ifndef NULL
#	define NULL 0
#endif

#ifdef BPTREE_OS_WINDOWS
#define bool char
#define false 0
#define true 1
#endif

/**
 * Default order is 4.
 */
#define BPTREE_DEFAULT_ORDER 4

/**
 * Minimum order is necessarily 3.  We set the maximum
 * order arbitrarily.  You may change the maximum order.
 */
#define BPTREE_MIN_ORDER 3
#define BPTREE_MAX_ORDER 20

/**
 * Default key length is 4
 */
#define	BPTREE_DEFAULT_KEY_LENGTH 4

/**
 * Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
typedef struct record {
	int value;
} record;

/**
 * Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct node {
	void ** pointers;
	char ** keys;
	struct node * parent;
	bool is_leaf;
	int num_keys;
	struct node * next; /**< Used for queue.*/
} node;

/**
 * BPlusTree class.
 */
class BPTREE_INTERFACE_API BPlusTree
{
public:
	/**
	 * BPlusTree constructor.
	 * @param nOrder		order of B+ tree
	 * @param nKeyLength	length of key
	 */
	BPlusTree(int nOrder = BPTREE_DEFAULT_ORDER, int nKeyLength = BPTREE_DEFAULT_KEY_LENGTH);
	
	/**
	 * BPlusTree destructor.
	 */
	~BPlusTree();
public:
	/**
	 * Inserts a key and an associated value into
	 * the B+ tree, causing the tree to be adjusted
	 * however necessary to maintain the B+ tree
	 * properties.
	 * @param key		The key
	 * @param value		The value
	 * @return		Return the root node pointer of B+ tree.
	 */
	node * Insert( char * key, int value );
	
	/**
	 * Delete node from B+ tree which key value is key.
	 * @param key		The key
	 * @return		Return the root node pointer of B+ tree.
	 */
	node * Delete( char * key );
	
	/**
	 * Finds and returns the record to which a key refers.
	 * @param key		The key
	 * @param verbose	Causes the pointer addresses to be printed out in hexadecimal notation next to their corresponding keys
	 * @return		Return the record pointer or NULL if not found.
	 */
	record * Find( char * key, bool verbose );
	
	/**
	 * Inserts a key and an associated value into
	 * the B+ tree, causing the tree to be adjusted
	 * however necessary to maintain the B+ tree
	 * properties.
	 * @param key		The key
	 * @param value		The value
	 * @return		Return the root node pointer of B+ tree.
	 */
	node * Insert( int key, int value );
	
	/**
	 * Delete node from B+ tree which key value is key.
	 * @param key		The key
	 * @return		Return the root node pointer of B+ tree.
	 */
	node * Delete( int key );
	
	/**
	 * Finds and returns the record to which a key refers.
	 * @param key		The key
	 * @param verbose	Causes the pointer addresses to be printed out in hexadecimal notation next to their corresponding keys
	 * @return		Return the record pointer or NULL if not found.
	 */
	record * Find( int key, bool verbose );
	
	/**
	 * Destory the B+ tree.
	 */
	void DestroyBPTree();
	
	void PrintBPTree();
	void PrintBPTreeLeaves();
	void FindAndPrint( char * key, bool verbose );
	void FindAndPrint( int key, bool verbose );
	
	void SetVerbose( bool verbose );
private:
	void enqueue( node * new_node );
	node * dequeue( void );
	int height( node * root );
	int path_to_root( node * root, node * child );
	void print_leaves( node * root );
	void print_tree( node * root );
	void find_and_print( node * root, char * key, bool verbose ); 
	node * find_leaf( node * root, char * key, bool verbose );
	record * Find( node * root, char * key, bool verbose );
	int cut( int length );
	
	record * make_record( int value );
	node * make_node( void );
	node * make_leaf( void );
	int get_left_index( node * parent, node * left );
	node * insert_into_leaf( node * leaf, char * key, record * pointer );
	node * insert_into_leaf_after_splitting( node * root, node * leaf, char * key, record * pointer );
	node * insert_into_node( node * root, node * parent, 
			int left_index, char * key, node * right );
	node * insert_into_node_after_splitting( node * root, node * parent, int left_index, 
			char * key, node * right );
	node * insert_into_parent( node * root, node * left, char * key, node * right );
	node * insert_into_new_root( node * left, char * key, node * right );
	node * start_new_tree( char * key, record * pointer );
	node * Insert( node * root, char * key, int value );
	
	int get_neighbor_index( node * n );
	node * adjust_root( node * root );
	node * coalesce_nodes( node * root, node * n, node * neighbor, int neighbor_index, char * k_prime );
	node * redistribute_nodes( node * root, node * n, node * neighbor, int neighbor_index, 
			int k_prime_index, char * k_prime );
	node * delete_entry( node * root, node * n, char * key, void * pointer );
	node * remove_entry_from_node( node * n, char * key, node * pointer );
	node * Delete(node * root, char * key);
	
	node * destroy_tree( node * root );
	void destroy_tree_nodes( node * root );
private:
	/**
	 * Order of B+ tree.
	 */
	int order;
	
	/**
	 * Length of key.
	 */
	int key_length;
	
	/**
	 * The user can toggle on and off the "verbose"
	 * property, which causes the pointer addresses
	 * to be printed out in hexadecimal notation
	 * next to their corresponding keys.
	 */
	bool verbose_output;
	
	/**
	 * The queue is used to print the tree in
	 * level order, starting from the root
	 * printing each entire rank on a separate
	 * line, finishing with the leaves.
	 */
	node * queue;
	
	/**
	 * The root node of B+ tree.
	 */
	node * root_node;
};

#endif

