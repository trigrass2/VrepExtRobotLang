#pragma once

#include <vector>
#include "VrepExtExport.h"
#include "IRobotMotion.h"
#include "Kss1500MoveRange.h"

//***********************************


class VREP_ROBOTLANG_API CKss1500RobotMotion : public IRobotMotion, public CKss1500MoveRange
{

public:
	//�� �� �� �κ� ��ɾ ����
	enum Kss1500MoveType {
        MOTOR_ROTATE = 0, MOTOR_1 = 1, MOTOR_2 = 2, MOTOR_3 = 3, GRIP = 4, MOVE = 5,
        ROTATE = 6, DRIVE = 7, HOME = 8,MOVE_JOINT = 9, MOVE_JOG = 10,
        MOTOR_3_JOG = 11, MOVE2 = 12, COMMAND_MOVE = 13, TP_MOVE = 14,
	};

public:
	
	//  �⺻ ��⿡ �ִ� ���. �Ʒ� �����Լ�����, ���ο� �κ��� �߰��� ��� �� �����Ͽ� ����Ѵ�. 
	CKss1500RobotMotion();
	virtual ~CKss1500RobotMotion();
	virtual bool GetFingerGrabberForce(float &maxiTorque);
	virtual bool SetFingerGrabberForce(float maxiTorque);
	virtual bool GrapObstacles(float detactableValue);
	virtual bool ReleaseObstacles(float detectableValue);
	virtual bool MoveKinematicGripperFinger(float amount);
	virtual bool MoveDynamicGripperFinger(float amount);
	virtual bool TurnOffSuction(void);
    virtual bool TurnOnSuction(void);
	virtual bool DriveCommand(int MotorNumber, float MotorSpeed, float MotorRange);				// FlowChart�� Drive ��ɾ� �����ϴ� �Լ�
	virtual bool HomeCommand(void);																// FlowChart�� Home ��ɾ� �����ϴ� �Լ�
	virtual	bool RotateCommand(float MotorRange);												// FlowChart�� Rotate ��ɾ� �����ϴ� �Լ�
	virtual bool InterpreteMotion(float MotorSpeed, float X_Axis, float Z_Axis, float Y_Axis, int type);	// FlowChart�� Move ��ɾ� �����ϴ� �Լ�
    virtual void EnableManualControlPanel(bool sw);                                             //  TP control
    bool InterpreteMotion(float MotorSpeed, float X_Axis, float Y_Axis, float Z_Axis, float R_Axis, float Gripper, int type);
	void GetCurrentJointValues(int index, float values[]);
    void SetCurrentJointValues(float values[], int size = TOTAXES);
    void SetTargetJointValues(int size, float value[]);
	float GetCurrentJointValue(int index);
	void SetCurrentXaxisPos(const float);
	void SetCurrentYaxisPos(const float);
	void SetCurrentZaxisPos(const float);
	void SetCurrentRaxisPos(const float);	
	float GetCurrentRaxisPos(void);
	float GetCurrentGripperPos(void);
	void SetCurrentGripperPos(const float);
	void SetCurrentJointPositions(const CPoint3D&);
	CPoint3D GetCurrentJointPositions(void);
	void SetTargetJointPositions(const CPoint3D&);
	void SetTargetJointValue(int, float);
	void SetTargetJointValue(float value1, float value2, float value3, float value4);
	bool GetProtectMoveStatus();
	void SetProtectMoveStatus(bool LockOrUnlock);
	void GetTargetJointValue(std::vector<float>& target) { target = _fTargetJointValue; }
	const CPoint3D& GetTargetJointPositions(void);
	void SetTargetRaxisValue(const float);
	float GetTargetRaxisValue(void); 
	float GetTargetRaxisValueByDegree(void);
    float GetCurrentRaxisValueByDegree(void);
    float GetCurrentGripperValueByDegree();
	void SetTargetRaxisValueByDegree(const float);
	float GetTargetGripperValueByDegree(void);
    void SetTargetGripperValueByDegree(const float pos);
    void SetCurrentGripperValueByDegree(const float pos);
    void GetCurrentJointVelocities(int size, float values[]);
    bool DetermineXYvelocities(float MotorSpeed, float MotorSpeeds[2]);

	//��� ��ɾ���� ���� �Լ�
	bool MoveCommand(Kss1500MoveType cmd, int MotorNumber, float MotorSpeed, float Distance_1, float Distance_2, float Distance_3, float Distance_4);
	bool GripCommand(float MotorRange);													// FlowChart�� Grip ��ɾ� ����
	bool Motor1InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode);		// 1���� �������� ����
	bool Motor2InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode);		// 2���� �������� ����
	bool Motor3InterpreteMotion(float MotorSpeed, float MotorRange, bool TPMode);		// 3���� �������� ����
	bool MotorRotateCommand(float MotorSpeed, float MotorRange);							// Rotate�� �������� ����
	bool CheckMaxMinRangeAndSpeed(int MotorNumber, float& MotorSpeed, float& MotorRange, bool TPMode);	//�� ���� �ӵ� �� Range�� üũ - Ư�������� ������ false �� �����Ѵ�.
	void ResetAngleSW();																//	Reset Angle Switch
		
	//  KSS1500 ���� �Լ����
	void inializeAll();					// ��� �ʱ�ȭ
	void initializeRobot();
	void defaultFirstSetting();			// Home ���� �� Step�� �ʱ�ȭ
							
	bool getSimulationMoveCommandFinish();
	void setSimulationMoveCommandFinish(bool status);
	const bool getCheckingReachTheRange(int MotorType);
	void setCheckingReachTheRange(int MotorType, bool statue);

	//********KSS1500 �κ� ���� ��ɾ�********
	void SetDeltaTime(float time) { _fDeltaTime = time; }
	float GetDeltaTime(void) {return _fDeltaTime;}
	bool InterpreteMotion(float MotorSpeed, float X_Axis, float Y_Axis, float Z_Axis, float R_Axis, int type);
	bool driveConverteCommand(int MotorNumber, float MotorSpeed, float & Distance_1);	// Drive ��� ���� �� ���� �κ����� �������� �����Ѵ�.

protected:
	float _fEuclidFeed;
	float _fDeltaTime;

	// Some states:
	bool _bTeachMode;
	bool _bMoveStatus;
	bool _bSimulationMoveCommandFinish;
	
	//������ Limit ����
	bool _iMotor1_FirstSetting;
	bool _iMotor2_FirstSetting;
	bool _iMotor3_FirstSetting;
	bool _iMotorRotate_FirstSetting;
	bool _bMotorGripFirstSetting;
	bool _bMotorMoveFirstSetting;

	//home
	int _iHomeStep;
	bool _bFinishGrip;
	bool _FinishRotate;

	//	Motors
	std::vector<float> _fCurrentJointValue;
	std::vector<float> _fTargetJointValue;
	std::vector<bool> _bFinishJointValue;
	std::vector<float> _fTargetJointVelocity;

	//Grip
	float _fCurrentGripperVelocity;
	float _fCurrentGripperValue;
	float _fTargetGripperValue;
	
	//	MOVE
	CPoint3D _XyzTargetPosition;
	CPoint3D _XyzCurrentPosition;
	
	bool _bLimitStatus[3];
};
