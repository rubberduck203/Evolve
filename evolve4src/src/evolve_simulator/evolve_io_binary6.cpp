/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 6 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * What's new from version 5 to 6?
 * ===============================
 *	* Added ROTATE instruction. Changed behavior of other instructions.
 *	no migration possible.
 *
 */
#include "stdafx.h"

int Universe_Migrate_6to7(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	sprintf(errbuf, "%s: file version 6 no longer supported, sorry.", filename);
	return 0;
}

