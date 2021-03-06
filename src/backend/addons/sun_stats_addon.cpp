#include "sun_stats_addon.hpp"

SunStatsAddon::SunStatsAddon() {
  name = "sun_stats";
  lastTime = 0;
  lastStoreTime = 0;

  path = "stats";

  bufferMax = 60;
  calcInterval = 300000; // every 5 minutes

  storeDailyFiles = false;
  storeEnabled = true;
}

void SunStatsAddon::setup() {
  name = "sun_stats";

  // create path at start, no wait
  mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_IWOTH);

  if (storeEnabled) {
    restore();
  }

  lightMeas = measTypeArray->byName(lightMeasName);
}

void SunStatsAddon::perform() {
  if (lastTime > ( Helper::mTime() - interval) ) {
    return;
  }

  meas_time initialTime = normalizeTime( lightMeas->buffer->earliestTime() );
  meas_time endTime = normalizeTime( Helper::mTime() ) + interval;
  meas_time t = initialTime;

  logArray->log("SunStatsAddon", "initialTime: " + std::to_string(initialTime));
  logArray->log("SunStatsAddon", "endTime:" + std::to_string(endTime));

  if (bufferStat.size() == 0) {
    // repopulate
    while (t < endTime) {
      logArray->log("SunStatsAddon", "repopulate: " + std::to_string(t));

      std::shared_ptr<SunStat> s = std::make_shared<SunStat>();
      s->time = t;
      updateStats(s);
      addToBuffer(s);

      t += interval;
    }

  } else {
    // take last stat from buffer and update
    std::shared_ptr<SunStat> s = bufferStat[ bufferStat.size() - 1];
    t = s->time;
    updateStats(s);

    while ( (t + interval) < endTime) {
      logArray->log("SunStatsAddon", "append fresh: " + std::to_string(t));

      std::shared_ptr<SunStat> s = std::make_shared<SunStat>();
      s->time = t;
      updateStats(s);
      addToBuffer(s);

      t += interval;
    }
  }

  // iterate and store
  if (storeEnabled) {
    for (unsigned int i = 0; i < bufferStat.size(); i++) {
      std::shared_ptr<SunStat> s = bufferStat[i];

      if (s->time > lastStoreTime) {
        store(s);
        lastStoreTime = s->time;
      }
    }
  }

  // store memory backup
  if (storeEnabled) {
    dump();
  }

  lastTime = Helper::mTime();
}

void SunStatsAddon::stop() {
  if (storeEnabled) {
    dump();
  }
}

std::string SunStatsAddon::backupFilename() {
  return path + "/addon_backup_" + name + ".txt";
}

std::string SunStatsAddon::storeFilename(std::shared_ptr<SunStat> s) {
  if (storeDailyFiles) {
    return path + "/" + name + "_" + Helper::currentDateSafe() + "_stats.csv";
  } else {
    return path + "/" + name + "_stats.csv";
  }
}


void SunStatsAddon::restore() {
  std::ifstream infile;
  infile.open(backupFilename(), std::ios_base::in);
  if (infile.good()) {
    int count;
    std::string line;

    getline(infile, line);
    count = std::atoi(line.c_str());
    getline(infile, line);
    lastStoreTime = std::stoull(line.c_str());

    logArray->log("SunStatsAddon", "load backup: " + std::to_string(count));

    getline(infile, line);
    getline(infile, line);
    getline(infile, line);

    for (int i = 0; i < count; i++) {
      std::shared_ptr<SunStat> s = std::make_shared<SunStat>();
      getline(infile, line);
      s->fromCsv(line);
      addToBuffer(s);
    }

  }
  infile.close();
}

void SunStatsAddon::dump() {
  std::ofstream outfile;
  outfile.open(backupFilename(), std::ios_base::out);
  outfile << bufferStat.size() << "\n";
  outfile << lastStoreTime << "\n";

  outfile << std::endl;
  outfile << std::endl;
  outfile << std::endl;

  for (unsigned int i = 0; i < bufferStat.size(); i++ ) {
    std::shared_ptr<SunStat> s = bufferStat[i];
    outfile << s->toCsv();
    outfile << std::endl;
  }

  outfile.close();
}


meas_time SunStatsAddon::normalizeTime(meas_time t) {
  return t - (t % interval);
}

void SunStatsAddon::updateStats(std::shared_ptr<SunStat> s) {
  s->timeLength = interval;
  s->sunIntegrated = 0;
  //s->sunriseTime = s->time;

  meas_buffer_index iFrom = lightMeas->timeToIndex(s->time);
  meas_buffer_index iTo = lightMeas->timeToIndex(s->time + interval);

  meas_buffer_index i = 0;
  meas_time lightInterval = lightMeas->buffer->calcInterval();

  double lightValue;
  double lightIntegrated;

  std::vector < unsigned int > raw = lightMeas->buffer->getFromBuffer(iFrom, iTo, 0);
  raw = lightMeas->buffer->filterVector(raw);

  for (i = 0; i < raw.size(); i++ ) {
    lightValue = lightMeas->avgValueAt(iTo - i, 10);
    if (lightValue > 2.0) {
      //logArray->log("SunStatsAddon", "found 2 at " + std::to_string(i));
      s->sunriseTime2 = s->time + (i * lightInterval);
      s->foundSunriseTime2 = true;
      break;
    }
  }

  for (i = 0; i < raw.size(); i++ ) {
    lightValue = lightMeas->avgValueAt(iTo - i, 10);
    if (lightValue > 8.0) {
      s->sunriseTime8 = s->time + (i * lightInterval);
      s->foundSunriseTime8 = true;
      break;
    }
  }

  for (i = 0; i < raw.size(); i++ ) {
    lightValue = lightMeas->avgValueAt(iTo - i, 10);
    if (lightValue > 40.0) {
      s->sunTime40 = s->time + (i * lightInterval);
      s->foundSunTime40 = true;
      break;
    }
  }

  for (i = 0; i < raw.size(); i++ ) {
    if (lightMeas->buffer->stored(iTo - i)) {
      // was SEGFAULT
      lightValue = lightMeas->valueAt(iTo - i);
      if (lightValue > 40.0) {
        s->sunInterval40 += lightInterval;
      }
      s->sunIntegrated += ( lightValue * (double)(lightInterval) / 1000000);
    }
  }

  // // index is reverse of time
  // for (i = iTo; i <= iFrom; i-- ) {
  //   lightValue = lightMeas->avgValueAt(i, 10);
  //   if (lightValue > 5.0) {
  //     logArray->log("SunStatsAddon", "found at " + std::to_string(i));
  //     s->sunriseTime = lightMeas->indexToTime(i);
  //     break;
  //   }
  // }
}

void SunStatsAddon::addToBuffer(std::shared_ptr<SunStat> ss) {
  for (unsigned int i = 0; i < bufferStat.size(); i++) {
    std::shared_ptr<SunStat> s = bufferStat[i];
    if (s->time == ss->time) {
      bufferStat[i] = ss;
      return;
    }
  }

  if ( (unsigned char) bufferStat.size() >= (unsigned char) bufferMax ) {
    bufferStat.erase( bufferStat.begin() );
  }
  bufferStat.push_back(ss);
}

void SunStatsAddon::store(std::shared_ptr<SunStat> s) {
  std::ofstream outfile;

  logArray->log("SunStatsAddon", "store path " + storeFilename(s));

  outfile.open(storeFilename(s), std::ios_base::app);
  outfile << s->toCsv();
  outfile << std::endl;
  outfile.close();

  logArray->log("SunStatsAddon", "stored");
}

#define NC_SS_ADDON_TIME 0
#define NC_SS_ADDON_TIME_COLUMN 16
#define NC_SS_ADDON_SUNRISE_2 NC_SS_ADDON_TIME + NC_SS_ADDON_TIME_COLUMN
#define NC_SS_ADDON_SUNRISE_8 NC_SS_ADDON_SUNRISE_2 + NC_SS_ADDON_TIME_COLUMN
#define NC_SS_ADDON_SUNRISE_DIFF NC_SS_ADDON_SUNRISE_8 + NC_SS_ADDON_TIME_COLUMN
#define NC_SS_ADDON_SUN_40 NC_SS_ADDON_SUNRISE_DIFF + NC_SS_ADDON_TIME_COLUMN
#define NC_SS_ADDON_SUN_INTERVAL_40 NC_SS_ADDON_SUN_40 + NC_SS_ADDON_TIME_COLUMN
#define NC_SS_ADDON_SUN_INTEGRATED NC_SS_ADDON_SUN_INTERVAL_40 + NC_SS_ADDON_TIME_COLUMN

#define NS_SS_ADDON_COLOR_HIGH NC_COLOR_PAIR_ERROR_SET
#define NS_SS_ADDON_COLOR_MED NC_COLOR_PAIR_NAME_SET
#define NS_SS_ADDON_COLOR_LOW NC_COLOR_PAIR_VALUE_SET

void SunStatsAddon::render() {
  wattron(window, NC_COLOR_PAIR_NAME_SET);
  mvwprintw(window, 1, 1, ("Sun Stats Addon - light: " + lightMeasName).c_str() );
  wattroff(window, NC_COLOR_PAIR_NAME_SET);

  int i = 3;

  // time
  wattron(window, NC_COLOR_PAIR_NAME_LESSER_SET);
  mvwprintw(window, i, 1 + NC_SS_ADDON_TIME, "time" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUNRISE_2, "first rise" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUNRISE_8, "second rise" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUNRISE_DIFF, "diff" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUN_40, "big rise" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUN_INTERVAL_40, "sunny int" );
  mvwprintw(window, i, 1 + NC_SS_ADDON_SUN_INTEGRATED, "tot amount" );
  wattroff(window, NC_COLOR_PAIR_NAME_LESSER_SET);

  if (bufferStat.size() == 0) {
    return;
  }

  for (int j = 0; j < (unsigned char) bufferStat.size(); j++) {
    int iRow = i + 1 + j;

    if (iRow >= (LINES - 3)) {
      break;
    }

    std::shared_ptr<SunStat> s = bufferStat.at(bufferStat.size() - 1 - j);

    wattron(window, NC_COLOR_PAIR_VALUE_SET);
    mvwprintw(window, iRow, 1 + NC_SS_ADDON_TIME, Helper::timeToDateString(s->time).c_str() );
    wattroff(window, NC_COLOR_PAIR_VALUE_SET);

    if (s->foundSunriseTime2) {
      wattron(window, NC_COLOR_PAIR_VALUE_SET);
      mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUNRISE_2, Helper::timeToTimeString(s->sunriseTime2).c_str() );
      wattroff(window, NC_COLOR_PAIR_VALUE_SET);
    }

    if (s->foundSunriseTime8) {
      wattron(window, NC_COLOR_PAIR_VALUE_SET);
      mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUNRISE_8, Helper::timeToTimeString(s->sunriseTime8).c_str() );
      wattroff(window, NC_COLOR_PAIR_VALUE_SET);
    }

    if ( (s->foundSunriseTime2) && (s->foundSunriseTime8) ) {
      wattron(window, NC_COLOR_PAIR_VALUE_SET);
      mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUNRISE_DIFF, Helper::intervalToString(s->sunriseTime8 - s->sunriseTime2).c_str() );
      wattroff(window, NC_COLOR_PAIR_VALUE_SET);
    }

    if (s->foundSunTime40) {
      wattron(window, NC_COLOR_PAIR_VALUE_SET);
      mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUN_40, Helper::timeToTimeString(s->sunTime40).c_str() );
      //
      mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUN_INTERVAL_40, MeasType::formattedValue(((double)(s->sunInterval40) / (3600.0*1000.0)), "h").c_str() );
      wattroff(window, NC_COLOR_PAIR_VALUE_SET);
    }

    if (s->sunIntegrated > 3000.0) {
      wattron(window, NS_SS_ADDON_COLOR_HIGH);
    } else if (s->sunIntegrated > 2000.0) {
      wattron(window, NS_SS_ADDON_COLOR_MED);
    } else {
      wattron(window, NS_SS_ADDON_COLOR_LOW);
    }

    mvwprintw(window, iRow, 1 + NC_SS_ADDON_SUN_INTEGRATED, MeasType::formattedValue(s->sunIntegrated, "").c_str() );

    if (s->sunIntegrated > 3000.0) {
      wattroff(window, NS_SS_ADDON_COLOR_HIGH);
    } else if (s->sunIntegrated > 2000.0) {
      wattroff(window, NS_SS_ADDON_COLOR_MED);
    } else {
      wattroff(window, NS_SS_ADDON_COLOR_LOW);
    }



  }
}

std::string SunStatsAddon::toJson() {
  std::string json = "{\"array\": [";

  for (int j = 0; j < (unsigned char) bufferStat.size(); j++) {
    std::shared_ptr<SunStat> ss = bufferStat.at(bufferStat.size() - 1 - j);
    json += ss->toJson();
    json += ",";
  }

  // remove last coma
  if (json[json.size() - 1] == ',') {
    json.resize(json.size() - 1);
  }

  std::string keyDesc = "[";
  keyDesc += "{\"key\": \"time\", \"type\": \"time\"}";
  keyDesc += ",{\"key\": \"sunriseTime2\", \"type\": \"time\"}";
  keyDesc += ",{\"key\": \"sunriseTime8\", \"type\": \"time\"}";
  keyDesc += ",{\"key\": \"sunTime40\", \"type\": \"time\"}";
  keyDesc += ",{\"key\": \"sunInterval40\", \"type\": \"interval\"}";
  keyDesc += ",{\"key\": \"sunIntegrated\", \"type\": \"float\"}";
  keyDesc += "]";

  json += "], \"name\": \"" + name + "\", \"keys\": " + keyDesc + "}";
  return json;
}
