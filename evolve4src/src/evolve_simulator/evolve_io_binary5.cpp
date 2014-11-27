/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 5 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * What's new from version 4 to 5?
 * ===============================
 *	* Added CB instruction (which invalidates older sim files)
 *
 */
#include "stdafx.h"

int Universe_Migrate_5to6(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	sprintf(errbuf, "%s: file version 5 no longer supported, sorry.", filename);
	return 0;
}
