//
// Created by svolkov on 06.10.15.
//

#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "ConvertUTF.h"

void print_buf(UTF16 *buffer, int len) {
        UTF8 *temp = (UTF8*)buffer;
        for (int i = 0; i<len * 2; i++) {
                printf("0x%02X, ", temp[i]);
                if ( (i+1) % 16 == 0)
                        printf("\n");
        }
        printf("\n");
}

int convert_text(char *text, int buf_len, char *message) {
        UTF16 buffer[64];
        memset(&buffer, 0, sizeof(buffer));

        // normal ascii convert
        const UTF8 *parg = (const UTF8 *)text;
        UTF16 *pbuffer = (UTF16*)&buffer;
        ConversionResult res = ConvertUTF8toUTF16(&parg, parg + strlen(text), &pbuffer, pbuffer + buf_len, //sizeof(buffer),
                                                  strictConversion);
        printf("INFO %s: ConversionResult=%d\n", message, res);
        //print_buf((UTF16*)&buffer, 16);
        return res;
}

char * test_unicode() {

        // normal ascii convert
        int res = convert_text("example", 8, "normal ascii");
        mu_assert("error, convert: normal ascii", res == 0);

        // source large the target
        res = convert_text("example_123", 8, "ascii source > target");
        mu_assert("error, convert: ascii source > target", res == 2);

        // normal utf8 convert
        res = convert_text("exam_опс", 8, "normal utf8");
        mu_assert("error, convert: normal utf8", res == 0);

        // utf8 source large the target
        res = convert_text("exam_опсс", 8, "utf8 source > target");
        mu_assert("error, convert: utf8 source > target", res == 2);

        return 0;
};
