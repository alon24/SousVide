/*
 * MyPID.h
 *
 *  Created on: Aug 29, 2015
 *      Author: ilan
 */

#ifndef INCLUDE_SousVideController_H_
#define INCLUDE_SousVideController_H_

#include <pid/PID_v1.h>
#include <pid/PID_AutoTune_v0.h>

class SousVideController {
	volatile long onTime = 0;

	// EEPROM addresses for persisted data
	const int SpAddress = 0;
	const int KpAddress = 8;
	const int KiAddress = 16;
	const int KdAddress = 24;

	//Specify the links and initial tuning parameters
	PID *myPID;

	// 10 second Time Proportional Output window
	int WindowSize = 10000;
	unsigned long windowStartTime;

	// ************************************************
	// Auto Tune Variables and constants
	// ************************************************
	byte ATuneModeRemember=2;

	double aTuneStep=500;
	double aTuneNoise=1;
	unsigned int aTuneLookBack=20;

	boolean tuning = false;

	PID_ATune *aTune;

	// ************************************************
	// States for state machine
	// ************************************************
	enum operatingState { OFF = 0, SETP, RUN, TUNE_P, TUNE_I, TUNE_D, AUTO};
	operatingState opState = OFF;
protected:
	void initPID() {
		LoadParameters();
		myPID->SetTunings(Kp,Ki,Kd);

		myPID->SetSampleTime(1000);
		myPID->SetOutputLimits(0, WindowSize);

	//	 // Run timer2 interrupt every 15 ms
	//	  TCCR2A = 0;
	//	  TCCR2B = 1<<CS22 | 1<<CS21 | 1<<CS20;
	//
	//	  //Timer2 Overflow Interrupt Enable
	//	  TIMSK2 |= 1<<TOIE2;
	}
public:

	//Define Variables we'll be connecting to
	double Setpoint = 36;
	double Input;
	double Output;
	// pid tuning parameters
	double Kp = 0;
	double Ki = 0;
	double Kd = 0;

	SousVideController() {
		myPID = new PID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
		aTune = new PID_ATune(&Input, &Output);
		initPID();
	};

	~SousVideController() {
		delete(myPID);
		delete(aTune);
	};

	void LoadParameters(){

	};

	void SaveParameters() {

	};

	// ************************************************
	// Initial State - press RIGHT to enter setpoint
	// ************************************************
	void Off()
	{
	   myPID->SetMode(MANUAL);
	   debugf("Selected OFF state");
	   opState = OFF;
	};

	void Tune_Sp()
	{
	//   lcd.setBacklight(TEAL);
	//   lcd.print(F("Set Temperature:"));
	//   uint8_t buttons = 0;
	//   while(true)
	//   {
	//      buttons = ReadButtons();
	//
	//      float increment = 0.1;
	//      if (buttons & BUTTON_SHIFT)
	//      {
	//        increment *= 10;
	//      }
	//      if (buttons & BUTTON_LEFT)
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      if (buttons & BUTTON_RIGHT)
	//      {
	//         opState = TUNE_P;
	//         return;
	//      }
	//      if (buttons & BUTTON_UP)
	//      {
	//         Setpoint += increment;
	//         delay(200);
	//      }
	//      if (buttons & BUTTON_DOWN)
	//      {
	//         Setpoint -= increment;
	//         delay(200);
	//      }
	//
	//      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      lcd.setCursor(0,1);
	//      lcd.print(Setpoint);
	//      lcd.print(" ");
	//      DoControl();
	//   }
	}

	// ************************************************
	// Proportional Tuning State
	// UP/DOWN to change Kp
	// RIGHT for Ki
	// LEFT for setpoint
	// SHIFT for 10x tuning
	// ************************************************
	void TuneP()
	{
	//   lcd.setBacklight(TEAL);
	//   lcd.print(F("Set Kp"));
	//
	//   uint8_t buttons = 0;
	//   while(true)
	//   {
	//      buttons = ReadButtons();
	//
	//      float increment = 1.0;
	//      if (buttons & BUTTON_SHIFT)
	//      {
	//        increment *= 10;
	//      }
	//      if (buttons & BUTTON_LEFT)
	//      {
	//         opState = SETP;
	//         return;
	//      }
	//      if (buttons & BUTTON_RIGHT)
	//      {
	//         opState = TUNE_I;
	//         return;
	//      }
	//      if (buttons & BUTTON_UP)
	//      {
	//         Kp += increment;
	//         delay(200);
	//      }
	//      if (buttons & BUTTON_DOWN)
	//      {
	//         Kp -= increment;
	//         delay(200);
	//      }
	//      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      lcd.setCursor(0,1);
	//      lcd.print(Kp);
	//      lcd.print(" ");
	//      DoControl();
	//   }
	}

	// ************************************************
	// Integral Tuning State
	// UP/DOWN to change Ki
	// RIGHT for Kd
	// LEFT for Kp
	// SHIFT for 10x tuning
	// ************************************************
	void TuneI()
	{
	//   lcd.setBacklight(TEAL);
	//   lcd.print(F("Set Ki"));
	//
	//   uint8_t buttons = 0;
	//   while(true)
	//   {
	//      buttons = ReadButtons();
	//
	//      float increment = 0.01;
	//      if (buttons & BUTTON_SHIFT)
	//      {
	//        increment *= 10;
	//      }
	//      if (buttons & BUTTON_LEFT)
	//      {
	//         opState = TUNE_P;
	//         return;
	//      }
	//      if (buttons & BUTTON_RIGHT)
	//      {
	//         opState = TUNE_D;
	//         return;
	//      }
	//      if (buttons & BUTTON_UP)
	//      {
	//         Ki += increment;
	//         delay(200);
	//      }
	//      if (buttons & BUTTON_DOWN)
	//      {
	//         Ki -= increment;
	//         delay(200);
	//      }
	//      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      lcd.setCursor(0,1);
	//      lcd.print(Ki);
	//      lcd.print(" ");
	//      DoControl();
	//   }
	}

	// ************************************************
	// Derivative Tuning State
	// UP/DOWN to change Kd
	// RIGHT for setpoint
	// LEFT for Ki
	// SHIFT for 10x tuning
	// ************************************************
	void TuneD()
	{
	//   lcd.setBacklight(TEAL);
	//   lcd.print(F("Set Kd"));
	//
	//   uint8_t buttons = 0;
	//   while(true)
	//   {
	//      buttons = ReadButtons();
	//      float increment = 0.01;
	//      if (buttons & BUTTON_SHIFT)
	//      {
	//        increment *= 10;
	//      }
	//      if (buttons & BUTTON_LEFT)
	//      {
	//         opState = TUNE_I;
	//         return;
	//      }
	//      if (buttons & BUTTON_RIGHT)
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      if (buttons & BUTTON_UP)
	//      {
	//         Kd += increment;
	//         delay(200);
	//      }
	//      if (buttons & BUTTON_DOWN)
	//      {
	//         Kd -= increment;
	//         delay(200);
	//      }
	//      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
	//      {
	//         opState = RUN;
	//         return;
	//      }
	//      lcd.setCursor(0,1);
	//      lcd.print(Kd);
	//      lcd.print(" ");
	//      DoControl();
	//   }
	}

	// ************************************************
	// PID COntrol State
	// SHIFT and RIGHT for autotune
	// RIGHT - Setpoint
	// LEFT - OFF
	// ************************************************
	void Run()
	{
	   //turn the PID on
	   myPID->SetMode(AUTOMATIC);
	   windowStartTime = millis();
	   opState = RUN; // start control

//	   SaveParameters();
//	   myPID.SetTunings(Kp,Ki,Kd);
//
//	   //periodically log to serial port in csv format
//	  if (millis() - lastLogTime > logInterval)
//	  {
//		Serial.print(Input);
//		Serial.print(",");
//		Serial.println(Output);
//	  }
	}
private:
	// ************************************************
	// Called by ISR every 15ms to drive the output
	// ************************************************
	void DriveOutput()
	{
	//  long now = millis();
	//  // Set the output
	//  // "on time" is proportional to the PID output
	//  if(now - windowStartTime>WindowSize)
	//  { //time to shift the Relay Window
	//     windowStartTime += WindowSize;
	//  }
	//  if((onTime > 100) && (onTime > (now - windowStartTime)))
	//  {
	//     digitalWrite(RelayPin,HIGH);
	//  }
	//  else
	//  {
	//     digitalWrite(RelayPin,LOW);
	//  }
	}

	// ************************************************
	// Execute the control loop
	// ************************************************
	void DoControl()
	{
	//  // Read the input:
	//  if (sensors.isConversionAvailable(0))
	//  {
	//    Input = sensors.getTempC(tempSensor);
	//    sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
	//  }
	//
	//  if (tuning) // run the auto-tuner
	//  {
	//     if (aTune.Runtime()) // returns 'true' when done
	//     {
	//        FinishAutoTune();
	//     }
	//  }
	//  else // Execute control algorithm
	//  {
	//     myPID.Compute();
	//  }
	//
	//  // Time Proportional relay state is updated regularly via timer interrupt.
	//  onTime = Output;
	}
};

////Forward Refs
//void SaveParameters();
//
//
//// ************************************************
//// PID Variables and constants
//// ************************************************
//
////Define Variables we'll be connecting to
//double Setpoint;
//double Input;
//double Output;
//
//volatile long onTime = 0;
//
//// pid tuning parameters
//double Kp;
//double Ki;
//double Kd;
//
//// EEPROM addresses for persisted data
//const int SpAddress = 0;
//const int KpAddress = 8;
//const int KiAddress = 16;
//const int KdAddress = 24;
//
////Forward References
//void LoadParameters();
//
////Specify the links and initial tuning parameters
//PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
//
//// 10 second Time Proportional Output window
//int WindowSize = 10000;
//unsigned long windowStartTime;
//
//// ************************************************
//// Auto Tune Variables and constants
//// ************************************************
//byte ATuneModeRemember=2;
//
//double aTuneStep=500;
//double aTuneNoise=1;
//unsigned int aTuneLookBack=20;
//
//boolean tuning = false;
//
//PID_ATune aTune(&Input, &Output);
//
//// ************************************************
//// States for state machine
//// ************************************************
//enum operatingState { OFF = 0, SETP, RUN, TUNE_P, TUNE_I, TUNE_D, AUTO};
//operatingState opState = OFF;
//
//void initPID() {
//	LoadParameters();
//	myPID.SetTunings(Kp,Ki,Kd);
//
//	myPID.SetSampleTime(1000);
//	myPID.SetOutputLimits(0, WindowSize);
//
////	 // Run timer2 interrupt every 15 ms
////	  TCCR2A = 0;
////	  TCCR2B = 1<<CS22 | 1<<CS21 | 1<<CS20;
////
////	  //Timer2 Overflow Interrupt Enable
////	  TIMSK2 |= 1<<TOIE2;
//}
//
//
////// ************************************************
////// Timer Interrupt Handler
////// ************************************************
////SIGNAL(TIMER2_OVF_vect)
////{
////  if (opState == OFF)
////  {
////    digitalWrite(RelayPin, LOW);  // make sure relay is off
////  }
////  else
////  {
////    DriveOutput();
////  }
////}
//
//////************************************************
////// Main Control Loop
//////
////// All state changes pass through here
////// ************************************************
////void loop()
////{
////  // wait for button release before changing state
////  while(ReadButtons() != 0) {}
////
////  lcd.clear();
////
////  switch (opState)
////  {
////  case OFF:
////     Off();
////     break;
////  case SETP:
////     Tune_Sp();
////     break;
////   case RUN:
////     Run();
////     break;
////  case TUNE_P:
////     TuneP();
////     break;
////  case TUNE_I:
////     TuneI();
////     break;
////  case TUNE_D:
////     TuneD();
////     break;
////  }
////}
//
//// ************************************************
//// Initial State - press RIGHT to enter setpoint
//// ************************************************
//void Off()
//{
//   myPID.SetMode(MANUAL);
////   lcd.setBacklight(0);
//
////   digitalWrite(RelayPin, LOW);  // make sure it is off
//
////   lcd.print(F("    Adafruit"));
////   lcd.setCursor(0, 1);
////   lcd.print(F("   Sous Vide!"));
////   uint8_t buttons = 0;
//
////   while(!(buttons & (BUTTON_RIGHT)))
////   {
////      buttons = ReadButtons();
////   }
////
////   // Prepare to transition to the RUN state
////   sensors.requestTemperatures(); // Start an asynchronous temperature reading
//
//   //turn the PID on
//   myPID.SetMode(AUTOMATIC);
//   windowStartTime = millis();
//   opState = RUN; // start control
//}
//
//// ************************************************
//// Setpoint Entry State
//// UP/DOWN to change setpoint
//// RIGHT for tuning parameters
//// LEFT for OFF
//// SHIFT for 10x tuning
//// ************************************************
//void Tune_Sp()
//{
////   lcd.setBacklight(TEAL);
////   lcd.print(F("Set Temperature:"));
////   uint8_t buttons = 0;
////   while(true)
////   {
////      buttons = ReadButtons();
////
////      float increment = 0.1;
////      if (buttons & BUTTON_SHIFT)
////      {
////        increment *= 10;
////      }
////      if (buttons & BUTTON_LEFT)
////      {
////         opState = RUN;
////         return;
////      }
////      if (buttons & BUTTON_RIGHT)
////      {
////         opState = TUNE_P;
////         return;
////      }
////      if (buttons & BUTTON_UP)
////      {
////         Setpoint += increment;
////         delay(200);
////      }
////      if (buttons & BUTTON_DOWN)
////      {
////         Setpoint -= increment;
////         delay(200);
////      }
////
////      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
////      {
////         opState = RUN;
////         return;
////      }
////      lcd.setCursor(0,1);
////      lcd.print(Setpoint);
////      lcd.print(" ");
////      DoControl();
////   }
//}
//
//// ************************************************
//// Proportional Tuning State
//// UP/DOWN to change Kp
//// RIGHT for Ki
//// LEFT for setpoint
//// SHIFT for 10x tuning
//// ************************************************
//void TuneP()
//{
////   lcd.setBacklight(TEAL);
////   lcd.print(F("Set Kp"));
////
////   uint8_t buttons = 0;
////   while(true)
////   {
////      buttons = ReadButtons();
////
////      float increment = 1.0;
////      if (buttons & BUTTON_SHIFT)
////      {
////        increment *= 10;
////      }
////      if (buttons & BUTTON_LEFT)
////      {
////         opState = SETP;
////         return;
////      }
////      if (buttons & BUTTON_RIGHT)
////      {
////         opState = TUNE_I;
////         return;
////      }
////      if (buttons & BUTTON_UP)
////      {
////         Kp += increment;
////         delay(200);
////      }
////      if (buttons & BUTTON_DOWN)
////      {
////         Kp -= increment;
////         delay(200);
////      }
////      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
////      {
////         opState = RUN;
////         return;
////      }
////      lcd.setCursor(0,1);
////      lcd.print(Kp);
////      lcd.print(" ");
////      DoControl();
////   }
//}
//
//// ************************************************
//// Integral Tuning State
//// UP/DOWN to change Ki
//// RIGHT for Kd
//// LEFT for Kp
//// SHIFT for 10x tuning
//// ************************************************
//void TuneI()
//{
////   lcd.setBacklight(TEAL);
////   lcd.print(F("Set Ki"));
////
////   uint8_t buttons = 0;
////   while(true)
////   {
////      buttons = ReadButtons();
////
////      float increment = 0.01;
////      if (buttons & BUTTON_SHIFT)
////      {
////        increment *= 10;
////      }
////      if (buttons & BUTTON_LEFT)
////      {
////         opState = TUNE_P;
////         return;
////      }
////      if (buttons & BUTTON_RIGHT)
////      {
////         opState = TUNE_D;
////         return;
////      }
////      if (buttons & BUTTON_UP)
////      {
////         Ki += increment;
////         delay(200);
////      }
////      if (buttons & BUTTON_DOWN)
////      {
////         Ki -= increment;
////         delay(200);
////      }
////      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
////      {
////         opState = RUN;
////         return;
////      }
////      lcd.setCursor(0,1);
////      lcd.print(Ki);
////      lcd.print(" ");
////      DoControl();
////   }
//}
//
//// ************************************************
//// Derivative Tuning State
//// UP/DOWN to change Kd
//// RIGHT for setpoint
//// LEFT for Ki
//// SHIFT for 10x tuning
//// ************************************************
//void TuneD()
//{
////   lcd.setBacklight(TEAL);
////   lcd.print(F("Set Kd"));
////
////   uint8_t buttons = 0;
////   while(true)
////   {
////      buttons = ReadButtons();
////      float increment = 0.01;
////      if (buttons & BUTTON_SHIFT)
////      {
////        increment *= 10;
////      }
////      if (buttons & BUTTON_LEFT)
////      {
////         opState = TUNE_I;
////         return;
////      }
////      if (buttons & BUTTON_RIGHT)
////      {
////         opState = RUN;
////         return;
////      }
////      if (buttons & BUTTON_UP)
////      {
////         Kd += increment;
////         delay(200);
////      }
////      if (buttons & BUTTON_DOWN)
////      {
////         Kd -= increment;
////         delay(200);
////      }
////      if ((millis() - lastInput) > 3000)  // return to RUN after 3 seconds idle
////      {
////         opState = RUN;
////         return;
////      }
////      lcd.setCursor(0,1);
////      lcd.print(Kd);
////      lcd.print(" ");
////      DoControl();
////   }
//}
//
//// ************************************************
//// PID COntrol State
//// SHIFT and RIGHT for autotune
//// RIGHT - Setpoint
//// LEFT - OFF
//// ************************************************
//void Run()
//{
////   // set up the LCD's number of rows and columns:
////   lcd.print(F("Sp: "));
////   lcd.print(Setpoint);
////   lcd.write(1);
////   lcd.print(F("C : "));
////
////   SaveParameters();
////   myPID.SetTunings(Kp,Ki,Kd);
////
////   uint8_t buttons = 0;
////   while(true)
////   {
////      setBacklight();  // set backlight based on state
////
////      buttons = ReadButtons();
////      if ((buttons & BUTTON_SHIFT)
////         && (buttons & BUTTON_RIGHT)
////         && (abs(Input - Setpoint) < 0.5))  // Should be at steady-state
////      {
////         StartAutoTune();
////      }
////      else if (buttons & BUTTON_RIGHT)
////      {
////        opState = SETP;
////        return;
////      }
////      else if (buttons & BUTTON_LEFT)
////      {
////        opState = OFF;
////        return;
////      }
////
////      DoControl();
////
////      lcd.setCursor(0,1);
////      lcd.print(Input);
////      lcd.write(1);
////      lcd.print(F("C : "));
////
////      float pct = map(Output, 0, WindowSize, 0, 1000);
////      lcd.setCursor(10,1);
////      lcd.print(F("      "));
////      lcd.setCursor(10,1);
////      lcd.print(pct/10);
////      //lcd.print(Output);
////      lcd.print("%");
////
////      lcd.setCursor(15,0);
////      if (tuning)
////      {
////        lcd.print("T");
////      }
////      else
////      {
////        lcd.print(" ");
////      }
////
////      // periodically log to serial port in csv format
////      if (millis() - lastLogTime > logInterval)
////      {
////        Serial.print(Input);
////        Serial.print(",");
////        Serial.println(Output);
////      }
////
////      delay(100);
////   }
//}
//
//// ************************************************
//// Execute the control loop
//// ************************************************
//void DoControl()
//{
////  // Read the input:
////  if (sensors.isConversionAvailable(0))
////  {
////    Input = sensors.getTempC(tempSensor);
////    sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
////  }
////
////  if (tuning) // run the auto-tuner
////  {
////     if (aTune.Runtime()) // returns 'true' when done
////     {
////        FinishAutoTune();
////     }
////  }
////  else // Execute control algorithm
////  {
////     myPID.Compute();
////  }
////
////  // Time Proportional relay state is updated regularly via timer interrupt.
////  onTime = Output;
//}
//
//// ************************************************
//// Called by ISR every 15ms to drive the output
//// ************************************************
//void DriveOutput()
//{
////  long now = millis();
////  // Set the output
////  // "on time" is proportional to the PID output
////  if(now - windowStartTime>WindowSize)
////  { //time to shift the Relay Window
////     windowStartTime += WindowSize;
////  }
////  if((onTime > 100) && (onTime > (now - windowStartTime)))
////  {
////     digitalWrite(RelayPin,HIGH);
////  }
////  else
////  {
////     digitalWrite(RelayPin,LOW);
////  }
//}
//
//// ************************************************
//// Start the Auto-Tuning cycle
//// ************************************************
//
//void StartAutoTune()
//{
//   // REmember the mode we were in
//   ATuneModeRemember = myPID.GetMode();
//
//   // set up the auto-tune parameters
//   aTune.SetNoiseBand(aTuneNoise);
//   aTune.SetOutputStep(aTuneStep);
//   aTune.SetLookbackSec((int)aTuneLookBack);
//   tuning = true;
//}
//
//// ************************************************
//// Return to normal control
//// ************************************************
//void FinishAutoTune()
//{
//   tuning = false;
//
//   // Extract the auto-tune calculated parameters
//   Kp = aTune.GetKp();
//   Ki = aTune.GetKi();
//   Kd = aTune.GetKd();
//
//   // Re-tune the PID and revert to normal control mode
//   myPID.SetTunings(Kp,Ki,Kd);
//   myPID.SetMode(ATuneModeRemember);
//
//   // Persist any changed parameters to EEPROM
//   SaveParameters();
//}
//
//
//// ************************************************
//// Save any parameter changes to EEPROM
//// ************************************************
//void SaveParameters()
//{
////   if (Setpoint != EEPROM_readDouble(SpAddress))
////   {
////      EEPROM_writeDouble(SpAddress, Setpoint);
////   }
////   if (Kp != EEPROM_readDouble(KpAddress))
////   {
////      EEPROM_writeDouble(KpAddress, Kp);
////   }
////   if (Ki != EEPROM_readDouble(KiAddress))
////   {
////      EEPROM_writeDouble(KiAddress, Ki);
////   }
////   if (Kd != EEPROM_readDouble(KdAddress))
////   {
////      EEPROM_writeDouble(KdAddress, Kd);
////   }
//}
//
//// ************************************************
//// Load parameters from EEPROM
//// ************************************************
//void LoadParameters()
//{
////  // Load from EEPROM
////   Setpoint = EEPROM_readDouble(SpAddress);
////   Kp = EEPROM_readDouble(KpAddress);
////   Ki = EEPROM_readDouble(KiAddress);
////   Kd = EEPROM_readDouble(KdAddress);
////
////   // Use defaults if EEPROM values are invalid
////   if (isnan(Setpoint))
////   {
////     Setpoint = 60;
////   }
////   if (isnan(Kp))
////   {
////     Kp = 850;
////   }
////   if (isnan(Ki))
////   {
////     Ki = 0.5;
////   }
////   if (isnan(Kd))
////   {
////     Kd = 0.1;
////   }
//}
//
////// ************************************************
////// Write floating point values to EEPROM
////// ************************************************
////void EEPROM_writeDouble(int address, double value)
////{
////   byte* p = (byte*)(void*)&value;
////   for (int i = 0; i < sizeof(value); i++)
////   {
////      EEPROM.write(address++, *p++);
////   }
////}
////
////// ************************************************
////// Read floating point values from EEPROM
////// ************************************************
////double EEPROM_readDouble(int address)
////{
////   double value = 0.0;
////   byte* p = (byte*)(void*)&value;
////   for (int i = 0; i < sizeof(value); i++)
////   {
////      *p++ = EEPROM.read(address++);
////   }
////   return value;
////}

#endif /* INCLUDE_SousVideController_H_ */
