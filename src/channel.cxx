/* channel.cxx
 *
 *
*/

#include "channel.h"

#include <structmember.h>

TChannel::TChannel( )
{
  PyObject_Init((PyObject*) this, &TChannel::oType);

  pName = PyUnicode_FromString("");
  if (pName == NULL)
  {
    throw TXAlloc();
  }

  pUnit = PyUnicode_FromString("");
  if (pUnit == NULL)
  {
    Py_DECREF(pName);
    throw TXAlloc();
  }

  function = Py_BuildValue("");
  if (function == NULL)
  {
    Py_DECREF(pName);
    Py_DECREF(pUnit);
    throw TXAlloc();
  }

  value = 0.0;
  min = 0;
  max = 0;
  dRangeMin = 0.0;
  dRangeMax = 1.0;
  validmin = 0.0;
  validmax = 0.0;
  bRecord = 0;
  bHidden = 0;
}

    
TChannel::~TChannel( )
{
  Py_XDECREF(pName);
  Py_XDECREF(pUnit);
  Py_XDECREF(function);
}

void TChannel::Update()
{
  PyObject* result;

  if(function && PyCallable_Check(function)) 
  {
    result = PyObject_CallObject(function, NULL);
    if(result != NULL)
    {
      value = PyFloat_AsDouble(result);

      if(value > max) max = value;
      if(value < min) min = value;
    }
  }
}


PyMemberDef TChannel::members[] = {
  {"value", T_DOUBLE, offsetof(TChannel, value), 0, "Value"},
  {"min", T_DOUBLE, offsetof(TChannel, min), 0, "Min Value"},
  {"max", T_DOUBLE, offsetof(TChannel, max), 0, "Max Value"},
  {"RangeMin", T_DOUBLE, offsetof(TChannel, dRangeMin), 0, "Range Min Value"},
  {"RangeMax", T_DOUBLE, offsetof(TChannel, dRangeMax), 0, "Range Max Value"},
  {"validmin", T_DOUBLE, offsetof(TChannel, validmin), 0, "Min valid Value"},
  {"validmax", T_DOUBLE, offsetof(TChannel, validmax), 0, "Max valid Value"},
  {"function", T_OBJECT_EX, offsetof(TChannel, function), 0, "Source Function Defintion"},
  {"recording", T_BOOL, offsetof(TChannel, bRecord), 0, "Recording enabled"},
  {"hidden", T_BOOL, offsetof(TChannel, bHidden), 0, "Hidden Channel"},
  {NULL}  /* Sentinel */
};

PyGetSetDef TChannel::getsets[] = {
  {"name", (getter)TChannel::PyGetName, (setter)TChannel::PySetName, "Name", NULL},
  {"unit", (getter)TChannel::PyGetUnit, (setter)TChannel::PySetUnit, "Unit", NULL},
  {NULL}  /* Sentinel */
};

PyMethodDef TChannel::methods[] = {
  {"Update", (PyCFunction)TChannel::PyUpdate, METH_NOARGS, "Update Values"},
  {NULL}  /* Sentinel */
};


PyObject* TChannel::PyUpdate(TChannel* self, PyObject *args)
{
  self->Update();
  Py_RETURN_TRUE;
}


PyObject* TChannel::PyGetName(TChannel* self, void* vp)
{
  Py_INCREF(self->pName);
  return self->pName;
}

int TChannel::PySetName(TChannel* self, PyObject* arg, void* vp)
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

PyObject* TChannel::PyGetUnit(TChannel* self, void* vp)
{
  Py_INCREF(self->pUnit);
  return self->pUnit;
}

int TChannel::PySetUnit(TChannel* self, PyObject* arg, void* vp)
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
  else if(PyBytes_Check(arg))
  {
    pTmp = self->pUnit;
    self->pUnit = PyUnicode_FromEncodedObject(arg, "iso8859-15", "ignore");
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}

int TChannel::Init(TChannel *self, PyObject *args, PyObject *kwds)
{
  PyObject* pName = NULL;
  PyObject* pUnit = NULL;
  PyObject* tmp;
  static char *kwlist[] = {"Name", "Unit", NULL};

  if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &pName, &pUnit))
    return -1; 

  if(PySetName(self, pName, NULL) != 0)
    return -1;

  if(pUnit && (PySetUnit(self, pUnit, NULL) != 0))
    return -1;

  self->value = 0.0;
  self->min = 0;
  self->max = 0;
  self->dRangeMin = 0.0;
  self->dRangeMax = 1.0;
  self->validmin = PyFloat_GetMin();
  self->validmax = PyFloat_GetMax();
  self->bRecord = 0;
  self->bHidden = 0;

  return 0;
}

PyObject* TChannel::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  PyObject* obj = type->tp_alloc(type, 0);

  return obj; 
}

PyObject* TChannel::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TChannel* obj;

  try
  {  
    obj = new TChannel();
  }
  catch(TXAlloc)
  {
    Py_DECREF(obj);
    return PyErr_NoMemory();
  }

  return (PyObject*)obj;
}

void TChannel::Dealloc(TChannel* self)
{
  Py_TYPE(self)->tp_free((PyObject*)self);
}

void TChannel::Free(TChannel* self)
{
  delete self;
}


PyTypeObject TChannel::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Channel",                 /*tp_name*/
    sizeof(TChannel),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)TChannel::Dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Channel object",          /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TChannel::methods,         /* tp_methods */
    TChannel::members,         /* tp_members */
    TChannel::getsets,         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TChannel::Init,  /* tp_init */
    (allocfunc)TChannel::Alloc,/* tp_alloc */
    (newfunc)TChannel::New,    /* tp_new */
    (freefunc)TChannel::Free,  /* tp_free */
};



void TChannel::PyInit(PyObject* module)
{
  if(PyType_Ready(&TChannel::oType) == 0)
    PyModule_AddObject(module, "Channel", (PyObject*) &TChannel::oType);
}
