//rb.h
//Red-Black Tree implementation for MuKe's standard library
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef _RB_H
#define _RB_H

#include <stdbool.h>
#include <stdint.h>

//Hey look everybody I took CSE310 and learned about Red-Black Trees

//Type used for keys in a Red-Black tree
typedef uint64_t _rb_key_t;

//Structure for holding a Red-Black tree
typedef struct _rb_tree_s
{
	//Item at the root of the tree.
	struct _rb_item_s *root;
	
	//Next ID to try allocating, when asked for an unused ID.
	//Not guaranteed to actually be unused.
	//Not guaranteed to actually be returned by idmap_unused.
	_rb_key_t unused_nexttry;
	
	//Whether to allow duplicate IDs
	bool allowdup;
	
} _rb_tree_t;

//Entry in an ID map - store in structures that participate.
typedef struct _rb_item_s
{
	//Red-Black tree that contains this item
	struct _rb_tree_s *tree;
	
	//ID of this item
	_rb_key_t id;
	
	//Pointer to the structure containing the item
	void *userptr;
	
	//Data below is internal, and other modules shouldn't depend on it.
	
	//Color of the node
	bool red;
	
	//Parent node
	struct _rb_item_s *parent;
	
	//Child nodes
	struct _rb_item_s *left;
	struct _rb_item_s *right;
	
} _rb_item_t;


//Inserts the given item into the Red-Black tree with the given ID and user-pointer
void _rb_insert(_rb_tree_t *tree, _rb_item_t *item, _rb_key_t id, void *userptr);

//Inserts the given item into the Red-Black tree with the given user-pointer. Assigns to it, and returns, a previously-unused ID.
_rb_key_t _rb_insert_newid(_rb_tree_t *tree, _rb_item_t *item, void *userptr);

//Removes the given item from the Red-Black tree
void _rb_remove(_rb_tree_t *tree, _rb_item_t *item);

//Checks if the given item is in the Red-Black tree
bool _rb_contains(_rb_tree_t *tree, _rb_item_t *item);

//Looks up the given ID in the Red-Black tree. Returns the item, or NULL if it's not found.
_rb_item_t *_rb_find(_rb_tree_t *tree, _rb_key_t id);

//Looks up the given ID in the Red-Black tree. Returns the item, or a nearby match if it's not found.
_rb_item_t *_rb_findabout(_rb_tree_t *tree, _rb_key_t id);

//Returns the lowest ID in the Red-Black tree. Returns NULL if no items are in the tree.
_rb_item_t *_rb_first(_rb_tree_t *tree);

//Returns the highest ID in the Red-Black tree. Returns NULL if no items are in the tree.
_rb_item_t *_rb_last(_rb_tree_t *tree);

//Returns the predecessor of the given item. Returns NULL if it's the first (lowest ID)
_rb_item_t *_rb_prev(_rb_tree_t *tree, const _rb_item_t *item);

//Returns the successor of the given item. Returns NULL if it's the last (highest ID)
_rb_item_t *_rb_next(_rb_tree_t *tree, const _rb_item_t *item);

//Returns a free ID in the given Red-Black tree
_rb_key_t _rb_unused(_rb_tree_t *tree);

#endif //_RB_H
