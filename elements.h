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

class E_String : public Element {
 public:
  E_String(std::string t);
  ~E_String();
  Type type();
  int size();
  const char* c_str();

 private:
  std::string _str;
};

#endif
