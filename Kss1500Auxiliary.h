#pragma once

#include "VrepExtExport.h"
#include "CommonH.h"
#include <string>

class VREP_ROBOTLANG_API CKss1500Auxiliary
{
public:

	// Elimination start
	float robotVelocity[2]; // The velocity to use for robot movements (cartesian and angular)
	float desiredConfigAtCurrentProgramLine[12];
	std::string _compilationErrorMessage;
	float _initialRobotVelocity[2];
	int _tipZeroHandle;
	int _ikTipHandle;
	int _ikTargetHandle;
	int _wristHandle;
	int _ikGroupHandle;

	int _ikFailedDialogId;
	int _ikFailedDialogStartShowTime;
	int _jointHandles[6];
	// couild be eliminated

	// 2014_1121 add 
	//----- linear	// X Pos, Y Pos, Z Pos
	float m_fPosVel[3];
	GEO m_OrgPos, m_CurPos, m_TarPos; 
	void InitPos(float o_x, float o_y, float o_z, float t_x, float t_y, float t_z); 
	void SetPos(float t_x, float t_y, float t_z);
	void SetPosX(float t_x);
	void SetPosY(float t_y);
	void SetPosZ(float t_z);
	void SetIncPos(float t_x, float t_y, float t_z);
	void SetCurPos(float t_x, float t_y, float t_z);
	void GetPos(float &x, float &y, float &z);
	bool MovePos(float deltaTime, float tar, float &cur, float vel); 

	//----- rotate	// 1√‡, 2√‡, Rotate√‡
	float m_fRotVel[3];
	ROT m_OrgRot, m_CurRot, m_TarRot; 
	void InitRot(float o_r1, float o_r2, float o_r3); //, float t_r1, float t_r2, float t_r3); 
	void SetRot(float t_r1, float t_r2, float t_r3);
	void SetRotR3(float t_r3);
	void SetIncRotR3(float t_r3);
	void SetIncRot(float t_r1, float t_r2, float t_r3);
	void SetCurRot(float t_r1, float t_r2, float t_r3); 
	void GetRot(float &r1, float &r2, float &r3);
	bool MoveRot(float deltaTime, float tar, float &cur, float vel); 

	//----- grip Out
	float m_fGrpOutVel;
	float m_fOrgGrpOutPos, m_fCurGrpOutPos, m_fTarGrpOutPos; 
	void InitGrpOut(float o_pos);
	void SetGrpOut(float t_pos);
	void SetIncGrpOut(float t_pos);
	void GetGrpOut(float &pos);
	bool MoveGrpOut(float deltaTime, float tar, float &cur, float vel); 


	CKss1500Auxiliary(void);
	~CKss1500Auxiliary(void);
};

