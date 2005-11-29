#ifndef _PY_GCOMPRIS_WORDLIST_H_
#define _PY_GCOMPRIS_WORDLIST_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisWordlistObject(GcomprisWordlist* wordlist);

typedef struct{
  PyObject_HEAD
  GcomprisWordlist* cdata;
} pyGcomprisWordlistObject;

#endif
