/* gcompris - anim.c
 *
 * Time-stamp: <2005/02/13 11:59:55 joe>
 *
 * Copyright (C) 2005 Joe Neeman
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

#include "anim.h"
#include <stdio.h>

#define TICK_TIME 20

/* the list of active animations; we need to update it if active != NULL */
static GSList *active;

/* private callback */
static gboolean anim_tick(void*);

GcomprisAnimation *gcompris_load_animation(char *filename)
{
  FILE *f = NULL;

  gchar *absolute_filename;

  absolute_filename = gcompris_find_absolute_filename(filename);

  if (absolute_filename){
    f = fopen(absolute_filename, "r");
    g_free(absolute_filename);
  }

  if(!f)
    {
      g_warning("Couldn't open animation-spec file '%s'\n", filename);
      return NULL;
    }

  char tmp[100];
  GSList *files = NULL;
  GcomprisAnimation *anim = NULL;

  /* read filenames, one per line, from the animation spec-file */
  while(fscanf(f, "%99s", tmp) == 1)
    {
      GcomprisBoard   *gcomprisBoard = get_current_gcompris_board();
      files = g_slist_append(files, 
                             g_strdup_printf("%s/%s", gcomprisBoard->board_dir, tmp));
    }

  anim = g_malloc(sizeof(GcomprisAnimation));
  anim->numstates = g_slist_length(files);
  anim->anim = g_malloc(sizeof(GdkPixbuf*) * anim->numstates);

  /* open the animations and assign them */
  GError *error = NULL;
  GSList *cur;
  char *name;
  int i;
  for(cur=files, i=0; cur; cur = g_slist_next(cur), i++)
    {
      name = (char*) cur->data;
      anim->anim[i] = gdk_pixbuf_animation_new_from_file(name, &error);
      g_warning("Opened animation %s\n", name);
      if(!anim->anim[i])
        {
          g_critical("Couldn't open animation %s: %s\n", name, error->message);
          return NULL;
        }
      g_free(name);
    }
  g_slist_free(files);
  return anim;
}

GcomprisAnimCanvasItem *gcompris_activate_animation(GnomeCanvasGroup *parent,
                                                    GcomprisAnimation *anim)
{
  GcomprisAnimCanvasItem *item = g_malloc(sizeof(GcomprisAnimCanvasItem));

  item->state = 0;
  item->anim = anim;
  item->iter = gdk_pixbuf_animation_get_iter(anim->anim[0], NULL);
  item->canvas = (GnomeCanvasPixbuf*) gnome_canvas_item_new(
                            parent,
                            GNOME_TYPE_CANVAS_PIXBUF,
                            "pixbuf",
                            gdk_pixbuf_animation_iter_get_pixbuf(item->iter),
                            NULL);

  if(active == NULL)
      g_timeout_add(TICK_TIME, (GSourceFunc)anim_tick, NULL);

  active = g_slist_append(active, item);
  return item;
}

void gcompris_swap_animation(GcomprisAnimCanvasItem *item, GcomprisAnimation *new_anim)
{
  item->anim = new_anim;
  gcompris_set_anim_state(item, 0);
}

void gcompris_deactivate_animation(GcomprisAnimCanvasItem *item)
{
  GSList *node = g_slist_find( active, item );
  if( !node )
  {
    g_critical( "Tried to deactive non-existant animation" );
    return;
  }

  if (GNOME_IS_CANVAS_ITEM(item->canvas)){
    gtk_object_destroy(GTK_OBJECT(item->canvas));
    item->canvas = NULL;
  }

  active = g_slist_delete_link(active, node);
  g_object_unref(item->iter);
  g_free(item);
}

void gcompris_free_animation(GcomprisAnimation *anim)
{
  int i;
  for(i=0; i<anim->numstates; i++)
    {
      g_object_unref(anim->anim[i]);
    }
  g_free(anim);
}

void gcompris_set_anim_state(GcomprisAnimCanvasItem *item, int state)
{
  if(state < item->anim->numstates)
    {
      item->state = state;
    }
  else
    {
      item->state = 0;
    }
  g_object_unref( item->iter );
  item->iter = gdk_pixbuf_animation_get_iter( item->anim->anim[item->state],
                                              NULL );
  gnome_canvas_item_set( (GnomeCanvasItem*)item->canvas, "pixbuf",
                         gdk_pixbuf_animation_iter_get_pixbuf(item->iter),
                         NULL);
}

/* private callback functions */

static gboolean anim_tick(void *ignore)
{
  if(active == NULL)
    {
      g_warning("deactivating anim_tick\n");
      return FALSE;
    }

  GSList *cur;
  for(cur=active; cur; cur = g_slist_next(cur))
    {
      GcomprisAnimCanvasItem *a = (GcomprisAnimCanvasItem*)cur->data;
      if( gdk_pixbuf_animation_iter_advance( a->iter, NULL) )
        {
          gnome_canvas_item_set((GnomeCanvasItem*)a->canvas, "pixbuf", 
                                gdk_pixbuf_animation_iter_get_pixbuf(a->iter),
                                NULL);
        }
    }
  return TRUE;
}

