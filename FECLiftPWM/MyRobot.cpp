#include "WPILib.h"
#include "math.h"
#include <stdio.h>
#include <iostream>
using namespace std;
#include <fstream>
#include <time.h>

#define WRITEFILE 1

#define JAGPORT 3
#define TICKS 250 
//ticks per revolution
#define MAXVOLT 12.0
//max output voltage
#define UNITSPERMETER (250.0/.19949)
//units for Set() per meter
#define ENCODERSLOTA 4
#define ENCODERSLOTB 4
#define ENCODERCHANNELA 1
#define ENCODERCHANNELB 2
//Encoder slots and channels
#define MAXDECEL -9.0
#define RATE 0
//Use rate PID method?
#if RATE
#define P 4.0
#define I 0.00
#define D 0.000
#else
#define P 0.5
#define I 0.00
#define D 0.05
#endif
//PID values

#define DISTANCE_PER_PULSE 1.0/UNITSPERMETER

class FECLift : public SimpleRobot{
public:
	Jaguar *theJag;
	DriverStation *driverStation;
	Encoder *fakeEncoder;
	Encoder *theEncoder;
	PIDController *theLift;
	
	ofstream myfile;
	
	
	bool decel;
	bool done;
	bool writing;
	bool closed;
	//bools for teleop
	
	double initTime;
	double vFinal;
	double xFinal;
	double decelRate; //m/s^2
	double rateSetpoint;
	Timer *theTimer;
	
	FECLift()
	{
		theTimer = new Timer();
		//initialized the jag
		theJag = new Jaguar(JAGPORT);
		fakeEncoder = new Encoder(6,6,6,7,fakeEncoder->k4X);
		theEncoder = new Encoder(ENCODERSLOTA, ENCODERCHANNELA, ENCODERSLOTB, ENCODERCHANNELB, false, theEncoder->k4X);
#if RATE
		theEncoder->SetPIDSourceParameter(Encoder::kRate);
#else
		theEncoder->SetPIDSourceParameter(Encoder::kDistance);
#endif
		
		theEncoder->SetDistancePerPulse(DISTANCE_PER_PULSE);
		driverStation = DriverStation::GetInstance();
		theLift = new PIDController(P,I,D,theEncoder,theJag);
		GetWatchdog().SetExpiration(.3);
		
		decel = false;
		done = false;
		writing = true;
		closed = false;
		initFile();
	}
	
	void Autonomous()
	{
	}
	
#define SETPOINT_METERS 2.0
	
	void initFile()
	{
		myfile.open("theData16.csv");
		myfile<<"Time,Position,Speed,\n";
	}
	
	void write()
	{
		myfile<<GetTime()<<","<<theEncoder->GetDistance()<<","<<theEncoder->GetRate()<<",\n";
	}
	
	void fileClose()
	{
		myfile.close();
	}
	
	void OperatorControl(void)
	{	
		//Open the file
		

		//setup watchdog	
		GetWatchdog().SetEnabled(true);
		GetWatchdog().SetExpiration(0.3);
		theEncoder->Start();
		//theLift->Enable();
#if RATE

#endif
		float maxHeight = 0;
		float maxRate = 0;

		while (IsOperatorControl())
		{	
			printf("Distance %f rate %f maxHeight %f maxRate %f\n", 
					theEncoder->GetDistance(),
					theEncoder->GetRate(),
					maxHeight,
					maxRate);
			if(writing)
			{
				write();
			}else if (!closed){
				printf("CLOSED!");
				myfile.close();
				closed = true;
			}
			
			Wait(.01);
			if(theEncoder->GetDistance() > maxHeight) maxHeight = theEncoder->GetDistance();
			if(theEncoder->GetRate() > maxRate) maxRate = theEncoder->GetRate();
			//check whether we should switch modes
			/*if (decelToStop(theEncoder->GetRate(),SETPOINT_METERS - theEncoder->GetDistance()) <= MAXDECEL && !decel)
			{
				decelRate = decelToStop(theEncoder->GetRate(),SETPOINT_METERS - theEncoder->GetDistance());
				cout<<"DecelRate: "<<decelRate<<endl;
				decel = true;
				printf("BEGINNING DECELERATION\n");
				initTime = GetTime();
			}*/
			
			/*if(theEncoder->GetDistance() > 1.0)
			{
				theJag->Set(0.0);
				writing = false;
				//theLift->SetPID(.3,0,0);
				//theLift->SetSetpoint(0.0);
				done = true;
			}else if(!done){
				theJag->Set(1.0);
				//theLift->SetSetpoint(1.5);
			}
			*/
			if(theEncoder->GetDistance() >= SETPOINT_METERS)
			{
				done = true;
				theJag->Set(0.0);
				writing = false;
			}else if (!done){
				theJag->Set(1.0);
			}
			rateSetpoint = vFinal + (decelRate * (GetTime() / pow(10,9)));
#if 0
			if(theEncoder->GetDistance() >= SETPOINT_METERS)
			{
				theJag->Set(0.0);
				writing = false;
			}else if(decel){
#if RATE
				//theLift->SetSetpoint(rateSetpoint);
				theJag->Set(0.0);
#else
				theLift->SetSetpoint(SETPOINT_METERS);
#endif
			}else{
				theJag->Set(1.0);
			}
#if RATE
			
#else
			theLift->SetSetpoint(SETPOINT_METERS);
#endif
#endif
			GetWatchdog().Feed();
		}
	}
	
	void Disabled()
	{
		printf("AAAAAAAAAAAAAAAAAAAAAAAA");
		decel = false;
		done = false;
		theLift->Reset();
	}

	//function to figure out what the required acceleration to stop at the end is assuming constant acceleration
	float decelToStop(float vZero, float distanceRemaining)
	{
		return (-pow(vZero,2))/(2 * distanceRemaining);
	}
};

START_ROBOT_CLASS(FECLift);

