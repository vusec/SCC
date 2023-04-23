#include "scc/driver/DriverUtils.h"
#include "gtest/gtest.h"

#include <cstdlib>

TEST(DriverUtils, prependPythonPathWithEmptyArg) {
  const char *PYTHONPATH = DriverUtils::pythonPathVar.c_str();
  const bool overwrite = true;

  ::unsetenv(PYTHONPATH);
  EXPECT_EQ(DriverUtils::prependPythonPath("/buildpath/bin/scc"),
            "/buildpath/runtime/python");

  ::setenv(PYTHONPATH, "/usr/lib/python", overwrite);
  EXPECT_EQ(DriverUtils::prependPythonPath("/buildpath/bin/scc"),
            "/buildpath/runtime/python:/usr/lib/python");
}
