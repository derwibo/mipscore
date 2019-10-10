/* sensor.h
 *
 *
*/

#ifndef SENSOR_H
#define SENSOR_H

#include <Python.h>

class TSensor
{
  public:
    PyObject_HEAD
    
  private:
    PyObject* pSourceFunction;

    double dPhysMin;
    double dPhysMax;
    double dLogMin;
    double dLogMax;

  public:
    TSensor(PyObject* sourcefunction, double phys_min, double phys_max, double log_min, double log_max);

  private:
    TSensor();
    ~TSensor();

    class TXAlloc { };
    class TXArgument{ };

  public:
    static PyMemberDef members[];
    static PyMethodDef methods[];

    static PyObject* Call(TSensor* self, PyObject* args, PyObject* kwds);

    static int Init(TSensor *self, PyObject *args, PyObject *kwds);
    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static PyObject* Alloc(PyTypeObject *type, Py_ssize_t nitems);
    static void Dealloc(TSensor* self);
    static void Free(TSensor* self);

    static PyTypeObject oType;

    static void PyInit(PyObject* module);
};

#endif // SENSOR_H
