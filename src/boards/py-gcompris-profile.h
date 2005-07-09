#ifndef _PY_GCOMPRIS_PROFILE_H_
#define _PY_GCOMPRIS_PROFILE_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisProfileObject(GcomprisProfile* profile);

typedef struct{
  PyObject_HEAD
  GcomprisProfile* cdata;
} pyGcomprisProfileObject;

#endif
