/* recordset.cxx
 *
*/

#include "recordset.h"

#include <structmember.h>


TRecordSet::TRecordSet()
{
  PyObject_Init((PyObject*) this, &TRecordSet::oType);
  
  pName = PyUnicode_FromString("");
  if(pName == NULL)
  {
    throw TXAlloc();
  }

  pSrcname = PyUnicode_FromString("");
  if(pSrcname == NULL)
  {
    throw TXAlloc();
  }

  pAttributeList = PyDict_New();
  if(pAttributeList == NULL)
  {
    throw TXAlloc();
  }

  pRecordList = PyList_New(0);
  if(pRecordList == NULL)
  {
    Py_DECREF(pAttributeList);
    throw TXAlloc();
  }
  pEventList = PyList_New(0);
  if(pEventList == NULL)
  {
    Py_DECREF(pEventList);
    throw TXAlloc();
  }
} 


TRecordSet::~TRecordSet()
{
  Py_DECREF(pAttributeList);
  Py_DECREF(pRecordList);
  Py_DECREF(pEventList);
  Py_DECREF(pSrcname);
  Py_DECREF(pName);
}

bool TRecordSet::CheckItem(PyObject* v)
{     
  return (PyObject_TypeCheck(v, &TRecord::oType));
}


int TRecordSet::SetName(PyObject* pName)
{
  PyObject* tmp;
  if(PyUnicode_Check(pName))
  {
    Py_INCREF(pName);
    tmp = this->pName;
    this->pName = pName;
    Py_XDECREF(tmp);
  }
  else
    return 1;

  return 0;
}

int TRecordSet::SetSrcName(PyObject* pSrcname)
{
  PyObject* tmp;

  if(PyUnicode_Check(pSrcname))
  {
    Py_INCREF(pSrcname);
    tmp = this->pSrcname;
    this->pSrcname = pSrcname;
    Py_XDECREF(tmp);
  }
  else
    return 1;

  return 0;
}

int TRecordSet::SetSrcName(char* srcname)
{
  PyObject* pSrcname;
  PyObject* tmp;

  pSrcname = PyUnicode_FromString(srcname);
  if(pSrcname == NULL)
  {
    throw TXAlloc();
  }
  Py_INCREF(pSrcname);
  tmp = this->pSrcname;
  this->pSrcname = pSrcname;
  Py_XDECREF(tmp);
  return 0;
}

bool TRecordSet::AddChannel(TRecord* channel, int index)
{
  PyObject* pChannel;
  pChannel = (PyObject*)channel;

  if(index < 0) PyList_Append(pRecordList, pChannel);
  else PyList_Insert(pRecordList, index, pChannel);

//  channel->_SetGrp(this->id);
  return true;
}

bool TRecordSet::AddChannel(TRecord* channel, TRecord* pInsertAfter)
{
  int iIndex;
  PyObject* pChannel;
  pChannel = (PyObject*)channel;

  for(iIndex = 0 ; iIndex < PyList_Size(pRecordList) ; iIndex++)
  {
    if(PyList_GetItem(pRecordList, iIndex) == (PyObject*)pInsertAfter)
    {
      PyList_Insert(pRecordList, iIndex+1, pChannel);
//      channel->_SetGrp(this->id);
      return true;
    }
  }
  return false;
}


TRecord* TRecordSet::GetChannel(int index)
{
  PyObject* pObject;

  pObject = PyList_GetItem(pRecordList, index);
 
  return (TRecord*) pObject;
}

/*
int TRecordSet::GetChannelIndex(int chnid)
{
  int i;

  PyObject* pObject;
  TRecord* pRecord;

  i = 0;

  while(pObject = PyList_GetItem(pRecordList, i))
  {
    pRecord = (TRecord*)pObject;
    if(pRecord->GetId() == chnid)
    {
      return i;
    }
    i++;
  }
  return -1;
}
*/

int TRecordSet::GetChannelIndex(TRecord* channel)
{
  int iIndex;

  for(iIndex = 0 ; iIndex < PyList_Size(pRecordList) ; iIndex++)
  {
    if(PyList_GetItem(pRecordList, iIndex) == (PyObject*)channel)
    {
      return iIndex;
    }
  }

  return -1;
}

bool TRecordSet::RemoveChannel(int index)
{
  if(PySequence_DelItem(pRecordList, index) != -1)
  {
    return true;
  }
  return false;
}

PyMemberDef TRecordSet::members[] = {
  {"attributes", T_OBJECT_EX, offsetof(TRecordSet, pAttributeList), READONLY, "Attributes"},
  {"records", T_OBJECT_EX, offsetof(TRecordSet, pRecordList), READONLY, "Records"},
  {"events", T_OBJECT_EX, offsetof(TRecordSet, pEventList), READONLY, "Events"},
  {NULL}  /* Sentinel */
};

PyMethodDef TRecordSet::methods[] = {
    {"add", (PyCFunction)TRecordSet::PyAdd, METH_VARARGS, "Add Item"},
    {"accept", (PyCFunction)TRecordSet::PyAccept, METH_VARARGS, "Accept"},
    {NULL}  /* Sentinel */
};

PyGetSetDef TRecordSet::getsets[] = {
  {"name", (getter)TRecordSet::PyGetName, (setter)TRecordSet::PySetName, "Name", NULL},
  {"srcname", (getter)TRecordSet::PyGetSrcname, (setter)TRecordSet::PySetSrcname, "Srcname", NULL},
  {NULL}  /* Sentinel */
};

PySequenceMethods TRecordSet::sequencemethods[] = {
  {
    (lenfunc)PySequence_Size,               // lenfunc sq_length 
    0,                                      // binaryfunc sq_concat 
    0,                                      // ssizeargfunc sq_repeat 
    (ssizeargfunc)PySequence_GetItem,       // ssizeargfunc sq_item
    0,                                      // ssizessizeargfunc sq_slice
    (ssizeobjargproc)PySequence_SetItem,    // ssizeobjargproc sq_ass_item
    0,                  // ssizessizeobjargproc sq_ass_slice
    0,                  // objobjproc sq_contains 
    0,                  // binaryfunc sq_inplace_concat 
    0,                  // ssizeargfuncs sq_inplace_repeat
  },
};

PyMappingMethods TRecordSet::mappingmethods[] = {
  {
    (lenfunc)PySequence_Size,               // lenfunc mp_length 
    (binaryfunc)PyMapping_GetItem,          // binaryfunc mp_subscript 
    (objobjargproc)PyMapping_SetItem,       // objobjargproc mp_ass_subscript 
  },
};


Py_ssize_t TRecordSet::PySequence_Size(TRecordSet* self)
{
  return PyList_Size(self->pRecordList);
}

PyObject* TRecordSet::PySequence_GetItem(TRecordSet* self, Py_ssize_t i)
{
  return PyList_GetItem(self->pRecordList, i);
}

int TRecordSet::PySequence_SetItem(TRecordSet* self, Py_ssize_t i, PyObject *v)
{
  int iRet;
  if(v && CheckItem(v))
  {
    iRet = PyList_SetItem(self->pRecordList, i, v);
  }
  else if (v == NULL)
  {
    iRet = PySequence_DelItem(self->pRecordList, i);
  }
  else
  {
    PyErr_SetString(PyExc_ValueError, "Invalid Object");
    iRet = -1;
  }
  return iRet;
}

PyObject* TRecordSet::PyMapping_GetItem(TRecordSet* self, PyObject *key)
{
  return PyObject_GetItem(self->pRecordList, key);
}

int TRecordSet::PyMapping_SetItem(TRecordSet* self, PyObject *key, PyObject *v)
{
  int iRet;

  if((v && CheckItem(v)) || (v == NULL))
  {
    iRet = PyObject_SetItem(self->pRecordList, key, v);
  }
  else if(PySequence_Check(v))
  {
    int i;
    PyObject* pItem;
    for(i=0 ; i < ::PySequence_Size(v) ; i++)
    {
      pItem = ::PySequence_GetItem(v, i);
      if(!CheckItem(pItem))
      {
        Py_DECREF(pItem);
        PyErr_SetString(PyExc_ValueError, "Invalid Object");
        return -1;
      }
      Py_DECREF(pItem);
    }
    iRet = PyObject_SetItem(self->pRecordList, key, v);
  }
  else
  {
    PyErr_SetString(PyExc_ValueError, "Invalid Object");
    iRet = -1;
  }
  return iRet;
}


PyObject* TRecordSet::PyAdd(TRecordSet* self, PyObject* args)
{
  PyObject* pObj;

  if (! PyArg_ParseTuple(args, "O", &pObj))
    return NULL;

  if(PyObject_TypeCheck(pObj, &TRecord::oType))
  {
    if(PyList_Append(self->pRecordList, pObj) != 0)
      return NULL;
  }
  else if(PyObject_TypeCheck(pObj, &TEvent::oType))
  {
    if(PyList_Append(self->pEventList, pObj) != 0)
      return NULL;
  }
  else
  {
    PyErr_SetString(PyExc_TypeError, "Unsupported Object");
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject* TRecordSet::PyAccept(TRecordSet* self, PyObject* args)
{
  PyObject* pObject;
  if (!PyArg_ParseTuple(args, "O", &pObject))
    return NULL;

  if(CheckItem(pObject))
  {
    Py_INCREF(Py_True);
    return Py_True;
  }
  else if(PySequence_Check(pObject))
  {
    int i;
    PyObject* pItem;
    for(i=0 ; i < ::PySequence_Size(pObject) ; i++)
    {
      pItem = ::PySequence_GetItem(pObject, i);
      if(CheckItem(pItem))
      {
        Py_DECREF(pItem);
        continue;
      }
      Py_DECREF(pItem);
      Py_INCREF(Py_False);
      return Py_False;
    }
    Py_INCREF(Py_True);
    return Py_True;
  }
  Py_INCREF(Py_False);
  return Py_False;
}


PyObject* TRecordSet::PyGetName(TRecordSet* self, void* vp)
{
  Py_INCREF(self->pName);
  return self->pName;
}

int TRecordSet::PySetName(TRecordSet* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;
  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pName;
    self->pName = arg;
    Py_DECREF(pTmp);
    return 0;
  }
  else if(PyBytes_Check(arg))
  {
    pTmp = self->pName;
    self->pName = PyUnicode_FromEncodedObject(arg, "iso8859-15", "ignore");
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}

PyObject* TRecordSet::PyGetSrcname(TRecordSet* self, void* vp)
{
  Py_INCREF(self->pSrcname);
  return self->pSrcname;
}

int TRecordSet::PySetSrcname(TRecordSet* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;
  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pSrcname;
    self->pSrcname = arg;
    Py_DECREF(pTmp);
    return 0;
  }
  else if(PyBytes_Check(arg))
  {
    pTmp = self->pSrcname;
    self->pSrcname = PyUnicode_FromEncodedObject(arg, "iso8859-15", "ignore");
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}


int TRecordSet::Init(TRecordSet *self, PyObject *args, PyObject *kwds)
{
  PyObject* pName = NULL;
  PyObject* pAttributeList = NULL;
  PyObject* pRecordList = NULL;
  PyObject* pEventList = NULL;
  PyObject* pTmp;

  if (! PyArg_ParseTuple(args, "O|OOO", &pName, &pAttributeList, &pRecordList, &pEventList))
    return -1;

  if(PyUnicode_Check(pName))
  {
    pTmp = self->pName;
    Py_INCREF(pName);
    self->pName = pName;
    Py_XDECREF(pTmp);
  }
  else
  {
    PyErr_SetString(PyExc_TypeError, "The name attribute value must be a string");
    return -1;
  }

  if (pAttributeList && PyDict_Check(pAttributeList))
  {
    pTmp = self->pAttributeList;
    Py_INCREF(pAttributeList);
    self->pAttributeList = pAttributeList;
    Py_XDECREF(pTmp);
  }

  if (pRecordList && PyList_Check(pRecordList))
  {
    pTmp = self->pRecordList;
    Py_INCREF(pRecordList);
    self->pRecordList = pRecordList;
    Py_XDECREF(pTmp);
  }

  if (pEventList && PyList_Check(pEventList))
  {
    pTmp = self->pEventList;
    Py_INCREF(pEventList);
    self->pEventList = pEventList;
    Py_XDECREF(pTmp);
  }

  return 0;
}

PyObject* TRecordSet::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  PyObject* obj = type->tp_alloc(type, 0);

  return obj; 
}

PyObject* TRecordSet::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TRecordSet* obj;

  try
  {  
    obj = new TRecordSet();
  }
  catch(TXAlloc)
  {
    return PyErr_NoMemory();
  }

  return (PyObject*)obj;
}

void TRecordSet::Dealloc(TRecordSet* self)
{
  Py_TYPE(self)->tp_free(self);
}

void TRecordSet::Free(TRecordSet* self)
{
  delete self;
}

PyTypeObject TRecordSet::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "RecordSet",                     /*tp_name*/
    sizeof(TRecordSet),              /*tp_basicsize*/
    0,                               /*tp_itemsize*/
    (destructor)TRecordSet::Dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    TRecordSet::sequencemethods,/*tp_as_sequence*/
    TRecordSet::mappingmethods, /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "RecordSet object",         /* tp_doc */
    0,		                /* tp_traverse */
    0,		                /* tp_clear */
    0,		                /* tp_richcompare */
    0,		                /* tp_weaklistoffset */
    0,		                /* tp_iter */
    0,		                /* tp_iternext */
    TRecordSet::methods,        /* tp_methods */
    TRecordSet::members,        /* tp_members */
    TRecordSet::getsets,        /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)TRecordSet::Init,      /* tp_init */
    (allocfunc)TRecordSet::Alloc,    /* tp_alloc */
    (newfunc)TRecordSet::New,        /* tp_new */
    (freefunc)TRecordSet::Free,      /* tp_free */
};

void TRecordSet::PyInit(PyObject* module)
{
  if(PyType_Ready(&TRecordSet::oType) == 0)
    PyModule_AddObject(module, "RecordSet", (PyObject*) &TRecordSet::oType);
}

