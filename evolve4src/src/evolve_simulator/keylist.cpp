/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * HASH TABLE associating keywords with strings
 *
 */
#include "stdafx.h"

static int keylist_hash(const char *key)
{
	const char *p;
	char c;
	unsigned h=0, g;

	ASSERT( key != NULL );

	for(p=key; *p; p++) {
		c = tolower(*p);

		h = (h<<4) + c;
		if( g = h&0xf0000000 ) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return h % KEYLIST_HASH_TABLE_SIZE;
}

/*
 * Create an empty keylist structure.
 *
 */
KEYLIST *KEYLIST_Make(void)
{
	KEYLIST *klist;

	klist = (KEYLIST*) CALLOC(1, sizeof(KEYLIST));
	ASSERT( klist != NULL );

	klist->table = (KEYLIST_ITEM**) CALLOC(KEYLIST_HASH_TABLE_SIZE, sizeof(KEYLIST_ITEM*));
	ASSERT( klist->table != NULL );

	return klist;
}

void KEYLIST_Delete(KEYLIST *klist)
{
	int i;
	KEYLIST_ITEM *curr, *nxt;

	ASSERT( klist != NULL );

	for(i=0; i<KEYLIST_HASH_TABLE_SIZE; i++) {
		for(curr = klist->table[i]; curr; curr=nxt) {
			nxt = curr->next;
			FREE(curr->key);
			FREE(curr->value);
			FREE(curr);
		}
	}
	FREE(klist->table);
	FREE(klist);
}

void KEYLIST_Set(KEYLIST *klist, const char *key, const char *value)
{
	KEYLIST_ITEM *item;
	KEYLIST_ITEM *curr;
	int hval;

	ASSERT( klist != NULL );
	ASSERT( key != NULL );
	ASSERT( value != NULL );

	hval = keylist_hash(key);
	
	for(curr=klist->table[hval]; curr; curr=curr->next) {
		if( stricmp(key, curr->key) == 0 ) {
			FREE(curr->value);
			curr->value = STRDUP(value);
			return;
		}
	}

	item = (KEYLIST_ITEM*) CALLOC(1, sizeof(KEYLIST_ITEM));
	ASSERT( item != NULL );

	item->key = STRDUP(key);
	item->value = STRDUP(value);
	item->next = klist->table[hval];

	klist->table[hval] = item;

}

void KEYLIST_Get(KEYLIST *klist, const char *key, char *value)
{
	KEYLIST_ITEM *curr;
	int hval;

	ASSERT( klist != NULL );
	ASSERT( key != NULL );
	ASSERT( value != NULL );

	hval = keylist_hash(key);
	for(curr=klist->table[hval]; curr; curr=curr->next) {
		if( stricmp(key, curr->key) == 0 ) {
			strcpy(value, curr->value);
			return;
		}
	}

	strcpy(value, "");
}

