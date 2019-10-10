/* event.cxx
 *
*/

#include "event.h"
#include <structmember.h>

TEvent::TEvent()
{
  PyObject_Init((PyObject*) this, &TEvent::oType);

  pEventInfo = Py_BuildValue("");
  if (pEventInfo == NULL)
  {
    throw TXAlloc();
  }

  dTime = 0.0;
}

TEvent::TEvent(PyObject* eventinfo, double time)
{
  PyObject_Init((PyObject*) this, &TEvent::oType);

  if(eventinfo)
  {
    Py_INCREF(eventinfo);
    this->pEventInfo = eventinfo;
  }
    else
  {
    throw TXArgument();
  }

  this->dTime = time;
}


TEvent::~TEvent()
{
  Py_DECREF(pEventInfo);
}


PyMemberDef TEvent::members[] = {
  {"info", T_OBJECT_EX, offsetof(TEvent, pEventInfo), 0, "Event Info"},
  {"time", T_DOUBLE, offsetof(TEvent, dTime), 0, "time"},
  {NULL}  /* Sentinel */
};


PyMethodDef TEvent::methods[] = {
    {NULL}  /* Sentinel */
};

int TEvent::Init(TEvent *self, PyObject *args, PyObject *kwds)
{
  PyObject* eventinfo = NULL;

  PyObject* tmp;

  if (! PyArg_ParseTuple(args, "|Od", &eventinfo, &self->dTime))
    return -1;

  if (eventinfo)
  {
    tmp = self->pEventInfo;
    Py_INCREF(eventinfo);
    self->pEventInfo = eventinfo;
    Py_XDECREF(tmp);
  }

  return 0;
}

PyObject* TEvent::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  PyObject* obj = type->tp_alloc(type, 0);

  return obj; 
}

PyObject* TEvent::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TEvent* obj;

  try
  {  
    obj = new TEvent();
  }
  catch(TXAlloc)
  {
    return PyErr_NoMemory();
  }

  return (PyObject*)obj;
}

void TEvent::Dealloc(TEvent* self)
{
  Py_TYPE(self)->tp_free(self);
}

void TEvent::Free(TEvent* self)
{
  delete self;
}



PyTypeObject TEvent::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Event",                   /*tp_name*/
    sizeof(TEvent),            /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)TEvent::Dealloc, /*tp_dealloc*/
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
    "Event object",            /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TEvent::methods,           /* tp_methods */
    TEvent::members,           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TEvent::Init,    /* tp_init */
    (allocfunc)TEvent::Alloc,  /* tp_alloc */
    (newfunc)TEvent::New,      /* tp_new */
    (freefunc)TEvent::Free,    /* tp_free */
};



void TEvent::PyInit(PyObject* module)
{
  if(PyType_Ready(&TEvent::oType) == 0)
    PyModule_AddObject(module, "Event", (PyObject*) &TEvent::oType);

}
