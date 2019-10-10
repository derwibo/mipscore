/* sensor.cxx
 *
 *
*/

#include "sensor.h"

#include <structmember.h>

TSensor::TSensor()
{
  PyObject_Init((PyObject*) this, &TSensor::oType);

  pSourceFunction = Py_BuildValue("");
  if (pSourceFunction == NULL)
  {
    throw TXAlloc();
  }

  dPhysMin = 0.0;
  dPhysMax = 1.0;
  dLogMin = 0.0;
  dLogMax = 1.0;
}

TSensor::TSensor(PyObject* sourcefunction, double phys_min, double phys_max, double log_min, double log_max)
{
  PyObject_Init((PyObject*) this, &TSensor::oType);

  if(PyCallable_Check(sourcefunction))
  {
    Py_INCREF(sourcefunction);
    this->pSourceFunction = sourcefunction;
  }
    else
  {
    throw TXArgument();
  }

  this->dPhysMin = phys_min;
  this->dPhysMax = phys_max;
  this->dLogMin = log_min;
  this->dLogMax = log_max;
}


TSensor::~TSensor()
{
  Py_DECREF(pSourceFunction);
}


PyMemberDef TSensor::members[] = {
  {"function", T_OBJECT_EX, offsetof(TSensor, pSourceFunction), 0, "Source Function Defintion"},
  {"phys_min", T_DOUBLE, offsetof(TSensor, dPhysMin), 0, "Physical Min Value"},
  {"phys_max", T_DOUBLE, offsetof(TSensor, dPhysMax), 0, "Physical Max Value"},
  {"log_min", T_DOUBLE, offsetof(TSensor, dLogMin), 0, "Logical Min Value"},
  {"log_max", T_DOUBLE, offsetof(TSensor, dLogMax), 0, "Logical Max Value"},
  {NULL}  /* Sentinel */
};


PyMethodDef TSensor::methods[] = {
    {NULL}  /* Sentinel */
};

PyObject* TSensor::Call(TSensor* self, PyObject *args, PyObject *kwds)
{
  PyObject* result;
  double    dresult;

  if(PyCallable_Check(self->pSourceFunction))
  {
    result = PyObject_CallObject(self->pSourceFunction, NULL);
    if(result != NULL)
    {
      dresult = PyFloat_AsDouble(result);
      dresult = self->dLogMin + (self->dLogMax - self->dLogMin) / (self->dPhysMax - self->dPhysMin) * (dresult - self->dPhysMin);
      return PyFloat_FromDouble(dresult);
    }
  }
  return NULL;
}

int TSensor::Init(TSensor *self, PyObject *args, PyObject *kwds)
{
  PyObject* sourcefunction = NULL;

  PyObject* tmp;

  if (! PyArg_ParseTuple(args, "|Odddd", &sourcefunction, &self->dPhysMin, &self->dPhysMax, &self->dLogMin, &self->dLogMax))
    return -1;

  if (sourcefunction && PyCallable_Check(sourcefunction))
  {
    tmp = self->pSourceFunction;
    Py_INCREF(sourcefunction);
    self->pSourceFunction = sourcefunction;
    Py_XDECREF(tmp);
  }

  return 0;
}

PyObject* TSensor::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  PyObject* obj = type->tp_alloc(type, 0);

  return obj; 
}

PyObject* TSensor::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TSensor* obj;

  try
  {  
    obj = new TSensor();
  }
  catch(TXAlloc)
  {
    return PyErr_NoMemory();
  }

  return (PyObject*)obj;
}

void TSensor::Dealloc(TSensor* self)
{
  Py_TYPE(self)->tp_free(self);
}

void TSensor::Free(TSensor* self)
{
  delete self;
}



PyTypeObject TSensor::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Sensor",                  /*tp_name*/
    sizeof(TSensor),           /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)TSensor::Dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    (ternaryfunc)TSensor::Call, /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Sensor object",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TSensor::methods,          /* tp_methods */
    TSensor::members,          /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TSensor::Init,   /* tp_init */
    (allocfunc)TSensor::Alloc, /* tp_alloc */
    (newfunc)TSensor::New,     /* tp_new */
    (freefunc)TSensor::Free,   /* tp_free */
};



void TSensor::PyInit(PyObject* module)
{
  if(PyType_Ready(&TSensor::oType) == 0)
    PyModule_AddObject(module, "Sensor", (PyObject*) &TSensor::oType);

}
