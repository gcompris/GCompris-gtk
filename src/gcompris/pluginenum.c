/*  GCompris -- This files comes from XMMS
 *
 *  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <dirent.h>
#include <sys/stat.h>

#include "gcompris.h"

#ifdef HPUX
#include <dl.h>
#else
#include <dlfcn.h>
#endif

#ifdef HPUX
# define SHARED_LIB_EXT ".sl"
#else
# define SHARED_LIB_EXT ".so"
#endif

#ifndef RTLD_NOW
# define RTLD_NOW 0
#endif

gchar *plugin_dir_list[] =
{
	PLUGINSUBS,
	NULL
};

extern struct BoardPluginData *bp_data;

void scan_plugins(char *dirname);
void add_plugin(gchar * filename);


/*
static int d_iplist_compare(const void *a, const void *b)
{
	return strcmp(((char *) a), ((char *) b));
}
*/

static int boardlist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((BoardPlugin *) a)->description, ((BoardPlugin *) b)->description);
}


void init_plugins(void)
{
	gchar *dir, *temp;
	GList *node;
	BoardPlugin *ip;
	gint dirsel = 0;

	bp_data = g_malloc0(sizeof (struct BoardPluginData));


#ifndef DISABLE_USER_PLUGIN_DIR
	/*
	 * This is in a separate loop so if the user puts them in the
	 * wrong dir we'll still get them in the right order (home dir
	 * first                                                - Zinx
	 */
	while (plugin_dir_list[dirsel])
	{
		dir = g_strconcat(g_get_home_dir(), "/.gcompris/Plugins/", plugin_dir_list[dirsel++], NULL);
		scan_plugins(dir);
		g_free(dir);
	}
	dirsel = 0;
#endif

	while (plugin_dir_list[dirsel])
	{
		dir = g_strconcat(PLUGIN_DIR, "/", plugin_dir_list[dirsel++], NULL);
		scan_plugins(dir);
		g_free(dir);
	}

	bp_data->board_list = g_list_sort(bp_data->board_list, boardlist_compare_func);

	node = bp_data->board_list;
	while (node)
	{
		ip = (BoardPlugin *) node->data;
		temp = g_basename(ip->filename);
		if (ip->init)
			ip->init();
		node = node->next;
	}

}

void add_plugin(gchar * filename)
{
#ifdef HPUX
	shl_t *h;
#else
	void *h;
#endif
	void *(*gpi) (void);

#ifndef DISABLE_USER_PLUGIN_DIR
	/*
	 * erg.. gotta check 'em all, surely there's a better way
	 *                                                 - Zinx
	 */
	{
		GList *l;
		gchar *base_filename = g_basename(filename);

		for (l = bp_data->board_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((BoardPlugin*)l->data)->filename)))
				return;
		}
	}
#endif
#ifdef HPUX
	if ((h = shl_load(filename, BIND_DEFERRED, 0)) != NULL)
		/* use shl_load family of functions on HP-UX 
		   HP-UX does not support dlopen on 32-bit
		   PA-RISC executables */
#else
	if ((h = dlopen(filename, RTLD_NOW)) != NULL)
#endif /* HPUX */
	{
#ifdef HPUX
 		if ((shl_findsym(&h, "get_bplugin_info", TYPE_PROCEDURE, (void*) &gpi)) == 0)
#else
		if ((gpi = dlsym(h, "get_bplugin_info")) != NULL)
#endif
		{
			BoardPlugin *p;
			g_message("    plugin %s loaded", filename);
			p = (BoardPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);

			bp_data->board_list = g_list_prepend(bp_data->board_list, p);
		}
		else
		{
#ifdef HPUX
                        shl_unload(h);
#else
			dlclose(h);
#endif
		}
	}
	else
#ifdef HPUX
		perror("Error loading plugin!"); 
#else
		fprintf(stderr, "%s\n", dlerror());
#endif
}

void scan_plugins(char *dirname)
{
	gchar *filename, *ext;
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;
	g_message("scan_plugins %s", dirname);
	dir = opendir(dirname);
	if (!dir)
		return;

	while ((ent = readdir(dir)) != NULL)
	{
		filename = g_strdup_printf("%s%s", dirname, ent->d_name);
		if (!stat(filename, &statbuf) && S_ISREG(statbuf.st_mode) &&
		    (ext = strrchr(ent->d_name, '.')) != NULL)
			if (!strcmp(ext, SHARED_LIB_EXT))
				add_plugin(filename);
		g_free(filename);
	}
	closedir(dir);
}

void cleanup_plugins(void)
{
#ifdef HPUX
        shl_t *h;
#endif
	BoardPlugin *ip;
	GList *node;

	if (get_board_playing())
		board_stop();

	node = get_board_list();
	while (node)
	{
		ip = (BoardPlugin *) node->data;
		if (ip && ip->cleanup)
		{
			ip->cleanup();
			GDK_THREADS_LEAVE();
			while(g_main_iteration(FALSE));
			GDK_THREADS_ENTER();

		}
#ifdef HPUX
                h = ip->handle;
                shl_unload(*h);
#else
		printf("cleanup_plugins %s\n", ip->name);
		dlclose(ip->handle);
#endif
		node = node->next;
	}
	if (bp_data->board_list)
		g_list_free(bp_data->board_list);
	g_free(bp_data);
}
