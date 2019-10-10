/* record.h
 *
*/

#ifndef RECORD_H
#define RECORD_H

#include <Python.h>
#include "list.h"

class TRecord
{
  public:
    PyObject_HEAD
    
  public:
    PyObject* pName;
    PyObject* pUnit;
    PyObject* pComment;
    PyObject* pSource;

    double*   pData;
    double    dMaxVal;
    double    dMinVal;
    double    dLoRange;
    double    dHiRange;
    unsigned long iLength;
    unsigned long iBufferLength;

  private:
    TRecord();
    ~TRecord();

    class TXAlloc { };
    class TXArgument{ };

  public:
    PyObject* GetName() { return pName; }
    PyObject* GetUnit() { return pUnit; }

    void _Set(unsigned long pos, double value) { pData[pos] = value; }
    double _Get(unsigned long pos) { return pData[pos]; }

    double& operator[](int pos) { return pData[pos]; }

    bool Set(unsigned long pos, double value);
    bool Get(unsigned long pos, double* value);

    double GetValue(long pos);
    double SetValue(long pos, double value);
    double _GetValue(long pos) { return pData[pos]; }
    double _SetValue(long pos, double value) { return pData[pos] = value; }
    double MaxValue() { return dMaxVal; }
    double MinValue() { return dMinVal; }
    double LoRange() { return dLoRange; }
    double HiRange() { return dHiRange; }
    long GetLength() { return iLength; }
    long SetLength(long length);
    long GetMaxLength() { return iBufferLength; }
    int UpdateStatVals();
    int SetRange(double lo, double hi);

    int GenerateSequence1(double start, double end);
    int GenerateSequence2(double start, double step);

    static void PyInit(PyObject* module);
    static PyMemberDef members[];
    static PyMethodDef methods[];
    static PyGetSetDef getsets[];
    static PySequenceMethods sequencemethods[];

    static Py_ssize_t PySequence_Size(TRecord* o);
    static PyObject* PySequence_GetItem(TRecord* o, Py_ssize_t i);
    static int PySequence_SetItem(TRecord* o, Py_ssize_t i, PyObject* v);
    static PyObject* PySequence_GetSlice(TRecord *o, Py_ssize_t i1, Py_ssize_t i2);

    static PyObject* PyGet(TRecord* self, PyObject* args);
    static PyObject* PySet(TRecord* self, PyObject* args);

    static PyObject* PyGetName(TRecord* self, void*);
    static int PySetName(TRecord* self, PyObject* arg, void*);

    static PyObject* PyGetUnit(TRecord* self, void*);
    static int PySetUnit(TRecord* self, PyObject* arg, void*);

    static PyObject* PyGetComment(TRecord* self, void*);
    static int PySetComment(TRecord* self, PyObject* arg, void*);

    static PyObject* PyGetRange(TRecord* self, void*);
    static int PySetRange(TRecord* self, PyObject* arg, void*);

    static int Init(TRecord *self, PyObject *args, PyObject *kwds);
    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static PyObject* Alloc(PyTypeObject *self, Py_ssize_t nitems);
    static void Dealloc(TRecord* self);
    static void Free(TRecord* self);

  public:
    static PyTypeObject oType;
};

#endif // RECORD_H
