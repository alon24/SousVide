/*
 * SousvideCommand.cpp
 *
 */

#include <SousvideCommand.h>

SousvideCommand::SousvideCommand(int relayPin, int dsTempPin, InfoUpdateSousDelegate delegate)
{
	debugf("SousvideCommand Instantiating");
	this->relayPin = relayPin;
	this->dsTempPin = dsTempPin;

	//init the relay
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, HIGH);
	updateSousDelegate = delegate;

	sousController = new SousVideController();

//	ReadTemp.Init(dsTempPin);  			// select PIN It's required for one-wire initialization!

}

void SousvideCommand::startwork() {
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
	readTempTimer.initializeMs(10000, TimerDelegate(&SousvideCommand::readData, this)).start();   // every 10 seconds
}

SousvideCommand::~SousvideCommand()
{
	delete(sousController);
	sousController = null;
}

void SousvideCommand::initCommand(int setpoint, int Kp, int Ki, int Kd)
{
	commandHandler.registerCommand(CommandDelegate("sousvide","Sousvide commands", "Application",commandFunctionDelegate(&SousvideCommand::processSousvideCommands,this)));
	sousController->Setpoint = setpoint;
	sousController->Kp = Kp;
	sousController->Ki = Ki;
	sousController->Kd = Kd;
}

void SousvideCommand::processSousvideCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);

	debugf("**** SousvideCommand Got commandLine = %s", commandLine.c_str());
	if (numToken == 1)
	{
		commandOutput->printf("Move Commands available : \r\n");
		commandOutput->printf("stop : Show example status\r\n");
		commandOutput->printf("xyz xValue yValue: Send X,Y and Z PWR (X,Y can be negative for reverse)\n");
	}
	else
	{
//		debugf("**** SousvideCommand cmd is:%s",commandToken[1].c_str());
		if(commandToken[1].equals("change-val-target")) {
			double temp = atof(commandToken[2].c_str());
			sousController->Setpoint = temp;
			updateOutsideWorld("target", commandToken[2]);
		}
		else if(commandToken[1].equals("change-val-p")) {
			double p = atof(commandToken[2].c_str());
			sousController->Kp = p;
			updateOutsideWorld("Kp", commandToken[2]);
		}
		else if(commandToken[1].equals("change-val-i")) {
			double i = atof(commandToken[2].c_str());
			sousController->Ki = i;
			updateOutsideWorld("Ki", commandToken[2]);
		}
		else if(commandToken[1].equals("change-val-d")) {
			double d = atof(commandToken[2].c_str());
			sousController->Kd = d;
			updateOutsideWorld("Kd", commandToken[2]);
		}
		else if (commandToken[1].equals("toggleRelay")) {
//			String state = commands.substring(12);
//			setRelayState(state.equals("true"));
			debugf("toggleRelay: op mode=%s, setting=%s", (operationMode == Manual ? "Manual" : "Sousvide"), commandToken[2].c_str());
			if (operationMode == Manual) {
				setRelayState(commandToken[2].equals("true"));
			}
		} else if(commandToken[1].equals("setOperationMode")) {
//			debugf("set op mode=%s", commandToken[2].c_str());
			setOperationMode(commandToken[2] == "1" ? Manual : Sousvide);
		} else if(commandToken[1].equals("highlow")) {
			debugf("set highlow mode=%s", commandToken[2].c_str());
			highLow = commandToken[2] == "true";
//			setOperationMode(commandToken[2] == "1" ? Manual : Sousvide);
		}
	}
}

//Set delegate to get data to the real world
void SousvideCommand::setOnUpdateOutsideWorld(InfoUpdateSousDelegate delegate) {
	updateSousDelegate = delegate;
}

void SousvideCommand::updateOutsideWorld(String param, String value) {
	if (updateSousDelegate) {
		updateSousDelegate(param, value);
	}
}

void SousvideCommand::readData()
{
	uint8_t a;
	uint64_t info;

	if (!ReadTemp.MeasureStatus())  // the last measurement completed
	{
      if (ReadTemp.GetSensorsCount()) {  // is minimum 1 sensor detected ?
	    Serial.println(" Reading temperature");
	    //only read first
	    int a=0;
	    float celsius=0;
	      Serial.print(" T");
	      Serial.print(a+1);
	      Serial.print(" = ");

	      if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
	        {
	    	  Serial.print(ReadTemp.GetCelsius(a));
	    	  Serial.print(" Celsius, (");
	    	  currentTemp = celsius;
	    	  celsius = ReadTemp.GetCelsius(a);
	    	  updateOutsideWorld("temp", String(currentTemp, 2));
//	    	  updateWebSockets("temp:" + String(celsius));
//	    	  infos->updateParamValue("temp", String(currentTemp, 2));
	    	  checkTempTriggerRelay(celsius);
	        }
	      else
	    	  Serial.println("Temperature not valid");

	      ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
      }
	}
	else
		Serial.println("No valid Measure so far! wait please");
}

void SousvideCommand::checkTempTriggerRelay(float temp) {
	//move on this only when Sousvide, do not interfere with manual mode
	if (operationMode == Sousvide)
	{
		int trigger = (int)sousController->Setpoint;
		boolean newState = false;
		if((int)temp < trigger && !relayState ) {
			debugf("current temp is below %i, so starting relay", temp);
			newState = true;
//			Serial.println("temp is " + String(trigger) + ", starting relay");
//			handleCommands("toggleRelay:true");
		} else if((int)temp >= trigger && relayState){
			debugf("current temp is %i, so stopping relay", temp);
//			Serial.println("temp is <  " + String(trigger) + ",stopping relay");
//			handleCommands("toggleRelay:false");
		}
		setRelayState(newState);
	}
}

void SousvideCommand::setRelayState(boolean state) {

	if (state != relayState) {
		relayState = state;
		debugf("new relaystate =%s ",String(relayState).c_str());
//		digitalWrite(relayPin, (relayState ? HIGH : LOW));

		if (highLow) {
			digitalWrite(relayPin, (relayState ? LOW : HIGH));
		} else {
			digitalWrite(relayPin, (relayState ? HIGH : LOW));
		}
	}
}

void SousvideCommand::setOperationMode(OperationMode mode) {
	operationMode = mode;
}

//void handleCommands(String commands) {
////	debugf("handleCommands::now handling %s", commands.c_str());
//
//	//iterate over commands
//	Vector<String> commandToken;
//	int numToken = splitString(commands, ';' , commandToken);
//
//	for (int i = 0; i < numToken; ++i) {
//		String command = commandToken.get(i);
//		String parsedCmd = getCommandAndData(command);
//
//		debugf("handleCommands::command=%s, data=%s", parsedCmd.c_str(), command.c_str());
//		//command is now the just the data (stripped of the command itself)
////		debugf("handleCommands::original cmd=%s, parsed=%s, data=%s", command.c_str(), parsedCmd.c_str(), command.c_str());
//
//		if(parsedCmd.equals("query")) {
//
//		}
//		else if (parsedCmd.equals("toggleSousvideOperation")) {
//			if (command.equals("false")) {
//				operationMode = Manual;
//				updateWebSockets("relayState:false");
//			} else {
//				operationMode = Sousvide;
//			}
//
//			debugf("sous operation = %s", (operationMode == Sousvide ? "Sousvide" : " Manual"));
//		}
//		else if (parsedCmd.equals("toggleRelay")) {
////			String state = commands.substring(12);
////			setRelayState(state.equals("true"));
//
//			setRelayState(command.equals("true"));
//
//	//		Serial.println("handleCommands::toggeled relay " + String(state));
//	//		relayState = state.equals("true") ? true : false;
//	//		digitalWrite(relayPin, (state.equals("true") ? HIGH : LOW));
//
//			Serial.println("handleCommand:: state.equals(true)==" + String(command.equals("true")));
//			updateWebSockets("relayState:" + String(relayState == true ? "true" : "false"));
//		}
//		else if(parsedCmd.equals("change-val-SetPoint")) {
//			double temp = atof(command.c_str());
//			sousController->Setpoint = temp;
//			infos->updateParamValue("Setpoint", command);
//		}
//		else if(parsedCmd.equals("change-val-p")) {
//			double p = atof(command.c_str());
//			sousController->Kp = p;
//			infos->updateParamValue("Kp", command);
//		}
//		else if(parsedCmd.equals("change-val-i")) {
//			double i = atof(command.c_str());
//			sousController->Ki = i;
//			infos->updateParamValue("Ki", command);
//		}
//		else if(parsedCmd.equals("change-val-d")) {
//			double d = atof(command.c_str());
//			sousController->Kd = d;
//			infos->updateParamValue("Kd", command);
//		}
//		else if(parsedCmd.equals("saveSettings")) {
//			ActiveConfig.Kp = sousController->Kp;
//			ActiveConfig.Ki = sousController->Ki;
//			ActiveConfig.Kd = sousController->Kd;
//			ActiveConfig.Needed_temp = sousController->Setpoint;
//			saveConfig(ActiveConfig);
//			//TODO: save changes to file
//		}
//		else if(parsedCmd.equals("wifi")) {
//			Vector<String> commandToken;
//			int numToken = splitString(command, ',' , commandToken);
//			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
//			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
//		}
//		else if(parsedCmd.equals("connect")) {
//			Vector<String> commandToken;
//			int numToken = splitString(command, ',' , commandToken);
//			ActiveConfig.NetworkSSID = commandToken.elementAt(0);
//			ActiveConfig.NetworkPassword = commandToken.elementAt(1);
//			saveConfig(ActiveConfig);
//			debugf("Connecting to %s, %s", ActiveConfig.NetworkSSID.c_str(), ActiveConfig.NetworkPassword.c_str());
//			WifiStation.enable(false);
//			WifiStation.config(ActiveConfig.NetworkSSID, ActiveConfig.NetworkPassword);
//			WifiStation.enable(true);
//			infos->updateParamValue("ssid", "try:" + ActiveConfig.NetworkSSID);
//			infos->updateParamValue("station", "reconnecting");
//			WifiStation.waitConnection(connectOk, 25, connectFail); // We recommend 20+ seconds for connection timeout at start
//		}
//		else if(parsedCmd.equals("reboot")) {
//			debugf("Rebooting");
//			System.restart();
//		}
//	}
//}

