#ifndef ONSCOPEEXIT_H
#define ONSCOPEEXIT_H

#include <functional>

class OnScopeExit {
  typedef std::function<void()> Callback;
  Callback callback;

public:
  OnScopeExit(Callback c) : callback(c) {}
  ~OnScopeExit() { callback(); }
};

#endif // ONSCOPEEXIT_H
