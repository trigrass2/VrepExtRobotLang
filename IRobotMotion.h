#pragma once
#include "CommonH.h"
#include "RobotLangExport.h"

//	An  I/F class for robot movement. Each functions should be redefined, if you should add additional robot or something.
class VREP_ROBOTLANG_API IRobotMotion
{

public:
	virtual bool GripCommand(float MotorRange) = 0;
	virtual bool MoveKinematicGripperFinger(float amount) = 0;		//	Need a Vrep API for iRoDi's gripper move command that has dynamic finger
	virtual bool MoveDynamicGripperFinger(float amount) = 0;		//	Need a Vrep API for iRoDi's gripper move command that has normal finger	that works kinematically
	virtual bool GrapObstacles(float detectableValue) = 0;			//	Codes for grapping obstacles.
	virtual bool ReleaseObstacles(float detectableValue) = 0;		//	Codes for releasing obstacles.
	virtual bool GetFingerGrabberForce(float &maxiTorque) = 0;		//	Get Force on a finger of gripper with V-Rep Api
	virtual bool SetFingerGrabberForce(float maxiTorque) = 0;		//	Set Force on a finger of gripper with V-Rep Api	
	virtual bool TurnOffSuction(void) = 0;							//	Suck an obstacle
    virtual bool TurnOnSuction(void) = 0;							//	Release an obstacle
	virtual void SetCurrentJointPositions(const CPoint3D&) = 0;		//	Set current robot position
	virtual CPoint3D GetCurrentJointPositions(void) = 0;			//	Get current robot position
	virtual void SetTargetJointPositions(const CPoint3D&) = 0;		//	Set target postition
	virtual bool DriveCommand(int MotorNumber, float MotorSpeed, float MotorRange) = 0;						// Dirve motors
	virtual bool HomeCommand(void) = 0;																		// Return to home
	virtual	bool RotateCommand(float MotorRange) = 0;														// Rotate an rotational axis
	virtual bool InterpreteMotion(float MotorSpeed, float X_Axis, float Z_Axis, float Y_Axis, int type) = 0;// General interpolation function
	virtual bool UpdateIOport(int input, int output, int ChangedOutput = 0) = 0;
	virtual bool GetTotalErrorCodes(std::vector<int>& variable, std::vector<int>& value) = 0;
	virtual bool GetTotalWarningMessages(std::vector<int>& variable, std::vector<int>& value) = 0;
    virtual bool SetCustomMotionData(std::string&) = 0;
    virtual void GetCurrentJointValues(int size, float values[]) = 0;
    virtual void GetCurrentJointVelocities(int size, float values[]) = 0;
    virtual int  GetRobotRunStatus(void) = 0;                       //  Get robot interpreter status
    virtual void SetRobotRunMode(int) = 0;                          //  Set robot running mode
    virtual void SetProgram(const std::string&)= 0;                 //  Set Robot Program
    virtual float GetEuclidSpeed(void) = 0;                         //  Get concurrent speed
    virtual void SetEuclidSpeed(float) = 0;                         //  Set concurrent speed
    virtual int GetInputPort(void) = 0;                           //  Get Inport status
    virtual int GetOutputPort(void) = 0;                          //  Get Outport status
    virtual float GetCurrentRaxisValueByDegree() = 0;               //  Get rotary axis value (unit : degree)
    virtual float GetCurrentGripperValueByDegree() = 0;             //  Get gripper value (unit : degree)
    virtual void EnableManualControlPanel(bool) = 0;                   //   Show/hide Teach Pendant
    virtual int GetSendingIoSignalType(void) = 0;
};

