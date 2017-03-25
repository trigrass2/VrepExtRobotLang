#pragma once

#include "VrepExtExport.h"
#include "CommonH.h"

//	Check move range with Kinematics
class VREP_ROBOTLANG_API CKss1500MoveRange
{
public:
	CKss1500MoveRange(void);
	~CKss1500MoveRange(void);

	bool ValidateMove(double txpos, double typos, double tzpos);
	bool ValidateDrive(int no, float v);
	bool ValidateRotate(float v);
	bool ValidateGrip(float v) ;
	bool ValidateSpeed(float v);
	void ForwardKinematics(const float joint[3], float coord[3]);
	void ForwardKinematics(const std::vector<float>& joint, CPoint3D& coord);
	bool Inversekinematics(const float coord[3], float joints[3]);
	void  DegreeToRadian(float & joints);
	void  RadianToDegree(float & joints);
	float DegToRad(float joints);
	float RadToDeg(float joints);

};

