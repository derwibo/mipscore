// mipsrecfile.h

#ifndef MIPSRECFILE_H
#define MIPSRECFILE_H

#include "Python.h"

class TMipsRecFile
{
  public:

    static PyObject* PyRead(PyObject* self, PyObject* pArg);
    static PyObject* PyWrite(PyObject* self, PyObject* pArg);

  private:
    struct MipsRecInfo
    {
      int ChannelCount;
      int MaxRecordLength;
      double SampleRate;
    };

    struct MipsRecChnInfo
    {
      char Name[32];
      char Unit[16];
      int Length;
    };

    static int GetGeneralParameters(char* buf, MipsRecInfo* recinfo);
    static int GetChannelInfo(char* buf, MipsRecChnInfo* chninfo);
    static int GetIntParam(char* buf,int buflen, char* paramname, int* param, int defval);
    static int GetDoubleParam(char* buf,int buflen, char* paramname, double* param, double defval);
    static int GetStringParam(char* buf,int buflen, char* paramname, char* param, int pbuflen, char* defval);

};

#endif // MIPSRECFILE_H
