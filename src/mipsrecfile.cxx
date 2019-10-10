// mipsrecfile.cxx

#include <Python.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mipsrecfile.h"
#include "recordset.h"
#include "record.h"

int TMipsRecFile::GetIntParam(char* buf,int buflen, char* paramname, int* param, int defval)
{
  char mbuf[32];
  buflen = buflen - strlen(paramname);
  *param = defval;
  while(buflen > 0)
  {
    if(strncmp(buf, paramname, strlen(paramname)) == 0)
    {
//          strncpy(mbuf, buf, 16);
//          mbuf[31] = '\0';
//          MessageBox(0, mbuf, "Found Param", MB_OK);

      while(*buf++ != '"');
//      while(! isdigit(*buf)) buf++;
      *param = atoi(buf);
//      sprintf(mbuf+16, " = %d", *param);
//      MessageBox(0, mbuf, "Found Param", MB_OK);

      break;
    }
    if(*buf == '>') break;
    buf++;
    buflen--;
  }
  return 0;
}

int TMipsRecFile::GetDoubleParam(char* buf,int buflen, char* paramname, double* param, double defval)
{
  char mbuf[32];
  buflen = buflen - strlen(paramname);
  *param = defval;
  while(buflen > 0)
  {
    if(strncmp(buf, paramname, strlen(paramname)) == 0)
    {
//          strncpy(mbuf, buf, 16);
//          mbuf[31] = '\0';
//          MessageBox(0, mbuf, "Found Param", MB_OK);

      while(*buf++ != '"');
//      while(! isdigit(*buf)) buf++;
      *param = atof(buf);
//      sprintf(mbuf+16, " = %f", *param);
//      MessageBox(0, mbuf, "Found Param", MB_OK);

      break;
    }
    if(*buf == '>') break;
    buf++;
    buflen--;
  }
  return 0;
}


int TMipsRecFile::GetStringParam(char* buf,int buflen, char* paramname, char* param, int pbuflen, char* defval)
{
  char mbuf[32];
  buflen = buflen - strlen(paramname);
  strncpy(param, defval, pbuflen);

  while(buflen > 0)
  {
    if(strncmp(buf, paramname, strlen(paramname)) == 0)
    {

//          strncpy(mbuf, buf, 32);
//          mbuf[31] = '\0';
//          MessageBox(0, mbuf, "Param", MB_OK);

      while(*buf++ != '"');
      while(pbuflen && (*buf != '"'))
      {
        *param = *buf;
        param++;
        buf++;
        pbuflen--;
      }
      *param = '\0';
      break;
    }
    if(*buf == '>') break;
    buf++;
    buflen--;
  }
  return 0;
}


int TMipsRecFile::GetChannelInfo(char* buf, MipsRecChnInfo* chninfo)
{
  GetStringParam(buf, 1024, "Name", chninfo->Name, 10, "Unbenannt");
  GetStringParam(buf, 1024, "Unit", chninfo->Unit, 4, "[-]");
  GetIntParam( buf, 1024, "Length", &chninfo->Length, 0);

  return 0;
}




PyObject* TMipsRecFile::PyRead(PyObject* self, PyObject* pArgs)
{
  TRecordSet* pRecordset = 0;
  TRecord* pRecord;
  PyObject* pFile;
  PyObject* pMode;
  PyObject* pData;
  PyObject* pName;
  PyObject* pUnit;
  PyObject* pObjArgs;
  int i, j;

  char buf[32];
  char paramnamebuf[32];
  double* dptr;

  if (!PyArg_ParseTuple(pArgs, "O", &pFile))
  {
    return NULL;   
  }
  else
  {
/*
    pMode = PyObject_GetAttrString(pFile, "mode")
    if(pMode == NULL)
    {
      if(PyErr_Occurred())
      {
        if(PyErr_ExceptionMatches(PyExc_AttributeError);
          PyErr_Clear();
        else
          return NULL;
      }
    }
    else
    {
      char* p1;
      p1 = PyString_AsString(pMode);
      if(p1[0] != 'b' && )
      {
        PyErr_SetString(PyExc_IOError, "File must be in binary mode");
        return NULL;
      }
    }
*/
    pData = PyObject_CallMethod(pFile, "read", "l", -1);
    if(pData == NULL)
    {
      return NULL;
    }
    else if(PyBytes_Check(pData))
    {
      char* data;
      char* ptr;
      MipsRecInfo  recinfo;
      MipsRecChnInfo chninfo[32];
      data = PyBytes_AsString(pData);

      pRecord = 0;

      recinfo.ChannelCount = 0;
      recinfo.MaxRecordLength = 0;
      recinfo.SampleRate = 0.0;

      ptr = data;

      if(strncmp(ptr, "MRF-0.1", 7) != 0)
      {
        Py_DECREF(pData);
        PyErr_SetString(PyExc_ValueError, "MRF Header missing");
        return NULL;
      }
      else
      {

        //pRecordset = new TRecordSet();
        pObjArgs   = Py_BuildValue("s", "");
        pRecordset = (TRecordSet*) PyObject_CallObject((PyObject*)&TRecordSet::oType, pObjArgs);
        Py_XDECREF(pObjArgs);

        while(1)
        {   
          while(*ptr++ != '<') ;
          if(strncmp(ptr, "Attributes", 10) == 0)
          {
            while(*ptr++ != '>') ;
            continue;
          }
          if(strncmp(ptr, "Attribute", 9) == 0)
          {
            GetStringParam(ptr, 1024, "Name", paramnamebuf, 32, "Unbenannt");
            if(strncmp(paramnamebuf, "SampleRate", 10) == 0)
            {
              GetDoubleParam(ptr, 1024, "Value", &recinfo.SampleRate, 0.1);
            }
            while(*ptr++ != '>') ;
            continue;
          }
          if(strncmp(ptr, "Channels", 8) == 0)
          {
            while(*ptr++ != '>') ;
            continue;
          }
          if(strncmp(ptr, "Channel", 7) == 0)
          {
            GetChannelInfo(ptr, &chninfo[recinfo.ChannelCount]);
            if(chninfo[recinfo.ChannelCount].Length > recinfo.MaxRecordLength)
              recinfo.MaxRecordLength = chninfo[recinfo.ChannelCount].Length;
            recinfo.ChannelCount += 1;
            while(*ptr++ != '>') ;
            continue;
          }
          if(strncmp(ptr, "Data", 4) == 0)
          {
            while(*ptr++ != '>') ;
            break;
          }
          while(*ptr++ != '>') ;
        }

        ptr += 2;
        if(recinfo.SampleRate > 0.0)
        {

          // pRecord = new TRecord(); //L"Zeit", L"s", recinfo.MaxRecordLength);
          // pRecord = new TRecord("Zeit", "s", recinfo.MaxRecordLength);
          
          pObjArgs = Py_BuildValue("ssi", "Zeit", "s", recinfo.MaxRecordLength);
          pRecord = (TRecord*) PyObject_CallObject((PyObject*)&TRecord::oType, pObjArgs);
          Py_XDECREF(pObjArgs);

          pRecord->SetLength(recinfo.MaxRecordLength);
          pRecord->GenerateSequence2(0.0, recinfo.SampleRate);
          pRecordset->AddChannel(pRecord, -1);
        }

        for(i=0 ; i<recinfo.ChannelCount ; i++)
        {
          pName = PyUnicode_DecodeUTF8(chninfo[i].Name, strlen(chninfo[i].Name), "ignore");
          pUnit = PyUnicode_DecodeUTF8(chninfo[i].Unit, strlen(chninfo[i].Unit), "ignore");
          pObjArgs = Py_BuildValue("OOi", pName, pUnit, chninfo[i].Length);
          pRecord = (TRecord*) PyObject_CallObject((PyObject*)&TRecord::oType, pObjArgs);
          Py_XDECREF(pObjArgs);
          Py_XDECREF(pName);
          Py_XDECREF(pUnit);

          if(pRecord == 0) break;
          pRecordset->AddChannel(pRecord, -1);
          dptr = (double*)ptr;

          for(j=0 ; j<chninfo[i].Length ; j++)
          {
            pRecord->_SetValue(j, *dptr);
            dptr++;
          }
          pRecord->SetLength(chninfo[i].Length);
          pRecord->UpdateStatVals();
          pRecord->SetRange(0.0, 100.0);
          ptr += chninfo[i].Length * sizeof(double);
        }
      }
    }
    Py_DECREF(pData);
  }

  return (PyObject*)pRecordset;
}


PyObject* TMipsRecFile::PyWrite(PyObject* self, PyObject* pArgs)
{
  PyObject* pRecordset;
  PyObject* pFile;
  PyObject* pRetVal;

  char* buf;
  int size;

  if (!PyArg_ParseTuple(pArgs, "OO", &pFile, &pRecordset))
  {
    return NULL;
  }
  Py_INCREF(pFile);
  Py_INCREF(pRecordset);

  if(!PyObject_HasAttrString(pFile, "write"))
  {
    Py_DECREF(pFile);
    Py_DECREF(pRecordset);
    PyErr_SetString(PyExc_TypeError, "File Object expected");
    return NULL;
  }

  if(!PyObject_TypeCheck(pRecordset, &TRecordSet::oType))
  {
    Py_DECREF(pFile);
    Py_DECREF(pRecordset);
    PyErr_SetString(PyExc_TypeError, "TRecordSet expected");
    return NULL;
  }



  int i;
  int n;

  char* pstr;

  PyObject* recordlist;
  PyObject* attributelist;
  PyObject* eventlist;

  PyObject* ptmp;
  PyObject* ptmp2;

  Py_ssize_t pos;
  PyObject* key;
  PyObject* value;
  PyObject* strvalue;

  TRecord* rec;
  PyObject* recobj;
  PyObject* eventobj;
  PyObject* eventstr;
  char*     u8eventstr;
  Py_ssize_t strsize;
  TEvent* event;

  char cTxtBuf[64];
  int  iTxtLen;

  recordlist =    ((TRecordSet*)pRecordset)->pRecordList;
  attributelist = ((TRecordSet*)pRecordset)->pAttributeList;
  eventlist =     ((TRecordSet*)pRecordset)->pEventList;

  pRetVal = PyObject_CallMethod(pFile, "write", "s", "MRF-0.1\r\n");

  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "<Attributes>\r\n", 14);

//  pos = 0;

  while (PyDict_Next(attributelist, &pos, &key, &value))
  {
    if (!PyUnicode_Check(key)) continue;
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "  <Attribute Name=\"", 19);
    pRetVal = PyObject_CallMethod(pFile, "write", "O", key);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\" Value=\"", 9);

    if (PyUnicode_Check(value))
    {
      pRetVal = PyObject_CallMethod(pFile, "write", "O", value);
    }
    else
    {
      strvalue = PyObject_Str(value); 
      if(strvalue)
      {
        pRetVal = PyObject_CallMethod(pFile, "write", "O", strvalue);
        Py_DECREF(strvalue);
      }
    }
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\">\r\n", 4);
  }

  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "</Attributes>\r\n", 15);
  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "<Channels>\r\n", 12);

  n = PyList_Size((PyObject*)recordlist);

  for (i=0 ; i<n ; i++)
  {
    recobj = PyList_GetItem(recordlist, i);
    rec = (TRecord*)recobj;

    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "  <Channel Name=\"", 17);
    ptmp = rec->GetName();
    ptmp2 = PyUnicode_AsUTF8String(ptmp);
    pRetVal = PyObject_CallMethod(pFile, "write", "O", ptmp2);
    Py_XDECREF(ptmp2);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\" Unit=\"", 8);
    ptmp = rec->GetUnit();
    ptmp2 = PyUnicode_AsUTF8String(ptmp);
    pRetVal = PyObject_CallMethod(pFile, "write", "O", ptmp2);
    Py_XDECREF(ptmp2);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\" RangeMin=\"", 12);
    iTxtLen = sprintf(cTxtBuf, "%f", rec->LoRange());
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", cTxtBuf, iTxtLen);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\" RangeMax=\"", 12);
    iTxtLen = sprintf(cTxtBuf, "%f", rec->HiRange());
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", cTxtBuf, iTxtLen);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\" Length=\"", 10);
    iTxtLen = sprintf(cTxtBuf, "%ld", rec->GetLength());
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", cTxtBuf, iTxtLen);
    pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\">\r\n", 4);
  }

  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "</Channels>\r\n", 13);
  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "<Data>\r\n", 8);

  for (i=0 ; i<n ; i++)
  {
    recobj = PyList_GetItem(recordlist, i);
    rec = (TRecord*)recobj;

    pRetVal = PyObject_CallMethod(pFile, "write", "s#", rec->pData, rec->GetLength() * sizeof(double));
  }

/*
  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\r\n</Data>\r\n<Events>\r\n", 21);

  n = PyList_Size((PyObject*)eventlist);

  for (i=0 ; i<n ; i++)
  {
    eventobj = PyList_GetItem(eventlist, i);
    event = (TEvent*)eventobj;

    pRetVal = PyObject_CallMethod(pFile, "write", "s#", &event->dTime, sizeof(double));
    eventstr = PyObject_Str(event->pEventInfo);
    if(eventstr)
    {
      u8eventstr = PyUnicode_AsUTF8AndSize(eventstr, &strsize);
      pRetVal = PyObject_CallMethod(pFile, "write", "s#y#", &strsize, sizeof(Py_ssize_t), u8eventstr, strsize);
      Py_DECREF(eventstr);
    }
    else
    {
      strsize = 0;
      pRetVal = PyObject_CallMethod(pFile, "write", "s#", &strsize, sizeof(int));
    }
  }

  pRetVal = PyObject_CallMethod(pFile, "write", "s#", "\r\n</Events>\r\n", 13);
*/
  Py_DECREF(pFile);
  Py_DECREF(pRecordset);

  Py_INCREF(Py_None);
  return Py_None;
}
