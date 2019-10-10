/* mips.cxx
*
*/

#include "mips.h"


PyMethodDef MipsMethods[] = {
  {"MRFread", (PyCFunction)TMipsRecFile::PyRead, METH_VARARGS, "Read"},
  {"MRFwrite", (PyCFunction)TMipsRecFile::PyWrite, METH_VARARGS, "Write"},
  {NULL}  /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "mips",
  "Maschinen Inbetriebnahme und Prüf-System", //  MipsDoc,
  -1,
  MipsMethods,
  NULL,
  NULL,
  NULL,
  NULL
};
#endif

PyObject* InitMips(void)
{
  PyObject *m;

  PyEval_InitThreads();

#if PY_MAJOR_VERSION >= 3
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule("mips", MipsMethods);
#endif

  if(m == 0)
    return 0;

  if(PyType_Ready(&TMipsCore::oType) == 0)
    PyModule_AddObject(m, "mips", (PyObject*) &TMipsCore::oType);

  if(PyType_Ready(&TSensor::oType) == 0)
    PyModule_AddObject(m, "Sensor", (PyObject*) &TSensor::oType);

  if(PyType_Ready(&TChannel::oType) == 0)
    PyModule_AddObject(m, "Channel", (PyObject*) &TChannel::oType);

  if(PyType_Ready(&TRecord::oType) == 0)
    PyModule_AddObject(m, "Record", (PyObject*) &TRecord::oType);

  if(PyType_Ready(&TRecordSet::oType) == 0)
    PyModule_AddObject(m, "Recordset", (PyObject*) &TRecordSet::oType);

//  if(PyType_Ready(&TMipsRecFile::oType) == 0)
//    PyModule_AddObject(m, "MipsRecFile", (PyObject*) &TMipsRecFile::oType);

  return m;
}
