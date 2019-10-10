/* mipscore.h
 *
 */

#ifndef MIPSCORE_H
#define MIPSCORE_H

#include <Python.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "recordset.h"
#include "record.h"
#include "sensor.h"
#include "channel.h"
#include "mipsrecfile.h"

class TMipsCore
{
  public:
    PyObject_HEAD

  private:

    static void* ScriptThreadFunc(void* pvSelf);
    int ScriptFunc();

    pthread_t hMainThread;
    int fdThreadTimer;

//    HANDLE hMainThreadEvent;

    enum TThreadSignal { None = 0, Start_Script = 1, Stop_Script = 2, Start_Recording = 4, Stop_Recording = 8 };
    unsigned int eThreadSignal;

    PyThreadState* pMainThreadState;
    PyThreadState* pScriptThreadState;

  public:

    enum TScriptState { Script_Off , Script_Running , Script_Stopped };
    enum TRecordingState { Recording_Off , Recording_Running , Recording_Stopped };

    TScriptState eScriptState;
    TRecordingState eRecordingState;

    PyObject* pScriptDir;
    PyObject* pRecordDir;

    static double dSampleRates[];
    unsigned int  iScriptRate;
    double        dScriptRate;
    double        dScriptTime;
    unsigned int  iSampleRate;
    double        dSampleRate;
    double        dRecordTime;

    PyObject* pAttributeList;
    PyObject* pChannelList;
    PyObject* pEventList;
    TRecordSet* pRecordSet;

    PyObject* pStartHandler;
    PyObject* pStopHandler;
    PyObject* pTickHandler;
    PyObject* pRecordingStartHandler;
    PyObject* pRecordingStopHandler;

    unsigned long uRecordLength;
    unsigned long uRecordCount;

  private:
    TMipsCore();
    ~TMipsCore();

  // Class Internal Functions
  private:


    int _StartRecord();

    int _SaveRecord(const char* filename, int filetype);

  // Application Interface Functions

  public:

    int _LoadProgram(char* filename);
    int _RunProgram();
    int _StopProgram();
    int Reset();
    int StartRecord();
    int StopRecord();

    int SaveRecord(char* filename, int filetype = 0);

    int SetScriptRate(unsigned int iScriptRate);
    int SetSampleRate(unsigned int iSampleRate);
    int SetRecordingParams(unsigned long length, unsigned int samplerate);

    TRecordingState GetRecordingStatus() { return eRecordingState; }
    int GetRecordingName(char* buf, int buflen);

    double ScriptTime() { return dScriptTime; }
    double Time() { return dRecordTime; }


  // Python Interface Functions

  public:
    static PyMemberDef members[];
    static PyMethodDef methods[];
    static PyGetSetDef getsets[];

    static PyObject* PyStart(PyObject* self, PyObject *args);
    static PyObject* PyStop(PyObject* self, PyObject *args);
    static PyObject* PyStartRecord(PyObject* self, PyObject *args);
    static PyObject* PyStopRecord(PyObject* self, PyObject *args);
    static PyObject* PySaveRecord(PyObject* self, PyObject *args);
    static PyObject* PyInputBox(PyObject* self, PyObject *args);
    static PyObject* PySetInput(PyObject* self, PyObject *args);
    static PyObject* PyAddEvent(PyObject* self, PyObject *args);

    static PyObject* PyGetScriptDir(TMipsCore* self, void*);
    static int PySetScriptDir(TMipsCore* self, PyObject* arg, void*);

    static PyObject* PyGetRecordDir(TMipsCore* self, void*);
    static int PySetRecordDir(TMipsCore* self, PyObject* arg, void*);

    static PyObject* PyGetScriptRate(TMipsCore* self, void*);
    static int PySetScriptRate(TMipsCore* self, PyObject* arg, void*);

    static PyObject* PyGetSampleRate(TMipsCore* self, void*);
    static int PySetSampleRate(TMipsCore* self, PyObject* arg, void*);

    static int Init(TMipsCore *self, PyObject *args, PyObject *kwds);
    static PyObject* New(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static PyObject* Alloc(PyTypeObject *self, Py_ssize_t nitems);
    static void Dealloc(TMipsCore* self);
    static void Free(TMipsCore* self);

    static PyTypeObject oType;

};

#endif // MIPSCORE_H

