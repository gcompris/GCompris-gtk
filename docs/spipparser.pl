#!/usr/bin/perl
use strict;
use Data::Dumper;

#-------------------------------------------------------------------------------
# Define some constants related to spip rubrique and section organisation
# We do not detect which locale sections are. They are hardcoded here.
my %sections = (
		"am", 0,
		"ar", 0,
		"az", 0,
		"ca", 0,
		"cs", 0,
		"da", 37,
		"de", 0,
		"el", 0,
		"en", 2,
		"en_CA", 0,
		"en_GB", 0,
		"es", 40,
		"fi", 36,
		"fr", 1,
		"ga", 0,
		"he", 0,
		"hi", 0,
		"hr", 0,
		"hu", 0,
		"it", 0,
		"lt", 0,
		"mk", 0,
		"ml", 0,
		"ms", 0,
		"nl", 0,
		"nb", 0,
		"nn", 38,
		"pa", 0,
		"pl", 0,
		"pt", 28,
		"pt_BR", 0,
		"ro", 0,
		"ru", 0,
		"sk", 0,
		"sl", 0,
		"sq", 0,
		"sr", 0,
		'sr@Latn', 0,
		"sv", 0,
		"tr", 0,
		"wa", 0,
	       );

my %rubriques = (
		 "am", 0,
		 "ar", 0,
		 "az", 0,
		 "ca", 0,
		 "cs", 0,
		 "da", 37,
		 "de", 0,
		 "el", 0,
		 "en", 12,
		 "en_CA", 0,
		 "en_GB", 0,
		 "es", 40,
		 "fi", 36,
		 "fr", 6,
		 "ga", 0,
		 "he", 0,
		 "hi", 0,
		 "hr", 0,
		 "hu", 0,
		 "it", 0,
		 "lt", 0,
		 "mk", 0,
		 "ml", 0,
		 "ms", 0,
		 "nl", 0,
		 "nb", 0,
		 "nn", 38,
		 "pa", 0,
		 "pl", 0,
		 "pt", 28,
		 "pt_BR", 0,
		 "ro", 0,
		 "ru", 0,
		 "sk", 0,
		 "sl", 0,
		 "sq", 0,
		 "sr", 0,
		 'sr@Latn', 0,
		 "sv", 0,
		 "tr", 0,
		 "wa", 0,
		);
#-------------------------------------------------------------------------------
my ($sec, $min, $hours, $day, $month, $year) = (localtime)[0,1,2,3,4,5];
my $date = "".($year+1900)."-".($month+1)."-"."$day $hours:$min:$sec";

my $gcompris_root_dir = "..";
my $boards_dir        = "$gcompris_root_dir/boards";
my $ALL_LINGUAS_STR   = `grep "ALL_LINGUAS=" $gcompris_root_dir/configure.in | cut -d= -f2`;
$ALL_LINGUAS_STR      =~ s/\"//g;
my @ALL_LINGUAS       = split(' ', $ALL_LINGUAS_STR);
push @ALL_LINGUAS, "en";	# Add english, it's not in the po list
# Debug
#@ALL_LINGUAS = qw/fr en/;

my $output_file = "all_article.spip";

# Erase previous output
unlink $output_file;

my $first_article = 10000;
my $article_id    = $first_article;



# First, loop over all the boards description files
opendir DIR, $boards_dir or die "cannot open dir $boards_dir: $!";
my @files = grep { $_ =~ /.xml$/} readdir DIR;
closedir DIR;

foreach my $board (@files) {

  print "\nProcessing $board\nLang:";

  # The first article is the reference article
  my $traduction_id = $article_id;

  # 2nd loop over each language
  foreach my $lang (@ALL_LINGUAS) {

    my $file = "$boards_dir/$board";
    my $xslfile = "spip_oneboard.xsl";

    $article_id++;

    print "$lang ";

#    print "xsltproc --stringparam language $lang --stringparam date '${date}' --stringparam article_id ${article_id} --stringparam rubrique_id $rubriques{$lang} --stringparam section_id $sections{$lang} --stringparam traduction_id ${traduction_id} $xslfile $file"."\n";
    my $output = `xsltproc --stringparam language $lang --stringparam date "${date}" --stringparam article_id ${article_id} --stringparam rubrique_id $rubriques{$lang} --stringparam section_id $sections{$lang} --stringparam traduction_id ${traduction_id} $xslfile $file`;

    if ($?>>8) {
      print "#\n";
      print "#\n";
      print "#\n";
      print "An error as been encountered in xslt processing: processing is left uncomplete\n";
      print "ERROR on file $file lang $lang#\n";
      print "#\n";
      print "#\n";

    } else {

      # Make some cleanup where needed
      # ------------------------------

      # Spip doesn't like shortucted tags like <ps/>. But xsltproc does this by default
      # and I dod not find a way to avoid that.
      # I put in this list the tags that could be empty.
      my @empty_tag = qw/id_trad nom_site url_site extra ps soustitre chapo/;
      foreach my $tag (@empty_tag) {
	$output =~ s/<$tag\/>/<$tag><\/$tag>/g;
      }

      # We need to html backquote html tags for spip
      # I put in uppercase all html tags
      my @html_tag = qw/HTML IMG A/;
      foreach my $tag (@html_tag) {
	$output =~ s/<$tag/&lt;$tag/g;
	$output =~ s/<\/$tag/&lt;$tag/g;
      }

      # Fix the tag <lien:auteur> because wa cannot use it in xslt name space is not defined
      $output =~ s/lien_auteur/lien:auteur/g;

      # Remove eMail adresses
      $output =~ s/[a-zA-Z0-9\_\-\.\(]+@[a-zA-Z0-9\_\-\.\)]+//g;

      open(OUTPUT, ">>$output_file")
	or die "Can't open: $!";

      print OUTPUT $output . "\n";

      close (OUTPUT);
    }
  }
}

print "\n\nCreated " . ($article_id - $first_article) . " Articles in $output_file\n";
print "Insert the content of this file in the SPIP dump.xml file.\n";
print "If screenshots were already in, remove them first\n";
exit 0;
