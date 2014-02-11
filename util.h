/* Copyright 2014 Matthieu Tourne */

#ifndef UTIL_H
#define UTIL_H

#include "photogram.h"

string basename(const std::string& pathname);
string remove_extension(const std::string& pathname);
unsigned long upper_power_of_two(unsigned long v);

#endif // !UTIL_H
