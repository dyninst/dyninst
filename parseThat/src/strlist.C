#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "strlist.h"
#include "utils.h"

strlist *strlist_alloc()
{
    strlist *result = (strlist *)malloc(sizeof(strlist));
    if (result) memset(result, 0, sizeof(strlist));
    return result;
}

bool strlist_insert(strlist *list, const char *data)
{
    int result = -1;
    strlist_elm *prev = NULL;
    strlist_elm *curr = list->head;
    strlist_elm *elem;

    while (curr && result < 0) {
	result = strncmp(curr->data, data, strlen(curr->data));
	if (result == 0) return true;

	prev = curr;
	curr = curr->next;
    }

    elem = (strlist_elm *)malloc(sizeof(strlist_elm));
    if (!elem) return false;

    elem->data = strdup(data);
    if (!elem->data) {
	free(elem);
	return false;
    }

    if (!prev) {
	elem->next = list->head;
	list->head = elem;

    } else {
	elem->next = curr;
	prev->next = elem;
    }

    if (!curr)
	list->tail = elem;

    ++list->count;
    return true;
}

bool strlist_push_front(strlist *list, const char *data)
{
    strlist_elm *elem = (strlist_elm *)malloc(sizeof(strlist_elm));
    if (!elem) return false;

    elem->data = strdup(data);
    if (!elem->data) {
	free(elem);
	return false;
    }

    if (!list->head)
	list->tail = elem;

    elem->next = list->head;
    list->head = elem;

    ++list->count;
    return true;
}

bool strlist_pop_front(strlist *list)
{
    strlist_elm *elem = list->head;

    if (!elem) return false;

    list->head = list->head->next;
    if (list->tail == elem)
	list->tail = NULL;
    --list->count;

    free(elem->data);
    free(elem);
    return true;
}

bool strlist_push_back(strlist *list, const char *data)
{
    strlist_elm *elem = (strlist_elm *)malloc(sizeof(strlist_elm));
    if (!elem) return false;

    elem->data = strdup(data);
    elem->next = NULL;
    if (!elem->data) {
	free(elem);
	return false;
    }

    if (list->tail) {
	list->tail->next = elem;
    } else {
	list->head = elem;
    }
    list->tail = elem;

    ++list->count;
    return true;
}

bool strlist_pop_back(strlist *list)
{
    strlist_elm *prev = NULL;
    strlist_elm *curr = list->head;

    while (curr) {
	prev = curr;
	curr = curr->next;
    }

    if (!prev) {
	list->head = NULL;
	list->tail = NULL;

    } else {
	list->tail = prev;
	prev->next = NULL;
    }
    --list->count;

    free(curr->data);
    free(curr);
    return true;
}

char *strlist_get(strlist *list, unsigned i)
{
    strlist_elm *curr = list->head;
    if (i >= list->count) return NULL;

    while (curr && i != 0) {
	curr = curr->next;
	--i;
    }
    return (curr ? curr->data : NULL);
}

void strlist_clear(strlist *list)
{
    strlist_elm *prev = NULL;
    strlist_elm *curr = list->head;

    while (curr) {
	prev = curr;
	curr = curr->next;

	free(prev->data);
	free(prev);
    }
    memset(list, 0, sizeof(strlist));
}

#include <stdio.h>
bool strlist_cmp(strlist *a, strlist *b)
{
    if (a->count != b->count) return false;

    for (unsigned i = 0; i < a->count; ++i)
	if (0 != strcmp(strlist_get(a, i), strlist_get(b, i)))
	    return false;
    return true;
}

/* --------------------------------------------------------
 * Helper functions for file storage of strlists.  Strings
 * separated by ((char)255).
 */
#define ARG_SEPARATOR ((char)255)
strlist char2strlist(char *buf)
{
    char *data = NULL;
    char *curr = buf;
    char *next = NULL;

    strlist result = STRLIST_INITIALIZER;
    while (curr && *curr) {
	while (isspace(*curr)) ++curr;

	data = decodeStr(curr, &next);
	if (!strlist_push_back(&result, data)) {
	    strlist_clear(&result);
	    break;
	}
	curr = next;
    }
    return result;
}

char *strlist2char(strlist *list)
{
    char *data = strlist_get(list, 0);

    data = strcat_static(encodeStr(data), "");
    for (unsigned i = 1; i < list->count; ++i) {
	data = strcat_static(data, " ");
	data = strcat_static(data, encodeStr(strlist_get(list, i)));
    }
    return data;
}
