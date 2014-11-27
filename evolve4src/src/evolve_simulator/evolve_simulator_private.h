#ifndef _EVOLVE_SIMULATOR_PRIVATE_H
#define _EVOLVE_SIMULATOR_PRIVATE_H

/*
 * Copyright (c) 2007 Stauffer Computer Consulting
 */

/***********************************************************************
 * evolve_simulator:
 *
 * Header file for the back-end evolve simulator.
 *
 * (Private stuff not needed by clients of the evolve_simulator library)
 *
 */
/*
 * DEFINES
 */

/* #define ASSERT(x)	assert(x) */
#define ASSERT(x)	_ASSERTE(x)

#define MALLOC(x)	malloc(x)
#define CALLOC(n,s)	calloc(n,s)
#define REALLOC(p,s)	realloc(p,s)
#define FREE(x)		free(x)
#define STRDUP(x)	_strdup(x)

/***********************************************************************
 * PROTOTYPES (private to this library)
 */

/*
 * random.cpp
 */
extern EVOLVE_RANDOM	*sim_random_make(unsigned long seed);
extern void 		sim_random_delete(EVOLVE_RANDOM *er);
extern long		sim_random(EVOLVE_RANDOM *er);


/*
 * evolve_io_ascii.cpp
 */
extern UNIVERSE		*Universe_ReadAscii(const char *filename, char *errbuf);
extern int		Universe_WriteAscii(UNIVERSE *u, const char *filename, char *errbuf);


/*
 * evolve_io_binary.cpp
 */
extern long	evbio_NTOH_INT32(long x);
extern long	evbio_HTON_INT32(long x);
extern int	evbio_fwrite_array_char(const unsigned char *carr, int count, FILE *fp);
extern int	evbio_fread_array_char(unsigned char *carr, int count, FILE *fp);
extern int	evbio_fwrite_array_int(int *iarr, int count, FILE *fp);
extern int	evbio_fread_array_int(int *iarr, int count, FILE *fp);
extern int	evbio_fwrite_int(int *i, FILE *fp);
extern int	evbio_fread_int(int *i, FILE *fp);
extern int	evbio_fwrite_long(long *l, FILE *fp);
extern int	evbio_fread_long(long *l, FILE *fp);
extern int	evbio_fwrite_array_long(long *larr, int count, FILE *fp);
extern int	evbio_fread_array_long(long *larr, int count, FILE *fp);
extern int	evbio_fwrite_unsigned_long(unsigned long *ul, FILE *fp);
extern int	evbio_fread_unsigned_long(unsigned long *ul, FILE *fp);
extern int	evbio_fwrite_array_LONG_LONG(LONG_LONG *llarr, int count, FILE *fp);
extern int	evbio_fread_array_LONG_LONG(LONG_LONG *llarr, int count, FILE *fp);
extern int	evbio_fwrite_LONG_LONG(LONG_LONG *ll, FILE *fp);
extern int	evbio_fread_LONG_LONG(LONG_LONG *ll, FILE *fp);
extern bool	evbio_write_keylist(FILE *fp, KEYLIST *keylist);
extern KEYLIST *evbio_read_keylist(FILE *fp);

/*
 * evolve_io_binary1.cpp
 */
extern int		Universe_Migrate_1to2(const char *filename, char *errbuf);

/*
 * evolve_io_binary2.cpp
 */
extern int		Universe_Migrate_2to3(const char *filename, char *errbuf);

/*
 * evolve_io_binary3.cpp
 */
extern int		Universe_Migrate_3to4(const char *filename, char *errbuf);

/*
 * evolve_io_binary4.cpp
 */
extern int		Universe_Migrate_4to5(const char *filename, char *errbuf);

/*
 * evolve_io_binary5.cpp
 */
extern int		Universe_Migrate_5to6(const char *filename, char *errbuf);

/*
 * evolve_io_binary6.cpp
 */
extern int		Universe_Migrate_6to7(const char *filename, char *errbuf);

/*
 * evolve_io_binary7.cpp
 */
extern int		Universe_Migrate_7to8(const char *filename, char *errbuf);
extern UNIVERSE		*Universe_ReadBinary7(const char *filename, char *errbuf);
extern int		Universe_WriteBinary7(UNIVERSE *u, const char *filename, char *errbuf);

/*
 * evolve_io_binary8.cpp
 */
extern int		Universe_Migrate_8to9(const char *filename, char *errbuf);
extern UNIVERSE		*Universe_ReadBinary8(const char *filename, char *errbuf);
extern int		Universe_WriteBinary8(UNIVERSE *u, const char *filename, char *errbuf);

/*
 * evolve_io_binary9.cpp
 */
extern UNIVERSE		*Universe_ReadBinary9(const char *filename, char *errbuf);
extern int		Universe_WriteBinary9(UNIVERSE *u, const char *filename, char *errbuf);


#endif
