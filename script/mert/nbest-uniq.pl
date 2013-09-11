#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my @handles;
for(@ARGV) {
   my $fh = IO::File->new("< $_") or die "didn't work for $_";
   binmode $fh, ":utf8";
   push @handles, $fh;
}

my $last = 0;
my (%next);
my @save;
while(<STDIN>) {
    chomp;
    /^(\d+) \|\|\|/ or die "Badly formed n-best $_\n";
    my $curr_line = $_;
    if($1 != $last) {
        my $next = $1;
        my %curr = %next;
        %next = ();
        for(@handles) {
            while(my $val = <$_>) {
                chomp $val;
                my @arr = split(/ \|\|\| /, $val);
                $arr[3] = join(" ", sort(split(/ /, $arr[3])));
                my $id = "$arr[0] ||| $arr[1] ||| $arr[3]";
                if($arr[0] != $last) {
                    $next{$id}++;
                    last;
                } else {
                    $curr{$id}++;
                }
            }
        }
        for(@save) {
            my @arr = split(/ \|\|\| /);
            $arr[3] = join(" ", sort(split(/ /, $arr[3])));
            my $id = "$arr[0] ||| $arr[1] ||| $arr[3]";
            print "$_\n" if not $curr{$id}++;
        }
        @save = ($curr_line);
        $last = $next;
    } else {
        push @save, $_;
    }
}
