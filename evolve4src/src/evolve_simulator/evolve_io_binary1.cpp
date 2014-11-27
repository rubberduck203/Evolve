/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY READ/WRITE operations. (Version 1 - no migration supported)
 *
 * This module reads/writes objects to files.
 * The format of files are "PHOTON ASCII".
 *
 * (THESE ROUTINES ARE NO LONGER BEING USED, THE FILE I/O
 * NO HAPPENS USING EVOLVE_IO_BINARY2.CPP)
 *
 * There is also NO migration capability for this old version, nothing
 * actually exists in this format anyway.
 *
 */
#include "stdafx.h"

int Universe_Migrate_1to2(const char *filename, char *errbuf)
{
	sprintf(errbuf, "%s: file version 1 no longer supported, sorry.", filename);
	return 0;
}
