#include <math.h>
#include <time.h>
#include "CommonH.h"
#include "Kss1500RobotMotion.h"

#if (_MSC_VER >= 1900)
#include "atlstr.h"
#include <mmsystem.h>
#endif

//???? Range ????
#define GRIPPER_GEAR_RATIO(x)

//?u????? ???? ????
#define START			1
#define END				0

//?????? ???? ????
#define LEFT	0
#define RIGHT	1
#define UP		0
#define DOWN	1

typedef CKss1500RobotMotion::Kss1500MoveType MOVETYPE;

enum {
    EnqRobotReady = 0x10, ModeChange, GotoHomePosition, CMD_Drive, CMD_Move, InterrupReq, CMD_Grip,
    X_mMove, Y_Move, Z_Move, ReqRobotPosition, ReqTemperature, CMD_Rotate
};

using namespace std;


CKss1500RobotMotion::~CKss1500RobotMotion()
{
}

CKss1500RobotMotion::CKss1500RobotMotion()
    : CKss1500MoveRange()
{
    _fEuclidFeed = 70;
    _fCurrentGripperValue = 0;
    _fTargetGripperValue = 0.0;
    _fCurrentJointValue.resize(NUM_POSE_PARAM);
    _fTargetJointValue.resize(NUM_POSE_PARAM);
    _fTargetJointVelocity.resize(NUM_POSE_PARAM);
    _bFinishJointValue.resize(NUM_POSE_PARAM);
    _fCurrentGripperVelocity = (float) GRIPMOVE_SPEED;
    _bSimulationMoveCommandFinish = false;
    SetProtectMoveStatus(true);
    SetDeltaTime(0xffff);
    for (int i = 0; i<_fTargetJointVelocity.size();i++)
    {
        _fTargetJointVelocity[i] = 3.14159;
    }

    CPoint3D pos(INITIAL_XPOS, INITIAL_YPOS, INITIAL_ZPOS);
   _XyzTargetPosition = pos;
   _XyzCurrentPosition = pos;

   float Axis[3] = { INITIAL_XPOS, INITIAL_YPOS, INITIAL_ZPOS };
   float joints[3] = { INITIAL_TDEG };

   Inversekinematics(Axis, joints);
   SetCurrentJointValues(joints, 3);
   SetTargetJointValues(3, joints);
   GripCommand(INITIAL_GRPDEG);
   SetTargetRaxisValueByDegree(INITIAL_TDEG);
   inializeAll();
}

/*
void CKss1500RobotMotion::degreeToRadian(float & joints)
{
joints = joints * PI / 180;
}

void CKss1500RobotMotion::radianToDegree(float & joints)
{
joints = joints * 180 / PI;
}
*/
void CKss1500RobotMotion::inializeAll()
{
    defaultFirstSetting();
    ResetAngleSW();
    _iHomeStep = HOMEMOVE_0_STEP;
}

bool CKss1500RobotMotion::GetProtectMoveStatus()
{
    return _bMoveStatus;
}
void CKss1500RobotMotion::SetProtectMoveStatus(bool LockOrUnlock)
{
    _bMoveStatus = LockOrUnlock;
}

bool CKss1500RobotMotion::getSimulationMoveCommandFinish()
{
    return _bSimulationMoveCommandFinish;
}

void CKss1500RobotMotion::setSimulationMoveCommandFinish(bool status)
{
    _bSimulationMoveCommandFinish = status;
}

void CKss1500RobotMotion::SetCurrentJointPositions(const CPoint3D& pos)
{
    _XyzCurrentPosition = pos;
}
void CKss1500RobotMotion::SetCurrentXaxisPos(const float X)
{
    _XyzCurrentPosition.X = X;
}

void CKss1500RobotMotion::SetCurrentYaxisPos(const float Y)
{
    _XyzCurrentPosition.Y = Y;
}

void CKss1500RobotMotion::SetCurrentZaxisPos(const float Z)
{
    _XyzCurrentPosition.Z = Z;
}

void CKss1500RobotMotion::SetCurrentRaxisPos(const float R)
{
    _fCurrentJointValue[3] = R;
}

void CKss1500RobotMotion::GetCurrentJointValues(int index, float values[])
{
    for (int i = 0; i < index; i++)
    {
        values[i] = _fCurrentJointValue[i];
    }
}

void CKss1500RobotMotion::SetCurrentJointValues(float values[], int size)
{
    for(int i = 0; i < size; i++)
    {
       _fCurrentJointValue[i] = values[i];
    }
}

//	from 0 to size()-1
float CKss1500RobotMotion::GetCurrentJointValue(int index)
{
    if(index < (int) _fCurrentJointValue.size())
        return (_fCurrentJointValue[index]);

    return UNCONTROLLABLE_VALUE;
}

float CKss1500RobotMotion::GetCurrentRaxisPos(void)
{
    return _fCurrentJointValue[3];
}

void CKss1500RobotMotion::SetCurrentGripperPos(const float G)
{
    _fCurrentGripperValue = G;
}

float CKss1500RobotMotion::GetCurrentGripperPos(void)
{
    return _fCurrentGripperValue;
}

CPoint3D CKss1500RobotMotion::GetCurrentJointPositions(void)
{
    return _XyzCurrentPosition;
}

float CKss1500RobotMotion::GetTargetRaxisValue(void)
{
    return _fTargetJointValue[3];
}

void CKss1500RobotMotion::SetTargetJointPositions(const CPoint3D& pos)
{
    _XyzTargetPosition = pos;
}

void CKss1500RobotMotion::SetTargetRaxisValueByDegree(const float deg)
{
    _fTargetJointValue[3] = (float)((3.14 / 180.)*deg);
}

void CKss1500RobotMotion::SetTargetRaxisValue(const float pos)
{
    _fTargetJointValue[3] = pos;
}

float CKss1500RobotMotion::GetTargetRaxisValueByDegree(void)
{
    return (float) _fTargetJointValue[3] * 180. / 3.14;
}

float CKss1500RobotMotion::GetCurrentRaxisValueByDegree(void)
{
    return (float) _fCurrentJointValue[3] * 180. / 3.14;
}

float CKss1500RobotMotion::GetTargetGripperValueByDegree(void)
{
    return 	(float) (_fTargetGripperValue / GRIP_DISPLACEMENT* GRIP_LIMIT_P);
}

void CKss1500RobotMotion::GetCurrentJointVelocities(int size, float values[])
{
    for(int i = 0; i < size; i++)
    {
       values[i] = _fTargetJointVelocity[i];
    }
}

float CKss1500RobotMotion::GetCurrentGripperValueByDegree()
{
    return _fCurrentGripperValue/GRIP_DISPLACEMENT*GRIP_LIMIT_P;
}

void CKss1500RobotMotion::SetTargetGripperValueByDegree(const float pos)
{
    _fTargetGripperValue = GRIP_DISPLACEMENT*pos/GRIP_LIMIT_P;
}

void CKss1500RobotMotion::SetCurrentGripperValueByDegree(const float pos)
{
    _fCurrentGripperValue = GRIP_DISPLACEMENT*pos/GRIP_LIMIT_P;
}

void CKss1500RobotMotion::ResetAngleSW()
{
    _bFinishGrip = false;
    _FinishRotate = false;

    _bFinishJointValue[0] = false;
    _bFinishJointValue[1] = false;
    _bFinishJointValue[2] = false;
    _bFinishJointValue[3] = false;
}

void CKss1500RobotMotion::defaultFirstSetting()
{
    _iMotor1_FirstSetting = false;
    _iMotor2_FirstSetting = false;
    _iMotor3_FirstSetting = false;
    _iMotorRotate_FirstSetting = false;
    _bMotorGripFirstSetting = false;
    _bMotorMoveFirstSetting = false;
}


const bool CKss1500RobotMotion::getCheckingReachTheRange(int MotorType)
{
    switch (MotorType)
    {
    case MOTOR_1:	return _iMotor1_FirstSetting;
        break;
    case MOTOR_2:	return _iMotor2_FirstSetting;
        break;
    case MOTOR_3:	return _iMotor3_FirstSetting;
        break;
    case MOTOR_ROTATE:	return  _iMotorRotate_FirstSetting;
        break;
    case GRIP:	return  _bMotorGripFirstSetting;
        break;
    case MOVE: return _bMotorMoveFirstSetting;
        break;
    default:		return false;
        break;
    }
}

void CKss1500RobotMotion::setCheckingReachTheRange(int MotorType, bool statue)
{
    switch (MotorType)
    {
    case MOTOR_1:	 _iMotor1_FirstSetting = statue;
        break;
    case MOTOR_2:	_iMotor2_FirstSetting = statue;
        break;
    case MOTOR_3:	_iMotor3_FirstSetting = statue;
        break;
    case MOTOR_ROTATE:	_iMotorRotate_FirstSetting = statue;
        break;
    case GRIP:	_bMotorGripFirstSetting = statue;
        break;
    case MOVE:	_bMotorMoveFirstSetting = statue;
        break;
    default:
        break;
    }
}

bool CKss1500RobotMotion::MoveCommand(Kss1500MoveType Command, int MotorNumber, float MotorSpeed, float Distance_1, float Distance_2, float Distance_3, float Distance_4)
{
    //?u????? ?????? ??????? ??????
    if (!getSimulationMoveCommandFinish())
    {
        switch (Command)
        {
        case MOVETYPE::MOTOR_3: if (Motor3InterpreteMotion(MotorSpeed, Distance_1, false))
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::MOTOR_3_JOG: if (Motor3InterpreteMotion(MotorSpeed, Distance_1, true))
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::HOME: if (HomeCommand())
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::GRIP: if (GripCommand(Distance_1))
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::MOVE: //3???? ?u?????? ???? ?¥ê????? ????? ????? ????.
            if (InterpreteMotion(MotorSpeed, Distance_1, Distance_2, Distance_3, MOVETYPE::COMMAND_MOVE))
                setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::MOVE2: //5???? ?u?????
            if (InterpreteMotion(MotorSpeed, Distance_1, Distance_2, Distance_3, Distance_4, MOVETYPE::COMMAND_MOVE))
                setSimulationMoveCommandFinish(true);
            break;

        case MOVETYPE::DRIVE:
//      case MOVETYPE::DRIVE:	if (driveConverteCommand(MotorNumber, MotorSpeed, Distance_1))
            if (DriveCommand(MotorNumber, MotorSpeed, Distance_1))
                setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::ROTATE: if (RotateCommand(Distance_1))
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::MOVE_JOINT: if (InterpreteMotion(MotorSpeed, Distance_1, Distance_2, Distance_3, MOVETYPE::TP_MOVE))
            setSimulationMoveCommandFinish(true);
            break;
        case MOVETYPE::MOVE_JOG:  if (InterpreteMotion(MotorSpeed, Distance_1, Distance_2, Distance_3, MOVETYPE::MOVE_JOG))
            setSimulationMoveCommandFinish(true);
            break;

        default:

            break;
        }
    }


    //?u??????? ????? ???? KSS1500 ?¥ê??? ?????? ???????
    if (getSimulationMoveCommandFinish() == true)
    {
        setSimulationMoveCommandFinish(false);
        return true;
    }
    return false;
}


void CKss1500RobotMotion::SetTargetJointValues(int size, float value[])
{
    for(int i = 0; i < size; i++)
    {
       _fTargetJointValue[i] = value[i];
    }
}

void CKss1500RobotMotion::SetTargetJointValue(int nDrive, float value)
{
    _fTargetJointValue[nDrive] = value;
}

void CKss1500RobotMotion::SetTargetJointValue(float value1, float value2, float value3, float value4)
{
    _fTargetJointValue[0] = value1;
    _fTargetJointValue[1] = value2;
    _fTargetJointValue[2] = value3;
    _fTargetJointValue[3] = value4;
}

bool CKss1500RobotMotion::driveConverteCommand(int MotorNumber, float MotorSpeed, float & Distance_1)
{
    float fCurrentJointPosition = 0;
    float fRelativeRange = 0;

//	simGetJointPosition(_iDriveJointHandles[MotorNumber - 1], &(fCurrentJointPosition));

    //??????? -> ????????? ???
    if ((MotorNumber == MOTOR_1) || (MotorNumber == MOTOR_2))
    {
        RadianToDegree (fCurrentJointPosition);
        //?u?????? ???? ?¥ê????? ????? ???? ??????.
        fRelativeRange = (fCurrentJointPosition + Distance_1);
    }
    if (MotorNumber == MOTOR_3)
    {
        fCurrentJointPosition = (fCurrentJointPosition*118.0f) / 0.118f;
        //?u?????? ???? ?¥ê????? ????? ???? ??????.
        fRelativeRange = (fCurrentJointPosition + Distance_1);
    }
    //Drive?? ?????? ???????
    if (DriveCommand(MotorNumber, MotorSpeed, fRelativeRange))
        return true;

    return false;
}

/*
void CKss1500RobotMotion::degreeToRadian(float & joints)
{
    joints = joints * PI / 180;
}

void CKss1500RobotMotion::radianToDegree(float & joints)
{
    joints = joints * 180 / PI;
}
*/

void CKss1500RobotMotion::initializeRobot()
{
    // ???? ??? ???? ?????¢¥??

    _iHomeStep = HOMEMOVE_0_STEP;
}

const CPoint3D& CKss1500RobotMotion::GetTargetJointPositions(void)
{
    return _XyzTargetPosition;
}
/*
float CKss1500RobotMotion::GetTargetGripperAxisValue(void)
{
    float rot = - (float)((180/ 3.14)*_fTargetJointValue[3]);
    return rot;
}
*/
bool CKss1500RobotMotion::InterpreteMotion(float MotorSpeed, float X_Axis, float Y_Axis, float Z_Axis, int type)
{
    int i = 0;
    float Axis[3] = { X_Axis, Y_Axis, Z_Axis };
    float joints[3] = { 0 };

    if (GetProtectMoveStatus() == true)
    {
        // Unit : Degree
         if(type == MOVETYPE::MOVE_JOG)
        {
             // XYZ coordinate value
             if (!Inversekinematics(Axis, joints))
                 return true;

            for (i = 0; i < 2; i++)
               Axis[i] = joints[i];

            Axis[2] = Z_Axis;

            SetTargetJointValues(3,Axis);
               CPoint3D targPos(X_Axis,Y_Axis,Z_Axis);
            SetTargetJointPositions( targPos);
            type = 0;   //   Range check enable
        }
         // Unit : Degree
         else if (type == MOVETYPE::COMMAND_MOVE)
         {
             if (!Inversekinematics(Axis, joints))
                 return true;

             for (i = 0; i < 2; i++)
                 Axis[i] = joints[i];

             Axis[2] = Z_Axis;
             CPoint3D targPos(X_Axis, Y_Axis, Z_Axis);
             SetTargetJointPositions(targPos);
             type = 0;   //   Range check enable
         }
        else
        {
            // ??????? ????
            RadianToDegree(Axis[0]);
            RadianToDegree(Axis[1]);
            Axis[2] = (Axis[2] * 118.0f) / 0.118f;
        }
        SetProtectMoveStatus(false);
    }

    float xySpeeds[2] = {0.0f,};
    if(!DetermineXYvelocities(MotorSpeed, xySpeeds))
    {
        xySpeeds[0] = MotorSpeed;
        xySpeeds[1] = MotorSpeed;
    }

    if (_bFinishJointValue[0] == false)
    {
        if (Motor1InterpreteMotion(xySpeeds[0], Axis[0], type))
            _bFinishJointValue[0] = true;
    }
    if (_bFinishJointValue[1] == false)
    {
        if (Motor2InterpreteMotion(xySpeeds[1], Axis[1], type))
            _bFinishJointValue[1] = true;
    }
    if (_bFinishJointValue[2] == false)
    {
        if (Motor3InterpreteMotion(MotorSpeed, Axis[2], type))
            _bFinishJointValue[2] = true;
    }

    if ((_bFinishJointValue[0] == true) && (_bFinishJointValue[1] == true) && (_bFinishJointValue[2] == true))
    {
        ResetAngleSW();
        SetProtectMoveStatus(true);
        return true;
    }
    return false;
}

bool CKss1500RobotMotion::InterpreteMotion(float MotorSpeed, float X_Axis, float Y_Axis, float Z_Axis, float R_Axis, float Gripper, int type)
{
    if(InterpreteMotion(MotorSpeed, X_Axis, Y_Axis, Z_Axis, R_Axis, type))
    {
        if (GripCommand(Gripper))
        {
            setSimulationMoveCommandFinish(true);
            ResetAngleSW();
            return true;
        }
        return false;
    }
    return false;
}

bool CKss1500RobotMotion::InterpreteMotion(float MotorSpeed, float X_Axis, float Y_Axis, float Z_Axis, float R_Axis, int type)
{
    int i = 0;
    float Axis[4] = {  X_Axis, Y_Axis, Z_Axis, R_Axis };
    float joints[4] = { 0, };

    if (GetProtectMoveStatus() == true)
    {
        // Unit : Degree
        if (type == MOVETYPE::MOVE_JOG)
        {
            if (!Inversekinematics(Axis, joints))
                return true;

            for (i = 0; i < 3; i++)
                Axis[i] = joints[i];

            SetTargetJointValues(3,Axis);
            CPoint3D targPos(X_Axis,Y_Axis,Z_Axis);
            SetTargetJointPositions( targPos);
            type = 0;   //   Range check enable
        }
        // Unit : Degree
        else if (type == MOVETYPE::COMMAND_MOVE)
        {
            if (!Inversekinematics(Axis, joints))
                return true;

            for (i = 0; i < 2; i++)
                Axis[i] = joints[i];

            Axis[2] = Z_Axis;
            CPoint3D targPos(X_Axis, Y_Axis, Z_Axis);
            SetTargetJointPositions(targPos);
            type = 0;   //   Range check enable
        }
        else
        {
            RadianToDegree(Axis[0]);
            RadianToDegree(Axis[1]);
            Axis[2] = (Axis[2] * 118.0f) / 0.118f;
        }
        SetProtectMoveStatus(false);
    }
    if (_bFinishJointValue[0] == false)
    {
        if (Motor1InterpreteMotion(MotorSpeed, Axis[0], type))
            _bFinishJointValue[0] = true;
    }
    if (_bFinishJointValue[1] == false)
    {
        if (Motor2InterpreteMotion(MotorSpeed, Axis[1], type))
            _bFinishJointValue[1] = true;
    }
    if (_bFinishJointValue[2] == false)
    {
        if (Motor3InterpreteMotion(MotorSpeed, Axis[2], type))
            _bFinishJointValue[2] = true;
    }
    if (_bFinishJointValue[3] == false)
    {
        if (MotorRotateCommand(MotorSpeed, Axis[3]))
            _bFinishJointValue[3] = true;
    }
    //????4???? ?????? ???????
    if ((_bFinishJointValue[0] == true) && (_bFinishJointValue[3] == true) && (_bFinishJointValue[1] == true) && (_bFinishJointValue[2] == true))
    {
        ResetAngleSW();
        SetProtectMoveStatus(true);
        return true;
    }
    return false;
}

bool CKss1500RobotMotion::RotateCommand(float MotorRange)
{
    if (MotorRotateCommand(ROTATE_SPEED, MotorRange))
        return true;
    else
        return false;
}

bool CKss1500RobotMotion::HomeCommand()
{

    initializeRobot();

    if (_iHomeStep == HOMEMOVE_0_STEP)
    {
        if (_bFinishGrip == false)
        {
            if (GripCommand(HOME_GRIP_ANG))
                _bFinishGrip = true;
        }
        if (_FinishRotate == false)
        {
            if (MotorRotateCommand(200, HOME_ROTATE_ANG))
                _FinishRotate = true;
        }
        if (_bFinishJointValue[2] == false)
        {
            if (Motor3InterpreteMotion(100, HOMEMOVE_KSS1500_Z_AXIS, false))
                _bFinishJointValue[2] = true;
        }
        if ((_bFinishGrip == true) && (_FinishRotate == true) && (_bFinishJointValue[2] == true))
            _iHomeStep = HOMEMOVE_1_STEP;
    }

    if (_iHomeStep == HOMEMOVE_1_STEP)
    {
        if (_bFinishJointValue[0] == false)
        {
            //  Can't understand the reason to use different value of documents from DMBH
            if (Motor1InterpreteMotion(HOMEMOVE_SPEED, MOTOR1_HOMEANGLE, true))
                _bFinishJointValue[0] = true;
        }
        if (_bFinishJointValue[1] == false)
        {
            if (Motor2InterpreteMotion(HOMEMOVE_SPEED,MOTOR2_HOMEANGLE, true))
                _bFinishJointValue[1] = true;
        }

        if ((_bFinishJointValue[0] == true) && (_bFinishJointValue[1] == true))
        {
            ResetAngleSW();
            _iHomeStep = HOMEMOVE_0_STEP;
            return true;
        }
    }

    return false;
}

bool CKss1500RobotMotion::CheckMaxMinRangeAndSpeed(int MotorNumber, float& MotorSpeed, float& MotorRange, bool TPMode)
{
    if(TPMode == true)
        return true;

    switch (MotorNumber)
    {
        //1?? ???????? -97.5 ~ +97.5??
    case MOTOR_1:
        if ((MotorRange <= MOTOR1_LIMIT_M) || (MotorRange >= MOTOR1_LIMIT_P))
        {
            _bLimitStatus[0] = 1;
            return false;
        }
        else
            _bLimitStatus[0] = 0;

    break;

    //2?? ???????? -117.8 ~ 117.8??
    case MOTOR_2:
        if ((MotorRange <= MOTOR2_LIMIT_M) || (MotorRange >= MOTOR2_LIMIT_P))
        {
            _bLimitStatus[1] = 1;
            return false;
        }
        else
            _bLimitStatus[1] = 0;
    break;

    //KSS1500?¥ê? 3?? ???????? ???¥á? : 0 ~ 116mm
    //SR750?¥ê?  3?? ???????? ???¥á? : 0 ~ 80mm
    case MOTOR_3:
            if ((MotorRange < MOTORZ_LIMIT_M) || (MotorRange > MOTORZ_LIMIT_P))
            {
                _bLimitStatus[2] = 1;
                return false;
            }
            else {
                _bLimitStatus[2] = 0;
            }

        break;

    case MOTOR_ROTATE:
        if (MotorRange > THETA_LIMIT_P)
        {
            MotorRange = (float)THETA_LIMIT_P;
        }
        else if (MotorRange < THETA_LIMIT_M)
        {
            MotorRange = THETA_LIMIT_M;
        }
        break;

    default:
        break;
    }
    //??? ????? ????? ????
    if (MotorSpeed < 10)
        MotorSpeed = 10;
    else if (MotorSpeed > 100)
        MotorSpeed = 100;
    //????1??2?? ??? ????????
    if (MotorNumber != MOTOR_3)
    {
        MotorSpeed = MotorSpeed /20;
    }
    //3?? ?????? ??? ????????
    else if (MotorNumber == MOTOR_3)	//3???? ???
    {
        MotorSpeed = 0.01f*(MotorSpeed)/6;
    }
    //else
    return true;
}

bool CKss1500RobotMotion::DriveCommand(int MotorNumber, float MotorSpeed, float MotorRange)
{
    initializeRobot();

    switch (MotorNumber)
    {
    case MOTOR_1: if (Motor1InterpreteMotion(MotorSpeed, MotorRange, false))
        return true;
        break;

    case MOTOR_2: if (Motor2InterpreteMotion(MotorSpeed, MotorRange, false))
        return true;
        break;

    case MOTOR_3: if (Motor3InterpreteMotion(MotorSpeed, MotorRange, false))
        return true;
        break;

    default:
        break;
    }

    return false;
}

void CKss1500RobotMotion::EnableManualControlPanel(bool sw)
{
#ifndef QT_COMPIL
    TRACE("Implement EnableManualControlPanel to Show Teachpendant \r\n");
#else
    qDebug()<<"Implement EnableManualControlPanel to Show Teachpendant \r\n";
#endif
}

bool CKss1500RobotMotion::MoveKinematicGripperFinger(float amount)
{
#ifndef QT_COMPIL
    TRACE("Implement MoveKinematicGripperFinger to simSetJointTargetPosition \r\n");
#else
    qDebug()<<"Implement MoveKinematicGripperFinger to simSetJointTargetPosition \r\n";
#endif
    return true;
}

bool CKss1500RobotMotion::MoveDynamicGripperFinger(float amount)
{
#ifndef QT_COMPIL
    TRACE("Implement MoveDynamicGripperFinger to simSetJointForce \r\n");
#else
    qDebug()<<"Implement MoveDynamicGripperFinger to simSetJointForce \r\n";
#endif
    return true;
}

bool CKss1500RobotMotion::GrapObstacles(float detactableValue)
{
#ifdef QT_COMPIL
    qDebug()<<"Implement GrapObstacles";
#else
    TRACE("Implement GrapObstacles");
#endif
    return true;
}

bool CKss1500RobotMotion::ReleaseObstacles(float MotorGrip_Cur_Ang)
{

#ifdef QT_COMPIL
    qDebug()<<"Implement ReleaseObstacles";
#else
    TRACE("Implement ReleaseObstacles");
#endif
    return true;
}

bool CKss1500RobotMotion::GetFingerGrabberForce(float &maxiTorque)
{
#ifdef QT_COMPIL
    qDebug()<<"Implement GetFingerGrabberForce\r\n";
#else
    TRACE("Implement GetFingerGrabberForce\r\n");
    maxiTorque = 0;
#endif
    return true;
}

bool CKss1500RobotMotion::SetFingerGrabberForce(float maxiTorque)
{
#ifdef QT_COMPIL
    qDebug()<<"Implement SetFingerGrabberForce\r\n";
#else
    TRACE("Implement SetFingerGrabberForce\r\n");
#endif
    return true;
}

bool CKss1500RobotMotion::TurnOffSuction(void)
{
#ifdef QT_COMPIL
    qDebug()<<"Implement TurnOffSuction\r\n";
#else
    TRACE("Implement TurnOffSuction\r\n");
#endif
    return true;
}


bool CKss1500RobotMotion::TurnOnSuction(void)
{
#ifdef QT_COMPIL
    qDebug()<<"Implement TurnOnSuction\r\n";
#else
    TRACE("Implement TurnOnSuction\r\n");
#endif
    return true;
}

// Input unit is Degree
bool CKss1500RobotMotion::GripCommand(float MotorRange)
{
    static float _fMotorGrip_TargetVelocity = 0;
    static float _fMotorGrip_nDirection = 0;
    static float _fMotorGrip_CurrentTime = 0;
    static float _fMotorGrip_ExecTime = 0;

    bool bMoveComplete = false;

    // 0 -> 180, 180 -> 0
    _fTargetGripperValue = GRIP_DISPLACEMENT*MotorRange/GRIP_LIMIT_P;

    if (_fTargetGripperValue <= (float) (GRIP_DISPLACEMENT))
        _fTargetGripperValue = (float) (GRIP_DISPLACEMENT);
    else if (_fTargetGripperValue >= (float) (GRIP_MINPOS))
        _fTargetGripperValue = (float) (GRIP_MINPOS);


    if (!getCheckingReachTheRange(GRIP))
    {
        float fGrippterMoveDistance = 0;

        //????? onoff ?????? ????
        if (_fTargetGripperValue - _fCurrentGripperValue >=0)
            _fMotorGrip_nDirection = GRIP_OFF;
        else
            _fMotorGrip_nDirection = GRIP_ON;

        float fMoveTime = 0;
        //??? ????
        fGrippterMoveDistance = fabs(_fCurrentGripperValue - _fTargetGripperValue);
        //??????¥ì? ????? ?©£?
        fMoveTime = (float)(fabs(fGrippterMoveDistance / GRIPMOVE_SPEED));

        //?????????? ?????? ?????? ?? ????? ???????.
        setCheckingReachTheRange(GRIP, true);
    }

    float fDeltaAng = 0;

    if ((_fCurrentGripperValue <= _fTargetGripperValue && _fMotorGrip_nDirection == GRIP_OFF)
    || ( _fCurrentGripperValue >= _fTargetGripperValue && _fMotorGrip_nDirection == GRIP_ON))
    {
            //??? ?©£??? ???? ??????? ?©£??? ????

            fDeltaAng = (float)(GRIPMOVE_SPEED*GetDeltaTime());
            SetFingerGrabberForce(GRIP_MAX_TORQUE);
            //	simSetJointForce(_iGripJointHandles[1], GRIP_MAX_TORQUE);

            //????? ???????? ?????¥ä?.
            if (_fMotorGrip_nDirection == GRIP_ON)
            {
                float maxiTorque = 0;

                GetFingerGrabberForce(maxiTorque);

                //	???? ??? ????? ????? ???????? ????? ????? ????? ??????? ???.
                if (fabs(maxiTorque) < GRIP_STOP_TORQUE)
                {
                    _fCurrentGripperValue += fDeltaAng;
                }
                else // ??????? ??????? ???? ??????? ???????? ??? ???? ???? ??????? ??? ???? ???®œ ??????? ???? ????.
                {
                    _fTargetGripperValue = _fCurrentGripperValue;
                    bMoveComplete = true;
                }
            }
            else if (_fMotorGrip_nDirection == GRIP_OFF)
            {
                _fCurrentGripperValue -= fDeltaAng;
                TurnOffSuction();
            }

            if (_fCurrentGripperValue >= _fTargetGripperValue && _fMotorGrip_nDirection == GRIP_ON)
            {
                _fCurrentGripperValue = _fTargetGripperValue;
                bMoveComplete = true;
            }
            else if (_fCurrentGripperValue <= _fTargetGripperValue && _fMotorGrip_nDirection == GRIP_OFF)
            {
                _fCurrentGripperValue = _fTargetGripperValue;
                bMoveComplete = true;
            }

            if (bMoveComplete)
            {
                if (_fMotorGrip_nDirection == GRIP_ON)
                {
                    TurnOnSuction();
                    GrapObstacles(_fCurrentGripperValue);
                }
                else if (_fMotorGrip_nDirection == GRIP_OFF)
                {
                    //TurnOffSuction();
                    ReleaseObstacles(_fCurrentGripperValue);
                }

                setCheckingReachTheRange(GRIP, false);
                return true;
            }

            return false;
    }
    else
    {
        setCheckingReachTheRange(GRIP, false);
        return true;
    }
#ifdef QT_COMPIL
    qDebug()<<("Derive this GripCommand function to move grip finger \r\n");
#else
    TRACE("Derive this GripCommand function to move grip finger \r\n");
#endif
    return false;
}

bool CKss1500RobotMotion::Motor1InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode)
{
    static float _fMotor1_nDirection = 0;

    if (!getCheckingReachTheRange(MOTOR_1))
    {
        if (!CheckMaxMinRangeAndSpeed(MOTOR_1, MotorSpeed, MotorRange, TPMode))
            return true;

        float fMotor1_fMoveAng;

        //?????? ??? ?????? ???

        _fTargetJointVelocity[0] = (float)(degToRad*MotorSpeed*JOINTSPEED);
        _fTargetJointValue[0] = (float)(degToRad*MotorRange);

        //?????? ?????? ??????? ??????  ???? ???????? ????????? ???????.
        //ex> ??????? : 45??, ??????? : 90?? --> 45???? ???????? ???.
        //??? ?????? ?????? ??
        if (((_fCurrentJointValue[0] >= 0) && (_fTargetJointValue[0] >= 0)) || ((_fCurrentJointValue[0] < 0) && (_fTargetJointValue[0] < 0)))
        {
            if (_fCurrentJointValue[0] >= _fTargetJointValue[0])
                _fMotor1_nDirection = LEFT;
            else
                _fMotor1_nDirection = RIGHT;

            fMotor1_fMoveAng = fabs(_fTargetJointValue[0] - _fCurrentJointValue[0]);
        }
        else
        {
            //??????? ???????? ??????? ?????????? ???
            if (_fTargetJointValue[0] <= 0)
                _fMotor1_nDirection = LEFT;
            else
                _fMotor1_nDirection = RIGHT;
            fMotor1_fMoveAng = fabs(_fTargetJointValue[0]) + fabs(_fCurrentJointValue[0]);
        }

        //???? ?????? o??(???)??? ???? ?©£?
        setCheckingReachTheRange(MOTOR_1, true);
    }

    float fDeltaAng = 0;


    fDeltaAng = _fTargetJointVelocity[0] *_fDeltaTime;
    //	fDeltaAng	= degToRad*_fTargetJointVelocity[0]*_fDeltaTime;
    //	??? ???????? ????? ?????¥ä?.
    if (_fMotor1_nDirection == RIGHT) {
        _fCurrentJointValue[0] += fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
        if (_fTargetJointValue[0] <= _fCurrentJointValue[0])
        {
            _fCurrentJointValue[0] = _fTargetJointValue[0];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_1, false);
            return true;
        }
    }
    else if (_fMotor1_nDirection == LEFT) {
        _fCurrentJointValue[0] -= fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
        if (_fTargetJointValue[0] >= _fCurrentJointValue[0])
        {
            _fCurrentJointValue[0] = _fTargetJointValue[0];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_1, false);
            return true;
        }
    }

    return false;
}

bool CKss1500RobotMotion::Motor2InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode)
{
    static float _fMotor2_nDirection = 0;

    if (!getCheckingReachTheRange(MOTOR_2))
    {
        if (!CheckMaxMinRangeAndSpeed(MOTOR_2, MotorSpeed, MotorRange, TPMode))
            return true;

        float fMotor1_MoveTime;
        float fMotor1_fMoveAng;
//		simGetJointPosition(_iDriveJointHandles[1], &_fCurrentJointValue[1]);
        //?????? ??? ?????? ???
        _fTargetJointVelocity[1] = (float)(degToRad*MotorSpeed*JOINTSPEED);
        _fTargetJointValue[1] = (float)(degToRad*MotorRange);

        //?????? ?????? ??????? ??????  ???? ???????? ????????? ???????.
        //ex> ??????? : 45??, ??????? : 90?? --> 45???? ???????? ???.
        //??? ?????? ?????? ??
        if (((_fCurrentJointValue[1] >= 0) && (_fTargetJointValue[1] >= 0)) || ((_fCurrentJointValue[1] < 0) && (_fTargetJointValue[1]< 0)))
        {
            if (_fCurrentJointValue[1] >= _fTargetJointValue[1])
                _fMotor2_nDirection = LEFT;
            else
                _fMotor2_nDirection = RIGHT;

            fMotor1_fMoveAng = fabs(_fTargetJointValue[1] - _fCurrentJointValue[1]);
        }
        else
        {
            //??????? ???????? ??????? ?????????? ???
            if (_fTargetJointValue[1] <= 0)
                _fMotor2_nDirection = LEFT;
            else
                _fMotor2_nDirection = RIGHT;
            fMotor1_fMoveAng = fabs(_fTargetJointValue[1]) + fabs(_fCurrentJointValue[1]);
        }

        //???? ?????? o??(???)??? ???? ?©£?
        fMotor1_MoveTime = abs(fMotor1_fMoveAng / _fTargetJointVelocity[1]);

        //?????????? ?????? ?????? ?? ????? ???????.
        setCheckingReachTheRange(MOTOR_2, true);
    }

    float fDeltaAng = 0;

    fDeltaAng = _fTargetJointVelocity[1]*_fDeltaTime;
    //	fDeltaAng	= degToRad*_fTargetJointVelocity[0]*_fDeltaTime;
    //	??? ???????? ????? ?????¥ä?.
    if (_fMotor2_nDirection == RIGHT) {
        _fCurrentJointValue[1] += fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);

        if (_fTargetJointValue[1] <= _fCurrentJointValue[1])
        {
            _fCurrentJointValue[1] = _fTargetJointValue[1];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_2, false);
            return true;
        }
    }
    else if (_fMotor2_nDirection == LEFT) {
        _fCurrentJointValue[1] -= fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);

        if (_fTargetJointValue[1] >= _fCurrentJointValue[1])
        {
            _fCurrentJointValue[1] = _fTargetJointValue[1];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_2, false);
            return true;
        }
    }

    return false;
}

bool CKss1500RobotMotion::Motor3InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode)
{
    static int _fMotor3_nDirection = 0;

    if (!getCheckingReachTheRange(MOTOR_3))
    {
        if (!CheckMaxMinRangeAndSpeed(MOTOR_3, MotorSpeed, MotorRange, TPMode))
            return true;

        float fMotor1_MoveTime;
        float fMotor1_fMoveAng;
        float fZdispVelocity = MotorRange / _fDeltaTime;

        _fTargetJointVelocity[2] = MotorSpeed*JOINTSPEED_Z_AXIS;
        _fTargetJointValue[2] = MotorRange;
        _XyzTargetPosition.Z = MotorRange;
        _fTargetJointValue[2] = (float)((MotorRange / 118)*0.118);

        if (_fCurrentJointValue[2] < _fTargetJointValue[2])
            _fMotor3_nDirection = UP;
        else
            _fMotor3_nDirection = DOWN;

        fMotor1_fMoveAng = fabs(_fTargetJointValue[2] - _fCurrentJointValue[2]);
        fMotor1_MoveTime = fabs(fMotor1_fMoveAng / _fTargetJointVelocity[2]);
        setCheckingReachTheRange(MOTOR_3, true);
    }

    float fDeltaAng = 0;


    fDeltaAng = _fTargetJointVelocity[2] *_fDeltaTime;
    //			fDeltaAng	= degToRad*_fTargetJointVelocity[2]*fDeltaTime;
    //??? ???????? ????? ?????¥ä?.
    if (_fMotor3_nDirection == UP) {
        _fCurrentJointValue[2] += fDeltaAng;
        _XyzCurrentPosition.Z += fDeltaAng*1000;

        if (_fTargetJointValue[2] <= _fCurrentJointValue[2])
        {
            _fCurrentJointValue[2] = _fTargetJointValue[2];
            _XyzCurrentPosition.Z = _XyzTargetPosition.Z;
            setCheckingReachTheRange(MOTOR_3, false);
            return true;
        }
    }
    else if (_fMotor3_nDirection == DOWN) {
        _fCurrentJointValue[2] -= fDeltaAng;
        _XyzCurrentPosition.Z -= fDeltaAng*1000;

        if (_fTargetJointValue[2] >= _fCurrentJointValue[2])
        {
            _fCurrentJointValue[2] = _fTargetJointValue[2];
            _XyzCurrentPosition.Z = _XyzTargetPosition.Z;
            setCheckingReachTheRange(MOTOR_3, false);
            return true;
        }
    }

    return false;
}

bool CKss1500RobotMotion::MotorRotateCommand(float MotorSpeed, float MotorRange)
{
    static float _fMotorRotate_TargetVelocity = 0;
    static float _fMotorRotate_nDirection = 0;
    static float _fMotorRotate_CurrentTime = 0;

    if (!CheckMaxMinRangeAndSpeed(MOTOR_ROTATE, MotorSpeed, MotorRange, false))
        return false;

    if (!getCheckingReachTheRange(MOTOR_ROTATE))
    {
        float fMotor1_MoveTime;
        float fMotor1_fMoveAng;

        _fMotorRotate_TargetVelocity = (float)((3.14 / 180)*MotorSpeed) * 10;
        SetTargetRaxisValueByDegree(MotorRange);

        //?????? ?????? ??????? ??????  ???? ???????? ????????? ???????.
        //ex> ??????? : 45??, ??????? : 90?? --> 45???? ???????? ???.
        //??? ?????? ?????? ??
        if (((_fCurrentJointValue[3] >= 0) && (_fTargetJointValue[3] >= 0)) || ((_fCurrentJointValue[3] < 0) && (_fTargetJointValue[3]< 0)))	//??? ?????? ?????? ??
        {
            if (_fCurrentJointValue[3] > _fTargetJointValue[3])
                _fMotorRotate_nDirection = LEFT;
            else
                _fMotorRotate_nDirection = RIGHT;

            fMotor1_fMoveAng = abs(_fTargetJointValue[3] - _fCurrentJointValue[3]);
        }
        else
        {
            //??????? ???????? ??????? ?????????? ???
            if (_fTargetJointValue[3] < 0)
                _fMotorRotate_nDirection = LEFT;
            else
                _fMotorRotate_nDirection = RIGHT;
            fMotor1_fMoveAng = abs(_fTargetJointValue[3]) + abs(_fCurrentJointValue[3]);
        }
        //???? ?????? o??(???)??? ???? ?©£?
        fMotor1_MoveTime = abs(fMotor1_fMoveAng / _fMotorRotate_TargetVelocity);
        //?????????? ?????? ?????? ?? ????? ???????.
        setCheckingReachTheRange(MOTOR_ROTATE, true);
    }

    float fDeltaAng = 0;

    fDeltaAng = _fMotorRotate_TargetVelocity*_fDeltaTime;
    //			fDeltaAng	= degToRad*_fTargetJointVelocity[2]*_fDeltaTime;
    //??? ???????? ????? ?????¥ä?.
    if (_fMotorRotate_nDirection == RIGHT) {
        _fCurrentJointValue[3] += fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
        if (_fTargetJointValue[3] < _fCurrentJointValue[3])
        {
            _fCurrentJointValue[3] = _fTargetJointValue[3];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_ROTATE, false);
            return true;
        }
    }
    else if (_fMotorRotate_nDirection == LEFT) {
        _fCurrentJointValue[3] -= fDeltaAng;
        ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
        if (_fTargetJointValue[3] > _fCurrentJointValue[3])
        {
            _fCurrentJointValue[3] = _fTargetJointValue[3];
            ForwardKinematics(_fCurrentJointValue, _XyzCurrentPosition);
            setCheckingReachTheRange(MOTOR_ROTATE, false);
            return true;
        }
    }
    return false;
}

bool CKss1500RobotMotion::DetermineXYvelocities(float v, float Vr[2])
{
    /* Going to somewhere for "deltaTime" seconds */
    double BlockLength = 0.0f;
    float Xf[2] = {_XyzTargetPosition.X,_XyzTargetPosition.Y};
    float Xi[2] = {_XyzCurrentPosition.X,_XyzCurrentPosition.Y};
    float l[2] = {0,0};

    for(int i = 0;i<2 ;i++)
    {
        // Axis travel lengths
        l[i]=fabs(Xf[i]-Xi[i]);
        BlockLength += l[i]*l[i];
    }

    //  unit velocity vector
    BlockLength=sqrt(BlockLength);

    if (BlockLength==0.)	// length is 0
        return false;

    for(int i = 0;i<2 ;i++)
    {
        // axis refrence velocities [x,y]
        Vr[i]=(l[i]/BlockLength)*v;
    }
    return true;
}

