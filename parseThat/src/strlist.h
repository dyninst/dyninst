#ifndef __STRLIST_H__
#define __STRLIST_H__

struct strlist_elm {
    char *data;
    strlist_elm *next;
};

struct strlist {
    strlist_elm *head, *tail;
    unsigned count;
};

#define STRLIST_INITIALIZER { NULL, NULL, 0 }

strlist *strlist_alloc();
void strlist_clear(strlist *);

// Alpha-numeric sorted insert.
bool strlist_insert(strlist *, const char *);

// Queue/Stack implementation.
bool strlist_push_front(strlist *, const char *);
bool strlist_pop_front(strlist *);
bool strlist_push_back(strlist *, const char *);
bool strlist_pop_back(strlist *);

// Data Retrieval
char *strlist_get(strlist *, unsigned);

bool strlist_cmp(strlist *, strlist *);
strlist char2strlist(char *);
char *strlist2char(strlist *, char *);

#endif
