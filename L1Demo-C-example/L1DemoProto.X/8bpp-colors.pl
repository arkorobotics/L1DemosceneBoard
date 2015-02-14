#!/usr/bin/perl

use warnings;
use strict;

# convert 8-bit to 24-bit using info from
# http://stackoverflow.com/questions/2442576/how-does-one-convert-16-bit-rgb565-to-24-bit-rgb888

my $c = 0;
while($c <= 0xFF) {
	my @bits = split(//, unpack("b8", chr($c)));
	my $r5 = $bits[7]<<4 | $bits[6]<<3 | $bits[5]<<2  | $bits[5]<<1 | $bits[5];
	my $g6 = $bits[4]<<5 | $bits[3]<<4 | $bits[2]<<3  | $bits[2]<<2 | $bits[2]<<1 | $bits[2];
	my $b5 = $bits[1]<<4 | $bits[0]<<3 | $bits[0]<<2  | $bits[0]<<1 | $bits[0];

	my $r8 = 255/31 * $r5;
	my $g8 = 255/63 * $g6;
	my $b8 = 255/31 * $b5;

	printf("0x%02x = #%02x%02x%02x\n", $c, $r8, $g8, $b8);

	$c++;
}
