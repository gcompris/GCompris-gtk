#ifndef _PY_GCOMPRIS_PROFILE_H_
#define _PY_GCOMPRIS_PROFILE_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisProfileObject(GcomprisProfile* profile);

typedef struct{
  PyObject_HEAD
  GcomprisProfile* cdata;
} pyGcomprisProfileObject;

PyObject* gcompris_new_pyGcomprisClassObject(GcomprisClass* class);

typedef struct{
  PyObject_HEAD
  GcomprisClass* cdata;
} pyGcomprisClassObject;

PyObject* gcompris_new_pyGcomprisGroupObject(GcomprisGroup* group);

typedef struct{
  PyObject_HEAD
  GcomprisGroup* cdata;
} pyGcomprisGroupObject;


PyObject* gcompris_new_pyGcomprisUserObject(GcomprisUser* user);

typedef struct{
  PyObject_HEAD
  GcomprisUser* cdata;
} pyGcomprisUserObject;

#endif
