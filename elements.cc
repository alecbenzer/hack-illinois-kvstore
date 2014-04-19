/**
 * @file elements.cc
 * Implementation of elements
 * @date April 2014
 * @author Vikram Jayashankar
 */

#include "elements.h"

Element::Element() {}
Element::~Element() {}

Element::Type String::type() { return Element::STRING; }

String::String(std::string t) : str_(t.c_str(), t.size()) {}

const mm::string& String::str() const { return str_; }
