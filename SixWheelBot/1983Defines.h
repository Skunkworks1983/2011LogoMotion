#define COMPBOT 1
#define OI 1
#define INNERGRIP 1
#define SPROCKET22 0
/*
 * Ports
 */

//PWM
#if COMPBOT
#define FRONTLEFTMOTORPORT 3
#define FRONTRIGHTMOTORPORT 1
#define REARLEFTMOTORPORT 4
#define REARRIGHTMOTORPORT 2
#define LIFTMOTORPORT1 5
#define LIFTMOTORPORT2 7
#define GRIPMOTORPORT1 9
#define GRIPMOTORPORT2 10
#define CAMERAXSERVOPORT 8
#define CAMERAYSERVOPORT 6
#endif

#if !(COMPBOT)
/*
#define FRONTLEFTMOTORPORT 3
#define FRONTRIGHTMOTORPORT 1
#define REARLEFTMOTORPORT 4
#define REARRIGHTMOTORPORT 2
*/
#define FRONTLEFTMOTORPORT 1
#define FRONTRIGHTMOTORPORT 3
#define REARLEFTMOTORPORT 2
#define REARRIGHTMOTORPORT 4
#endif

//Digital I/O
#define LEFTLIGHTSENSORPORT 3
#define MIDLIGHTSENSORPORT 2
#define RIGHTLIGHTSENSORPORT 1
#define LEFTMOTORA 4
#define LEFTMOTORB 5
#define RIGHTMOTORA 6 //channels for initializing encoders
#define RIGHTMOTORB 7
#define LIFTMOTORA 13
#define LIFTMOTORB 14
#define COMPRESSORSWITCH 12

//Analog I/O
#define ULTRASOUNDPORT 1
#define GRIPPOTPORT 2

//Relays
#define LIGHTPORT 1
#define BRAKEPORT 7 //red wire
#define SHIFTERPORT 2 //white wire
#define MINIBOTPORT 4
//#define MINIBOTPORT2 6
#define COMPRESSORPORT 8

//Solenoids
#define GRIPOPENPORT1 1 //green wire
#define GRIPOPENPORT2 2
#define GRIPPOPPORT1 3 //orange wire
#define GRIPPOPPORT2 4
#define GRIPLATCH1 5
#define GRIPLATCH2 6
//#define MINIBOTPORT 

/*
 * Program Constants
 */

#define GRIPOPEN gripOpen->kForward
#define GRIPCLOSE gripOpen->kReverse
#define GRIPPOPOUT gripPop->kForward
#define GRIPPOPIN gripPop->kReverse

//Autonomous
#define FOLLOWLINE 1
#define STRAIGHTLINEMODE 1
#define NOLINEMODE 0 //Not to be confused with FOLLOWLINE, do not modify this one
#define FORKMODE 2
#define FORKRIGHTMODE 3
#define OUTRIGHTMODE 5
#define FIRSTBREAKDISTANCE 0 //BUGBUG fake values
#define SECONDBREAKDISTANCE 0
#define FORKDISTANCE 95
#define WALLDISTANCE 20
#define STRAIGHTSCOREDISTANCE 0
#define FORKTURNDISTANCE 70
#define SCOREDISTANCE 2


//PIDs
#define DRIVEPROPGAIN 2.3 //last working value: .29
#define DRIVEINTGAIN 0.06 //last working value: 0.001
#define DRIVEDERIVGAIN 0.00 //last working value: 0.001

#define LIFTPROPGAIN 3.0 //last working value: 3.0
#define LIFTINTGAIN 0.06 //last working value: 0.038
#define LIFTDERIVGAIN 0.075 //last working value: 0.055
#define PI 3.14159
#if SPROCKET22
#define LIFTDISTPERPULSE 0.00857421 //(16./22.)*(16./22.)*(PI*1.29)/250 //BUGBUG new 22-tooth sprocket
#else
#define LIFTDISTPERPULSE (16./22.)*(16./36.)*(PI*1.29)/250 //0.00523980 
#endif
#define LIFTHEIGHT 28.75
#define LIFTNEUTRALPOWER 0.0
#if !INNERGRIP
#define LIFTLOW1 0
#define LIFTLOW2 0.069565 + 0.025
#define LIFTMID1 0.425287 + 0.025
#define LIFTMID2 0.522988 + 0.025
#define LIFTHIGH1 0.839080 + 0.05
#define LIFTHIGH2 0.942529 + 0.05 
#else
#define LIFTLOW1 0
#define LIFTLOW2 0.069565
#define LIFTMID1 0.425287
#define LIFTMID2 0.522988
#define LIFTHIGH1 0.839080 + 0.025
#define LIFTHIGH2 0.942529
#endif
#define RADIANSPERVOLT 1.9*PI/5
#define GRIPPROPGAIN 2.0 //last working value: 1.8
#define GRIPINTGAIN 0.02 //last working value: 0.025
#define GRIPDERIVGAIN 0.0 //last working value: 0.3

//OI Buttons/Switches
#define PIDRESETBUTTON myEIO.GetDigital(1)
//#define LIGHTBUTTON stickR.GetRawButton(1)
//#define BACKWARDBUTTON stickL.GetRawButton(4)
#define BACKWARDBUTTON myEIO.GetDigital(3)
#define DEMOSWITCH myEIO.GetDigital(3)
//#define SHIFTBUTTON stickR.GetRawButton(2)
#define SHIFTBUTTON stickR.GetRawButton(1)

#if COMPBOT && OI
#define MINIBOTSWITCH myEIO.GetDigital(5)
#define LIFTBOTTOMBUTTON myEIO.GetDigital(5)
#define LIFTLOW1BUTTON myEIO.GetDigital(6)
#define LIFTLOW2BUTTON myEIO.GetDigital(9)
#define LIFTMID1BUTTON myEIO.GetDigital(7)
#define LIFTMID2BUTTON myEIO.GetDigital(10)
#define LIFTHIGH1BUTTON myEIO.GetDigital(8)
#define LIFTHIGH2BUTTON myEIO.GetDigital(11)
#define MODESWITCH myEIO.GetDigital(12) //switch between button/PID control and slider control for lift
#define LIFTSLIDER myEIO.GetAnalogIn(1)
#define GRIPPERPOSUP myEIO.GetDigital(13)
#define GRIPPERPOSDOWN myEIO.GetDigital(14)
#define GRIPPEROPEN myEIO.GetDigital(16)
#define GRIPPERCLOSE myEIO.GetDigital(1) //BUGBUG Fake value
#define GRIPPERPOP myEIO.GetDigital(15) //controls the intertube-pusher-offer

#define CAMERAFRONT myEIO.GetDigital(3)
#define CAMERADOWN myEIO.GetDigital(2)
#endif
#if !OI
#define MINIBOTSWITCH stickOp.GetRawButton(4) && stickOp.GetRawButton(1)
#define LIFTBOTTOMBUTTON 0
#define LIFTLOW1BUTTON 0
#define LIFTLOW2BUTTON 0
#define LIFTMID1BUTTON 0
#define LIFTMID2BUTTON 0
#define LIFTHIGH1BUTTON 0
#define LIFTHIGH2BUTTON 0
#define MODESWITCH 0 //switch between button/PID control and slider control for lift
#define LIFTSLIDER 0
#define GRIPPERPOSUP 0
#define GRIPPERPOSDOWN 0
#define GRIPPEROPEN 0
#define GRIPPERCLOSE 0 //BUGBUG Fake value
#define GRIPPERPOP 0 //controls the intertube-pusher-offer

#define CAMERAFRONT 0
#define CAMERADOWN 0
#endif
