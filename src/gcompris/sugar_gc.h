/* gcompris - sugar.h
 *
 * Copyright (C) 2010, Aleksey Lim
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

#ifndef SUGAR_GC_H
#define SUGAR_GC_H

#ifdef USE_SUGAR
  /** Initialize sugar libraries */
  void sugar_setup(int *argc, char ***argv);

  /** Was sugar mode detected on startup */
  gboolean sugar_detected(void);

  /** If sugar object restred from Journal, root menu to launch */
  const gchar *sugar_jobject_root_menu(void);

  /** Start is delayed until sugar retreaves remote data */
  gboolean sugar_delayed_start(void);

  /** Load sugar profile */
  void sugar_setup_profile(const gchar *root, gboolean administration);

  /** Setup X11 properties for main GC window */
  void sugar_setup_x11();

  /** Cleanup sugar related data */
  void sugar_cleanup(void);

  /** Get filename from current journal object */
  const char *sugar_load(void);

  /** Save file to current journal object */
  void sugar_save(const char *path);

  /** Start image chooser sugar dialog */
  void sugar_choose_image(ImageSelectorCallBack iscb, void *user_context);

  /** Get journal file by id previously returned by sugar_choose_image */
  gchar *sugar_get_journal_file(gchar *file_id);

  /** Get profile id imported to db from journal object */
  gint sugar_get_profile_id(void);

  /** Register a log report within sugar */
  void sugar_report(const gchar *date, guint duration, const gchar *user,
          GcomprisBoard *board, GCBonusStatusList status,
          const gchar *comment);
#else
# define sugar_setup(argc, argv)
# define sugar_detected() FALSE
# define sugar_jobject_root_menu() FALSE
# define sugar_delayed_start() FALSE
# define sugar_setup_profile(root, administration)
# define sugar_setup_x11()
# define sugar_cleanup()
# define sugar_load() NULL
# define sugar_save(path)
# define sugar_choose_image(iscb, user_context)
# define sugar_get_journal_file(file_id) (file_id)
# define sugar_get_profile_id() 1
# define sugar_report(date, duration, user, board, status, comment)
#endif

#endif
