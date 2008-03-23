/* gcompris - anim.c
 *
 * Copyright (C) 2005, 2008 Joe Neeman
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

#include "anim.h"
#include <glib/gstdio.h>

#define TICK_TIME 20

/* the list of active animations; we need to update it if active != NULL */
static GSList *active;

/* private callback */
static gboolean anim_tick(void*);

GcomprisAnimation *
gc_anim_load(char *filename)
{
  FILE *f = NULL;

  gchar *absolute_filename;

  absolute_filename = gc_file_find_absolute(filename);

  if (absolute_filename){
    f = g_fopen(absolute_filename, "r");
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
    files = g_slist_append(files,
			   g_strdup_printf("%s/%s",
					   gc_prop_get()->package_data_dir, tmp));
  fclose(f);
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

GcomprisAnimCanvasItem *
gc_anim_activate(GooCanvasItem *parent,
		 GcomprisAnimation *anim)
{
  GcomprisAnimCanvasItem *item;

  g_assert(parent != NULL);
  //  g_assert(anim != NULL);

  item = g_malloc(sizeof(GcomprisAnimCanvasItem));

  item->state = 0;
  item->anim = anim;
  item->iter = gdk_pixbuf_animation_get_iter(anim->anim[0], NULL);
  item->canvas = goo_canvas_image_new(parent,
				      gdk_pixbuf_animation_iter_get_pixbuf(item->iter),
				      0, 0,
				      NULL);

  if(active == NULL)
      g_timeout_add(TICK_TIME, (GSourceFunc)anim_tick, NULL);

  active = g_slist_append(active, item);
  return item;
}

void
gc_anim_swap(GcomprisAnimCanvasItem *item, GcomprisAnimation *new_anim)
{
  item->anim = new_anim;
  gc_anim_set_state(item, 0);
}

void
gc_anim_deactivate(GcomprisAnimCanvasItem *item)
{
  GSList *node = g_slist_find( active, item );
  if( !node )
  {
    g_critical( "Tried to deactive non-existant animation" );
    return;
  }

  if (GOO_IS_CANVAS_ITEM(item->canvas)){
    goo_canvas_item_remove(item->canvas);
    item->canvas = NULL;
  }

  active = g_slist_delete_link(active, node);
  g_object_unref(item->iter);
  g_free(item);
}

void
gc_anim_free(GcomprisAnimation *anim)
{
  int i;
  for(i=0; i<anim->numstates; i++)
    {
      g_object_unref(anim->anim[i]);
    }
  g_free(anim->anim);
  g_free(anim);
}

void
gc_anim_set_state(GcomprisAnimCanvasItem *item, int state)
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
  g_object_set( (GooCanvasItem*)item->canvas,
		"pixbuf",
		gdk_pixbuf_animation_iter_get_pixbuf(item->iter),
		NULL);
}

/* private callback functions */

static gboolean
anim_tick(void *ignore)
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
          g_object_set(a->canvas, "pixbuf",
		       gdk_pixbuf_animation_iter_get_pixbuf(a->iter),
		       NULL);
        }
    }
  return TRUE;
}

