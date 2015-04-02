#ifndef MEAS_TYPE_H
#define	MEAS_TYPE_H

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <iomanip>
#include <vector>
#include <time.h>

#include "io_proxy.hpp"
#include "meas_buffer.hpp"
#include "meas_type_storage.hpp"

using namespace std;

class MeasType {
public:
  MeasType();
  unsigned int fetch();
  unsigned int addRaw(unsigned int);
  double rawToValue(unsigned int);
  double valueAt(unsigned long int i);
  unsigned int lastRaw();
  double lastValue();
  double lastValueFor(unsigned int);
  string toJson();
  void logInfo(string log); // overwritten color
  
  unsigned long int minTimeDiffToStore;
  unsigned long int maxTimeDiffToStore;
  double valueDiffToStore;
  
  void prepareFetch();
  string fetchString(unsigned int raw);

  MeasBuffer *buffer;
  
  string name; // name of measurements
  unsigned char priority; // 0 - low priority, 1 - high (at this moment), used only in frontend
  char command; // at this moment only 1 byte commands
  char responseSize; // amount of bytes returned from uC, normally 1 or 2
  
  double coefficientLinear;
  int coefficientOffset;
  
  bool started;
  string logPrefix;
  
  IoProxy *ioProxy;
  MeasTypeStorage *measTypeStorage;

  //mutex storageMutex;  
  unsigned long int timeToIndex(unsigned long long t);
  vector < StorageHash > prepareStorage(unsigned long long timeFrom, unsigned long long timeTo);
  vector < StorageHash > storageArray(unsigned long long timeFrom, unsigned long long timeTo);
  string storageJson(unsigned long long timeFrom, unsigned long long timeTo);

	
};

#include "meas_type.cpp"
#endif	
