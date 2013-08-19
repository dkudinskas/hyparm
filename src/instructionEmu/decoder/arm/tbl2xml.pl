#!/usr/bin/perl -w

# Hack to make decoder.xml from tables.inc.c ... sort of. Requires manual tweaking (the other way is more logical but table search is the reference decoder).

use strict;
use warnings;

use Data::Dumper;

my $currentCategory = '';
my %categories;

print '<?xml version="1.0" encoding="iso-8859-1"?>' . "\n\n" . '<autodecoder name="arm">' . "\n\n";

open my $f, 'tables.inc.c' or die 'Cannot open decoding tables file';
while (<$f>)
{
	chomp;
	next if (m/^\s*([{}];?)?\s*$/ or m/^\s*(\/[\/\*]|[\#\*])/ or m/\\$/
	         or m/^\s*ENTRY\s*\(\s*IRC_REPLACE\s*,\s*undefinedInstruction\s*,\s*[^\s,]+\s*,\s*0x00000000\s*,\s*0x00000000\s*,\s*"[^"]+"\)$/
	         or m/^\s*\{\s*0x00000000\s*,\s*0x00000000\s*,\s*[^\s\}]+\s*\}\s*$/);
	if (m/^\s*static\s+struct\s+decodingTableEntry\s+([^\s]+?)\[\]/)
	{
		$currentCategory = $1;
		$categories{$currentCategory} = [];
	}
	elsif (m/^\s*ENTRY\s*\(\s*(IRC_[A-Z_]+)\s*,\s*([^\s,]+)\s*,\s*([^\s,]+)\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*"([^"]+)"\s*\)\s*,\s*$/)
	{
		die "Error: instruction outside category" unless $currentCategory ne '';
		push @{$categories{$currentCategory}}, {'name' => $6, 'code' => $1, 'interpreter' => $2, 'patcher' => $3, 'value' => lc($4), 'mask' => lc($5)};
	}
	elsif (m/^\s*static\s+struct\s+decodingTable\s+armCategories\s*\[\]/)
	{
		# Category list
	}
	elsif (m/^\s*\{\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*([^\s\}]+)\s*\}\s*,\s*$/)
	{
		my $mask = lc($1);
		my $value = lc($2);
		print "<category name=\"$3\" mask=\"$mask\" value=\"$value\">\n";
		foreach my $instr (@{$categories{$3}})
		{
			my %instruction = %{$instr};
			print "  <instruction name=\"$instruction{name}\" mask=\"$instruction{mask}\" value=\"$instruction{value}\" ";
			if ($instruction{'code'} eq 'IRC_SAFE')
			{
				# Do nothing
			}
			else
			{
				print "code=\"$instruction{code}\" ";
				if ($instruction{'code'} eq 'IRC_REPLACE')
				{
					print "handler=\"$instruction{interpreter}\" ";
				}
				elsif ($instruction{'code'} eq 'IRC_PATCH_PC')
				{
					print "handler=\"$instruction{patcher}\" ";
				}
				elsif ($instruction{'code'} ne 'IRC_REMOVE')
				{
					die "Error: unknown code $instruction{'code'} for instruction $instruction{'name'}\n";
				}
			}
			print "/>\n";
		}
		print "</category>\n\n";
	}
	else
	{
		print "Error: unrecognized line: $_\n";
	}
}
close $f;

print "</autodecoder>\n";
#print Dumper(\%categories);
