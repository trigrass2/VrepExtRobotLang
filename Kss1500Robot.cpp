//**************************************************
// This class represents a Kss1500 robot inside this
// extension module. Each Kss1500 model in V-REP
// should have its corresponding CKss1500Robot object
// in this extension module. If a Kss1500 model
// in V-REP is removed (i.e. erased), then its
// corresponding CKss1500Robot object should also be
// removed. Same when a new Kss1500 model is added
//**************************************************

#include "stdafx.h"
#include "Kss1500Robot.h"
#include "v_rep.h"
#include "AllInstances.h"
#include <boost/lexical_cast.hpp>
#include <math.h>
#include <fstream>

// CTestServerDlg 대화 상자
int portNb=25000;
CSimpleInConnection *_connection;


#define DEFAULT_JOINT_VELOCITY (45.0f*piValue/180.0f)
#define DEFAULT_ROBOT_SPEED		10	//추가 - 2015.08.21

CKss1500Robot::CKss1500Robot()
{
	_id=-1;
	_iGraspHandle = -1;
	_associatedObjectID=-1;
	_associatedObjectUniqueID=-1;
	_robotLanguageProgram="";
	_options=1; // bit 1= automatic execution, bit2=socket or pipes (0=pipes), bit3=show server console
	_connectionAttemptPerformed=false;

	_ikFailedDialogStartShowTime=0;
	_connection = NULL; 
	_iGripJointHandles[0]  = simGetObjectHandle(std::string("left_finger_joint").c_str());
	_iGripJointHandles[1]  = simGetObjectHandle(std::string("right_finger_joint").c_str());
}

CKss1500Robot::~CKss1500Robot()
{
	_disconnect();
}

void CKss1500Robot::setID(int newID)
{
	_id=newID;
}

int CKss1500Robot::getID()
{
	return(_id);
}

void CKss1500Robot::setAssociatedObject(int objID,int objUniqueID)
{
	_associatedObjectID=objID;
	_associatedObjectUniqueID=objUniqueID;
}

int CKss1500Robot::getAssociatedObject()
{
	return(_associatedObjectID);
}

int CKss1500Robot::getAssociatedObjectUniqueId()
{
	return(_associatedObjectUniqueID);
}

void CKss1500Robot::setProgram(const std::string& prg)
{
	CKss1500Interprerter::setProgram(prg);
	attachCustomDataToSceneObject();
}

std::string CKss1500Robot::getProgram()
{
	return(CKss1500Interprerter::getProgram());
}

void CKss1500Robot::setOptions(char e)
{
	_options=e;
	attachCustomDataToSceneObject();
}

char CKss1500Robot::getOptions()
{
	return(_options);
}

void CKss1500Robot::readCCustomDataFromSceneObject()
{ // We update CKss1500Robot's data from its associated scene object custom data (for now only contains the robot language program):
	// 1. Get all the developer data attached to the associated scene object (this is custom data added by the developer):
	int buffSize=simGetObjectCustomDataLength(_associatedObjectID,DEVELOPER_DATA_HEADER);
	char* datBuff=new char[buffSize];
	simGetObjectCustomData(_associatedObjectID,DEVELOPER_DATA_HEADER,datBuff);
	std::vector<unsigned char> developerCustomData(datBuff,datBuff+buffSize);
	delete[] datBuff;
	// 2. From that retrieved data, try to extract sub-data with the KSS1500_PROGRAM and KSS1500_OPTIONS tag:
	std::vector<unsigned char> Kss1500Data;
	_robotLanguageProgram.clear(); 
	if (CAllInstances::extractSerializationData(developerCustomData,KSS1500_PROGRAM,Kss1500Data)) // extract the data stored under 'KSS1500_PROGRAM'-tag
	{ // the data was present!
		for (unsigned int i=0;i<Kss1500Data.size();i++)
			_robotLanguageProgram.push_back(Kss1500Data[i]);
	}
	if (CAllInstances::extractSerializationData(developerCustomData,KSS1500_OPTIONS,Kss1500Data)) // extract the data stored under 'KSS1500_OPTIONS'-tag
	{ // the data was present!
		_options=Kss1500Data[0];
	}
}

void CKss1500Robot::attachCustomDataToSceneObject()
{ // We update CKss1500Robot's associated scene object custom data (for now only contains the robot language program):
	// 1. Get all the developer data attached to the associated scene object (this is custom data added by the developer):
	int buffSize=simGetObjectCustomDataLength(_associatedObjectID, DEVELOPER_DATA_HEADER);
	char* datBuff=new char[buffSize];
	simGetObjectCustomData(_associatedObjectID, DEVELOPER_DATA_HEADER, datBuff);
	std::vector<unsigned char> developerCustomData(datBuff,datBuff+buffSize);
	delete[] datBuff;
	// 2. From that retrieved data, extract sub-data with the KSS1500_PROGRAM and KSS1500_OPTIONS tags, update them, and add them back to the retrieved data:
	std::vector<unsigned char> Kss1500Data;

	CAllInstances::extractSerializationData(developerCustomData,KSS1500_PROGRAM,Kss1500Data); // extract the data stored under 'KSS1500_PROGRAM'-tag
	Kss1500Data.clear(); // we discard the old value (if present)
	// we replace it with the new value:
	for (unsigned int i=0;i<_robotLanguageProgram.length();i++)
		Kss1500Data.push_back(_robotLanguageProgram[i]);
	if (Kss1500Data.size()==0)
		Kss1500Data.push_back(' '); // put an empty char if we don't have a prg yet
	CAllInstances::insertSerializationData(developerCustomData,KSS1500_PROGRAM,Kss1500Data); // insert the new data under 'KSS1500_PROGRAM'-tag




	CAllInstances::extractSerializationData(developerCustomData,KSS1500_OPTIONS,Kss1500Data); // extract the data stored under 'KSS1500_OPTIONS'-tag
	Kss1500Data.clear(); // we discard the old value (if present)
	// we replace it with the new value:
	Kss1500Data.push_back(_options);
	CAllInstances::insertSerializationData(developerCustomData,KSS1500_OPTIONS,Kss1500Data); // insert the new data under 'KSS1500_OPTIONS'-tag

	// 3. We add/update the scene object with the updated custom data:
	simAddObjectCustomData(_associatedObjectID, DEVELOPER_DATA_HEADER, (simChar*)&developerCustomData[0], int(developerCustomData.size()));
}

bool CKss1500Robot::_connect()
{ // we have a virtual connection here (we are not using socket nor pipe connections anymore)
	// We reset the inout buffer only here, since it is not really part of the robot
	if (!_connectionAttemptPerformed)
	{
		for (int i=0;i<4;i++)
			inputData[i]=0;
		_connectionAttemptPerformed=true;
	}
	return(true);
}

void CKss1500Robot::_disconnect()
{ // we have a virtual deconnection here (we are not using socket nor pipe connections anymore)
}

void CKss1500Robot::startOfSimulation()
{ 
	
}

void CKss1500Robot::endOfSimulation()
{ 
	simSetObjectParent(_iGraspHandle,-1,true);
}

void CKss1500Robot::simulationPass()
{
}

int CKss1500Robot::reset()
{
	_run_home= false; 
	_run_workmove = false; 

	_compiled=false;
	_running = false; 

	cReturnUse = 0; 


	for (int i=0;i<4;i++)
	{
		outputData[i]=0;
		inputData[i]=0;
	}

	_compiled=false;
	_running=false; 

	m_fPosVel[0] = m_fPosVel[1] = m_fPosVel[2] = (float) INIT_POS_VEL; 
	m_fRotVel[0] = m_fRotVel[1] = m_fRotVel[2] = (float) INIT_ROT_VEL;
	m_fGrpOutVel = (float) INIT_GRIP_VEL; 

	m_bIK = TRUE; 

	_nInputM = 0;
	_nOutputM = 0;
	_nInputSim = 0;
	_nOutputSim = 0;

	cReturnUse = 0; 
	timeAlreadySpentAtCurrentProgramLine = 0; 

	_run_home = false;

	// flag 
	_preProgramLine = -1; 
	_bSend = true; 
	_b_232 = false; 
	_iRotCase = 0; 
	_bDrive = true; 

	if( CAllInstances::Kss1500RobotDialog->m_pCmdDlg )
	{
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetCompileInfoText(""); 
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetBuildInfoText(""); 
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetInfo2Text(""); 
	}

	ZeroMemory(&_GeoWorkMove, sizeof(_GeoWorkMove)); 
	
	return(1);
}

int CKss1500Robot::run(float timeStep,std::string& message)
{
	if( _compiled && !_running )
		return (-1); 

	//_connect();
	if (!_compiled)
	{
		if( CAllInstances::Kss1500RobotDialog->m_pCmdDlg != NULL)
		{
			_robotLanguageProgram = CAllInstances::Kss1500RobotDialog->m_pCmdDlg->GetEditCommand(); 
		}

		if (_robotLanguageProgram.length() == 0)
			return(-1); // don't have prog code
		
		int iErrorLine = 0;
		compileCode(iErrorLine, message);
		
		if (message.compare("") != 0)
		{
			CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetInfo2Text(message); 
			CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetBuildInfoText(_compiledRobotLanguageProgram[currentProgramLine].correspondingUncompiledCode, _compiledRobotLanguageProgram[currentProgramLine].iLineNumber);
			_compiled=true;
			_running = false; 
			return(0); // Compilation error
		}

		if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
				CAllInstances::Kss1500RobotDialog->m_CommPort.InitFlag(); 
		
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetCompileInfoText("compile complete."); 

		_compiled=true;
		_running = true; 
	}

	// The prog. is supposed to have been compiled here.
	// Run the interpreter for timeStep:
	if( _running ) 
		message = runProgram(timeStep);

	CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetInfo2Text(message); 

	if (message.compare("Normal") ==0)
	{
		return(1); // Program executes normally
	}
	else
	{
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetInfo2Text("build complete."); 
		_running = false; 
		return(2); // program end!
	}
}

bool CKss1500Robot::readOutput(int data[4])
{
	return(true);
}

bool CKss1500Robot::readInput(int data[4])
{
	return(true);
}

bool CKss1500Robot::writeInput(int data[4])
{
	return(true);
}


bool CKss1500Robot::addOutputInputConnection(int inputBit,int outputRobotHandle,int outputBit,int connectionType)
{
	return(true);
}

void CKss1500Robot::removeOutputInputConnection(int inputBit)
{
	
}

void CKss1500Robot::_updateInputsFromConnections()
{
	
}

bool CKss1500Robot::GetDataFromSimIO(CString io, int &value) 
{
	int v = 0; 

	if ( io == "IN") v = _nInputSim & 0xFF;
	else if( io == "IN0") v = _nInputSim & 1;
	else if( io == "IN1") v = ( _nInputSim >> 1 ) & 1;
	else if( io == "IN2") v = ( _nInputSim >> 2 ) & 1;
	else if( io == "IN3") v = ( _nInputSim >> 3 ) & 1;
	else if( io == "IN4") v = ( _nInputSim >> 4 ) & 1;
	else if( io == "IN5") v = ( _nInputSim >> 5 ) & 1;
	else if( io == "IN6") v = ( _nInputSim >> 6 ) & 1;
	else if( io == "IN7") v = ( _nInputSim >> 7 ) & 1;
	else if( io == "OUT") v = _nOutputSim & 0xFF; 
	else if( io == "OUT0") v = _nOutputSim & 1; 
	else if( io == "OUT1") v = ( _nOutputSim >> 1 ) & 1; 
	else if( io == "OUT2") v = ( _nOutputSim >> 2 ) & 1; 
	else if( io == "OUT3") v = ( _nOutputSim >> 3 ) & 1; 
	else if( io == "OUT4") v = ( _nOutputSim >> 4 ) & 1; 
	else if( io == "OUT5") v = ( _nOutputSim >> 5 ) & 1; 
	else if( io == "OUT6") v = ( _nOutputSim >> 6 ) & 1; 
	else if( io == "OUT7") v = ( _nOutputSim >> 7 ) & 1; 
	else 
	{
		return false; 
	}

// 	if( CAllInstances::Kss1500RobotDialog->_bRs232Use )
// 		CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_IN, NULL, NULL);

	value = v; 

	return true; 
}

void CKss1500Robot::SetDataToOut(CString io, int &value) 
{
	if( io == "OUT") 
	{
		_nOutputM = value; 
		return ; 
	}

	if( value == 1 )
	{
		if( io == "OUT0") _nOutputM |= 0x01; 
		else if( io == "OUT1") _nOutputM |= (0x01 << 1); 
		else if( io == "OUT2") _nOutputM |= (0x01 << 2); 
		else if( io == "OUT3") _nOutputM |= (0x01 << 3); 
		else if( io == "OUT4") _nOutputM |= (0x01 << 4); 
		else if( io == "OUT5") _nOutputM |= (0x01 << 5);  
		else if( io == "OUT6") _nOutputM |= (0x01 << 6); 
		else if( io == "OUT7") _nOutputM |= (0x01 << 7); 
	}
	else if( value == 0 )
	{
		if( io == "OUT0")	   _nOutputM &= ~0x01; 
		else if( io == "OUT1") _nOutputM &= ~( 0x01 << 1 ); 
		else if( io == "OUT2") _nOutputM &= ~( 0x01 << 2 ); 
		else if( io == "OUT3") _nOutputM &= ~( 0x01 << 3 ); 
		else if( io == "OUT4") _nOutputM &= ~( 0x01 << 4 ); 
		else if( io == "OUT5") _nOutputM &= ~( 0x01 << 5 ); 
		else if( io == "OUT6") _nOutputM &= ~( 0x01 << 6 ); 
		else if( io == "OUT7") _nOutputM &= ~( 0x01 << 7 ); 
	}

}

void CKss1500Robot::SetDataToSimOut(CString io, int &value) 
{
	if( io == "OUT") 
	{
		_nOutputSim = value; 
		return ; 
	}

	if( value == 1 )
	{
		if( io == "OUT0") _nOutputSim |= 0x01; 
		else if( io == "OUT1") _nOutputSim |= (0x01 << 1); 
		else if( io == "OUT2") _nOutputSim |= (0x01 << 2); 
		else if( io == "OUT3") _nOutputSim |= (0x01 << 3); 
		else if( io == "OUT4") _nOutputSim |= (0x01 << 4); 
		else if( io == "OUT5") _nOutputSim |= (0x01 << 5);  
		else if( io == "OUT6") _nOutputSim |= (0x01 << 6); 
		else if( io == "OUT7") _nOutputSim |= (0x01 << 7); 
	}
	else if( value == 0 )
	{
		if( io == "OUT0")	   _nOutputSim &= ~0x01; 
		else if( io == "OUT1") _nOutputSim &= ~( 0x01 << 1 ); 
		else if( io == "OUT2") _nOutputSim &= ~( 0x01 << 2 ); 
		else if( io == "OUT3") _nOutputSim &= ~( 0x01 << 3 ); 
		else if( io == "OUT4") _nOutputSim &= ~( 0x01 << 4 ); 
		else if( io == "OUT5") _nOutputSim &= ~( 0x01 << 5 ); 
		else if( io == "OUT6") _nOutputSim &= ~( 0x01 << 6 ); 
		else if( io == "OUT7") _nOutputSim &= ~( 0x01 << 7 ); 
	}

}


int CKss1500Robot::CheckCoordQuadrant(float x, float y)
{
	int Q = 0; 

	if( x >= 0 && y >= 0 ) 
		Q = 1; 
	else if( x < 0 && y >= 0 ) 
		Q = 2; 
	else if( x < 0 && y < 0 ) 
		Q = 3; 
	else 
		Q = 4; 

	return Q; 
}

int CKss1500Robot::IsRotateCase(float c_x, float c_y, float t_x, float t_y)
{
	//////////////////////////////////////////////////////////////////
	// 4사분면으로 로봇 위치를 나누어서 중간점으로 Moter1을 회전시킬지 판별
	// 로봇 홈 위치가 3사분면으로 시계방향으로 3-> 2-> 1-> 4 분면 순

	int r = 0; 
	int cQ = CheckCoordQuadrant(c_x, c_y);		// 로봇의 현재 사분면 위치
	int tQ = CheckCoordQuadrant(t_x, t_y);		// 목표점의 사분면 위치

	if( cQ == tQ ) 
		return r; 
	
	if( cQ == 3 && tQ == 4 )
		r = 2; // rot == 90
	else if( cQ == 3 && tQ == 1 )
		r = 1;// rot == 0
	else if( cQ == 2 && (tQ == 4) ) // tQ == 1 || 
		r = 2;// rot == 90

	else if( cQ == 4 && tQ == 2 )
		r = 1;// rot == 0
	else if( cQ == 4 && tQ == 3 )
		r = 3;// rot == -90

	else if( cQ == 1 && tQ == 3 ) //tQ == 2 ||
		r = 3;// rot == -90


	return r; 
}

bool CKss1500Robot::GetSerialMotionData(CCompiledProgramLine& ProgLine, float fPos[], float& Spd)
{
	static int iSubProgramNo = 0;
	float fVelocity = 0.0f;

	for(int i = 0; i< ProgLine.intParameter[1] ;i++)
	{
		std::string axisValue;			
		float _vel = 0.0f;
		int iValue = 0;
		axisValue = ProgLine.strLabel[i];

		if(GetDataFromMap(axisValue.c_str(), _vel)) 
			fPos[i] = _vel;
		else if(_getFloatFromWord(axisValue.c_str(), _vel)) 
			fPos[i] = _vel;
		else if(GetDataFromIO(axisValue.c_str(),iValue))
			fPos[i] = (float) iValue;
		else if(axisValue.front() == 'S') 
		{
			axisValue.erase(axisValue.begin());
			if(axisValue.front() == '=') 
				axisValue.erase(axisValue.begin());

			bool bRtnValue = _extractOneWord(axisValue, axisValue);
			bRtnValue = _getFloatFromWord(axisValue, _vel);
			if( !CAllInstances::Kss1500RobotDialog->ValidateSpeed(_vel) ) 
			{
				return("Wrong Parameter !"); // program end
			}
			Spd = _vel;
		}
		else if(axisValue.front() == 'R') 
		{
			bool bRtnValue = _extractOneWord(axisValue, axisValue);
			bRtnValue = _getFloatFromWord(axisValue, fPos[4]);
			if( !CAllInstances::Kss1500RobotDialog->ValidateRotate(fPos[4]) ) 
			{
				return("Wrong Parameter !"); // program end
			}
			Spd = _vel;
		}
	}

	return true;
}

std::string CKss1500Robot::runProgram(float deltaTime)
{
	// This function runs the compiled robot language program for "deltaTime" seconds
	// return value "" means program ended!
	CString str;  
	if (int(_compiledRobotLanguageProgram.size())<=currentProgramLine)
		return(""); // program end
	int cmd = _compiledRobotLanguageProgram[currentProgramLine].command; // get the current command
	int loopCnt = 0;
	static CIfElseThen firstIfElse;

	if( _preProgramLine != currentProgramLine ) // shseo 2014_1203 add
	{
		_bSend = true; 
		_b_232 = false;
		_iRotCase = 0; 
		_bDrive = true; 
	}
	_preProgramLine = currentProgramLine; // shseo 2014_1203 add


	while (true)
	{
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetBuildInfoText(_compiledRobotLanguageProgram[currentProgramLine].correspondingUncompiledCode, _compiledRobotLanguageProgram[currentProgramLine].iLineNumber);
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->DisplayMemValue(_nInputSim, _nOutputSim);		// 시뮬레이션 로봇 데이터

		if(cmd == ID_VALUE) // 변수 = 값; 변수 = 변수; 변수 = INT ; OUT = 변수(값); 
		{
			CString str; 
			std::string codeWord; 
			float fSimValue, fRealValue, fstrValue;	//float f; 
			int iSimValue, iRealValue;
			
			std::string word = (_compiledRobotLanguageProgram[currentProgramLine].strLabel[4].compare("POSE") == 0)	? _compiledRobotLanguageProgram[currentProgramLine].strLabel[0] : _compiledRobotLanguageProgram[currentProgramLine].strLabel[1]; 
			int iRtnValue = 0;

			//	A = A + 3
			if(_compiledRobotLanguageProgram[currentProgramLine].intParameter[0] == 1)	//	A= 3*X
			{
				word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[2];
				iRtnValue = DoMathOperation(word);
				_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0] = atof(word.c_str());
				word.clear();
			}
			//	P1 = P1+P2
			else if(_compiledRobotLanguageProgram[currentProgramLine].strLabel[4].compare("POSE") == 0)	//	A= 3*X
			{
				iRtnValue = DoMathOperation(word);
				_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0] = atof(word.c_str());
				word.clear();
			}
			else	//	P0= P1+(X,10,20)
			{
				float fTargValue[7] = {0.0f,};
				
				if(DoPoseOperation(_compiledRobotLanguageProgram[currentProgramLine], fTargValue, word, m_CurPos, m_CurRot, m_fCurGrpOutPos) > 0)
				{
					SetDataToINI(word.c_str(), fTargValue);
					goto POSE_LABEL;
				}
				else
					ASSERT(-1);
			}

			//	기존 루틴.. 그냥 타고가면 된다.
			if( word == "")  
			{
				fstrValue = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
			}
			else
			{
				str = CA2CT(word.c_str());

				if( GetDataFromMap(str, fstrValue) ) // 일반 변수 
				;
				//////////////////////////////////////////////////
				// IO 처리 Real Robot과  Simulation과 분리 - 2015.09.08
				//else if( GetDataFromIO(str, i) ) // io 변수  
				//	f=(float)i;
				else
				{
					if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
					{
						if( GetDataFromSimIO(str, iSimValue) ) // io 변수
							fstrValue = fSimValue = (float) iSimValue;
					}

					if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
					{

						if( _bSend && !_b_232)		// 통신 수신 후 다음 지령으로 넘어가도록 수정 - 2016.02.03
						{
							if( GetDataFromIO(str, iRealValue) ) // io 변수
							{
								CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_IN, NULL, NULL);
								_bSend = false; 
								break;
							}
							else	// IO 입출력 지령이 아닌경우
							{
								_bSend = false;
								_b_232 = TRUE;
							}
						}
						else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_IN) && !_b_232) 
						{
							if( GetDataFromIO(str, iRealValue) ) // io 변수
								fstrValue = fRealValue = (float) iRealValue;

							_b_232 = true;
						}
						else if (!_b_232)	// 회신이 아직 안왔을 때
							break;
					}
					else
						_b_232 = true; // 통신 안할 시
				}
			}
				
			codeWord = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 

			str = CA2CT(codeWord.c_str());

			if( GetDataFromIO(str, iSimValue) )		// 변수명이 포트 인 경우
			{
				//i = (int) f; 
				//SetDataToOut(str, i); 

				if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
				{
					if( word == "") 
						iSimValue = (int) fstrValue; 
					else
						iSimValue = (int) fSimValue; 

					SetDataToSimOut(str, iSimValue); 
				}

				// 통신 
				if( CAllInstances::Kss1500RobotDialog->_bRs232Use )
				{
					if (str.Find(_T("OUT")) != -1)	// "OUT"이라는 문자열이 포함된 경우만 - 2016.02.03
					{
						if( word == "") 
							iSimValue = (int) fstrValue; 
						else
							iRealValue = (int) fRealValue; 

						SetDataToOut(str, iRealValue); 

						int iParam[4]; 
						iParam[0] = _nOutputM; 
						CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_OUT, iParam, NULL);
					}
				}

			}
			else
			{
				m_mIntVar.SetAt(str, fstrValue);
			}
			
			POSE_LABEL:
			currentProgramLine++;
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		
		if (cmd == ID_GOHOME) // GOHOME
		{
			bool b_sim = false; 
			bool b[6] = {0,0,0,0,0,0}; 
			
			
			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator - release -> z up + rotate -> joint home 
			{
				if( !b[0] )
				{
					float rot = 0; 
					float r = 0.013 * rot / 180 - 0.02;

					// 180 max 
					SetGrpOut(r);
					b[0] = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 
				}

				if ( b[0] && !b[1] )
				{
					m_bIK = true; 
					SetPosZ(0); 
					b[1] = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
				}

				if ( b[1] && !b[2])
				{
					m_bIK = false; 
					SetRotR3(0); 
					b[2] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 

				}

				if ( b[1] && b[2] ) 
				{
					m_bIK = false; 
					SetRot(-120, -115, 0); 
					b[3] = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
					b[4] = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 
					b[5] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 
				}
				
				if( b[3] && b[4] && b[5] )
				{
					b_sim = true; 
		
				}
				
			}
			else
				b_sim = true; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_HOME, NULL, NULL);
					_bSend = false; 

					TRACE("send - home \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_HOME) ) 
				{
					_b_232 = true;

					TRACE("send - home comp \r\n"); // test
				}		
					
			}
			else 
				_b_232 = true; 

			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else				// 반드시 있어야 함. 
				break;			// 반드시 있어야 함. 

		}

		if (cmd == ID_VAR || cmd == ID_ENDSWITCH || cmd == ID_CHANGE || cmd == ID_BLANK || cmd == ID_LABEL_DEFAULT || cmd == ID_LINE_COMMENT || cmd == ID_LABEL) // VAR or EmptyLine + label + comment 
		{
			currentProgramLine++;
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		if (cmd == ID_ELSE) // VAR or EmptyLine + label + comment 
		{
			if(firstIfElse.bIgnoreCondition_ == true)
			{
				currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0]-1; // we jump
			}
			else
				currentProgramLine++;
			
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		if (cmd == ID_SPEED) // SPEED - 변수처리 ok
		{
			//float rot ; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[2]; 
			if( word == "") 
				_vel = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0]; 
			else 
			{
				CString str = CA2CT(word.c_str());
				int iRtnValue = DoMathOperation(word);
				if(iRtnValue<0)
					break;
				_getFloatFromWord(word, _vel);
			}
			
			if( !CAllInstances::Kss1500RobotDialog->ValidateSpeed(_vel) ) 
			{
				return("Wrong Parameter !"); // program end
			}

			// sim 
			m_fPosVel[0] = m_fPosVel[1] = m_fPosVel[2] = MAX_POS_VEL * _vel / 100; 
			m_fRotVel[0] = m_fRotVel[1] = m_fRotVel[2] = MAX_ROT_VEL * _vel / 100;
			m_fGrpOutVel = INIT_GRIP_VEL * _vel / 100; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					int iParam[4]; 
					iParam[0] = _vel; // rotate angle

					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_SPEED, iParam, NULL);
					_bSend = false; 

					TRACE("send - speed \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_SPEED) ) 
				{
					_b_232 = true;

					TRACE("send - speed comp \r\n"); // test
				}
			}
			else
				_b_232 = true; 

			if( _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
				//break; 
			}
			else 
				break; // add 

		}

		if (cmd == ID_ROTATE) // ROTATE - 변수처리 ok
		{
			float rot ; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			if( word == "") 
				rot = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, rot) ; 
			}
			
			if( !CAllInstances::Kss1500RobotDialog->ValidateRotate(rot) ) 
			{
				return("Wrong Parameter !"); // program end
			}

			bool b_sim = false; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				bool b = 0; 
				m_bIK = false; 
				SetRotR3(-1*rot);
				b = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 

				if( b)
				{
					b_sim = true; 
				}
				//else 
					//break;
			}
			else
				b_sim = true; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					float fParam[3]; 
					fParam[0] = rot; // rotate angle

					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_ROTATE, NULL, fParam);
					_bSend = false; 

					TRACE("send - rotate \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_ROTATE) ) 
				{
					_b_232 = true;

					TRACE("send - rotate comp \r\n"); // test
				}
			}
			else 
				_b_232 = true; 
	
			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else
				break; 

		}

		
		if (cmd == ID_GRASP) // GRASP - 변수처리 ok
		{
			float rot ; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			if( word == "") 
				rot = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, rot) ; 
			}

			if( !CAllInstances::Kss1500RobotDialog->ValidateGrip(rot) ) 
			{
				return("Wrong Parameter !"); // program end
			}

			bool b_sim = false; 
			bool b = 0; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				float r = 0.013 * rot / 180 - 0.02;

				// 180 max 
				SetGrpOut(r);
				b = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 
				if( b )
				{
					b_sim = true; 
				}
			}
			else
				b_sim = true;


			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					float fParam[3]; 
					fParam[0] = rot; // rotate angle

					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_GRASP, NULL, fParam);
					_bSend = false; 

					TRACE("send - grip (grasp) \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_GRASP) ) 
				{
					_b_232 = true;

					TRACE("send - grip (grasp) comp \r\n"); // test
				}
				
			}
			else 
				_b_232 = true; 

			
			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else 
				break; 
		}


		if (cmd == ID_RELEASE) // RELEASE - 변수처리 ok
		{
			float rot ; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			if( word == "") 
				//rot = 181 - _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0]; // shseo ? 확인 필요 
				rot = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, rot) ; 
			}

			if( !CAllInstances::Kss1500RobotDialog->ValidateGrip(rot) ) 
			{
				return("Wrong Parameter !"); // program end
			}

			bool b_sim = false; 
			bool b = 0; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				float r = 0.013 * rot / 180 - 0.02;

				// 180 max 
				SetGrpOut(r);
				b = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 
				if( b )
				{
					b_sim = true; 
				}
				//else 
					//break; 
			}
			else
				b_sim = true;


			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					float fParam[3]; 
					fParam[0] = rot; // rotate angle

					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_GRASP, NULL, fParam);
					_bSend = false; 

					TRACE("send - release ( grasp) \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_GRASP) ) 
				{
					_b_232 = true;

					TRACE("send - release ( grasp) comp \r\n"); // test
				}
			}
			else 
				_b_232 = true; 

			
			if( b_sim && _b_232 )
			{

				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else 
				break; 
		}

	

		if (cmd == ID_DRIVE) // DRIVE  - 변수처리 ok
		{
			int i; 
			float d, d2, d3;
			static float d4 = 0; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			if( word == "") 
				i = _compiledRobotLanguageProgram[currentProgramLine].intParameter[0]; 
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, d) ; 
				i = (int)d; 
			}

			word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[1];
			if( word == "") 
				d = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0]; 
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, d) ; 
			}
			
			if(i==3)
				d3 = d/1000; 
			else
				d3 = d; 

			if( !CAllInstances::Kss1500RobotDialog->ValidateDrive(i, d3) ) 
			{
				return("Wrong Parameter !"); // program end
			}

			bool b = 0; 
			bool b_sim = false; 
			
			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				if( i == 1)
				{
					m_bIK = false; 

					if( _bDrive )
					{
						_bDrive = false; 
						d4 = m_CurRot.fR1;
					}

					SetRot(d +d4, m_CurRot.fR2, m_CurRot.fR3); 
					//SetRot(d +m_CurRot.fR1, m_CurRot.fR2, m_CurRot.fR3); 
					b = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
				}
				else if( i == 2)
				{
					m_bIK = false; 

					if( _bDrive )
					{
						_bDrive = false; 
						d4 = m_CurRot.fR2;
					}

					SetRot(m_CurRot.fR1, d +d4, m_CurRot.fR3); 
					//SetRot(m_CurRot.fR1, d +m_CurRot.fR2, m_CurRot.fR3); 
					b = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 
				}
				else if( i == 3)
				{
					m_bIK = true; 

					if( _bDrive )
					{
						_bDrive = false; 
						d4 = m_CurPos.fZ;
					}

					d2 = d *(-1) / 1000;

					SetPosZ(d2 +d4);
					//SetPosZ(d2 +m_CurPos.fZ);
					b = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
				}

				if( b )
				{
					b_sim = true; 
				}
			}
			else
				b_sim = true; 
			

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					int iParam[4];
					float fParam[3]; 
					iParam[0] = i; // joint index
					fParam[0] = d; // rotate angle
					
					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_DRIVE, iParam, fParam);
					_bSend = false; 

					TRACE("send - drive \r\n"); // test
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_DRIVE) ) 
				{
					_b_232 = true;

					TRACE("send - drive comp \r\n"); // test
				}
			}
			else 
				_b_232 = true; 

			
			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else 
				break; 
		}
		
		if (cmd == ID_MOVE || cmd == ID_READY || cmd == ID_MOVE2 )	// MOVE + READY + MOVE2  - 2015.08.28
		{
			float fRobotPosData[5] = {0.0f,};
			int iForLimit = 3;
			int iCount;
			bool b_sim = false, _b_232 = false;
			float fTargValue[7] = {0.0f,};
			std::string tmpString0(_compiledRobotLanguageProgram[currentProgramLine].strLabel[4]);
			std::string tmpString = "NULL";

			//	MOVE L, P001＋(0, 0, 10), ?＋(20, 0, 0) 처리.
			if(_compiledRobotLanguageProgram[currentProgramLine].strLabel[4].compare("POSE") == 0)	//	A= 3*X
			{
				GEO tmpCurPos = m_CurPos;
				ROT tmpCurRot= m_CurRot;
				float tmpCurGripPos = m_fCurGrpOutPos;

				GetCurrentRobotMotionInfo(_compiledRobotLanguageProgram[currentProgramLine], tmpCurPos, tmpCurRot, tmpCurGripPos);
				if(!DoPoseOperation(_compiledRobotLanguageProgram[currentProgramLine], fTargValue, tmpString, tmpCurPos, tmpCurRot, tmpCurGripPos) > 0)
					ASSERT(-1);
				
				cmd = ID_MOVE;

				for (iCount = 0 ; iCount < 5 ; iCount++)
					fRobotPosData[iCount] = fTargValue[iCount];
			}
			else 
			{
				//	기존루틴
				if (cmd == ID_MOVE2)	// MOVE2 방식의 경우 변수 추가 처리
					iForLimit = 4;

				for (iCount = 0 ; iCount < iForLimit ; iCount++)
				{
					std::string strLableWord = _compiledRobotLanguageProgram[currentProgramLine].strLabel[iCount]; 

					if( strLableWord == "" || strLableWord == "NULL" ) 
						fRobotPosData[iCount] = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[iCount];
					else 
					{
						CString str = CA2CT(strLableWord.c_str());
						GetDataFromMap(str, fRobotPosData[iCount]);
					}
				}
			}
			
			// 범위 오류 판단
			if( !CAllInstances::Kss1500RobotDialog->ValidateMove(fRobotPosData[0] / 1000, fRobotPosData[1] / 1000, fRobotPosData[2] / 1000) ) 
				return("X, Y, Z Position Wrong Parameter !"); // program end
			if (!CAllInstances::Kss1500RobotDialog->ValidateRotate(fRobotPosData[3]) && cmd == ID_MOVE2)
				return("R Position Wrong Parameter !"); // program end
			if (!CAllInstances::Kss1500RobotDialog->ValidateGrip(fRobotPosData[4]) && cmd != ID_MOVE2)
				return("Griap Position Wrong Parameter !"); // program end

			// 키네마틱스 계산
			if( _iRotCase == 0 || _iRotCase == 1 )	// IsRotateCase() 대신에 IK를 사용한 ValidateMove()를 사용 - 2015.07.01
			{
				_iRotCase = 1;
				double tJoint1 =0;
				double tJoint2 =0;
				CAllInstances::Kss1500RobotDialog->ValidateMove(fRobotPosData[0] / 1000, fRobotPosData[1] / 1000, tJoint1, tJoint2);

				if (cmd == ID_MOVE2)	// 모든 축 목표점 적용
				{
					SetRot(tJoint1, tJoint2, (-1 * fRobotPosData[3]));
					//SetRot(tJoint1, tJoint2, m_CurRot.fR3);
					SetPosZ( fRobotPosData[2]/1000*(-1));
					//SetRotR3(-1 * fRobotPosData[3]);
				}
				else	// X,Y축만 계산 적용
				{
					SetRot(tJoint1, tJoint2, m_CurRot.fR3);
				}
			}


			// 모션 처리
			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )	// 송신
				{
					float fParam[3]; 
					fParam[0] = fRobotPosData[0]; 
					fParam[1] = fRobotPosData[1]; 
					
					if (cmd == ID_MOVE2)
					{
						int iParam[2];	// Z축과 R축 값은 100 곱하여 int 형으로 전달
						iParam[0] = (int) (fRobotPosData[2] * 100); 
						iParam[1] = (int) (fRobotPosData[3] * 100); 
						CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_MOVE2, iParam, fParam);
						TRACE("send - move2 \r\n");
					}
					else
					{
						fParam[2] = fRobotPosData[2]; 
						CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_MOVE, NULL, fParam);
						TRACE("send - move \r\n");
					}
					_bSend = false; 
				}
				else  // 수신 체크
				{
					if (cmd == ID_MOVE2)
						_b_232 = CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_MOVE2);
					else
						_b_232 = CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_MOVE);

					if (_b_232)
						TRACE("send - move comp \r\n");
				}				
			}
			else 
				_b_232 = true; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				bool bTagetRot1End = false;
				bool bTagetRot2End = false;
				bool bTagetZPosEnd = false;
				bool bTagetRPosEnd = false;

				if (cmd == ID_MOVE2)	// MOVE2 동작 처리
				{
					////////////////////////////////////////////////////////////////////////////////
					// V-REP IK를 킨 상태로만 동작하도록 수정 - 2016.02.16
					// MoveRot()로 축 값 로드 -> Forword Kinematics 사용하여 좌표로 변경 -> V-REP IK로 동작

/*					// X, Y, R 축 동작 후 Z 축 동작 
					if (_iRotCase == 1)
					{
						m_bIK = false;	// V-REP의 IK 정지

						bTagetRot1End = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]);	// 1축
						bTagetRot2End = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]);	// 2축
						//bTagetZPosEnd = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]);	// Z축
						bTagetRPosEnd = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]);	// R축

						//if(!(bTagetRot1End && bTagetRot2End && bTagetZPosEnd && bTagetRPosEnd))	// 동작 미완료 시
						if(!(bTagetRot1End && bTagetRot2End && bTagetRPosEnd))	// 동작 미완료 시
							break;
						else		// 1, 2, X, R 축 동작 완료
							_iRotCase = 10; // pass
					}

					m_bIK = true;

					bTagetRot1End = MovePos(deltaTime, m_TarPos.fX, m_CurPos.fX, m_fPosVel[0]); 
					bTagetRot2End = MovePos(deltaTime, m_TarPos.fY, m_CurPos.fY, m_fPosVel[1]);
					//bTagetZPosEnd = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]);
					bTagetRPosEnd = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]);
					
					if (bTagetRot1End && bTagetRot2End && bTagetRPosEnd)		// 1, 2, R 축 동작 후 Z 축 동작
					{
						m_bIK = true; 
						SetPosZ(fRobotPosData[2] / 1000 * (-1));
						bTagetZPosEnd = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
					}

					if( bTagetRot1End && bTagetRot2End && bTagetZPosEnd && bTagetRPosEnd)	// X,Y,Z,R 동작 완료
					{
						b_sim = true;
					}

/*/					// X, Y, Z, R 축 동시 동작
					if (_iRotCase == 1)
					{
						m_bIK = FALSE;	// V-REP의 IK OFF

						bTagetRot1End = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]);	// 1축
						bTagetRot2End = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]);	// 2축

						SetPosZ(fRobotPosData[2] / 1000 * (-1));

						bTagetZPosEnd = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]);
						bTagetRPosEnd = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]);

						if(!(bTagetRot1End && bTagetRot2End && bTagetZPosEnd && bTagetRPosEnd))	// 동작 미완료 시
						{
							break;
						}
						else		// 1, 2, X, R 축 동작 완료
						{
							_iRotCase = 10; // pass
							b_sim = true;
						}
					}
//*/
				}
				else		// 기존 MOVE 및 READY 시뮬레이션 동작 처리
				{
					if (_iRotCase == 1)	// 키네마틱스 계산 후 1,2 축 동작
					{
						m_bIK = false; 

						bTagetRot1End = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
						bTagetRot2End = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 

						if( !(bTagetRot1End && bTagetRot2End) )
							break; 
						else		// 1, 2 축 동작 완료
							_iRotCase = 10; // pass
					}

					m_bIK = true; 
					SetPos(fRobotPosData[0] / 1000, fRobotPosData[1] / 1000, m_CurPos.fZ); 
					bTagetRot1End = MovePos(deltaTime, m_TarPos.fX, m_CurPos.fX, m_fPosVel[0]); 
					bTagetRot2End = MovePos(deltaTime, m_TarPos.fY, m_CurPos.fY, m_fPosVel[1]); 

					if (bTagetRot1End && bTagetRot2End)		// 1, 2 축 동작 후 Z 축 동작
					{
						m_bIK = true; 
						SetPosZ(fRobotPosData[2] / 1000 * (-1));
						bTagetZPosEnd = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
					}

					if( bTagetRot1End && bTagetRot2End && bTagetZPosEnd )	// X,Y,Z 동작 완료
					{
						b_sim = true;
					}
				}
				
			}
			else
				b_sim = true;

			// 라인 종료
			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;

			}
			else 
				break; 
		}	// MOVE / READY End

		if (cmd == ID_STOP||cmd == ID_END) //STOP, END
		{ 
			currentProgramLine++;
			currentProgramLineNotYetPartiallyProcessed=true;
			return(""); // program end
		}
		
		if (cmd == ID_IF || cmd == ID_ELSEIF)
		{ // IF
			CString str; 
			std::string var = _compiledRobotLanguageProgram[currentProgramLine].strLabel[3];
			float fVarValue = 0.0f; 
			int v1 = 0; 
			int v2 = 0;

			/////////////////////////////////////////////////////////////////
			// 단순 처리로 IF문 사용 시 INPUT 상태 업데이트 - 2016.02.03
			if( CAllInstances::Kss1500RobotDialog->_bRs232Use )
			{
				if( _bSend && !_b_232)		// 통신 수신 후 다음 지령으로 넘어가도록 수정 - 2016.02.03
				{
					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_IN, NULL, NULL);
					_bSend = false; 
					break;
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_IN) && !_b_232) 
					_b_232 = true;
				else if (!_b_232)	// 회신이 아직 안왔을 때
					break;
			}
			// End - 2016.02.03


			/*	GO-SUB문에서 되돌아 왔을때의 처리 버그수정*/
			if(firstIfElse.bIfElseEnable_ == true && firstIfElse.bIgnoreCondition_ == true &&  cmd == ID_ELSEIF)
			{
				currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0]-1; // we jump
				continue;
			}
			// 새 조건문 처리
			if(var.size() > 0) 
			{
				DoRelationalOperation(var);
				DoLogicalOperation(var);
				v2 = atoi(var.c_str());
				var = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			}
			else
			{
				int tmpValue = 0;
				v2 = _compiledRobotLanguageProgram[currentProgramLine].intParameter[0];
				
				//	기존 조건문 처리
				//	IF IN1=0 THEN LABEL1 ELSE GOTO LABEL2
				if(GetDataFromIO(_compiledRobotLanguageProgram[currentProgramLine].strLabel[0].c_str(), tmpValue))
				{
					v1 = (int)tmpValue; 
					if( v1 == v2 )
					{
						TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[1] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[1]); 

						currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[1]; // we jump
					}
					else
					{
						TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[2] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[2]); 

						currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[2]; // we jump
					}
					goto ID_IF_EXIT;
				}
				else
					var = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			}

			str = CA2CT(var.c_str());	

			if( GetDataFromMap(str, fVarValue))
			{
				v1 = (int)fVarValue; 
				if( v1 == v2 )
				{
					TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[1] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[1]); 

					currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[1]; // we jump
				}
				else
				{
					TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[2] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[2]); 

					currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[2]; // we jump
				}
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else if(  (str, v1)) // 2014_1229 shseo "if -> else if"
			{
				if( v1 == v2 )
				{
					TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[1] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[1]); 

					currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[1]; // we jump
				}
				else
				{
					TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d].intParameter[2] =%d \r\n", currentProgramLine, currentProgramLine, _compiledRobotLanguageProgram[currentProgramLine].intParameter[2]); 

					currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[2]; // we jump
				}
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else if(var.size() == 0)
			{
				if (v2>0)
				{
					currentProgramLine++;
					firstIfElse.bIgnoreCondition_ = true;
				}
				else
				{
					currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0]-1; // we jump
					if(currentProgramLine <0)
					{
						return "IF ERROR";
					}
				}

			}
			else
			{
				//fail
				return(""); // program end
			}
			ID_IF_EXIT:;
		}
		if (cmd == ID_WAIT) // WAIT - 변수처리 ok
		{ 
			// WAIT X=1, 500
			float timeToWait, f; 
			int v1; 
			int v2=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0];
			std::string var = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			CString str  = CA2CT(var.c_str());

			if( GetDataFromMap(str, f) )
			{
				v1 = (int)f; 
				if( v1 == v2)
				{
					currentProgramLine++; 
				}
				else
				{
					std::string word;

					if(_compiledRobotLanguageProgram[currentProgramLine].strLabel[2].find_first_of(MATH_OP) == std::string::npos)
						word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[1]; 
					else
					{
						word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[2];
						
						if(DoMathOperation(word ) >= 0)
						{
							_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0] = atof(word.c_str())/1000;
							word.clear();
						}
						else
							break;
					}
					//	 기존 루틴
					if( word == "")  
					{
						timeToWait=_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
						timeToWait=timeToWait; // convert from ms to s
					}
					else 
					{
						CString str = CA2CT(word.c_str());
						GetDataFromMap(str, timeToWait) ; 
						timeToWait=timeToWait/1000.0f; // convert from ms to s

					}

					timeToWait-=timeAlreadySpentAtCurrentProgramLine;
					if (timeToWait>deltaTime)
					{
						timeAlreadySpentAtCurrentProgramLine+=deltaTime;

						break;
					}
					else
					{
						deltaTime-=timeToWait;
						currentProgramLine++;
						currentProgramLineNotYetPartiallyProcessed=true;
					}
				}
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else
			{
				//fail
				return(""); // program end
			}
		}

		if (cmd == ID_SET)
		{ // SET
			// we have a variable here // 변수 값 할당 ex) AA = 10
			bool error = true;

			int v1, v2; 
			std::string io = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			v2 = 1;

			if( CAllInstances::Kss1500RobotDialog->_bRs232Use )	// 통신
			{
				//if( io == "OUT") _nOutputM = v1; 
				if( io == "OUT0")	   { _nOutputM |= v2; v1 = 0; }
				else if( io == "OUT1") { _nOutputM |= ( v2 << 1 );  v1 = 1; }
				else if( io == "OUT2") { _nOutputM |= ( v2 << 2 );  v1 = 2; }
				else if( io == "OUT3") { _nOutputM |= ( v2 << 3 );  v1 = 3; } // out port 는 4개 
				else if( io == "OUT4") { _nOutputM |= ( v2 << 4 );  v1 = 4; }
				else if( io == "OUT5") { _nOutputM |= ( v2 << 5 );  v1 = 5; }
				else if( io == "OUT6") { _nOutputM |= ( v2 << 6 );  v1 = 6; }
				else if( io == "OUT7") { _nOutputM |= ( v2 << 7 );  v1 = 7; }
				else if( io == "IN0") _nInputM |= v2; 
				else if( io == "IN1") _nInputM |= ( v2 << 1 ); 
				else if( io == "IN2") _nInputM |= ( v2 << 2 ); 
				else if( io == "IN3") _nInputM |= ( v2 << 3 ); 
				else if( io == "IN4") _nInputM |= ( v2 << 4 ); 
				else if( io == "IN5") _nInputM |= ( v2 << 5 ); 
				else if( io == "IN6") _nInputM |= ( v2 << 6 ); 
				else if( io == "IN7") _nInputM |= ( v2 << 7 ); 
				else if( io == "ALL") 
				{
					_nInputM = 0xFF; 
					_nOutputM = 0xFF; 
				}

			}

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				if( io == "OUT0")	   { _nOutputSim |= v2; v1 = 0; }
				else if( io == "OUT1") { _nOutputSim |= ( v2 << 1 );  v1 = 1; }
				else if( io == "OUT2") { _nOutputSim |= ( v2 << 2 );  v1 = 2; }
				else if( io == "OUT3") { _nOutputSim |= ( v2 << 3 );  v1 = 3; } // out port 는 4개 
				else if( io == "OUT4") { _nOutputSim |= ( v2 << 4 );  v1 = 4; }
				else if( io == "OUT5") { _nOutputSim |= ( v2 << 5 );  v1 = 5; }
				else if( io == "OUT6") { _nOutputSim |= ( v2 << 6 );  v1 = 6; }
				else if( io == "OUT7") { _nOutputSim |= ( v2 << 7 );  v1 = 7; }
				else if( io == "IN0") _nInputSim |= v2; 
				else if( io == "IN1") _nInputSim |= ( v2 << 1 ); 
				else if( io == "IN2") _nInputSim |= ( v2 << 2 ); 
				else if( io == "IN3") _nInputSim |= ( v2 << 3 ); 
				else if( io == "IN4") _nInputSim |= ( v2 << 4 ); 
				else if( io == "IN5") _nInputSim |= ( v2 << 5 ); 
				else if( io == "IN6") _nInputSim |= ( v2 << 6 ); 
				else if( io == "IN7") _nInputSim |= ( v2 << 7 ); 
				else if( io == "ALL") 
				{
					_nInputSim = 0xFF; 
					_nOutputSim = 0xFF; 
				}
			}
	
			// 통신 
			if( CAllInstances::Kss1500RobotDialog->_bRs232Use )
			{
				int iParam[4]; 
				iParam[0] = v1; 
				CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_SET, iParam, NULL);
			}

			currentProgramLineNotYetPartiallyProcessed=true;
			currentProgramLine++; 
		}

		if (cmd == ID_RESET)
		{ // RESET
			// we have a variable here // 변수 값 할당 ex) AA = 10
			bool error = true;

			int v1, v2; 
			std::string io = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 

			v2 = 1;

			if( CAllInstances::Kss1500RobotDialog->_bRs232Use )	// 통신
			{
				if( io == "OUT0")	   { _nOutputM &= ~v2; v1 = 0; }
				else if( io == "OUT1") { _nOutputM &= ~( v2 << 1 ); v1 = 1; }
				else if( io == "OUT2") { _nOutputM &= ~( v2 << 2 ); v1 = 2; }
				else if( io == "OUT3") { _nOutputM &= ~( v2 << 3 ); v1 = 3; }
				else if( io == "OUT4") { _nOutputM &= ~( v2 << 4 ); v1 = 4; }
				else if( io == "OUT5") { _nOutputM &= ~( v2 << 5 ); v1 = 5; }
				else if( io == "OUT6") { _nOutputM &= ~( v2 << 6 ); v1 = 6; }
				else if( io == "OUT7") { _nOutputM &= ~( v2 << 7 ); v1 = 7; }
				else if( io == "IN0") _nInputM &= ~v2; 
				else if( io == "IN1") _nInputM &= ~( v2 << 1 ); 
				else if( io == "IN2") _nInputM &= ~( v2 << 2 ); 
				else if( io == "IN3") _nInputM &= ~( v2 << 3 ); 
				else if( io == "IN4") _nInputM &= ~( v2 << 4 ); 
				else if( io == "IN5") _nInputM &= ~( v2 << 5 ); 
				else if( io == "IN6") _nInputM &= ~( v2 << 6 ); 
				else if( io == "IN7") _nInputM &= ~( v2 << 7 ); 
				else if( io == "ALL") 
				{
					_nInputM = 0x0; 
					_nOutputM = 0x0; 
				}

			}


			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				//if( io == "OUT") _nOutputM = v1; 
				if( io == "OUT0")	   { _nOutputSim &= ~v2; v1 = 0; }
				else if( io == "OUT1") { _nOutputSim &= ~( v2 << 1 ); v1 = 1; }
				else if( io == "OUT2") { _nOutputSim &= ~( v2 << 2 ); v1 = 2; }
				else if( io == "OUT3") { _nOutputSim &= ~( v2 << 3 ); v1 = 3; }
				else if( io == "OUT4") { _nOutputSim &= ~( v2 << 4 ); v1 = 4; }
				else if( io == "OUT5") { _nOutputSim &= ~( v2 << 5 ); v1 = 5; }
				else if( io == "OUT6") { _nOutputSim &= ~( v2 << 6 ); v1 = 6; }
				else if( io == "OUT7") { _nOutputSim &= ~( v2 << 7 ); v1 = 7; }
				else if( io == "IN0") _nInputSim &= ~v2; 
				else if( io == "IN1") _nInputSim &= ~( v2 << 1 ); 
				else if( io == "IN2") _nInputSim &= ~( v2 << 2 ); 
				else if( io == "IN3") _nInputSim &= ~( v2 << 3 ); 
				else if( io == "IN4") _nInputSim &= ~( v2 << 4 ); 
				else if( io == "IN5") _nInputSim &= ~( v2 << 5 ); 
				else if( io == "IN6") _nInputSim &= ~( v2 << 6 ); 
				else if( io == "IN7") _nInputSim &= ~( v2 << 7 ); 
				else if( io == "ALL") 
				{
					_nInputSim = 0x0; 
					_nOutputSim = 0x0; 
				}
			}

			// 통신 
			if( CAllInstances::Kss1500RobotDialog->_bRs232Use )
			{
				int iParam[4]; 
				iParam[0] = v1; 
				CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_RESET, iParam, NULL);
			}

			currentProgramLineNotYetPartiallyProcessed=true;
			currentProgramLine++; 
		}

		if (cmd == ID_FOR)
		{ // FOR
			int min, max, step, jump; 
			CString var; 

			var = CA2CT(_compiledRobotLanguageProgram[currentProgramLine].strLabel[1].c_str());
			min = _compiledRobotLanguageProgram[currentProgramLine].intParameter[0]; // min
			max = _compiledRobotLanguageProgram[currentProgramLine].intParameter[1]; // max
			step = (int)_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0]; // step 
			jump = (int)_compiledRobotLanguageProgram[currentProgramLine].floatParameter[1]; // next 

			TRACE("build: NEXT = %d, %d, %d \r\n", min, max, step); 

			if( min <= max )
			{
				min+=step; 
				m_mIntVar.SetAt(var, min);
				_compiledRobotLanguageProgram[currentProgramLine].intParameter[0] = min; 

				currentProgramLine++; 

			}
			else
			{
				currentProgramLine= jump;  //shseo 2015_0309 -1 add 
			}
			currentProgramLineNotYetPartiallyProcessed=true;

		}

		if (cmd == ID_NEXT)
		{ // NEXT
			currentProgramLine = _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0] -1; 
			currentProgramLineNotYetPartiallyProcessed=true;
		}

 		if (cmd == ID_ENDIF)
		{ // NEXT
			firstIfElse.bIfElseEnable_ = false;
			firstIfElse.bIgnoreCondition_ =  false;
			currentProgramLine++; 
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		if (cmd == ID_SWITCH)
		{ // SWITCH
			float fBranch = 0;
			bool bRtnValue = GetDataFromMap(_compiledRobotLanguageProgram[currentProgramLine].strLabel[1].c_str(), fBranch); 
			std::map<int, int>::iterator iBranch;
			iBranch = _compiledRobotLanguageProgram[currentProgramLine].switchBranch->find((int) fBranch);

			if(_compiledRobotLanguageProgram[currentProgramLine].switchBranch->end() != iBranch)
				currentProgramLine = (*iBranch).second; 
			else 
			{
				iBranch = _compiledRobotLanguageProgram[currentProgramLine].switchBranch->find((int) 0xffff);
				
				if(_compiledRobotLanguageProgram[currentProgramLine].switchBranch->end() != iBranch)
					currentProgramLine = (*iBranch).second; 
				else
						currentProgramLine = _compiledRobotLanguageProgram[currentProgramLine].intParameter[1];
			}

			currentProgramLineNotYetPartiallyProcessed=true;
		}
		
		if (cmd == ID_CASE || cmd == ID_BREAK || cmd == ID_DEFAULT )//	|| cmd == ID_NEXT) // NEXT 명령어는 FOR문과만 사용 - 2016.02.22
		{ // BREAK
			currentProgramLine = _compiledRobotLanguageProgram[currentProgramLine].intParameter[1] -1; 
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		if (cmd == ID_DELAY)// DELAY - 변수 ok
		{ 
			float timeToWait; 
			std::string word = _compiledRobotLanguageProgram[currentProgramLine].strLabel[0]; 
			if( word == "")  
			{
				timeToWait=_compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];

				
			}
			else 
			{
				CString str = CA2CT(word.c_str());
				GetDataFromMap(str, timeToWait) ; 
				timeToWait=timeToWait/1000.0f; // convert from ms to s

			}
			timeToWait-=timeAlreadySpentAtCurrentProgramLine;
			currentProgramLineNotYetPartiallyProcessed=false;

			//TRACE("delay: timeToWait = %f \r\n", timeToWait); 

			if (timeToWait>deltaTime)
			{
				timeAlreadySpentAtCurrentProgramLine+=deltaTime;
				
				break;
			}
			else
			{
				deltaTime-=timeToWait;
				currentProgramLine++;
				//int emp = _nOutputM;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
		}
		if (cmd == ID_GOTO)
		{ // GOTO
			currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0]; // we jump
			currentProgramLineNotYetPartiallyProcessed=true;
		}
		if (cmd == ID_GOSUB)
		{ // GOSUB
			_arrReturn.push_back(currentProgramLine);

			currentProgramLine=_compiledRobotLanguageProgram[currentProgramLine].intParameter[0]; // we jump
			currentProgramLineNotYetPartiallyProcessed=true;		
		}
		if (cmd == ID_RETURN)
		{ // RETURN
			if( _arrReturn.size() < cReturnUse +1) 
				return("RETURN Error !"); // program end

			TRACE("_arrReturn.GetSize() = %d, cReturnUse =%d \r\n", _arrReturn.size(),cReturnUse); 

			int i = _arrReturn.at(cReturnUse++); 
			currentProgramLine= i+1; 
			currentProgramLineNotYetPartiallyProcessed=true;
		}

		if (cmd == ID_PRESET)	// PRESET
		{
			std::string codeWord;
			bool b_sim = false, _b_232 = false; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x02 ) // rs232
			{
				if( _bSend )
				{
					int iSendData[4];
					iSendData[0] = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0];
					iSendData[1] = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[1];
					iSendData[2] = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[2];
					iSendData[3] = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[3];

					CAllInstances::Kss1500RobotDialog->m_CommPort.SendCmd(CMD_PRESET, iSendData, NULL);
					_bSend = false; 
				}
				else if ( CAllInstances::Kss1500RobotDialog->m_CommPort.CheckCmdRecvComp(CMD_PRESET) ) 
				{
					_b_232 = true;	// 완료
				}
			}
			else 
				_b_232 = true; 

			if( CAllInstances::Kss1500RobotDialog->m_iRunMode & 0x01 ) // simulator
			{
				CString strInputIO, strOutputIO;
				int iGetValue;

				strInputIO.Format("IN%d", (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[0]);
				strOutputIO.Format("OUT%d", (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[2]);
				int iInputData = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[1];
				int iOutputData = (int) _compiledRobotLanguageProgram[currentProgramLine].floatParameter[3];

				if (strInputIO == "IN-1")	// PRESET OFF
					b_sim = TRUE;
				else if (GetDataFromSimIO(strInputIO, iGetValue))
				{
					if (iGetValue == iInputData)
					{
						SetDataToSimOut(strOutputIO, iOutputData);
						b_sim = TRUE;
					}
				}
			}
			else
				b_sim = TRUE;

			// 동작 완료
			if( b_sim && _b_232 )
			{
				currentProgramLine++;
				currentProgramLineNotYetPartiallyProcessed=true;
			}
			else 
				break; 
		}	// PRESET End
				
		timeAlreadySpentAtCurrentProgramLine=0.0f;

		if (int(_compiledRobotLanguageProgram.size()) <= currentProgramLine)
			return(""); // program end
		loopCnt++;
		
		if (loopCnt > 1000)
			break; // We looped too often... maybe waiting for an input signal or simply infinite loop! we leave here
		cmd=_compiledRobotLanguageProgram[currentProgramLine].command;

		if( _preProgramLine != currentProgramLine ) // shseo 2014_1203 add
		{
			_bSend = true; 
			_b_232 = false; 
			_iRotCase = 0;
			_bDrive = true; 
		}
		_preProgramLine = currentProgramLine; // shseo 2014_1203 add

	}


	//////////////////////////////////////////////////////////////////////////
	// 출력 데이터 체크 - 2016.02.17
// #ifdef _DEBUG
// 	CString sstr, szFile;
// 
// 	sstr.Format("Curr X Pos : %f\tCurr Y Pos : %f\nCurr 1Angle : %f\tCurr 2Angle : %f\n",
// 		m_CurPos.fX, m_CurPos.fY, m_CurRot.fR1, m_CurRot.fR2);
// 
// 	sstr += "\r\n";
// 	szFile = "d:\\KSS1500_MOVE_Point.txt";
// 
// 	std::ofstream file2(szFile, std::ios_base::app | std::ios_base::out);
// 	file2 << sstr;
// 	file2.close();
// #endif
	// End - 2016.02.17

	return("Normal"); // return the string command that is being executed now
}

bool CKss1500Robot::Grip(float _fMotorGrip_Cur_Ang)
{
	//Proximity_sensor이름으로 정의된 센서 모듈의 핸들값을 얻는다.
	simFloat* fDetectPos = NULL;
	simInt* iDetObjectHnd = NULL;
	simFloat* fDetectNorPos = NULL;
	const float GRIPMOVE_SPEED = 0.04;
	const float GRIP_STROKE_OFFSET = 0.02; 
	simInt objectSensor = simGetObjectHandle("AttachSensor");
	simInt connector = simGetObjectHandle("GripperCover");
	simFloat maxiTorque = 0;
	simInt iTmps = 0;
	int iCurrDirection = 0;
	static int iPrevDirection = 0;
	static float prevMotorCommand = _fMotorGrip_Cur_Ang;
	simFloat fGripperStroke = -_fMotorGrip_Cur_Ang * 10000;
	simInt iTempInt = simJointGetForce(_iGripJointHandles[1],&maxiTorque);

	// 그립 방향 판별
	if (_fMotorGrip_Cur_Ang - prevMotorCommand > 0 )
		iCurrDirection = 1;
	else if (_fMotorGrip_Cur_Ang - prevMotorCommand < 0 )
		iCurrDirection = -1;
	else
		iCurrDirection = 0;

	// 떨어뜨릴때 
	if(iCurrDirection != iPrevDirection && iCurrDirection < 0) {	//	Dropping
			//	흡착기를 이용하여 붙인 물체를 떨어뜨린다. 
			simInt ParamLength = 0;	//	 false <- 5글자
			simInt TempInt = simGetObjectHandle("suctionPadLoopClosureDummy1#0");
			TempInt = simGetObjectParent(TempInt);
			simChar* scriptChar = simGetScriptSimulationParameter(sim_handle_all,"active",&ParamLength);

			// suctionPad 요소에 false 신호를 전송
			ParamLength = 5;
			simInt obj = simGetScriptHandle("suctionPad");
			TempInt = simSetScriptSimulationParameter(obj,"active", "false", ParamLength);
			
			// 공작물의 부모/자식 관계를 해제
			simSetObjectParent(_iGraspHandle,-1,true);

			//	그립력을 낮추도록 조정
			iTempInt = simSetJointForce(_iGripJointHandles[1],10.5);

			//	 위치지령
			iTempInt = simSetJointPosition(_iGripJointHandles[0],_fMotorGrip_Cur_Ang); //1축 모터	
			iTempInt = simSetJointTargetVelocity(_iGripJointHandles[1],GRIPMOVE_SPEED); 
			iTempInt = simSetJointTargetPosition(_iGripJointHandles[1],2*_fMotorGrip_Cur_Ang+GRIP_STROKE_OFFSET); //	2축 모터
	}
	//	집을 때
	else if(iCurrDirection != iPrevDirection && iCurrDirection > 0) {	
			simInt ParamLength = 0;

			simInt iTempInt = simJointGetForce(_iGripJointHandles[1],&maxiTorque);			
			iTempInt = simSetJointTargetVelocity(_iGripJointHandles[1], GRIPMOVE_SPEED); //1축 모터	
			simChar* scriptChar = simGetScriptSimulationParameter(sim_handle_all,"active",&ParamLength);
			
			//	본래 걸린 힘에 최대 5배의 조인트 포스를 가한다. 
			simSetJointForce(_iGripJointHandles[1],(GRIP_STOP_TORQUE*2));
			iTempInt = simSetJointTargetPosition(_iGripJointHandles[1],2*_fMotorGrip_Cur_Ang+GRIP_STROKE_OFFSET);	//	2축 모터
			simSetJointPosition(_iGripJointHandles[0],_fMotorGrip_Cur_Ang); //1축 모터	
			
			// 그리퍼가 잡을 공작물의 속성을 파악한다.
			simInt* iParameter = NULL;
			simFloat* fParameter = NULL;
			int index = 0;

			//	그리퍼 주변에 있는 물체가 무엇인지 찾는다.
			while(TRUE)
			{ 
				int shape = simGetObjects(index,sim_object_shape_type);

				// Object Index 값이 할당이 안된 경우는 Shape이 없는 경우임.
				if (shape == -1) 
					break;

				//	Model Property 값을 조사
				iTempInt = simGetModelProperty(shape);

				int iCond1 = simGetObjectIntParameter(shape,3003,iParameter);	//	static
				int iCond2 = simGetObjectIntParameter(shape,3004,iParameter);	//	respondable

				//	근접센서에 감지된 형상이 static & respondable 형식인지 판별
				if((simCheckProximitySensor(objectSensor,shape, fParameter)==1 ) &&iCond1 == 0 && iCond2 != 0)
				{
					// 공작물을 자식 모델로 변경
					_iGraspHandle = shape;
					iTempInt = simSetObjectParent(_iGraspHandle,connector,true);
					iTempInt = simGetObjectHandle("suctionPadLoopClosureDummy1#0");
					iTempInt = simGetObjectParent(iTempInt);

					//	Suction on
					simInt ParamLength = 0;
					simChar* scriptChar = simGetScriptSimulationParameter(sim_handle_all,"active",&ParamLength);
					simInt obj = simGetScriptHandle("suctionPad");
					ParamLength = 4;  // true <- 4
					iTempInt = simSetScriptSimulationParameter(obj,"active", "true", 4);
					break;
				}
				index=index+1;
			}
	}
	//	이동
	else 
	{
		//	일정 토크 이상, 그립을 죄는 방향의 입력이 들어오면 그리퍼 핑거의 이동을 멈추도록 한다. 
		if(fabs(maxiTorque) > GRIP_STOP_TORQUE*0.1 && iCurrDirection > 0)
		{
			simSetJointPosition(_iGripJointHandles[0],prevMotorCommand); //1축 모터	
			simInt TempInt = simSetJointTargetVelocity(_iGripJointHandles[1],GRIPMOVE_SPEED); 	
			TempInt = simSetJointTargetPosition(_iGripJointHandles[1],2*prevMotorCommand+GRIP_STROKE_OFFSET);	//2축 모터	
			_fMotorGrip_Cur_Ang = prevMotorCommand;
		}
		else 
		{
			simSetJointPosition(_iGripJointHandles[0],_fMotorGrip_Cur_Ang); //1축 모터	
			simInt TempInt = simSetJointTargetVelocity(_iGripJointHandles[1],GRIPMOVE_SPEED); 	
			TempInt = simSetJointTargetPosition(_iGripJointHandles[1],2*_fMotorGrip_Cur_Ang+GRIP_STROKE_OFFSET); //2축 모터	
		}
	}

	//	그립을 죄는 방향인지 푸는 방향인지 판별하기 위해...
	prevMotorCommand = _fMotorGrip_Cur_Ang;
	iPrevDirection = iCurrDirection; 
	
	return true;
}


bool CKss1500Robot::Run(float deltaTime)
{
	bool b[8] ={0,0,0,0,0,0,0,0}; 

	b[0] = MovePos(deltaTime, m_TarPos.fX, m_CurPos.fX, m_fPosVel[0]); 
	b[1] = MovePos(deltaTime, m_TarPos.fY, m_CurPos.fY, m_fPosVel[1]); 
	b[2] = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]);
	b[3] = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
	b[4] = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 
	b[5] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 
	b[6] = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 

	//TRACE("b = %d, %d, %d, %d | %d, %d, %d, %d \r\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);

	return (b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6]); 
}

bool CKss1500Robot::Run_Home(float deltaTime)
{
	if( !_run_home ) return false; 
	
	bool b[6] = {0,0,0,0,0,0}; 
	
	if( !b[0] )
	{
		float rot = 0; 
		float r = 0.013 * rot / 180 - 0.02;

		// 180 max 
		SetGrpOut(r);
		b[0] = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 
	}

	if ( b[0] && !b[1] )
	{
		m_bIK = true; 
		SetPosZ(0); 
		b[1] = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
	}

	if( b[1] && !b[2])
	{
		m_bIK = false; 
		SetRotR3(0);
		b[2] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 
	}

	if ( b[1] && b[2] ) 
	{
		m_bIK = false; 
		SetRot(-120, -115, 0); 
		b[3] = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
		b[4] = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 
		b[5] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 
	}
	
	if( b[0] && b[1] && b[2] && b[3] && b[4] && b[5] )
	{
		_run_home = false; 
		return false; 
	}

	return true ; 
}

void CKss1500Robot::Set_Run_WorkMove_Pos(float fX, float fY, float fZ)
{
	_GeoWorkMove.fX = fX; 
	_GeoWorkMove.fY = fY; 
	_GeoWorkMove.fZ = fZ; 


	_iRotCase = 0;
	_b_Zmove2 =false; 
	_b_RotRead = true; 
}

void CKss1500Robot::Set_Run_WorkMove_Rot(float PieceRot, float GripRot)
{
	if( _b_RotRead == false ) 
		return ; 
	
	_WorkMoverot = (PieceRot - GripRot) * 180 / piValue; 
}

bool CKss1500Robot::Run_WorkMove(float deltaTime)
{
	if( !_run_workmove ) return false; 
	if( !CAllInstances::Kss1500RobotDialog->ValidateMove(_GeoWorkMove.fX, _GeoWorkMove.fY, _GeoWorkMove.fZ) ) 
	{
		_iRotCase = 0;
		_b_Zmove2 =false; 
		_b_RotRead = true; 
		_run_workmove = false; 
		return false; 
	}

	float d1_dumm, d2_dumm, d3_dumm ;
	
	//d1_dumm = d1 / 1000; 
	//d2_dumm = d2 / 1000;
	//d3_dumm = d3 / 1000 * (-1);
	d1_dumm = _GeoWorkMove.fX; 
	d2_dumm = _GeoWorkMove.fY; 
	d3_dumm = _GeoWorkMove.fZ; 

	bool b_sim = false, _b_232 = false; 
	bool bb[4] = {0,0,0,0};
	bool b[4] = {0,0,0,0};
	bool bpre[2] = {0,0}; 
	static bool bZ =false; 

	// IsRotateCase 오류 수정 - 2015.06.30
	if( _iRotCase == 0 ) 
	{
		//////////////////////////////////////////////////////////////////
		// IsRotateCase() 대신에 IK를 사용한 ValidateMove()를 사용 - 2015.07.01

		//_iRotCase = IsRotateCase(m_CurPos.fX, m_CurPos.fY, d1_dumm, d2_dumm); 

		_iRotCase = 1;
		double tJoint1 =0;
		double tJoint2 =0;
		CAllInstances::Kss1500RobotDialog->ValidateMove(m_CurPos.fX, m_CurPos.fY, tJoint1, tJoint2);

		SetRot(tJoint1, tJoint2, m_CurRot.fR3);
		// End - 2015.07.01
	}
	
	if( !bb[0] )
	{
		float rot = 0; 
		float r = 0.013 * rot / 180 - 0.02;

		// 180 max 
		SetGrpOut(r);
		bb[0] = MoveGrpOut(deltaTime, m_fTarGrpOutPos, m_fCurGrpOutPos, m_fGrpOutVel); 
	}


	if ( bb[0] && !bb[1] && !_b_Zmove2)
	{
		m_bIK = true; 
		SetPosZ(0); 
		bb[1] = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
	}

	if( bb[1] )
	{
		// IK 적용 되어 움직이도록 변경 - 2015.07.01
		/*
		if( _iRotCase >= 1 && _iRotCase != 10)
		{
			if( !bpre[0] || !bpre[1])
			{
				// 2015.06.30
				if( _iRotCase == 2 ) 
					fRotCaseValue = 90; 
				else if( _iRotCase == 3 ) 
					fRotCaseValue = -90; 
				else if( _iRotCase == 0 )
					fRotCaseValue = 0;

				m_bIK = false; 

				//SetRot(0, m_CurRot.fR2, m_CurRot.fR3); 
				SetRot(fRotCaseValue, m_CurRot.fR2, m_CurRot.fR3);	// SetRot 값 변경 - 2015.06.30

				bpre[1] = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 

				if( !bpre[1] )
					return true; 
				else
					_iRotCase = 10; // pass
			}
		}
		*/
		if (_iRotCase == 1)
		{
			if( !bpre[0] || !bpre[1])
			{
				m_bIK = false; 
				bool bTagetRot1End = false;
				bool bTagetRot2End = false;

				bTagetRot1End = MoveRot(deltaTime, m_TarRot.fR1, m_CurRot.fR1, m_fRotVel[0]); 
				bTagetRot2End = MoveRot(deltaTime, m_TarRot.fR2, m_CurRot.fR2, m_fRotVel[1]); 
				bpre[1] = bTagetRot1End && bTagetRot2End;

				if( !bpre[1] )
					return true; 
				else
					_iRotCase = 10; // pass
			}
		}
		// End - 2015.07.01

		if ( !b[0] || !b[1] )
		{
			m_bIK = true; 
			SetPos(d1_dumm, d2_dumm, m_CurPos.fZ); 
			b[0] = MovePos(deltaTime, m_TarPos.fX, m_CurPos.fX, m_fPosVel[0]); 
			b[1] = MovePos(deltaTime, m_TarPos.fY, m_CurPos.fY, m_fPosVel[1]); 
		}
	}

	
	if(  (b[0] && b[1]) )
	{

		m_bIK = false; 
		//SetRotR3(-1*_WorkMoverot);
		SetRotR3(_WorkMoverot);
		TRACE("_WorkMoverot = %f \r\n", _WorkMoverot); 
		b[2] = MoveRot(deltaTime, m_TarRot.fR3, m_CurRot.fR3, m_fRotVel[2]); 
		_b_RotRead = false; 
	}
	

	if ( b[2] || _b_Zmove2 ) 
	{
		_b_Zmove2 = true; 

		m_bIK = true; 
		SetPosZ(d3_dumm);
		b[2] = MovePos(deltaTime, m_TarPos.fZ, m_CurPos.fZ, m_fPosVel[2]); 
	}




	//if( b[0] && b[1] && b[2] )
	if( _b_Zmove2 && b[2] )
	{
		_iRotCase = 0;
		_b_Zmove2 = false; 
		_b_RotRead =true; 
		_run_workmove = false; 
		return false; 
	}



	return true; 
}

UINT RecvConnection(LPVOID pParam)
{
	
	TRACE("Waiting to client...\n");
	CKss1500Robot *p = (CKss1500Robot *)pParam; 
	
	if(_connection !=NULL)
	if (_connection->connectToClient())
	{
		TRACE("Connected to client.\n");
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetIpcText("Connect"); 
		CAllInstances::Kss1500RobotDialog->m_pCmdDlg->EnableIpcBtn(false); 
		
		while (true)
		{
			TRACE("."); 

			int dataSize;
			char* data=_connection->receiveData(dataSize); // shseo - 데이터 취득 
			if (dataSize>0)
			{ // We received data. The server ALWAYS replies!

				TRACE("*"); // Indicate when data is received

				std::vector<char> returnData;
				unsigned char command=data[0];
				//if (command==3)
				{ // compile command
					std::string prg;
					for (int i=0;i<dataSize;i++)
					{
						prg+=data[i];
					}

					CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetEditCommand(prg); 

					//_running
					prg.clear(); 
					p->SendDataToIpc(prg, 99); // 99 는 data 수신 피드백 
		
				}
			}
			if (dataSize==-1)
			{
				TRACE("error\n");
				break; // error
			}

			// if (dataSize==0) // time out 은 loop 로 되돌아가서 다시 대기한다. 
		}
	}
	else
		TRACE("Failed connecting to client.\n");

	TRACE("exit.\n");

	// 2015_0515 shseo add 

	std::string e = "";
	CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetEditCommand(e); 
	CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetIpcText("Disconnect"); 
	CAllInstances::Kss1500RobotDialog->m_pCmdDlg->EnableIpcBtn(true); 

	//_connection = NULL; 
	if( _connection ) // shseo 2015_0306 add
	{
		delete _connection;
		_connection = NULL;
	}


	return TRUE;
}

//------------------------------------------------------------------------------------

void CKss1500Robot::ConnectIpc()
{
	if( _connection ) 
	{
		delete _connection;
		_connection=NULL;
	}
	_connection = new CSimpleInConnection(portNb); 

	AfxBeginThread(RecvConnection, this);
	CAllInstances::Kss1500RobotDialog->m_pCmdDlg->SetIpcText("Waiting to CubicNote"); 
}

void CKss1500Robot::DisConnectIpc()
{
	if( _connection ) 
	{
		delete _connection;
		_connection=NULL;
	}
}


void CKss1500Robot::SendDataToIpc(std::string &message, int command, int m) 
{
	if( !_connection ) return ; 

	std::vector<char> returnData;
	 

	// Now execute the command:
	//std::string message("server data!");
	// Now prepare the return data:
	returnData.push_back(command);
	// 추후 호환성을 위해 3 바이트 더 쓴다. 

	
	if( command == 3 ) // build info
	{
		returnData.push_back((m >>8) &0xff);
		returnData.push_back(m & 0xff);
		returnData.push_back(0);

		TRACE("line = %d, %d, %d \r\n", m, (m >>8) &0xff, m & 0xff); 
	}
	else
	{
		returnData.push_back(0);
		returnData.push_back(0);
		returnData.push_back(0);
	}

	for (int i=0;i<int(message.length());i++)
		returnData.push_back(message[i]);
		
	if (_connection->replyToReceivedData(&returnData[0],int(returnData.size())))
	{
		TRACE("send - ok.\n");
		return ; 
	}

}