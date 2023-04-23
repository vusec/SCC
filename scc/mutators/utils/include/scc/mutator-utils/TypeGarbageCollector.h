#ifndef TYPEGARBAGECOLLECTOR_H
#define TYPEGARBAGECOLLECTOR_H

#include "scc/program/Program.h"

/// Removes all unused types from the program's type table.
class TypeGarbageCollector {
  Program &p;

public:
  TypeGarbageCollector(Program &p) : p(p) {}
  void run();
};

#endif // TYPEGARBAGECOLLECTOR_H
