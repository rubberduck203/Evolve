#ifndef _UTILITY_H_
#define _UTILITY_H_

/*
 * Copyright (c) 1991 Ken Stauffer, All Rights Reserved
 */

/***********************************************************************
 * utility.h
 */

extern int		StoreUniverse(LPCTSTR lpszPathName, UNIVERSE *u, char *errbuf);
extern UNIVERSE		*LoadUniverse(LPCTSTR lpszPathName, char *errbuf);

extern UNIVERSE		*CreateUniverse(NewUniverseOptions *nuo, char *errbuf);

extern void		itoa_comma(char *buf, int value);
extern void		lltoa_comma(char *buf, LONG_LONG value);

extern int		atoi_comma(const char *buf);
extern LONG_LONG	atoll_comma(const char *buf);

extern void		DDX_CommaValue(CDataExchange *pDX, int nIDC, int& value);
extern void		DDX_CommaValue(CDataExchange *pDX, int nIDC, LONG_LONG& value);

extern void		DDX_RangeValue(CDataExchange *pDX, int nIDC, int& value,
								int minval, int maxval);

extern void		DDX_CommaValuePositive(CDataExchange *pDX, int nIDC, int& value);

extern void		DDX_MutationRate(CDataExchange *pDX, int nIDC, int& value);

extern void		DDX_ProgramFilename(CDataExchange *pDX, int nIDC,
				bool &sexonly,
				int &population,
				CString& filename);

extern bool		StringIsBlank(const char *str);

#endif
