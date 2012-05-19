#!/usr/bin/perl

#
# Run this program in the top level GCompris directory
# with a locale parameter like ./check_missing_voices fr
#
# It will tell you which voices are missing
#
use strict;
use Data::Dumper;

if(!defined $ARGV[0])
  {
    print "Usage: ./check_missing_voices <locale>\n";
    print "       e.g. ./check_missing_voices fr\n";
    exit 1;
  }
my $TARGET_LOCALE =  $ARGV[0];

my $BASEDIR="./boards/voices";

# I don't set alphabet dir, it's too locale specific
my @SUBDIRS= qw/geography misc colors/;

if(! -f "$BASEDIR/en/misc/welcome.ogg")
  {
    print "ERROR: You must run this tool from the top GCompris directory this way:\n";
    print "       ./tools/check_missing_voices.pl $TARGET_LOCALE\n";
    print "       You must make a link from boards/voices to its actual location.\n";
    exit(1);
  }

my @LOCALES;
foreach my $file (`ls $BASEDIR`)
  {
    chomp($file);
    if (-d "$BASEDIR/$file" &&
	($file =~ /^[a-z]{2}$/ || $file =~ /^[a-z]{2}_[a-zA-Z]{2}$/))
      {
       push(@LOCALES, $file);
      }
  }

printf("Locale already supported: @LOCALES\n");

# Create the longest list possible
my @ALL_FILES;
foreach my $locale (@LOCALES)
  {
    foreach my $subdir (@SUBDIRS)
      {
	if (! opendir DIR, "$BASEDIR/$locale/$subdir")
	  {
	    next;
	  }

	foreach my $file ( grep { $_ =~ /\.ogg$/} readdir DIR)
	  {
	    if("@ALL_FILES" !~ /$subdir\/$file/g)
	      {
		push(@ALL_FILES, "$subdir/$file");
	      }
	  }
	closedir DIR;
      }
  }

#
# Now we have the uniq list of all the files of all locales.
# We now check each one is translated in the target locale
#
print "Missing files for locale '$TARGET_LOCALE':\n";
my $got_error = 0;
foreach my $file (@ALL_FILES)
  {
    if(! -f "$BASEDIR/$TARGET_LOCALE/$file")
      {
	print "$file\n";
	$got_error = 1;
      }
  }

#
# Check intro voices
#
my $database = glob("~/.config/gcompris/gcompris_sqlite.db");
if (! -e $database) {
    print "ERROR: The GCompris sqlite database is not in $database." .
	" Run GCompris once to create it\n";
    $got_error = 1;
}

my $request = "select name from boards where type != 'menu' and section != '/experimental'" .
    " and name != 'tuxpaint' and name !='administration' and name !='login';";

my $results = `sqlite3 $database "$request"`;
foreach my $name (split /\n/, $results)
{
    my $file = "intro/$name.ogg";
    if (! -e "$BASEDIR/$TARGET_LOCALE/$file") {
	print "$file\n";
	$got_error = 1;
    }
}



print "\nGreat, nothing is missing !\n" if !$got_error;
print "\nI did not checked the directory '$BASEDIR/$TARGET_LOCALE/alphabet'\n"
