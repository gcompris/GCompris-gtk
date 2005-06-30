#ifndef _PY_GCOMPRIS_PROPERTIES_H_
#define _PY_GCOMPRIS_PROPERTIES_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisPropertiesObject(GcomprisProperties* properties);

typedef struct{
  PyObject_HEAD
  GcomprisProperties* cdata;
} pyGcomprisPropertiesObject;

#endif
