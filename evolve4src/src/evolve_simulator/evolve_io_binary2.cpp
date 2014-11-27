/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 2 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * This version hopefully will allow these binary files to
 * be readable/writable on linux as well as windows.
 *
 */
#include "stdafx.h"

int Universe_Migrate_2to3(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	sprintf(errbuf, "%s: file version 2 no longer supported, sorry.", filename);
	return 0;
}

