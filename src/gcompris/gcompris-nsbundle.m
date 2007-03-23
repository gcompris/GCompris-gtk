#include <Foundation/Foundation.h>
#include "gcompris.h"

#include "config.h"

@interface NSMutableString (Perso)
- (id) stringCleanPath; 
@end

void gcompris_fix_gtk_etc (void);

void set_prefix( NSString *source_dir, 
		      NSString *filename, 
		      NSString *tmp_dir,
		      NSString *prefix,
		 const char *var);

gchar *gcompris_nsbundle_resource(void)
{
  gchar *resourcePath = NULL;

  NSAutoreleasePool *pool;
  pool = [NSAutoreleasePool new];
  
  resourcePath = g_strdup_printf("%s",[[[NSBundle mainBundle] resourcePath] UTF8String]);

  printf("NSBundle resourcePath %s\n", resourcePath);

  gcompris_fix_gtk_etc ();

  [pool release];
  return resourcePath;
}

void gcompris_fix_gtk_etc (void)
{
  NSMutableString *gtk_path = [[[[NSBundle mainBundle] executablePath] mutableCopy] autorelease] ;

  NSString *base_dir = @"/../";
  NSString *gtk_dir = @NSBUNDLE_GTK_DIR ;
  NSString *gtk_conf_dir = @"/etc/gtk-2.0";
  NSString *pango_conf_dir = @"/etc/pango";
  NSString *gtk_immodules = @"/gtk.immodules";
  NSString *gdk_pixbuf_loaders =  @"/gdk-pixbuf.loaders";
  NSString *pango_modules = @"/pango.modules";
  NSString *pangorc = @"/pangorc";

  printf("NSBundle executablePath %s\n", [gtk_path UTF8String]);

  // first is to suppress last component (executable name)
  [gtk_path appendString: base_dir];
  [gtk_path appendString: gtk_dir];
  [gtk_path stringCleanPath];

  printf("NSBundle Gtk Dir Path %s\n", [gtk_path UTF8String]);

  // Get the temporary filenames
  NSString *tmp_dir = [NSTemporaryDirectory() autorelease];
  printf("Temporary directory %s\n", [tmp_dir UTF8String]);

  set_prefix( gtk_conf_dir, 
	      gtk_immodules, 
	      tmp_dir,
	      gtk_path,
	      "GTK_IM_MODULE_FILE");


  set_prefix( gtk_conf_dir, 
	      gdk_pixbuf_loaders, 
	      tmp_dir,
	      gtk_path,
	      "GDK_PIXBUF_MODULE_FILE");

  set_prefix( pango_conf_dir, 
	      pango_modules, 
	      tmp_dir,
	      gtk_path,
	      NULL);


  // just copy the pangorc file
  NSMutableString *pango_rc = [[gtk_path mutableCopy] autorelease];
  [pango_rc appendString: pango_conf_dir];
  [pango_rc appendString: pangorc];

  NSMutableString *tmp_pango_rc = [[tmp_dir mutableCopy] autorelease];
  [tmp_pango_rc appendString: pangorc];

  [[NSFileManager defaultManager] copyPath: pango_rc toPath: tmp_pango_rc handler: nil];

  printf("Copy %s file to %s\n", [pango_rc UTF8String], [tmp_pango_rc  UTF8String]);
  // PANGO_RC_FILE gives path to pango.modules
  setenv ("PANGO_RC_FILE", g_strdup([tmp_pango_rc UTF8String]), TRUE);

  //Now we just need to adjust some environnement variables
  setenv ("GTK_EXE_PREFIX", g_strdup([gtk_path UTF8String]), TRUE);
  setenv ("GTK_DATA_PREFIX", g_strdup([gtk_path UTF8String]), TRUE);


  printf ("GTK environnemnt set to %s\n", getenv("GTK_EXE_PREFIX"));

  // Unused ?
  NSMutableString *gtk_rc = [[gtk_path mutableCopy] autorelease];
  [gtk_rc appendString: gtk_conf_dir];
  [gtk_rc appendString: @"/gtkrc"];
  setenv ("GTK2_RC_FILES", g_strdup([gtk_rc UTF8String]), TRUE);

  printf ("GTK environnemnt set to %s\n", getenv("GTK2_RC_FILES"));

  // is that usefull for GCompris ?
  NSMutableString *fontconfig_path = [[gtk_path mutableCopy] autorelease];
  [fontconfig_path appendString: @"/etc/fonts"];
  setenv ("FONTCONFIG_PATH", g_strdup([fontconfig_path UTF8String]), TRUE);

  printf ("FC environnemnt set to %s\n", getenv("FONTCONFIG_PATH"));

  printf ("GTK environnemnt set to %s\n", getenv("GTK_EXE_PREFIX"));
  
}


void set_prefix( NSString *source_dir, 
		      NSString *filename, 
		      NSString *tmp_dir,
		      NSString *prefix,
		      const char *var)
{
  NSMutableString *source_file = [[prefix mutableCopy] autorelease];
  [source_file appendString: source_dir];
  [source_file appendString: filename];

  NSMutableString *tmp_file = [[tmp_dir mutableCopy] autorelease];
  [tmp_file appendString: filename];

  NSMutableString *file_content = [[[NSString stringWithContentsOfFile: source_file] mutableCopy] autorelease];
  [file_content replaceOccurrencesOfString: @"@{prefix}" 
	    withString: prefix
	    options: NSLiteralSearch
	    range: NSMakeRange(0, [file_content length])];
  [file_content writeToFile: tmp_file atomically: NO ];

  printf("Write %s file to %s\n", [source_file UTF8String], [tmp_file  UTF8String]);

  if (var)
    setenv(var, g_strdup([tmp_file UTF8String]), 1);
}


@implementation NSMutableString (Perso)

// suppress '..' and component just before.
- (id) stringCleanPath
{
  NSMutableArray *tmpPath;
  uint index =1;

  tmpPath = [[[self pathComponents] mutableCopy] autorelease];

  while (index < [tmpPath count]) {
    if ([[tmpPath objectAtIndex: index] isEqualToString: @".."]) {
      if ( index == 0)
	return self;
      else {
        index--;
	[tmpPath removeObjectAtIndex: index];
	[tmpPath removeObjectAtIndex: index];
      }
    }
    else
      index++;
  }

  // this is to make +[NSString pathWithComponents:] construct
  // an absolute path if necessary
  if ([super isAbsolutePath] == YES)
    {
      [tmpPath replaceObjectAtIndex: 0 withObject: @""];
    }

  [self setString:[[NSString pathWithComponents: tmpPath] autorelease]] ;

  return self;
}
@end
