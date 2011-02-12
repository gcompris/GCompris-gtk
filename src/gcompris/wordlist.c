/* gcompris - wordlist.h
 *
 * Copyright (C) 2003, 2008 GCompris Developpement Team
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

void gc_wordlist_dump(GcomprisWordlist *wl);

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
  guint level;

  GcomprisWordlist     *wordlist;
  xmlChar              *text;

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
      g_free(filename);
      return NULL;
    }

  g_warning("Wordlist found %s\n", xmlfilename);

  xmldoc = xmlParseFile(xmlfilename);

  if(!xmldoc){
    g_warning("Couldn't parse file %s !", xmlfilename);
    g_free(filename);
    g_free(xmlfilename);
    return NULL;
  }
  g_free(xmlfilename);

  if(/* if there is no root element */
     !xmldoc->children ||
     /* if it doesn't have a name */
     !xmldoc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp((gchar *)xmldoc->children->name,(gchar *)"GCompris")!=0) {
    g_warning("No Gcompris node");
    xmlFreeDoc(xmldoc);
    g_free(filename);
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
    g_free(filename);
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

    level=-1;
    text = xmlGetProp ( node,
		    (const xmlChar *) "value");
    if (text) {
	    level = atoi((gchar *) text);
	    xmlFree (text);
    }
    text = xmlNodeGetContent ( wordsNode);
    gc_wordlist_set_wordlist(wordlist, level, (const gchar*)text);
    xmlFree(text);

    node = node->next;
  }
  xmlFreeDoc(xmldoc);
  return wordlist;
}

void gc_wordlist_dump(GcomprisWordlist *wl)
{
	GSList *level, *words;

	printf("Wordlist dump\n");
	printf("filename:%s\n",wl->filename);
	printf("number of level:%d\n",wl->number_of_level);

	for(level = wl->levels_words; level; level = level->next)
	{
		printf("Level %d\n", ((LevelWordlist*)level->data)->level);
		printf("Words :");
		for(words=((LevelWordlist*)level->data)->words; words; words=words->next)
		{
			printf(" %s", (char*)words->data);
		}
		puts("");
	}
}

LevelWordlist*gc_wordlist_get_levelwordlist(GcomprisWordlist *wordlist, guint level)
{
	GSList *lev_list, *list;
	LevelWordlist *lw;

	if(!wordlist)
		return NULL;

	lev_list = wordlist->levels_words;

	for (list = lev_list; list != NULL; list = list->next)
	{
		lw = list->data;
		if(lw->level == level)
			return lw;
	}
	return NULL;
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
	LevelWordlist *lw;
	gchar *word;

	if(level>wordlist->number_of_level)
		level = wordlist->number_of_level;

	lw = gc_wordlist_get_levelwordlist(wordlist, level);
	if(!lw)
		return NULL;

	g_warning("Level : %d", lw->level);

	/* We got the proper level, find a random word */
	word = (gchar *)g_slist_nth_data(lw->words,
			RAND(0, g_slist_length(lw->words))
			);
	g_warning("returning random word '%s'", word);
	return(g_strdup(word));
}

static void gc_wordlist_free_level(LevelWordlist *lw)
{
	GSList *words;

	if(!lw)
		return;

	for (words = lw->words; words !=NULL; words = words->next)
		g_free(words->data);
	g_slist_free(lw->words);
	g_free(lw);
}

/** call it to free a GcomprisWordlist as returned by gc_wordlist_get_from_file
 *
 * \param wordlist
 *
 */
void
gc_wordlist_free(GcomprisWordlist *wordlist)
{
  GSList *list;

  if(!wordlist)
    return;

  g_free ( wordlist->filename);
  g_free ( wordlist->description);
  g_free ( wordlist->locale);
  g_free ( wordlist->name);

  for ( list = wordlist->levels_words; list !=NULL; list=list->next){
    LevelWordlist *lw = (LevelWordlist *)list->data;
    gc_wordlist_free_level(lw);
  }
  g_slist_free ( wordlist->levels_words);
  g_free (wordlist);
}

void gc_wordlist_set_wordlist(GcomprisWordlist *wordlist, guint level, const gchar*text)
{
	LevelWordlist *lw;
	GSList *words=NULL;
	gchar **wordsArray;
	int i;
	
	g_warning("wordlist : add level=%d text=%s\n", level, text);
	/* remove level */
	if((lw = gc_wordlist_get_levelwordlist(wordlist, level)))
	{
		g_warning("remove level %d", lw->level);
		wordlist->levels_words = g_slist_remove(wordlist->levels_words, lw);
		gc_wordlist_free_level(lw);
		wordlist->number_of_level--;
	}

	/* add new level */
	wordsArray = g_strsplit_set (text, " \n\t", 0);

	i=0;
	for(i=0;wordsArray[i] != NULL; i++)
		if (wordsArray[i][0]!='\0' &&
			!g_slist_find_custom(words, wordsArray[i], (GCompareFunc)strcmp))
				words = g_slist_append( words, g_strdup(wordsArray[i]));

	g_strfreev ( wordsArray);

	if(words==NULL)
		return;

	/* initialise LevelWordlist struct */
	LevelWordlist *level_words = g_malloc0(sizeof(LevelWordlist));

	level_words->words = words;
	level_words->level = level;

	wordlist->number_of_level++;
	wordlist->levels_words = g_slist_append( wordlist->levels_words, level_words);
}

void gc_wordlist_save(GcomprisWordlist *wordlist)
{
	GSList *listlevel,*listword;
	LevelWordlist *level;
	gchar *filename, *tmp;
	xmlNodePtr wlnode, levelnode, node;
	xmlDocPtr doc;

	if(!wordlist)
		return;
	doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
	if(!doc)
		return;

	node = xmlNewNode(NULL, BAD_CAST "GCompris");
	xmlDocSetRootElement(doc,node);
	wlnode = xmlNewChild(node, NULL, BAD_CAST "Wordlist", NULL);

	if(wordlist->name)
		xmlSetProp(wlnode, BAD_CAST "name", BAD_CAST wordlist->name);
	if(wordlist->description)
		xmlSetProp(wlnode, BAD_CAST "description", BAD_CAST wordlist->description);
	if(wordlist->locale)
		xmlSetProp(wlnode, BAD_CAST "locale", BAD_CAST wordlist->locale);
	for(listlevel = wordlist->levels_words; listlevel; listlevel = listlevel->next)
	{
		level = (LevelWordlist*)listlevel->data;
		levelnode = xmlNewChild(wlnode, NULL, BAD_CAST "level", NULL);
		if((tmp = g_strdup_printf("%d", level->level)))
		{
			xmlSetProp(levelnode, BAD_CAST "value", BAD_CAST tmp);
			g_free(tmp);
		}
		for(listword = level->words; listword; listword=listword->next)
		{
			xmlNodeAddContent(levelnode, BAD_CAST listword->data);
			xmlNodeAddContent(levelnode, BAD_CAST " ");
		}
	}

	filename = gc_file_find_absolute_writeable(wordlist->filename);
	if(filename)
	{
		if(xmlSaveFormatFileEnc(filename, doc, NULL, 1)<0)
		{
			g_warning("Fail to write %s", filename);
			g_free(filename);
		}
		g_free(filename);
	}
	xmlFreeDoc(doc);
}

