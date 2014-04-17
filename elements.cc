/**
 * @file elements.cc
 * Implementation of elements
 * @date April 2014
 * @author Vikram Jayashankar
 */

#include "elements.h"

Element::Element() {}
Element::~Element() {}

Element::Type E_String::type() { return Element::STRING; }

E_String::E_String(std::string t) : _str(t) {}

const char* E_String::c_str() { return _str.c_str(); }

int E_String::size() { return _str.size(); }
