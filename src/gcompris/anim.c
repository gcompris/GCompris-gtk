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

/* used for keeping track of active animations */
typedef struct {
  GnomeCanvasGroup *parent;
  GnomeCanvasPixbuf *item;
  GcomprisAnimation *anim;
  GdkPixbufAnimationIter *iter;
} ActiveAnim;

/* a list of ActiveAnim*s */
static GSList *active_list;

/* private callback */
static gboolean anim_tick(ActiveAnim*);

/* compare function to test if an ActiveAnim has a particular CanvasPixbuf */
static gint anim_item_cmpr(const ActiveAnim*, const ActiveAnim*);
static gint anim_anim_cmpr(const ActiveAnim*, const ActiveAnim*);

GcomprisAnimation *gcompris_load_animation(char *filename)
{
  GIOChannel *ch;
  if(filename[0] == '/')  /* we were probably called by load_animation_asset */
    {
      ch = g_io_channel_new_file(filename, "r", NULL);
    }
  else
    {
      gchar *tmp = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, filename);
      ch = g_io_channel_new_file(tmp, "r", NULL);
      g_free(tmp);
    }

  if(!ch)
    {
      g_warning("Couldn't open animation-spec file\n");
      return NULL;
    }

  gchar *tmp = NULL;
  GSList *files = NULL;
  GcomprisAnimation *anim = NULL;

  /* read whitespace-separated filenames form the animation spec-file */
  while(g_io_channel_read_line(ch, &tmp, NULL, NULL, NULL)==G_IO_STATUS_NORMAL)
    {
      char *c;
      for(c=tmp; *c && !isspace(*c); c++) /* get rid of everything from the
                                           * first whitespace onwards */
          ;
      *c = '\0';
      files = g_slist_append(files, tmp);
    }

  anim = g_malloc(sizeof(GcomprisAnimation));
  anim->numstates = g_slist_length(files);
  anim->anim = g_malloc(sizeof(GdkPixbuf*) * anim->numstates);

  /* open the animations and assign them */
  GError *error = NULL;
  GSList *cur;
  int i;
  for(cur=files, i=0; cur; cur = g_slist_next(cur), i++)
    {
      tmp = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, (char*)cur->data);
      anim->anim[i] = gdk_pixbuf_animation_new_from_file(tmp, &error);
      printf("Opened animation %s\n", tmp);
      g_free(tmp);
      if(!anim->anim[i])
        {
          tmp = g_strdup_printf("Couldn't open animation %s: %s\n",
                                (char*)cur->data,
                                error->message);
          printf("%s\n", tmp);
          g_free(tmp);
        }
    }
  return anim;
}

GcomprisAnimation *gcompris_load_animation_asset(gchar *dataset,
                                                 gchar *categories,
                                                 gchar *mimetype,
                                                 gchar *name)
{
  gchar *file = NULL;

  file = gcompris_get_asset_file(dataset, categories, mimetype, name);

  if(file)
    return gcompris_load_animation(file);

  return NULL;
}

GnomeCanvasPixbuf *gcompris_activate_animation(GnomeCanvasGroup *parent,
                                               GcomprisAnimation *anim)
{
  ActiveAnim *active = g_malloc(sizeof(ActiveAnim));

  active->parent = parent;
  active->anim = anim;
  active->iter = gdk_pixbuf_animation_get_iter(anim->anim[0], NULL);
  active->item = (GnomeCanvasPixbuf*) gnome_canvas_item_new(
                            parent,
                            GNOME_TYPE_CANVAS_PIXBUF,
                            "pixbuf",
                            gdk_pixbuf_animation_iter_get_pixbuf(active->iter),
                            NULL);

  if(active_list == NULL)
      g_timeout_add(TICK_TIME, (GSourceFunc)anim_tick, NULL);

  /* update the active list */
  active_list = g_slist_append(active_list, active);

  return active->item;
}

void gcompris_deactivate_animation(GnomeCanvasItem *item)
{
  ActiveAnim dummy;
  dummy.item = (GnomeCanvasPixbuf*)item;

  GSList *active_node = g_slist_find_custom(active_list,
                                            &dummy,
                                            (GCompareFunc)anim_item_cmpr);

  if(!active_node)
    {
      g_warning("tried to deactivate inactive animation\n");
      return;
    }

  ActiveAnim *active = (ActiveAnim*) active_node->data;
  g_object_unref(active->iter);
  g_object_unref(active->item);
  g_free(active);

  active_list = g_slist_delete_link(active_list, active_node);
}

gboolean gcompris_free_animation(GcomprisAnimation *anim)
{
  ActiveAnim dummy;
  dummy.anim = anim;

  /* if any active animations are using anim, abort */
  if(g_slist_find_custom(active_list, &dummy, (GCompareFunc)anim_anim_cmpr)
      != NULL)
    {
      g_warning("Tried to free an active animation\n");
      return FALSE;
    }

  int i;
  for(i=0; i<anim->numstates; i++)
    {
      g_object_unref(anim->anim[i]);
    }
  g_free(anim);
  return TRUE;
}

/* private callback functions */

static gboolean anim_tick(ActiveAnim *ignore)
{
  if(active_list == NULL)
      return FALSE;

  GSList *cur;
  for(cur=active_list; cur; cur = g_slist_next(cur))
    {
      ActiveAnim *a = (ActiveAnim*)cur->data;
      if( gdk_pixbuf_animation_iter_advance( a->iter, NULL) )
        {
          gnome_canvas_item_set((GnomeCanvasItem*)a->item, "pixbuf", 
                                gdk_pixbuf_animation_iter_get_pixbuf(a->iter),
                                NULL);
        }
    }
  return TRUE;
}

static gint anim_item_cmpr(const ActiveAnim *a1, const ActiveAnim *a2)
{
  if(a1->item == a2->item)
      return 0;
  return 1;
}

static gint anim_anim_cmpr(const ActiveAnim *a1, const ActiveAnim *a2)
{
  if(a1->anim == a2->anim)
      return 0;
  return 1;
}
