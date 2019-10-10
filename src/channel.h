/* channel.h
 *
 *
*/

#ifndef CHANNEL_H
#define CHANNEL_H

#include <Python.h>

class TChannel
{
  public:
    PyObject_HEAD
    
  public:
    PyObject* pName;
    PyObject* pUnit;
    double value;
    double min;
    double max;
    double dRangeMin;
    double dRangeMax;
    double validmin;
    double validmax;
    PyObject* function;
    char bRecord;
    char bHidden;

  private:
    TChannel( );
    ~TChannel( );

    class TXAlloc { };
  public:
    void Update();
    PyObject* GetName() { return pName; }
    PyObject* GetUnit() { return pUnit; }
    double GetValue() { return value; }
    double GetMin() { return min; }
    double GetMax() { return max; }
    double RangeMin() { return dRangeMin; }
    double RangeMax() { return dRangeMax; }

    char   RecordingEnabled() { return bRecord; }

    static PyMemberDef members[];
    static PyMethodDef methods[];
    static PyGetSetDef getsets[];

    static PyObject* PyGetName(TChannel* self, void*);
    static int PySetName(TChannel* self, PyObject* arg, void*);
    static PyObject* PyGetUnit(TChannel* self, void*);
    static int PySetUnit(TChannel* self, PyObject* arg, void*);

    static PyObject* PyUpdate(TChannel* self, PyObject *args);

    static int Init(TChannel *self, PyObject *args, PyObject *kwds);

    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static PyObject* Alloc(PyTypeObject *type, Py_ssize_t nitems);
    static void Dealloc(TChannel* self);
    static void Free(TChannel* self);

    static PyTypeObject oType;

    static void PyInit(PyObject* module);
};


#endif // CHANNEL_H

