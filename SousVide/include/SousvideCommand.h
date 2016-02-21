/*
 * SousvideCommand.h
 *
 */

#ifndef Sousvide_COMMAND_H_
#define Sousvide_COMMAND_H_

#include "SmingCore.h"
#include "../include/pid/SousVideController.h"

//https://www.gitbook.com/book/smartarduino/user-mannual-for-esp-12e-motor-shield/details

//Direcrtion
#define FW 0
#define BK 1
#define STOP 2
//#define FW_NEUTRAL 3

//Turn
#define TL 4
#define TR 5
#define STRAIGHT 6

//Motor movements
#define MOTOR_FW 1
#define MOTOR_BK 0

typedef Delegate<void()> carMotorDelegate;

struct CarParamaters {
	int freq = 30;
	bool useSteeringMotor = false;
};

class SousvideCommand
{
public:
	SousvideCommand(int leftMotorPWM, int rightMotorPWM, int leftMotorDir, int rightMotorDir);
	virtual ~SousvideCommand();
	void initCommand();

private:
	SousVideController *sousController;
	void processSousvideCommands(String commandLine, CommandOutput* commandOutput);
};

#endif /* Sousvide_COMMAND_H_ */
