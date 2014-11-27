/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 4 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * What's new from version 3 to 4?
 * ===============================
 *	* Relative code block addressing
 *	* New instructions SIGN, PACK2, UNPACK2, PACK4, UNPACK4
 *	* Changed operand order for SEND instruction arguments: ( m x y -- )
 *
 */
#include "stdafx.h"

int Universe_Migrate_4to5(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	sprintf(errbuf, "%s: file version 4 no longer supported, sorry.", filename);
	return 0;
}
