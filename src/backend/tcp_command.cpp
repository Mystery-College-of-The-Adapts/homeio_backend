TcpCommand::TcpCommand() {
}

void TcpCommand::logInfo(string log) {
  logWithColor(log, CYAN);
}

// sample command: "meas;batt_u;0;100;"
string TcpCommand::processCommand(string command) {
  logInfo("TCP command: " + command);
  
  string commandName, response;
  response = "{}";
  
  commandName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);

  if (commandName == "measIndex") {
    response = processMeasIndexCommand(command);
  }
  if (commandName == "measShow") {
    response = processMeasShowCommand(command);
  }
  if (commandName == "actionIndex") {
    response = processActionIndexCommand(command);
  }
  if (commandName == "actionShow") {
    response = processActionShowCommand(command);
  }
  if (commandName == "overseerIndex") {
    response = processOverseerIndexCommand(command);
  }
  if (commandName == "overseerShow") {
    response = processOverseerShowCommand(command);
  }
  
  

  //
  if (commandName == "measNameList") {
    response = processMeasNameListCommand(command);
  }
  
  if (commandName == "meas") {
    response = processMeasCommand(command);
  }

  if (commandName == "measStorage") {
    response = processMeasStorageCommand(command);
  }
  

  if (commandName == "actionExecute") {
    response = processActionExecuteCommand(command);
  }

  if (commandName == "actionHistory") {
    response = processActionHistoryCommand(command);
  }
  
  
  
  logInfo("TCP response: " + response);
  
  return response;
}

// meas#index
string TcpCommand::processMeasIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<MeasType>::iterator it = measTypeArray->measTypes.begin(); it != measTypeArray->measTypes.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// meas#show
string TcpCommand::processMeasShowCommand(string command) {
  string response, measName;
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    response = "{\"status\":0,\"object\":" + foundMeasType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }
  
  return response;
}

// actions#index
string TcpCommand::processActionIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<ActionType>::iterator it = actionTypeArray->actionTypes.begin(); it != actionTypeArray->actionTypes.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }  
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// action#show
string TcpCommand::processActionShowCommand(string command) {
  string response, actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    response = "{\"status\":0,\"object\":" + foundActionType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action_type\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;
}

// overseers#index
string TcpCommand::processOverseerIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<Overseer>::iterator it = overseerArray->overseers.begin(); it != overseerArray->overseers.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }  
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// overseers#show
string TcpCommand::processOverseerShowCommand(string command) {
  string response, overseerName;
  overseerName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  Overseer *foundOverseer = overseerArray->byName(overseerName);
  
  if (foundOverseer) {
    response = "{\"status\":0,\"object\":" + foundOverseer->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"overseer\":\"" + overseerName + "\",\"reason\":\"overseer_not_found\"}";
  }
  
  return response;
}



string TcpCommand::processMeasNameListCommand(string command) {
  string response;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<MeasType>::iterator it = measTypeArray->measTypes.begin(); it != measTypeArray->measTypes.end(); ++it) {
    response += "\"" + it->name + "\",";
  }
  
  // remove last coma
  if (response[response.size() - 1] == ',') {
    response.resize(response.size() - 1);
  }
  response += "]}";
    
  return response;
}

//
string TcpCommand::processMeasCommand(string command) {
  string measName, fromString, toString, response;
  unsigned long int from, to;
  
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  fromString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  toString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  stringstream(fromString) >> from;
  stringstream(toString) >> to;
  
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    string bufferString = foundMeasType->buffer->jsonArray(from, to);
    response = "{\"status\":0,\"meas_type\":\"" + measName + "\"";
    response += ",\"lastTime\":" + to_string( foundMeasType->buffer->lastTime );
    response += ",\"firstTime\":" + to_string( foundMeasType->buffer->firstTime );
    response += ",\"interval\":" + to_string( foundMeasType->buffer->calcInterval() );
    response += ",\"count\":" + to_string( foundMeasType->buffer->count );
    response += ",\"miliSeconds\":1";
    response += ",\"data\": " + bufferString;
    response += "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }
  
  return response;
}





string TcpCommand::processMeasStorageCommand(string command) {
  string measName, fromString, toString, response;
  unsigned long long from, to;
  
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  fromString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  toString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  stringstream(fromString) >> from;
  stringstream(toString) >> to;
  
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    string bufferString = foundMeasType->storageJson(from, to);
    response = "{\"status\":0,\"meas_type\":\"" + measName + "\",";
    response +=  "\"data\":" + bufferString;
    response += "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }  
  return response;
}


string TcpCommand::processActionExecuteCommand(string command) {
  string actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  string response;
  
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    foundActionType->execute();
    response = "{\"status\":0,\"action\":" + foundActionType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;  
}

string TcpCommand::processActionHistoryCommand(string command) {
  string actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  string response;
  
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    response = "{\"status\":0,\"action\":" + foundActionType->historyToJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;  
}



