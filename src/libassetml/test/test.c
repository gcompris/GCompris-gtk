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

void *load_asset(gchar *dataset, gchar* categories, gchar* name, 
		 gchar* mimetype, guint expected_number);


/*
 * Test the Load of an asset from the assetml database
 */
void *load_asset(gchar *dataset, gchar* categories, gchar* name, gchar* mimetype,
		 guint expected_number)
{
  GList *gl_result;

  gl_result = assetml_get_asset(dataset, categories, mimetype, name);

  if(g_list_length(gl_result)==expected_number)
    printf("%15s %15s %15s %15s %2d/%2d PASSED\n", dataset, categories, mimetype, name, 
	   expected_number, g_list_length(gl_result));
  else
    printf("%15s %15s %15s %15s %2d/%2d FAILED\n", dataset, categories, mimetype, name, 
	   expected_number, g_list_length(gl_result));

  assetml_free_assetlist(gl_result);
}

int
main (int argc, char *argv[])
{

  printf("%15s %15s %15s %15s %2s/%2s\n", "Dataset", "Category", "Mimetype", "Name", "Ex", "Op");

  load_asset("assetml test", NULL, "number one", "image/png", 1);
  load_asset("assetml test", NULL, "number one", NULL, 2);
  load_asset("assetml test", NULL, "Number One", "audio/x-ogg", 1);
  load_asset("assetml test", NULL, "number Two", NULL, 2);
  load_asset("assetml test", NULL, "number", NULL, 0);
  load_asset("assetml test", NULL, NULL, NULL, 20);
  load_asset("assetml test", "math", "number one", NULL, 2);
  load_asset("assetml test", "nonexistant", NULL, NULL, 0);
  load_asset("assetml test", "algebra", NULL, "image/png", 10);
  load_asset("assetml test", "math", NULL, "audio/x-ogg", 10);
  load_asset("assetml test", "gcompris", NULL, NULL, 20);

}
