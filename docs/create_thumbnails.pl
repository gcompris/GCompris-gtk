#!/usr/bin/perl
use strict;
use Data::Dumper;

my $gcompris_root_dir	= "..";
my $screenshots_dir     = "$gcompris_root_dir/docs/screenshots";
my $thumb_size		= "130x130";

# First, Get all the boards description files
opendir DIR, $screenshots_dir or die "cannot open dir $screenshots_dir: $!";
my @files = grep { $_ =~ /\.jpg$/} readdir DIR;
closedir DIR;

#-------------------------------------------------------------------------------


foreach my $image (@files) {

  print "Processing $image\n";
  my $file  = "$screenshots_dir/$image";
  (my $thumb = $file) =~ s/\.jpg/_small\.jpg/;
  system ("convert -geometry ".$thumb_size.
      " -quality 100 $file $thumb") == 0
      || die "Problems with convert: $?\n";

}

exit 0;
