/*
 * SousvideCommand.h
 *
 */

#ifndef Sousvide_COMMAND_H_
#define Sousvide_COMMAND_H_

#include "SmingCore.h"
#include "../include/pid/SousVideController.h"
#include <libraries/OneWire/OneWire.h>
#include <libraries/DS18S20/ds18s20.h>
#include <basicStructures.h>

typedef Delegate<void(String param, String value)> InfoUpdateSousDelegate;

class SousvideCommand
{
public:
	SousVideController *sousController;
	bool relayState = false;
	bool enabled = false;
	float currentTemp = 0;

	SousvideCommand(int relayPin, int dsTempPin, InfoUpdateSousDelegate delegate = null);
	virtual ~SousvideCommand();
	void initCommand(int setpoint, int Kp, int Ki, int Kd);
	void setOnUpdateOutsideWorld(InfoUpdateSousDelegate delegate);
	void startwork();
private:
	int relayPin;
	int dsTempPin;

	OperationMode operationMode = Manual;



	DS18S20 ReadTemp;
	Timer readTempTimer;
	InfoUpdateSousDelegate updateSousDelegate = null;

	void checkTempTriggerRelay(float temp);
	void processSousvideCommands(String commandLine, CommandOutput* commandOutput);
	void updateOutsideWorld(String param, String value);
	void readData();
	void setRelayState(boolean state);
};

#endif /* Sousvide_COMMAND_H_ */
