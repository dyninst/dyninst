/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * list.h - list ADT
 *
 * $Log: List.h,v $
 * Revision 1.18  1994/07/11 23:00:57  jcargill
 * Fixed bug where added two lists with (+=) operator could result in
 * duplicate key entries
 *
 * Revision 1.17  1994/07/07  03:20:36  markc
 * Added removeAll function to list class.
 * Added machineType headers to specify pvm, cm5, ...
 *
 * Revision 1.16  1994/05/30  19:37:39  hollings
 * added pragma for external g++ functions.
 *
 * Revision 1.15  1994/03/11  21:01:23  hollings
 * Changed Boolean from int to char to match X11 convention.
 *
 * Revision 1.14  1994/02/25  00:25:57  hollings
 * added tunable constants.
 *
 * Revision 1.13  1994/02/24  07:05:28  markc
 * Man page for librpcUtil.a
 * Extended list class to provide map function.  rpcUtil supports internet domain
 * sockets.
 *
 * Revision 1.12  1994/02/10  23:08:21  hollings
 * Fixed list.h ++ function to work when a hash table has an element at
 * slot zero in the table.
 *
 * Removed unused fields in hist class.
 *
 * Revision 1.11  1994/02/09  22:37:09  hollings
 * Added print routines to list and hash table.
 *
 * Revision 1.10  1994/02/08  00:30:32  hollings
 * Make libutil more compatable with ATT CC.
 *
 * Revision 1.9  1994/02/03  23:30:43  hollings
 * changed listHash to a macro to work with g++ 2.5.2.
 *
 * Revision 1.8  1994/01/25  20:49:40  hollings
 * First real version of utility library.
 *
 * Revision 1.7  1994/01/19  20:46:17  hollings
 * guardef defn of true/false.
 *
 * Revision 1.6  1993/12/15  21:06:54  hollings
 * removed destructors.  Our current list semantics don't support auto
 * destruction of list comonents since list elements can be shared between
 * lists.
 *
 * Revision 1.5  1993/12/13  20:11:14  hollings
 * added destructor for List class.
 *
 * Revision 1.4  1993/10/19  15:31:52  hollings
 * assorted small fixes.
 *
 * Revision 1.3  1993/08/02  22:46:37  hollings
 * added remove which was missing.
 *
 * Revision 1.2  1993/07/01  17:02:36  hollings
 * ansi endif comments
 *
 * Revision 1.1  1993/05/07  20:21:15  hollings
 * Initial revision
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */
#ifndef LIST_H
#define LIST_H
#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>

typedef char Boolean;

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#pragma interface

#define ListHash(ptr, size) (((int)(ptr) % (int)(size)))

template <class Type> class List;
template <class Type> class StringList;

template <class Type> class ListItem {
    friend class List<Type>;
    friend class StringList<Type>;
    private:
	Type		data;
	void		*key;
	ListItem<Type>	*next;
};

template <class Type> class List {
    public:
	List() { head = NULL; current = NULL; }
	int  empty() { return (head == NULL);}
	friend ostream &operator<<(ostream&, List<Type>&);
	void print();
	void add(Type data, void *key);
	void add(Type data);
	Boolean addUnique(Type data);
	Boolean addUnique(Type data, void *key) {
	    Type temp;

	    temp = find(key);
	    if (!temp) {
		add(data, key);
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	}
	Type find(void *key);
	Boolean remove(void *key);
	void removeAll(int deleteItem=1);
	int count()	{
	    int c;
	    ListItem<Type> *curr;

	    for (curr=head,c=0; curr; curr=curr->next) c++;
	    return(c);
	}
	Type operator *() { 
	    if (head) 
		return(head->data); 
	    else
		return((Type) NULL);
	}
	void operator +=(List<Type> mergee) {
	    ListItem<Type> *curr;

	    for (curr=mergee.head; curr; curr=curr->next) {
		addUnique(curr->data, curr->key);
	    }
	}
	Type operator ++() { 
	    Type ret = (Type) NULL;
	    if (head) {
		ret = head->data;
		head = head->next; 
	    }
	    return(ret); 
	}
	void map (void (*map_function)(const Type item)) {
	  const ListItem<Type> *temp_ptr = 0;

	  if (!map_function) return;
	  for (temp_ptr = head; temp_ptr && temp_ptr->data; temp_ptr = temp_ptr->next)
	    map_function (temp_ptr->data);
	}
	void setCurrent() { current = head;}
	const Type getCurrent() { 
	  if (current) return ( (const Type) current->data);
	  else return 0;
	}
	const Type next() {
	  advanceCurrent();
	  return getCurrent();
	}
	void advanceCurrent() { if (current) current = current->next;}
    protected:
	ListItem<Type>	*head;
	ListItem<Type>  *current;
};

template <class Type> ostream &operator<<(ostream &os, List<Type> &data)
{
    List<Type> curr;

    for (curr= data; *curr; curr++) {
        os << *curr << endl;
    }
    return os;
}

//
// Warning this function should be replaced by the stream operator above, but
//   g++ is broken and won't create it.
// 
template <class Type> void List<Type>::print()
{
    ListItem<Type> *curr;

    for (curr=head; curr; curr=curr->next) {
        cout << (curr->data) << endl;
    }
}

template <class Type> void List<Type>::add(Type data, void *key)
{
    ListItem<Type> *ni;

    ni = new(ListItem<Type>);
    ni->data = data;
    ni->key = key;

    ni->next = head;
    head = ni;
}

template <class Type> void List<Type>::add(Type data) 
{ 
    add(data, (void *) data); 
}

template <class Type> Boolean List<Type>::addUnique(Type data) 
{ 
    return(addUnique(data, (void *) data)); 
}

template <class Type> void List<Type>::removeAll(int deleteItem)
{
   ListItem<Type> *curr, *next;

   if (deleteItem) {
       curr = head;
       while (curr) {
           next = curr->next;
           delete (curr);
           curr = next;
       }
   }

   head = 0;
   current = 0;
}

template <class Type> Boolean List<Type>::remove(void *key)
{
    ListItem<Type> *lag;
    ListItem<Type> *curr;

    for (curr=head, lag = NULL; curr; curr=curr->next) {
	if (curr->key == key) {
	    break;
	}
	lag = curr;
    }

    if (curr) {
	if (lag) {
	    lag->next = curr->next;
	} else {
	    head = curr->next;
	}
	// if the 'current' pointer is this element, advance it
	if (current == curr)
	  advanceCurrent();
	delete(curr);
	return(TRUE);
    } else {
	return(FALSE);
    }
}

template <class Type> Type List<Type>::find(void *data)
{
    ListItem<Type> *curr;

    for (curr=head; curr; curr=curr->next) {
	if (curr->key == data) {
	    return(curr->data);
	}
    }
    return((Type) 0);
}

template <class Type> class HTable {

    public:
	friend ostream &operator<<(ostream&, HTable<Type>&);
	void print();
	HTable(Type data) { (void) HTable(); add(data, (void *) data); }
	void add(Type data, void *key);
	HTable(); 
	Boolean addUnique(Type data, void *key) {
	    Type temp;

	    temp = find(key);
	    if (temp ) {
		return(0);
	    } else {
		add(data, key);
		return(1);
	    }
	}
	Type find(void *key);
	Boolean remove(void *key);
        HTable<Type> operator =(HTable<Type> arg) {
	    table = arg.table;
	    tableSize = arg.tableSize;

	    // find the first item.
	    currHid = -1;
	    (*this)++;
	    return(*this);
	}
        Type operator *() {
            return(*currList);
        }
	Type operator ++() {
	    Type curr;

	    currList++;
            curr = *currList;
            if (curr) return(curr);
            for (currHid++; currHid < tableSize; currHid++) {
                if (table[currHid]) {
                    currList = *table[currHid];
                    curr = *currList;
                    if (curr) return(curr);
                }
	    }
	    return(NULL);
	}
	int count()	{
	    int i, total;

	    total = 0;
	    for (i=0; i < tableSize; i++) {
		if (table[i]) {
		    total += table[i]->count();
		}
	    }
	    return(total);
	}
    private:
	List<Type> **table;
	List<Type> currList;
	int currHid;
	int tableSize;
};

template <class Type> ostream &operator<<(ostream &os, HTable<Type> &data)
{
    int i, total;

    total = 0;
    for (i=0; i < data.tableSize; i++) {
	if (data.table[i]) {
            os << *data.table[i];
	}
    }
    return os;
}


//
// Warning this function should be replaced by the stream operator above, but
//   g++ is broken and won't create it.
// 
template <class Type> void HTable<Type>::print()
{
    int i, total;

    total = 0;
    for (i=0; i < tableSize; i++) {
	if (table[i]) {
            table[i]->print();
	}
    }
}

template <class Type> HTable<Type>::HTable()
{ 
    table = NULL;
    currHid = 0;
    tableSize = 0;
}

template <class Type> Type HTable<Type>::find(void *key)
{
    int hid;

    if (!tableSize) return(NULL);
    hid = ListHash(key, tableSize);
    if (!table[hid]) {
	return(NULL);
    } else {
	return(table[hid]->find(key));
    }
}

template <class Type> void HTable<Type>::add(Type data, void *key)
{
    int hid;

    if (!tableSize) {
	tableSize = 97;
	table = (List<Type>**) calloc(tableSize, sizeof(List<Type>*));
    }
    hid = ListHash(key, tableSize);
    if (!table[hid]) {
	table[hid] = new(List<Type>);
    }
    table[hid]->add(data, key);
}


template <class Type> Boolean HTable<Type>::remove(void *key)
{
    int hid;

    hid = ListHash(key, tableSize);
    if (table[hid]) {
	return(table[hid]->remove(key));
    }
    return(FALSE);
}

template <class Type> class StringList: public List<Type> {
    public:
	Type find(void *key);
};

template <class Type> Type StringList<Type>::find(void *data) 
{
    ListItem<Type> *curr;

    for (curr=head; curr; curr=curr->next) {
	if (!strcmp((char *) curr->key, (char *) data)) {
	    return(curr->data);
	}
    }
    return((Type) 0);
}

#endif /* LIST_H */
