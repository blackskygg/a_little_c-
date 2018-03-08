#pragma once

#include <cstring>
#include <algorithm>

template <class T>
class my_vector {
public:
  explicit my_vector(T *buf, unsigned long size):
    _buf(buf && size ? new T[size] : nullptr), _size(_buf ? size : 0)
  {
    std::memcpy(_buf, buf, size);
  }

  my_vector(const my_vector &v):
    _buf(v._buf ? new T[v._size] : nullptr), _size(v._size)
  {
    std::memcpy(_buf, v._buf, v._size);
  }

  my_vector(my_vector &&v):
    _buf(v._buf), _size(v._size)
  {
    v._buf = nullptr;
    v._size = 0;
  }

  friend void swap(my_vector &a, my_vector &b)
  {
    using std::swap;

    swap(a._buf, b._buf);
    swap(a._size, b._size);
  }

  // Below we are defining a "pass-by-const-reference" and a
  // "pass-by-rvalue-reference" assignment.
  // If we just use the pass-by-value + swap idiom, we will lose an optimization
  // opportunity when we can reuse our current space:
  // Let's think of the situation when the statement "my_vector = lvalue" is,
  // executed, if we use pass-by-value, we will first deep-copy (copy-construct)
  // the lvalue, and then we swap *this and the deep-copy, causing one allocation
  // and one destruction. But if this->_size >= rhs._size, we actully don't need,
  // the extra allocation and destruction. This optimization also happens when we
  // do self-assignment, where the pass-by-value version will cause one unnecessary
  // allocation and destruction.
  my_vector& operator=(const my_vector &rhs)
  {
    // optimization 1
    if (&rhs == this)
      return *this;

    if (rhs._size <= _size) {
      // optimization 2
      memcpy(_buf, rhs._buf, rhs._size);
      _size = rhs._size;
    } else {
      // copy and swap
      auto tmp(rhs);
      swap(tmp, *this);
    }

    return *this;
  }

  my_vector& operator=(my_vector &&rhs)
  {
    // copy and swap
    swap(*this, rhs);

    return *this;
  }

  ~my_vector()
  {
    if (_buf)
      delete[] _buf;
  }

private:
  T *_buf;
  unsigned long _size;
};
