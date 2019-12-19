/*
 * Kyocera KPSL filter for CUPS.
 *
 * Copyright 2015 by svolkov
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more information.
 */

//
// Created by svolkov on 29.09.15.
//

#ifndef RASTERTOKPSL_RASTERTOKPSL_H
#define RASTERTOKPSL_RASTERTOKPSL_H

#include <stdio.h>
#include <wchar.h>

/****** Common helper macros ***********************************************/

#if !defined(LOBYTE)
#define LOBYTE(w)           ((unsigned char)(w))
#endif
#if !defined(HIBYTE)
#define HIBYTE(w)           ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))
#endif

#if !defined(LOWORD)
#define LOWORD(d)           ((unsigned short)(d))
#endif
#if !defined(HIWORD)
#define HIWORD(d)           ((unsigned short)((((unsigned long)(d)) >> 16) & 0xFFFF))
#endif

#define LODWORD(q)          ((q).u.dwLowDword)
#define HIDWORD(q)          ((q).u.dwHighDword)

// <cups/language-private.h>
// declarations from full source cups/language-private.h
// not included in base osx system

/*
 * Macro for localized text...
 */

#  define _(x) x

extern void		_cupsLangPrintError(const char *prefix,
                                               const char *message);
extern int		_cupsLangPrintFilter(FILE *fp, const char *prefix,
                                               const char *message, ...)
        __attribute__ ((__format__ (__printf__, 3, 4)));

// end <cups/language-private.h>

/*
 * Prototypes...
 */

void	Setup(void);
void	StartPage(/* ppd_file_t *ppd,*/ cups_page_header2_t *header);
void	EndPage(int sectionEnd);
void	Shutdown(void);

void	CancelJob(int sig);
void	CompressData(unsigned char *line, unsigned length, unsigned plane, unsigned type);
void	OutputLine(cups_page_header2_t *header);

int     rastertokpsl(cups_raster_t *ras, const char *user, const char *title, int copies,
                     const char *options);

#endif //RASTERTOKPSL_RASTERTOKPSL_H
