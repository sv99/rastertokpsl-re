/*
 * Kyocera KPSL filter for CUPS.
 *
 * Copyright 2015 by svolkov
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more information.
 */

/*
 * Include necessary headers...
 */

#include <cups/cups.h>
#include <cups/raster.h>
//#include <cups/language-private.h>

#include <math.h>
#include <fcntl.h>
#include <signal.h>

#include "rastertokpsl.h"
#include "halfton.h"
#include "libjbig/jbig.h"
#include "unicode/ConvertUTF.h"

/*
 * Macros...
 */

#define LOBYTE(w)           ((unsigned char)(w))
#define HIBYTE(w)           ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

#define FORMAT_SHORT "%c%c"
#define FORMAT_INT "%c%c%c%c"
#define FORMAT_INT_START "%c%c%c%c@@@@"
#define FORMAT_INT_START_DOC "%c%c%c%c@@@@0100"
#define pwrite_short(n) printf(FORMAT_SHORT, LOBYTE((n)), HIBYTE((n)))
#define pwrite_int_f(f, n) printf((f), LOBYTE((n)), HIBYTE((n)), LOBYTE((n >> 16)), HIBYTE((n >> 16)))

#define pwrite_int(n) pwrite_int_f((FORMAT_INT), (n))
#define pwrite_int_start(n) pwrite_int_f((FORMAT_INT_START), (n))
#define pwrite_int_start_doc(n) pwrite_int_f((FORMAT_INT_START_DOC), (n))


/*
 * Globals...
 */

int vertFlag;
int endOfDataFlag;
int light[2];

unsigned Page;
int pages;
int pdfFlag;
cups_orient_t Orientation;
const char *paperSizeName;

int nup;

unsigned numVer;
unsigned numVertPacked;

unsigned char *nextLines;
unsigned iLineSize;
unsigned char *Lines;

//int skipFlag;
int insideBandCounter;

// StartPage
//unsigned printarea_x;
//unsigned printarea_y;
unsigned WidthInBytes;
unsigned iRealPlaneSize;
unsigned iPlaneSize;
unsigned iPlaneSize8;

// buffers
unsigned char *Planes;
unsigned char *Planes8;
unsigned char *OutBuffer;

//EndPage
//unsigned sectionEndFlag;

//SendPlanesData
unsigned y;                     /* Current line */
unsigned fWriteJBigHeader;
unsigned compressedLength;

int isBigEndian() {
        return (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
}

void Setup(void) { }

void StartPage(/*ppd_file_t *ppd,*/ cups_page_header2_t *header) {
        signed short orientation1, orientation2; // [sp+68h] [bp-18h]@4
        signed short pageSizeEnum; // [sp+78h] [bp-8h]@1

        pageSizeEnum = 0;
        switch ((int) header->Orientation) {
                case 5:
                        orientation1 = 1;
                        orientation2 = 1;
                        break;
                case 6:
                        orientation1 = 0;
                        orientation2 = 2;
                        break;
                case 4:
                        orientation1 = 1;
                        orientation2 = 3;
                        break;
                default:
                        orientation1 = 0;
                        orientation2 = 0;
                        break;
        }
        if (paperSizeName) {
                if (!strcmp(paperSizeName, "EnvMonarch")) {
                        pageSizeEnum = 1;
                }
                else if (!strcmp(paperSizeName, "Env10")) {
                        pageSizeEnum = 2;
                }
                else if (!strcmp(paperSizeName, "EnvDL")) {
                        pageSizeEnum = 3;
                }
                else if (!strcmp(paperSizeName, "EnvC5")) {
                        pageSizeEnum = 4;
                }
                else if (!strcmp(paperSizeName, "Executive")) {
                        pageSizeEnum = 5;
                }
                else if (!strcmp(paperSizeName, "Letter")) {
                        pageSizeEnum = 6;
                }
                else if (!strcmp(paperSizeName, "Legal")) {
                        pageSizeEnum = 7;
                }
                else if (!strcmp(paperSizeName, "A4")) {
                        pageSizeEnum = 8;
                }
                else if (!strcmp(paperSizeName, "B5")) {
                        pageSizeEnum = 9;
                }
                else if (!strcmp(paperSizeName, "A3")) {
                        pageSizeEnum = 10;
                }
                else if (!strcmp(paperSizeName, "B4")) {
                        pageSizeEnum = 11;
                }
                else if (!strcmp(paperSizeName, "Tabloid")) {
                        pageSizeEnum = 12;
                }
                else if (!strcmp(paperSizeName, "A5")) {
                        pageSizeEnum = 13;
                }
                else if (!strcmp(paperSizeName, "A6")) {
                        pageSizeEnum = 14;
                }
                else if (!strcmp(paperSizeName, "B6")) {
                        pageSizeEnum = 15;
                }
                else if (!strcmp(paperSizeName, "Env9")) {
                        pageSizeEnum = 16;
                }
                else if (!strcmp(paperSizeName, "EnvPersonal")) {
                        pageSizeEnum = 17;
                }
                else if (!strcmp(paperSizeName, "ISOB5")) {
                        pageSizeEnum = 18;
                }
                else if (!strcmp(paperSizeName, "EnvC4")) {
                        pageSizeEnum = 30;
                }
                else if (!strcmp(paperSizeName, "OficioII")) {
                        pageSizeEnum = 33;
                }
                else if (!strcmp(paperSizeName, "P16K")) {
                        pageSizeEnum = 40;
                }
                else if (!strcmp(paperSizeName, "Statement")) {
                        pageSizeEnum = 50;
                }
                else if (!strcmp(paperSizeName, "Folio")) {
                        pageSizeEnum = 51;
                }
                else if (!strcmp(paperSizeName, "OficioMX")) {
                        pageSizeEnum = 42;
                }
                else {
                        pageSizeEnum = 19;
                }
        }
        WidthInBytes = (unsigned) floor(32.0 * ceil((4 * ((header->cupsWidth + 31) >> 5)) / 32.0));
        //iLineSize = (unsigned) floor(header->cupsBytesPerLine / header->cupsBitsPerColor);
        iRealPlaneSize = WidthInBytes << 8;
        iPlaneSize = iRealPlaneSize;
        iPlaneSize8 = iPlaneSize * 8;
        fprintf(stderr, "INFO: StartPage()\n");
        fprintf(stderr, "INFO: cupsHeight=%d(0x%X)\n", header->cupsHeight, header->cupsHeight);
        fprintf(stderr, "INFO: cupsWidth=%d(0x%X) 0x%X\n", header->cupsWidth, header->cupsWidth, header->cupsWidth >> 3);
        fprintf(stderr, "INFO: WidthInBytes=%d(0x%X)\n", WidthInBytes, WidthInBytes);
        //fprintf(stderr, "INFO: iLineSize=%d(0x%X)\n", iLineSize, iLineSize);
        fprintf(stderr, "INFO: iRealPlaneSize=%d(0x%X)\n", iRealPlaneSize, iRealPlaneSize);
        fprintf(stderr, "INFO: iPlaneSize=%d(0x%X)\n", iPlaneSize, iPlaneSize);
        fprintf(stderr, "INFO: iPlaneSize8=%d(0x%X)\n", iPlaneSize8, iPlaneSize8);

        Planes = (unsigned char *) malloc(iPlaneSize);
        memset(Planes, 0, iPlaneSize);
        Planes8 = (unsigned char *) malloc(iPlaneSize8);
        memset(Planes8, 0, iPlaneSize8);
        Lines = (unsigned char *) malloc(8 * WidthInBytes);
        memset(Lines, 0, 8 * WidthInBytes);
        nextLines = (unsigned char *) malloc(8 * WidthInBytes);
        memset(nextLines, 0, 8 * WidthInBytes);
        OutBuffer = (unsigned char *) malloc(0x100000);
        memset(OutBuffer, 0, 0x100000);
        //if (!skipFlag) {
        printf("\x1B$0P");      // fwrite("\x1B$0P", 1, 4, fp);
        pwrite_int_start(3);    // fprintf(fp, "%c%c%c%c@@@@", 3, 0, 0, 0);
        pwrite_short(orientation1);      // fprintf(fp, "%c%c", LOBYTE(v15), HIBYTE(v15));
        pwrite_short(orientation2);      // fprintf(fp, "%c%c", LOBYTE(v16), HIBYTE(v16));
        unsigned short metricWidth = (unsigned short) floor(10.0 * (header->PageSize[0] * 0.352777778));
        unsigned short metricHeight = (unsigned short) floor(10.0 * (header->PageSize[1] * 0.352777778));
        fprintf(stderr, "INFO: metricWidth=%d\n", metricWidth);
        fprintf(stderr, "INFO: metricHeight=%d\n", metricHeight);
        pwrite_short(metricWidth);      // fprintf(fp, "%c%c", LOBYTE(v11), HIBYTE(v11));
        pwrite_short(metricHeight);      //fprintf(fp, "%c%c", LOBYTE(v12), HIBYTE(v12));
        pwrite_short(pageSizeEnum);      //fprintf(fp, "%c%c", LOBYTE(v17), HIBYTE(v17));
        pwrite_short(header->cupsMediaType); //fprintf(fp, "%c%c", LOBYTE(h->cupsMediaType), HIBYTE(h->cupsMediaType));
        //}
}


void EndPage(int sectionEnd) {
        fprintf(stderr, "INFO: EndPage()\n");
        //if (!skipFlag) {
        printf("\x1B$0F");      //fwrite("\x1B$0F", 1, 4, fp);
        pwrite_int_start(1);    //fprintf(fp, "%c%c%c%c@@@@", 1, 0, 0, 0);
        /* sectionEndFlag = 0;
        if (pdfFlag && endOfDataFlag) {
                sectionEndFlag = 1;
        }
        else if (pdfFlag || (Page % (unsigned) floor(ceil((float) pages / (float) nup)))) {
                if (!pdfFlag && endOfDataFlag)
                        sectionEndFlag = 1;
        }
        else {
                sectionEndFlag = 1;
        } */
        fprintf(stderr, "INFO: sectionEndFlag=%d\n", sectionEnd);
        pwrite_int( sectionEnd); // fprintf(fp, "%c%c%c%c", LOBYTE(sectionEndFlag), HIBYTE(sectionEndFlag), LOBYTE(sectionEndFlag >> 16), HIBYTE( sectionEndFlag >> 16));
        fflush(stdout);
        //        skipFlag = 0;
        //}
        free(Planes);
        //Planes = NULL;
        free(Planes8);
        //Planes8 = NULL;
        free(Lines);
        //Lines = NULL;
        free(nextLines);
        //nextLines = NULL;
        if (OutBuffer != 0) {
                free(OutBuffer);
                //OutBuffer = NULL;
        }
}

void Shutdown(void) {
        printf("%c%c", '\x1B', 'E');
}

void CancelJob(int sig) {
        for (int i = 0; i <= 599; ++i)
                putchar(0);
        EndPage(1);
        Shutdown();
        exit(0);
}

void OutputBie(unsigned char *start, size_t len, void *file) {
        if (fWriteJBigHeader && len == 20) {
                fWriteJBigHeader = 0;
        }
        else {
                size_t v3 = len;
                unsigned char *out = OutBuffer + compressedLength;
                unsigned char *in = start;
                while (v3) {
                        *out++ = *in++;
                        --v3;
                }
                compressedLength += len;
        }
}

void SendPlanesData(cups_page_header2_t *header) {
        int v26; // [sp+ECh] [bp-24h]@13
        unsigned int v27; // [sp+F0h] [bp-20h]@27

        if (header->cupsCompression) {
                memcpy(Planes8 + 8 * WidthInBytes * insideBandCounter, Lines, 8 * WidthInBytes);

                if ((y && insideBandCounter == 255) || (header->cupsHeight - 1 == y)) {
                        if (y && insideBandCounter == 255) {
                                HalftoneDibToDib(
                                        Planes8,
                                        Planes,
                                        8 * WidthInBytes,
                                        256,
                                        light[1],
                                        light[0]);
                        }
                        else if (header->cupsHeight - 1 == y) {
                                HalftoneDibToDib(
                                        Planes8,
                                        Planes,
                                        8 * WidthInBytes,
                                        numVer,
                                        light[1],
                                        light[0]);
                        }
                        fWriteJBigHeader = 1;
                        compressedLength = 0;
                        struct jbg_enc_state encState;
                        jbg_enc_init(&encState, 8 * WidthInBytes, numVer, 1, &Planes, OutputBie, stdout);
                        jbg_enc_layers(&encState, 0);
                        jbg_enc_options(&encState, 0, 0, 256, 0, 0);
                        jbg_enc_out(&encState);
                        jbg_enc_free(&encState);
                        v26 = 32 * (signed int) floor((compressedLength + 31) / 32.0);
                        if (iPlaneSize >= v26) {
                                printf("\x1B$0B");
                                pwrite_int_start(v26 / 4 + 13); //fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v23), HIBYTE(v23), LOBYTE(v23 >> 16), HIBYTE(v23 >> 16));
                                pwrite_int(1 << 16); //fprintf(fp, "%c%c%c%c", 0, 0, 1, 0);
                                pwrite_int(header->cupsWidth); //fprintf(fp, "%c%c%c%c", LOBYTE(printarea_x), HIBYTE(printarea_x), LOBYTE(printarea_x >> 16), HIBYTE(printarea_x >> 16));
                                pwrite_int(WidthInBytes); //fprintf(fp, "%c%c%c%c", LOBYTE(WidthInBytes), HIBYTE(WidthInBytes), LOBYTE(WidthInBytes >> 16), HIBYTE(WidthInBytes >> 16));
                                pwrite_int(numVer); //fprintf(fp, "%c%c%c%c", LOBYTE(numVer), HIBYTE(numVer), LOBYTE(numVer >> 16), HIBYTE(numVer >> 16));
                                pwrite_int(numVertPacked); //fprintf(fp, "%c%c%c%c", LOBYTE(numVertPacked), HIBYTE(numVertPacked), LOBYTE(numVertPacked >> 16), HIBYTE(numVertPacked >> 16));
                                pwrite_int(1 << 8); //fprintf(fp, "%c%c%c%c", 0, 1, 0, 0);
                                pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                                pwrite_int(compressedLength); //fprintf(fp, "%c%c%c%c", LOBYTE(compressedLength), HIBYTE(compressedLength), LOBYTE(compressedLength >> 16), HIBYTE(compressedLength >> 16));
                                pwrite_int(v26); //fprintf(fp, "%c%c%c%c", LOBYTE(v26), HIBYTE(v26), LOBYTE(v26 >> 16), HIBYTE(v26 >> 16));
                                pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                                pwrite_int(y - 255); //fprintf(fp, "%c%c%c%c", LOBYTE(v25), HIBYTE(v25), LOBYTE(v25 >> 16), HIBYTE(v25 >> 16));
                                pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                                pwrite_int(1); //fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
                                if (compressedLength & 0x1F)
                                        v27 = 32
                                              - (((LOBYTE(compressedLength) +
                                                   ((compressedLength >> 31) >> 27)) & 0x1F)
                                                 - ((compressedLength >> 31) >> 27));
                                else
                                        v27 = 0;
                                memset(OutBuffer + compressedLength, 0, v27);
                                fwrite(OutBuffer, 1, v27 + compressedLength, stdout);
                                memset(OutBuffer, 0, 0x100000);
                                memset(Planes, 0, numVer * WidthInBytes);
                                memset(Planes8, 0, 8 * numVer * WidthInBytes);
                                if (!vertFlag) {
                                        numVer = LOBYTE(header->cupsHeight + (header->cupsHeight >> 31 >> 24)) - (header->cupsHeight >> 31 >> 24);
                                        numVertPacked = 256;
                                        iRealPlaneSize = numVer * WidthInBytes;
                                        iPlaneSize = numVer * WidthInBytes;
                                        iPlaneSize8 = 8 * numVer * WidthInBytes;
                                }
                        }
                        else {
                                printf("\x1B$0R");
                                pwrite_int_start(iPlaneSize / 4 + 10); //fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v23), HIBYTE(v23), LOBYTE(v23 >> 16), HIBYTE(v23 >> 16));
                                pwrite_int(header->cupsWidth); //fprintf(fp, "%c%c%c%c", LOBYTE(printarea_x), HIBYTE(printarea_x), LOBYTE(printarea_x >> 16), HIBYTE(printarea_x >> 16));
                                pwrite_int(WidthInBytes); //fprintf(fp, "%c%c%c%c", LOBYTE(WidthInBytes), HIBYTE(WidthInBytes), LOBYTE(WidthInBytes >> 16), HIBYTE(WidthInBytes >> 16));
                                pwrite_int(numVer); //fprintf(fp, "%c%c%c%c", LOBYTE(numVer), HIBYTE(numVer), LOBYTE(numVer >> 16), HIBYTE(numVer >> 16));
                                pwrite_int(numVertPacked); //fprintf(fp, "%c%c%c%c", LOBYTE(numVertPacked), HIBYTE(numVertPacked), LOBYTE(numVertPacked >> 16), HIBYTE(numVertPacked >> 16));
                                pwrite_int(iRealPlaneSize); //fprintf(fp, "%c%c%c%c", LOBYTE(iRealPlaneSize), HIBYTE(iRealPlaneSize), LOBYTE(iRealPlaneSize >> 16), HIBYTE(iRealPlaneSize >> 16));
                                pwrite_int(iPlaneSize); //fprintf(fp, "%c%c%c%c", LOBYTE(iPlaneSize), HIBYTE(iPlaneSize), LOBYTE(iPlaneSize >> 16), HIBYTE(iPlaneSize >> 16));
                                pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                                pwrite_int(y - 255); //fprintf(fp, "%c%c%c%c", LOBYTE(v25), HIBYTE(v25), LOBYTE(v25 >> 16), HIBYTE(v25 >> 16));
                                pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                                pwrite_int(1); //fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
                                if (y && insideBandCounter == 255) {
                                        fwrite(Planes, 1, WidthInBytes << 8, stdout);
                                        memset(Planes, 0, WidthInBytes << 8);
                                        if (!vertFlag) {
                                                numVer = LOBYTE(header->cupsHeight + (header->cupsHeight >> 31 >> 24)) - (header->cupsHeight >> 31 >> 24);
                                                numVertPacked = 256;
                                                iRealPlaneSize = numVer * WidthInBytes;
                                                iPlaneSize = numVer * WidthInBytes;
                                                iPlaneSize8 = 8 * numVer * WidthInBytes;
                                        }
                                }
                                else {
                                        if (header->cupsHeight - 1 == y) {
                                                fwrite(Planes, 1, numVer * WidthInBytes, stdout);
                                                memset(Planes, 0, numVer * WidthInBytes);
                                                insideBandCounter = -1;
                                        }
                                }
                        }
                }
        }
        else {
                if ((y && insideBandCounter == 255) || (header->cupsHeight - 1 == y)) {
                        printf("\x1B$0R");
                        pwrite_int_start(iPlaneSize / 4 + 10); //fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v28), HIBYTE(v28), LOBYTE(v28 >> 16), HIBYTE(v28 >> 16));
                        pwrite_int(header->cupsWidth); //fprintf(fp, "%c%c%c%c", LOBYTE(printarea_x), HIBYTE(printarea_x), LOBYTE(printarea_x >> 16), HIBYTE(printarea_x >> 16));
                        pwrite_int(WidthInBytes); //fprintf(fp, "%c%c%c%c", LOBYTE(WidthInBytes), HIBYTE(WidthInBytes), LOBYTE(WidthInBytes >> 16), HIBYTE(WidthInBytes >> 16));
                        pwrite_int(numVer); //fprintf(fp, "%c%c%c%c", LOBYTE(numVer), HIBYTE(numVer), LOBYTE(numVer >> 16), HIBYTE(numVer >> 16));
                        pwrite_int(numVertPacked); //fprintf(fp, "%c%c%c%c", LOBYTE(numVertPacked), HIBYTE(numVertPacked), LOBYTE(numVertPacked >> 16), HIBYTE(numVertPacked >> 16));
                        pwrite_int(iRealPlaneSize); //fprintf(fp, "%c%c%c%c", LOBYTE(iRealPlaneSize), HIBYTE(iRealPlaneSize), LOBYTE(iRealPlaneSize >> 16), HIBYTE(iRealPlaneSize >> 16));
                        pwrite_int(iPlaneSize); //fprintf(fp, "%c%c%c%c", LOBYTE(iPlaneSize), HIBYTE(iPlaneSize), LOBYTE(iPlaneSize >> 16), HIBYTE(iPlaneSize >> 16));
                        pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                        pwrite_int(y - 255); //fprintf(fp, "%c%c%c%c", LOBYTE(v30), HIBYTE(v30), LOBYTE(v30 >> 16), HIBYTE(v30 >> 16));
                        pwrite_int(0); //fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                        pwrite_int(1); //fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
                }
                memcpy(Planes + (numVer - insideBandCounter - 1) * WidthInBytes,
                       Lines,
                       WidthInBytes);
                if (y && insideBandCounter == 255) {
                        fwrite(Planes, 1, WidthInBytes << 8, stdout);
                        memset(Planes, 0, WidthInBytes << 8);
                        if (!vertFlag) {
                                numVer = LOBYTE(header->cupsHeight + (header->cupsHeight >> 31 >> 24)) - (header->cupsHeight >> 31 >> 24);
                                numVertPacked = 256;
                                iRealPlaneSize = numVer * WidthInBytes;
                                iPlaneSize = numVer * WidthInBytes;
                                iPlaneSize8 = 8 * numVer * WidthInBytes;
                        }
                }
                else {
                        if (header->cupsHeight - 1 == y) {
                                fwrite(Planes, 1, numVer * WidthInBytes, stdout);
                                memset(Planes, 0, numVer * WidthInBytes);
                                insideBandCounter = -1;
                        }
                }
        }
}

/* original stupid implementation converter ascii -> utf16
void asciitounicode(uint16_t *dest, char *source) {
        uint16_t *v4 = dest;
        uint8_t *v3 = source;
        int swap = isBigEndian();
        while (*v3) {
                *v4 = *v3;
                if (swap)
                        *v4 = ((LOBYTE(*v4) << 8) | HIBYTE(*v4) >> 8);
                v3++;
                v4++;
        }
}
*/

char *timestring(char *out) {
        char buffer[14]; // [sp+16h] [bp-22h]@1
        time_t v3 = time(0);

        //struct tm *v4 = localtime(&v3);
        strftime((char *) &buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&v3));
        return strncpy(out, (char *) &buffer, sizeof(buffer));
}

/*
 * usage rastertopcl job-id user title copies options [raster_file]
 * cups_raster_t *ras;                Raster stream for printing
 */
int rastertokpsl(cups_raster_t *ras, const char *user, const char *title, int copies,
                     const char *opts) {
        cups_page_header2_t header;       /* Page header from file */
        //ppd_file_t *ppd;                /* PPD file - not used in the rastertokpsl anymore */

        sigset(SIGTERM, CancelJob);

        /*
         * Make sure status messages are not buffered...
         */

        setbuf(stderr, 0);

        int num_options = 0;
        cups_option_t *options = NULL;
        num_options = cupsParseOptions(opts, 0, &options);

        /*
         * Initialize the print device...
         */

        //ppd = ppdOpenFile(getenv("PPD"));

        /*
         * Process pages as needed...
         */

        Page = 0;

        while (cupsRasterReadHeader2(ras, &header)) {
        //do {
                //memset(&header, 0, sizeof(header));
                //endOfDataFlag = cupsRasterReadHeader2(ras, &header) == 0;

                const char *value = NULL;

                ++Page;

                vertFlag = 1;
                if (Page == 1) {

                        /*
                         * Setup job in the raster read circle - for setup needs data from header!
                         */

                        Setup();

                        printf("%c%c%c%c%c%c%c%c", 'L', 'S', 'P', 'K', '\x1B', '$', '0', 'J');
                        pwrite_int_start_doc('\r'); //fprintf(fp, "%c%c%c%c@@@@0100", '\r', '\0', '\0', '\0');

                        UTF16 buffer[64];
                        memset(&buffer, 0, sizeof(buffer));
                        UTF16 *pbuffer = (UTF16*)&buffer;
                        const UTF8 *parg = (UTF8*)user; //argv[2];
                        ConversionResult res =
                        ConvertUTF8toUTF16(&parg, parg + strlen(user),
                                           &pbuffer, pbuffer + sizeof(buffer),
                                           strictConversion);
                        //fprintf(stderr, "INFO: ConversionResult=%d\n", res);
                        //fprintf(stderr, "INFO: argv[2]=%s len=%d\n", argv[2], (int)strlen(argv[2]));
                        //char s_user[32];
                        //strcpy((char *) &s_user, argv[2]);
                        //wchar_t w_user[16];
                        //memset(&w_user, 0, sizeof(w_user));
                        //asciitounicode2((wchar_t *) &w_user, (char *) &s_user, 16);
                        fwrite(&buffer, 2, 16, stdout);

                        char buf_time[14];
                        timestring((char *) &buf_time);
                        fwrite(&buf_time, 1, sizeof(buf_time), stdout);
                        pwrite_short(0); //fprintf(fp, "%c%c", '\0', '\0');

                        value = cupsGetOption("CaBrightness", num_options, options);
                        if (value)
                                light[0] = -atoi(value);
                        else
                                light[0] = 0;
                        fprintf(stderr, "INFO: CaBrightness=%d\n", light[0]);
                        value = cupsGetOption("CaContrast", num_options, options);
                        if (value)
                                light[1] = atoi(value);
                        else
                                light[1] = 0;
                        fprintf(stderr, "INFO: CaContrast=%d\n", light[1]);

                        value = cupsGetOption("com.apple.print.PrintSettings.PMTotalBeginPages..n.",
                                              num_options, options);
                        if (value)
                                pages = atoi(value);
                        else
                                pdfFlag = 1;
                        fprintf(stderr, "INFO: pages=%d\n", pages);
                        fprintf(stderr, "INFO: pdfFlag=%d\n", pdfFlag);

                        //value = cupsGetOption("com.apple.print.PrintSettings.PMCopies..n.", num_options,
                        //                      options);
                        //if (value)
                        //        v44 = atoi(value);

                        /*
                         * N-Up printing places multiple document pages on a single printed page
                         * CUPS supports 1, 2, 4, 6, 9, and 16-Up formats; the default format is 1-Up
                         *   lp -o number-up=2 filename
                         */

                        int nup_col = 0;
                        int nup_row = 0;
                        value = cupsGetOption("com.apple.print.PrintSettings.PMLayoutColumns..n.", num_options,
                                              options);
                        if (value)
                                nup_col = atoi(value);

                        value = cupsGetOption("com.apple.print.PrintSettings.PMLayoutRows..n.", num_options,
                                              options);
                        if (value)
                                nup_row = atoi(value);

                        nup = nup_row * nup_col;
                        if (nup == 0)
                                nup = 1;

                        fprintf(stderr, "INFO: PMLayoutColumns=%d\n", nup_col);
                        fprintf(stderr, "INFO: PMLayoutRows=%d\n", nup_row);
                        fprintf(stderr, "INFO: nup=%d\n", nup);

                        //char s_title[64];
                        //strcpy((char *) &s_title, argv[3]);
                        //uint16_t w_title[32];
                        //memset(&w_title, 0, sizeof(w_title));
                        //asciitounicode2((uint16_t *) &w_title, (char *) &s_title, 32);
                        printf("\x1B$0D");
                        pwrite_int_start(16); //fprintf(fp, "%c%c%c%c@@@@", 16, 0, 0, 0);

                        memset(&buffer, 0, sizeof(buffer));
                        pbuffer = (UTF16*)&buffer;
                        parg = (UTF8*)title;
                        //fprintf(stderr, "INFO: argv[3]=%s len=%d\n", argv[3], (int)strlen(argv[3]));
                        res = ConvertUTF8toUTF16(&parg, parg + strlen(title), &pbuffer, pbuffer + sizeof(buffer),
                                                   lenientConversion);
                        //fprintf(stderr, "INFO: ConversionResult=%d\n", res);
                        fwrite(&buffer, 2, 0x20, stdout);

                        /*
                         * Multiple Copies, normally not collated
                         *   lp -n num-copies -o Collate=True filename
                         */

                        int collate = 0;
                        //int copies = atoi(argv[4]);
                        if (strstr(opts, " collate")) {
                                collate = 1;
                                copies = 1;
                        }
                        printf("\x1B$0C");
                        pwrite_int_start(1); //fprintf(fp, "%c%c%c%c@@@@", 1, 0, 0, 0);
                        pwrite_short(copies); //fprintf(fp, "%c%c", LOBYTE(i_arg4), HIBYTE(i_arg4));
                        pwrite_short(collate); //fprintf(fp, "%c%c", LOBYTE(v35), HIBYTE(v35));

                        printf("\x1B$0S");
                        pwrite_int_start(2); //fprintf(fp, "%c%c%c%c@@@@", 2, 0, 0, 0);
                        pwrite_short(
                                header.MediaPosition); //fprintf(fp, "%c%c", LOBYTE(header.MediaPosition), HIBYTE(header.MediaPosition));
                        int duplex = 0;
                        if (header.Duplex)
                                duplex = header.Tumble + header.Duplex;
                        else
                                duplex = 0;
                        value = cupsGetOption("com.apple.print.PrintSettings.PMDuplexing..n.", num_options,
                                              options);
                        if (value)
                                duplex = atoi(value) - 1;
                        pwrite_short(duplex); //fprintf(fp, "%c%c", LOBYTE(v37), HIBYTE(v37));
                        value = cupsGetOption("Feeding", num_options, options);
                        int feeding = value && !strcmp(value, "On");
                        pwrite_short(feeding); //fprintf(fp, "%c%c", LOBYTE(v38), HIBYTE(v38));
                        value = cupsGetOption("EngineSpeed", num_options, options);
                        int engine_speed = value && !strcmp(value, "On");
                        pwrite_short(engine_speed); //fprintf(fp, "%c%c", LOBYTE(v39), HIBYTE(v39));

                        fprintf(stderr, "INFO: Duplex=%d\n", duplex);
                        fprintf(stderr, "INFO: Feeding=%d\n", feeding);
                        fprintf(stderr, "INFO: EngineSpeed=%d\n", engine_speed);

                        int w_resolution = 600;
                        int h_resolution = 600;
                        printf("\x1B$0G");
                        pwrite_int_start(3); //fprintf(fp, "%c%c%c%c@@@@", 3, 0, 0, 0);
                        value = cupsGetOption("Resolution", num_options, options);
                        if (value && !strcmp(value, "300dpi")) {
                                h_resolution = 300;
                                w_resolution = 300;
                        }
                        pwrite_short(w_resolution); //fprintf(fp, "%c%c", LOBYTE(v40), HIBYTE(v40));
                        pwrite_short(h_resolution); //fprintf(fp, "%c%c", LOBYTE(v41), HIBYTE(v41));
                        pwrite_short(1); //fprintf(fp, "%c%c", LOBYTE(v42), HIBYTE(v42));
                        pwrite_short(1); //fprintf(fp, "%c%c", LOBYTE(v43), HIBYTE(v43));
                        pwrite_short(32); //fprintf(fp, "%c%c", ' ', 0);
                        pwrite_short(1 << 8); //fprintf(fp, "%c%c", 0, 1);

                        paperSizeName = cupsGetOption("PageSize", num_options, options);
                        if (!paperSizeName)
                                paperSizeName = cupsGetOption("media", num_options, options);

                        value = cupsGetOption("orientation-requested", num_options, options);
                        if (value)
                                Orientation = (cups_orient_t) atoi(value);
                        else
                                Orientation = (cups_orient_t) 0;
                }
                if (Page > 1)
                        // Not last page
                        EndPage(0);

                header.cupsBitsPerColor = 1;
                header.cupsCompression = 1;
                header.Orientation = Orientation;

                /*
                 * Start the page...
                 */

                StartPage(/*ppd,*/ &header);

                //band = header.cupsHeight;
                numVer = 256;
                numVertPacked = 256;

                /*
                 * Loop for each line on the page...
                 */

                for (y = 0; y < header.cupsHeight; ++y) {
                /*        v45 = cupsRasterReadPixels(ras, nextLines, iLineSize);
                        insideBandCounter = LOBYTE(y + (y >> 31 >> 24)) - (y >> 31 >> 24);
                        if (vertFlag && band - y <= 0xFF)
                                vertFlag = 0;
                        memcpy(Lines, nextLines, v45);
                        if (!skipFlag)
                                SendPlanesData(header.cupsCompression);
                        v45 = cupsRasterReadPixels(ras, nextLines, iLineSize);
                        if (!v45 && !cupsRasterReadHeader2(ras, &header)) {
                                endOfDataFlag = 1;
                                break;
                        }
                        */

                        /*
                         * Print progress...
                         */

                        if ((y & 0x3FF) == 0)
                        {
                                _cupsLangPrintFilter(stderr, "INFO",
                                                     _("Printing page %d, %u%% complete."),
                                                     Page, 100 * y / header.cupsHeight);
                                fprintf(stderr, "ATTR: job-media-progress=%u\n",
                                        100 * y / header.cupsHeight);
                        }

                        /*
                         * Read a line of graphics...
                         */

                        if (cupsRasterReadPixels(ras, nextLines, header.cupsBytesPerLine) < 1)
                                break;

                        insideBandCounter = LOBYTE(y + (y >> 31 >> 24)) - (y >> 31 >> 24);
                        if (vertFlag && header.cupsHeight - y <= 0xFF)
                                vertFlag = 0;
                        //fprintf(stderr, "INFO: insideBandCounter=%d\n", insideBandCounter);
                        //fprintf(stderr, "INFO: vertFlag=%d\n", vertFlag);

                        /*
                         * Write it to the printer...
                         */

                        memcpy(Lines, nextLines, header.cupsBytesPerLine);
                        SendPlanesData(&header);
                }

/*
                EndPage();
                if ((pdfFlag || Page % (signed int) floor(ceil(((float) pages / (float) nup))))
                    && (!pdfFlag || !endOfDataFlag)) {
                        skipFlag = 0;
                }
                else if (skipFlag) {
                        if (Page != 1 && endOfDataFlag) {
                                printf("\x1B$0E");
                                pwrite_int_start(0); //fprintf(fp, "%c%c%c%c@@@@", 0, 0, 0, 0);
                        }
                        skipFlag = 0;
                }
                else {
                        if (endOfDataFlag) {
                                printf("\x1B$0E");
                                pwrite_int_start(0); //fprintf(fp, "%c%c%c%c@@@@", 0, 0, 0, 0);
                        }
                        if (Page != 1) {
                                if (v37) {
                                        if (nup == 0) {
                                                v9 = 0;
                                        } else {
                                                v9 = (signed int) floor(ceil(((float) pages / (float) nup)));
                                        }
                                        if ((((char) v9 + (v9 >> 31)) & 1) - (v9 >> 31) == 1) {
                                                skipFlag = 1;
                                                --Page;
                                        }
                                }
                        }
                }
                */
        }
        //while (!endOfDataFlag);

        // last page end
        EndPage(1);

        printf("\x1B$0E");
        pwrite_int_start(0); //fprintf(fp, "%c%c%c%c@@@@", 0, 0, 0, 0);

        /*
         * Shutdown the printer...
         */

        printf("\x1B$0T");
        pwrite_int_start(0); //printf("%c%c%c%c@@@@", 0, 0, 0, 0);

        return Page;
}
