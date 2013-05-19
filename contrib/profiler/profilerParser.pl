#!/usr/bin/perl -w

use strict;
use warnings;

use Env qw(CROSS_COMPILE);


my @nmSymbols = `${CROSS_COMPILE}nm build/hypervisor.elf | sort -r`;
my @symbols;
my @symbolNums;

for my $line (@nmSymbols)
{
  chomp $line;
  if ($line =~ m/^([0-f]{8})\s/)
  {
    push @symbols, $line;
    push @symbolNums, hex($1);
  }
}

my $log = "/home/kudinsd6/arm-dev/profiler/data/record.log";
my $fh;
my $buffer;

open $fh, "<$log";
binmode $fh;
while (read($fh, $buffer, 4))
{
  my $pc = unpack("V", $buffer);
  printf "%08x:\n", $pc;
  # binary search would be more appropriate i guess
  my $found = 0;
  for my $symIndex (0..$#symbolNums)
  {
    my $addr = $symbolNums[$symIndex];
    next if $addr > $pc;
    $pc = $addr unless $found;
    last if $addr != $pc;
    print "$symbols[$symIndex]\n";
    $found = 1;
  }
  print "\n";
}
close $fh;
