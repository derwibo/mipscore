// exception.h

#ifndef EXCEPTION_H
#define EXCEPTION_H

class TXBase
{
  private:
    long lErrorCode;
  public:
    TXBase() { lErrorCode = -1; }
    TXBase(long errorcode) { lErrorCode = errorcode; }
    ~TXBase() { }

    long ErrorCode() { return lErrorCode; }
};

class TXAlloc : public TXBase
{
  public:
    TXAlloc() : TXBase(-2) {  }
};

#endif // EXCEPTION_H