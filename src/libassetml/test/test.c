/* assetml - test.c
 *
 * Copyright (C) 2003 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "assetml.h"
#include <glib.h>
#include <config.h>
#include <stdio.h>

/*
 * Test is too basic, only the expected nnumber of matches are checked
 * => Dangerous
 */

void *load_asset(gchar *dataset, gchar* categories, gchar* file, 
		 gchar* mimetype, gchar* locale, guint expected_number);


/*
 * Test the Load of an asset from the assetml database
 */
void *load_asset(gchar *dataset, gchar* categories, gchar* file, gchar* mimetype, gchar* locale,
		 guint expected_number)
{
  GList *gl_result;

  gl_result = assetml_get_asset(dataset, categories, mimetype, locale, file);

  if(g_list_length(gl_result)==expected_number)
    printf("%15s %15s %15s %15s %6s %2d/%2d PASSED\n", dataset, categories, mimetype, file, locale,
	   expected_number, g_list_length(gl_result));
  else
    printf("%15s %15s %15s %15s %6s %2d/%2d FAILED\n", dataset, categories, mimetype, file, locale,
	   expected_number, g_list_length(gl_result));

  assetml_free_assetlist(gl_result);
}

int
main (int argc, char *argv[])
{

  printf("%15s %15s %15s %15s %6s %2s/%2s\n", "Dataset", "Category", "Mimetype", "File", "Locale", "Ex", "Ob");

  load_asset("assetml test", NULL, "1.png", "image/png", NULL, 1);
  load_asset("assetml test", NULL, "1.png", "image/png", "fr", 1);
  load_asset("assetml test", NULL, "1.png", NULL, NULL, 1);
  load_asset("assetml test", NULL, "1.ogg", "audio/x-ogg", NULL, 1);
  load_asset("assetml test", NULL, "2.png", NULL, NULL, 1);
  load_asset("assetml test", NULL, "number", NULL, NULL, 0);
  load_asset("assetml test", NULL, NULL, NULL, NULL, 20);
  load_asset("assetml test", "math", "1.png", NULL, NULL, 1);
  load_asset("assetml test", "nonexistant", NULL, NULL, NULL, 0);
  load_asset("assetml test", "algebra", NULL, "image/png", NULL, 10);
  load_asset("assetml test", "math", NULL, "audio/x-ogg", NULL, 10);
  load_asset("assetml test", "gcompris", NULL, NULL, NULL, 20);
  load_asset("assetml test", "gcompris", NULL, NULL, "fr", 20);

}
