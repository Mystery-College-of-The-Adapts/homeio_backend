#ifndef ADDONS_ARRAY
#define	ADDONS_ARRAY

#include <vector>
#include <mutex>

#include "helper.hpp"
#include "./addons/abstract_addon.hpp"
#include "meas_type_array.hpp"
#include "log/log_array.hpp"

class AddonsArray {
public:
  AddonsArray();
  void start();
  void stop();
  void add(AbstractAddon *a);

  std::vector <std::unique_ptr<AbstractAddon>> addons;
  unsigned long int cycleInterval;
  bool isRunning;
  std::mutex shutdownMutex;

  MeasTypeArray* measTypeArray;
  LogArray *logArray;
};

//#include "addons_array.cpp"
#endif
