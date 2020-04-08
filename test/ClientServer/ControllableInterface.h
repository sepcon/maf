#pragma once

#include <maf/export/MafExport_global.h>

namespace maf {
namespace test {

struct ControllableIF;
typedef ControllableIF *(*GetControllableObjectFunc)();
struct ControllableDetails {
  int apiVersion;
  const char *fileName;
  const char *className;
  const char *objectName;
  const char *objectVersion;
  GetControllableObjectFunc initializeFunc;
};

struct ControllableIF {
  virtual void init() = 0;
  virtual void deinit() = 0;
  virtual void start() = 0;
  virtual void pause() = 0;
  virtual void stop() = 0;
};

struct ControllableDefault : ControllableIF {
  void init() override {}
  void deinit() override {}
  void start() override {}
  void pause() override {}
  void stop() override {}
};

#define MAF_PLUGIN_API_VERSION 1

#define MAF_STANDARD_PLUGIN_STUFF MAF_PLUGIN_API_VERSION, __FILE__

#define MAF_PLUGIN(classType, objectName, objectVersion)                       \
  extern "C" {                                                                 \
  MAF_EXPORT maf::test::ControllableIF *getControllableObject() {              \
    static classType singleton;                                                \
    return &singleton;                                                         \
  }                                                                            \
  MAF_EXPORT maf::test::ControllableDetails exports = {                        \
      MAF_STANDARD_PLUGIN_STUFF, #classType, objectName, objectVersion,        \
      getControllableObject,                                                   \
  };                                                                           \
  }

} // namespace test
} // namespace maf
