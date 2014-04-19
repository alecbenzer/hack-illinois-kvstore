/**
 * @file elements.h
 * Header Superclass for different datatypes
 * @date April 2014
 * @author Vikram Jayashankar
 */

#ifndef _ELEMENTS_H
#define _ELEMENTS_H

#include <string>
#include <vector>
#include <iostream>
#include "mmap_allocator.h"

class Element {
 public:
  enum Type {
    STRING,
    VECTOR
  };
  Element();
  ~Element();
  virtual Type type() = 0;
};
/*
class E_Vector : public Element {
    public:
        E_Vector();
        ~E_Vector();
        Type type();
    private:
        std::vector<std::string> _vec;
};*/

class String : public Element {
 public:
  String(std::string t);
  Type type();
  const mm::string& str() const;

 private:
  mm::string str_;
};

#endif
