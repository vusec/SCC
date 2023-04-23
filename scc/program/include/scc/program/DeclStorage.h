#pragma once

#include <memory>
#include <unordered_map>

#include "scc/program/IdentTable.h"
#include "scc/program/NamedDecl.h"

struct DeclStorage {
  DeclStorage() = default;
  DeclStorage(const DeclStorage &o) { *this = o; }

  DeclStorage &operator=(const DeclStorage &o);

  std::vector<Decl *> asVec() const {
    std::vector<Decl *> res;
    for (const auto &up : decls)
      res.push_back(up.get());
    return res;
  }

  size_t size() const { return decls.size(); }

  NamedDecl &store(NamedDecl *f, size_t pos = 0) {
    decls.emplace(decls.begin() + pos, f);
    namedDecls[f->getNameID()] = f;
    return *f;
  }

  void remove(Decl *d) {
    for (auto iter = namedDecls.begin(); iter != namedDecls.end(); ++iter)
      if (iter->second == d) {
        namedDecls.erase(iter);
        break;
      }
    for (auto iter = decls.begin(); iter != decls.end(); ++iter)
      if (iter->get() == d) {
        decls.erase(iter);
        return;
      }
    assert(false && "Couldn't find decl to remove?");
  }

  NamedDecl *find(IdentTable::NameID name) const {
    auto iter = namedDecls.find(name);
    if (iter == namedDecls.end())
      return nullptr;
    return iter->second;
  }

  OptError print(PrintState &state) const;

private:
  std::vector<const Decl *> getFilteredDecls(Decl::Kind kind) const;

  std::unordered_map<IdentTable::NameID, NamedDecl *> namedDecls;
  std::vector<std::unique_ptr<Decl>> decls;
};
