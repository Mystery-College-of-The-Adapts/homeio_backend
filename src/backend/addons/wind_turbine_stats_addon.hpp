#ifndef WIND_TURBINE_STATS_ADDON_HPP
#define	WIND_TURBINE_STATS_ADDON_HPP

#include <string>
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <memory>

#include "abstract_addon.hpp"
#include "wind_turbine_stat.hpp"
#include "../utils/helper.hpp"
#include "../meas/meas_definitions.hpp"

class WindTurbineStatsAddon : public AbstractAddon {
 public:
  WindTurbineStatsAddon();

  void setup();
  void perform();
  void render();
  std::string toJson();

  std::string measNameU;
  std::string measNameI;
  std::string measCoil;
  std::string measResistor;

  std::shared_ptr<MeasType> u;
  std::shared_ptr<MeasType> i;
  std::shared_ptr<MeasType> coil;
  std::shared_ptr<MeasType> resistor;

  unsigned char bufferMax;
  std::vector<std::shared_ptr<WindTurbineStat>> bufferStat;
  void addToBuffer(std::shared_ptr<WindTurbineStat> wts);

  double coilThresholdVoltage;
  double batteryThresholdCurrent;

  static constexpr unsigned long long hour = 60*60*1000;

  std::string path;
  unsigned long long lastTime;

  unsigned long long calculateTimeFrom();
  std::shared_ptr<WindTurbineStat> calculateStats(unsigned long long t);
  void store(std::shared_ptr<WindTurbineStat> s);
  void repopulateFromBuffer();
};

#endif
