#include "CommonH.h"
#include <math.h>
#include "Kss1500MoveRange.h"


CKss1500MoveRange::CKss1500MoveRange(void)
{
}


CKss1500MoveRange::~CKss1500MoveRange(void)
{
}


void CKss1500MoveRange::DegreeToRadian(float & joints)
{
	joints = joints * 3.1415927f/180;	
}

void CKss1500MoveRange::RadianToDegree(float & joints)
{
	joints = joints * 180.0f/ 3.14159265358979323846f;
}


float CKss1500MoveRange::DegToRad(float joints)
{
	joints = joints * 3.14159265358979323846f/180.0f;
	return joints;
}

float CKss1500MoveRange::RadToDeg(float joints)
{
	joints = joints * 180/3.1415927f;
	return joints;
}

bool CKss1500MoveRange::Inversekinematics(const float coord[3], float joints[3])
{
	double x2 = coord[1];		//YÁÂÇ¥ 
	double y2 = coord[0];		//XÁÂÇ¥ 
	//Z ÁÂÇ¥ »ç¿ë ¾ÈÇÔ

	double L1= 0;
	double L2= 0;

	L1= 200.0;
	L2= 200.0;

	if(coord[0] > 0)
		y2 *= -1;

	double d = (x2*x2 + y2*y2 - L1*L1 - L2*L2) / (2*L1*L2);
	double d1, d2 = 0.f;
	if(x2<0.0) d2 = atan2(sqrt(1-d*d),d);
	else d2 = atan2(-1.0*sqrt(1-d*d),d);
	d2 = atan2(-1.0*sqrt(1-d*d),d);
	d1 = atan2(y2,x2) - atan2((L2*sin(d2)),(L1+L2*cos(d2)));
	joints[0] = (float)d1;
    joints[1] = (float)d2;

	if(d1 < -9.8) {
		d1 = 360.0 + d1;
		joints[0] = (float)(2*PI+d1/180*PI);
	}
	RadianToDegree(joints[0]);
	RadianToDegree(joints[1]);
    joints[2] = coord[2]/1000;

	if(coord[0] > 0){
		joints[0] *= -1; 
		joints[1] *= -1;
	}

    if((joints[0]>= MOTOR1_LIMIT_M && joints[0]<= MOTOR1_LIMIT_P)
            &&(joints[1]>= MOTOR2_LIMIT_M && joints[1]<= MOTOR2_LIMIT_P)
            &&(joints[2]>= MOTORZ_LIMIT_M && joints[2]<= MOTORZ_LIMIT_P))
	{	
		return true;
	}

	return false;
}

void CKss1500MoveRange::ForwardKinematics(const float joint[3], float coord[3])
{

	double L1= 0;
	double L2= 0;
	
		L1= 200.0;
		L2= 200.0;

	const double joint_1 = joint[0];
	const double joint_2 = joint[1];

	coord[0] = (float)(L1 * cos(joint_1) + L2 * cos(joint_1+joint_2));
	coord[1] = (float)(L1 * sin(joint_1) + L2 * sin(joint_1+joint_2));

	float temp;

	temp = coord[0];
	coord[0] = coord[1];
	coord[1] = temp;
    coord[2] = joint[2]*1000;

}


void CKss1500MoveRange::ForwardKinematics(const std::vector<float>& joint, CPoint3D& coord)
{

	double L1 = 0;
	double L2 = 0;

	L1 = 200.0;
	L2 = 200.0;

	const double joint_1 = joint[0];
	const double joint_2 = joint[1];

	coord.X = (float)(L1 * cos(joint_1) + L2 * cos(joint_1 + joint_2));
	coord.Y = (float)(L1 * sin(joint_1) + L2 * sin(joint_1 + joint_2));

	float temp;

	temp = coord.X;
	coord.X = coord.Y;
	coord.Y = temp;
    coord.Z = joint[2]*1000.0f;

}

bool CKss1500MoveRange::ValidateMove(double txpos, double typos, double tzpos)
{
	double mt1_target_angle, mt2_target_angle; 
	double d,f,t1,t2,mt1_angr,mt2_angr;

	d = ((txpos*txpos) + (typos*typos) -(MOTOR1_ARM_LENGTH*MOTOR1_ARM_LENGTH) -(MOTOR2_ARM_LENGTH*MOTOR2_ARM_LENGTH)) / (2.0*MOTOR1_ARM_LENGTH*MOTOR2_ARM_LENGTH);

	if( 1 - d*d < 0 ) return false; 

	f = sqrt(1-d*d);

	mt2_angr = atan2(f,d);

	t1 = atan2(typos,txpos);

	t2 = atan2(MOTOR2_ARM_LENGTH*sin(mt2_angr),MOTOR1_ARM_LENGTH+MOTOR2_ARM_LENGTH*cos(mt2_angr));
	mt1_angr = t1-t2;

	if((mt1_angr * 180.0 / PI) <= -90.0)
	{
		mt1_angr += (360.0 * PI / 180.0);

		if((90.0 - mt1_angr * 180.0 / PI) < MOTOR1_HOMEANGLE ||(90.0 - mt1_angr * 180.0 / PI) > (MOTOR1_HOMEANGLE*-1.0))
		{
			mt2_angr *= -1.0;
			t1 = atan2(typos,txpos);
			t2 = atan2(MOTOR2_ARM_LENGTH*sin(mt2_angr),MOTOR1_ARM_LENGTH+MOTOR2_ARM_LENGTH*cos(mt2_angr));
			mt1_angr = t1-t2;
		}
	}
	else if((90.0 - mt1_angr * 180.0 / PI) > (MOTOR1_HOMEANGLE*-1.0) && (mt1_angr * 180.0 / PI) > -90.0)
	{
		mt2_angr *= -1.0;
		t1 = atan2(typos,txpos);
		t2 = atan2(MOTOR2_ARM_LENGTH*sin(mt2_angr),MOTOR1_ARM_LENGTH+MOTOR2_ARM_LENGTH*cos(mt2_angr));
		mt1_angr = t1-t2;
	}
	
	mt1_target_angle = mt1_angr * 180.0 / PI;
	mt2_target_angle = mt2_angr * 180.0 / PI * -1.0;
	mt1_target_angle = 90.0 -mt1_target_angle;
	if(mt1_target_angle < MOTOR1_HOMEANGLE)
	{
		mt1_target_angle += 270.0;
		mt2_target_angle *= -1.0;
	}

    if( (MOTOR1_LIMIT_M <= mt1_target_angle && mt1_target_angle <= MOTOR1_LIMIT_P) &&
        (MOTOR2_LIMIT_M <= mt2_target_angle && mt2_target_angle <= MOTOR2_LIMIT_P) )
	{
		return true; 
	}

	return false; 
}


bool CKss1500MoveRange::ValidateDrive(int no, float v)
{
	if( no == 1)
	{
        if( v < MOTOR1_LIMIT_M || MOTOR1_LIMIT_P > v )
			return false; 
	}
	else if( no == 2)
	{
        if( v < MOTOR2_LIMIT_M || MOTOR2_LIMIT_P > v )
			return false; 
	}
	else if( no == 3)
	{
        if( v < MOTORZ_LIMIT_M || MOTORZ_LIMIT_P > v )
			return false; 
	}
	else 
		return false; 

	return true; 
}

bool CKss1500MoveRange::ValidateRotate(float v)
{
    if( v < THETA_LIMIT_M || THETA_LIMIT_P < v )
		return false; 

	return true; 
}

bool CKss1500MoveRange::ValidateGrip(float v) 
{
    if( v < GRIP_LIMIT_M || GRIP_LIMIT_P < v )
		return false; 

	return true; 
}

bool CKss1500MoveRange::ValidateSpeed(float v)
{
    if( v < SPEED_LIMIT_M || SPEED_LIMIT_P < v )
		return false; 

	return true; 
}

