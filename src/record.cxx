/* record.cxx
 *
*/

#include "record.h"

#include <structmember.h>


TRecord::TRecord()
{
  PyObject_Init((PyObject*) this, &TRecord::oType);

  pName = PyUnicode_FromString("");
  if (pName == NULL) throw TXAlloc();

  pUnit = PyUnicode_FromString("");
  if (pUnit == NULL) throw TXAlloc();

  pComment = PyUnicode_FromString("");
  if (pComment == NULL) throw TXAlloc();

  pSource = 0;
  iLength = 0;
  iBufferLength = 0;
  pData = 0;
  dMaxVal = 0.0;
  dMinVal = 0.0;
  dLoRange = 0.0;
  dHiRange = 0.0;
}

TRecord::~TRecord()
{
  if(pData != 0) delete[] pData;
  Py_DECREF(pName);
  Py_DECREF(pUnit);
  Py_DECREF(pComment);
}

bool TRecord::Set(unsigned long pos, double value)
{
  if(pos >= iBufferLength) return false;
  pData[pos] = value;
  return true;
}

bool TRecord::Get(unsigned long pos, double* value)
{
  if(pos >= iLength || pos >= iBufferLength) return false;
  *value = pData[pos];
  return true;
}


double TRecord::GetValue(long pos)
{
  if(pos < 0 || pos >= iLength) return 0;
  return pData[pos];
}


double TRecord::SetValue(long pos, double value)
{
  if(pos < 0 || pos >= iBufferLength) return 0;
  if(pos >= iLength) iLength = pos+1; 
  return pData[pos] = value;
}

long TRecord::SetLength(long length)
{
  if(length < 0) length = 0;
  if(length > this->iBufferLength) length = this->iBufferLength;
  this->iLength = length;
  return this->iLength;
}

int TRecord::UpdateStatVals()
{
  int i;
  if(this->iLength < 1)
  {
    this->dMinVal = this->dMaxVal = 0.0;
  }
  else
  {
    this->dMinVal = pData[0];
    this->dMaxVal = pData[0];
    for( i=1 ; i < this->iLength ; i++ )
    {
      if(this->dMinVal > pData[i]) this->dMinVal = pData[i];
      if(this->dMaxVal < pData[i]) this->dMaxVal = pData[i];
    }
  }
  return 0;
}

int TRecord::SetRange(double lo, double hi)
{
  this->dLoRange = lo;
  this->dHiRange = hi;
}

int TRecord::GenerateSequence1(double start, double end)
{
  double step;
  step = (end - start) / (double) this->iLength;
  return GenerateSequence2(start, step);
}

int TRecord::GenerateSequence2(double start, double step)
{
  int i;
  double val = start;
  if(this->iLength <= 0) return 1;
  pData[0] = val;
  for(i=1 ; i < this->iLength ; i++) pData[i] = val += step;
  if(step >= 0.0)
  {
    this->dMinVal = start;
    this->dMaxVal = val;
  }
  else
  {
    this->dMinVal = val;
    this->dMaxVal = start;
  }
  this->dLoRange = this->dMinVal;
  this->dHiRange = this->dMaxVal;
  return 0;
}



PyMemberDef TRecord::members[] = {
//  {"length", T_ULONG, offsetof(TRecord, iLength), 0, "Record Length"},
//  {"bufferlength", T_ULONG, offsetof(TRecord, iBufferLength), READONLY, "Buffer Length"},
  {NULL}  /* Sentinel */
};


PyMethodDef TRecord::methods[] = {
    {"get", (PyCFunction)TRecord::PyGet, METH_VARARGS, "Get Value"},
    {"set", (PyCFunction)TRecord::PySet, METH_VARARGS, "Set Value"},
    {NULL}  /* Sentinel */
};

PyGetSetDef TRecord::getsets[] = {
  {"name", (getter)TRecord::PyGetName, (setter)TRecord::PySetName, "Name", NULL},
  {"unit", (getter)TRecord::PyGetUnit, (setter)TRecord::PySetUnit, "Unit", NULL},
  {"comment", (getter)TRecord::PyGetComment, (setter)TRecord::PySetComment, "Comment", NULL},
  {"range", (getter)TRecord::PyGetRange, (setter)TRecord::PySetRange, "Range", NULL},
  {NULL}  /* Sentinel */
};

PySequenceMethods TRecord::sequencemethods[] = {
  {
    (lenfunc)PySequence_Size,               // lenfunc sq_length 
    0,                                      // binaryfunc sq_concat 
    0,                                      // ssizeargfunc sq_repeat 
    (ssizeargfunc)PySequence_GetItem,       // ssizeargfunc sq_item
//    (ssizessizeargfunc)PySequence_GetSlice, // ssizessizeargfunc sq_slice
//    (ssizeobjargproc)PySequence_SetItem,    // ssizeobjargproc sq_ass_item
    0,                  // ssizessizeobjargproc sq_ass_slice
    0,                  // objobjproc sq_contains 
    0,                  // binaryfunc sq_inplace_concat 
    0,                  // ssizeargfuncs sq_inplace_repeat
  },
};

Py_ssize_t TRecord::PySequence_Size(TRecord* o)
{
  return o->iLength;
}

PyObject* TRecord::PySequence_GetItem(TRecord* o, Py_ssize_t i)
{
  if(i >= o->iBufferLength) return NULL;
  return PyFloat_FromDouble(o->pData[i]);
}

int TRecord::PySequence_SetItem(TRecord* o, Py_ssize_t i, PyObject *v)
{
  double value;
  value = PyFloat_AsDouble(v);

  if(i >= o->iBufferLength) return -1;
  o->pData[i] = value;

  return 0;
}

PyObject* TRecord::PySequence_GetSlice(TRecord *o, Py_ssize_t i1, Py_ssize_t i2)
{
  int i;
  int len;
  PyObject* pObj;

  if(i1 >= o->iBufferLength) return NULL;
  if(i2 >= o->iBufferLength) return NULL;

  len = i2 - i1;
  pObj = PyTuple_New(len);
  for(i=0 ; i< len ; i++)
  {
    PyTuple_SetItem(pObj, i, PyFloat_FromDouble(o->pData[i1 + i]));
  }
  return pObj;
}


PyObject* TRecord::PyGet(TRecord* self, PyObject* args)
{
  unsigned long index;

  if (! PyArg_ParseTuple(args, "k", &index))
    return NULL;

  if(index >= self->iLength)
    return NULL;

  return PyFloat_FromDouble(self->pData[index]);
//   Py_BuildValue("d", self->pData[index]);
}

PyObject* TRecord::PySet(TRecord* self, PyObject* args)
{
  unsigned long index;
  double value;

  if (! PyArg_ParseTuple(args, "kd", &index, &value))
    return NULL;

  if(index >= self->iBufferLength)
    return NULL;
  if(index >= self->iLength) self->iLength = index+1;

  self->pData[index] = value;
  if(value > self->dMaxVal) self->dMaxVal = value;
  if(value < self->dMinVal) self->dMinVal = value;

//  return PyFloat_FromDouble(value);
  Py_INCREF(Py_None);
  return Py_None;
}


PyObject* TRecord::PyGetName(TRecord* self, void* vp)
{
  Py_INCREF(self->pName);
  return self->pName;
}

int TRecord::PySetName(TRecord* self, PyObject* arg, void* vp)
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

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}

PyObject* TRecord::PyGetUnit(TRecord* self, void* vp)
{
  Py_INCREF(self->pUnit);
  return self->pUnit;
}

int TRecord::PySetUnit(TRecord* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;

  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pUnit;
    self->pUnit = arg;
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}


PyObject* TRecord::PyGetComment(TRecord* self, void* vp)
{
  Py_INCREF(self->pComment);
  return self->pComment;
}

int TRecord::PySetComment(TRecord* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;

  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pComment;
    self->pComment = arg;
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}


PyObject* TRecord::PyGetRange(TRecord* self, void* vp)
{
  PyObject* pObj;
  pObj = PyTuple_New(2);
  PyTuple_SetItem(pObj, 0, PyFloat_FromDouble(self->dLoRange));
  PyTuple_SetItem(pObj, 1, PyFloat_FromDouble(self->dHiRange));
  return pObj;
}

int TRecord::PySetRange(TRecord* self, PyObject* arg, void* vp)
{
  if(PyTuple_Check(arg))
  {
    if(PyTuple_Size(arg) == 2)
    {
      self->dLoRange = PyFloat_AsDouble(PyTuple_GetItem(arg, 0));
      self->dHiRange = PyFloat_AsDouble(PyTuple_GetItem(arg, 1));      
      return 0;
    }
    PyErr_SetString(PyExc_TypeError, "Tuple with 2 elements expected");
    return -1;
  }
  PyErr_SetString(PyExc_TypeError, "Tuple expected");
  return -1;
}


int TRecord::Init(TRecord *self, PyObject *args, PyObject *kwds)
{
  PyObject* pName = NULL;
  PyObject* pUnit = NULL;
  unsigned long iLength = 0;
  PyObject* pArg = NULL;

  PyObject* pTmp;

  if (! PyArg_ParseTuple(args, "OOk|O", &pName, &pUnit, &iLength, &pArg))
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

  if(PyUnicode_Check(pUnit))
  {
    pTmp = self->pUnit;
    Py_INCREF(pUnit);
    self->pUnit = pUnit;
    Py_XDECREF(pTmp);
  }
  else
  {
    PyErr_SetString(PyExc_TypeError, "The name attribute value must be a string");
    return -1;
  }

  self->iLength = 0;
  self->iBufferLength = iLength;
  if(self->pData != 0) delete[] self->pData;
  self->pData = new double[iLength];
  if(self->pData == NULL) return -1;

  if(pArg)
  {
    if(PyGen_Check(pArg))
    {
      int i;
      PyObject* pVal;
      for(i=0 ; i<iLength ; i++)
      {
        pVal = PyIter_Next(pArg);
        if(pVal == NULL) break;
        self->pData[i] = PyFloat_AsDouble(pVal);
      }
      self->iLength = i;
    }
    else if(PyTuple_Check(pArg))
    {
      int i;
      int len;
      PyObject* pVal;
      len = PyTuple_Size(pArg);
      if(len > iLength) len = iLength;
      for(i=0 ; i<len ; i++)
      {
        pVal = PyTuple_GetItem(pArg, i);
        self->pData[i] = PyFloat_AsDouble(pVal);
      }
      self->iLength = len;
    }
    else if(PyFloat_Check(pArg))
    {
      int i;
      double val;
      val = PyFloat_AsDouble(pArg);
      for(i=0; i<iLength ; i++) self->pData[i] = val;
      self->iLength = iLength;
    }
  }

  return 0;
}

PyObject* TRecord::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  PyObject* obj = type->tp_alloc(type, 0);

  return obj; 
}

PyObject* TRecord::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TRecord* obj;

  try
  {  
    obj = new TRecord();
  }
  catch(TXAlloc)
  {
    return PyErr_NoMemory();
  }

  return (PyObject*)obj;
}

void TRecord::Dealloc(TRecord* self)
{
  Py_TYPE(self)->tp_free(self);
}

void TRecord::Free(TRecord* self)
{
  delete self;
}


PyTypeObject TRecord::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Record",                  /*tp_name*/
    sizeof(TRecord),           /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)TRecord::Dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    TRecord::sequencemethods,  /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Record object",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TRecord::methods,          /* tp_methods */
    TRecord::members,          /* tp_members */
    TRecord::getsets,          /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TRecord::Init,   /* tp_init */
    (allocfunc)TRecord::Alloc, /* tp_alloc */
    (newfunc)TRecord::New,     /* tp_new */
    (freefunc)TRecord::Free,   /* tp_free */
};



void TRecord::PyInit(PyObject* module)
{
  if(PyType_Ready(&TRecord::oType) == 0)
    PyModule_AddObject(module, "Record", (PyObject*) &TRecord::oType);

}
