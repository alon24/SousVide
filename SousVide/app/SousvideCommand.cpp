/*
 * CarCommand.cpp
 *
 */

#include <SousvideCommand.h>

CarCommand::CarCommand(int leftMotorPWM, int rightMotorPWM, int leftMotorDir, int rightMotorDir)
{
	this->leftMotorPWM = leftMotorPWM;
	this->rightMotorPWM = rightMotorPWM;
	this->leftMotorDir = leftMotorDir;
	this->rightMotorDir = rightMotorDir;

	uint8_t pins[8] = { (uint8_t)leftMotorPWM, (uint8_t)rightMotorPWM }; // List of pins that you want to connect to pwm

	pwmMotors = new HardwarePWM(pins, 2);
	setFreq(carParams.freq);
	debugf("CarCommand Instantiating");
}

CarCommand::~CarCommand()
{
}

void CarCommand::initCommand()
{
	commandHandler.registerCommand(CommandDelegate("Move","Example Command from Class","Application",commandFunctionDelegate(&CarCommand::processCarCommands,this)));

	pinMode(leftMotorDir, OUTPUT);
	digitalWrite(leftMotorDir, HIGH);
	pinMode(rightMotorDir, OUTPUT);
	digitalWrite(rightMotorDir, HIGH);

	//Check and act upon car commands
	motorTimer.setCallback(carMotorDelegate(&CarCommand::handleMotorTimer, this));
	motorTimer.setIntervalMs(150);

//	motorTimer.setCallback(carMotorDelegate(&CarCommand::testPWM, this));
//	motorTimer.setIntervalMs(1000);
//	motorTimer.start(true);
}

void CarCommand::tuneCarParamaters(int freq, bool useSteering) {
	setFreq(freq);
	carParams.useSteeringMotor = useSteering;
}

int CarCommand::getCurrentFreq() {
	return carParams.freq;
}

void CarCommand::setFreq(int freq){
	carParams.freq = freq;
	pwmMotors->setPeriod(1000000 / carParams.freq);
}

void CarCommand::testPWM()
{
//	if(countUp){
//		i++;
//		if(i == 100){
//			countUp = false;
//			countDown = true;
//		}
//	}
//	else{
//		i--;
//		if(i == 0){
//			countUp = true;
//			countDown = false;
//		}
//	}

	if (countUp) {

		i = 100;
		countUp = false;
		countDown = true;
		pwmMotors->setDuty(leftMotorPWM, 1023);
	}else {
		i = 1;
		countUp = true;
		countDown = false;
		pwmMotors->setDuty(leftMotorPWM,0);
//		pwmMotors.setDuty(leftMotorPWM, 1);
	}

	int pp = map(i, 0, 100, 0, 1023);
	Serial.println(pp);
//	pwmMotors.setDuty(leftMotorPWM, pp);
//	digitalWrite(leftMotorDir, LOW);
//	pwmMotors.setDuty(rightMotorPWM, pp);
//	digitalWrite(rightMotorDir, HIGH);
}

void CarCommand::processCarCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);
	int rightPwm =0;
	int leftPwm = 0;
	int rightDir = 0;
	int leftDir = 0;

	debugf("Got commandLine = %s", commandLine.c_str());
	if (numToken == 1)
	{
		commandOutput->printf("Move Commands available : \r\n");
		commandOutput->printf("stop : Show example status\r\n");
		commandOutput->printf("xyz xValue yValue: Send X,Y and Z PWR (X,Y can be negative for reverse)\n");
	}
	else
	{
		//first thing stop the "stop timer"
		if (motorTimer.isStarted()) {
			motorTimer.stop();
		}

		if (commandToken[1] == "xyJoystick") {
			int x = commandToken[2].toInt();
			int y = commandToken[3].toInt();
			handleJoystickXY(x, y);
		}
		else if (commandToken[1] == "freq") {
			int pin = commandToken[2].toInt();
			int freq = commandToken[3].toInt();
			int pwr = commandToken[4].toInt();
			handleCheckFreq(pin, freq, pwr);
		}
		else if (commandToken[1] == "tune") {
			int freq = commandToken[2].toInt();
			int useSteering = commandToken[3].toInt();
			tuneCarParamaters(freq, useSteering);
		}
		else if (commandToken[1].startsWith("xy")) {
			int x = commandToken[2].toInt();
			int y = commandToken[3].toInt();

			debugf("ilan1:y=%i, abs(y) =%i, leftP=%i,rightP=%i",y, abs(y), leftPwm, rightPwm);
			//check direction to move(needed for knowing which side to move - wheels)
			if (y > 0) {
				dir = FW;
			} else if (y == 0) {
				dir = STOP;
			}
			else {
				dir = BK;
			}

			// if currently we just do right or left, keep the last heading (lastY)
			if (x != 0 && y == 0) {
				if (lastY != 0) {
					if (lastY > 0) {
						dir = FW;
						y = lastY;
					}
					else {
						dir = BK;
						y = lastY;
					}
				}
			}

			lastY = y;
			lastX = x;

			if (dir != STOP) {
				if (x>0) {
					debugf("!@Stop=1");
					tdir = TR;
				} else if (x<0) {
					debugf("!@Stop=2");
					tdir = TL;
				} else {
					debugf("!@Stop=3");
					tdir = STRAIGHT;
				}

				//set motors to run (power)
				leftPwm = map(abs(y), 0, 100,  0, 1023);
				rightPwm = map(abs(y), 0, 100,  0, 1023);
				debugf("ilan13: leftP=%i,rightP=%i",leftPwm, rightPwm);
				//check if we want to move right
				if (dir == FW) {
					if (tdir == TL) {
						debugf("FW1");
						rightDir = HIGH;
						leftPwm = 0;
					} else if (tdir == TR) {
						debugf("FW2");
						leftDir = HIGH;
						rightPwm = 0;
					}else if (tdir == STRAIGHT) {
						debugf("FW3");
						leftDir = HIGH;
						rightDir = HIGH;
					}
				}
				else if (dir == BK)
				{
					if (tdir == TL) {
						debugf("bk1");
						rightDir = LOW;
						leftPwm = 0;
					} else if (tdir == TR) {
						debugf("bk2");
						leftDir = LOW;
						rightPwm = 0;
					}else if (tdir == STRAIGHT) {
						debugf("bk3");
						leftDir = LOW;
						rightDir = LOW;
					}
				}
			}
			else {
				leftPwm = 0;
				rightPwm = 0;
			}

			debugf("inside command:leftD=%i,leftP=%i,rightD=%i,rightP=%i", leftDir, leftPwm, rightDir, rightPwm);
			drive(leftDir, leftPwm, rightDir, rightPwm);
			motorTimer.startOnce();
		}
	}
}

/*
 * pin - pin number
 * Freq in Hz so 250 == 250Hz
 * pwr - how much power 0-1023
 */
void CarCommand::handleCheckFreq(int pin, int frq, int pwr) {
	int time = 1000000 / frq;
	pwmMotors->setPeriod(time);
	pwmMotors->analogWrite(pin, pwr);
	debugf("handleCheckFreq pin=%i, freq=%i, time=%i, pwr=%i", pin, frq, time, pwr);
}

void CarCommand::handleRegularXy() {

}

void CarCommand::handleJoystickXY(int x, int y) {
	if (y == 0){
		drive(0,0,0,0);
		return;
	}

	int absY = abs(y);
	int powerLeft = absY;
	int powerRight = powerLeft;
	int absX = abs(x);

	int dirLeft = 1;
	int dirRight =1;

	//in an abs world - decrease half the mortor power from the turning direction
	if ( absX > 0 ) {
		powerRight = powerLeft - absY * absX / 100;
		if (powerRight < 0) {
			powerRight = 0;
		}
	}

	//now Translate to actual directions... 4 quadrents are available
	if (y > 0) {
		dir = FW;
	} else if (y == 0) {
		dir = STOP;
	}
	else {
		dir = BK;
	}

	if (dir != STOP) {
		if (x>0) {
			debugf("!@Stop=1");
			tdir = TR;
		} else if (x<0) {
			debugf("!@Stop=2");
			tdir = TL;
		} else {
			debugf("!@Stop=3");
			tdir = STRAIGHT;
		}

		//FF + TR -> Already covered!!!
		if (dir == FW && tdir == TL) {
			//switch MAIN engine (so turn left
			powerLeft += powerRight;
			powerRight = powerLeft - powerRight;
			powerLeft = powerLeft - powerRight;
		} else if (dir == BK) {
			dirLeft = 0;
			dirRight = 0;

			if (tdir == TL) {
				powerLeft += powerRight;
				powerRight = powerLeft - powerRight;
				powerLeft = powerLeft - powerRight;
			}
		}
	}

	int leftPwm = map(abs(powerLeft), 0, 100,  minPower, 1023);
	int rightPwm = map(abs(powerRight), 0, 100,  minPower, 1023);
	debugf("************* handleJoystickXY: x=%i, y=%i, dirLeft=%i, leftPwm=%i, dirRight=%i, rightPwm=%i", x, y, dirLeft, leftPwm, dirRight, rightPwm);
	drive(dirLeft, leftPwm, dirRight, rightPwm);
}

int CarCommand::roundMovement(int power) {
	//scale is 1 -> 10
	//output is 0->1023
	int tmpPower = 1023 / 10 * power;

	return tmpPower;
}

void CarCommand::drive(int leftDir, int leftPwm, int rightDir, int rightPwm) {
	debugf("drive command:leftD=%i,leftP=%i,rightD=%i,rightP=%i", leftDir, leftPwm, rightDir, rightPwm);

	digitalWrite(leftMotorDir, leftDir);
	digitalWrite(rightMotorDir, rightDir);

	if (leftPwm < minPower ){
		pwmMotors->setDuty(leftMotorPWM, 0, false);
	} else {
		pwmMotors->setDuty(leftMotorPWM, leftPwm, false);
	}

	if (rightPwm < minPower ){
		pwmMotors->setDuty(rightMotorPWM, 0, false);
	} else {
		pwmMotors->setDuty(rightMotorPWM, rightPwm, false);
	}

	pwmMotors->restart();
}

//Stop the car
void CarCommand::handleMotorTimer() {
	drive(0,0,0,0);
};
