#pragma once

#include <algorithm>
#include <iostream>

template<class T>
class my_shared_ptr {
public:
  explicit my_shared_ptr(T *ptr):
    _refcnt(ptr ? new int(1) : nullptr), _ptr(ptr)
  {
    std::cout << "explicit init" << std::endl;
  }

  my_shared_ptr(const my_shared_ptr &ptr):
    _refcnt(ptr._refcnt), _ptr(ptr._ptr)
  {
    std::cout << "copy construct" << std::endl;
    (*_refcnt)++;
  }

  my_shared_ptr(my_shared_ptr &&ptr):
    _refcnt(ptr._refcnt), _ptr(ptr._ptr)
  {
    std::cout << "move construct" << std::endl;
    ptr._refcnt = nullptr;
    ptr._ptr = nullptr;
  }

  // This is for the copy-and swap idiom
  friend void swap(my_shared_ptr &a, my_shared_ptr &b)
  {
    using std::swap;

    swap(a._refcnt, b._refcnt);
    swap(a._ptr, b._ptr);
  }

  // This will only cause overhead when doing self-assignment, causing one extra
  // incr and one extra decrease.
  // Otherwise it's quite fast due to copy elision:
  // When doing = rvalue: rvalue will be move-constructed into rhs directy.
  //                      no refcnt++ for rhs, but we have refcnt-- for *this;
  // When doing = lvalue: lvalue will be copy-constructed into rhs.
  //                      refcnt++ for rhs and refcnt-- for *this;
  my_shared_ptr& operator=(my_shared_ptr rhs)
  {
    swap(*this, rhs);
    return *this;
  }

  ~my_shared_ptr()
  {
    std::cout << "destruction" << std::endl;

    if (!_ptr)
      return;

    if (--*_refcnt == 0) {
      delete _ptr;
      delete _refcnt;
    }
  }

  T& operator*()
  {
    return *_ptr;
  }


private:
  int *_refcnt;
  T* _ptr;
};
