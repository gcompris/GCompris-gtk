/* assetml - assetml.c
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
#include <config.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <locale.h>
#include <dirent.h>
#include <string.h>

#define FILE_EXT ".assetml"

gchar		*assetml_get_locale(void);
static gchar	*reactivate_newline(gchar *str);
static void	 dump_asset(AssetML *assetml);
int		 selectAssetML(const struct dirent *d);
void		 assetml_read_xml_file(GList **gl_result, char *fname,
				       gchar *dataset, gchar* categories, gchar* mimetype, 
				       gchar *locale, gchar* file);
void		 assetml_load_xml(GList **gl_result, gchar *dataset, gchar* categories, 
				  gchar* mimetype, 
				  gchar* locale, gchar* file);
void		 free_asset(AssetML *assetml);

/*
 * This returns the locale for which text must be displayed
 *
 */
gchar *assetml_get_locale()
{
  char *locale;

  locale = g_getenv("LC_ALL");
  if(locale == NULL)
    locale = g_getenv("LC_MESSAGES");
  if(locale == NULL)
    locale = g_getenv("LANG");


  if(locale!=NULL)
    return(locale);

  return("en_US.UTF-8");
}

/*
 * Return a copy of the given string in which it has
 * changes '\''n' to '\n'.
 * The recognized sequences are \b
 * \f \n \r \t \\ \" and the octal format.
 * 
 */
static gchar *reactivate_newline(gchar *str)
{
  gchar *newstr;

  if(str==NULL)
    return NULL;

  newstr = g_strcompress(str);
  
  g_free(str);

  return newstr;
}

void dump_asset(AssetML *assetml)
{

  printf("Dump Asset\n");

  if(assetml==NULL)
    return;

  printf("  dataset     = %s\n",assetml->dataset);
  printf("  file        = %s\n",assetml->file);
  printf("  locale      = %s\n",assetml->locale);
  printf("  description = %s\n",assetml->description);
  printf("  categories  = %s\n",assetml->categories);
  printf("  mimetype    = %s\n",assetml->mimetype);
  printf("  credits     = %s\n",assetml->credits);

}

/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static AssetML *assetml_add_xml_to_data(xmlDocPtr doc,
					xmlNodePtr xmlnode, 
					gchar *rootdir,
					GNode * child)
{
  AssetML *assetml = NULL;
  gchar   *tmpstr;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Asset" */
     (g_strcasecmp(xmlnode->name,"Asset")!=0)
     )
    return NULL;

  assetml = g_malloc0 (sizeof (AssetML));

  /* get the specific values */
  tmpstr = xmlGetProp(xmlnode,"file");
  if(tmpstr && strlen(tmpstr)>0)
    if(rootdir[0]!='/')
      /* This is a relative path, add ASSETML_DIR and rootdir prefix */
      assetml->file		= g_build_filename(ASSETML_DIR, rootdir, tmpstr, NULL);
    else
      assetml->file		= g_build_filename(rootdir, tmpstr, NULL);
  else
    assetml->file		= NULL;
  xmlFree(tmpstr);

  tmpstr = xmlGetProp(xmlnode,"mimetype");
  if(tmpstr && strlen(tmpstr)>0)
  assetml->mimetype		= g_strdup(tmpstr);
  else
    assetml->mimetype		= NULL;
  xmlFree(tmpstr);

  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");

    /* get the description of the asset */
    if (!strcmp(xmlnode->name, "Description")
	&& (lang==NULL ||
	    !strcmp(lang, assetml_get_locale())
	    || !strncmp(lang, assetml_get_locale(), 2)))
      {
	assetml->description = reactivate_newline(xmlNodeListGetString(doc, 
								       xmlnode->xmlChildrenNode, 1));
      }

    /* get the description of the Credits */
    if (!strcmp(xmlnode->name, "Credits")
	&& (lang==NULL ||
	    !strcmp(lang, assetml_get_locale())
	    || !strncmp(lang, assetml_get_locale(), 2)))
      {
	assetml->credits = reactivate_newline(xmlNodeListGetString(doc, 
								   xmlnode->xmlChildrenNode, 1));
      }


    /* get the description of the Categories */
    if (!strcmp(xmlnode->name, "Categories")
	&& (lang==NULL ||
	    !strcmp(lang, assetml_get_locale())
	    || !strncmp(lang, assetml_get_locale(), 2)))
      {
	assetml->categories = reactivate_newline(xmlNodeListGetString(doc, 
								      xmlnode->xmlChildrenNode, 1));
      }

    xmlnode = xmlnode->next;
  }

  return(assetml);
}

/*
 * Given the assetml and the dataset, categories, name
 * return true if the assetml matches the requirements
 */
static gboolean matching(AssetML *assetml, gchar *mydataset, 
			 gchar *dataset, gchar* categories, gchar* mimetype, 
			 gchar* mylocale, gchar* locale, gchar* file)
{
  g_assert(assetml);

  assetml->dataset = g_strdup(mydataset);
  if(assetml->dataset && dataset)
    if(g_ascii_strcasecmp(assetml->dataset, dataset))
      return FALSE;

  /* Check the leading locale definition matches the leading user request so that
   * File   Requested   Status
   * fr     fr_FR.UTF8  OK
   * pt     pt_BR       OK
   * pt_BR  pt          NO
   */
  assetml->locale = g_strdup(mylocale);
  if(assetml->locale && locale)
    if(g_ascii_strncasecmp(assetml->locale, locale, strlen(assetml->locale)))
      return FALSE;

  if(assetml->mimetype && mimetype)
    if(g_ascii_strcasecmp(assetml->mimetype, mimetype))
      return FALSE;

  if(assetml->file && file)
    {
      gchar *str1;
      gchar *str2;
      gboolean nomatch;
      /* We test only the basename of the file so that caller do not need to specify a full path */
      str1 = g_path_get_basename(assetml->file);
      str2 = g_path_get_basename(file);

      nomatch = g_ascii_strcasecmp(str1, str2);

      g_free(str1);
      g_free(str2);

      if(nomatch)
	return FALSE;
    }

  if(assetml->categories && categories)
    {
      guint i;
      for(i=0; i<strlen(assetml->categories)-strlen(categories)+1; i++)
	{
	  if(!g_ascii_strncasecmp(assetml->categories+i, categories, strlen(categories)))
	    {
	      return TRUE;
	    }
	}
      return FALSE;
    }
  
  return TRUE;
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(GList **gl_result, xmlDocPtr doc, 
	  gchar *mydataset, gchar *rootdir, gchar* mylocale,
	  gchar *dataset, gchar* categories, gchar* mimetype, gchar* locale, gchar* file)
{
  xmlNodePtr node;

  /* find <Asset> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the board to the list, there are no children so
       we pass NULL as the node of the child */
    AssetML *assetml = assetml_add_xml_to_data(doc, node, rootdir, NULL);

    if(assetml && matching(assetml, mydataset, dataset, categories, mimetype, mylocale, locale, file))
      *gl_result = g_list_append (*gl_result, assetml);
				
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set
   Fill the gl_result list with all matching asseml items
*/
void assetml_read_xml_file(GList **gl_result, char *assetmlfile,
			   gchar *dataset, gchar* categories, gchar* mimetype, gchar *locale, gchar* file)
{
  /* pointer to the new doc */
  xmlDocPtr doc;
  gchar *rootdir;
  gchar *mylocale;
  gchar *mydataset;

  g_return_if_fail(assetmlfile!=NULL);

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(assetmlfile);

  /* in case something went wrong */
  if(!doc) {
    g_warning("Oups, the parsing of %s failed", assetmlfile);
    return;
  }
  
  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a Assetml node */
     g_strcasecmp(doc->children->name,"AssetML")!=0) 
    {
      xmlFreeDoc(doc);
      g_warning("Oups, the file %s is not of the assetml type", assetmlfile);
      return;
    }

  rootdir   = xmlGetProp(doc->children,"rootdir");
  mydataset = xmlGetProp(doc->children,"dataset");
  mylocale    = xmlGetProp(doc->children,"locale");

  /* parse our document and replace old data */
  parse_doc(gl_result, doc, mydataset, rootdir, mylocale, dataset, categories, mimetype, locale, file);

  xmlFree(rootdir);
  xmlFree(mydataset);

  xmlFreeDoc(doc);
}


/*
 * Select only files with FILE_EXT
 */
int selectAssetML(const struct dirent *d)
{
  gchar *file = ((struct dirent *)d)->d_name;
  guint ext_length = strlen(FILE_EXT);

  if(strlen(file)<ext_length)
    return 0;

  return (strncmp (&file[strlen(file)-ext_length], FILE_EXT, ext_length) == 0);
}

/* load all the xml files in the assetml path
 * into our memory structures.
 */
void assetml_load_xml(GList **gl_result, gchar *dataset, gchar* categories, gchar* mimetype, gchar *locale, 
		      gchar* name)
{
  struct dirent *one_dirent;
  int n;
  DIR *dir;

  dir = opendir(ASSETML_DIR);
  if(!dir) {
    g_warning("opendir returns no files with extension %s in directory %s", FILE_EXT, ASSETML_DIR);
    return;
  }

  while((one_dirent = readdir(dir)) != NULL) {

    if(strstr(one_dirent->d_name, FILE_EXT)) {
      gchar *assetmlfile = g_strdup_printf("%s/%s", ASSETML_DIR, one_dirent->d_name);
      
      assetml_read_xml_file(gl_result, assetmlfile,
			    dataset, categories, mimetype, locale, name);
      
      g_free(assetmlfile);
    }
  }
  closedir(dir);
}



void free_asset(AssetML *assetml)
{

  xmlFree(assetml->locale);
  xmlFree(assetml->dataset);
  xmlFree(assetml->description);
  xmlFree(assetml->categories);
  xmlFree(assetml->file);
  xmlFree(assetml->mimetype);
  xmlFree(assetml->credits);

  g_free(assetml);
}

void assetml_free_assetlist(GList *assetlist)
{
  g_list_foreach (assetlist, (GFunc) free_asset, NULL);
  g_list_free(assetlist);


}

GList*	 assetml_get_asset(gchar *dataset, gchar* categories, gchar* mimetype, gchar *locale, gchar* file)
{
  GList *gl_result = NULL;

  assetml_load_xml(&gl_result, dataset, categories, mimetype, locale, file);

  if(g_list_length(gl_result)==0)
    {
      g_list_free(gl_result);
      return NULL;
    }
  else
    {
#ifdef DEBUG
      printf("Dumping return value of assetml_get_asset\n");
      g_list_foreach (gl_result, (GFunc) dump_asset, NULL);
#endif
      return gl_result;
    }
}
