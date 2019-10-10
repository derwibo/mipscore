// list.h

#ifndef LIST_H
#define LIST_H

#include "exception.h"
#include "comparator.h"

class CXList : public TXBase
{
};

template <class T>
class CListIterator;

template <class T>
class CLink
{
  public:
    CLink<T>* next;
    T obj;

    CLink(T& a) : obj(a) { }
};

template <class T>
class CList
{
  private:
    CLink<T>* last;
    int nitems;

  public:
    CList();
    ~CList();

    int Insert(T a);
    int Insert(T a, int position);
    int Append(T a);

    void Remove(T a);
    void Remove(CComparator<T>* c);

    T Find(CComparator<T>* c);
    bool Find(T* pt, CComparator<T>* c);

    int Index(CComparator<T>* c);

    T First() { return last ? last->next : 0; }
    T Last() { return last; }

    T Pop();

    friend class CListIterator<T>;      
};


template <class T>
class CListIterator
{
  private:
    CList<T>* list;
    CLink<T>* current;

  public:
    CListIterator(CList<T>* list);
    ~CListIterator();
    T Next();
    bool Next(T* pt);
    void Reset();
};



template <class T>
CList<T>::CList()
{
  last = 0;
  nitems = 0;
}

template <class T>
CList<T>::~CList()
{
  CLink<T>* link;
  if(last)
  {
    do
    {
      link = last->next;
      last->next = link->next;
      delete link;
    }
    while(link != last);
  }
}

template <class T>
int CList<T>::Insert(T a)
{
  CLink<T>* link;
  link = new CLink<T>(a);
  if(link == 0) throw CXList();
  if(last)
  {
    link->next = last->next;
    last->next = link;  
  }
  else
  {
    link->next = link;
    last = link;
  }
  nitems++;
  return 1;
}

template <class T>
int CList<T>::Insert(T a, int position)
{
  if(position <= 0)
  {
    return Append(a);
  }
  if(position == 1)
  {
    return Insert(a);
  }
  CLink<T>* link;
  CLink<T>* tlink;

  if(last)
  {
    tlink = last;
    while(position--)
    {      
      tlink = tlink->next;
      if(tlink == last)
      {
        return Append(a);
      }
    }
    link = new CLink<T>(a);
    if(link == 0) throw CXList();
    link->next = tlink->next;
    tlink->next = link;
    nitems++;
    return position;  
  }
  else
  {
    return Append(a);
  }
}

template <class T>
int CList<T>::Append(T a)
{
  CLink<T>* link;
  link = new CLink<T>(a);
  if(link == 0) throw CXList();
  if(last)
  {
    link->next = last->next;
    last->next = link;
    last = link;
  }
  else
  {
    link->next = link;
    last = link;
  }
  nitems++;
  return nitems;
}


template <class T>
void CList<T>::Remove(T a)
{
  CValueComparator<T> c(a);
  Remove(&c);
}

template <class T>
void CList<T>::Remove(CComparator<T>* c)
{
  CLink<T>* link;
  CLink<T>* prevlink;
  if(last)
  {
    link = last->next;
    prevlink = last;
    do
    {
      if(c->Match(link->obj))
      {
        if(link == prevlink)
        {
          last = 0;
        }
        else
        {
          if(link == last)
          { 
            last = prevlink;
          }
          prevlink->next = link->next;
        }
        delete link;
        nitems--;
        return;
      }
      prevlink = link;
      link = link->next;
    }
    while(link != last);
  }
}

template <class T>
T CList<T>::Find(CComparator<T>* c)
{
  CLink<T>* link;
  if(last)
  {
//    link = last->next;
    link = last;
    do
    {
      link = link->next;
      if(c->Match(link->obj)) return link->obj;
//      link = link->next;
    }
    while(link != last);
  }
  return 0;
}

template <class T>
bool CList<T>::Find(T* pt, CComparator<T>* c)
{
  CLink<T>* link;
  if(last)
  {
//    link = last->next;
    link = last;
    do
    {
      link = link->next;
      if(c->Match(link->obj))
      {
        *pt = link->obj;
        return true;
      }
//      link = link->next;
    }
    while(link != last);
  }
  return false;
}

template <class T>
int CList<T>::Index(CComparator<T>* c)
{
  int index = 0;
  CLink<T>* link;
  if(last)
  {
    link = last;
    do
    {
      link = link->next;
      index++;
      if(c->Match(link->obj)) return index;
    }
    while(link != last);
  }
  return 0;
}

template <class T>
T CList<T>::Pop()
{
  CLink<T>* link;
  T obj;
  link = last;
  if(last)
  {
    if(last->next == last) last = 0;
    else last = last->next;
    obj = link->obj;
    delete link;
    return obj;
  }
  return 0;
}

template <class T>
CListIterator<T>::CListIterator(CList<T>* l)
{
  list = l;
  if(list == 0) throw CXList();
  current = list->last;
}

template <class T>
CListIterator<T>::~CListIterator()
{

}

template <class T>
T CListIterator<T>::Next()
{
  CLink<T>* link;
  link = current ? (current = current->next) : 0;
  if(current == list->last) current = 0;
  return link ? link->obj : 0;
}

template <class T>
bool CListIterator<T>::Next(T* pt)
{
  CLink<T>* link;
  link = current ? (current = current->next) : 0;
  if(current == list->last) current = 0;
  if(link)
  {
    *pt = link->obj;
    return true;
  }
  return false;
}

template <class T>
void CListIterator<T>::Reset()
{
  current = list->last;
}


#endif // LIST_H
