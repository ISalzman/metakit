// PyView.h --
// $Id: PyView.h 1269 2007-03-09 16:53:45Z jcw $
// This is part of MetaKit, see http://www.equi4.com/metakit/
//
//  Copyright 1999 McMillan Enterprises, Inc. -- www.mcmillan-inc.com
//  Copyright (C) 1999-2001 Jean-Claude Wippler <jcw@equi4.com>
//
//  View class header

#if !defined INCLUDE_PYVIEW_H
#define INCLUDE_PYVIEW_H

#include <mk4.h>
#include <PWOSequence.h>
#include <PWOCallable.h>
#include <PWONumber.h>
#include "PyHead.h"

#define PyView_Check(v) ((v)->ob_type==&PyViewtype)

class PyView;
class PyRowRef;

extern PyTypeObject PyViewtype;

class PyView : public PyHead, public c4_View {
  PyView *_base;
public:
  PyView();
  PyView(const c4_View& o, PyView *owner=0);
  ~PyView() {}
  void insertAt(int i, PyObject* o);
  PyRowRef *getItem(int i);
  PyView *getSlice(int s, int e);
  int setItemRow(int i, const c4_RowRef& v) {
    if (i < 0)
      i += GetSize();
    if (i > GetSize() || i < 0)
      Fail(PyExc_IndexError, "Index out of range");
    SetAt(i, v);
    return 0;
  };
  int setItem(int i, PyObject* v);
  void addProperties(const PWOSequence& lst);
  int setSlice(int s, int e, const PWOSequence& lst);
  PyObject* structure();
  void makeRow(c4_Row& temp, PyObject* o, bool useDefaults=true);
  void map(const PWOCallable& func);
  void map(const PWOCallable& func, const PyView& subset);
  PyView *filter(const PWOCallable& func);
  PyObject *reduce(const PWOCallable& func, PWONumber& start);
  void remove(const PyView& indices);
  PyView *indices(const PyView& subset);
};

PyObject* PyView_new(PyObject* o, PyObject* _args);

#endif
