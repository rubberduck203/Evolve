#ifndef _PHASCII_H
#define _PHASCII_H

/*
 * Copyright (c) 1994 Photon Systems, All Rights Reserved
 */
/***********************************************************************
 * Header file for the photon ascii routines.
 *
 * DOCUMENTATION:
 * A photon ascii file is a self describing file format.
 *
 * The first line must be a comment, with the word PHOTON and ASCII in them:
 *
 *	# PHOTON ASCII
 *
 * The preamble of the file describes 1 or more structure types.
 *
 * struct ORGANISM {
 *	ORGANISM_ID
 *	PROGRAM_FILENAME
 *	PROGRAM[N] {
 *		SOURCE_LINE
 *	}
 *	CELLS[N] {
 *		X
 *		Y
 *	}
 * }
 *
 * struct UNIVERSE {
 *	EON
 *	STEP
 *	SEED
 * }
 *
 * Once the structures have been defined, 1 or more instances can be specified:
 *
 * ORGANISM 100 "" { "blah" "blah" } { +0 +0 }
 *
 * All data is treated as strings.
 *
 * Multi-dimensional array are allowed:
 *
 * struct CRAP[A B] {
 *	COMP1
 *	COMP2 = "hello"
 *	COMP3
 * }
 *
 *
 * Using '=' inside of a structure definition will force a default value for that field for
 * all instances. When specifying the instance that field must be ignored:
 *
 * CRAP 2 2
 *	"comp1" "comp3"			# Where's COMP2? It was given a default value, so cannot
 *	"comp1" "comp3"			# be changed.
 *
 * Blanks and whitespace are only required to seperate things.
 *
 * Curly braces can be used to surround arrays when the size isn't known:
 *
 *  struct MYARRAY[N] {
 *	MYFIELD1
 *	MYFIELD2
 *  }
 *
 * MYARRAY {
 * 	"mydata1" "mydata2"
 * 	"mydata1" "mydata2"
 * 	"mydata1" "mydata2"
 * 	"mydata1" "mydata2"
 * 	"mydata1" "mydata2"
 * }
 *
 * Multiple dimensional arrays are supported. The '{'
 * only works for the FIRST dimension.
 *
 * struct MATRIX[W H] {
 *	VALUE TYPE
 * }
 *
 * MATRIX { 2
 *	1010 "X"	# w=0, h=0
 *	1020 "Y"	# w=0, h=1
 *
 *	2010 "X"	# w=1, h=0
 *	2020 "Y"	# w=1, h=1
 *
 *	3010 "X"	# w=2, h=0
 *	3020 "Y"	# w=2, h=1
 * }
 *
 * Comments use the hash mark '#'. The rest of the line is ignored.
 *
 * All data elements are treated as strings. Double quotes can be used
 * whenever you want. But they are required when the data item has spaces in them, or
 * other wierd characters.
 *
 *	0			# no quotes required
 *	-90.45			# no quotes required
 *	RED			# no quotes required
 *	True			# no quotes required
 *
 *	"Hello,World"		# required
 *	"blah blagh df sd"	# required
 *
 *
 * A routine Phascii_Makestring(char *str) exists that will examine
 * the item and convert it to a proper string.
 *
 * Strings can contain escape sequences:
 *	\"		double quote
 *	\\		back slash
 *	\a		bell
 *	\b		backspace
 *	\r		return
 *	\v		new page
 *	\n		newline
 *	\t		tab
 *
 *
 *
 */
#define	PHASCII_MAGIC	"# Photon Systems ASCII Format v1.0"

typedef void *PHASCII_FILE;
typedef void *PHASCII_INSTANCE;

enum {
	PHASCII_STRUCT, PHASCII_ARRAY, PHASCII_COMPONENT
};

typedef struct phascii_indexes {
	char	*name;
	struct	phascii_indexes *next;
} PHASCII_INDEXES;

typedef struct phascii_schema {
	int	type;
	char	*name;
	PHASCII_INDEXES *indexes;
	struct	phascii_schema *next;
} PHASCII_SCHEMA;

extern int		Phascii_FileIsPhotonAscii(const char *filename);
extern PHASCII_FILE	Phascii_Open(const char *filename, char *mode);
extern void		Phascii_Close(PHASCII_FILE phf);
extern int		Phascii_IsInstance(PHASCII_INSTANCE instance, char *name);
extern PHASCII_INSTANCE	Phascii_GetInstance(PHASCII_FILE phf);
extern void		Phascii_FreeInstance(PHASCII_INSTANCE ph_inst);
extern int		Phascii_Get(PHASCII_INSTANCE ph_entry, char *fmt, ...);
extern char		*Phascii_GetError(void);
extern char		*Phascii_Error(PHASCII_FILE phf);
extern int		Phascii_Eof(PHASCII_FILE phf);
extern char		*Phascii_MakeString(char *buf);
extern PHASCII_SCHEMA	*Phascii_GetSchema(PHASCII_INSTANCE ph_entry, char *expr);
extern void		Phascii_FreeSchema(PHASCII_SCHEMA *);

extern int		Phascii_Lineno(PHASCII_FILE phf);

#endif
