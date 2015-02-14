#!/usr/bin/perl

use warnings;
use strict;

use IO::Uncompress::RawInflate qw(rawinflate);

rawinflate "-" => "-";
