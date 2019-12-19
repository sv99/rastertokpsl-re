# rastertokpl

Kyocera KPSL filter for CUPS licensed under the Apache License Version 2.0. See the file "LICENSE" for more information.

The reason for development is unexpected printing errors with original driver.
The problem in th non ASCII encoding of the file names for printed files.

**FS-1020MFP**

- PPD path: /Library/Printers/PPDs/Contents/Resources/Kyocera FS-1020MFPGDI.ppd
- Installed in the /Library/Printers/Kyocera/kpsl/rastertokpsl.app
- Full path to the filter: /Library/Printers/Kyocera/kpsl/rastertokpsl.app/Contents/MacOS/rastertokpl

[convert for russian letter in the name](http://help.ubuntu.ru/wiki/%D0%BF%D0%B5%D1%80%D0%B8%D1%84%D0%B5%D1%80%D0%B8%D0%B9%D0%BD%D1%8B%D0%B5_%D1%83%D1%81%D1%82%D1%80%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%B0/%D0%BB%D0%B0%D0%B7%D0%B5%D1%80%D0%BD%D1%8B%D0%B9_%D0%BF%D1%80%D0%B8%D0%BD%D1%82%D0%B5%D1%80_kyocera_fs-1040)

Problem with utf8 simbols in the job title.

## cups

Maverick 10.9.5 have cups version 1.7.2.
El Capitan 10.11.4 have cups version 2.1.0
High Sierra 10.13.6 hav cups version 2.2.5

[Documentation CUPS 1.7](http://www.cups.org/documentation.php?VERSION=1.7&Q=)

[cups web admin:631](http://127.0.0.1:631)

```
cupsctl WebInterface=on
```

```
sudo launchctl unload -w /System/Library/LaunchDaemons/org.cups.cupsd.plist
sudo launchctl list | grep cupsd
sudo launchctl load -w /System/Library/LaunchDaemons/org.cups.cupsd.plist
sudo launchctl list | grep cupsd
```

Debug cups filters:

```
cupsctl --debug-logging
cupsctl --no-debug-logging
```

Print to file. Use printer/foo to convert to the printer format defined  by  the
filters in the PPD file.

```
/usr/sbin/cupsfilter -p Kyocera_FS-1020MFPGDI_RE.ppd -m printer/foo data/d001.pdf -e > t.kpsl
```

[Filter Chain debugging](http://osdir.com/ml/printing.cups.devel/2004-10/msg00026.html)

Debian pages:

[Dissecting and Debugging the CUPS Printing System](https://wiki.debian.org/Dissecting%20and%20Debugging%20the%20CUPS%20Printing%20System#Capturing_the_File_which_is_Sent_to_the_Printer)

[The cupsfilter Utility](https://wiki.debian.org/The%20cupsfilter%20Utility).

[RasterView 1.4.1](http://www.msweet.org/projects.php?Z7)

## install
```
./install.sh
```

On the El Capitan original driver not installed with error - Not found software for install.

Need copy kpsl/ to the /Library/Printers/Kyocera/. Installation with warning - printer worked!!

## reverse engineering

Used CLion and IdaPro

OSX version have image symbols

Used [JBIG-KIT 2.0](http://www.cl.cam.ac.uk/~mgk25/jbigkit/) - link in the resources.

[JBIG-KIT on github](https://github.com/qyot27/jbigkit)

### cups filter internal

[Raster API](http://www.cups.org/documentation.php/api-raster.html#cupsRasterReadHeader2)

Filter read raster stream and write pages in the **stdout**.

### Some rewrites

Based on the existing realisation filters in the cups source: rastertohp.c and rastertoepson.c.

### replace asciitounicode

Use ConvertUTF.c for UTF8 -> UTF16 conversion.

### miniunit.h

[miniunit.h](http://www.jera.com/techinfo/jtns/jtn002.html#Introduction)

## pdftoraster.sh rastertokpsl.sh rastertokpsl-re.sh

Direct call filters without cupsfilter. Used for developments and testing.

pdftoraster.sh - make raster file from pdf.

rastertokpsl.sh - convert raster file to the kpsl, using original filter.

rastertokpsl-re.sh - convert raster file to the kpsl? using re filter.
 

## kpslcmp.pl

Perl script - simple check two kpsl files, skip from compare user, timestamp and title field from kpsl header.

## data_kpslcmp.pl

Perl script, which generates kpsl files for all pdf files in the data directory, using
original rastertokpsl and rastertokpsl-re filters and compare it.

rastertokpsl-re must be properly installed.

## ToDo

kpsldump.pl - kpsl dumper.

## CaBrigtness and CaContrast

Change +-100.
