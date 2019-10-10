/* event.h
 *
*/

#ifndef EVENT_H
#define EVENT_H

#include <Python.h>

class TEvent
{
  public:
    PyObject_HEAD
    
  private:
    double dTime;
    PyObject* pEventInfo;

  private:
    TEvent();
    ~TEvent();

  public:
    TEvent(PyObject* eventinfo, double time);

    class TXAlloc { };
    class TXArgument{ };

    static PyMemberDef members[];
    static PyMethodDef methods[];

    static int Init(TEvent *self, PyObject *args, PyObject *kwds);

    static PyObject* Alloc(PyTypeObject *type, Py_ssize_t nitems);
    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static void Dealloc(TEvent* self);
    static void Free(TEvent* self);

  public:

    static PyTypeObject oType;

    static void PyInit(PyObject* module);
};

#endif // EVENT_H
