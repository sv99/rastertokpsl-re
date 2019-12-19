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

#include "string.h"
#include <stdio.h>
#include "halfton.h"

/*
 * Globals...
 */

// Transfer2
float m_fContrast;
float m_fBrightness;

// SetDitherGrayTable
unsigned char *m_pDitherTable;

unsigned m_DitherTableW;
unsigned m_DitherTableH;
int m_DitherTablePitch;


/*
static char DeviceBestDither[256] = {
        145, 185, 177, 137, 107, 67, 75, 115, 147, 187, 179,
        139, 105, 65, 73, 113, 193, 241, 233, 161, 59, 11, 19,
        91, 195, 243, 235, 163, 57, 9, 17, 89, 201, 249, 225,
        169, 51, 3, 27, 83, 203, 251, 227, 171, 49, 1, 25, 81,
        153, 217, 209, 129, 99, 35, 43, 123, 155, 219, 211,
        131, 97, 33, 41, 121, 108, 68, 76, 116, 148, 188, 180,
        140, 110, 70, 78, 118, 150, 190, 182, 142, 60, 12, 20,
        92, 196, 244, 236, 164, 62, 14, 22, 94, 198, 246, 238,
        166, 52, 4, 28, 84, 204, 252, 228, 172, 54, 6, 30, 86,
        206, 254, 230, 174, 100, 36, 44, 124, 156, 220, 212,
        132, 102, 38, 46, 126, 158, 222, 214, 134, 146, 186,
        178, 138, 104, 64, 72, 112, 144, 184, 176, 136, 106,
        66, 74, 114, 194, 242, 234, 162, 56, 8, 16, 88, 192,
        240, 232, 160, 58, 10, 18, 90, 202, 250, 226, 170, 48,
        0, 24, 80, 200, 248, 224, 168, 50, 2, 26, 82, 154, 218,
        210, 130, 96, 32, 40, 120, 152, 216, 208, 128, 98, 34,
        42, 122, 111, 71, 79, 119, 151, 191, 183, 143, 109,
        69, 77, 117, 149, 189, 181, 141, 63, 15, 23, 95, 199,
        247, 239, 167, 61, 13, 21, 93, 197, 245, 237, 165, 55,
        7, 31, 87, 207, 254, 231, 175, 53, 5, 29, 85, 205, 253,
        229, 173, 103, 39, 47, 127, 159, 223, 215, 135, 101,
        37, 45, 125, 157, 221, 213, 133
};*/

unsigned char Transfer2(unsigned char value, int contrast, int brightness) {

        if (contrast) {
                m_fContrast = contrast * 0.000024999999F * contrast
                                       + 0.0074999998F * contrast + 1.0F;
                float c = (value - 128) * m_fContrast + 128.0F;
                if (c < 0.0F)
                        value = 0;
                else if (c >= 255.0F)
                        value = 255;
                else
                        value = (unsigned char)c;

        }
        if (brightness) {
                m_fBrightness = (float)(0.0049999999 * brightness);
                float b = value + m_fBrightness * 255.0F;
                if (b < 0.0F)
                        value = 0;
                else if (b >= 255.0F)
                        value = 255;
                else
                        value = (unsigned char)b;
        }
        return value;
}

static unsigned char DeviceBestDither[256] = {
        0x91, 0xB9, 0xB1, 0x89, 0x6B, 0x43, 0x4B, 0x73, 0x93, 0xBB, 0xB3, 0x8B, 0x69, 0x41, 0x49, 0x71,
        0xC1, 0xF1, 0xE9, 0xA1, 0x3B, 0x0B, 0x13, 0x5B, 0xC3, 0xF3, 0xEB, 0xA3, 0x39, 0x09, 0x11, 0x59,
        0xC9, 0xF9, 0xE1, 0xA9, 0x33, 0x03, 0x1B, 0x53, 0xCB, 0xFB, 0xE3, 0xAB, 0x31, 0x01, 0x19, 0x51,
        0x99, 0xD9, 0xD1, 0x81, 0x63, 0x23, 0x2B, 0x7B, 0x9B, 0xDB, 0xD3, 0x83, 0x61, 0x21, 0x29, 0x79,
        0x6C, 0x44, 0x4C, 0x74, 0x94, 0xBC, 0xB4, 0x8C, 0x6E, 0x46, 0x4E, 0x76, 0x96, 0xBE, 0xB6, 0x8E,
        0x3C, 0x0C, 0x14, 0x5C, 0xC4, 0xF4, 0xEC, 0xA4, 0x3E, 0x0E, 0x16, 0x5E, 0xC6, 0xF6, 0xEE, 0xA6,
        0x34, 0x04, 0x1C, 0x54, 0xCC, 0xFC, 0xE4, 0xAC, 0x36, 0x06, 0x1E, 0x56, 0xCE, 0xFE, 0xE6, 0xAE,
        0x64, 0x24, 0x2C, 0x7C, 0x9C, 0xDC, 0xD4, 0x84, 0x66, 0x26, 0x2E, 0x7E, 0x9E, 0xDE, 0xD6, 0x86,
        0x92, 0xBA, 0xB2, 0x8A, 0x68, 0x40, 0x48, 0x70, 0x90, 0xB8, 0xB0, 0x88, 0x6A, 0x42, 0x4A, 0x72,
        0xC2, 0xF2, 0xEA, 0xA2, 0x38, 0x08, 0x10, 0x58, 0xC0, 0xF0, 0xE8, 0xA0, 0x3A, 0x0A, 0x12, 0x5A,
        0xCA, 0xFA, 0xE2, 0xAA, 0x30, 0x00, 0x18, 0x50, 0xC8, 0xF8, 0xE0, 0xA8, 0x32, 0x02, 0x1A, 0x52,
        0x9A, 0xDA, 0xD2, 0x82, 0x60, 0x20, 0x28, 0x78, 0x98, 0xD8, 0xD0, 0x80, 0x62, 0x22, 0x2A, 0x7A,
        0x6F, 0x47, 0x4F, 0x77, 0x97, 0xBF, 0xB7, 0x8F, 0x6D, 0x45, 0x4D, 0x75, 0x95, 0xBD, 0xB5, 0x8D,
        0x3F, 0x0F, 0x17, 0x5F, 0xC7, 0xF7, 0xEF, 0xA7, 0x3D, 0x0D, 0x15, 0x5D, 0xC5, 0xF5, 0xED, 0xA5,
        0x37, 0x07, 0x1F, 0x57, 0xCF, 0xFE, 0xE7, 0xAF, 0x35, 0x05, 0x1D, 0x55, 0xCD, 0xFD, 0xE5, 0xAD,
        0x67, 0x27, 0x2F, 0x7F, 0x9F, 0xDF, 0xD7, 0x87, 0x65, 0x25, 0x2D, 0x7D, 0x9D, 0xDD, 0xD5, 0x85
};

void SetDitherGrayTable(signed char *table, unsigned width, unsigned height) {
        int v7; // [sp+Ch] [bp-34h]@4

        if (m_pDitherTable)
                free(m_pDitherTable);
        m_DitherTableW = width;
        m_DitherTableH = height;
        if (width & 7)
                v7 = 7;
        else
                v7 = 0;
        m_DitherTablePitch = m_DitherTableW + v7;
        size_t s = (m_DitherTableW + v7) * m_DitherTableH;
        m_pDitherTable = malloc(s);
        memset(m_pDitherTable, 0, s);
        for (int j = 0; j < m_DitherTableH; ++j) {
                for (int k = 0; k < m_DitherTablePitch; ++k)
                        m_pDitherTable[k + j * m_DitherTablePitch] =
                                (unsigned char)( -1 - table[j * m_DitherTableW + k % m_DitherTableW]);
//                        *(k + j * m_DitherTablePitch + m_pDitherTable) =
//                                (char)(-1 - *(table + j * m_DitherTableW + k % m_DitherTableW));
        }
}

void SetDefaultScreen() {
        SetDitherGrayTable((signed char*)&DeviceBestDither, 16, 16);
}

int GetLineBytes(int width, int mult)
{
        return ((mult * width + 31) & 0xFFFFFFE0) >> 3;
}

//void HalftoneDibToDib(unsigned char*, unsigned char*, unsigned, unsigned, unsigned, unsigned);

void HalftoneDibToDib(unsigned char *planes8, unsigned char *planes, int width, int numver, int contrast, int brightness) {
        int v7; // eax@7
        int v8; // eax@7
        unsigned char transferTable[256]; // [sp+10h] [bp-138h]@4
        int v12; // [sp+114h] [bp-34h]@6
        unsigned int v13; // [sp+118h] [bp-30h]@6
        unsigned char *v17; // [sp+128h] [bp-20h]@7
        unsigned char *v18; // [sp+12Ch] [bp-1Ch]@7
        unsigned char v21; // [sp+138h] [bp-10h]@11

        if (!m_pDitherTable)
                SetDefaultScreen();

        for (int i = 0; i < 256; i++) {
                transferTable[i] = Transfer2((unsigned char)i, contrast, brightness);
        }

        /*
        fprintf(stderr, "contrast=%d brightness=%d\n", contrast, brightness);
        fprintf(stderr, "DeviceBestDither\n");
        for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++)
                        fprintf(stderr, "0x%02X, ", DeviceBestDither[i*16 + j]);
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "m_pDitherTable\n");
        for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++)
                        fprintf(stderr, "0x%02X, ", m_pDitherTable[i*16 + j]);
                fprintf(stderr, "\n");
        }
        */

        v12 = width / 8;
        v13 = (((char) width + ((unsigned int) (width >> 31) >> 29)) & 7) - ((unsigned int) (width >> 31) >> 29);

        for (int j = 0; j < numver; j++) {
                unsigned char *v16 = &m_pDitherTable[j % m_DitherTableH * m_DitherTablePitch];
                v7 = GetLineBytes(width, 8);
                v17 = &planes8[j * v7];
                v8 = GetLineBytes(width, 1);
                v18 = &planes[j * v8];
                int v19 = 0;
                for (int k = 0; k < v12; k++) {
                        unsigned char *v20 = v16 + v19;
                        *v18 = (unsigned char)(
                                (((transferTable[v17[6]] + *(v20 + 6)) & 0x100) >> 7)
                               | (((transferTable[v17[5]] + *(v20 + 5)) & 0x100) >> 6)
                               | (((transferTable[v17[4]] + *(v20 + 4)) & 0x100) >> 5)
                               | (((transferTable[v17[3]] + *(v20 + 3)) & 0x100) >> 4)
                               | (((transferTable[v17[2]] + *(v20 + 2)) & 0x100) >> 3)
                               | (((transferTable[v17[1]] + *(v20 + 1)) & 0x100) >> 2)
                               | (((transferTable[*v17] + *(v20)) & 0x100) >> 1)
                               | ((transferTable[v17[7]] + *(v20 + 7)) >> 8)
                        );
                        v19 = (v19 + 8) % m_DitherTableW;
                        v17 += 8;
                        v18++;
                }
                if (v13) {
                        v21 = 0;
                        for (int l = 0; l < v13; ++l)
                                v21 |= ((v17[l] + *(v16 + v19 + l)) & 0x100) >> (l + 1);
                        *v18 = v21;
                }
        }
}
