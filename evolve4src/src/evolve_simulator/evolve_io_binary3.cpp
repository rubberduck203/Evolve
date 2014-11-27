/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 3 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * Added keyword list to associated misc. application data
 * with a simulation file.
 *
 * Added a new field to the ORGANISM structure (int strain).
 * Added a new field to the SPORE structure (int strain).
 *
 */
#include "stdafx.h"

int Universe_Migrate_3to4(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	sprintf(errbuf, "%s: file version 3 no longer supported, sorry.", filename);
	return 0;
}
