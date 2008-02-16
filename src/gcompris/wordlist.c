/* gcompris - wordlist.h
 *
 * Copyright (C) 2003 GCompris Developpement Team
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <string.h>

#include "gcompris.h"

/** Load a wordlist formatted xml file. It contains a list of words.
 *
 * The xml file format must be like this:
 * <?xml version="1.0" encoding="UTF-8" standalone="no"?>
 * <GCompris>
 * <Wordlist name="default-da" description="Default Danish" locale="da">
 * <level value="1">
 * word1
 * word2
 * ...
 * </level>
 * <level value="2">
 * word3
 * word4
 * ...
 * </level>
 * <level value="3">
 * ...
 * </level>
 * </Wordlist>
 * </GCompris>
 *
 *
 * \param format: the xml file to load (ex: wordsgame/default-fr.xml)
 *                If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return a new GcomprisWordlist or NULL
 */
GcomprisWordlist
*gc_wordlist_get_from_file(const gchar *format, ...)
{
  va_list args;
  gchar* xmlfilename;
  gchar* filename;
  xmlDocPtr xmldoc;
  xmlNodePtr wlNode;
  xmlNodePtr node;
  xmlNodePtr wordsNode;
  int i;
  guint number_of_level = 0;

  GcomprisWordlist     *wordlist;
  xmlChar              *text;

  gchar               **wordsArray;
  GSList                *words = NULL;

  if (!format)
    return NULL;

  va_start (args, format);
  filename = g_strdup_vprintf (format, args);
  va_end (args);

  xmlfilename = gc_file_find_absolute(filename);

  /* if the file doesn't exist */
  if(!xmlfilename)
    {
      g_warning("Couldn't find file %s !", filename);
      g_free(xmlfilename);
      return NULL;
    }

  g_warning("Wordlist found %s\n", xmlfilename);

  xmldoc = xmlParseFile(xmlfilename);
  g_free(xmlfilename);

  if(!xmldoc){
    g_warning("Couldn't parse file %s !", xmlfilename);
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
     g_strcasecmp((gchar *)wlNode->name,"Wordlist")!=0) {
    g_warning("No wordlist node %s", (wlNode == NULL) ? (gchar *)wlNode->name : "NULL node");
    xmlFreeDoc(xmldoc);
    return NULL;
  }


  /* ok, we can process the wordlist */
  wordlist = g_malloc0(sizeof(GcomprisWordlist));

  wordlist->filename = filename;

  /* Get name */
  text = xmlGetProp ( wlNode,
		      (const xmlChar *) "name");

  if (text) {
    wordlist->name = g_strdup ((gchar *) text);
    xmlFree (text);
  }


  /* Get description */
  text = xmlGetProp ( wlNode,
		      (const xmlChar *) "description");
  if (text) {
    wordlist->description = g_strdup ((gchar *) text);
    xmlFree (text);
  }

  /* Get locale */
  text = xmlGetProp ( wlNode,
		      (const xmlChar *) "locale");
  if (text) {
    wordlist->locale = g_strdup ((gchar *) text);
    xmlFree (text);
  }

  /* Levels loop */

  node = wlNode->children;
  while((node!=NULL)) {
    words = NULL;
    if (node->type!=XML_ELEMENT_NODE){
      node = node->next;
      continue;
    }

    if (strcmp((char *)node->name,"level")!=0){
      g_warning("Parsing %s error", filename);
      break;
    }

    wordsNode = node->children;
    if (wordsNode->type!=XML_TEXT_NODE){
      g_warning("Parsing %s error", filename);
      break;
    }

    text = xmlNodeGetContent ( wordsNode);

    wordsArray = g_strsplit_set ((const gchar *) text,
				 (const gchar *) " \n\t",
				 0);

    g_warning("Wordlist read : %s", text);

    xmlFree (text);

    i=0;
    while (wordsArray[i] != NULL) {
      if (wordsArray[i][0]!='\0')
	words = g_slist_append( words, g_strdup( wordsArray[i]));
      i++;
    }

    g_strfreev ( wordsArray);


    /* initialise LevelWordlist struct */
    LevelWordlist *level_words = g_malloc0(sizeof(LevelWordlist));

    number_of_level++;

    text = xmlGetProp ( node,
			(const xmlChar *) "value");
    if (text) {
      level_words->level = atoi((gchar *) text);
      xmlFree (text);
    }

    level_words->words = words;

    wordlist->levels_words = g_slist_append( wordlist->levels_words, level_words);

    node = node->next;
  }

  wordlist->number_of_level = number_of_level;

  return wordlist;
}

/** get a random word from the wordlist in the given level
 *
 * \param wordlist: the wordlist
 * \param level: the level
 *
 * \return a newly allocated word or NULL
 */
gchar *
gc_wordlist_random_word_get(GcomprisWordlist *wordlist, guint level)
{
  GSList *lev_list, *list;

  if(!wordlist)
    return NULL;

  lev_list = wordlist->levels_words;

  /* cap to the number_of_level */
  if(level > wordlist->number_of_level)
    level = wordlist->number_of_level;

  for (list = lev_list; list != NULL; list = list->next)
    {
      LevelWordlist *lw = list->data;

      if(lw->level == level)
	{
	  gchar *word;
	  g_warning("Level : %d", lw->level);

	  /* We got the proper level, find a random word */
	  word = (gchar *)g_slist_nth_data(lw->words,
					  RAND(0, g_slist_length(lw->words)-1)
					  );
	  g_warning("returning random word '%s'", word);
	  return(g_strdup(word));
	}
    }

  return NULL;
}

/** call it to free a GcomprisWordlist as returned by gc_wordlist_get_from_file
 *
 * \param wordlist
 *
 */
void
gc_wordlist_free(GcomprisWordlist *wordlist)
{
  GSList *list, *words;

  if(!wordlist)
    return;

  g_free ( wordlist->filename);
  g_free ( wordlist->description);
  g_free ( wordlist->locale);
  g_free ( wordlist->name);

  for ( list = wordlist->levels_words; list !=NULL; list=list->next){
    LevelWordlist *lw = (LevelWordlist *)list->data;
    for ( words = lw->words; words !=NULL; words = words->next)
      g_free(words->data);
    g_slist_free(lw->words);
    g_free(lw);
  }
  g_slist_free ( wordlist->levels_words);
  g_free (wordlist);
}
