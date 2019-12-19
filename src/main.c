/*
 * Kyocera KPSL filter for CUPS.
 *
 * Copyright 2015 by svolkov
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more information.
 */

//
// Created by svolkov on 06.10.15.
//

#include <fcntl.h>
#include <cups/raster.h>
//#include <cups/language-private.h>

#include "rastertokpsl.h"

/*
 * usage rastertopcl job-id user title copies options [raster_file]
 */
int main(int argc, const char **argv, const char **envp) {
        int fd;                /* File descriptor */
        cups_raster_t *ras;                /* Raster stream for printing */

        /*
         * Make sure status messages are not buffered...
         */

        setbuf(stderr, 0);

        /*
         * Check command-line...
         */

        if (argc < 6 || argc > 7) {
                /*
                 * We don't have the correct number of arguments; write an error message
                 * and return.
                 */
                _cupsLangPrintFilter(stderr, "ERROR",
                                     _("%s job-id user title copies options [raster_file]"),
                                     "rastertokpsl");
                return (1);
        }

        if (argc == 7) {
                if ((fd = open(argv[6], O_RDONLY)) == -1) {
                        _cupsLangPrintFilter(stderr, "ERROR", _("Unable to open raster file"));
                        sleep(1);
                        return (1);
                }
        }
        else
                fd = 0; // stdin

        ras = cupsRasterOpen(fd, CUPS_RASTER_READ);

        int pages = rastertokpsl(ras, argv[2], argv[3], atoi(argv[4]), argv[5]);

        cupsRasterClose(ras);
        if (fd != 0)
                close(fd);

        //fclose(fp);

        if (pages != 0) {
                _cupsLangPrintFilter(stderr, "INFO", _("Ready to print."));
                return 0;
        } else {
                _cupsLangPrintFilter(stderr, "ERROR", _("No pages found!"));
                return 1;
        }
}
