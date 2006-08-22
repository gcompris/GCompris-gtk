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
 
#ifndef WORDLIST_H
#define WORDLIST_H

typedef struct {
  gint level;
  GSList *words;
} LevelWordlist;

typedef struct {
  gchar         *filename;
  gchar         *name;
  gchar         *description;
  gchar         *locale;
  guint		 number_of_level;
  /* LevelWordlist list */
  GSList         *levels_words;
} GcomprisWordlist;

GcomprisWordlist *gc_wordlist_get_from_file(const gchar *fileformat, ...);
void              gc_wordlist_free(GcomprisWordlist *wordlist);
gchar		 *gc_wordlist_random_word_get(GcomprisWordlist *wordlist, guint level);

#endif
