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

void *load_asset(gchar *dataset, gchar* categories, gchar* name, guint expected_number);


/*
 * Test the Load of an asset from the assetml database
 */
void *load_asset(gchar *dataset, gchar* categories, gchar* name, guint expected_number)
{
  GList *gl_result;

  gl_result = assetml_get_asset(dataset, categories, name);

  if(g_list_length(gl_result)==expected_number)
    printf("Test Dataset=%15s Categorie=%12s Name=%10s PASSED\n", dataset, categories, name);
  else
    printf("Test Dataset=%15s Categorie=%12s Name=%10s FAILED\n", dataset, categories, name);

  assetml_free_assetlist(gl_result);
}

int
main (int argc, char *argv[])
{

  load_asset("gcompris core", NULL, "number one", 1);
  load_asset("gcompris core", NULL, "Number One", 1);
  load_asset("gcompris core", NULL, "number Two", 1);
  load_asset(NULL, NULL, "number", 0);
  load_asset(NULL, NULL, NULL, 10);
  load_asset(NULL, "math", "number one", 1);
  load_asset(NULL, "nonexistant", NULL, 0);
  load_asset(NULL, "algebra", NULL, 10);
  load_asset(NULL, "math", NULL, 10);
  load_asset(NULL, "gcompris", NULL, 10);

}
