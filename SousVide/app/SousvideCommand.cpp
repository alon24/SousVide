/*
 * SousvideCommand.cpp
 *
 */

#include <SousvideCommand.h>

SousvideCommand::SousvideCommand(int leftMotorPWM, int rightMotorPWM, int leftMotorDir, int rightMotorDir)
{
	debugf("SousvideCommand Instantiating");
}

SousvideCommand::~SousvideCommand()
{
}

void SousvideCommand::initCommand()
{
	commandHandler.registerCommand(CommandDelegate("sous","Example Command from Class","Application",commandFunctionDelegate(&SousvideCommand::processSousvideCommands,this)));
}

void SousvideCommand::processSousvideCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);

	debugf("Got commandLine = %s", commandLine.c_str());
	if (numToken == 1)
	{
		commandOutput->printf("Move Commands available : \r\n");
		commandOutput->printf("stop : Show example status\r\n");
		commandOutput->printf("xyz xValue yValue: Send X,Y and Z PWR (X,Y can be negative for reverse)\n");
	}
	else
	{
	}
}

