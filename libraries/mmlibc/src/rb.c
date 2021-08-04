//rb.c
//Red-Black Tree implementation for MuKe's standard library
//Bryan E. Topp <betopp@betopp.com> 2021

#include "rb.h"

#include <assert.h>
#include <stddef.h>

//Asserts bloat this code quite a bit, but are very useful when memory management is involved.
//#define assert(x) (void)(x)

//Red-Black helpers:

//Returns the parent of the node, or NULL if passed NULL or the root.
static _rb_item_t *_rb_item_parent(const _rb_item_t *item)
{
	if(item == NULL)
	{
		return NULL;
	}
	
	return item->parent;
}

//Returns the grandparent of the node, or NULL if passed NULL, the root, or a child of the root.
static _rb_item_t *_rb_item_grandparent(const _rb_item_t *item)
{
	return _rb_item_parent(_rb_item_parent(item));
}

//Returns the sibling of a node - the other node under its parent. Returns NULL in any case where it doesn't exist.
static _rb_item_t *_rb_item_sibling(const _rb_item_t *item)
{
	const _rb_item_t *parent = _rb_item_parent(item);
	if(parent == NULL)
	{
		return NULL;
	}
	
	assert(parent->tree == item->tree);
	assert(parent->left == item || parent->right == item);
	
	if(parent->left == item)
	{
		assert(parent->right != item);
		return parent->right;
	}
	
	if(parent->right == item)
	{
		assert(parent->left != item);
		return parent->left;
	}
	
	assert(0);
	return NULL;
}

//Returns the uncle of a node - the sibling of its parent. Returns NULL in any case where it doesn't exist.
static _rb_item_t *_rb_item_uncle(const _rb_item_t *item)
{
	return _rb_item_sibling(_rb_item_parent(item));
}

//Performs a "left rotation" on the node. The item's right child takes the place of the item. The item becomes its left child.
static void _rb_item_rotleft(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item->tree == tree);
	assert(item->right != NULL);
	
	_rb_item_t *oldright = item->right;
	_rb_item_t *oldparent = _rb_item_parent(item);
	
	item->parent = oldright;
	item->right = oldright->left;
	if(item->right != NULL)
	{
		item->right->parent = item;
	}
		
	oldright->left = item;
	oldright->parent = oldparent;
	
	if(oldparent != NULL)
	{
		if(oldparent->left == item)
		{
			oldparent->left = oldright;
		}
		else if(oldparent->right == item)
		{
			oldparent->right = oldright;
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		assert(tree->root == item);
		tree->root = oldright;
	}
	

	assert(item->parent != NULL);	
	assert(tree->root != NULL);
	assert(tree->root->parent == NULL);
}

//Performs a "right rotation" on the node. The item's left child takes the place of the item. The item becomes its right child.
static void _rb_item_rotright(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item->tree == tree);
	assert(item->left != NULL);
	
	_rb_item_t *oldleft = item->left;
	_rb_item_t *oldparent = _rb_item_parent(item);
	
	item->parent = oldleft;
	item->left = oldleft->right;
	if(item->left != NULL)
	{
		item->left->parent = item;
	}
	
	oldleft->right = item;
	oldleft->parent = oldparent;
	
	if(oldparent != NULL)
	{
		if(oldparent->left == item)
		{
			oldparent->left = oldleft;
		}
		else if(oldparent->right == item)
		{
			oldparent->right = oldleft;
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		assert(tree->root == item);
		tree->root = oldleft;
	}
	
	assert(item->parent != NULL);	
	assert(tree->root != NULL);
	assert(tree->root->parent == NULL);
}

void _rb_insert(_rb_tree_t *tree, _rb_item_t *item, _rb_key_t id, void *userptr)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == NULL);
	assert(item->parent == NULL);
	assert(item->left == NULL);
	assert(item->right == NULL);
	
	//Store the ID and user pointer in the item
	item->id = id;
	item->userptr = userptr;
	
	//Insert the item into the correct spot for a binary tree.
	if(tree->root == NULL)
	{
		//Trivial case - we're the root
		tree->root = item;
	}
	else
	{		
		//Traverse tree to find an appropriate parent
		_rb_item_t *parent = tree->root;
		while(1)
		{
			assert(parent != NULL);
			assert(tree->allowdup || (parent->id != id));
			assert(parent->tree == tree);
			
			if(id < parent->id)
			{
				if(parent->left == NULL)
				{
					parent->left = item;
					item->parent = parent;
					break;
				}
				
				parent = parent->left;
			}
			else
			{
				if(parent->right == NULL)
				{
					parent->right = item;
					item->parent = parent;
					break;
				}
				
				parent = parent->right;
			}
		}
	}
	
	assert(tree->root->parent == NULL);
	
	//Note that it's part of the tree now, and start its color as red
	item->tree = tree;
	item->red = true;
	
	//Repair the red-black properties of the tree, based on what just happened
	_rb_item_t *repair = item;
	while(1)
	{
		if(repair->parent == NULL)
		{
			//Inserted the root. Make it black.
			assert(tree->root == repair);
			repair->red = false;
			break;
		}
		else if(!(repair->parent->red))
		{
			//Parent is black.
			//So, it's okay for it to have a red child.
			//We replaced an implicitly-black NULL child with our new red node and its two black NULL children.
			//So, nobody's black-depth changed. We're fine.
			break;
		}
		else if(_rb_item_uncle(repair) != NULL && _rb_item_uncle(repair)->red)
		{
			//Parent and uncle are red.
			//Set the parent and uncle to black and the grandparent (was black) to red.
			//Then, repair the grandparent. (Having an uncle implies that there's a grandparent.)
			assert(repair->parent != NULL);
			assert(repair->parent->red);
			
			repair->parent->red = false;
			_rb_item_uncle(repair)->red = false;
			
			assert(_rb_item_grandparent(repair) != NULL);
			_rb_item_grandparent(repair)->red = true;
			repair = _rb_item_grandparent(repair);
			continue;
		}
		else
		{
			//Parent is red but uncle is not.
			//Rotate the node into the grandparent's position.
			
			_rb_item_t *parent = _rb_item_parent(repair);
			_rb_item_t *grandparent = _rb_item_grandparent(repair);
			
			assert(parent != NULL);
			assert(grandparent != NULL); //The node has a parent and the parent is red, so parent is not root
			
			//Get the node to the outside by rotating the parent, if necessary
			if(repair == parent->right && parent == grandparent->left)
			{
				_rb_item_rotleft(tree, parent);
				repair = repair->left;
			}
			else if(repair == parent->left && parent == grandparent->right)
			{
				_rb_item_rotright(tree, parent);
				repair = repair->right;
			}
			
			//Rotate the grandparent 
			parent = _rb_item_parent(repair);
			grandparent = _rb_item_grandparent(repair);
			if(repair == parent->left)
			{
				_rb_item_rotright(tree, grandparent);
			}
			else
			{
				_rb_item_rotleft(tree, grandparent);
			}
			
			parent->red = false;
			grandparent->red = true;
			break;
		}
	}
	
	assert(repair != NULL);
	assert( (!repair->red) || (repair->parent == NULL) || (!repair->parent->red) );
}

_rb_item_t *_rb_first(_rb_tree_t *tree)
{
	assert(tree != NULL);
	
	_rb_item_t *item = tree->root;
	if(item == NULL)
	{
		//No nodes
		return NULL;
	}
	
	//Traverse down the left side as far as we can go
	while(item != NULL)
	{
		assert(item->tree == tree);
		
		if(item->left == NULL)
		{
			return item;
		}
		
		if(tree->allowdup)
		{
			assert(item->left->id <= item->id);
		}
		else
		{
			assert(item->left->id < item->id);
		}
		
		item = item->left;
	}
	
	//Should be unreachable
	assert(0);
	return NULL;
}

_rb_item_t *_rb_find(_rb_tree_t *tree, _rb_key_t id)
{
	assert(tree != NULL);
	
	//Normal binary-tree search
	_rb_item_t *iptr = tree->root;	
	while(iptr != NULL)
	{
		//Validate basic facts about the node...
		assert(iptr->tree == tree);		
		if(iptr->red)
		{
			//If a node is red, both its children are black.
			assert(iptr->left == NULL || !(iptr->left->red));
			assert(iptr->right == NULL || !(iptr->right->red));
		}
		
		if(id < iptr->id)
		{
			//We're looking for an ID lower than this node's.
			//Look at the left subtree.
			if(iptr->left != NULL)
			{
				assert(iptr->left->parent == iptr);
				
				if(tree->allowdup)
				{
					assert(iptr->left->id <= iptr->id);
				}
				else
				{
					assert(iptr->left->id < iptr->id);
				}
			}
			
			iptr = iptr->left;
			continue;
		}
		
		if(id > iptr->id)
		{
			//We're looking for an ID higher than this node's.
			//Look at the right subtree.
			if(iptr->right != NULL)
			{
				assert(iptr->right->parent == iptr);
				
				if(tree->allowdup)
				{
					assert(iptr->right->id >= iptr->id);
				}
				else
				{
					assert(iptr->right->id > iptr->id);
				}
			}
				
			iptr = iptr->right;
			continue;
		}
		
		//ID matches, this is the node we're looking for
		assert(id == iptr->id);
		return iptr;
	}
	
	//Didn't find the node
	return NULL;
}

_rb_item_t *_rb_next(_rb_tree_t *tree, const _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == tree);
	
	if(item->right != NULL)
	{
		//Item has a right child.
		//The successor is the leftmost element in its right subtree.
		_rb_item_t *successor = item->right;
		while(successor->left != NULL)
		{
			successor = successor->left;
		}
		
		assert(successor != NULL);
		return successor;
	}
	
	//No right subtree.
	//Look for a node on the left side its parent.
	//Then, the parent is the successor.
	while(item != NULL)
	{
		assert(item->tree == tree);
		
		if(item->parent != NULL && item->parent->left == item)
		{
			return item->parent;
		}
		
		item = item->parent;
	}
	
	//No right subtree and didn't find any nodes in the parentage who were left-children.
	//We were the last node.
	return NULL;	
}

_rb_key_t _rb_unused(_rb_tree_t *tree)
{
	//Prefer to allocate in-order, advancing the ID each time.
	//If that fails, actually traverse the tree to find a free ID.
	if((tree->unused_nexttry > 0) && (_rb_find(tree, tree->unused_nexttry) == NULL))
	{
		_rb_key_t retval = tree->unused_nexttry;
		tree->unused_nexttry++;
		return retval;
	}
	
	//In-order approach didn't work. We must have asked for a real huge number of IDs...
	//Start looking for an ID we can use
	_rb_key_t try_id = 1;
	
	//Start looking at the lowest ID in the tree
	_rb_item_t *nextitem = _rb_first(tree);
	
	//Advance our attempt, one possible ID at a time, one used ID at a time
	while(try_id > 0) //Go until overflow.
	{
		//If the item is using this ID, we can't use it.
		if(nextitem != NULL && nextitem->id == try_id)
		{
			//Try the next ID.
			try_id++;
			
			//If it's present in the tree, it will be the next item.
			nextitem = _rb_next(tree, nextitem);
			
			continue;
		}
		
		//We found an ID that doesn't line up with our in-order traversal of the tree.
		//That means there's a gap - the item we see should be ahead of the ID we're trying.
		assert(nextitem == NULL || nextitem->id > try_id);
		
		//The ID should then be usable.
		assert(_rb_find(tree, try_id) == NULL);
		tree->unused_nexttry = try_id + 1;
		return try_id;
	}
	
	//...we shouldn't ever run out of IDs to use in a tree.
	//(Given that we use 64-bit identifiers, and have a 64-bit address space...)
	assert(0);
	return -1;
}

_rb_key_t _rb_insert_newid(_rb_tree_t *tree, _rb_item_t *item, void *userptr)
{
	_rb_key_t new_id = _rb_unused(tree);
	_rb_insert(tree, item, new_id, userptr);
	return new_id;
}

//Swaps the positions of the two items in the tree and their colors.
//(i.e. colors remain at their positions in the tree.)
//Only safe if the two items are adjacent in the tree ordering!
static void _rb_item_swap(_rb_tree_t *tree, _rb_item_t *a, _rb_item_t *b)
{
	assert(tree != NULL);
	assert(a != NULL);
	assert(b != NULL);
	assert(a != b);
	assert(a->tree == tree);
	assert(b->tree == tree);
	
	_rb_item_t *temp;
	
	//If B is the parent of A, swap the arguments to favor "A" being a parent instead.
	//(Fewer cases to handle later, then.)
	if(b->left == a || b->right == a)
	{
		temp = a;
		a = b;
		b = temp;
	}
	
	assert(a->parent != b);
	assert(b->left != a);
	assert(b->right != a);
	
	//These are then always safe to exchange
	if(a->parent != NULL && a->parent->left == a)
	{
		a->parent->left = b;
	}
	
	if(a->parent != NULL && a->parent->right == a)
	{
		a->parent->right = b;
	}
	
	if(b->left != NULL)
	{
		b->left->parent = a;
	}
	
	if(b->right != NULL)
	{
		b->right->parent = a;
	}
	
	if(a->parent == NULL)
	{
		assert(tree->root == a);
		tree->root = b;
	}
	
	if(b->parent == NULL)
	{
		assert(tree->root == b);
		tree->root = a;
	}

	
	//Then, handle each case - B is a left child of A, B is a right child of A, B is not linked to A.
	if(a->left == b)
	{
		assert(b->parent == a);
		assert(a->right != b);
		
		if(a->right != NULL)
		{
			a->right->parent = b;
		}
		
		a->left = b->left;
		b->left = a;
		
		temp = a->right;
		a->right = b->right;
		b->right = temp;
			
		b->parent = a->parent;
		a->parent = b;
		
	}
	else if(a->right == b)
	{
		assert(b->parent == a);	
		assert(a->left != b);
		
		if(a->left != NULL)
		{
			a->left->parent = b;
		}
		
		a->right = b->right;
		b->right = a;
		
		temp = a->left;
		a->left = b->left;
		b->left = temp;
		
		b->parent = a->parent;
		a->parent = b;
	}
	else
	{
		assert(b->parent != a);
		assert(a->left != b);
		assert(a->right != b);
		
		if(a->left != NULL)
		{
			a->left->parent = b;
		}
		
		if(a->right != NULL)
		{
			a->right->parent = b;
		}
		
		if(b->parent != NULL && b->parent->left == b)
		{
			b->parent->left = a;
		}
		
		if(b->parent != NULL && b->parent->right == b)
		{
			b->parent->right = b;
		}
		
		temp = a->left;
		a->left = b->left;
		b->left = temp;
		
		temp = a->right;
		a->right = b->right;
		b->right = temp;
	
		temp = a->parent;
		a->parent = b->parent;
		b->parent = temp;
	}
	
	//Consistency checks - linkage only, not ordering.
	//We expect that we've messed up the ordering of the two nodes we swapped.
	if(a->parent != NULL)
	{		
		assert(tree->root != a);
	}
	else
	{
		//A is root so B can't be.
		assert(b->parent != NULL);
		assert(tree->root == a);
	}
	
	if(b->parent != NULL)
	{
		assert(tree->root != b);
	}
	else
	{
		//B is root so A can't be.
		assert(a->parent != NULL);
		assert(tree->root == b);
	}
	
	if(a->left != NULL)
	{
		assert(a->left->parent == a);
	}
	
	if(a->right != NULL)
	{
		assert(a->right->parent == a);
	}
	
	if(b->left != NULL)
	{
		assert(b->left->parent == b);
	}
	
	if(b->right != NULL)
	{
		assert(b->right->parent == b);
	}
	
	//Swap the colors, so the tree has the same coloration as when we started.
	bool a_red = a->red;
	a->red = b->red;
	b->red = a_red;
}

//Removes an item from the tree, replacing it with its one child (if any) or NULL otherwise.
//Returns the child that is now in its place.
static _rb_item_t *_rb_item_replacewithchild(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == tree);
	
	_rb_item_t *item_to_validate = NULL;
	
	if(item->left != NULL)
	{
		//Left child will take our place. Right child must be NULL.
		assert(item->right == NULL);
		assert(item->left->tree == item->tree);
		
		if(item->parent != NULL && item->parent->left == item)
		{
			assert(tree->root != item);
			assert(item->parent->right != item);
			item->parent->left = item->left;
		}
		
		if(item->parent != NULL && item->parent->right == item)
		{
			assert(tree->root != item);
			assert(item->parent->left != item);
			item->parent->right = item->left;
		}
		
		if(item->parent == NULL)
		{
			assert(tree->root == item);
			tree->root = item->left;
		}
		
		item->left->parent = item->parent;
		item_to_validate = item->left;
	}
	else if(item->right != NULL)
	{
		//Right child will take our place. Left child must be null.
		assert(item->left == NULL);
		assert(item->right->tree == item->tree);
		
		if(item->parent != NULL && item->parent->left == item)
		{
			assert(tree->root != item);
			assert(item->parent->right != item);
			item->parent->left = item->right;
		}
		
		if(item->parent != NULL && item->parent->right == item)
		{
			assert(tree->root != item);
			assert(item->parent->left != item);
			item->parent->right = item->right;
		}
		
		if(item->parent == NULL)
		{
			assert(tree->root == item);
			tree->root = item->right;
		}
		
		item->right->parent = item->parent;
		item_to_validate = item->right;
	}
	else
	{
		//Item has no children.
		if(item->parent != NULL && item->parent->left == item)
		{
			assert(tree->root != item);
			assert(item->parent->right != item);
			item->parent->left = NULL;
		}
		
		if(item->parent != NULL && item->parent->right == item)
		{
			assert(tree->root != item);
			assert(item->parent->left != item);
			item->parent->right = NULL;
		}
		
		if(item->parent == NULL)
		{
			assert(tree->root == item);
			tree->root = NULL;
		}
		
		item_to_validate = NULL;
	}
	
	if(item_to_validate != NULL)
	{
		if(item_to_validate->parent != NULL)
		{
			assert(item_to_validate->parent->left == item_to_validate || item_to_validate->parent->right == item_to_validate);
			assert(tree->root != item_to_validate);
		}
		else
		{
			assert(tree->root == item_to_validate);
		}
		
		if(item_to_validate->left != NULL)
		{
			assert(item_to_validate->left->parent == item_to_validate);
		}
		
		if(item_to_validate->right != NULL)
		{
			assert(item_to_validate->right->parent == item_to_validate);
		}
		
		assert(item_to_validate->tree == tree);
		
		//Verify that nothing is referencing the item anymore
		assert(item_to_validate->left != item);
		assert(item_to_validate->right != item);
		assert(item_to_validate->parent != item);
	}
	
	
	//Remove the item from the tree
	item->left = NULL;
	item->right = NULL;
	item->parent = NULL;
	item->tree = NULL;
	
	assert(tree->root != item);
	
	return item_to_validate;
}

static void _rb_remove_fixup(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == tree);
	
	//Only nodes with 0 or 1 non-NULL child are suitable for removal.
	assert(item->left == NULL || item->right == NULL);
	
	if(item->red)
	{
		//Removing a red item.
		
		//Its children must be black.
		assert(item->left == NULL || !item->left->red);
		assert(item->right == NULL || !item->right->red);
		
		//No black-depth is affected.
		//No further action is needed.
		return;
	}
	
	//Removing a black item.
	
	//If its one child is red, we'll just turn the child black.
	//Then the child can take the place of the item.
	if(item->left != NULL && item->left->red)
	{
		assert(item->right == NULL);
		item->left->red = false;
		return;
	}
	
	if(item->right != NULL && item->right->red)
	{
		assert(item->left == NULL);
		item->right->red = false;
		return;
	}
	
	//We're about to remove a black item, and its replacement will also be black.
	//Black-depths will be affected. We need to fixup our way towards the root of the tree.
	while(1)
	{
		assert(item != NULL);
		
		if(item->parent == NULL)
		{
			//If we're about to remove the black root node and replace it with its single black child...
			//then everyone's black-depth will decrease equally. No fixup is needed.
			assert(tree->root == item);
			return;
		}
		
		_rb_item_t *sibling = _rb_item_sibling(item);
		if(sibling != NULL && sibling->red)
		{
			item->parent->red = true;
			sibling->red = false;
			if(item == item->parent->left)
			{
				_rb_item_rotleft(tree, item->parent);
			}
			else
			{
				_rb_item_rotright(tree, item->parent);
			}
		}
		
		sibling = _rb_item_sibling(item);
		
		bool parent_black = item->parent == NULL || !item->parent->red;
		bool sibling_black = sibling == NULL || !sibling->red;
		bool sibling_left_black = sibling == NULL || sibling->left == NULL || !sibling->left->red;
		bool sibling_right_black = sibling == NULL || sibling->right == NULL || !sibling->right->red;
		
		if(sibling_black && sibling_left_black && sibling_right_black)
		{
			if(parent_black)
			{		
				//Recurse up the tree
				sibling->red = true;
				item = item->parent;
				continue;
			}
			else
			{
				sibling->red = true;
				item->parent->red = false;
				
				//No further fixup needed
				return;
			}
		}
		

		if(sibling_black)
		{
			if( (item == item->parent->left) && sibling_right_black && !sibling_left_black)
			{
				sibling->red = true;
				sibling->left->red = false;
				_rb_item_rotright(tree, sibling);
			}
			else if( (item == item->parent->right) && sibling_left_black && !sibling_right_black)
			{
				sibling->red = true;
				sibling->right->red = false;
				_rb_item_rotleft(tree, sibling);
			}
		}
		
		sibling = _rb_item_sibling(item);
		
		sibling->red = item->parent->red;
		item->parent->red = false;
		
		if(item == item->parent->left)
		{
			sibling->right->red = false;
			_rb_item_rotleft(tree, item->parent);
		}
		else
		{
			sibling->left->red = false;
			_rb_item_rotright(tree, item->parent);
		}
		
		//No further fixup needed
		return;
	}
}

void _rb_remove(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == tree);
	
	//If the item has both left and right non-NULL children, swap it with its immediate successor.
	//(Maintain the coloring based on the position in the tree.)
	if(item->left != NULL && item->right != NULL)
	{
		assert(item->left->tree == tree);
		assert(item->right->tree == tree);
		
		if(tree->allowdup)
		{
			assert(item->left->id <= item->id);
			assert(item->right->id >= item->id);
		}
		else
		{
			assert(item->left->id < item->id);
			assert(item->right->id > item->id);
		}
		
		//The successor is the leftmost node in the right subtree.
		_rb_item_t *successor = item->right;
		while(successor->left != NULL)
		{
			assert(successor->tree == tree);
			
			if(tree->allowdup)
			{
				assert(successor->left->id <= successor->id);
			}
			else
			{
				assert(successor->left->id < successor->id);
			}
			
			successor = successor->left;
		}
		
		assert(successor != NULL);
		
		if(tree->allowdup)
		{
			assert(successor->id >= item->id);
		}
		else
		{
			assert(successor->id > item->id);
		}
		
		assert(successor->left == NULL);
		
		_rb_item_swap(tree, item, successor);
	}
	
	//The item in question should now have 0 or 1 non-NULL child.
	assert(item->left == NULL || item->right == NULL);
	
	//Perform fixups necessary if we're removing a black node
	_rb_remove_fixup(tree, item);
	
	//Remove it.
	_rb_item_replacewithchild(tree, item);
	
	//Item should be removed
	assert(tree->root != item);
	assert(item->tree == NULL);
	assert(item->parent == NULL);
	assert(item->left == NULL);
	assert(item->right == NULL);
}

bool _rb_contains(_rb_tree_t *tree, _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	
	if(item->tree == tree)
	{
		return true;
	}
	else
	{
		return false;
	}
}

_rb_item_t *_rb_findabout(_rb_tree_t *tree, _rb_key_t id)
{
	assert(tree != NULL);
	
	if(tree->root == NULL)
	{
		//No items
		return NULL;
	}
	
	//Normal binary-tree search
	_rb_item_t *iptr = tree->root;	
	while(iptr != NULL)
	{
		//Validate basic facts about the node...
		assert(iptr->tree == tree);		
		if(iptr->red)
		{
			//If a node is red, both its children are black.
			assert(iptr->left == NULL || !(iptr->left->red));
			assert(iptr->right == NULL || !(iptr->right->red));
		}
		
		if(id < iptr->id)
		{
			//We're looking for an ID lower than this node's.
			//Look at the left subtree.
			if(iptr->left != NULL)
			{
				assert(iptr->left->parent == iptr);
				
				if(tree->allowdup)
				{
					assert(iptr->left->id <= iptr->id);
				}
				else
				{
					assert(iptr->left->id < iptr->id);
				}
				
				iptr = iptr->left;
				continue;
			}
			else
			{
				//Nothing to our left... we didn't find the value we wanted.
				//Return the node we arrived at.
				return iptr;
			}
		}
		
		if(id > iptr->id)
		{
			//We're looking for an ID higher than this node's.
			//Look at the right subtree.
			if(iptr->right != NULL)
			{
				assert(iptr->right->parent == iptr);
				
				if(tree->allowdup)
				{
					assert(iptr->right->id >= iptr->id);
				}
				else
				{
					assert(iptr->right->id > iptr->id);
				}
					
				iptr = iptr->right;
				continue;
			}
			else
			{
				//Nothing to our right... we didn't find the value we wanted.
				//Return the node we arrived at.
				return iptr;
			}
			
		}
		
		//ID matches, this is the node we're looking for
		assert(id == iptr->id);
		return iptr;
	}
	
	//Shouldn't arrive here, because we only set iptr to known non-NULL values.
	assert(0);
	return NULL;
}

_rb_item_t *_rb_last(_rb_tree_t *tree)
{
	assert(tree != NULL);
	
	_rb_item_t *item = tree->root;
	if(item == NULL)
	{
		//No nodes
		return NULL;
	}
	
	//Traverse down the right side as far as we can go
	while(item != NULL)
	{
		assert(item->tree == tree);
		
		if(item->right == NULL)
		{
			return item;
		}
		
		if(tree->allowdup)
		{
			assert(item->right->id >= item->id);
		}
		else
		{
			assert(item->right->id > item->id);
		}
		
		item = item->right;
	}
	
	//Should be unreachable
	assert(0);
	return NULL;
}

_rb_item_t *_rb_prev(_rb_tree_t *tree, const _rb_item_t *item)
{
	assert(tree != NULL);
	assert(item != NULL);
	assert(item->tree == tree);
	
	if(item->left != NULL)
	{
		//Item has a left child.
		//The predecessor is the rightmost element in its left subtree.
		_rb_item_t *predecessor = item->left;
		while(predecessor->right != NULL)
		{
			predecessor = predecessor->right;
		}
		
		assert(predecessor != NULL);
		return predecessor;
	}
	
	//No left subtree.
	//Look for a node on the right side its parent.
	//Then, the parent is the predecessor.
	while(item != NULL)
	{
		assert(item->tree == tree);
		
		if(item->parent != NULL && item->parent->right == item)
		{
			return item->parent;
		}
		
		item = item->parent;
	}
	
	//No left subtree and didn't find any nodes in the parentage who were right-children.
	//We were the first node.
	return NULL;	
}

