/* gcompris - python.c
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

#include <Python.h>
//#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include <pygtk/pygtk.h>
#include "gcompris/gcompris.h"
#include "py-gcompris-board.h"
#include "py-mod-gcompris.h"
#include "py-gcompris-profile.h"

static GcomprisBoard *gcomprisBoard = NULL;

static PyObject* python_gcomprisBoard = NULL;
static PyObject* python_board_module = NULL;
static PyObject* python_board_instance = NULL;

static GcomprisBoard *gcomprisBoard_config = NULL;
static PyObject* python_gcomprisBoard_config = NULL;;
static PyObject* python_board_config_module = NULL;
static PyObject* python_board_config_instance = NULL;

static void	 pythonboard_init (GcomprisBoard *agcomprisBoard);
static void	 pythonboard_start (GcomprisBoard *agcomprisBoard);
static void	 pythonboard_pause (gboolean pause);
static void	 pythonboard_end (void);

static gboolean	 pythonboard_is_our_board (GcomprisBoard *agcomprisBoard);

static gint	 pythonboard_key_press (guint keyval, gchar *commit_str, gchar *preedit_str);
static void	 pythonboard_ok (void);
static void	 pythonboard_set_level (guint level);
static void	 pythonboard_config(void);
static void	 pythonboard_repeat (void);
static void	 pythongc_board_config_start (GcomprisBoard *agcomprisBoard,
					   GcomprisProfile *aProfile);
static void	 pythongc_board_config_stop (void);

static gboolean  pythonboard_is_ready = FALSE;


/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Python Board"),
    N_("Special board that embeds python into GCompris."),
    "Olivier Samyn <osamyn@ulb.ac.be>",
    pythonboard_init,
    NULL,
    NULL,
    NULL,
    pythonboard_start,
    pythonboard_pause,
    pythonboard_end,
    pythonboard_is_our_board,
    pythonboard_key_press,
    pythonboard_ok,
    pythonboard_set_level,
    pythonboard_config,
    pythonboard_repeat,
    pythongc_board_config_start,
    pythongc_board_config_stop
  };

static BoardPlugin menu_bp_no_config =
  {
    NULL,
    NULL,
    N_("Python Board"),
    N_("Special board that embeds python into gcompris."),
    "Olivier Samyn <osamyn@ulb.ac.be>",
    pythonboard_init,
    NULL,
    NULL,
    NULL,
    pythonboard_start,
    pythonboard_pause,
    pythonboard_end,
    pythonboard_is_our_board,
    pythonboard_key_press,
    pythonboard_ok,
    pythonboard_set_level,
    pythonboard_config,
    pythonboard_repeat,
    NULL,
    NULL
  };

/*
 * Return the plugin structure (Common to all gcompris boards)
 */

GET_BPLUGIN_INFO(python)


/*
 * Tests if a python interpreter is available
 * and that all required imports can be loaded.
 */

static GList *config_boards= NULL;

GList *
get_pythonboards_list()
{
  GList *pythonboards_list = NULL;
  GList *boards_list = gc_menu_get_boards();
  GList *list;
  GcomprisBoard *board;

  for (list = boards_list; list != NULL; list = list->next){
    board = (GcomprisBoard *) list->data;
    if (g_ascii_strncasecmp(board->type, "python", 6)==0)
      pythonboards_list = g_list_append(pythonboards_list, board);
  }

  return pythonboards_list;
}

/* This loops over all the boards and for the python one
 * checks if they have a config method defined. If so
 * they are put in a list.
 */
static void
init_config_boards()
{
  static gboolean called_once = FALSE;
  GList *python_boards;
  GList *list;
  PyObject* globals;
  static char *python_args[]={ "" };
  static char* python_prog_name="gcompris";
  char* boardclass;
  char* board_file_name;
  PyObject* main_module;
  char* boarddir;
  gchar *userplugindir;
  GcomprisProperties *properties = gc_prop_get();
  PyObject* module_dict;
  PyObject* py_boardclass;

  if (called_once)
    return;

  called_once = TRUE;

  if(!Py_IsInitialized()){
    /* Initialize the python interpreter */
    Py_SetProgramName(python_prog_name);
    Py_Initialize();

    PySys_SetArgv(1, python_args);

    init_pygobject();

    main_module = PyImport_AddModule("__main__");
    globals = PyModule_GetDict(main_module);

    if(globals==NULL){
      g_print("Cannot get info from the python interpreter. Seems there is a problem with this one.\n");
      return;
    }

    /* Add the python plugins dir to the python's search path */
    boarddir = g_strdup_printf("import sys; sys.path.append('%s')",
			       properties->package_python_plugin_dir );

    PyRun_SimpleString(boarddir);
    g_free(boarddir);

#ifndef DISABLE_USER_PLUGIN_DIR
    g_free(userplugindir);
#endif

    /* Load the gcompris modules */
    python_gcompris_module_init();
  }
  else {
    main_module = PyImport_AddModule("__main__"); /* Borrowed reference */
    globals = PyModule_GetDict(main_module); /* Borrowed reference */
  }


  /* Get the list of python boards */
  python_boards = get_pythonboards_list();

  /* Search in the list each one with a config entry */
  for(list = python_boards; list != NULL; list = list->next) {
    GcomprisBoard *board = (GcomprisBoard *) list->data;

    /* Python is now initialized we create some usefull variables */
    board_file_name = strchr(board->type, ':')+1;
    boardclass = g_strdup_printf("Gcompris_%s", board_file_name);

    /* Test if board come with --python_plugin_dir option */

    g_message("board_dir: '%s' python_plugin_dir '%s' file_name '%s'",
	      board->board_dir,
	      properties->package_python_plugin_dir,
	      board_file_name);

    /* Insert the board module into the python's interpreter */
    python_board_module = PyImport_ImportModuleEx(board_file_name,
						  globals,
						  globals,
						  NULL);

    if(python_board_module!=NULL){
      /* Get the module dictionnary */
      module_dict = PyModule_GetDict(python_board_module);

      /* Get the python board class */
      py_boardclass = PyDict_GetItemString(module_dict, boardclass);

      if (PyObject_HasAttrString( py_boardclass, "config_start")) {
	config_boards = g_list_append(config_boards, board);
	g_message("The board '%s' has a configuration entry",
		  board_file_name);
      }
    }
    g_free(boardclass);
  }
  g_list_free(python_boards);

  /* Finalize the python interpreter */
  Py_Finalize();
}

static void
pythonboard_init (GcomprisBoard *agcomprisBoard){
  PyObject* main_module;
  PyObject* globals;
  gchar* execstr;
  gchar* userplugindir;

  GcomprisProperties *properties = gc_prop_get();

  if (pythonboard_is_ready)
    return ;

  /* Initialize the python interpreter */
  Py_Initialize();

  static char *python_args[]={ "" };
  PySys_SetArgv( 1, python_args);

  pythonboard_is_ready = TRUE;

  main_module = PyImport_AddModule("__main__"); /* Borrowed reference */
  globals = PyModule_GetDict(main_module); /* Borrowed reference */

  if(globals==NULL){
    g_warning("! Python disabled: Cannot get info from the python interpreter.\n");
    pythonboard_is_ready = FALSE;
  } else {
    /* Add the python plugins dir to the python's search path */
#ifndef DISABLE_USER_PLUGIN_DIR
    userplugindir = g_strconcat(g_get_home_dir(), properties->config_dir,
				"/Plugins/",
				NULL);
    execstr = g_strdup_printf("import sys; sys.path.append('%s/python'); sys.path.append('%s')",
			      userplugindir, properties->package_python_plugin_dir);
    g_free(userplugindir);
#else
    execstr = g_strdup_printf("import sys; sys.path.append('%s')",properties->package_python_plugin_dir );
#endif

    g_message("Executing %s\n", execstr);
    if(PyRun_SimpleString(execstr)!=0){
      pythonboard_is_ready = FALSE;
      g_warning("! Python disabled: Cannot add plugins dir into search path\n");
    } else {
      /* Try to import pygtk modules */
      g_free(execstr);
      execstr = g_strdup("import gtk; import gtk.gdk");
      if(PyRun_SimpleString(execstr)!=0){
	pythonboard_is_ready = FALSE;
	g_warning("! Python disabled: Cannot import pygtk modules\n");
      } else {
	/* Try to import gcompris modules */

	/* Load the gcompris modules */
	python_gcompris_module_init();

	g_free(execstr);
	execstr = g_strdup("import gcompris; import gcompris.bonus; "
			   "import gcompris.score; import gcompris.sound;"
			   "import gcompris.skin; import gcompris.timer;"
			   "import gcompris.utils; import gcompris.anim");
	if(PyRun_SimpleString(execstr)!=0){
	  pythonboard_is_ready = FALSE;
	  g_warning("! Python disabled: Cannot import gcompris modules\n");
	}
    }
    g_free(execstr);

  }

  /* Finalize the python interpreter */
  Py_Finalize();

  }
}

/*
 * Start the board.
 * In this case:
 * - initialize python interpreter
 * - import gcompris functions/objects
 * - import gtk/gnome functions/objects
 * - load the python written board
 * - call the board start function
 */
static void
pythonboard_start (GcomprisBoard *agcomprisBoard){
  PyObject* main_module;
  PyObject* py_function_result;
  PyObject* module_dict;
  PyObject* py_boardclass;
  PyObject* py_boardclass_args;
  PyObject* globals;
  static char *python_args[]={ "" };
  static char* python_prog_name="gcompris";
  char* boarddir;
  char* boardclass;
  char* board_file_name;
  gchar *userplugindir;

  GcomprisProperties	*properties = gc_prop_get();

  if(agcomprisBoard!=NULL){
    /* Initialize the python interpreter */
    Py_SetProgramName(python_prog_name);
    Py_Initialize();

    PySys_SetArgv(1, python_args);

    init_pygobject();

    main_module = PyImport_AddModule("__main__");
    globals = PyModule_GetDict(main_module);

    if(globals==NULL){
      g_print("Cannot get info from the python interpreter. Seems there is a problem with this one.\n");
      return;
    } else {
      gcomprisBoard = agcomprisBoard;
    }

    /* Add the python plugins dir to the python's search path */
#ifndef DISABLE_USER_PLUGIN_DIR
    userplugindir = g_strconcat(g_get_home_dir(), "/.gcompris/Plugins/", NULL);
    boarddir = g_strdup_printf("import sys; sys.path.append('%s/python'); sys.path.append('%s')",
			       userplugindir,
			       properties->package_python_plugin_dir);
#else
    boarddir = g_strdup_printf("import sys; sys.path.append('%s')",properties->package_python_plugin_dir );
#endif

    PyRun_SimpleString(boarddir);
    g_free(boarddir);


    /* Test if board come with -L option */
    if (strcmp(gcomprisBoard->board_dir, properties->package_data_dir)!=0){
      boarddir = g_strdup_printf("sys.path.append('%s/../python/')", gcomprisBoard->board_dir);

      PyRun_SimpleString(boarddir);
      g_free(boarddir);
    }


#ifndef DISABLE_USER_PLUGIN_DIR
    g_free(userplugindir);
#endif

    /* Load the gcompris modules */
    python_gcompris_module_init();

    /* Python is now initialized we create some usefull variables */
    board_file_name = strchr(agcomprisBoard->type, ':')+1;
    boardclass = g_strdup_printf("Gcompris_%s", board_file_name);

    /* Insert the board module into the python's interpreter */
    python_board_module = PyImport_ImportModuleEx(board_file_name,
 						  globals,
 						  globals,
 						  NULL);

    if(python_board_module!=NULL){
      /* Get the module dictionnary */
      module_dict = PyModule_GetDict(python_board_module);

      /* Get the python board class */
      py_boardclass = PyDict_GetItemString(module_dict, boardclass);

      /* Create a python gcompris board */
      python_gcomprisBoard=gcompris_new_pyGcomprisBoardObject(agcomprisBoard);

      /* Create an instance of the board class */
      py_boardclass_args = PyTuple_New(1);
      Py_INCREF(python_gcomprisBoard);
      PyTuple_SetItem(py_boardclass_args, 0, python_gcomprisBoard);
      python_board_instance = PyInstance_New(py_boardclass, py_boardclass_args, NULL);
      Py_DECREF(py_boardclass_args);

      /* Call the function */
      py_function_result = PyObject_CallMethod(python_board_instance, "start", NULL);
      if( py_function_result != NULL){
	Py_DECREF(py_function_result);
      } else {
	PyErr_Print();
      }
    } else {
      PyErr_Print();
    }

    g_free(boardclass);
  }
}

/*
 * Pause the board.
 */
static void pythonboard_pause (gboolean pause){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "pause", "i", pause);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print();
  }
}

/*
 * End the board.
 * In this case:
 * - call the board end function
 * - finalise python interpreter
 */
static void pythonboard_end (void){
  PyObject* result = NULL;

  if(python_gcomprisBoard!=NULL){
    result = PyObject_CallMethod(python_board_instance, "end", NULL);
    if( result == NULL){
      PyErr_Print();
    } else {
      Py_DECREF(result);
    }
    Py_XDECREF(python_board_module);
    Py_XDECREF(python_board_instance);
    Py_XDECREF(python_gcomprisBoard);
    //FIXME: COMMENTED BECAUSE THIS MAKES GCOMPRIS CRASHES AFTER ENTERING/EXITING
    //       ANY PHYTHON PLUGIN LESS THAN 10 TIMES
    //Py_Finalize();
  }
}

/*
 * Return TRUE if the board is a python one.
 */
static gboolean pythonboard_is_our_board (GcomprisBoard *gcomprisBoard){

  if (gcomprisBoard->plugin)
    return TRUE;

  if(pythonboard_is_ready) {
    if (gcomprisBoard!=NULL) {

      if (g_ascii_strncasecmp(gcomprisBoard->type, "python", 6)==0) {

	init_config_boards();
	/* Set the plugin entry */
	if (g_list_find (config_boards, gcomprisBoard)){
	  gcomprisBoard->plugin = &menu_bp;
	} else {
	  gcomprisBoard->plugin = &menu_bp_no_config;
	}

	return TRUE;
      }
    }
  }
  gcomprisBoard->plugin=NULL;
  return FALSE;
}

/*
 * Key press
 */
static gint pythonboard_key_press (guint keyval, gchar *commit_str, gchar *preedit_str){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "key_press", "iss", keyval, commit_str, preedit_str);

  if (result==NULL) return FALSE;

  if (PyInt_Check(result) && (PyInt_AsLong(result)>0)){
    Py_DECREF(result);
    return TRUE;
  } else {
    Py_DECREF(result);
    return FALSE;
  }
}

/*
 * OK button pressed
 */
static void pythonboard_ok (void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "ok", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print();
  }
}

/*
 * Set Level
 */
static void pythonboard_set_level (guint level){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "set_level", "i", level);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print();
  }
}

/*
 * Config
 */
static void pythonboard_config(void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "config", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print();
  }
}

/*
 * Repeat
 */
static void pythonboard_repeat (void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "repeat", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print();
  }
}

/*
 * Start the board config_start.
 * In this case:
 * - initialize python interpreter
 * - import gcompris functions/objects
 * - import gtk/gnome functions/objects
 * - load the python written board
 * - call the board start function
 */

/*
 * Normally python in already running when config_start is called.
 * If not config_stop has to stop it.
 */

static gboolean python_run_by_config = FALSE;

static void
pythongc_board_config_start (GcomprisBoard *agcomprisBoard,
			  GcomprisProfile *aProfile
			  )
{
  GcomprisProperties *properties = gc_prop_get();
  PyObject* py_function_result;
  PyObject* module_dict;
  PyObject* py_boardclass;
  PyObject* py_boardclass_args;
  PyObject* globals;
  static char *python_args[]={ "" };
  static char* python_prog_name="gcompris";
  char* boardclass;
  char* board_file_name;
  PyObject* main_module;
  char* boarddir;
  gchar *userplugindir;

  g_assert (agcomprisBoard != NULL);

  if(!Py_IsInitialized()){
    /* Initialize the python interpreter */
    Py_SetProgramName(python_prog_name);
    Py_Initialize();

    PySys_SetArgv(1, python_args);

    init_pygobject();

    main_module = PyImport_AddModule("__main__");
    globals = PyModule_GetDict(main_module);

    if(globals==NULL){
      g_print("Cannot get info from the python interpreter. Seems there is a problem with this one.\n");
      return;
    } else {
      gcomprisBoard_config = agcomprisBoard;
    }

    /* Add the python plugins dir to the python's search path */
#ifndef DISABLE_USER_PLUGIN_DIR
    userplugindir = g_strconcat(g_get_home_dir(), "/.gcompris/Plugins/", NULL);
    boarddir = g_strdup_printf("import sys; sys.path.append('%s/python'); sys.path.append('%s'); sys.path.append('%s')",
			       userplugindir,
			       properties->package_python_plugin_dir,
			       gcomprisBoard_config->board_dir);
#else
    boarddir = g_strdup_printf("import sys; sys.path.append('%s')", properties->package_python_plugin_dir );
#endif

    PyRun_SimpleString(boarddir);
    g_free(boarddir);

#ifndef DISABLE_USER_PLUGIN_DIR
    g_free(userplugindir);
#endif

    /* Load the gcompris modules */
    python_gcompris_module_init();

    python_run_by_config = TRUE;

  }
  else {
    main_module = PyImport_AddModule("__main__"); /* Borrowed reference */
    globals = PyModule_GetDict(main_module); /* Borrowed reference */
  }

  /* Python is now initialized we create some usefull variables */
  board_file_name = strchr(agcomprisBoard->type, ':')+1;
  boardclass = g_strdup_printf("Gcompris_%s", board_file_name);

  /* Insert the board module into the python's interpreter */
  python_board_config_module = PyImport_ImportModuleEx(board_file_name,
						       globals,
						       globals,
						       NULL);

  if(python_board_config_module!=NULL){
    /* Get the module dictionnary */
    module_dict = PyModule_GetDict(python_board_config_module);

    /* Get the python board class */
    py_boardclass = PyDict_GetItemString(module_dict, boardclass);

    /* Create a python gcompris board */
    python_gcomprisBoard_config=gcompris_new_pyGcomprisBoardObject(agcomprisBoard);
    /* Create an instance of the board class */
    py_boardclass_args = PyTuple_New(1);
    Py_INCREF(python_gcomprisBoard_config);
    PyTuple_SetItem(py_boardclass_args, 0, python_gcomprisBoard_config);
    python_board_config_instance = PyInstance_New(py_boardclass, py_boardclass_args, NULL);
    Py_DECREF(py_boardclass_args);

    py_function_result = PyObject_CallMethod(python_board_config_instance,
					     "config_start",
					     "O",
					     gcompris_new_pyGcomprisProfileObject(aProfile));

    if( py_function_result != NULL){
      Py_DECREF(py_function_result);
    } else {
      PyErr_Print();
    }
  } else {
    PyErr_Print();
  }

  g_free(boardclass);
}

/*
 * End the board.
 * In this case:
 * - call the board end function
 * - finalise python interpreter
 */
static void pythongc_board_config_stop (void){
  PyObject* result = NULL;

  if(python_gcomprisBoard_config!=NULL){
    result = PyObject_CallMethod(python_board_config_instance, "config_stop", NULL);
    if( result == NULL){
      PyErr_Print();
    } else {
      Py_DECREF(result);
    }
    Py_XDECREF(python_board_config_module);
    Py_XDECREF(python_board_config_instance);
    Py_XDECREF(python_gcomprisBoard_config);
    if (python_run_by_config){
      Py_Finalize();
      python_run_by_config = FALSE;
    }
  }
}
