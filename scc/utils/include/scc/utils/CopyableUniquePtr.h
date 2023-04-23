#ifndef COPYABLEUNIQUEPTR_H
#define COPYABLEUNIQUEPTR_H

#include "scc/utils/SCCAssert.h"

#include <memory>

/// A unique_ptr that does a deep-copy.
template <class Data> struct CopyableUniquePtr {
  // TODO: Rename to ValuePtr or so.
  std::unique_ptr<Data> data;
  CopyableUniquePtr() = default;
  CopyableUniquePtr(const CopyableUniquePtr<Data> &e) { *this = e; }
  CopyableUniquePtr(CopyableUniquePtr<Data> &&e) { std::swap(data, e.data); }
  CopyableUniquePtr &operator=(const CopyableUniquePtr<Data> &e) {
    // Clear whatever data we store in any case.
    data.reset();
    // If the other pointer has data then clone it.
    if (e.data)
      data = e.data->clone();
    return *this;
  }

  const Data *operator->() const {
    SCCAssert(data.get(), "Deref'ing invalid ptr?");
    return data.get();
  }

  Data *operator->() {
    SCCAssert(data.get(), "Deref'ing invalid ptr?");
    return data.get();
  }

  const Data &operator*() const {
    SCCAssert(data.get(), "Deref'ing invalid ptr?");
    return *data;
  }

  Data &operator*() {
    SCCAssert(data.get(), "Deref'ing invalid ptr?");
    return *data;
  }
};

#endif // COPYABLEUNIQUEPTR_H
