/*
 * Copyright (c) 2007 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2007 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * GENERIC EVOLVE BINARY I/O ROUTINES (evbio_xxxxxx)
 *
 * These routines are used by the evolve_io_binaryN.cpp modules
 *
 *
 */
#include "stdafx.h"

long evbio_NTOH_INT32(long x)
{
	/* Convert Network-to-Host */

	/* on non-intel architectures, this may have to be modified */

	return x;
}

long evbio_HTON_INT32(long x)
{
	/* Convert Host-to-Network */

	/* on non-intel architectures, this may have to be modified */

	return x;
}

int evbio_fwrite_array_char(const unsigned char *carr, int count, FILE *fp)
{
	int n;

	ASSERT( carr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	n = (int) fwrite(carr, sizeof(unsigned char), count, fp);
	return n;
}

int evbio_fread_array_char(unsigned char *carr, int count, FILE *fp)
{
	int n;

	ASSERT( carr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	n = (int) fread(carr, sizeof(unsigned char), count, fp);
	return n;
}

int evbio_fwrite_array_int(int *iarr, int count, FILE *fp)
{
	int i, n, cnt;
	long portable_int32;

	ASSERT( iarr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	cnt = 0;
	for(i=0; i<count; i++) {
		portable_int32 = evbio_HTON_INT32(iarr[i]);

		n = (int) fwrite(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		cnt += 1;
	}

	return cnt;
}

int evbio_fread_array_int(int *iarr, int count, FILE *fp)
{
	int i, n, cnt;
	long portable_int32;

	ASSERT( iarr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	cnt = 0;
	for(i=0; i<count; i++) {
		n = (int) fread(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		iarr[i] = evbio_NTOH_INT32(portable_int32);

		cnt += 1;
	}

	return cnt;
}

int evbio_fwrite_int(int *i, FILE *fp)
{
	ASSERT( i != NULL );
	ASSERT( fp != NULL );

	return evbio_fwrite_array_int(i, 1, fp);
}

int evbio_fread_int(int *i, FILE *fp)
{
	ASSERT( i != NULL );
	ASSERT( fp != NULL );

	return evbio_fread_array_int(i, 1, fp);
}

int evbio_fwrite_long(long *l, FILE *fp)
{
	ASSERT( l != NULL );
	ASSERT( fp != NULL );

	return evbio_fwrite_int((int*)l, fp);
}

int evbio_fread_long(long *l, FILE *fp)
{
	ASSERT( l != NULL );
	ASSERT( fp != NULL );

	return evbio_fread_int((int*)l, fp);
}

int evbio_fwrite_array_long(long *larr, int count, FILE *fp)
{
	ASSERT( larr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	return evbio_fwrite_array_int((int*)larr, count, fp);
}

int evbio_fread_array_long(long *larr, int count, FILE *fp)
{
	ASSERT( larr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	return evbio_fread_array_int((int*)larr, count, fp);
}

int evbio_fwrite_unsigned_long(unsigned long *ul, FILE *fp)
{
	ASSERT( ul != NULL );
	ASSERT( fp != NULL );

	return evbio_fwrite_int((int*)ul, fp);
}

int evbio_fread_unsigned_long(unsigned long *ul, FILE *fp)
{
	ASSERT( ul != NULL );
	ASSERT( fp != NULL );

	return evbio_fread_int((int*)ul, fp);
}

int evbio_fwrite_array_LONG_LONG(LONG_LONG *llarr, int count, FILE *fp)
{
	int i, n, cnt;
	LONG_LONG ll;
	long portable_int32, *lp1, *lp2;

	ASSERT( llarr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	cnt = 0;
	for(i=0; i<count; i++) {
		ll = llarr[i];

		lp1 = (long*) ( (char*)&ll + 0);
		lp2 = (long*) ( (char*)&ll + 4);

		portable_int32 = evbio_HTON_INT32(*lp1);
		n = (int) fwrite(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		portable_int32 = evbio_HTON_INT32(*lp2);
		n = (int) fwrite(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		cnt += 1;
	}

	return cnt;
}

int evbio_fread_array_LONG_LONG(LONG_LONG *llarr, int count, FILE *fp)
{
	int i, n, cnt;
	LONG_LONG ll;
	long portable_int32, *lp1, *lp2;

	ASSERT( llarr != NULL );
	ASSERT( count >= 0 );
	ASSERT( fp != NULL );

	cnt = 0;
	for(i=0; i<count; i++) {
		lp1 = (long*) ( (char*)&ll + 0);
		lp2 = (long*) ( (char*)&ll + 4);

		n = (int) fread(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		*lp1 = evbio_NTOH_INT32(portable_int32);

		n = (int) fread(&portable_int32, sizeof(portable_int32), 1, fp);
		if( n != 1 )
			break;

		*lp2 = evbio_NTOH_INT32(portable_int32);

		llarr[i] = ll;

		cnt += 1;
	}

	return cnt;
}

int evbio_fwrite_LONG_LONG(LONG_LONG *ll, FILE *fp)
{
	ASSERT( ll != NULL );
	ASSERT( fp != NULL );

	return evbio_fwrite_array_LONG_LONG(ll, 1, fp);
}

int evbio_fread_LONG_LONG(LONG_LONG *ll, FILE *fp)
{
	ASSERT( ll != NULL );
	ASSERT( fp != NULL );

	return evbio_fread_array_LONG_LONG(ll, 1, fp);
}


bool evbio_write_keylist(FILE *fp, KEYLIST *keylist)
{
	int success;
	int count, i, len;
	KEYLIST_ITEM *curr;

	ASSERT( fp != NULL );
	ASSERT( keylist != NULL );

	count = 0;
	for(i=0; i < KEYLIST_HASH_TABLE_SIZE; i++) {
		if( keylist->table[i] == NULL )
			continue;

		for(curr=keylist->table[i]; curr; curr=curr->next) {
			count += 1;
		}
	}

	success = evbio_fwrite_int(&count, fp);
	if( ! success )
		return false;

	for(i=0; i < KEYLIST_HASH_TABLE_SIZE; i++) {
		if( keylist->table[i] == NULL )
			continue;

		for(curr=keylist->table[i]; curr; curr=curr->next) {

			len = (int)strlen(curr->key) + 1;
			success = evbio_fwrite_int(&len, fp);
			if( ! success )
				return false;

			success = evbio_fwrite_array_char((unsigned char*)curr->key, len, fp);
			if( ! success )
				return false;

			len = (int)strlen(curr->value) + 1;
			success = evbio_fwrite_int(&len, fp);
			if( ! success )
				return false;

			success = evbio_fwrite_array_char((unsigned char*)curr->value, len, fp);
			if( ! success )
				return false;
		}
	}

	return true;
}

KEYLIST *evbio_read_keylist(FILE *fp)
{
	char key[ 5000 ];
	char value[ 5000 ];
	int success;
	int count, i, len;
	KEYLIST *keylist;

	ASSERT( fp != NULL );

	keylist = KEYLIST_Make();

	ASSERT( keylist != NULL );

	success = evbio_fread_int(&count, fp);
	if( ! success ) {
		KEYLIST_Delete(keylist);
		return NULL;
	}

	for(i=0; i < count; i++) {
		success = evbio_fread_int(&len, fp);
		if( ! success ) {
			KEYLIST_Delete(keylist);
			return NULL;
		}

		success = evbio_fread_array_char((unsigned char*)key, len, fp);
		if( ! success ) {
			KEYLIST_Delete(keylist);
			return NULL;
		}

		success = evbio_fread_int(&len, fp);
		if( ! success ) {
			KEYLIST_Delete(keylist);
			return NULL;
		}

		success = evbio_fread_array_char((unsigned char*)value, len, fp);
		if( ! success ) {
			KEYLIST_Delete(keylist);
			return NULL;
		}

		KEYLIST_Set(keylist, key, value);
	}

	return keylist;
}

