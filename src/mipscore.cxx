/* mipscore.cxx
 * 
*/

#include <Python.h>
#include <structmember.h>

#include <sys/timerfd.h>

#include "mipscore.h"
#include "channel.h"
#include "sensor.h"
#include "event.h"

double TMipsCore::dSampleRates[] = {  0.020 ,  0.050 ,  0.100 ,  0.200 ,  0.500 ,  1.000 ,  2.000  };

TMipsCore::TMipsCore()
{
  PyObject_Init((PyObject*) this, &TMipsCore::oType);

  fdThreadTimer = -1;
  //pthread_cond_init(&cond, NULL);
  //pthread_mutex_init(&mutex, NULL);

  // hMainThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  eScriptState = Script_Off;
  eThreadSignal = None;
  eRecordingState = Recording_Off;
  pRecordSet = 0;

  pMainThreadState = PyThreadState_Get();;
  pScriptThreadState = NULL;
  pAttributeList = 0;
  pChannelList = 0;
  pEventList = 0;

  pStartHandler = 0;
  pStopHandler  = 0;
  pTickHandler  = 0;
  pRecordingStartHandler = 0;
  pRecordingStopHandler  = 0;

  iScriptRate = 2;
  iSampleRate = 2;

  dScriptTime  = 0.0;
  dScriptRate  = dSampleRates[iScriptRate];
  dRecordTime  = 0.0;
  dSampleRate  = dSampleRates[iSampleRate];
  uRecordLength = 601;
  uRecordCount = 0;

}

TMipsCore::~TMipsCore()
{
//  pthread_mutex_destroy(&mutex);
//  pthread_cond_destroy(&cond);
//  CloseHandle(hMainThreadEvent);

  Py_XDECREF(pEventList);
  Py_XDECREF(pChannelList);
  Py_XDECREF(pAttributeList);
}


int TMipsCore::_StartRecord()
{
  int n;
  int i;
  TRecord* prec;
  PyObject* item;
  TChannel* chn;

  if(eRecordingState != Recording_Running)
  {
    PyObject* name;
    PyObject* list;
    PyObject* eventlist;
    TRecordSet* rs;

    if(pRecordSet)
    {
      Py_DECREF((PyObject*) pRecordSet);
      pRecordSet = 0;
    }

#if PY_MAJOR_VERSION >= 3
    name = PyUnicode_FromString("NewRecordset");
#else
    name = PyString_FromString("NewRecordset");
#endif
    if(name == NULL) return 1;

    list = PyList_New(0);
    if(list == NULL) return 1;

    eventlist = PyList_New(0);
    if(eventlist == NULL) return 1;

    rs = (TRecordSet*) PyObject_CallObject((PyObject*)&TRecordSet::oType, Py_BuildValue("OOOO", name, pAttributeList, list, eventlist));
                      // new TRecordSet(TEXT(""), (PyObject*)pAttributeList, list, eventlist);
    if(rs == NULL) return 2;

    PyDict_SetItemString(rs->GetAttributeList(), "SampleRate", PyFloat_FromDouble(dSampleRate));

    pRecordSet = rs;
//    uRecordLength = length;
    if(uRecordLength <= 0) uRecordLength = 601;
    dRecordTime = 0.0;
    uRecordCount = 0;

    n = PyList_Size((PyObject*)pChannelList);

    for (i = 0; i < n; i++)
    {
      item = PyList_GetItem((PyObject*)pChannelList, i);
      if (!PyObject_TypeCheck(item, &TChannel::oType)) continue;
      chn = (TChannel*)item;
      if(chn->RecordingEnabled())
      {
        prec = (TRecord*) PyObject_CallObject((PyObject*)&TRecord::oType, Py_BuildValue("OOk", chn->GetName(), chn->GetUnit(), uRecordLength));
               // new TRecord(chn->GetName(), chn->GetUnit(), uRecordLength);
        prec->pSource = (PyObject*)chn;
        PyList_Append(rs->GetRecordList(), (PyObject*) prec); 
      }
    }

    eThreadSignal |= Start_Recording;

    if(eScriptState != Script_Running) _RunProgram();
  }
  return 0;
}

int TMipsCore::_SaveRecord(const char* filename, int filetype)
{

  int res;
//  TRecordingFile* recfile;

  wchar_t fullpathname[256];
  wchar_t* p1;
  wchar_t* p2;
  wchar_t* p3;

  if(eRecordingState != Recording_Stopped) return 1;

/*  recfile = new TRecordingFile();

  res = recfile->Create(filename);

  if( res )
  {
    PySys_WriteStdout("SaveRecord: OpenFile returned error\n");

    return 2;
  }

  recfile->Write(pRecordSet, uRecordCount);

  recfile->Close();

  delete recfile;
*/

  return 0;
}


int TMipsCore::_LoadProgram(char* filename)
{
  PyObject* PyFileObject;

/*  PyFileObject = PyFile_FromString(filename, "r");
  if(PyFile_Check(PyFileObject))
  {
    PyRun_SimpleFile(PyFile_AsFile(PyFileObject), filename);
    Py_DECREF(PyFileObject);
  }
  else
  {
    PySys_WriteStdout("PyFileObject error");
  }
*/

  return 0;
}

int TMipsCore::_RunProgram()
{
  PyThreadState* _save;

  if(eScriptState != Script_Running)
  {
//    DWORD lpThreadId;

    eThreadSignal |= Start_Script;

    // PyEval_InitThreads();

    // _save = PyEval_SaveThread();
    pMainThreadState = PyThreadState_Get();

//    hMainThread = CreateThread(NULL, 0, ScriptThreadFunc, (LPVOID)this, 0, &lpThreadId);
    pthread_create(&hMainThread, NULL, ScriptThreadFunc, this);

    // PyEval_RestoreThread(_save);
  }
  return 0;
}

int TMipsCore::_StopProgram()
{
  PyThreadState* _save;
//  DWORD dwExitCode;
  if(eScriptState == Script_Running)
  {
    _save = PyEval_SaveThread();

    eThreadSignal = Stop_Script;
//    pthread_cancel(hMainThread);
    pthread_join(hMainThread, NULL);
/*
    SetEvent(hMainThreadEvent);
    do
    {
      GetExitCodeThread(hMainThread, &dwExitCode);
    }
    while(dwExitCode == STILL_ACTIVE);
*/
    PyEval_RestoreThread(_save);
  }
  return 0;
}

int TMipsCore::Reset()
{
  return 0;
}

int TMipsCore::StartRecord()
{
  int iRet;

  PyEval_AcquireLock();
  PyThreadState_Swap(pMainThreadState);

  iRet = _StartRecord();

  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  return iRet;
}

int TMipsCore::StopRecord()
{
  if(eRecordingState == Recording_Running)
  {
    eThreadSignal |= Stop_Recording;
  }
  return 0;
}

int TMipsCore::SaveRecord(char* filename, int filetype)
{
  int iRet;

  PyEval_AcquireLock();
  PyThreadState_Swap(pMainThreadState);

  iRet = _SaveRecord(filename, filetype);

  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  return iRet;
}

int TMipsCore::SetScriptRate(unsigned int iScriptRate)
{
  this->iScriptRate = (iScriptRate <= 2) ? iScriptRate : 2 ;
  this->dScriptRate = dSampleRates[iScriptRate];

  return 0;
}

int TMipsCore::SetSampleRate(unsigned int iSampleRate)
{
  this->iSampleRate = (iSampleRate <= 6) ? iSampleRate : 6 ;
  this->dSampleRate = dSampleRates[iSampleRate];

  return 0;
}

int TMipsCore::SetRecordingParams(unsigned long length, unsigned int samplerate)
{
  if(eRecordingState == Recording_Running) return 1;

  SetSampleRate(samplerate);

  uRecordLength = length;
  return 0;
}



PyMemberDef TMipsCore::members[] = {
  {"attributelist", T_OBJECT_EX, offsetof(TMipsCore, pAttributeList), READONLY, "Attribute Dictionary"},
  {"channellist", T_OBJECT_EX, offsetof(TMipsCore, pChannelList), READONLY, "Channel List"},
  {"starthandler", T_OBJECT, offsetof(TMipsCore, pStartHandler), 0, "Start Handler"},
  {"stophandler", T_OBJECT, offsetof(TMipsCore, pStopHandler), 0, "Stop Handler"},
  {"tickhandler", T_OBJECT, offsetof(TMipsCore, pTickHandler), 0, "Tick Handler"},
  {"recordingstarthandler", T_OBJECT, offsetof(TMipsCore, pRecordingStartHandler), 0, "Recording Start Handler"},
  {"recordingstophandler", T_OBJECT, offsetof(TMipsCore, pRecordingStopHandler), 0, "Recording Stop Handler"},
  {"time", T_DOUBLE, offsetof(TMipsCore, dScriptTime), READONLY, "Script Time"},
  {"dt", T_DOUBLE, offsetof(TMipsCore, dScriptRate), READONLY, "ScriptRate"},
  {"recordingtime", T_DOUBLE, offsetof(TMipsCore, dRecordTime), READONLY, "Recording Time"},
  {"recordingstate", T_INT, offsetof(TMipsCore, eRecordingState), READONLY, "Recording State"},
  {"scriptstate", T_INT, offsetof(TMipsCore, eScriptState), READONLY, "Script State"},
//  {"recordingdir", T_STRING, offsetof(TMipsCore, sRecordDir), READONLY, "Recording Directory"},
  {NULL}  /* Sentinel */
};

PyMethodDef TMipsCore::methods[] = {
  {"Start", (PyCFunction)TMipsCore::PyStart, METH_NOARGS, "Start Measure"},
  {"Stop", (PyCFunction)TMipsCore::PyStop, METH_NOARGS, "Stop Measure"},
  {"StartRecord", (PyCFunction)TMipsCore::PyStartRecord, METH_NOARGS, "Start Record"},
  {"StopRecord", (PyCFunction)TMipsCore::PyStopRecord, METH_NOARGS, "Stop Record"},
  {"SaveRecord", (PyCFunction)TMipsCore::PySaveRecord, METH_VARARGS, "Save Record"},
  {"AddEvent", (PyCFunction)TMipsCore::PyAddEvent, METH_VARARGS, "Add Event"},
  {NULL}  /* Sentinel */
};

PyGetSetDef TMipsCore::getsets[] = {
  {"ScriptDir", (getter)TMipsCore::PyGetScriptDir, (setter)TMipsCore::PySetScriptDir, "Script Directory", NULL},
  {"RecordDir", (getter)TMipsCore::PyGetRecordDir, (setter)TMipsCore::PySetRecordDir, "Record Directory", NULL},
  {"ScriptRate", (getter)TMipsCore::PyGetScriptRate, (setter)TMipsCore::PySetScriptRate, "Script Rate", NULL},
  {"SampleRate", (getter)TMipsCore::PyGetSampleRate, (setter)TMipsCore::PySetSampleRate, "Sample Rate", NULL},
  {NULL}  /* Sentinel */
};

PyObject* TMipsCore::PyStart(PyObject* self, PyObject *args)
{
  return Py_BuildValue("i", ((TMipsCore*)self)->_RunProgram());
}

PyObject* TMipsCore::PyStop(PyObject* self, PyObject *args)
{
  return Py_BuildValue("i", ((TMipsCore*)self)->_StopProgram());
}

PyObject* TMipsCore::PyStartRecord(PyObject* self, PyObject *args)
{
  return Py_BuildValue("i", ((TMipsCore*)self)->_StartRecord());
}

PyObject* TMipsCore::PyStopRecord(PyObject* self, PyObject *args)
{
  return Py_BuildValue("i", ((TMipsCore*)self)->StopRecord());
}

PyObject* TMipsCore::PySaveRecord(PyObject* self, PyObject *args)
{
  PyObject* pFileName;
  char* strFileName;
  int iFileType;

  iFileType = 0;

#ifdef UNICODE
  if (! PyArg_ParseTuple(args, "U|i", &pFileName, &iFileType))
    return NULL; 

  return Py_BuildValue("i", ((TMipsCore*)self)->_SaveRecord(PyUnicode_AS_DATA(pFileName), iFileType));
#else
  if (! PyArg_ParseTuple(args, "s|i", &strFileName, &iFileType))
    return NULL; 

  return Py_BuildValue("i", ((TMipsCore*)self)->_SaveRecord(strFileName, iFileType));
#endif
}

PyObject* TMipsCore::PyAddEvent(PyObject* self, PyObject *args)
{
  PyObject* eventinfo = NULL;
  PyObject* eventobj = NULL;

  if (! PyArg_ParseTuple(args, "|O", &eventinfo))
    return NULL;

  if (eventinfo)
  {
    eventobj = (PyObject*) new TEvent(eventinfo, ((TMipsCore*)self)->dScriptTime);
    PyList_Append((PyObject*)((TMipsCore*)self)->pEventList, eventobj);    

    if(((TMipsCore*)self)->eRecordingState == Recording_Running)
    {
      eventobj = (PyObject*) new TEvent(eventinfo, ((TMipsCore*)self)->dRecordTime);
      PyList_Append((PyObject*)((TMipsCore*)self)->pRecordSet->pEventList, eventobj);     
    }
  }
  return NULL;
}


PyObject* TMipsCore::PyGetScriptDir(TMipsCore* self, void* vp)
{
  Py_INCREF(self->pScriptDir);
  return self->pScriptDir;
}

int TMipsCore::PySetScriptDir(TMipsCore* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;
  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pScriptDir;
    self->pScriptDir = arg;
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}

PyObject* TMipsCore::PyGetRecordDir(TMipsCore* self, void* vp)
{
  Py_INCREF(self->pRecordDir);
  return self->pRecordDir;
}

int TMipsCore::PySetRecordDir(TMipsCore* self, PyObject* arg, void* vp)
{
  PyObject* pTmp;
  if(PyUnicode_Check(arg))
  {
    Py_INCREF(arg);
    pTmp = self->pRecordDir;
    self->pRecordDir = arg;
    Py_DECREF(pTmp);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "String expected");
  return -1;
}

PyObject* TMipsCore::PyGetScriptRate(TMipsCore* self, void* vp)
{
  return PyLong_FromLong(self->iScriptRate);
}

int TMipsCore::PySetScriptRate(TMipsCore* self, PyObject* arg, void* vp)
{
  long index;
  if(PyLong_Check(arg))
  {
    index = PyLong_AsLong(arg);
    self->SetScriptRate(index);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "Integer expected");
  return -1;
}

PyObject* TMipsCore::PyGetSampleRate(TMipsCore* self, void* vp)
{
  return PyLong_FromLong(self->iSampleRate);
}

int TMipsCore::PySetSampleRate(TMipsCore* self, PyObject* arg, void* vp)
{
  long index;
  if(PyLong_Check(arg))
  {
    index = PyLong_AsLong(arg);
    self->SetSampleRate(index);
    return 0;
  }

  PyErr_SetString(PyExc_TypeError, "Integer expected");
  return -1;
}


int TMipsCore::Init(TMipsCore *self, PyObject *args, PyObject *kwds)
{
  PyObject* pAttributeList;
  PyObject* pChannelList;
  PyObject* pEventList;

  PyObject* pTmp;

  //if (! PyArg_ParseTuple(args, "OOk|O", &pName, &pUnit, &iLength, &pArg))
  //  return -1;

  pAttributeList = PyDict_New();
  if(pAttributeList)
  {
    pTmp = self->pAttributeList;
    Py_INCREF(pAttributeList);
    self->pAttributeList = pAttributeList;
    Py_XDECREF(pTmp);
  }
  else
  {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate Object");
    return -1;
  }

  pChannelList = PyList_New(0);
  if(pChannelList)
  {
    pTmp = self->pChannelList;
    Py_INCREF(pChannelList);
    self->pChannelList = pChannelList;
    Py_XDECREF(pTmp);
  }
  else
  {
    Py_XDECREF(self->pAttributeList);
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate Object");
    return -1;
  }

  pEventList = PyList_New(0);
  if(pEventList)
  {
    pTmp = self->pEventList;
    Py_INCREF(pEventList);
    self->pEventList = pEventList;
    Py_XDECREF(pTmp);
  }
  else
  {
    Py_XDECREF(self->pChannelList);
    Py_XDECREF(self->pAttributeList);
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate Object");
    return -1;
  }

  return 0;
}


PyObject* TMipsCore::New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  return type->tp_alloc(type, 0);
}

PyObject* TMipsCore::Alloc(PyTypeObject *self, Py_ssize_t nitems)
{
  TMipsCore* obj;

  try
  {  
    obj = new TMipsCore();
  }
  catch(TXAlloc)
  {
    Py_DECREF(obj);
    return PyErr_NoMemory();
  }

  return (PyObject*) obj;
}

void TMipsCore::Dealloc(TMipsCore* self)
{
  Py_TYPE(self)->tp_free(self);
}

void TMipsCore::Free(TMipsCore* self)
{
  delete self;
}

PyTypeObject TMipsCore::oType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mips",                 /*tp_name*/
    sizeof(TMipsCore),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)TMipsCore::Dealloc, /*tp_dealloc*/
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
    "MIPS Application object", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TMipsCore::methods,         /* tp_methods */
    TMipsCore::members,         /* tp_members */
    TMipsCore::getsets,         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TMipsCore::Init,      /* tp_init */
    (allocfunc)TMipsCore::Alloc,    /* tp_alloc */
    (newfunc)TMipsCore::New,        /* tp_new */
    (freefunc)TMipsCore::Free,      /* tp_free */
};


void* TMipsCore::ScriptThreadFunc(void* pvSelf)
{
  TMipsCore* self = (TMipsCore*)pvSelf;
  PyThreadState *_save;
  int iRet;

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PySys_WriteStdout("Script Thread started\n");
/*  PyEval_AcquireLock();
  myThreadState = PyThreadState_New(self->pMainThreadState->interp);
  self->pScriptThreadState = myThreadState;

  PyThreadState_Swap(myThreadState);

  PySys_WriteStdout("Script Thread started\n");

  iRet = self->ScriptFunc();

  PySys_WriteStdout("Script Thread stopped\n");

  PyThreadState_Swap(NULL);
//  PyThreadState_Clear(myThreadState);
//  PyThreadState_Delete(myThreadState);
  PyEval_ReleaseLock();
*/
  iRet = self->ScriptFunc();

  // _save = PyEval_SaveThread();

  // sleep(15);

  // PyEval_RestoreThread(_save);

  PySys_WriteStdout("Script Thread stopping\n");

  PyGILState_Release(gstate);

  pthread_exit((void*)iRet);
  // ExitThread(0);
}


int TMipsCore::ScriptFunc()
{
  PyThreadState* _save;
  TMipsCore* self = this;

//  HANDLE hTimer[2];
//  DWORD dwWaitResult;
  int i;
  int n;

  unsigned int uTimerPeriod = 1;

//  const double dScriptRate = 0.1;
//  const UINT   iScriptRate_ms = 100;
  double dScriptRate;
  unsigned int iScriptRate_ms;
  const double dUpdateRate = 0.1;
  unsigned int   iUpdateFactor;
  unsigned int   iUpdateCounter = 1;

  double dSampleRate;
  unsigned int   iSampleFactor;
  unsigned int   iSampleCounter = 1;
  PyObject* pThreadTickHandler = 0;
  PyObject* eventobj;

  double dTstern;

/*
  MMRESULT mmres;

  TIMECAPS tc;
  mmres = timeGetDevCaps(&tc, sizeof(tc));

  char tbuf[48];

  if(mmres == MMSYSERR_NOERROR)
  {
    sprintf(tbuf, "TR Soll: %d - Min: %d ms\n", uTimerPeriod, tc.wPeriodMin);
    PySys_WriteStdout(tbuf);
    if(uTimerPeriod < tc.wPeriodMin)
    {
      uTimerPeriod = tc.wPeriodMin;
    }
  }

  mmres = timeBeginPeriod(uTimerPeriod);
*/

  dScriptRate = this->dSampleRates[this->iScriptRate];
  this->dScriptRate = dScriptRate;

  iScriptRate_ms = (int)(dScriptRate * 1000.0);

  iUpdateFactor = (unsigned int) (dUpdateRate / dScriptRate);

  dSampleRate = dScriptRate;
  iSampleFactor = 1;
  iSampleCounter = 1;

  self->eThreadSignal &= ~Start_Script;
  self->eScriptState = Script_Running;

  if(self->pStartHandler)
  {
    if(PyCallable_Check(self->pStartHandler)) 
    {
      PyObject_CallObject(self->pStartHandler, NULL);
    }
  }

  if(self->pTickHandler)
  {
    if(PyCallable_Check(self->pTickHandler)) 
    {
      Py_INCREF(self->pTickHandler);
      pThreadTickHandler = self->pTickHandler;
    }
  }

/*
#ifndef UNDER_CE
  hTimer[0] = CreateWaitableTimer(NULL, FALSE, TEXT("MainAppTimer"));
  if(hTimer[0] == NULL)
  {
    MessageBox(0, TEXT("CreateWaitableTimer failed"), TEXT("TMipsCore::ThreadFunc"), MB_OK);
    ExitThread(0);
    return 0;
  }

  LARGE_INTEGER lDueTime;
  lDueTime.LowPart = -1000;
  lDueTime.HighPart = 0;
  SetWaitableTimer(hTimer[0], &lDueTime, iScriptRate_ms, NULL, NULL, FALSE);

#else
  MMRESULT mTimerResult;
  hTimer[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
  mTimerResult = timeSetEvent(iScriptRate_ms, uTimerPeriod, (LPTIMECALLBACK)hTimer[0], 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
  if(mTimerResult == NULL)
  {
    MessageBox(0, TEXT("timeSetEvent failed"), TEXT("TMipsCore::ThreadFunc"), MB_OK);
    ExitThread(0);
    return 0;
  }
#endif

  hTimer[1] = self->hMainThreadEvent;
*/


  //pthread_mutex_lock(&self->mutex);

  self->fdThreadTimer = timerfd_create(CLOCK_MONOTONIC, 0);
  if(self->fdThreadTimer == -1)
  {
    PySys_WriteStdout("Timer error\n");
    return -1;
  }

  unsigned int ns;
  unsigned int sec;
  struct itimerspec itval;
  unsigned int period;

  period = iScriptRate_ms * 1000; // ms

  sec = period/1000000;
  ns = (period - (sec * 1000000)) * 1000;
  itval.it_interval.tv_sec = sec;
  itval.it_interval.tv_nsec = ns;
  itval.it_value.tv_sec = sec;
  itval.it_value.tv_nsec = ns;
  timerfd_settime(this->fdThreadTimer, 0, &itval, NULL);

  uint64_t missed;

  while(1)
  {
    _save = PyEval_SaveThread();

    if(read(this->fdThreadTimer, &missed, sizeof (missed)) == -1)
    {
      close(this->fdThreadTimer);

      PyEval_RestoreThread(_save);
      PySys_WriteStdout("Timer failed\n");

      return -1;
    }
    // WaitForSingleObject(hTimer[0], INFINITE);

    PyEval_RestoreThread(_save);

    if(self->eThreadSignal & Stop_Script) break;

    if(self->pChannelList)
    {
      TChannel* chn;
      PyObject* chnobj;
      PyObject* result;

      n = PyList_Size((PyObject*)self->pChannelList);

      for (i = 0; i < n; i++)
      {
        chnobj = PyList_GetItem((PyObject*)self->pChannelList, i);

        if (!PyObject_TypeCheck(chnobj, &TChannel::oType)) continue;
        chn = (TChannel*)chnobj;

        if(PyCallable_Check(chn->function)) 
        {
          result = PyObject_CallObject(chn->function, NULL);
          chn->value = PyFloat_AsDouble(result);
          if(chn->value > chn->max) chn->max = chn->value;
          if(chn->value < chn->min) chn->min = chn->value;
        }

      }

      if(--iUpdateCounter == 0)
      {

        // _save = PyEval_SaveThread();
        // self->parent->SendSignal();
        // PyEval_RestoreThread(_save);

        iUpdateCounter = iUpdateFactor;
      }
    }

    if(pThreadTickHandler)
    {
      
      PyObject_CallObject(pThreadTickHandler, NULL);
    }

    if(self->eThreadSignal & Start_Recording)
    {
      if(self->iSampleRate < self->iScriptRate) self->iSampleRate = self->iScriptRate;
      dSampleRate = self->dSampleRates[self->iSampleRate];
      self->dSampleRate = dSampleRate;
      iSampleFactor = (unsigned int) (dSampleRate / dScriptRate);
      iSampleCounter = 1;

      self->eThreadSignal &= ~Start_Recording;
      self->eRecordingState = Recording_Running;
      if(self->pRecordingStartHandler)
      {
        if(PyCallable_Check(self->pRecordingStartHandler)) 
        {
          PyObject_CallObject(self->pRecordingStartHandler, NULL);
        }
      }
    }

    if(self->eThreadSignal & Stop_Recording || (self->uRecordCount >= self->uRecordLength && self->eRecordingState == Recording_Running))
    {
      self->eThreadSignal &= ~Stop_Recording;
      self->eRecordingState = Recording_Stopped;
      if(self->pRecordingStopHandler)
      {
        if(PyCallable_Check(self->pRecordingStopHandler)) 
        {
          PyObject_CallObject(self->pRecordingStopHandler, NULL);
        }
      }              
    }
/*
    n = PyList_Size((PyObject*)self->pEventList);

    for (i = 0; i < n; i++)
    {
      eventobj = PyList_GetItem((PyObject*)self->pEventList, 0);
      if(self->eRecordingState == Recording_Running)
      {
        PyList_Append((PyObject*)self->pRecordSet->pEventList, eventobj);
      }
      PySequence_DelItem((PyObject*)self->pEventList, 0);
    }
*/
    if(self->eRecordingState == Recording_Running)
    {
      if(--iSampleCounter == 0)
      {
        TRecord* rec;
        TChannel* chn;
        PyObject* recobj;
        double dValue;

        n = PyList_Size((PyObject*)self->pRecordSet->pRecordList);

        for (i = 0; i < n; i++)
        {
          recobj = PyList_GetItem((PyObject*)self->pRecordSet->pRecordList, i);
          rec = (TRecord*)recobj;
          chn = (TChannel*)rec->pSource;
          rec->_Set(rec->iLength, chn->GetValue());
          rec->iLength++;
        }
        self->uRecordCount++;
        self->dRecordTime += dSampleRate;
        iSampleCounter = iSampleFactor;
      }
    }
    self->dScriptTime += dScriptRate;
  }

  if(self->eRecordingState == Recording_Running)
  {
    self->eThreadSignal &= ~Stop_Recording;
    self->eRecordingState = Recording_Stopped;
    if(self->pRecordingStopHandler && PyCallable_Check(self->pRecordingStopHandler)) 
    {
      PyObject_CallObject(self->pRecordingStopHandler, NULL);
    }
  }

  if(self->pStopHandler)
  {
    if(PyCallable_Check(self->pStopHandler)) 
    {
      PyObject_CallObject(self->pStopHandler, NULL);
    }
  }

  PySys_WriteStdout("Main Thread stopped\n");

  if(pThreadTickHandler)
  {
    Py_DECREF(pThreadTickHandler);
  }

  self->eScriptState = Script_Stopped;
  self->eThreadSignal = None;

/*
#ifndef UNDER_CE
  CancelWaitableTimer(hTimer[0]);
  CloseHandle(hTimer[0]);
#else
  timeKillEvent(mTimerResult);
  CloseHandle(hTimer[0]);
#endif

  mmres = timeEndPeriod(uTimerPeriod);
*/
  close(self->fdThreadTimer);
  return 0;
  // ExitThread(0);
}




