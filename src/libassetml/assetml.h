/* gcompris - assetml.h
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

#ifndef ASSETML_H
#define ASSETML_H

#include <glib.h>

/* Returns a list of AssetML struct
   dataset: the name of the dataset, NULL for ANY dataset
   categories: a list of categorie columns ';' separated (NULL for all)
   mimetype: an official mimetype describing the content (NULL for all)
   name: the asset name to get or NULL for any name
*/
   
GList*	 assetml_get_asset(gchar *dataset, gchar* categories, gchar* mimetype, gchar* name);
void	 assetml_free_assetlist(GList *assetlist);

typedef struct _AssetML         AssetML;

struct _AssetML
{
  gchar		*dataset;
  gchar		*categories;
  gchar		*name;
  gchar		*name_noi18n;
  gchar		*description;
  gchar		*file;
  gchar		*mimetype;
  gchar		*credits;
};


#endif
