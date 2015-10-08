#!/usr/bin/perl -w
use strict;
use Encode qw(decode encode);
use utf8;

require "kpslcmp.pl";

binmode(STDOUT,':utf8');

# Usage: data_kpslcmp.pl
# generates kpsl from all pdf in the data directory.

sub checkFile($$) {
    my ($file, $opts) = @_;
    my $kpsl1 = $file;
    $kpsl1 =~ s/\.pdf/1\.kpsl/;
    my $kpsl2 = $file;
    $kpsl2 =~ s/\.pdf/2\.kpsl/;

    print "File: $file\n";

    # original kpsl
    `/usr/sbin/cupsfilter -p Kyocera_FS-1020MFPGDI.ppd -m printer/foo $opts $file -e > $kpsl1`;

    # re kpsl
    `/usr/sbin/cupsfilter -p Kyocera_FS-1020MFPGDI_RE.ppd -m printer/foo $opts $file -e > $kpsl2`;

    # check kpsl

    if (compKpsl($kpsl1, $kpsl2)) {
        print "Not equal kpsl: $file\n";
        return 1;
    } else {
        # rm kpsl
        unlink $kpsl1, $kpsl2;
        return 0;
    }
}

if (-e "data")
{
    (-d _) or die ("data not drectory!");
}

foreach my $file (<"data/*.pdf">) {


    if (-f $file) {
        (!checkFile($file, "")) or die "Not equal kpsl: $file\n";
        (!checkFile($file, "-o CaBrightness=20")) or die "Not equal kpsl: $file\n";
        (!checkFile($file, "-o CaContrast=20")) or die "Not equal kpsl: $file\n";
    }
}

print "ALL OK\n";


