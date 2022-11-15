#!/usr/bin/env perl

use strict;
use warnings;

use bigint qw/hex/;

my @sym_table;

my $str_table_offset = 8;
my $str_table_length = 0;

while (my $line = <>) {
    if ($line =~ /^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9]+)\s+([a-zA-Z_][a-zA-Z0-9_]+)$/) {
	$str_table_offset += 16;
	$str_table_length += length($5) + 1;
	push @sym_table, [ hex($1), $5 ];
    }
}

@sym_table = sort { $a->[0] <=> $b->[0] } @sym_table;

binmode(STDOUT);

print pack("Q", $str_table_offset);

my $entry_offset = 0;

for my $entry (@sym_table) {
    print pack("Q*", $entry->[0], $entry_offset);
    $entry_offset += length($entry->[1]) + 1;
}

for my $entry (@sym_table) {
    print pack("Z*", $entry->[1]);
}
