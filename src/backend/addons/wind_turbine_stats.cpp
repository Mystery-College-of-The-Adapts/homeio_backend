WindTurbineStats::WindTurbineStats() {
  name = "WindTurbineStats";
  lastTime = 0;

  path = "data";
}

void WindTurbineStats::perform() {
  if (calculateTimeFrom() > lastTime) {
    // calculate now

    lastTime = calculateTimeFrom();
    double w = calculatePowerForHour(lastTime);
    WindTurbineStats::store(lastTime, w);
  }
}

unsigned long long WindTurbineStats::calculateTimeFrom() {
  unsigned long long t = mTime();
  t = t - hour; // calculate for previous hour only
  t = t - (t % hour);
  return t;
}

double WindTurbineStats::calculatePowerForHour(unsigned long long t) {
  double w = 0.0;

  MeasType *u = measTypeArray->byName(measNameU);
  MeasType *i = measTypeArray->byName(measNameI);

  vector < unsigned int > uRaw = u->buffer->getFromBuffer(u->timeToIndex(t), u->timeToIndex(t + hour), 0);
  vector < unsigned int > iRaw = i->buffer->getFromBuffer(i->timeToIndex(t), i->timeToIndex(t + hour), 0);

  float measInterval = (float) u->buffer->calcInterval();

  for (unsigned long j = 0; j < uRaw.size(); j++ ) {
    w += ( u->rawToValue( uRaw.at(j) ) ) * ( i->rawToValue( iRaw.at(j) ) ) * measInterval;
  }

  return 0.0;
}

void WindTurbineStats::store(unsigned long long t, double w) {
  unsigned long int measCount = 0;
  ofstream outfile;
  string currentDate( currentDateSafe() );
  string filename = path + "/wind_turbine_stats_" + currentDate + ".csv";

  logInfo("WindTurbineStats store path " + filename);

  outfile.open(filename, ios_base::app);
  outfile << t << "; ";
  outfile << w << "; ";
  outfile << endl;
  outfile.close();

  logInfo("WindTurbineStats stored");
}
