/* gcompris - skin.c
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

#include "string.h"
 
#include "skin.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

GHashTable* gcompris_skin_fonts   = NULL;
GHashTable* gcompris_skin_colors  = NULL;
GHashTable* gcompris_skin_numbers = NULL;

guint32 gcompris_skin_color_title;
guint32 gcompris_skin_color_text_button;
guint32 gcompris_skin_color_content;
guint32 gcompris_skin_color_subtitle;
guint32 gcompris_skin_color_shadow;

gchar* gcompris_skin_font_title;
gchar* gcompris_skin_font_subtitle;
gchar* gcompris_skin_font_content;

gchar* gcompris_skin_font_board_tiny;
gchar* gcompris_skin_font_board_small;
gchar* gcompris_skin_font_board_medium;
gchar* gcompris_skin_font_board_big;
gchar* gcompris_skin_font_board_big_bold;
gchar* gcompris_skin_font_board_fixed;
gchar* gcompris_skin_font_board_title;
gchar* gcompris_skin_font_board_title_bold;
gchar* gcompris_skin_font_board_huge;
gchar* gcompris_skin_font_board_huge_bold;


/*
 * Given an image name, return an image name
 * that includes the skin path
 * eg : xx.png -> skins/default/xx.png
 *
 * The caller must free the returned string
 */
gchar *gcompris_image_to_skin(gchar *pixmapfile)
{
  GcomprisProperties	*properties = gcompris_get_properties();
  gchar *filename;

  /* First, test if pixmapfile is in the current skin dir */
  filename = g_strdup_printf("%s/skins/%s/%s", PACKAGE_DATA_DIR, properties->skin, pixmapfile);

  if (g_file_test ((filename), G_FILE_TEST_EXISTS)) {
    g_free(filename);
    
    filename = g_strdup_printf("skins/%s/%s", properties->skin, pixmapfile);

    return(filename);
  }

  /* Now check if pixmapfile is in the default skin dir */

  filename = g_strdup_printf("%s/skins/%s/%s", PACKAGE_DATA_DIR, DEFAULT_SKIN, pixmapfile);
    
  if (g_file_test ((filename), G_FILE_TEST_EXISTS)) {
    g_free(filename);

    filename = g_strdup_printf("skins/%s/%s", DEFAULT_SKIN, pixmapfile);

    return(filename);
  }

  g_warning ("Couldn't find file %s !", filename);

  return(NULL);
}

/*
 * Load a pixmap from the current skin directory
 * If not found, try in the default skin directory
 * If not found abort gcompris
 */
GdkPixbuf *gcompris_load_skin_pixmap(char *pixmapfile)
{
  gchar *filename;
  GdkPixbuf *result_pixbuf;

  filename = gcompris_image_to_skin(pixmapfile);

  result_pixbuf = gcompris_load_pixmap (filename);
  
  g_free(filename);
  
  return (result_pixbuf);
}

/*
 * Utility function used when freeing the memory used by 
 * a hashtable containing strings.
 */ 
void gcompris_skin_free_string(gpointer data)
{
  g_free(data);
}

/*
 * Initialize some common variables 
 * (the one that have to be defined in each skin)
 */
void gcompris_skin_setup_vars(void)
{
  gcompris_skin_color_title = 
    gcompris_skin_get_color_default("gcompris/title", COLOR_TITLE);
  gcompris_skin_color_text_button = 
    gcompris_skin_get_color_default("gcompris/text button", COLOR_TEXT_BUTTON);
  gcompris_skin_color_content = 
    gcompris_skin_get_color_default("gcompris/content", COLOR_CONTENT);
  gcompris_skin_color_subtitle = 
    gcompris_skin_get_color_default("gcompris/subtitle", COLOR_SUBTITLE);
  gcompris_skin_color_shadow = 
    gcompris_skin_get_color_default("gcompris/shadow", COLOR_SHADOW);
  
  gcompris_skin_font_title = 
    gcompris_skin_get_font_default("gcompris/title", FONT_TITLE);
  gcompris_skin_font_subtitle =
    gcompris_skin_get_font_default("gcompris/subtitle", FONT_SUBTITLE);
  gcompris_skin_font_content = 
    gcompris_skin_get_font_default("gcompris/content", FONT_CONTENT);
  
  gcompris_skin_font_board_tiny =
    gcompris_skin_get_font_default("gcompris/board/tiny", FONT_BOARD_TINY);
  gcompris_skin_font_board_small =
    gcompris_skin_get_font_default("gcompris/board/small", FONT_BOARD_SMALL);
  gcompris_skin_font_board_medium =
    gcompris_skin_get_font_default("gcompris/board/medium", FONT_BOARD_MEDIUM);
  gcompris_skin_font_board_big =
    gcompris_skin_get_font_default("gcompris/board/big", FONT_BOARD_BIG);
  gcompris_skin_font_board_big_bold =
    gcompris_skin_get_font_default("gcompris/board/big bold", FONT_BOARD_BIG_BOLD);
  gcompris_skin_font_board_fixed =
    gcompris_skin_get_font_default("gcompris/board/fixed", FONT_BOARD_FIXED);
  gcompris_skin_font_board_title =
    gcompris_skin_get_font_default("gcompris/board/title", FONT_BOARD_TITLE);
  gcompris_skin_font_board_title_bold =
    gcompris_skin_get_font_default("gcompris/board/title bold", FONT_BOARD_TITLE_BOLD);
  gcompris_skin_font_board_huge =
    gcompris_skin_get_font_default("gcompris/board/huge", FONT_BOARD_HUGE);
  gcompris_skin_font_board_huge_bold =
    gcompris_skin_get_font_default("gcompris/board/huge bold", FONT_BOARD_HUGE_BOLD);
}

/*
 * Convert from string a color expressed in the form 0xRRGGBBAA
 * to a unsigned 32 bit integer.
 */
gboolean gcompris_skin_str_to_color(gchar* data, guint32* color){
  char c;
  int i;
  int n = 32;
  guint32 result=0;

  if(strlen(data)<10)
    return FALSE;

  for(i=0; i<8;i++){
    c = data[i+2];
    n -= 4;
    switch(c){
    case '0':
      break;
    case '1':
      result+=(1<<n);
      break;
    case '2':
      result+=(2<<n);
      break;
    case '3':
      result+=(3<<n);
      break;
    case '4':
      result+=(4<<n);
      break;
    case '5':
      result+=(5<<n);
      break;
    case '6':
      result+=(6<<n);
      break;
    case '7':
      result+=(7<<n);
      break;
    case '8':
      result+=(8<<n);
      break;
    case '9':
      result+=(9<<n);
      break;
    case 'a':
    case 'A':
      result+=(10<<n);
      break;
    case 'b':
    case 'B':
      result+=(11<<n);
      break;
    case 'c':
    case 'C':
      result+=(12<<n);
      break;
    case 'd':
    case 'D':
      result+=(13<<n);
      break;
    case 'e':
    case 'E':
      result+=(14<<n);
      break;
    case 'f':
    case 'F':
      result+=(15<<n);
      break;
    default:
      return FALSE;
    }
  }
  *color = result;
  return TRUE;
}


/*
 * Parse a skin.xml file located in the skin directory
 * and load the skin properties into memory
 */
void gcompris_skin_xml_load (gchar* skin)
{
  gchar* xmlfilename;
  xmlDocPtr xmldoc;
  xmlNodePtr skinNode;
  xmlNodePtr node;
  gchar* key;
  gchar* data;
  guint32 color;

  g_return_if_fail(skin!=NULL);
  
  xmlfilename = g_strdup_printf("%s/skins/%s/skin.xml", PACKAGE_DATA_DIR, skin);

  /* if the file doesn't exist */
  if(!g_file_test(xmlfilename, G_FILE_TEST_EXISTS))
    {
      g_warning("Couldn't find file %s !", xmlfilename);
      return;
    }

  xmldoc = xmlParseFile(xmlfilename);
  g_free(xmlfilename);  

  if(!xmldoc)
    return;

  if(/* if there is no root element */
     !xmldoc->children ||
     /* if it doesn't have a name */
     !xmldoc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp((gchar *)xmldoc->children->name, "GCompris")!=0) {
    g_warning("No Gcompris node");
    xmlFreeDoc(xmldoc);
    return;
  }

  skinNode = xmldoc->children->children;
  while((skinNode!=NULL)&&(skinNode->type!=XML_ELEMENT_NODE))
    skinNode = skinNode->next;
  
  if((skinNode==NULL)||
     g_strcasecmp((gchar *)skinNode->name, "Skin")!=0) {
    g_warning("No Skin node %s", xmldoc->children->children->name);
    xmlFreeDoc(xmldoc);
    return;
  }

  node = skinNode->children;
  while(node !=NULL)
    {
      if(g_strcasecmp((gchar *)node->name, "color")==0){
	key = (gchar *)xmlGetProp(node,  BAD_CAST "id");
	data =(gchar *) xmlGetProp(node,  BAD_CAST "rgba");
	if((key!=NULL)&&(data!=NULL)){
	  if(gcompris_skin_str_to_color(data, &color)){
	    g_hash_table_insert(gcompris_skin_colors, key, GUINT_TO_POINTER(color));
	  } else {
	    if(key!=NULL) g_free(key);
	  }	
	}
	if(data!=NULL) g_free(data);
      } 
      else if(g_strcasecmp((gchar *)node->name, "font")==0){
	key = (gchar *)xmlGetProp(node,  BAD_CAST "id");
	data = (gchar *)xmlGetProp(node,  BAD_CAST "name");
	if((key!=NULL)&&(data!=NULL)){
	  g_hash_table_insert(gcompris_skin_fonts, key, data);
	} else {
	  if(key!=NULL) g_free(key);
	  if(data!=NULL) g_free(data);
	}
      }
      else if(g_strcasecmp((gchar *)node->name, "number")==0){
	key = (gchar *)xmlGetProp(node, BAD_CAST "id");
	data = (gchar *)xmlGetProp(node, BAD_CAST "value");
	if((key!=NULL)&&(data!=NULL)){
	  int number = atoi(data);
	  g_hash_table_insert(gcompris_skin_numbers, key, GUINT_TO_POINTER(number));
	} else {
	  if(key!=NULL) g_free(key);
	  if(data!=NULL) g_free(data);
	}	
      }
      node = node->next;
    }

  xmlFreeDoc(xmldoc);
}

/*
 * Parse the default skin.xml file and the one located in the skin 
 * directory then load all skins properties into memory
 */
void gcompris_skin_load (gchar* skin)
{

  if(skin==NULL)
    return;

  gcompris_skin_free();
  
  gcompris_skin_fonts = g_hash_table_new_full(g_str_hash, g_str_equal,
					      gcompris_skin_free_string,
					      gcompris_skin_free_string);
  gcompris_skin_colors = g_hash_table_new_full(g_str_hash, g_str_equal,
					      gcompris_skin_free_string,
					      NULL);
  gcompris_skin_numbers = g_hash_table_new_full(g_str_hash, g_str_equal,
					      gcompris_skin_free_string,
					      NULL);
  gcompris_skin_xml_load(DEFAULT_SKIN);
  if(strcmp(skin,DEFAULT_SKIN)!=0)
    gcompris_skin_xml_load(skin);

  gcompris_skin_setup_vars();
}

/*
 * Free the memory used to store the skin properties.
 */
void gcompris_skin_free (void)
{
  if(gcompris_skin_fonts!=NULL)
    g_hash_table_destroy(gcompris_skin_fonts);

  if(gcompris_skin_colors!=NULL)
    g_hash_table_destroy(gcompris_skin_colors);
}

/*
 * Get the skin color associated to the id
 */
guint32 gcompris_skin_get_color_default(gchar* id, guint32 def)
{
  gpointer result;
  result = g_hash_table_lookup(gcompris_skin_colors, (gpointer)id);
  if(result!=NULL)
    return GPOINTER_TO_UINT(result);
  return def;
}

/*
 * Get the skin gdkcolor associated to the id
 *
 * The color is returned in the given gdkcolor
 */
void gcompris_skin_get_gdkcolor_default(gchar* id, guint32 def, GdkColor *gdkcolor)
{
  gchar *tmp;
  guint32 color;

  color = gcompris_skin_get_color_default(id, def);

  tmp = g_strdup_printf("#%06X", gcompris_skin_get_color(id) >> 8);
  gdk_color_parse(tmp, gdkcolor);
  g_free(tmp);
}

/*
 * Get the skin font name associated to the id
 */
gchar* gcompris_skin_get_font_default(gchar* id, gchar* def)
{
  gpointer result;
  result = g_hash_table_lookup(gcompris_skin_fonts, (gpointer)id);
  if(result!=NULL)
    return (gchar*)result;
  return def;
}

/*
 * Get the skin 'number' associated to the id
 */
guint32 gcompris_skin_get_number_default(gchar* id, guint32 def)
{
  gpointer result;
  result = g_hash_table_lookup(gcompris_skin_numbers, (gpointer)id);
  if(result!=NULL)
    return GPOINTER_TO_UINT(result);
  return def;
}
