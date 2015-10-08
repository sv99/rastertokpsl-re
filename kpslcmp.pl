#!/usr/bin/perl -w
use strict;
use Encode qw(decode encode);
use utf8;

binmode(STDOUT,':utf8');

sub openKPSL ($) {
    my $handle;
    my $fileName = $_[0];
    my $size = 0;
    if (-e $fileName) {
        $size = -s _;
        open ($handle, '<', $fileName) or die ("Cannot open file $fileName");
    } else {
         die ("File $fileName not exists!");
    }

    # check file signature
    read($handle, my $sign, 4);
    ($sign eq "LSPK") or  die ("File $fileName not have kpsl signature.");

    return ($handle, $size);
}

sub readUTF16($$) {
    my ($handle, $len) = @_;
    read($handle, my $value, $len);
    return decode("UTF-16LE", $value);
}

sub compKpsl($$) {

    my $result = 0;

    my ($fh1, $size1) = openKPSL($_[0]);
    my ($fh2, $size2) = openKPSL($_[1]);

    print "Compare: $_[0] -> $_[1]\n";

    # compare sizes
    if ($size1 != $size2) {
        print "File has different sizes.";
        exit 1;
    }

    my $pos = 4;
    #my ($f1, $f2);

    while ($pos < $size1) {
        # read user and timestamp
        if ($pos == 20) {
            my $user1 = readUTF16($fh1, 32);
            my $user2 = readUTF16($fh2, 32);
            print "User: $user1 $user2\n";
            read($fh1, my $ts1, 14);
            read($fh2, my $ts2, 14);
            print "Timestamp: $ts1 $ts2\n";
            $pos += 46;
            next;
        };
        # read title
        if ($pos == 80) {
            my $title1 = readUTF16($fh1, 64);
            my $title2 = readUTF16($fh2, 64);

            print "Title: $title1 $title2\n";
            $pos += 64;
            next;
        };

        read($fh1, my $f1, 1);
        read($fh2, my $f2, 1);
        if (ord($f1) != ord($f2)) {
            printf ("0x%02X %d: 0x%02X 0x%02X\n", $pos, $pos, ord($f1), ord($f2));
            $result = 1;
        }
        $pos++;
    }

    close ($fh1);
    close ($fh2);

    if ($result) {
        print "Not equal!\n";
    } else {
        print "OK\n";
    }
    return $result;
}

unless (caller) {
    die "Usage: kpslcmp.pl file1 file2" if ( $#ARGV < 1 );
    exit compKpsl($ARGV[0], $ARGV[1]) unless (caller);
}

