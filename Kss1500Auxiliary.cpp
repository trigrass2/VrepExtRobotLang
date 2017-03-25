#include "StdAfx.h"
#include "Kss1500Auxiliary.h"
#include <math.h>

CKss1500Auxiliary::CKss1500Auxiliary(void)
{
}


CKss1500Auxiliary::~CKss1500Auxiliary(void)
{
}


//-----------------------------------------------------------------------------------------------
//----- linear 

void CKss1500Auxiliary::InitPos(float o_x, float o_y, float o_z, float t_x, float t_y, float t_z)
{
	m_OrgPos.fX = o_x; 
	m_OrgPos.fY = o_y; 
	m_OrgPos.fZ = o_z; 
	m_CurPos.fX = t_x; 
	m_CurPos.fY = t_y; 
	m_CurPos.fZ = t_z; 
	m_TarPos = m_CurPos;
	//m_TarPos.fX += 0.5;
}


void CKss1500Auxiliary::SetPos(float t_x, float t_y, float t_z)
{
	m_TarPos.fX = t_x; 
	m_TarPos.fY = t_y; 
	m_TarPos.fZ = t_z; 
}

void CKss1500Auxiliary::SetCurPos(float t_x, float t_y, float t_z)
{
	m_CurPos.fX = t_x; 
	m_CurPos.fY = t_y; 
	m_CurPos.fZ = t_z; 
}

void CKss1500Auxiliary::SetPosX(float t_x)
{
	m_TarPos.fX = t_x; 
}

void CKss1500Auxiliary::SetPosY(float t_y)
{
	m_TarPos.fY = t_y; 
}

void CKss1500Auxiliary::SetPosZ(float t_z)
{
	m_TarPos.fZ = t_z; 
}

void CKss1500Auxiliary::SetIncPos(float i_x, float i_y, float i_z)
{
	m_TarPos.fX = m_TarPos.fX + i_x; 
	m_TarPos.fY = m_TarPos.fY + i_y; 
	m_TarPos.fZ = m_TarPos.fZ + i_z; 
}

void CKss1500Auxiliary::GetPos(float &x, float &y, float &z)
{
	//m_Coord.fX += 0.001;

	x = m_CurPos.fX; 
	y = m_CurPos.fY; 
	z = m_CurPos.fZ; 
}

// b[0] = MovePos(deltaTime, m_TarPos.fX, m_CurPos.fX, m_fPosVel[0]); 
bool CKss1500Auxiliary::MovePos(float deltaTime, float tar, float &cur, float vel)
{
	if( fabs(tar - cur) < 0.00001 ) 
		return true; 

	//m_bIK = true; 

	float inc_cur=0.0f;
	float deltaTimeLeft=0.0f;
	float biggestT=0.0f;
	float dj=0.0f;

	while( 1 )
	{
		dj= tar - cur; 

		float vv=vel; // 0 is x 
		float t=fabs(dj)/vv;

		if (t>biggestT)
		{
			biggestT=t; // biggestT == 남아있는 시간 
		}
		
		if (biggestT>deltaTime ) 
		{
			if (dj != 0.0f)
			{
				float vv=fabs(dj)/biggestT;
				inc_cur = vv*deltaTime*dj/fabs(dj);
				cur += inc_cur;

			}
			deltaTimeLeft=0.0f;
		}
		else
		{
			dj = tar - cur;
			inc_cur = dj;
			cur += inc_cur; 

			deltaTimeLeft=deltaTime-biggestT;
		}
		
		deltaTime=deltaTimeLeft;
		if (deltaTime>0.0f)
		{				
			int i=0;
		}
		else
			break;
	}

	return false; 
}


//-----------------------------------------------------------------------------------------------
//----- rotate
void CKss1500Auxiliary::InitRot(float o_r1, float o_r2, float o_r3) //, float t_r1, float t_r2, float t_r3) 
{
	m_OrgRot.fR1 = o_r1; 
	m_OrgRot.fR2 = o_r2; 
	m_OrgRot.fR3 = o_r3; 
	m_CurRot.fR1 = o_r1; 
	m_CurRot.fR2 = o_r2; 
	m_CurRot.fR3 = o_r3; 
	//m_CurRot.fR1 = 90; //
	//m_CurRot.fR2 = 90; //
	//m_CurRot.fR3 = 90; //
	m_TarRot = m_CurRot;


}

void CKss1500Auxiliary::SetRot(float t_r1, float t_r2, float t_r3)
{
	m_TarRot.fR1 = t_r1; 
	m_TarRot.fR2 = t_r2; 
	m_TarRot.fR3 = t_r3; 
}



void CKss1500Auxiliary::SetRotR3(float t_r3)
{
	m_TarRot.fR3 = t_r3; 
}

void CKss1500Auxiliary::SetIncRotR3(float t_r3)
{
	m_TarRot.fR3 = m_TarRot.fR3 + t_r3; 
}


void CKss1500Auxiliary::SetIncRot(float t_r1, float t_r2, float t_r3)
{
	m_TarRot.fR1 = m_TarRot.fR1 + t_r1; 
	m_TarRot.fR2 = m_TarRot.fR2 + t_r2; 
	m_TarRot.fR3 = m_TarRot.fR3 + t_r3; 
}

void CKss1500Auxiliary::SetCurRot(float t_r1, float t_r2, float t_r3)
{
	m_CurRot.fR1 = t_r1; 
	m_CurRot.fR2 = t_r2; 
	m_CurRot.fR3 = t_r3; 
}

void CKss1500Auxiliary::GetRot(float &r1, float &r2, float &r3)
{
	r1 = m_CurRot.fR1; 
	r2 = m_CurRot.fR2; 
	r3 = m_CurRot.fR3;
}

bool CKss1500Auxiliary::MoveRot(float deltaTime, float tar, float &cur, float vel)
{
	if( fabs(tar - cur) < 0.00001 ) 
		return true; 

	//m_bIK = false; 

	float inc_cur=0.0f;
	float deltaTimeLeft=0.0f;
	float biggestT=0.0f;
	float dj=0.0f;

	while( 1 )
	{
		dj= tar - cur; 

		float vv=vel; // 0 is x 
		float t=fabs(dj)/vv;

		if (t>biggestT)
		{
			biggestT=t;
		}

		if (biggestT>deltaTime ) 
		{
			if (dj != 0.0f)
			{
				float vv=fabs(dj)/biggestT;
				inc_cur = vv*deltaTime*dj/fabs(dj);
				cur += inc_cur;

			}
			deltaTimeLeft=0.0f;
		}
		else
		{
			dj = tar - cur;
			inc_cur = dj;
			cur += inc_cur; 

			deltaTimeLeft=deltaTime-biggestT;
		}

		deltaTime=deltaTimeLeft;
		if (deltaTime>0.0f)
		{				
			int i=0;
		}
		else
			break;
	}

	return false; 
}

//-----------------------------------------------------------------------------------------------
//----- Grp Out 

void CKss1500Auxiliary::InitGrpOut(float o_pos)
{
	m_fOrgGrpOutPos = o_pos; 
	m_fCurGrpOutPos = o_pos; 
	m_fTarGrpOutPos = o_pos;
}


void CKss1500Auxiliary::SetGrpOut(float t_pos)
{
	m_fTarGrpOutPos = t_pos; 
}

void CKss1500Auxiliary::SetIncGrpOut(float t_pos)
{
	m_fTarGrpOutPos = m_fTarGrpOutPos + t_pos; 
}

void CKss1500Auxiliary::GetGrpOut(float &pos)
{
	pos = m_fCurGrpOutPos; 
}


bool CKss1500Auxiliary::MoveGrpOut(float deltaTime, float tar, float &cur, float vel)
{
	if( fabs(tar - cur) < 0.00001 )  // shseo 0.001 -> 0.00001
		return true; 

	float inc_cur=0.0f;
	float deltaTimeLeft=0.0f;
	float biggestT=0.0f;
	float dj=0.0f;

	while( 1 )
	{
		dj= tar - cur; 

		float vv=vel; // 0 is x 
		float t=fabs(dj)/vv;

		if (t>biggestT)
		{
			biggestT=t;
		}

		if (biggestT>deltaTime ) 
		{
			if (dj != 0.0f)
			{
				float vv=fabs(dj)/biggestT;
				inc_cur = vv*deltaTime*dj/fabs(dj);
				cur += inc_cur;

			}
			deltaTimeLeft=0.0f;
		}
		else
		{
			dj = tar - cur;
			inc_cur = dj;
			cur += inc_cur; 

			deltaTimeLeft=deltaTime-biggestT;
		}

		deltaTime=deltaTimeLeft;
		if (deltaTime>0.0f)
		{				
			int i=0;
		}
		else
			break;
	}

	return false; 
}