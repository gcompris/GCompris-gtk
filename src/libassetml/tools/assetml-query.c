/* assetml-query - assetml-query.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <libintl.h>
#include <locale.h>
#include <popt.h>

#include <assetml.h>
#include <stdio.h>
#include <stdlib.h>

/* I18N Helper macros */
#define _(str) gettext (str)
#define gettext_noop(str) (str)
#define N_(str) gettext_noop (str)

void	 dump_asset(AssetML *assetml);

/****************************************************************************/
/* Command line params */

/*** gcompris-popttable */
static gchar *dataset	= NULL;
static gchar *name		= NULL;
static gchar *categories	= NULL;
static gchar *mimetype		= NULL;
static int showVersion		= FALSE;

static struct poptOption options[] = {
  {"dataset", 'd', POPT_ARG_STRING, &dataset, 0,
   N_("Specify the dataset to search in"), NULL},
  {"name", 'n', POPT_ARG_STRING, &name, 0,
   N_("Specify a name to search"), NULL},
  {"categories", 'c', POPT_ARG_STRING, &categories, 0,
   N_("Specify a category to search in"), NULL},
  {"mimetype", 'm', POPT_ARG_STRING, &mimetype, 0,
   N_("Specify a mimetype to search in (eg: image/png)"), NULL},
  {"version", '\0', POPT_ARG_NONE, &showVersion, 0,
   N_("Prints the version of " PACKAGE), NULL},
  POPT_AUTOHELP
  {
    NULL,
    '\0',
    0,
    NULL,
    0,
    NULL,
    NULL
  }
};

static void printVersion(void) {
    fprintf(stdout, _("assetml-query version %s\n"), "1.0");
}


void dump_asset(AssetML *assetml)
{

  if(assetml==NULL)
    return;

  printf("dataset         = %s\n",assetml->dataset);
  printf("  categories    = %s\n",assetml->categories);
  printf("  name          = %s\n",assetml->name_noi18n);
  printf("    name(i18n)  = %s\n",assetml->name);
  printf("    mimetype    = %s\n",assetml->mimetype);
  printf("    description = %s\n",assetml->description);
  printf("    file        = %s\n",assetml->file);
  printf("    credits     = %s\n",assetml->credits);

}

/*****************************************
 * Main
 *
 */

int
main (int argc, const char **argv)
{
  poptContext optCon; 
  GList *result;

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  optCon = poptGetContext("assetml-query", argc, argv, options, 0);


  /* although there are no options to be parsed, check for --help */
  poptGetNextOpt(optCon);

  optCon = poptFreeContext(optCon); 

  if (showVersion) 
    {
      printVersion();
      return 0;
    }

  /*------------------------------------------------------------*/
  result = assetml_get_asset(dataset, categories, mimetype, name);

  g_list_foreach (result, (GFunc) dump_asset, NULL);

  assetml_free_assetlist(result);

  return 0;
}

