// PyHead.h --
// $Id: PyHead.h 1246 2007-03-09 16:29:26Z jcw $
// This is part of MetaKit, see http://www.equi4.com/metakit/
//
//  Copyright 1999 McMillan Enterprises, Inc. -- www.mcmillan-inc.com
//  Copyright (C) 1999-2001 Jean-Claude Wippler <jcw@equi4.com>
//
//  Common object header class

#if !defined INCLUDE_PYHEAD_H
#define INCLUDE_PYHEAD_H

#include <Python.h>

class PyHead : public PyObject {
public:
    PyHead(PyTypeObject& t)
    {
#ifdef Py_TRACE_REFS
        _ob_next = 0;
        _ob_prev = 0;
#endif
        ob_refcnt = 1;
        ob_type = &t;
    }
};

#endif

