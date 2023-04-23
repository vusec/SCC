#include "scc/mutator-utils/TypeGarbageCollector.h"
#include <iostream>

void TypeGarbageCollector::run() {
  bool madeProgress = true;
  while (madeProgress) {
    madeProgress = false;
    for (Type &t : p.getTypes()) {
      if (p.getBuiltin().isBuiltin(t.getRef()))
        continue;
      if (p.isTypeUsed(t.getRef()))
        continue;
      t = Type();
      p.verifySelf();
      madeProgress = true;
    }
  }
  p.verifySelf();
  p.getTypes().shrinkToFit();
  p.verifySelf();
}
