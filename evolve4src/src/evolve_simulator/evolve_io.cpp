/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * READ/WRITE operations
 *
 * This module determines what version the file is in, and
 * tries to migrate to the latest.
 *
 */
#include "stdafx.h"

/*
 * 'xx' will be the actual version number like 01, 02, 03, .... 99
 */
#define EVOLVE_BINARY_MAGIC	"EVOLVE_BINARY_FORMAT:Vxx     "
#define EVOLVE_BINARY_MAGIC_LEN	30

/*
 * Return a non-zero version number. If 'filename' does not
 * contain a valid magic number header, then return 0 and set
 * errbuf to an error message.
 *
 */
static int Universe_GetBinaryVersion(const char *filename, char *errbuf)
{
	char buf[ 1000 ];
	char version_string[ 10 ];
	int version;
	int n;
	FILE *fp;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	fp = fopen(filename, "rb");
	if( fp == NULL ) {
		sprintf(errbuf, "%s: %s", filename, strerror(errno));
		return 0;
	}

	n = (int) fread(buf, sizeof(char), EVOLVE_BINARY_MAGIC_LEN, fp);
	if( n != EVOLVE_BINARY_MAGIC_LEN ) {
		sprintf(errbuf, "%s: cannot read %d bytes to get file info",
				filename, EVOLVE_BINARY_MAGIC_LEN);
		return 0;
	}

	fclose(fp);

	if( strnicmp(buf, EVOLVE_BINARY_MAGIC, 22) != 0 ) {
		sprintf(errbuf, "%s: file is not a EVOLVE binary file", filename);
		return 0;
	}

	/*
	 * magic number is correct, now extract 2 digit version code
	 */
	strncpy(version_string, buf+22, 2);
	version_string[2] = '\0';

	version = atoi(version_string);

	if( version == 0 ) {
		sprintf(errbuf, "%s: bogus version code 0.", filename);
		return 0;
	}

	return version;
}

/*
 * Migrate 'filename' to the latest. Obviously 'filename' must
 * be writeable. We first fetch the version number and
 * begin migration with that version all the way to the latest
 * version.
 *
 * This assumes the sim file is writeable.
 *
 */
static int Universe_Migrate(const char *filename, char *errbuf)
{
	int result;
	int version;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	version = Universe_GetBinaryVersion(filename, errbuf);
	if( version == 0 )
		return 0;

	/*
	 * IMPORANT: Fall-thru logic is used here.
	 *
	 * We migrate in small steps until the file is converted to the
	 * latest version.
	 *
	 */
	switch( version ) {
	case 1:
		result = Universe_Migrate_1to2(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 2:
		result = Universe_Migrate_2to3(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 3:
		result = Universe_Migrate_3to4(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 4:
		result = Universe_Migrate_4to5(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 5:
		result = Universe_Migrate_5to6(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 6:
		result = Universe_Migrate_6to7(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 7:
		result = Universe_Migrate_7to8(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 8:
		result = Universe_Migrate_8to9(filename, errbuf);
		if( ! result ) {
			return 0;
		}
		/* fall-thru */

	case 9:
		/* current version */
		break;

	default:
		sprintf(errbuf, "More recent .evolve file version %d is not readable by this program (9).",
					version);
		return 0;
	}

	return 1;
}

/*
 * Read binary, but first migrate the file to the latest
 * version (if needed).
 *
 */
static UNIVERSE *Universe_ReadBinary(const char *filename, char *errbuf)
{
	int result;

	result = Universe_Migrate(filename, errbuf);
	if( ! result )
		return NULL;

	return Universe_ReadBinary9(filename, errbuf);
}

static int Universe_WriteBinary(UNIVERSE *u, const char *filename, char *errbuf)
{
	return Universe_WriteBinary9(u, filename, errbuf);
}

/*
 * Top-level call to read ANY simulation version (or ascii/binary)
 */
UNIVERSE *Universe_Read(const char *filename, char *errbuf)
{
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	if( Phascii_FileIsPhotonAscii(filename) ) {
		return Universe_ReadAscii(filename, errbuf);
	} else {
		return Universe_ReadBinary(filename, errbuf);
	}
}

/*
 * Top-level call to write a simulation in the latest version
 * (will write to ascii format is the filename extension is .txt,
 * otherwise will write in binary format)
 */
int Universe_Write(UNIVERSE *u, const char *filename, char *errbuf)
{
	char *ext;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	ext = strrchr(filename, '.');

	if( ext != NULL && stricmp(ext, ".txt") == 0 ) {
		return Universe_WriteAscii(u, filename, errbuf);
	} else {
		return Universe_WriteBinary(u, filename, errbuf);
	}
}

