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

/*! \file assetml.h
  \brief The libassetml API
*/

#ifndef ASSETML_H
#define ASSETML_H

#include <glib.h>

/**
   \fn GList*	 assetml_get_asset(gchar *dataset, gchar* categories, gchar* mimetype, gchar* name)
   \brief Returns a list of AssetML struct
   \param dataset the name of the dataset, NULL for ANY dataset
   \param categories a list of categorie columns ';' separated (NULL for all)
   \param mimetype an official mimetype describing the content (NULL for all)
   \param locale is the locale to search for or NULL for the current locale
   \param file the asset file name to get or NULL for any file name
*/
GList*	 assetml_get_asset(gchar *dataset, gchar* categories, gchar* mimetype, const gchar* locale, gchar* name);


void	 assetml_free_assetlist(GList *assetlist);

typedef struct _AssetML         AssetML;

struct _AssetML
{
  gchar		*dataset;
  gchar		*categories;
  gchar		*locale;
  gchar		*description;
  gchar		*file;
  gchar		*name;
  gchar		*mimetype;
  gchar		*credits;
};


#endif
