/* gcompris - wordlist.h
 *
 * Copyright (C) 2003 GCompris Developpement Team
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gcompris.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

GcomprisWordlist     *gcompris_get_wordlist_from_file(gchar *filename)
{
  gchar* xmlfilename;
  xmlDocPtr xmldoc;
  xmlNodePtr wlNode;
  xmlNodePtr node;
  int i;

  GcomprisWordlist     *wordlist;
  xmlChar              *text;

  gchar               **wordsArray;              
  GList                *words = NULL;

  GcomprisProperties	*properties = gcompris_get_properties ();

  xmlfilename = g_strdup_printf("%s/wordsgame/%s.xml", properties->shared_dir, filename);

  /* if the file doesn't exist */
  if(!g_file_test(xmlfilename, G_FILE_TEST_EXISTS))
    {
      g_free(xmlfilename);
      xmlfilename = g_strdup_printf("%s/wordsgame/%s.xml", properties->package_data_dir, filename);
      if(!g_file_test(xmlfilename, G_FILE_TEST_EXISTS)){
	g_warning(_("Couldn't find file %s !"), xmlfilename);
	g_free(xmlfilename);
	return NULL;
      }
    }

  g_warning("Wordlist found %s\n", xmlfilename);

  xmldoc = xmlParseFile(xmlfilename);
  g_free(xmlfilename);  

  if(!xmldoc){
    g_warning(_("Couldn't parse file %s !"), xmlfilename);
    return NULL;
  }

  if(/* if there is no root element */
     !xmldoc->children ||
     /* if it doesn't have a name */
     !xmldoc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp((gchar *)xmldoc->children->name,(gchar *)"GCompris")!=0) {
    g_warning("No Gcompris node");
    xmlFreeDoc(xmldoc);
    return NULL;
  }

  /* there is only one element child */
  wlNode = xmldoc->children->children;
  while((wlNode!=NULL)&&(wlNode->type!=XML_ELEMENT_NODE))
    wlNode = wlNode->next;

 if((wlNode==NULL)||
     g_strcasecmp((gchar *)wlNode->name,"wordlist")!=0) {
    g_warning("No wordlist node %s", (wlNode == NULL) ? (gchar *)wlNode->name : "NULL node");
    xmlFreeDoc(xmldoc);
    return NULL;
  }
 
 node = wlNode->children;
 while((node!=NULL)&&(node->type!=XML_TEXT_NODE))
    node = node->next;

 if (!node){
   g_warning("No wordlist text node %s", wlNode->name);
   return NULL;
 }

 /* ok, we can process the wordlist */
 wordlist = g_malloc0(sizeof(GcomprisWordlist));

 wordlist->filename = g_strdup(filename);
 text = xmlGetProp ( wlNode,
		     (const xmlChar *) "description");
 if (text) {
   wordlist->description = g_strdup ((gchar *) text);
   xmlFree (text);
 }

 text = xmlGetProp ( wlNode,
		     (const xmlChar *) "locale");
 if (text) {
   wordlist->locale = g_strdup ((gchar *) text);
   xmlFree (text);
 }
 text = xmlGetProp ( wlNode,
		     (const xmlChar *) "level");
 if (text) {
   wordlist->level = g_strdup ((gchar *) text);
   xmlFree (text);
 }

 text = xmlNodeGetContent ( node);
 
 wordsArray = g_strsplit_set ((const gchar *) text,
			      (const gchar *) " \n\t",
			      0);
 
 xmlFree (text);

 i=0;
 while (wordsArray[i] != NULL) {
   words = g_list_append( words, g_strdup( wordsArray[i++]));
 }

 g_strfreev ( wordsArray);

 wordlist->words = words;

 return wordlist;
}

 void gcompris_wordlist_free(GcomprisWordlist *wordlist)
{
  GList *list;
  g_free ( wordlist->filename);
  g_free ( wordlist->description);
  g_free ( wordlist->locale);
  g_free ( wordlist->level);
  
  for ( list = wordlist->words; list !=NULL; list=list->next)
    g_free(list->data);

  g_list_free ( wordlist->words);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
