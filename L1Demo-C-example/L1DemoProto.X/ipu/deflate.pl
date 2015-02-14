#!/usr/bin/perl

use warnings;
use strict;

use IO::Compress::RawDeflate qw(rawdeflate :level :strategy);

rawdeflate "-" => "-", -Level => Z_BEST_COMPRESSION, -Strategy => Z_FIXED;
