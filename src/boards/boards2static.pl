#!/usr/bin/perl

# Convert all the .c files in this directory from boards dynamic naming (Default CVS, Linux)
# To static naming (For windows)

my $outputdir = "static";

opendir BOARDS, ".";

foreach my $file (readdir BOARDS) {
  if (($file =~ /\.c$/) && ($file !~ /^\./)) {
#  if ($file =~ /^clickgame.c$/) {
    print "\nProcessing $file\n";
    open BOARD, $file;
    my @tmp = split('\.', $file);
    my $basefile = $tmp[0];
    print "basefile=$basefile\n";
    open (OUTPUT, ">$outputdir/$file")
      or die "Can't open $outputdir/$file for update: $!";

    foreach $line (readline BOARD) {
      if ($line =~ "^static BoardPlugin ") {
	print OUTPUT "BoardPlugin ${basefile}_menu_bp =\n";
      } elsif ($line =~ "get_bplugin_info") {
	$line =~ s/get_bplugin_info/${basefile}_get_bplugin_info/;
	print OUTPUT $line;
      } elsif ($line =~ "return \&menu_bp") {
	print OUTPUT "  return \&${basefile}_menu_bp;\n";
      } elsif ($line =~ "gcomprisBoard->plugin=&menu_bp;") {
	print OUTPUT "	  gcomprisBoard->plugin=&${basefile}_menu_bp;\n";
      } else {
	print OUTPUT $line;
      }
    }
    close OUTPUT;
    close BOARD;
  }
  close BOARDS;
}
