// comparator.h

#ifndef COMPARATOR_H
#define COMPARATOR_H


template <class T>
class CComparator
{
  public:
    virtual bool Match(T a) { return false; }
    virtual int Compare(T a, T b) { return 0; }
};

template <class T>
class CValueComparator : public CComparator<T>
{
  private:
    T value;

  public:
    CValueComparator(T v) : value(v) {  }
    bool Match(T a) { return (a == value) ? true : false; }
};

template <class T>
class CIndexComparator : public CComparator<T>
{
  private:
    int index;

  public:
    CIndexComparator(int index) { this->index = (index < 1) ? 1 : index ; }
    bool Match(T a) { return (--index == 0) ? true : false; }
};

#endif // COMPARATOR_H
