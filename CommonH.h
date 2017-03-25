#pragma once


#ifdef QT_COMPIL
    #define TRUE 1
    #define FALSE 0
    #include <qdebug.h>
#endif

#if (_MSC_VER >= 1900)
    #include <afxtempl.h>
#endif


#define MAX_POS_VEL (0.2)
#define MAX_ROT_VEL (60)
#define MAX_GRIP_VEL (0.05)

#define INIT_POS_VEL (0.02)
#define INIT_ROT_VEL (6)
#define INIT_GRIP_VEL (0.005)

#define LOGICAL_OP "&|!^"
#define RELATIONAL_OP "<>=!"
#define MATH_OP "+*-/%"
#define POS_OP "@+-"

#define PI (3.14159265358979323846)

#define radToDeg 57.2957795130785499f
#define degToRad 0.017453292519944444f

#define TOTAXES 5

//  Machine configuration values
#define MOTOR1_ARM_LENGTH (0.2)
#define MOTOR2_ARM_LENGTH (0.2)
#define MOTOR1_LIMIT_M	 (-120) //  degree
#define MOTOR1_LIMIT_P	 (120)  //  degree
#define MOTOR2_LIMIT_M	 (-115) //  degree
#define MOTOR2_LIMIT_P	 (115)  //  degree
#define MOTORZ_LIMIT_M	 (0)    //  displacement
#define MOTORZ_LIMIT_P	 (120)  //  displacement
#define THETA_LIMIT_M	 (0)    //  degree
#define THETA_LIMIT_P	 (350)  //  degree
#define GRIP_LIMIT_M	 (0)    //  no-unit.
#define GRIP_LIMIT_P	 (180.01) // +0.1 because of truncation error
#define SPEED_LIMIT_M	 (10)   //%
#define SPEED_LIMIT_P	 (100)  //%
#define DEFAULT_ROBOT_SPEED		9.5
#define INITIAL_XPOS  (0)       // Coordinate
#define INITIAL_YPOS  (400)     // Coordinate
#define INITIAL_ZPOS  (0)       // Coordinate
#define INITIAL_TDEG  (0)       // Degree
#define INITIAL_GRPDEG  (0)     // No-unit

//  Gear for determining the direction o
#define ZAXIS_GEARRATIO -1
#define TAXIS_GEARRATIO -1
#define GRIP_GEARRATIO 1.4

// Home
#define MOTOR1_HOMEANGLE (-120)         //degree
#define MOTOR2_HOMEANGLE (-115)         //degree
#define HOMEMOVE_KSS1500_Z_AXIS		0   //displacement
#define HOME_ROTATE_ANG				0   //degree
#define HOME_GRIP_ANG				0   //no-unit

//Grip
#define GRIP_ON			   0
#define GRIP_OFF		   1
#define GRIP_DISPLACEMENT -0.0180
#define GRIP_MINPOS        0
#define GRIP_STOP_TORQUE   40
#define GRIP_MAX_TORQUE    450
#define GRIPMOVE_SPEED     0.003
#define GRIP_STROKE_OFFSET -0.0180

//Rotate
#define ROTATE_SPEED	   100
#define JOINTSPEED			8
#define JOINTSPEED_Z_AXIS	1

// Defining Homestep 0 -> 1
#define HOMEMOVE_0_STEP		0
#define HOMEMOVE_1_STEP		1

// Speed constant
#define HOMEMOVE_SPEED		8
#define JOG_SPEED           11

// Property setting
#define TPSAVE_POINTLIST	"Point_List"
#define UNCONTROLLABLE_VALUE (-0xFFFF)
#define	MAX_POINT_NUM		99
#define NUM_POSE_PARAM 5

#define ID_SIMULATOR   0x101
#define ID_RS232C_COMM 0x102    //   Now it doesn't use it
#define ID_SOCKET_COMM 0x103

#pragma warning( disable : 4251 )

#include "RobotLangCommApis.h"
