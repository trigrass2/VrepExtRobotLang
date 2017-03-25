#include "Kss1500MotionPlannar.h"
#include "CommSubscriberConrete.h"
#include "RobotMessages.h"

typedef CCompiledProgramLine::PGM_CMD CMD;
typedef CRobotLangParserAndOperator::VAR_TYPE VAR;
typedef CRobotMessages::EWarningID WRN;
typedef CRobotMessages::eRunTimeReturnCode RTNCODE;
typedef std::pair<std::string, float> pairData;
typedef std::pair<int, CRobotMessages::ECompileError> errorData;
typedef std::pair<int, CRobotMessages::EWarningID> warningData;

#define LOOP_LIMIT 1000

static std::string serialCommName("serialPort");
static std::string IPCName("IPC");
static std::string iRoDiName("simulator");

CKss1500MotionPlannar::CKss1500MotionPlannar()
    :  CKss1500Interprerter()
{
    _commModules = nullptr;
    _commModules = new CCommSubjectConrete;
    _bCalRelativeValueForDrive = false;
    _fTarJointValByRelJointVal = 0.0f;
      Reset();
}

CKss1500MotionPlannar::~CKss1500MotionPlannar()
{

    delete _commModules;
}

void CKss1500MotionPlannar::Reset(void)
{
    _timeAlreadySpentAtCurrentProgramLine = 0;
    _previousProgramLine = 0;
    _currentProgramLine = 0;
    _timeAlreadySpentAtCurrentProgramLine = 0.0f;
    _bStopAtMotion = 0;
    LockSignalDataUpdate(false);
    _eInterpreterStatus = MSTATUS_STANDBY;
    ClearCompliedErrorLog();
    SetSendingIoSignalToSerialPort(true);
}

void CKss1500MotionPlannar::SetStopAtMotionSwitch(bool sw)
{
    _bStopAtMotion = sw;
}

bool CKss1500MotionPlannar::GetStopAtMotionStatus(void)
{
    return _bStopAtMotion;
}

const CCompiledProgramLine* CKss1500MotionPlannar::GetCurrentCompiledProgramLine(void)
{
    if (GetCurrentLineNumber() >= (int) _compiledRobotLanguageProgram.size())
        return nullptr;

    return &_compiledRobotLanguageProgram[GetCurrentLineNumber()];
}

const CCompiledProgramLine* CKss1500MotionPlannar::GetPreviousCompiledProgramLine(void)
{
    if (GetPreviousLineNumber() >= (int) _compiledRobotLanguageProgram.size())
        return nullptr;

    return &_compiledRobotLanguageProgram[GetPreviousLineNumber()];
}

int  CKss1500MotionPlannar::GetRobotRunStatus(void) {
    return (int) _eInterpreterStatus;
}

void CKss1500MotionPlannar::SetRobotRunMode(int mode)
{
    _eInterpreterStatus = (CKss1500Interprerter::eInterpreterStatus) mode;
}

void CKss1500MotionPlannar::SetCurrentLineNumber(int lineNO)
{
    if (_previousProgramLine != lineNO)
        _previousProgramLine = _currentProgramLine;

    _currentProgramLine = lineNO;
}

int CKss1500MotionPlannar::GetCurrentLineNumber(void)
{
    return _currentProgramLine;
}


int CKss1500MotionPlannar::GetPreviousLineNumber(void)
{
    return _previousProgramLine;
}

void CKss1500MotionPlannar::SetPreviousLineNumber(int lineNO)
{
    _previousProgramLine = lineNO;
}

int CKss1500MotionPlannar::GetTotalLineNumber(void)
{
    return _compiledRobotLanguageProgram.size();
}

void CKss1500MotionPlannar::SynchronizeCurrToTarget(void)
{
    SetTargetJointValue(_fCurrentJointValue[0],_fCurrentJointValue[1],_fCurrentJointValue[2],_fCurrentJointValue[3]);
    _fTargetGripperValue = _fCurrentGripperValue;
}
/*
void CKss1500MotionPlannar::SetReadyToCommandMode(void)
{
    Reset();
    _compiledRobotLanguageProgram.clear();
    CCompiledProgramLine singleLine;
    _compiledRobotLanguageProgram.push_back(singleLine);
    _eInterpreterStatus = MSTATUS_COMMAND;
    SetCurrentLineNumber(0);
}
*/

bool CKss1500MotionPlannar::MakeOnelineProgramforCommandMode(const std::string& prog)
{
    _eInterpreterStatus = MSTATUS_COMMAND;

    SetProgram(prog);

    int nTotalError =0, iFirstErrorLine = 0;
    CompileCode(nTotalError, iFirstErrorLine);

    SetPreviousLineNumber(-1);
    SetCurrentLineNumber(0);

	ResetRelativeJointValue();

    if(nTotalError >0)
        return false;
    else
        return true;
}

void CKss1500MotionPlannar::ResetRelativeJointValue(void) {
	_bCalRelativeValueForDrive = true;
	_fTarJointValByRelJointVal = 0.0f;
}

bool CKss1500MotionPlannar::SetCustomMotionData(std::string& data)
{
    vector<string> extracedWords;

    CRobotLangParserAndOperator::ExtractMultiWord(data,extracedWords);

    float axes[TOTAXES] = {0.0f};
    if(extracedWords.size() > 0)
    {
        if(extracedWords[0].compare("JOINTS") != 0)
            return false;

        for(int i = 1;i<extracedWords.size();i++)
        {
            axes[i-1] = atof(extracedWords[i].c_str());
        }

        SetCurrentJointValues(axes);
    }
    else
        return false;

    SetCurrentGripperPos(axes[4]);

    return true;
}
int CKss1500MotionPlannar::GetSendingIoSignalType(void)
{
    return CRobotLangParserAndOperator::GetSendingIoSignalType();
}

int CKss1500MotionPlannar::RunProgram(float deltaTime)
{

    // This function runs the compiled robot language program for "deltaTime" seconds
    // return value "" means program ended!

    static CIfElseThen firstIfElse;
    static bool currentProgramLineNotYetPartiallyProcessed = false;
    int cmd = CMD::ID_BLANK;
    int loopCnt = 0;
    int iWarningMsg = 0;

    SetDeltaTime(deltaTime);

    CRobotMessages::eRunTimeReturnCode rtnCode = RTNCODE::PROGRAM_NORMALLY_PROGRESSING;

    if (_eInterpreterStatus == MSTATUS_COMMAND) {
		 SetCurrentInterpreterStatus(MSTATUS_RUN);

         // to stop gripper
         _commModules->NotifyToAllSubscribers();
    }

    if (_compiledRobotLanguageProgram.size() ==0)
    {
        return eRunTimeReturnCode::LINE_AT_PROGRAMEND; // program end
    }
    else if (int(_compiledRobotLanguageProgram.size()) <= _currentProgramLine)
    {
        return eRunTimeReturnCode::LINE_AT_PROGRAMEND; // program end
    }

    cmd = _compiledRobotLanguageProgram[_currentProgramLine]._command; // get the current command

   
    if (_previousProgramLine != _currentProgramLine)
    {
        currentProgramLineNotYetPartiallyProcessed = false;
		ResetRelativeJointValue();

        if ((_previousProgramLine > -1 
			&& _previousProgramLine < (int) _compiledRobotLanguageProgram.size()) 
			&& (int) _compiledRobotLanguageProgram.size() > _currentProgramLine)
            if (_compiledRobotLanguageProgram[_previousProgramLine]._iLineNumber != _compiledRobotLanguageProgram[_currentProgramLine]._iLineNumber)
            {
                std::vector<float> currPos;
                currPos.push_back(_XyzCurrentPosition.X);
                currPos.push_back(_XyzCurrentPosition.Y);
                currPos.push_back(_XyzCurrentPosition.Z);
                currPos.push_back(_fCurrentJointValue[3]);
                GetCurrentRobotMotionInfo(_compiledRobotLanguageProgram[_currentProgramLine], currPos, _fCurrentGripperValue, true);
            }
    }

	if (_eInterpreterStatus == MSTATUS_STANDBY)
		_previousProgramLine = _currentProgramLine;
	else if (_eInterpreterStatus != MSTATUS_COMMUNCATION)
		_eInterpreterStatus = MSTATUS_RUN;
	
    if (_eInterpreterStatus == MSTATUS_STANDBY || _eInterpreterStatus == MSTATUS_RUN)
    {

        if (cmd == CMD::ID_VALUE) // 변수 = 값; 변수 = 변수; 변수 = INT ; OUT = 변수(값);
        {
            std::string codeWord;
            float fRealValue = 0.0f, fstrValue = 0.0f;	//float f;
            int iSimValue = 0, iRealValue = 0;
            int iWarnMessage = -1;
            float fValue1[7] = { 0.0f, };
            float fValue2[7] = { 0.0f, };
            std::string word = (_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[4].compare("POSE") == 0) ? _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0] : _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1];
            int iRtnValue = 0;

            //	A = A + 3   <- strLabel[0] = A, strLabel[1] = A, strLabel[2] = A+3,
            if (_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0] == 1)	//	A= 3*X
            {
                word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2];
                iRtnValue = DoMathOperation(word, iWarningMsg);

                if (iRtnValue == FALSE)
                {
                    iRtnValue = DoLogicalOperation(word, iWarningMsg);
                    if (iRtnValue == FALSE)
                        iRtnValue = DoRelationalOperation(word, iWarningMsg);
                }

                _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0] = (float) atof(word.c_str());
                word.clear();
            }
            //	P1 = P1+P2
            else if (_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[4].compare("POSE") != 0)
            {
                //	P1 = P2 or DefPOsX = 10,20,30
                if (GetDataFromUserPose(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0].c_str(), fValue1))
                {
                    //	P1 = P2
                    if (_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2].empty())
                    {
                        GetDataFromUserPose(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2].c_str(), fValue2);
                        GetDataFromUserPose(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0].c_str(), fValue2);
                    }
                }
            }
            else	//	P0= P1+$(X,10,20)
            {
                float fTargValue[7] = { 0.0f, };

                std::vector<float> currPos;
                currPos.push_back(_XyzCurrentPosition.X);
                currPos.push_back(_XyzCurrentPosition.Y);
                currPos.push_back(_XyzCurrentPosition.Z);
                currPos.push_back(_fCurrentJointValue[3]);
                GetCurrentRobotMotionInfo(_compiledRobotLanguageProgram[_currentProgramLine], currPos, _fCurrentGripperValue);

                //  뒷부분 연산이 없거나 Xpos = P1 or P1 = P2 의 연산으로 변경된 후의 연산
                if (DoPoseOperation(_compiledRobotLanguageProgram[_currentProgramLine], fTargValue, word, currPos, _fCurrentGripperValue) > 0)
                {
                    if(IsInternalUserdefPose(word.c_str()))
                        SetDataToUserPose(word.c_str(), fTargValue);
                    else if (IsInternalPose(word.c_str()))
                        SetDataToPose(word.c_str(), fTargValue, iWarnMessage);
                    goto POSE_LABEL;
                }
                else
                    #ifndef QT_COMPIL
                        ASSERT(-1);
                    #else
                        Q_ASSERT(false);
                    #endif
            }

            //	A = 30, 숫자 입력
            if (word.empty())
            {
                fstrValue = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            }
            else
            {
                //	A = B, 변수 입력
                bool bRtnValue = GetDataFromMap(word, fstrValue);

                if (bRtnValue == false) // 일반 변수
                {
                    if (IsDataPassingByExternalDevice())
                    {
                        if (GetDataFromIO(word.c_str(), iRealValue)) // io 변수
                            fstrValue = fRealValue = (float)iRealValue;
                    }

                    else if (IsInternalArray(word))
                    {
                        ADDRESS  addType;
                        if (!GetValueAtOneWord(word, fstrValue, addType, iWarningMsg))
                            return CRobotMessages::ECompileError::ERR_VAR;

                        if (IsInternalArray(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0])) {
                            if (!SetDataToArray(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0], fstrValue, iWarningMsg))
                                return CRobotMessages::ECompileError::ERR_VAR;
                        }
                    }
                }
            }

            //
            codeWord = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];

            if (IsInternalPorts(word.c_str()))
            {
                GetDataFromIO(word.c_str(), iSimValue);		// 변수명이 포트 인 경우
                //iSimValue = (int)fstrValue; //	04_IF_THEN_ELSE
                //SetDataToOut(word.c_str(), iSimValue);

                //	X = IN0
                if (GetDataFromMap(codeWord.c_str(), fRealValue))
                    _internalVariable[codeWord.c_str()].SetValue((float) iSimValue);
            }
            // 배열처리
            else if(IsInternalArray(codeWord))
            {
                if (!SetDataToArray(codeWord, fstrValue, iWarnMessage))
                    return RTNCODE::WRONG_ALLOCATION;
            }
            else
            {
                if (IsInternalPorts(codeWord.c_str()))
                {
                    SetDataToOut(codeWord.c_str(), int(fstrValue));

                    if(GetSendingIoSignalToSerialPortStatus())
                        LockSignalDataUpdate(true);
                }
                else
                    _internalVariable[codeWord.c_str()].SetValue((float) fstrValue);
            }
            //	단순히 할당후 빠져나오기 위해 사용됨

        POSE_LABEL:
            if (!IsDataPassingByExternalDevice())
                _eInterpreterStatus = MSTATUS_STEPDONE;
            else if(!(GetSendingIoSignalToSerialPortStatus() && GetLockSignalDataUpdateStatus()))
                _eInterpreterStatus = MSTATUS_STEPDONE;
        }
        else if (cmd == CMD::ID_GOHOME) // GOHOME
        {
            bool b[6] = { 0,0,0,0,0,0 };

            // simulator - release -> z up + rotate -> joint home
            /*if (IsDataPassingBySimulator())*/
            {
                //SetTargetJointValue(0, 0, 0, 0);

                if(MoveCommand(HOME,0,0,0,0,0,0))
                    _eInterpreterStatus = MSTATUS_STEPDONE;
            }
        }

        else if (cmd == CMD::ID_MOVE || cmd == CMD::ID_READY || cmd == CMD::ID_MOVE2)	// MOVE + READY + MOVE2  - 2015.08.28
        {
            float fRobotPosData[5] = { 0.0f, };
            int iForLimit = 3;
            int iCount;
            float fTargValue[7] = { 0.0f, };
            std::string tmpString = "NULL";

            //	MOVE L, P001＋(0, 0, 10), ?＋(20, 0, 0) 처리.
            if (_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[4].compare("POSE") == 0)	//	A= 3*X
            {
                cmd = CMD::ID_MOVE2;

                std::vector<float> currPos = { _XyzCurrentPosition.X, _XyzCurrentPosition.Y, _XyzCurrentPosition.Z, _fCurrentJointValue[3] };
                bool UpdateSavedPos = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[3].compare("CLEAR_CURRENTPOS") == 0 ? true : false;
                GetCurrentRobotMotionInfo(_compiledRobotLanguageProgram[_currentProgramLine], currPos, _fCurrentGripperValue, UpdateSavedPos);

                if (DoPoseOperation(_compiledRobotLanguageProgram[_currentProgramLine], fTargValue, tmpString, currPos, _fCurrentGripperValue) < 0)
                    return CRobotMessages::ECompileError::ERR_POSEOPR;

                for (iCount = 0; iCount < NUM_POSE_PARAM; iCount++)
                    fRobotPosData[iCount] = fTargValue[iCount];
            }
            else
            {
                // MOVE2 방식의 경우 변수 추가 처리됨. (기존 루틴)
                if (cmd == CMD::ID_MOVE2)
                    iForLimit = NUM_POSE_PARAM -1;
                else
                    iForLimit = 3;	//	3axis

                for (iCount = 0; iCount < iForLimit; iCount++)
                {
                    std::string strLableWord = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[iCount];

                    if (strLableWord.empty() || strLableWord == "NULL")
                        fRobotPosData[iCount] = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[iCount];
                    else
                    {
                        if (!GetDataFromMap(strLableWord.c_str(), fRobotPosData[iCount]))
                            GetDataFromArray(strLableWord, fRobotPosData[iCount], iWarningMsg);
                    }
                }
            }

            // 범위 오류 판단
            if (!ValidateMove(fRobotPosData[0] / 1000, fRobotPosData[1] / 1000, fRobotPosData[2] / 1000))
                return(RTNCODE::KINEMATIC_ERROR); // program end
            if (!ValidateRotate(fRobotPosData[3]) && cmd == CMD::ID_MOVE2)
                return(RTNCODE::INVALID_ROTATION_RANGE); // program end
            if (!ValidateGrip(fRobotPosData[4]) && cmd != CMD::ID_MOVE2)
                return(RTNCODE::INVALUD_GRABBING_RANGE); // program end

            //	 Assigning target values
            CPoint3D targetPos(fRobotPosData[0], fRobotPosData[1], fRobotPosData[2]);
            SetTargetJointPositions(targetPos);

            if (cmd == CMD::ID_MOVE2)
                SetTargetRaxisValueByDegree(fRobotPosData[3]);

            if (cmd == CMD::ID_MOVE2)
                SetTargetGripperValueByDegree(fRobotPosData[4]);

            //XYZ 모션
            if (cmd == CMD::ID_MOVE2) {
                if (MoveCommand(MOVE2, iForLimit, (float)_fEuclidFeed, fRobotPosData[0], fRobotPosData[1], fRobotPosData[2], fRobotPosData[3]))
                    _eInterpreterStatus = MSTATUS_STEPDONE;
            }

            else if (cmd == CMD::ID_MOVE) {
                if (MoveCommand(MOVE, iForLimit, (float)_fEuclidFeed, fRobotPosData[0], fRobotPosData[1], fRobotPosData[2], fRobotPosData[3]))
                    _eInterpreterStatus = MSTATUS_STEPDONE;
            }

        }
        else if (cmd == CMD::ID_ROTATE) // ROTATE - 변수처리 ok
        {
            float rot = 0;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            if (word.empty())
                rot = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            else
                GetValueAtOneWord(word, rot, eAddr, iWarningMsg);

            if (!ValidateRotate(rot))
                return(RTNCODE::INVALID_ROTATION_RANGE); // program end

            if (MoveCommand(ROTATE, 0, (float)_fTargetJointVelocity[3], rot, rot, rot, rot))
                _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_GRASP) // GRASP - 변수처리 ok
        {
            float rot = 0;
            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2];
            if (word.empty())
                rot = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            else
            {
                int iRtnValue = DoMathOperation(word, iWarningMsg);
                if (iRtnValue < 0)
                    return CRobotMessages::eRunTimeReturnCode::INVALUD_GRABBING_RANGE;

                GetFloatFromWord(word, rot);
            }

            if (!ValidateGrip(rot))
                return CRobotMessages::eRunTimeReturnCode::INVALUD_GRABBING_RANGE;

            if (MoveCommand(GRIP, GRIP, _fTargetJointVelocity[0], (float)rot, 0, 0, 0))
                _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_LET)	//	좌표값 할 당
        {
            int iWarning = 0;
            bool error = false;
            string varPoseName;
            float fLetValue = 0.0f;
            unsigned int uAxisNum = 0xFFFF;

            uAxisNum = GetAxisNumberByAxisName(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0]);

            if (uAxisNum != 0xFFFF)
            {
                bool bRtnType = false;
                ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                varPoseName = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1];

                //	변수인지, 상수인지 판별
                bRtnType = GetValueAtOneWord(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2], fLetValue, eAddr, iWarning);

                if (bRtnType == false)
                {
                    error = true;
                    return(RTNCODE::INVALED_LET_USE);
                }

                float fValues[TOTAXES] = { 0.0f, };


                bool bThisIsPoseData = IsInternalPose(varPoseName);
                bool bThisIsUserPoseData = IsInternalUserdefPose(varPoseName);

                // POSE 데이터인지 판별
                if (bThisIsPoseData)
                    GetDataFromPose(varPoseName, fValues, iWarning);
                else if (bThisIsUserPoseData)
                    GetDataFromUserPose(varPoseName, fValues);

                if (uAxisNum < TOTAXES)
                    fValues[uAxisNum] = fLetValue;

                if (bThisIsPoseData)
                    SetDataToPose(varPoseName, fValues, iWarning);
                else if (bThisIsUserPoseData)
                    SetDataToUserPose(varPoseName, fValues);
                else
                    error = true;

                if (uAxisNum == 0xFFFF || bRtnType == false)
                {
                    error = true;
                    return(RTNCODE::INVALED_LET_USE);
                }
                _eInterpreterStatus = MSTATUS_STEPDONE;
            }
            if (error)
            {
                return(RTNCODE::INVALED_LET_USE);
            }
        }
        else if (cmd == CMD::ID_RELEASE) // RELEASE - 변수처리 ok
        {
            float rot;
            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];

            if (word.empty())
                rot = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            else
            {
                ADDRESS eAddr = ADDRESS::GENERAL_VAR;
                GetValueAtOneWord(word, rot, eAddr, iWarningMsg);
            }

            if (!ValidateGrip(rot))
            {
                return CRobotMessages::ECompileError::ERR_GRASP;
            }

            float r = (float) (0.013 * rot / 180. - 0.02);

            // 180 max
            _fTargetGripperValue = r;
            if (MoveCommand(GRIP, GRIP, _fTargetJointVelocity[0], (float)r, 0, 0, 0))
            //if (MoveCommand(GRIP, 0, (float)r, 0, 0, 0, 0))
            //if (MoveCommand(GRIP, NULL, NULL, NULL, r, NULL, NULL))
                _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_DRIVE) // DRIVE  - 변수처리 ok
        {
            int i = 0;
            float mapData;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            if (word.empty())
                i = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0];
            else
            {
                GetValueAtOneWord(word, mapData, eAddr, iWarningMsg);
                i = (int)mapData;
            }

            mapData = 0;

            word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1];
            if (word.empty())
                mapData = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            else {
                if (GetValueAtOneWord(word, mapData, eAddr, iWarningMsg))
                    _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0] = mapData;
                else if (DoMathOperation(word, iWarningMsg))
                {
                    GetFloatFromWord(word, mapData);
                    _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0] = mapData;
                }
                else
                    return(RTNCODE::INVALID_DRIVE_VALUE); // program end
            }
          
            if (_bCalRelativeValueForDrive)
			{
				if(i == MOTOR_3) // Z-axis
                    _fTarJointValByRelJointVal = mapData + 1000*(GetCurrentJointValue(i-1));
				else
                    _fTarJointValByRelJointVal = mapData + RadToDeg(GetCurrentJointValue(i-1));
				
				if (ValidateDrive(i-1, mapData))
					return(RTNCODE::INVALID_DRIVE_VALUE); // program end

                _bCalRelativeValueForDrive = false;
			}

            bool bCompleteParam = false;

            if (i == MOTOR_1)
            {
                if (MoveCommand(DRIVE, MOTOR_1, _fEuclidFeed, _fTarJointValByRelJointVal, NULL, NULL, NULL))
                    bCompleteParam = true;
            }
            else if (i == MOTOR_2)
            {
                if (MoveCommand(DRIVE, MOTOR_2, _fEuclidFeed, _fTarJointValByRelJointVal, NULL, NULL, NULL))
                    bCompleteParam = true;
            }
            else if (i == MOTOR_3)
            {
                if (MoveCommand(DRIVE, MOTOR_3, _fEuclidFeed, _fTarJointValByRelJointVal, NULL, NULL, NULL))
                    bCompleteParam = true;
            }

            if (bCompleteParam )
			{
				_eInterpreterStatus = MSTATUS_STEPDONE;
			}
        }
        else if (cmd == CMD::ID_VAR) // VAR - 변수 선언, ex) VAR AA
        {
            bool error = true;
            std::string strLvalue, strRvalue, strCommandWord;
            std::string oneLineString = _compiledRobotLanguageProgram[_currentProgramLine]._originalUncompiledCode;

            ExtractOneWord(oneLineString, strCommandWord);

            //	컴마의 갯수 만큼이 정의되는 변수의 개수가 됨.한개도 없다면 한개의 변수는 정의 된 것으로 본다.
            unsigned int uNumOfComma = GetCountComma(oneLineString) + 1;

            for (int i = 0; i < (int)uNumOfComma; i++)
            {
                int  iWarnMessage = -1;
                VAR_TYPE iVarType = VAR_TYPE::NONE;

                if (ExtractVarDefine(oneLineString, iVarType, iWarnMessage, strLvalue, strRvalue))
                {
                    error = false;
                    VARIABLE varData;

                    auto result = _internalVariable.find(strLvalue.c_str());
                    if (result != _internalVariable.end()) {
                        int iTmp = 0;
                        float fTemp = 0.0f;

                        switch ((VAR)iVarType)
                        {
                        case VAR::VAR_UINT:	// unsigned int
                            if (!strRvalue.empty()) {
                                iTmp = atoi(strRvalue.c_str());
                                _internalVariable[strLvalue.c_str()].SetValue((float)iTmp);
                            }
                            break;

                        case VAR::VAR_INT:	//	int
                            if (!strRvalue.empty()) {
                                iTmp = atoi(strRvalue.c_str());
                                _internalVariable[strLvalue.c_str()].SetValue((float)iTmp);
                            }
                            break;

                        default:
                        case VAR::VAR_FLOAT:	//	float
                            if (!strRvalue.empty()) {
                                fTemp = atof(strRvalue.c_str());
                                _internalVariable[strLvalue.c_str()].SetValue((float)fTemp);
                            }
                            break;
                        }
                    }
                }
            }
            _eInterpreterStatus = MSTATUS_STEPDONE;
        }
        else if (cmd == CMD::ID_ELSE  || cmd == CMD::ID_VAR			  || cmd == CMD::ID_ENDSWITCH    || cmd == CMD::ID_CHANGE
              || cmd == CMD::ID_BLANK || cmd == CMD::ID_LABEL_DEFAULT || cmd == CMD::ID_LINE_COMMENT || cmd == CMD::ID_LABEL
              || cmd == CMD::ID_STOP  || cmd == CMD::ID_END			  || cmd == CMD::ID_SET			 || cmd == CMD::ID_RESET
              || cmd == CMD::ID_DIM   || cmd == CMD::ID_DEFPOS)
        {
            _eInterpreterStatus = MSTATUS_STEPDONE;
        }
        else if (cmd == CMD::ID_SPEED) // SPEED - 변수처리 ok
        {
            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2];
            if (word.empty())
                _fEuclidVelocity = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            else
            {
                ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                int iRtnValue = DoMathOperation(word, iWarningMsg);
                if (iRtnValue < 0)
                    return RTNCODE::INVALUD_SPEED_RANGE;

                GetValueAtOneWord(word, _fEuclidVelocity, eAddr, iWarningMsg);
            }

            if (!ValidateSpeed(_fEuclidVelocity))
            {
                return(RTNCODE::INVALUD_SPEED_RANGE); // program end
            }

            _fCurrentGripperVelocity = (float) (INIT_GRIP_VEL * _fEuclidVelocity / 100.);

            for (float  &value: _fTargetJointVelocity)
                value = (float) (MAX_POS_VEL * _fEuclidVelocity / 100.);

            _eInterpreterStatus = MSTATUS_STEPDONE;
        }
        else if (cmd == CMD::ID_IF || cmd == CMD::ID_ELSEIF)
        {
            // IF
            std::string conditionalStatement = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[3];
            float fVarValue = 0.0f;
            int variableValue = 0;
            int conditionValue = 0;

            if (cmd == CMD::ID_IF)
                firstIfElse.clear();

            if (firstIfElse._bIfElseEnable == true && firstIfElse._bIgnoreCondition == true && cmd == CMD::ID_ELSEIF)
            {
                SetCurrentLineNumber((_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]) - 1);
                return MSTATUS_STANDBY;
            }
            // 새 조건문 처리
            if (conditionalStatement.size() > 0)
            {
                DoRelationalOperation(conditionalStatement, iWarningMsg);
                DoLogicalOperation(conditionalStatement, iWarningMsg);
                conditionValue = atoi(conditionalStatement.c_str());

                conditionalStatement = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            }
            else
            {
                int tmpValue = 0;
                conditionValue = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0];

                //	기존 조건문 처리
                //	IF IN1=0 THEN LABEL1 ELSE GOTO LABEL2
                if (GetDataFromIO(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0].c_str(), tmpValue))
                {
                    variableValue = (int)tmpValue;
                    if (variableValue == conditionValue)
                    {
#ifndef QT_COMPIL

                        TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d]._intParameter[1] =%d \r\n", _currentProgramLine, _currentProgramLine, _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1]);
#endif
                        SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1]);
                    }
                    else
                    {
#ifndef QT_COMPIL
                        TRACE("currentProgramLine =%d, _compiledRobotLanguageProgram[%d]._intParameter[2] =%d \r\n", _currentProgramLine, _currentProgramLine, _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[2]);
#endif
                        SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[2]);
                    }
                    goto ID_IF_EXIT;
                }
                else
                    conditionalStatement = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            }

            if (GetDataFromMap(conditionalStatement, fVarValue))
            {
                variableValue = (int)fVarValue;
                if (variableValue == conditionValue)
                {
                    SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1]);
                }
                else
                {
                    SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[2]);
                }
                currentProgramLineNotYetPartiallyProcessed = true;
            }
            else if ((conditionalStatement.c_str(), variableValue))
            {
                if (variableValue == conditionValue)
                {
                    SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1]);
                }
                else
                {
                    SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[2]);
                }
                currentProgramLineNotYetPartiallyProcessed = true;
            }
            else if (conditionalStatement.size() == 0)
            {
                if (conditionValue>0 && firstIfElse._bIgnoreCondition == false)
                {
                    SetCurrentLineNumber(_currentProgramLine+1);
                    firstIfElse._bIgnoreCondition = true;
                }
                else
                {
                    SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]);
                    if (_currentProgramLine <0)
                        return RTNCODE::ABNORMAL_RUNTIME_ERROR;
                }
            }
            else
            {
                return(RTNCODE::PROGRAM_TERMINATED); // program end
            }
            ID_IF_EXIT:;
        }

        else if (cmd == CMD::ID_WAIT) // WAIT - 변수처리 ok
        {
            // WAIT X=1, 500
            float timeToWait, fValue;
            int iLeftWord = 0;
            ADDRESS eAddr;
            int iRightWord = 0;
            if (GetValueAtOneWord(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1], fValue, eAddr, iWarningMsg))
                iRightWord = (int) fValue;
            else
                iRightWord = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0];

            std::string var = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            bool bMapValue = GetDataFromMap(var, fValue);
            bool bArrayValue = GetDataFromArray(var, fValue, iWarningMsg);
            bool bIOvalue = GetDataFromIO(var.c_str(), iLeftWord);

            if (IsDataPassingByExternalDevice() == false) {
                if (bMapValue || bArrayValue || bIOvalue)
                {
                    if (!bIOvalue)
                        iLeftWord = (int)fValue;

                    if (iLeftWord == iRightWord)
                        _eInterpreterStatus = MSTATUS_STEPDONE;
                    else
                    {
                        std::string word;

                        if (_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2].find_first_of(MATH_OP) == std::string::npos)
                            word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2];
                        else
                        {
                            word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2];

                            if (DoMathOperation(word, iWarningMsg) >= 0)
                            {
                                _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0] = (float)atof(word.c_str()) / 1000;
                                word.clear();
                            }
                            else
                                return CRobotMessages::ECompileError::ERR_WAIT;	// break;
                        }
                        //	 기존 루틴
                        if (word.empty())
                        {
                            timeToWait = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
                            timeToWait = timeToWait; // convert from ms to s
                        }
                        else
                        {
                            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                            GetValueAtOneWord(word, timeToWait, eAddr, iWarningMsg);
                            timeToWait = timeToWait / 1000.0f; // convert from ms to s
                        }

                        timeToWait -= _timeAlreadySpentAtCurrentProgramLine;
                        if (timeToWait > deltaTime)
                        {
                            _timeAlreadySpentAtCurrentProgramLine += deltaTime;
                            _eInterpreterStatus = MSTATUS_RUN;
                        }
                        else
                        {
                            deltaTime -= timeToWait;
                            _timeAlreadySpentAtCurrentProgramLine += deltaTime;
                            LockSignalDataUpdate(false);
                            _eInterpreterStatus = MSTATUS_STEPDONE;
                        }
                    }
                }
                else
                {
                    _eInterpreterStatus = MSTATUS_STEPDONE;
                }
            }
            else
            {
                LockSignalDataUpdate(false);
                GetValueAtOneWord(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0], fValue, eAddr, iWarningMsg);
                _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[2] = fValue;	//result
                _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[3] = (float) iRightWord;	//result

                LockSignalDataUpdate(false);
                GetValueAtOneWord(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[2], fValue, eAddr, iWarningMsg);
                _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[4] = fValue;	//result

                LockSignalDataUpdate(false);
                _eInterpreterStatus = MSTATUS_STEPDONE;
            }
        }

        else if (cmd == CMD::ID_DELAY)// DELAY - 변수 ok
        {
            float timeToWait = 0.0f;
            int iWarningMsg = 0;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            std::string word = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            if (word.empty())
            {
                timeToWait = _compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0];
            }
            else
            {
                GetValueAtOneWord(word, timeToWait, eAddr, iWarningMsg);
                timeToWait = timeToWait / 1000.0f; // convert from ms to s

            }
            timeToWait -= _timeAlreadySpentAtCurrentProgramLine;
            currentProgramLineNotYetPartiallyProcessed = false;
#ifndef QT_COMPIL
            TRACE("delay: timeToWait = %f \r\n", timeToWait);
#else
            qDebug()<< "Delay: timeToWait = "<<timeToWait;
#endif

            if (timeToWait>deltaTime)
            {
                _timeAlreadySpentAtCurrentProgramLine += deltaTime;
                return MSTATUS_RUN;
            }
            else
            {
                deltaTime -= timeToWait;
                _eInterpreterStatus = MSTATUS_STEPDONE;
            }
        }
        else if (cmd == CMD::ID_SET)
        { // SET
            int iSetParam = 1;
            std::string io = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            // SetDataToSimOut(io.c_str(), iSetParam);
            SetDataToOut(io.c_str(), iSetParam);
            _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_RESET)
        { // RESET
            int iResetParam = 0;
            std::string io = _compiledRobotLanguageProgram[_currentProgramLine]._strLabel[0];
            //	SetDataToSimOut(io.c_str(), iResetParam);
            SetDataToOut(io.c_str(), iResetParam);
            _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_FOR)
        { // FOR
            int min, max, step, jump;
            min = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]; // min
            max = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1]; // max
            step = (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0]; // step
            jump = (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[1]; // next

            #ifndef QT_COMPIL
                TRACE("build: NEXT = %d, %d, %d \r\n", min, max, step);
            #else
                qDebug()<<"build: NEXT ="<<min<<", "<<max<<", "<<step<<"\r\n";
            #endif

            if (min <= max)
            {
                _internalVariable[_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1].c_str()].SetValue((float) min);
                min += step;
                _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0] = min;
                _eInterpreterStatus = MSTATUS_STEPDONE;
            }
            else
            {
                //	Return to initial value
                _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0] = (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[2];	// Restore initial value
                SetCurrentLineNumber(jump);
            }
        }

        else if (cmd == CMD::ID_NEXT)
        { // NEXT
            SetCurrentLineNumber((int) (_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0]));
            currentProgramLineNotYetPartiallyProcessed = true;
        }

        else if (cmd == CMD::ID_ENDIF)
        { // NEXT
            firstIfElse._bIfElseEnable = false;
            firstIfElse._bIgnoreCondition = false;
            _eInterpreterStatus = MSTATUS_STEPDONE;
        }

        else if (cmd == CMD::ID_GOTO)
        { // GOTO
            //	Goto 문으로 if문을 탈출할 경우, 데이터를 초기화한다.
            if (firstIfElse._bIgnoreCondition == true)
                firstIfElse.clear();

            _eInterpreterStatus = MSTATUS_STEPDONE;
        }
        else if (cmd == CMD::ID_GOSUB)
        { // GOSUB
            _arrReturn.push_back(_currentProgramLine);

            _currentProgramLine = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]; // we jump
            currentProgramLineNotYetPartiallyProcessed = true;
        }
        else if (cmd == CMD::ID_RETURN)
        { // RETURN
            if ((int) _arrReturn.size() < 1)
                return(RTNCODE::ABNORMAL_RUNTIME_ERROR); // program end

            #ifndef QT_COMPIL
                TRACE("_arrReturn.GetSize() = %d\r\n", _arrReturn.size());
            #else
                qDebug()<<"_arrReturn.GetSize() ="<< _arrReturn.size()<< "\r\n";
            #endif

            int i = _arrReturn.back();
            _arrReturn.pop_back();
            _currentProgramLine = i + 1;
            currentProgramLineNotYetPartiallyProcessed = true;
        }

        else if (cmd == CMD::ID_PRESET)	// PRESET
        {
            std::string codeWord;
            int iGetValue;
            char chIOs[12];
           #ifndef WIN_VREP
            sprintf(chIOs, "IN%d", (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0]);
           #else
            sprintf_s(chIOs, "IN%d", (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0]);
           #endif
            std::string strInputIO(chIOs);

#ifndef WIN_VREP
 sprintf(chIOs, "IN%d", (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[0]);
#else
 sprintf_s(chIOs, "OUT%d", (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[2]);
#endif

            std::string strOutputIO(chIOs);

            int iInputData = (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[1];
            int iOutputData = (int)_compiledRobotLanguageProgram[_currentProgramLine]._floatParameter[3];

            // 시리얼 통신을 할 경우, OUT 데이터만 갱신하고 조건 판별은 하지 않는다.
            if (IsDataPassingByExternalDevice())
            {
                SetDataToOut(strOutputIO.c_str(), iOutputData);
                _eInterpreterStatus = MSTATUS_STEPDONE;
            }
            //	 정상 케이스 처리(시뮬레이션)
            else {
                if (strInputIO == "IN-1")	// PRESET OFF
                    _eInterpreterStatus = MSTATUS_STEPDONE;
                else if (GetDataFromIO(strInputIO.c_str(), iGetValue))
                {
                    if (iGetValue == iInputData)
                    {
                        SetDataToOut(strOutputIO.c_str(), iOutputData);
                        _eInterpreterStatus = MSTATUS_STEPDONE;
                    }
                }
            }
        }	// PRESET End

        else if (cmd == CMD::ID_SWITCH)
        { // SWITCH
            float fBranch = 0;
            int iWarningMsg = 0;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            GetValueAtOneWord(_compiledRobotLanguageProgram[_currentProgramLine]._strLabel[1].c_str(), fBranch, eAddr, iWarningMsg);
            std::map<int, int>::iterator iBranch;
            iBranch = _compiledRobotLanguageProgram[_currentProgramLine]._switchBranch->find((int)fBranch);

            if (_compiledRobotLanguageProgram[_currentProgramLine]._switchBranch->end() != iBranch)
                _currentProgramLine = (*iBranch).second;
            else
            {
                iBranch = _compiledRobotLanguageProgram[_currentProgramLine]._switchBranch->find((int)0xffff);

                if (_compiledRobotLanguageProgram[_currentProgramLine]._switchBranch->end() != iBranch)
                    _currentProgramLine = (*iBranch).second;
                else
                    _currentProgramLine = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1];
            }

            currentProgramLineNotYetPartiallyProcessed = true;
        }

        else if (cmd == CMD::ID_CASE || cmd == CMD::ID_BREAK || cmd == CMD::ID_DEFAULT)//	|| cmd == ID_NEXT) // NEXT 명령어는 FOR문과만 사용 - 2016.02.22
        { // BREAK
            _currentProgramLine = _compiledRobotLanguageProgram[_currentProgramLine]._intParameter[1] - 1;
            currentProgramLineNotYetPartiallyProcessed = true;
        }
    }

    _commModules->NotifyToAllSubscribers();

    //	통신 상태일 경우
    if (_eInterpreterStatus == MSTATUS_COMMUNCATION)
    {
        //	데이터 전송 Lock이 걸려 있으면 IO 데이터를 바꾸지 않는다.
        if (!GetLockSignalDataUpdateStatus())
        {
            _eInterpreterStatus = MSTATUS_RUN;
            SetSendingIoSignalToSerialPort(false);
        }
    }
    else if (_eInterpreterStatus == MSTATUS_STEPDONE)
    {
        _eInterpreterStatus = MSTATUS_STANDBY;
        _previousProgramLine = _currentProgramLine;
        _timeAlreadySpentAtCurrentProgramLine = 0.0f;
        SetSendingIoSignalToSerialPort(true);
        LockSignalDataUpdate(false);

        //	다음 라인으로 분기하는 명령어와, 점프하는 명령어를 구분한다.
        if (cmd == CMD::ID_RETURN && firstIfElse._bIgnoreCondition == true)
            SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]);
        else if (cmd == CMD::ID_GOTO && firstIfElse._bIgnoreCondition == false)
            SetCurrentLineNumber(_compiledRobotLanguageProgram[_currentProgramLine]._intParameter[0]);
        else
            SetCurrentLineNumber(_currentProgramLine + 1);

        currentProgramLineNotYetPartiallyProcessed = true;

        if (cmd == CMD::ID_STOP || cmd == CMD::ID_END)
            rtnCode = RTNCODE::PROGRAM_TERMINATED;
    }
    //	IO 상태 업데이트 루틴(RS-232C 포함)
    else if (IsDataPassingByExternalDevice() && GetLockSignalDataUpdateStatus())
    {
        SetSendingIoSignalToSerialPort(true);

        if (cmd != CMD::ID_WAIT)
            _eInterpreterStatus = MSTATUS_COMMUNCATION;

        _commModules->NotifyIOportsToAllSubscribers(GetRefInputMememory(), GetRefOutputMememory(), GetSendingIoSignalType());

        //	OUT의 경우는 RS-232C 통신 결과를 체크할 필요 없으므로, 바로 Lock을 풀어준다.
        if (GetSendingIoSignalType() == 1)
            LockSignalDataUpdate(false);
    }

    if (int(_compiledRobotLanguageProgram.size()) < _currentProgramLine)
        rtnCode = RTNCODE::ABNORMAL_RUNTIME_ERROR; // program end

    loopCnt++;

    if (loopCnt > LOOP_LIMIT)
        rtnCode = RTNCODE::ABNORMAL_RUNTIME_ERROR; // We looped too often... maybe waiting for an input signal or simply infinite loop! we leave here

    if (rtnCode != RTNCODE::PROGRAM_NORMALLY_PROGRESSING)
    {
        firstIfElse.clear();
        currentProgramLineNotYetPartiallyProcessed = false;
        _eInterpreterStatus = MSTATUS_ERROR;
    }
    return rtnCode;	//	normal return that is being executed now
}

