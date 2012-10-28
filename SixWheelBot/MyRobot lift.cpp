#if SPROCKET22

#include "WPILib.h"
#include "math.h"
#include "dashboarddataformat.h"
#include "1983PIDController.h"
#include "TankPIDOutput.cpp"
#include "1983Lift.cpp"
#include "1983Defines.h"

#if COMPBOT
class EncoderPIDSource : public PIDSource
{
public:
	EncoderPIDSource(Encoder *pSensor, float *maxSpeed) 
	{
		m_encoder = pSensor;
		m_maxSpeed = maxSpeed;
		m_ticks = m_encoder->Get();
		m_time = GetClock();
		m_nCount = 0;
	}
	
	double PIDGet() 
	{
		return -m_encoder->GetRate()/ *m_maxSpeed;	
	}
	
private:
	Encoder	*m_encoder;
	double m_time;
	float *m_maxSpeed;
	int m_ticks;
	static int m_nCount;
};
int EncoderPIDSource::m_nCount=0;

class SixWheelBot : public SimpleRobot
{
	//RobotDrive myRobot; // robot drive system
	Jaguar *frontLeftMotor;
	Jaguar *rearLeftMotor;
	Jaguar *frontRightMotor;
	Jaguar *rearRightMotor;
	Jaguar *liftMotor1;
	Jaguar *liftMotor2;
	Victor *gripMotor1;
	//Victor *gripMotor2;
	Servo *camGimble;
	
	Joystick stickL; //Port 1
	Joystick stickR; //Port 2
	AxisCamera &camera;
	
	DigitalInput *lsLeft;
	DigitalInput *lsMiddle;
	DigitalInput *lsRight;
	DigitalInput *compSwitch; //compressor limit switch
	AnalogChannel *ultrasonic;
	/*DigitalInput *bcd1;
	DigitalInput *bcd2;
	DigitalInput *bcd3;*/
	
	Encoder *fakeEncoderA;
	Encoder *leftEncoder;
	//Encoder *fakeEncoderB;
	Encoder *liftEncoder;
	Encoder *rightEncoder;
	//Encoder *fakeEncoderC;
	//Encoder *liftEncoder;
	AnalogChannel *gripPot;
	
	Relay *light;
	Relay *brake;
	Relay *shifter;
	Solenoid *gripOpen1;
	Solenoid *gripOpen2;
	Solenoid *gripPop1;
	Solenoid *gripPop2;
	Solenoid *gripLatch1;
	Solenoid *gripLatch2;
	Relay *miniDep;
	Relay *compressor;
	
	DriverStation *driverStation;
	C1983_Lift *lift;
	
	PIDController *PIDDriveLeft;
	PIDController *PIDDriveRight;
	PIDController *PIDLift;
	PIDController *PIDGrip;
	
	EncoderPIDSource *leftPIDSource;
	EncoderPIDSource *rightPIDSource;
	LiftPIDSource *liftPIDSource;
	GripPIDSource *gripPIDSource;
	
	TankPIDOutput *leftPIDOutput;
	TankPIDOutput *rightPIDOutput;
	LiftPIDOutput *liftPIDOutput;
	GripPIDOutput *gripPIDOutput;
	
	float maxSpeed;
	bool lineFollowDone; //autonomous things
	bool goPop;
	bool leftValue;
	bool middleValue;
	bool rightValue;
	int stopCount;
	int counts;
	bool autonomous;
	

public:
	SixWheelBot(void):
		//myRobot(1, 2, 3, 4),	// these must be initialized in the same order
		stickL(1), stickR(2), camera()		// as they are declared above.
	{
		//myRobot.SetExpiration(0.1);
		frontLeftMotor = new Jaguar(FRONTLEFTMOTORPORT);
		rearLeftMotor = new Jaguar(REARLEFTMOTORPORT);
		frontRightMotor = new Jaguar(FRONTRIGHTMOTORPORT);
		rearRightMotor = new Jaguar(REARRIGHTMOTORPORT);
		liftMotor1 = new Jaguar(LIFTMOTORPORT1);
		liftMotor2 = new Jaguar(LIFTMOTORPORT2);
		gripMotor1 = new Victor(GRIPMOTORPORT1);
		//gripMotor2 = new Victor(GRIPMOTORPORT2);
		
		fakeEncoderA = new Encoder(6,1,6,2,false, fakeEncoderA->k4X);
		leftEncoder = new Encoder(4, LEFTMOTORA, 4, LEFTMOTORB, true, leftEncoder->k4X);
		//fakeEncoderB = new Encoder(6,3,6,4,false, fakeEncoderB->k1X);
		liftEncoder = new Encoder(4, LIFTMOTORA, 4, LIFTMOTORB, false, liftEncoder->k4X);
		rightEncoder = new Encoder(4, RIGHTMOTORA, 4, RIGHTMOTORB, false, rightEncoder->k4X);
		//fakeEncoderC = new Encoder(6,5,6,6,false, fakeEncoderC->k1X);
		//liftEncoder = new Encoder(4, LIFTMOTORA, 4, LIFTMOTORB, false, liftEncoder->k4X);
		gripPot = new AnalogChannel(1, GRIPPOTPORT);
		compSwitch = new DigitalInput(4, COMPRESSORSWITCH);
		lsRight = new DigitalInput(3); //light sensors
		lsMiddle = new DigitalInput(2);
		lsLeft = new DigitalInput(1);
		
		light = new Relay(4, LIGHTPORT, light->kForwardOnly);
		//light = new Relay(6, 8);
		brake = new Relay(4, BRAKEPORT, brake->kBothDirections);
		shifter = new Relay(4, SHIFTERPORT, shifter->kBothDirections);
		miniDep = new Relay(4, MINIBOTPORT);
		miniDep->SetDirection(miniDep->kBothDirections);
		gripOpen1 = new Solenoid(8, GRIPOPENPORT1);
		gripOpen2 = new Solenoid(8, GRIPOPENPORT2);
		gripPop1 = new Solenoid(8, GRIPPOPPORT1);
		gripPop2 = new Solenoid(8, GRIPPOPPORT2);
		gripLatch1 = new Solenoid(8, GRIPLATCH1);
		gripLatch2 = new Solenoid(8, GRIPLATCH2);
		compressor = new Relay(4, COMPRESSORPORT, compressor->kBothDirections);
		
		driverStation = DriverStation::GetInstance();
		lift = new C1983_Lift(liftEncoder, brake);
		
		ultrasonic = new AnalogChannel(1, ULTRASOUNDPORT);
		camGimble = new Servo(CAMERAYSERVOPORT);
		maxSpeed = 500;
		
		leftPIDSource = new EncoderPIDSource(leftEncoder, &maxSpeed);
		rightPIDSource = new EncoderPIDSource(rightEncoder, &maxSpeed);
		liftPIDSource = new LiftPIDSource(lift);
		gripPIDSource = new GripPIDSource(gripPot);
		
		leftPIDOutput = new TankPIDOutput(frontLeftMotor, rearLeftMotor, false);
		rightPIDOutput = new TankPIDOutput(frontRightMotor, rearRightMotor, true);
		liftPIDOutput = new LiftPIDOutput(liftMotor1, liftMotor2);
		//gripPIDOutput = new GripPIDOutput(gripMotor1, gripMotor2);
		gripPIDOutput = new GripPIDOutput(gripMotor1);
		
		PIDDriveLeft = new PIDController(DRIVEPROPGAIN, DRIVEINTGAIN, DRIVEDERIVGAIN,
						leftPIDSource, leftPIDOutput);
		PIDDriveRight = new PIDController(DRIVEPROPGAIN, DRIVEINTGAIN, DRIVEDERIVGAIN,
				rightPIDSource, rightPIDOutput);
		PIDLift = new PIDController(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN,
				liftPIDSource, liftPIDOutput);
		PIDGrip = new PIDController(GRIPPROPGAIN, GRIPINTGAIN, GRIPDERIVGAIN,
				gripPIDSource, gripPIDOutput);

		/*bcd1 = new DigitalInput(4);
		bcd2 = new DigitalInput(5);
		bcd3 = new DigitalInput(6);*/
	}


	void followLine()
	{
		float turnMod;
		if(leftValue && middleValue && rightValue && !lineFollowDone){
			stopCount++;
			if(/*getDistance() < 87 &&*/ stopCount >= 10)
			{
				frontLeftMotor->Set(0.0);
				rearLeftMotor->Set(0.0);
				frontRightMotor->Set(0.0);
				rearRightMotor->Set(0.0);
				lineFollowDone = true;
				printf("LINE FOLLOW IS DONE!!!!!!!!!!1\n");
			}
		}else if(lineFollowDone){
			frontLeftMotor->Set(0.0);
			rearLeftMotor->Set(0.0);
			frontRightMotor->Set(0.0);
			rearRightMotor->Set(0.0);
		}else{
			stopCount = 0;
			if (middleValue){
				turnMod = 0;
			}
			else if (leftValue < rightValue)
			{
				turnMod = -.1;
			}
			else if (leftValue > rightValue)
			{
				turnMod = .1;
			}
			else
			{
				turnMod = 0;
			}
			//myRobot.SetLeftRightMotorOutputs(.5 - turnMod, .5 + turnMod);
			frontLeftMotor->Set(-.4 + turnMod);
			rearLeftMotor->Set(-.4 + turnMod);
			frontRightMotor->Set(.4 + turnMod);
			rearRightMotor->Set(.4 + turnMod);
		}
	}
	
	//const double fUltrasonicCalibration = 503.0/5.0;
	//const double fUltrasonicVoltageBaseline = 4.625;
	
	//Returns distance in inches
	float getDistance()
	{
		return ultrasonic->GetVoltage()*(503.0/5.0);
	}
	
	/**
	 * Drive left & right motors for 2 seconds then stop
	 */

	void Autonomous(void)
	{
#if 1
		/*int autoMode = 0;
		autoMode |= bcd1->Get();
		autoMode <<= 1;
		autoMode |= bcd2->Get();
		autoMode <<= 1;
		autoMode |= bcd3->Get()
		;*/
		//double ignoreLineStart = 0;
		GetWatchdog().SetEnabled(true);
		GetWatchdog().SetExpiration(0.2);		
		float liftSP = 0;
		PIDLift->SetTolerance(10);
		PIDLift->SetContinuous(false);
		PIDLift->SetOutputRange(-0.5, 0.75); //BUGBUG half value
		PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN);
		PIDLift->Enable();
		PIDGrip->SetSetpoint(0);
		PIDGrip->Enable();
		stopCount = 0;

		float reduction;
		int counts = 0;
		leftEncoder->Start();
		rightEncoder->Start();
		leftEncoder->Reset();
		rightEncoder->Reset();
		liftEncoder->Start();
		liftEncoder->Reset();
		leftEncoder->SetDistancePerPulse((6 * PI / 500)/reduction);
		rightEncoder->SetDistancePerPulse((6 * PI / 500)/reduction);
		double avgEncoderCount;
		float leftSpeed = .2, rightSpeed = .2;
		short int lsL,lsM,lsR;
		lineFollowDone = false;
		counts = 0;
		//int fancyWaiter = 0;
		int popstage = 0;
		goPop = false;
		double backStart = 0;
		double backTime = 0;
		double popStart = 0;
		double popTime = 0;
		bool backDone = false;
		miniDep->Set(miniDep->kForward);
		
		int liftCount = 0;
		bool disengageBrake = false;
		float lastLiftSP = 0;
		
		gripOpen1->Set(true);
		gripOpen2->Set(false);
		
		gripLatch1->Set(true);
		gripLatch2->Set(false);
		
		
		while(IsAutonomous())
		{
			if(!(counts % 100))printf("%2.2f \n",getDistance());
			if(backStart) backTime = GetClock();
			if(popStart) popTime = GetClock();
			
			//if(!ignoreLineStart)ignoreLineStart = GetClock();
			
			if(!compSwitch->Get()) compressor->Set(compressor->kReverse);
			else compressor->Set(compressor->kOff);
			
			if(counts%3 == 0)
			{
				leftValue = lsLeft->Get();
				middleValue = lsMiddle->Get();
				rightValue = lsRight->Get();
			}
			counts++;
			GetWatchdog().Feed();
			//avgEncoderCount = (leftEncoder->Get() + rightEncoder->Get())/2;
			//myRobot.SetLeftRightMotorOutputs(.2,.2);
			
			//All three/five autonomous programs will do the same thing up until 87 inches from the wall
			
			if (counts % 100 == 0){//TESTING
				if(lsLeft->Get()){
					lsL = 1;
				}else{
					lsL = 0;
				}
				if(lsRight->Get()){
					lsR = 1;
				}else{
					lsR = 0;
				}
				if(lsMiddle->Get()){
					lsM = 1;
				}else{
					lsM = 0;
				}
				//printf("Encoder: %2.2f \n", (float)avgEncoderCount);
				//printf("Distance: %2.2f SensorL:%1d SensorM:%1d SensorR:%1d\n",getDistance(),lsL,lsM,lsR);//TESTING
			}
			
#if FOLLOWLINE
			/*if(GetClock() - ignoreLineStart <= 0.5)
			{
				frontLeftMotor->Set(-.4);
				rearLeftMotor->Set(-.4);
				frontRightMotor->Set(.4);
				rearRightMotor->Set(.4);
			}
			else */if (false){//(avgEncoderCount <= SECONDBREAKDISTANCE){
				followLine();
			}
#else
			if (getDistance() > 24){
				frontLeftMotor->Set(-leftSpeed);
				rearLeftMotor->Set(-leftSpeed);
				frontRightMotor->Set(rightSpeed);
				rearRightMotor->Set(rightSpeed);
				if(leftEncoder->Get() > rightEncoder->Get() && leftSpeed == .2){
					rightSpeed += .03;
				}else if(leftEncoder->Get() >rightEncoder->Get() && leftSpeed > .2){
					leftSpeed -= .03;
				}else if(leftEncoder->Get() < rightEncoder->Get() && rightSpeed == .2){
					leftSpeed += .03;
				}else if(leftEncoder->Get() < rightEncoder->Get() && rightSpeed > .2){
					rightSpeed -= .03;
				}
			}
#endif
			else{
				if(counts % 100 == 0) {printf("DISTANCE: %2.2f\n",getDistance());}
				switch(FOLLOWLINE)
				{
				case STRAIGHTLINEMODE: //Straight line. Scores on middle column of left or right grid.
					//if(lineFollowDone && !(counts %50)) printf("Lift error: %f \n", PIDLift->GetError());
					lastLiftSP = liftSP;
					
					if(!lineFollowDone)
					{
						followLine();
					}
					else if(!PIDLift->GetSetpoint() && !popstage && !backStart)
					{
						//if(counts % 50 == 0)printf("Go backward\n");
						frontLeftMotor->Set(.3);
						rearLeftMotor->Set(.3);
						frontRightMotor->Set(-.3);
						rearRightMotor->Set(-.3);
						//PIDLift->SetSetpoint(LIFTMID2);
						liftSP = LIFTHIGH2 + 0.025;
						//fancyWaiter = counts;
						backStart = GetClock();
						printf("Setpoint set setpoint set setpoint set \n");
						/*
						if(leftValue && middleValue && rightValue)
						{
							printf("Stopped 2nd time\n");
							goPop = true;
							frontLeftMotor->Set(0);
							rearLeftMotor->Set(0);
							frontRightMotor->Set(0);
							rearRightMotor->Set(0);
							PIDLift->SetSetpoint(LIFTHIGH2);
						}
						*/
					}
#if 1				//if we've backed up for half a second and we're not popping
					else if((backTime - backStart) > 0.65 && !backDone)
					{
						printf("Stopping!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ONE\n");
						frontLeftMotor->Set(0);
						rearLeftMotor->Set(0);
						frontRightMotor->Set(0);
						rearRightMotor->Set(0);
						//PIDLift->SetSetpoint(LIFTMID2);
						liftSP = LIFTHIGH2;
						backDone = true;
						//Wait(.01);
						//lift->brakeOff();
						//fancyWaiter = counts;
						//printf("Fancy waiter set Fancy waiter set Fancy waiter set");
					}
					/*
					else if(lift->getPosition() < LIFTHIGH2)
					{
						//Move teh lifts
						//if(counts % 100 == 0) printf("Stopping because lineFollowDone == true\n");
						PIDLift->SetSetpoint(LIFTHIGH2);
					}
					*/
					//if the lift is at the top and we're done backing up
					else if(PIDLift->GetSetpoint() && fabs(liftSP - lift->getPosition()) < 0.015 && backDone)
					{
						if(!popStart) popStart = GetClock();
						if(popstage == 0)
						{
							//lift->brakeOn();
							//PIDLift->SetSetpoint(lift->getPosition());
							popstage++;
							printf("BRAKE BRAKE BRAKE BRAKE BRAKE \n");
						}
						else if(popstage == 1 && popTime - popStart > 0.1)
						{
#if !(INNERGRIP)
							gripOpen1->Set(true);
							gripOpen2->Set(false);
#else
							gripOpen1->Set(false);
							gripOpen2->Set(true);
#endif
							popstage++;
							printf("OPEN OPEN OPEN OPEN OPEN \n");
						}
						else if(popstage == 2 && popTime - popStart > 0.35)
						{
							gripPop1->Set(true);
							gripPop2->Set(false);
							popstage++;
							printf("POP POP POP POP POP POP POP \n");
						}
						else if(popstage == 3 && popTime - popStart > 1.35)
						{
							gripPop1->Set(false);
							gripPop2->Set(true);
							
							frontLeftMotor->Set(.2);
							rearLeftMotor->Set(.2);
							frontRightMotor->Set(-.2);
							rearRightMotor->Set(-.2);
							
							popstage++;
							printf("UNPOP UNPOP UNPOP UNPOP UNPOP \n");
						}
						else if(popstage == 4 && popTime - popStart > 1.85)
						{
							printf("DOWN DOWN DOWN DOWN DOWN DOWN \n");
							frontLeftMotor->Set(0);
							rearLeftMotor->Set(0);
							frontRightMotor->Set(0);
							rearRightMotor->Set(0);
							//PIDLift->SetSetpoint(0);
							liftSP = 0;
						}
					}
					
					//Start tele-op lift code
					if(!lift->isBraking() && !disengageBrake)
					{
						PIDLift->SetSetpoint(liftSP);
						if(liftSP == 0 && liftPIDSource->PIDGet() < 0.1)
						{
							//PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, 3*LIFTDERIVGAIN);
							PIDLift->SetOutputRange(-liftPIDSource->PIDGet() - 0.1, 1);
						}
						else PIDLift->SetOutputRange(-0.75, 1);
					}
					if(lift->isBraking() && lastLiftSP != liftSP)
					{
						PIDLift->SetSetpoint(lift->getPosition() + 0.04);
						PIDLift->SetPID(10.5 + 2*lift->getPosition(), LIFTINTGAIN + 0.4 + 3*lift->getPosition()/10, 0);
						lift->brakeOff();
						disengageBrake = true;
						if(!liftCount) liftCount = counts;
					}
					//set brake (because near)
					if(fabs(PIDLift->GetError()) < 0.015 && !lift->isBraking() && !disengageBrake)
					{
						if(liftCount == 0) liftCount = counts;
						if(counts - liftCount > 1000)
						{
							PIDLift->Reset();
							liftMotor1->Set(LIFTNEUTRALPOWER);
							liftMotor2->Set(LIFTNEUTRALPOWER);
							Wait(0.02);
							lift->brakeOn();
							Wait(0.02);
							liftMotor1->Set(0.0);
							liftMotor2->Set(0.0);
							PIDLift->Enable();
							//PIDLift->SetSetpoint(lift->getPosition());
							liftCount = 0;
						}
						if(counts%3000) printf("Braking/Not PID \n");
					}
					if(lift->isBraking() && !(counts%100000)) PIDLift->SetSetpoint(lift->getPosition());
					if(fabs(PIDLift->GetError()) < 0.01 && disengageBrake && counts - liftCount > 1000)
					{
						disengageBrake = false;
						PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN);
						liftCount = 0;
					}
					//End tele-op lift code
#endif
					//myRobot.SetLeftRightMotorOutputs(0,0);
					break;
				case NOLINEMODE: //Fork turn left. Scores on far right column of left grid.
					lineFollowDone = true;
					if(!lineFollowDone){}
					else if(!PIDLift->GetSetpoint() && !popstage && !backStart)
					{
					//if(counts % 50 == 0)printf("Go backward\n");
						frontLeftMotor->Set(.3);
						rearLeftMotor->Set(.3);
						frontRightMotor->Set(-.3);
						rearRightMotor->Set(-.3);
						//PIDLift->SetSetpoint(LIFTMID2);
						liftSP = LIFTHIGH2;
						//fancyWaiter = counts;
						backStart = GetClock();
						printf("Setpoint set setpoint set setpoint set \n");
						/*
						if(leftValue && middleValue && rightValue)
						{
							printf("Stopped 2nd time\n");
							goPop = true;
							frontLeftMotor->Set(0);
							rearLeftMotor->Set(0);
							frontRightMotor->Set(0);
							rearRightMotor->Set(0);
							PIDLift->SetSetpoint(LIFTHIGH2);
						}
						*/
					}
#if 1				//if we've backed up for half a second and we're not popping
					else if((backTime - backStart) > 0.65 && !backDone)
					{
						printf("Stopping!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ONE\n");
						frontLeftMotor->Set(0);
						rearLeftMotor->Set(0);
						frontRightMotor->Set(0);
						rearRightMotor->Set(0);
						//PIDLift->SetSetpoint(LIFTMID2);
						liftSP = LIFTHIGH2;
						backDone = true;
						//Wait(.01);
						//lift->brakeOff();
						//fancyWaiter = counts;
						//printf("Fancy waiter set Fancy waiter set Fancy waiter set");
					}
					/*
					else if(lift->getPosition() < LIFTHIGH2)
					{
						//Move teh lifts
						//if(counts % 100 == 0) printf("Stopping because lineFollowDone == true\n");
						PIDLift->SetSetpoint(LIFTHIGH2);
					}
					*/
					//if the lift is at the top and we're done backing up
					else if(PIDLift->GetSetpoint() && fabs(liftSP - lift->getPosition()) < 0.015 && backDone)
					{
						if(!popStart) popStart = GetClock();
						if(popstage == 0)
						{
							//lift->brakeOn();
							//PIDLift->SetSetpoint(lift->getPosition());
							popstage++;
							printf("BRAKE BRAKE BRAKE BRAKE BRAKE \n");
						}
						else if(popstage == 1 && popTime - popStart > 0.1)
						{
#if !(INNERGRIP)
							gripOpen1->Set(true);
							gripOpen2->Set(false);
#else
							gripOpen1->Set(false);
							gripOpen2->Set(true);
#endif
							popstage++;
							printf("OPEN OPEN OPEN OPEN OPEN \n");
						}
						else if(popstage == 2 && popTime - popStart > 0.35)
						{
							gripPop1->Set(true);
							gripPop2->Set(false);
							popstage++;
							printf("POP POP POP POP POP POP POP \n");
						}
						else if(popstage == 3 && popTime - popStart > 1.35)
						{
							gripPop1->Set(false);
							gripPop2->Set(true);
							
							frontLeftMotor->Set(.2);
							rearLeftMotor->Set(.2);
							frontRightMotor->Set(-.2);
							rearRightMotor->Set(-.2);
							
							popstage++;
							printf("UNPOP UNPOP UNPOP UNPOP UNPOP \n");
						}
						else if(popstage == 4 && popTime - popStart > 1.85)
						{
							printf("DOWN DOWN DOWN DOWN DOWN DOWN \n");
							frontLeftMotor->Set(0);
							rearLeftMotor->Set(0);
							frontRightMotor->Set(0);
							rearRightMotor->Set(0);
							//PIDLift->SetSetpoint(0);
							liftSP = 0;
						}
					}
					
					//Start tele-op lift code
					if(!lift->isBraking() && !disengageBrake)
					{
						PIDLift->SetSetpoint(liftSP);
						if(liftSP == 0 && liftPIDSource->PIDGet() < 0.25) //BUGBUG double value
						{
							//PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, 3*LIFTDERIVGAIN);
							PIDLift->SetOutputRange(-liftPIDSource->PIDGet() - 0.075, 0.75); //BUGBUG
						}
						else PIDLift->SetOutputRange(-0.3, 0.0.75); //BUGBUG half value
					}
					if(lift->isBraking() && lastLiftSP != liftSP)
					{
						PIDLift->SetSetpoint(lift->getPosition() + 0.04);
						PIDLift->SetPID(10.5 + 2*lift->getPosition(), LIFTINTGAIN + 0.4 + 3*lift->getPosition()/10, 0);
						lift->brakeOff();
						disengageBrake = true;
						if(!liftCount) liftCount = counts;
					}
					//set brake (because near)
					if(fabs(PIDLift->GetError()) < 0.015 && !lift->isBraking() && !disengageBrake)
					{
						if(liftCount == 0) liftCount = counts;
						if(counts - liftCount > 1000)
						{
							PIDLift->Reset();
							liftMotor1->Set(LIFTNEUTRALPOWER);
							liftMotor2->Set(LIFTNEUTRALPOWER);
							Wait(0.02);
							lift->brakeOn();
							Wait(0.02);
							liftMotor1->Set(0.0);
							liftMotor2->Set(0.0);
							PIDLift->Enable();
							//PIDLift->SetSetpoint(lift->getPosition());
							liftCount = 0;
						}
						if(counts%3000) printf("Braking/Not PID \n");
					}
					if(lift->isBraking() && !(counts%100000)) PIDLift->SetSetpoint(lift->getPosition());
					if(fabs(PIDLift->GetError()) < 0.01 && disengageBrake && counts - liftCount > 1000)
					{
						disengageBrake = false;
						PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN);
						liftCount = 0;
					}
					//End tele-op lift code
#endif
					//myRobot.SetLeftRightMotorOutputs(0,0);
					break;
				case FORKRIGHTMODE://Fork turn right. Scores on far left column of right grid.
					if(leftEncoder->GetDistance() <= rightEncoder->GetDistance() + 6)
					{
						frontLeftMotor->Set(.2);
						rearLeftMotor->Set(.2);
						frontRightMotor->Set(0);
						rearRightMotor->Set(0);
					}
					else if(getDistance() >= SCOREDISTANCE) 
					{
						followLine();
					}
					//score here				
					//myRobot.SetLeftRightMotorOutputs(0,0);
					break;
				}
			}
		}
		/*frontRightMotor->Set(0);
		rearRightMotor->Set(0);
		frontLeftMotor->Set(0);
		rearLeftMotor->Set(0);*/
		Wait(.02);
#endif
	}
	
	void Disabled(void)
	{
		bool stopped = false;
		while(!IsOperatorControl() && !IsAutonomous())
		{
			if(!stopped)
			{
				frontLeftMotor->Set(0.0);
				rearLeftMotor->Set(0.0);
				frontRightMotor->Set(0.0);
				rearRightMotor->Set(0.0);
				liftMotor1->Set(0.0);
				liftMotor2->Set(0.0);
				gripMotor1->Set(0.0);
				//gripMotor2->Set(0.0);
				PIDLift->Reset();
				PIDDriveLeft->Reset();
				PIDDriveRight->Reset();
				PIDGrip->Reset();
				stopped = true;
			}
		}
	}

	void OperatorControl(void)
	{
		autonomous = false;
		//myRobot.SetSafetyEnabled(false);
		//myRobot.SetInvertedMotor(kFrontLeftMotor, true);
		//	myRobot.SetInvertedMotor(3, true);
		//variables for great pid
		double rightSpeed,leftSpeed;
		float rightSP, leftSP, liftSP, lastLiftSP, gripSP, tempLeftSP, tempRightSP;
		float stickY[2];
		float stickYAbs[2];
		bool lightOn = false;
		AxisCamera &camera = AxisCamera::GetInstance();
		camera.WriteResolution(AxisCameraParams::kResolution_160x120);
		camera.WriteBrightness(50);
		camera.WriteRotation(AxisCameraParams::kRotation_0);
		rightEncoder->Start();
		leftEncoder->Start();
		liftEncoder->Start();
		rightEncoder->Reset();
		leftEncoder->Reset();
		liftEncoder->Reset();
		bool fastest = false; //transmission mode
		float reduction = 1; //gear reduction from 
		bool bDrivePID = false;
		//float maxSpeed = 500;
		float liftPower = 0;
		float liftPos = -10;
		bool disengageBrake = false;
		int count = 0;
		//int popCount = 0;
		double popStart = 0;
		double popTime = 0;
		int popStage = 0;
		int liftCount = 0;
		int liftCount2 = 0;
		const float LOG17 = log(17.61093344);
		float liftPowerAbs = 0;
		float gripError = 0;
		float gripErrorAbs = 0;
		float gripPropMod = 0;
		float gripIntMod = 0;
		bool shiftHigh = false;
		leftEncoder->SetDistancePerPulse((6 * PI / 1000)/reduction); //6-inch wheels, 1000 raw counts per revolution,
		rightEncoder->SetDistancePerPulse((6 * PI / 1000)/reduction); //about 1:1 gear ratio
		DriverStationEnhancedIO &myEIO = driverStation->GetEnhancedIO();
		GetWatchdog().SetEnabled(true);
		GetWatchdog().SetExpiration(0.3);

		PIDDriveLeft->SetOutputRange(-1, 1);
		PIDDriveRight->SetOutputRange(-1, 1);
		//PIDDriveLeft->SetInputRange(-244,244);
		//PIDDriveRight->SetInputRange(-244,244);
		PIDDriveLeft->SetTolerance(5);
		PIDDriveRight->SetTolerance(5);
		PIDDriveLeft->SetContinuous(false);
		PIDDriveRight->SetContinuous(false);
		//PIDDriveLeft->Enable();
		//PIDDriveRight->Enable();
		PIDDriveRight->SetPID(DRIVEPROPGAIN, DRIVEINTGAIN, DRIVEDERIVGAIN);
		PIDDriveLeft->SetPID(DRIVEPROPGAIN, DRIVEINTGAIN, DRIVEDERIVGAIN);
		
		liftSP = 0;
		PIDLift->SetTolerance(10);
		PIDLift->SetContinuous(false);
		PIDLift->SetOutputRange(-0.5, .75); //BUGBUG half value
		PIDLift->Enable();
		
		gripSP = 0;
		float goalGripSP = 0;
		bool useGoalSP = true;
		bool gripPIDOn = true;
		float gripP[10];
		float gripI[10];
		float gripD[10];
		PIDGrip->SetOutputRange(-0.5, 0.28); //Negative goes up
		PIDGrip->SetTolerance(5);
		PIDGrip->SetSetpoint(0);
		PIDGrip->Enable();
		miniDep->Set(miniDep->kForward);
		int i = 0;
		while(i < 10)
		{
			gripP[i] = GRIPPROPGAIN;
			i++;
		}
		i = 0;
		while(i < 10)
		{
			gripI[i] = GRIPINTGAIN;
			i++;
		}
		i = 0;
		while(i < 10)
		{
			gripD[i] = GRIPDERIVGAIN;
			i++;
		}

		while (IsOperatorControl())
		{
			GetWatchdog().Feed();
			count++;
			sendVisionData();
			/*
			if(LIFTLOW1BUTTON && !(counts%10)) printf("LIFTLOW1BUTTON\n");
			if(LIFTLOW2BUTTON && !(counts%10)) printf("LIFTLOW2BUTTON\n");
			if(LIFTMID1BUTTON && !(counts%10)) printf("LIFTMID1BUTTON\n");
			if(LIFTMID2BUTTON && !(counts%10)) printf("LIFTMID2BUTTON\n");
			if(LIFTHIGH1BUTTON && !(counts%10)) printf("LIFTHIGH1BUTTON\n");
			if(LIFTHIGH2BUTTON && !(counts%10)) printf("LIFTHIGH2BUTTON\n");
			*/
			/*
			if(lsLeft->Get()) printf("LSLEFT\n");
			if(lsMiddle->Get()) printf("LSMIDDLE\n");
			if(lsRight->Get()) printf("LSRIGHT\n");
			*/
			stickY[0] = stickL.GetY();
			stickY[1] = stickR.GetY();
			stickYAbs[0] = fabs(stickY[0]);
			stickYAbs[1] = fabs(stickY[1]);
			if(bDrivePID)
			{
	#if 0
				frontLeftMotor->Set(stickY[0]);
				rearLeftMotor->Set(stickY[0]);
				frontRightMotor->Set(stickY[1]);
				rearRightMotor->Set(stickY[1]);
				
				if(!(count%5)) printf("Speeds: %4.2f %4.2f Outputs: %f %f \n", leftEncoder->GetRate(),
						rightEncoder->GetRate(), frontLeftMotor->Get(), frontRightMotor->Get());
	#endif		
				if(stickYAbs[0] <= 0.05 )
				{
					leftSP = 0;
					if(!(count%3) && !BACKWARDBUTTON)
					{
						PIDDriveLeft->Reset();
						PIDDriveLeft->Enable();
					}
				}
				else leftSP = stickY[0] * stickY[0] * (stickY[0]/stickYAbs[0]); //set points for pid control
				if(stickYAbs[1] <= 0.05)
				{
					rightSP = 0;
					if(!(count%3) && !BACKWARDBUTTON)
					{
						PIDDriveRight->Reset();
						PIDDriveRight->Enable();
					}
				}
				else rightSP = stickY[1] * stickY[1] * (stickY[1]/stickYAbs[1]);
				
				if (BACKWARDBUTTON)
				{
					tempRightSP = rightSP;
					tempLeftSP = leftSP;
					rightSP = -tempLeftSP;
					leftSP = -tempRightSP; //This line and above line sets opposite values for the controller. ...Theoretically.
				}
				
				PIDDriveLeft->SetSetpoint(leftSP);
				PIDDriveRight->SetSetpoint(rightSP);
					
				
				leftSpeed = leftEncoder->GetRate();
				rightSpeed = rightEncoder->GetRate();
				if(!(count++ % 5))
				{
				printf("rate L: %2.2f R: %2.2f SP %2.4f %2.4f ERR %2.2f %2.2f Pow: %1.2f %1.2f\n", 
						leftPIDSource->PIDGet(), rightPIDSource->PIDGet(), leftSP, rightSP,
						PIDDriveLeft->GetError(), PIDDriveRight->GetError(), frontLeftMotor->Get(),
						frontRightMotor->Get());
						
				
				//printf("Throttle value: %f", stickR.GetThrottle());
				if(PIDDriveRight->OnTarget()) printf("Right on \n");
				if(PIDDriveLeft->OnTarget()) printf("Left on \n");
				}
					
				if(PIDRESETBUTTON)
				{
					//PIDDriveRight->SetPID(stickR.GetThrottle()+1,DRIVEINTGAIN, DRIVEDERIVGAIN); 
					//PIDDriveLeft->SetPID(stickR.GetThrottle()+1,DRIVEINTGAIN, DRIVEDERIVGAIN);
					PIDDriveLeft->Reset();
					PIDDriveRight->Reset();
					PIDDriveLeft->Enable();
					PIDDriveRight->Enable();
				}
			}
			else
			{
				if(PIDDriveLeft->IsEnabled()) PIDDriveLeft->Reset();
				if(PIDDriveRight->IsEnabled()) PIDDriveRight->Reset();
				if(stickYAbs[0] > 0.05)
				{
					frontLeftMotor->Set(stickY[0]);
					rearLeftMotor->Set(stickY[0]);
				}
				else
				{
					frontLeftMotor->Set(0);
					rearLeftMotor->Set(0);
				}
				if(stickYAbs[1] > 0.05)
				{
					frontRightMotor->Set(-stickY[1]);
					rearRightMotor->Set(-stickY[1]);
				}
				else
				{
					frontRightMotor->Set(0);
					rearRightMotor->Set(0);
				}
			}
			
			if(stickL.GetRawButton(2) && stickL.GetRawButton(3) && stickR.GetRawButton(2) &&
					stickR.GetRawButton(3) && BACKWARDBUTTON && !(count%5)) bDrivePID = !bDrivePID;
			
			if(SHIFTBUTTON && shiftHigh)
			{
				shifter->Set(shifter->kReverse);
				shiftHigh = false;
				maxSpeed = 12;
			}
			else if(!SHIFTBUTTON && !shiftHigh)
			{
				shifter->Set(shifter->kForward);
				shiftHigh = true;
				maxSpeed = 25; //last value 35
			}
			
			sendIOPortData();
			
			/*
			if(LIGHTBUTTON) lightOn = true;
			else lightOn = false;
			if(!lightOn) light->Set(light->kOff);
			if(lightOn) light->Set(light->kOn);
			*/
			if(!MODESWITCH)
			{
				lastLiftSP = liftSP;
				if(!PIDLift->IsEnabled()) PIDLift->Enable();
				if(LIFTLOW1BUTTON) liftSP = LIFTLOW1;
				if(LIFTLOW2BUTTON) liftSP = LIFTLOW2;
				if(LIFTMID1BUTTON) liftSP = LIFTMID1;
				if(LIFTMID2BUTTON) liftSP = LIFTMID2;
				if(LIFTHIGH1BUTTON) liftSP = LIFTHIGH1;
				if(LIFTHIGH2BUTTON) liftSP = LIFTHIGH2;
				
				if(!lift->isBraking() && !disengageBrake)
				{
					PIDLift->SetSetpoint(liftSP);
					if(liftSP == 0 && liftPIDSource->PIDGet() < 0.25) //BUGBUG double value
					{
						//PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, 3*LIFTDERIVGAIN);
						PIDLift->SetOutputRange(-liftPIDSource->PIDGet() + 0.075, .75); //BUGBUG
					}
					else PIDLift->SetOutputRange(-0.3, .75); //BUGBUG half value
				}
				if(lift->isBraking() && lastLiftSP != liftSP)
				{
					PIDLift->SetSetpoint(lastLiftSP + 0.06);
					PIDLift->SetPID(11 + 1.5*lift->getPosition(), LIFTINTGAIN + 0.6 + 3*lift->getPosition()/10, 0);
					lift->brakeOff();
					disengageBrake = true;
					if(!liftCount) liftCount = count;
				}
				//set brake (because near)
				if(fabs(PIDLift->GetError()) < 0.01 && !lift->isBraking() && !disengageBrake)
				{
					if(liftCount == 0) liftCount = count;
					if(count - liftCount > 5)
					{
						PIDLift->Reset();
						liftMotor1->Set(LIFTNEUTRALPOWER);
						liftMotor2->Set(LIFTNEUTRALPOWER);
						Wait(0.02);
						lift->brakeOn();
						Wait(0.02);
						liftMotor1->Set(0.0);
						liftMotor2->Set(0.0);
						PIDLift->Enable();
						PIDLift->SetSetpoint(lift->getPosition());
						liftCount = 0;
					}
					//if(!(count%50)) printf("Braking/Not PID \n");
				}
				if(lift->isBraking() && !(count%10)) PIDLift->SetSetpoint(lift->getPosition());
				if(fabs(PIDLift->GetError()) < 0.01 && disengageBrake && count - liftCount > 3)
				{
					disengageBrake = false;
					if(liftEncoder->PIDGet() < liftSP) PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN - 0.015);
					else PIDLift->SetPID(LIFTPROPGAIN, LIFTINTGAIN, LIFTDERIVGAIN + 0.015);
					liftCount = 0;
				}
				
				//pid
				/*
				else if(!(fabs(PIDLift->GetError()) < 0.04) && !lift->isBraking() && liftPos == -20)
				{
					PIDLift->Enable();
					liftPos = -10;
					printf("PID GO PID GO PID GO PID GO PID GO \n");
				}
				*/
				//when liftPos is positive, measures position
				//when liftPos = -10, is pidding
				//when liftPos = -20, has just released brake
			}
			else //(MODESWITCH)
			{
				if(PIDLift->IsEnabled()) PIDLift->Reset();
				liftPower = .8*pow(2*((log(LIFTSLIDER + 0.003/.0208116511)/LOG17) + 0.116), 2)*(2*((log(LIFTSLIDER + 0.003/.0208116511)/LOG17) + 0.116)/fabs(2*((log(LIFTSLIDER + 0.003/.0208116511)/LOG17) + 0.116)));
				liftPowerAbs = fabs(liftPower);
				if(liftPowerAbs > 1) liftPower /= liftPowerAbs;
				//if(!(count%5)) printf("Slider: %f", liftPower);
				
				if(lift->isBraking() && liftPowerAbs > 0.05) lift->brakeOff();
				else if(!lift->isBraking() && liftPowerAbs <= 0.05 && !(count%5)) lift->brakeOn();
				if (liftPowerAbs > 0.05)
				{
					liftMotor1->Set(liftPower);
					liftMotor2->Set(liftPower);
				}
				else
				{
					liftMotor1->Set(0);
					liftMotor2->Set(0);
				}
			}
			if(MODESWITCH && LIFTLOW1BUTTON && LIFTMID1BUTTON && LIFTHIGH1BUTTON) liftEncoder->Reset();
			/*
			if(!(count%5))
			{
				printf("Lift pos: %f Lift error: %f Lift SP: %f \n", liftPIDSource->PIDGet(),
						PIDLift->GetError(), PIDLift->GetSetpoint());
			}
			*/
			if(!(count%5) && MODESWITCH && GRIPPERPOSUP && GRIPPERPOSDOWN && GRIPPERPOP)
			{	
				gripPIDOn = !gripPIDOn;
				printf("Switch'd\n");
			}
			if(gripPIDOn)	
			{
				if(!PIDGrip->IsEnabled()) PIDGrip->Enable();
				if(GRIPPERPOSUP && !GRIPPERPOSDOWN)
				{
					goalGripSP = 0;
				}
				else if(GRIPPERPOSDOWN && !GRIPPERPOSUP && lift->getPosition() < 0.5)
				{
					goalGripSP = 1;
				}
				/*
				else if(!GRIPPERPOSDOWN && !GRIPPERPOSUP)
				{
					goalGripSP = 0.5;
				}
				*/
				
				gripError = PIDGrip->GetError();
				gripErrorAbs = fabs(gripError);
				PIDGrip->SetSetpoint(goalGripSP);
				
				if(gripErrorAbs < 0.4) PIDGrip->SetOutputRange(-0.4, 0.5); //negative is up
				else PIDGrip->SetOutputRange(-0.9, 0.7); //negative is up
				if(gripErrorAbs > 0.0 && gripErrorAbs < 0.2)
				{
					PIDGrip->SetPID(GRIPPROPGAIN - 1.25*(1 - gripErrorAbs) + gripPropMod, GRIPINTGAIN + gripIntMod, 0.3 + 0.1*(1 - gripPIDSource->PIDGet()));
				}
				else
				{
					PIDGrip->SetPID(GRIPPROPGAIN - 1.*(1 - gripErrorAbs) + gripPropMod, 0, 0.02);
				}
				if(fabs(gripPIDSource->PIDGet()) < 0.03 && PIDGrip->GetSetpoint() == 0)
				{
					gripLatch1->Set(true);
					gripLatch2->Set(false);
				}
				else if(!(gripLatch1->Get() && PIDGrip->GetSetpoint() == 0) || 
						gripPIDSource->PIDGet() < 0) 
				{
					gripLatch1->Set(false);
					gripLatch2->Set(true);
				}
					
				if(gripLatch1->Get() && !(count%20)) 
				{
					PIDGrip->Reset();
					PIDGrip->Enable();
				}
				/*
				if(stickL.GetRawButton(1) && !(count%5)){
					gripIntMod -= 0.001;
				}
				
				if(stickR.GetRawButton(1) &&!(count%5))
				{
					gripIntMod += 0.001;
				}
				if(stickL.GetRawButton(2) && !(count%5))
				{
					gripPropMod -= 0.1;
				}
				if(stickL.GetRawButton(3) && !(count%5))
				{
					gripPropMod += 0.1;
				}
				*/
				/*
				if(LIFTBOTTOMBUTTON)
				{
					PIDGrip->Reset();
					PIDGrip->Enable();
				}
				*/
				/*
				if(!(count%5))
				{
					printf("Gripper pos: %f Gripper error: %f Grip power: %f \n",
							gripPIDSource->PIDGet(), PIDGrip->GetError(), gripMotor1->Get());
				}
				*/
			
			}
			//Calibration routine
			else
			{
				if(gripLatch1->Get() == true) 
				{
					gripLatch1->Set(false);
					gripLatch2->Set(true);
				}
				if(PIDGrip->IsEnabled()) PIDGrip->Reset();
				if(GRIPPERPOSUP)
				{
					gripMotor1->Set(-0.5);
					//gripMotor2->Set(0.5);
				}
				else if(GRIPPERPOSDOWN)
				{
					gripMotor1->Set(0.5);
					//gripMotor2->Set(-0.5);
					
				}
				else
				{
					gripMotor1->Set(0);
					//gripMotor2->Set(0);
				}
			}
			//if(!(count%5)) printf("Grip volts: %f \n", gripPot->GetVoltage());
			//if(!(count%5)) printf("Grip 1 voltage: %f \n", gripMotor1->Get());
			if(GRIPPEROPEN && !popStage)
			{
#if !(INNERGRIP)
				gripOpen1->Set(true);
				gripOpen2->Set(false);
#else
				gripOpen1->Set(false);
				gripOpen2->Set(true);
#endif
			}
			else if(!popStage)
			{
#if !(INNERGRIP)
				gripOpen1->Set(false);
				gripOpen2->Set(true);
#else
				gripOpen1->Set(true);
				gripOpen2->Set(false);
#endif
			}
			if(GRIPPERPOP && !popStage && goalGripSP == 0 && !(GRIPPEROPEN && GRIPPERCLOSE)) popStage = 1;
			if(popStage) popTime = GetClock();
			if(popStage == 1)
			{
				//popCount = count;
				popStart = GetClock();
				popStage++;
				//printf("POP START POP START POP START \n");
			}
			if(popStage == 2)
			{
#if !(INNERGRIP)
				gripOpen1->Set(true);
				gripOpen2->Set(false);
#else
				gripOpen1->Set(false);
				gripOpen2->Set(true);
#endif
				popStage++;
				//printf("GRIP OPEN GRIP OPEN GRIP OPEN \n");
			}
			if(popStage == 3 && popTime - popStart > 0.0) //used to be 0.15
			{
				gripPop1->Set(true);
				gripPop2->Set(false);
				popStage++;
				//printf("POP OUT POP OUT POP OUT \n");
			}
			if(popStage == 4 && popTime - popStart > .75) //time was 0.9
			{
				gripPop1->Set(false);
				gripPop2->Set(true);
				popStage++;
				//printf("POP IN POP IN POP IN \n");
			}
			if(popStage == 5 && popTime - popStart > 0.9)	popStage = 0; //time was 1.05
			
			if(MINIBOTSWITCH && !(MODESWITCH && stickR.GetRawButton(1) && stickL.GetRawButton(1))) miniDep->Set(miniDep->kReverse);
			else if(MINIBOTSWITCH && MODESWITCH && stickR.GetRawButton(1) && stickL.GetRawButton(1)) miniDep->Set(miniDep->kOn);
			
			if(!compSwitch->Get()) compressor->Set(compressor->kReverse);
			else compressor->Set(compressor->kOff);
			/*
			if(stickR.GetRawButton(1)) compressor->Set(compressor->kReverse);
			else compressor->Set(compressor->kForward);
			*/
			Wait(0.02);				// wait for a motor update time
		}
	}
};

START_ROBOT_CLASS(SixWheelBot);

#endif
#endif