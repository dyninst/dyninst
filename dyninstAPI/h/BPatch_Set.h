#ifndef _BPatch_Set_h_
#define _BPatch_Set_h_

#if defined(external_templates)
#pragma interface
#endif

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include <stdlib.h>

#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif

/** template struct that will be used for default compare 
  * class for BPatch_Set operations.
  */

template<class T>
struct comparison {
	/** default comparison operator
	  * returns -1 if the first arg < second arg
	  * returns 1 if the first arg > second arg
	  * returns 0 if the first arg == second arg
	  * @param x first argument to be compared 
	  * @param y second argument to be compared
	  */
	int operator()(const T& x, const T& y) const {
		if(x<y) return -1;
		if(x>y) return 1;
		return 0;
	}
};

/** template class for BPatch_Set. The implementation is based on red black
  * tree implementation for efficiency concerns and for getting sorted
  * elements easier. The template depends on two types. The first one is the
  * the type of the elements in the BPatch_Set and the second one is the template
  * structure that is used to compare the elements of the BPatch_Set. The template
  * structure has to overload () for comparison of two elements as explained above
  */

#if defined i386_unknown_nt4_0

static const bool RED = true;
static const bool BLACK = false;

#endif

template<class T,class Compare = comparison<T> >
class BPatch_Set {
private:

#ifndef i386_unknown_nt4_0
	/** color variable used for red-black tree */
	static const bool RED = true;

	/** color variable used for red-black tree */
	static const bool BLACK = false;
#endif

	/** tree implementation structure. Used to implement the RB tree */
	typedef struct entry {
		T data; 	/* data  element */
		bool color;	/* color of the node true=red,false=blue*/
		struct entry* left; /* left child */
		struct entry* right; /* right child */
		struct entry* parent; /* parent of the node */

		/** constructor for structure */
		entry() 
			: color(BLACK),left(NULL),right(NULL),parent(NULL) {}

		/** constructor used for non-nil elements 
		  * @param e nil entry
		  */	  
		entry(entry* e) //constructor with nil entry 
			: color(RED),left(e),right(e),parent(NULL) {}

		/** constructor
		  * @param d data element
		  * @param e nill entry 
		  */
		entry(const T& d,entry* e) 
		    	: data(d),color(RED),left(e),right(e),parent(NULL){}

		/** constrcutor 
		  * @param e the entry structure that will be copied 
		  */
		entry(const entry& e) : data(e.data),color(e.color),
			left(NULL),right(NULL),parent(NULL) {}
	} entry;

	/** pointer to define the nil element of the tree NULL is not used
	  * since some operations need sentinel nil which may have non-nil
	  * parent.
	  */
	entry* nil;

	/** size of the BPatch_Set */
	int setSize;
	
	/** pointer to the tree structure */
	entry* setData;

	/** the structure that will be used for comparisons. Default is the
	  * the comparsion structure explained above */
	Compare compareFunc;

	// method that replicates the tree structure of this tree
	DO_INLINE_P entry* replicateTree(entry*,entry*,entry*,entry*);

	// method that implements left rotattion used by RB tree for balanced
	// tree construction and keeps the RBtree properties.
	DO_INLINE_P void leftRotate(entry*);

	// method that implements right rotattion used by RB tree for balanced
	// tree construction and keeps the RBtree properties.
	DO_INLINE_P void rightRotate(entry*);

	// method that modifies the tree structure after deletion for keeping
	// the RBtree properties.
	DO_INLINE_P void deleteFixup(entry*);

	// insertion to a binary search tree. It returns the new element pointer
	// that is inserted. If element is already there it returns NULL
	DO_INLINE_P entry* treeInsert(const T&);

	// finds the elemnts in the tree that will be replaced with the element
	// being deleted in the  deletion. That is the element with the largets
	// smallest value than the element being deleted. 
	DO_INLINE_P entry* treeSuccessor(entry* ) const;

	// method that returns the entry pointer for the element that is searched
	//for. If the entry is not found then it retuns NULL
	DO_INLINE_P entry* find(const T&) const;

	// infix traverse of the RB tree. It traverses the tree in ascending order
	DO_INLINE_P void traverse(T*,entry*,int&) const;

	// deletes the tree structure for deconstructor.
	DO_INLINE_P void destroy(entry*);

public:

	/** constructor. The default comparison structure is used */
	DO_INLINE_F BPatch_Set() : setSize(0) 
	{ 
		nil = new entry;
		setData = nil;
		compareFunc = Compare();
	}

	/** copy constructor.
	  * @param newBPatch_Set the BPatch_Set which will be copied
	  */
	DO_INLINE_F BPatch_Set(const BPatch_Set<T,Compare>& newBPatch_Set){
		nil = new entry;
		compareFunc = newBPatch_Set.compareFunc;
		setSize = newBPatch_Set.setSize;
		setData = replicateTree(newBPatch_Set.setData,NULL,newBPatch_Set.nil,nil);
	} 

	/** destructor which deletes all tree structure and allocated entries */
	DO_INLINE_F ~BPatch_Set() 
	{
		destroy(setData);
		delete nil;
	}

	/** returns the cardinality of the tree , number of elements */
	DO_INLINE_F int size() const { return setSize; }

	/** returns true if tree is empty */
	DO_INLINE_F bool empty() const { return (setData == nil); }

	/** inserts the element in the tree 
	  * @param 1 element that will be inserted
	  */
	DO_INLINE_F void insert(const T&);

	/** removes the element in the tree 
	  * @param 1 element that will be removed  
	  */
	DO_INLINE_F void remove(const T&);

	/** returns true if the argument is member of the BPatch_Set
	  * @param e the element that will be searched for
	  */
	DO_INLINE_F bool contains(const T&) const;

	/** fill an buffer array with the sorted
	  * elements of the BPatch_Set in ascending order according to comparison function
	  * if the BPatch_Set is empty it retuns NULL, other wise it returns 
 	  * the input argument.
	  */
	DO_INLINE_F T* elements(T*) const;

	/** returns the minimum valued member in the BPatch_Set according to the 
	  * comparison function supplied. If the BPatch_Set is empty it retuns 
	  * any number. Not safe to use for empty sets 
	  */
	DO_INLINE_F T minimum() const;

	/** returns the maximum valued member in the BPatch_Set according to the 
	  * comparison function supplied. If the BPatch_Set is empty it retuns 
	  * any number. Not safe to use for empty sets 
	  */
	DO_INLINE_F T maximum() const;
	
	/** assignment operator for BPatch_Set. It replicate sthe tree 
	  * structure into the new BPatch_Set.
	  * @param 1 BPatch_Set that will be used in assignment
	  */
	DO_INLINE_F BPatch_Set<T,Compare>& operator= (const BPatch_Set<T,Compare>&);

	/** equality comparison for the BPatch_Set
	  * @param 1 BPatch_Set that will be used equality check
	  */
	DO_INLINE_F bool operator== (const BPatch_Set<T,Compare>&) const;

	/** inequality comparison for the BPatch_Set
	  * @param 1 BPatch_Set that will be used inequality check
	  */
	DO_INLINE_F bool operator!= (const BPatch_Set<T,Compare>&) const;

	/** insertion in to the BPatch_Set 
	  * @param 1 element that will be inserted 
	  */
	DO_INLINE_F BPatch_Set<T,Compare>& operator+= (const T&);

	/** union operation with this BPatch_Set 
	  * @param 1 BPatch_Set that will be used in union operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare>& operator|= (const BPatch_Set<T,Compare>&);

	/** intersection operation with this BPatch_Set 
	  * @param 1 BPatch_Set that will be used in intersection operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare>& operator&= (const BPatch_Set<T,Compare>&);

	/** difference operation with this BPatch_Set 
	  * @param 1 BPatch_Set that will be used in difference operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare>& operator-= (const BPatch_Set<T,Compare>&);

	/** union operation 
	  * @param 1 BPatch_Set that will be used in union operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare> operator| (const BPatch_Set<T,Compare>&) const;

	/** intersection operation 
	  * @param 1 BPatch_Set that will be used in intersection operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare> operator& (const BPatch_Set<T,Compare>&) const;

	/** difference operation 
	  * @param 1 BPatch_Set that will be used in difference operation
	  */
	DO_INLINE_F BPatch_Set<T,Compare> operator- (const BPatch_Set<T,Compare>&) const;

	/** removes the element in the root of the tree 
	  * if the BPatch_Set is empty it return false
	  * @param e refernce to the element that the value of removed
	  * element will be copied.
	  */
	DO_INLINE_F bool extract(T&);

};

template <class T,class Compare>
DO_INLINE_P
BPatch_Set<T,Compare>::entry* BPatch_Set<T,Compare>::replicateTree(entry* node,entry* parent,
 				     entry* oldNil,entry* newNil)
{
	if(node == oldNil)
		return newNil;	

	entry* newNode = new entry(*node);
	newNode->parent = parent; 
	newNode->left = replicateTree(node->left,newNode,oldNil,newNil);
	newNode->right = replicateTree(node->right,newNode,oldNil,newNil);
	return newNode; 
}

template <class T,class Compare>
DO_INLINE_P
void BPatch_Set<T,Compare>::destroy(entry* node){
	if(!node || (node == nil))
		return;
	if(node->left != nil)
		destroy(node->left);
	if(node->right != nil)
		destroy(node->right);
	delete node;
}
	
template <class T,class Compare>
DO_INLINE_P
void BPatch_Set<T,Compare>::traverse(T* all,entry* node,int& n) const{
	if(node == nil)
		return;
	if(node->left != nil)
		traverse(all,node->left,n);
	if(all)
		all[n++] = node->data;
	if(node->right != nil)
		traverse(all,node->right,n);
}
template <class T,class Compare>
DO_INLINE_F T BPatch_Set<T,Compare>::minimum() const{
	if(setData == nil)
		return nil->data;
	entry* node = setData;
	while(node->left != nil)
		node = node->left;
	return node->data;
}

template <class T,class Compare>
DO_INLINE_F T BPatch_Set<T,Compare>::maximum() const{
	if(setData == nil)
		return nil->data;
	entry* node = setData;
	while(node->right != nil)
		node = node->right;
	return node->data;
}
template <class T,class Compare>
DO_INLINE_F T* BPatch_Set<T,Compare>::elements(T* buffer) const{
	if(setData == nil) return NULL;
	if(!buffer) return NULL;
	int tmp = 0;
	traverse(buffer,setData,tmp);	
	return buffer;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare>& BPatch_Set<T,Compare>::operator= (const BPatch_Set<T,Compare>& newBPatch_Set){
	if(this == &newBPatch_Set)
		return *this;
	destroy(setData);
	compareFunc = newBPatch_Set.compareFunc;
	setSize = newBPatch_Set.setSize;
	nil->parent = NULL;
	setData = replicateTree(newBPatch_Set.setData,NULL,newBPatch_Set.nil,nil);
	return *this;
}
template <class T,class Compare>
DO_INLINE_F bool BPatch_Set<T,Compare>::operator== (const BPatch_Set<T,Compare>& newBPatch_Set) const{
	int i;
	if(this == &newBPatch_Set)
		return true;
	T* all = new T[newBPatch_Set.size()];
	newBPatch_Set.elements(all);
	for(i=0;i<newBPatch_Set.size();i++)
		if(!contains(all[i]))
			return false;
	delete[] all;
	all = new T[setSize];
	elements(all);
	for(i=0;i<setSize;i++)
		if(!newBPatch_Set.contains(all[i]))
			return false;
	delete[] all;
	return true;
}
template <class T,class Compare>
DO_INLINE_F bool BPatch_Set<T,Compare>::operator!= (const BPatch_Set<T,Compare>& newBPatch_Set) const{
	return !(*this == newBPatch_Set);
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare>& BPatch_Set<T,Compare>::operator+= (const T& element){
	insert(element);
	return *this;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare>& BPatch_Set<T,Compare>::operator|= (const BPatch_Set<T,Compare>& newBPatch_Set){
	if(this == &newBPatch_Set)
		return *this;
	T* all = new T[newBPatch_Set.size()];
	newBPatch_Set.elements(all);
	for(int i=0;i<newBPatch_Set.size();i++)
		insert(all[i]);	
	delete[] all;
	return *this;
} 
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare>& BPatch_Set<T,Compare>::operator&= (const BPatch_Set<T,Compare>& newBPatch_Set){
	if(this == &newBPatch_Set)
		return *this;
	T* all = new T[setSize];
	elements(all);
	int s = setSize;
	for(int i=0;i<s;i++)
		if(!newBPatch_Set.contains(all[i]))
			remove(all[i]);
	delete[] all;
	return *this;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare>& BPatch_Set<T,Compare>::operator-= (const BPatch_Set<T,Compare>& newBPatch_Set){
	T* all = new T[setSize];
	elements(all);
	int s = setSize;
	for(int i=0;i<s;i++)
		if(newBPatch_Set.contains(all[i]))
			remove(all[i]);
	delete[] all;
	return *this;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare> BPatch_Set<T,Compare>::operator| (const BPatch_Set<T,Compare>& newBPatch_Set) const{
	BPatch_Set<T,Compare> ret;
	ret |= newBPatch_Set;
	ret |= *this;
	return ret;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare> BPatch_Set<T,Compare>::operator& (const BPatch_Set<T,Compare>& newBPatch_Set) const{
	BPatch_Set<T,Compare> ret;
	ret |= newBPatch_Set;
	ret &= *this;
	return ret;
}
template <class T,class Compare>
DO_INLINE_F BPatch_Set<T,Compare> BPatch_Set<T,Compare>::operator- (const BPatch_Set<T,Compare>& newBPatch_Set) const{
	BPatch_Set<T,Compare> ret;
	ret |= *this;
	ret -= newBPatch_Set;
	return ret;
}

template <class T,class Compare>
DO_INLINE_P
void BPatch_Set<T,Compare>::leftRotate(entry* pivot){
	if(!pivot || (pivot == nil))
		return;
	entry* y = pivot->right;
	if(y == nil)
		return;
	pivot->right = y->left;
	if(y->left != nil)
		y->left->parent = pivot;
	y->parent = pivot->parent;
	if(!pivot->parent)
		setData = y;
	else if(pivot == pivot->parent->left)
		pivot->parent->left = y;
	else
		pivot->parent->right = y;
	y->left = pivot;
	pivot->parent = y;
}
template <class T,class Compare>
DO_INLINE_P
void BPatch_Set<T,Compare>::rightRotate(entry* pivot){
	if(!pivot || (pivot == nil))
		return;
	entry* x = pivot->left;
	if(x == nil)
		return;
	pivot->left = x->right;
	if(x->right != nil)
		x->right->parent = pivot;
	x->parent = pivot->parent;
	if(!pivot->parent)
		setData = x;
	else if(pivot == pivot->parent->left)
		pivot->parent->left = x;
	else
		pivot->parent->right = x;
	x->right = pivot;
	pivot->parent = x;
}

template <class T,class Compare>
DO_INLINE_P
BPatch_Set<T,Compare>::entry* BPatch_Set<T,Compare>::find(const T& element) const{
	entry* x = setData;
	while(x != nil){
		int check = compareFunc(element,x->data);
		if(check < 0)
			x = x->left;
		else if(check > 0)
			x = x->right;
		else
			return x;
	}	
	return NULL;
}
template <class T,class Compare>
DO_INLINE_F bool BPatch_Set<T,Compare>::contains(const T& element) const{
	entry* x = setData;
	while(x != nil){
		int check = compareFunc(element,x->data);
		if(check < 0)
			x = x->left;
		else if(check > 0)
			x = x->right;
		else
			return true;
	}	
	return false;
}
/** return pointer if the node is inserted, returns NULL if thevalue is 
  * already there
  */
template <class T,class Compare>
DO_INLINE_P
BPatch_Set<T,Compare>::entry* BPatch_Set<T,Compare>::treeInsert(const T& element){
	entry* y = NULL;
	entry* x = setData;
	while(x != nil){
		y = x;
		int check = compareFunc(element,x->data);
		if(check < 0)
			x = x->left;
		else if(check > 0)
			x = x->right;
		else
			return NULL;
	}	
	entry* z = new entry(element,nil);
	z->parent = y;
	if(!y)
		setData = z;
	else {
		int check = compareFunc(element,y->data);
		if(check < 0)
			y->left = z;
		else if(check > 0)
			y->right = z;
	}
	setSize++;
	return z;
}
/** finds the minimum value node when x is being deleted */
template <class T,class Compare>
DO_INLINE_P
BPatch_Set<T,Compare>::entry* BPatch_Set<T,Compare>::treeSuccessor(entry* x) const{
	if(!x || (x == nil))
		return NULL;
	if(x->right != nil){
		entry* z = x->right;
		while(z->left != nil) z = z->left;
		return z;
	}
	entry* y = x->parent;
	while(y && (x == y->right)){
		x = y;
		y = y->parent;
	}
	return y;
}
template <class T,class Compare>
DO_INLINE_P
void BPatch_Set<T,Compare>::deleteFixup(entry* x){
	while((x != setData) && 
	      (x->color == BLACK))
	{
		if(x == x->parent->left){
			entry* w = x->parent->right;
			if(w->color == RED){
				w->color = BLACK;
				x->parent->color = RED;
				leftRotate(x->parent);
				w = x->parent->right;
			}
			if((w->left->color == BLACK) &&
			   (w->right->color == BLACK)){
				w->color = RED;
				x = x->parent;
			}
			else{
				if(w->right->color == BLACK){
					w->left->color = BLACK;
					w->color = RED;
					rightRotate(w);
					w = x->parent->right;
				}
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				leftRotate(x->parent);
				x = setData;
			}
		}
		else{
			entry* w = x->parent->left;
			if(w->color == RED){
				w->color = BLACK;
				x->parent->color = RED;
				rightRotate(x->parent);
				w = x->parent->left;
			}
			if((w->right->color == BLACK) &&
			   (w->left->color == BLACK)){
				w->color = RED;
				x = x->parent;
			}
			else{
				if(w->left->color == BLACK){
					w->right->color = BLACK;
					w->color = RED;
					leftRotate(w);
					w = x->parent->left;
				}
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rightRotate(x->parent);
				x = setData;
			}
		}
	}
	x->color = BLACK;
}

template <class T,class Compare>
DO_INLINE_F void BPatch_Set<T,Compare>::insert(const T& element){
	entry* x = treeInsert(element);
	if(!x) return;
	x->color = RED;
	while((x != setData) && (x->parent->color == RED)){
		if(x->parent == x->parent->parent->left){
			entry* y = x->parent->parent->right;
			if(y->color == RED){
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			}
			else{
				if(x == x->parent->right){
					x = x->parent;
					leftRotate(x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rightRotate(x->parent->parent);
			}
		}
		else{
			entry* y = x->parent->parent->left;
			if(y->color == RED){
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			}
			else{
				if(x == x->parent->left){
					x = x->parent;
					rightRotate(x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				leftRotate(x->parent->parent);
			}
		}
	}
	setData->color = BLACK;
}
template <class T,class Compare>
DO_INLINE_F void BPatch_Set<T,Compare>::remove(const T& element){
	entry* z = find(element);
	if(!z)
		return;
	entry* y=((z->left == nil)||(z->right == nil)) ? z : treeSuccessor(z);
	entry* x=(y->left != nil) ? y->left : y->right;
	x->parent = y->parent;
	if(!y->parent)
		setData = x;
	else if(y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;
	if(y != z)
		z->data = y->data;
	if(y->color == BLACK)
		deleteFixup(x);
	setSize--;
	delete y;
}
template <class T,class Compare>
DO_INLINE_F 
bool BPatch_Set<T,Compare>::extract(T& element){
	element = setData->data;
	if(setData == nil)
		return false;
	remove(element);
	return true;
}
#endif /* _BPatch_Set_h_ */

