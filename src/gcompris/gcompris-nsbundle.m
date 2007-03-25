#include <Foundation/Foundation.h>
#include "gcompris.h"

#include "config.h"

void gcompris_fix_gtk_etc (void);

void set_prefix( NSString *prefix_source_dir,
		 NSString *source_dir, 
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
  NSMutableString *gtk_path = [[[[[NSBundle mainBundle] executablePath] stringByDeletingLastPathComponent] mutableCopy] autorelease] ;

  NSString *gtk_dir = @NSBUNDLE_GTK_DIR ;
  NSString *gtk_conf_dir = @"/etc/gtk-2.0";
  NSString *pango_conf_dir = @"/etc/pango";
  NSString *gtk_immodules = @"/gtk.immodules";
  NSString *gdk_pixbuf_loaders =  @"/gdk-pixbuf.loaders";
  NSString *pango_modules = @"/pango.modules";
  NSString *pangorc = @"/pangorc";
  NSString *pythonpath1 = @"/lib/python/site-packages:";
  NSString *pythonpath2 = @"/lib/python/site-packages/gtk-2.0";

  printf("NSBundle executablePath %s\n\n", [gtk_path UTF8String]);

  // first is to suppress last component (executable name)
  [gtk_path appendString: gtk_dir];
  gtk_path = [gtk_path  stringByStandardizingPath];

  printf("NSBundle Gtk Dir Path %s\n\n", [gtk_path UTF8String]);

  // Get the temporary filenames
  NSString *tmp_dir = NSTemporaryDirectory();
  printf("Temporary directory %s\n\n", [tmp_dir UTF8String]);

  set_prefix( gtk_path,
	      gtk_conf_dir, 
	      gtk_immodules, 
	      tmp_dir,
	      gtk_path,
	      "GTK_IM_MODULE_FILE");


  set_prefix( gtk_path,
	      gtk_conf_dir, 
	      gdk_pixbuf_loaders, 
	      tmp_dir,
	      gtk_path,
	      "GDK_PIXBUF_MODULE_FILE");

  set_prefix( gtk_path,
	      pango_conf_dir, 
	      pango_modules, 
	      tmp_dir,
	      gtk_path,
	      NULL);

  // Warning -> the tmp_dir because the files are in tmp directory too !!!
  set_prefix( gtk_path,
	      pango_conf_dir, 
	      pangorc, 
	      tmp_dir,
	      tmp_dir,
	      "PANGO_RC_FILE");

  //Now we just need to adjust some environnement variables
  setenv ("GTK_EXE_PREFIX", g_strdup([gtk_path UTF8String]), TRUE);
  setenv ("GTK_DATA_PREFIX", g_strdup([gtk_path UTF8String]), TRUE);
  printf ("GTK_EXE_PREFIX  environnemnt set to %s\n", getenv("GTK_EXE_PREFIX"));
  printf ("GTK_DATA_PREFIX environnemnt set to %s\n", getenv("GTK_DATA_PREFIX"));

  // Unused ?
  NSMutableString *gtk_rc = [[gtk_path mutableCopy] autorelease];
  [gtk_rc appendString: gtk_conf_dir];
  [gtk_rc appendString: @"/gtkrc"];
  setenv ("GTK2_RC_FILES", g_strdup([gtk_rc UTF8String]), TRUE);
  printf ("GTK2_RC_FILES   environnemnt set to %s\n", getenv("GTK2_RC_FILES"));

  // is that usefull for GCompris ?
  NSMutableString *fontconfig_path = [[gtk_path mutableCopy] autorelease];
  [fontconfig_path appendString: @"/etc/fonts"];
  setenv ("FONTCONFIG_PATH", g_strdup([fontconfig_path UTF8String]), TRUE);
  printf ("FONTCONFIG_PATH environnemnt set %s\n", getenv("FONTCONFIG_PATH"));

  NSMutableString *python_modules_path = [[gtk_path mutableCopy] autorelease];
  [python_modules_path appendString: pythonpath1];
  [python_modules_path appendString: gtk_path];
  [python_modules_path appendString: pythonpath2];
  setenv ("PYTHONPATH", g_strdup([python_modules_path UTF8String]), TRUE);
  printf ("PYTHONPATH      environnemnt set %s\n", getenv("PYTHONPATH"));
  
}


void set_prefix( NSString *prefix_source_dir,
		 NSString *source_dir, 
		 NSString *filename, 
		 NSString *tmp_dir,
		 NSString *prefix,
		 const char *var)
{
  NSMutableString *source_file = [[prefix_source_dir mutableCopy] autorelease];
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

  printf("Write %s\n   to %s\n", [source_file UTF8String], [tmp_file  UTF8String]);

  if (var) {
    setenv(var, g_strdup([tmp_file UTF8String]), 1);
    printf ("%s environnemnt set to %s\n\n", var, getenv(var));
  }
  
}

