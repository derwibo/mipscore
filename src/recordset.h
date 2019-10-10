/* recordset.h
 *
*/

#ifndef RECORDSET_H
#define RECORDSET_H

#include <Python.h>
#include "record.h"
#include "event.h"

class TRecordSet
{
  public:
    PyObject_HEAD

  public:
    int id;
    PyObject* pName;
    PyObject* pSrcname;

    PyObject* pAttributeList;
    PyObject* pRecordList;
    PyObject* pEventList;

  public:
    TRecordSet();
    ~TRecordSet();

    class TXAlloc { };
    class TXArgument{ };

    static bool CheckItem(PyObject* v);

    PyObject* GetAttributeList() { return pAttributeList; }
    PyObject* GetRecordList() { return pRecordList; }
    PyObject* GetEventList() { return pEventList; }

    int SetName(char* pName);
    PyObject* GetName() { return pName; }
    int SetName(PyObject* pName);

    int SetSrcName(char* pSrcname);
    PyObject* GetSrcName() { return pSrcname; }
    int SetSrcName(PyObject* pSrcname);

    bool AddChannel(TRecord* record, int index = 0);
    bool AddChannel(TRecord* record, TRecord* pInsertAfter);
    TRecord* GetChannel(int index);
    int GetChannelIndex(int chnid);
    int GetChannelIndex(TRecord* record);
    bool RemoveChannel(int index);


    static void PyInit(PyObject* module);
    static PyMemberDef members[];
    static PyMethodDef methods[];
    static PyGetSetDef getsets[];
    static PySequenceMethods sequencemethods[];
    static PyMappingMethods mappingmethods[];

    static Py_ssize_t PySequence_Size(TRecordSet* o);
    static PyObject* PySequence_GetItem(TRecordSet* o, Py_ssize_t i);
    static int PySequence_SetItem(TRecordSet* o, Py_ssize_t i, PyObject* v);

    static PyObject* PyMapping_GetItem(TRecordSet* self, PyObject *key);
    static int PyMapping_SetItem(TRecordSet* self, PyObject *key, PyObject *v);

    static PyObject* PyAccept(TRecordSet* self, PyObject* args);

    static PyObject* PyAdd(TRecordSet* self, PyObject* args);

    static PyObject* PyGetName(TRecordSet* self, void*);
    static int PySetName(TRecordSet* self, PyObject* arg, void*);

    static PyObject* PyGetSrcname(TRecordSet* self, void*);
    static int PySetSrcname(TRecordSet* self, PyObject* arg, void*);

    static int Init(TRecordSet *self, PyObject *args, PyObject *kwds);
    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static PyObject* Alloc(PyTypeObject *type, Py_ssize_t nitems);
    static void Dealloc(TRecordSet* self);
    static void Free(TRecordSet* self);

    static PyTypeObject oType;
};

#endif // RECORDSET_H
