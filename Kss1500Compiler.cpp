#include "CommonH.h"
#include "Kss1500Compiler.h"
#include <boost/lexical_cast.hpp>
using namespace std;

typedef CCompiledProgramLine::PGM_CMD CMD;
typedef CRobotLangParserAndOperator::VAR_TYPE VAR;

typedef std::pair<std::string, float> pairData;
typedef std::pair<int, CRobotMessages::ECompileError> errorData;
typedef std::pair<int, CRobotMessages::EWarningID> warningData;
typedef CRobotMessages::EWarningID WRN;

CKss1500Interprerter::CKss1500Interprerter(void)
    : CKss1500RobotMotion(), CRobotLangParserAndOperator(), CRobotMessages()
{

}

CKss1500Interprerter::~CKss1500Interprerter(void)
{
}

void CKss1500Interprerter::SetProgram(const std::string& prg)
{
    _robotLanguageProgram=prg;
}

void CKss1500Interprerter::SetSubProgram(const std::string& prg)
{
    _robotSubProgram = prg;
}

void CKss1500Interprerter::SetProgram(const char* prg)
{
    _robotLanguageProgram.clear();
    _robotLanguageProgram.assign(prg);
}

const std::string& CKss1500Interprerter::GetProgram()
{
    return(_robotLanguageProgram);
}

const std::string& CKss1500Interprerter::GetSubProgram()
{
    return(_robotSubProgram);
}

void CKss1500Interprerter::AddOneLetterToProgram(const char& oneLetter)
{
    _robotSubProgram.push_back(oneLetter);
}

char CKss1500Interprerter::GetAtLetterFromProgram(int i)
{
    if((unsigned int) i <_robotSubProgram.size())
        return _robotSubProgram[i];
    else
        return '\r';
}

bool CKss1500Interprerter::UpdateIOport(int input, int output, int ChangedOutput)
{
    CRobotLangParserAndOperator::UpdateIOport(input, output, ChangedOutput);

    LockSignalDataUpdate(false);
    return true;
}

bool CKss1500Interprerter::UpdateInPort(int input)
{
    CRobotLangParserAndOperator::UpdateInPort(input);

    LockSignalDataUpdate(false);
    return true;
}

void CKss1500Interprerter::CompileCode(int& nTotalError, int& iFirstErrorLine)
{
    nTotalError = 0;

    _internalVariable.clear();
    _arrReturn.clear();
    _internalArray.clear();
    _internalPoseArray.clear();
    _internalFunction.clear();
    _labels.clear();
    _labelLocations.clear();

    CSwitch firstLvSwitch;
    CIfElseThen firstIfElse;

    _eInterpreterStatus = MSTATUS_STANDBY;

    //Clear compiled line.
    while(_compiledRobotLanguageProgram.size()>0)
    {
        _compiledRobotLanguageProgram.back().delete_switchBranch();
        _compiledRobotLanguageProgram.pop_back();
    }

    //	Temporary copyed string from original program
    std::string tempCopyedProgram(_robotLanguageProgram);
    std::string oneProgramLine;

    int currentCodeLineNb = 0;
    int iWarningMsg = 0;
    bool bCommentLine = false;	// 첫줄에 코멘트 라인 초기화

    //	Check the number of IF/ENDIF
    std::list<int> nofIFstatement;
    std::list<int> nofENDIFstatement;

    // the number of FOR/NEXT
    std::vector<int> forLocations;

    int errline = currentCodeLineNb;

    std::string originalLine;

    while (true)
    {
        //  get one code line
        ExtractOneLine(tempCopyedProgram,oneProgramLine);	//	Whole Program, One Line
        currentCodeLineNb++;
        bCommentLine = RemoveFrontAndBackSpaces(oneProgramLine, bCommentLine);

#ifndef QT_COMPIL
    TRACE ("compiling: %d \r\n", currentCodeLineNb);
#else
    qDebug ()<<"compiling : "<< currentCodeLineNb<<"\r\n";
#endif
        errline = currentCodeLineNb;

        // space만 있는 문장처리
        if( oneProgramLine.length() == 0)
        {
            int cmd = 0;
            GetCommandFromWord(oneProgramLine,cmd);

            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
            _compiledRobotLanguageProgram.push_back(a);
        }
        //	정상 문장 처리
        else
        {
            originalLine = oneProgramLine;
            std::string codeWord;

            //	명령어 파싱
            if (ExtractOneWord(oneProgramLine,codeWord))
            { // get the first word in the code line

                int cmd = CMD::ID_LABEL_DEFAULT;

                // get the equivalent command index for the word
                GetCommandFromWord(codeWord,cmd);

                //	Grammar check
                std::list<CRobotMessages::EWarningID> iWarningCode;
                std::list<CRobotMessages::ECompileError> currentErrorCode;

                if (!OnelineSyntaxCheckinCompileTime(oneProgramLine, currentCodeLineNb, (CMD)cmd, currentErrorCode, iWarningCode))
                {
#ifndef QT_COMPIL
                    for each(auto comm in currentErrorCode)
#else
                    for(auto comm : currentErrorCode)
#endif
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, comm));
                        continue;
                    }
#ifndef QT_COMPIL
                    for each(auto comm in iWarningCode)
#else
                    for(auto comm : iWarningCode)
#endif
                        _compileWarningCode.insert(warningData(currentCodeLineNb, comm));
                }

                if (cmd == CMD::ID_LABEL_DEFAULT)
                {
                    _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_LABEL_DEFAULT));
                    CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                    _compiledRobotLanguageProgram.push_back(a);
                    continue;
                }

                // label
                if (cmd == CMD::ID_LABEL)
                { // we have a label here
                    RemoveFrontAndBackSpaces(oneProgramLine);
                    if (oneProgramLine.length()==0)
                    { // the line is ok.
                        // 2중 label catch
                        bool bDoubleLabelFound = false;
                        for( int i =0; i< (int) _labels.size(); i++)
                        {
                            if( _labels.at(i) == codeWord )
                            {
                                _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_DOUBLE_LABLE));
                                bDoubleLabelFound = true;
                            }
                        }
                        if (bDoubleLabelFound)
                        {
                            _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));
                            continue;
                        }

                        _labels.push_back(codeWord);
                        _labelLocations.push_back(int(_compiledRobotLanguageProgram.size()));

                    #ifndef QT_COMPIL
                        TRACE("tmp = %d \r\n", int(_compiledRobotLanguageProgram.size()));
                    #else
                        qDebug ()<<"Program size(lineNo) : "<< int(_compiledRobotLanguageProgram.size())<<"\r\n";
                    #endif

                        CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                    }
                    else
                    { 	// we have a variable here // 변수 값 할당 ex) AA = 10
                        bool error = true;
                        std::string variable, rValue;
                        variable = codeWord;

                        if (oneProgramLine.at(0) == '=')
                        {
                            oneProgramLine.erase(0, 1);
                            RemoveFrontAndBackSpaces(oneProgramLine);
                        }
                        rValue = oneProgramLine;

                        // 수치 연산
                        int iRtnValue = 0;
                        float fTmpValue[7] = {0.0f,};

                        //	User Pose Data 일 경우, P = P1+P2
                        if (IsInternalUserdefPose(codeWord.c_str()))
                        {

                            CCompiledProgramLine a(CMD::ID_VALUE, originalLine, currentCodeLineNb);
                            iRtnValue =ExactractPoseData(codeWord, oneProgramLine, a);

                            if(iRtnValue < 0)
                            {
                                error = true;
                                _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_POSE_LABEL));
                            } else if (iRtnValue == 0)
                                ExtractXyzrgPosition(oneProgramLine, a._floatParameter);

                            error = false;
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        //	DEFPOS X = 100,23,3
                        else if (IsInternalPose(codeWord.c_str()))
                        {
                            CCompiledProgramLine a;
                            a._iLineNumber = currentCodeLineNb;
                            a._originalUncompiledCode = originalLine;
                            iRtnValue = ExactractPoseData(codeWord, oneProgramLine, a);
                            a._command = CMD::ID_VALUE;

                            if (iRtnValue < 0)
                            {
                                error = true;
                                _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_POSE_LABEL));
                                a._command = CMD::ID_BLANK;
                                _compiledRobotLanguageProgram.push_back(a);
                                continue;
                            }
                            else if (iRtnValue == 0)
                                ExtractXyzrgPosition(oneProgramLine, a._floatParameter);

                            error = false;
                            _compiledRobotLanguageProgram.push_back(a);

                            // 정재웅 요청 수정
                            if (!SetDataToPose(codeWord.c_str(), a._floatParameter, iWarningMsg))
                            {
                                error = true;
                                a._command = CMD::ID_BLANK;
                                _compiledRobotLanguageProgram.push_back(a);
                                continue;
                            }
                        }

                        //	A = A + 2
                        else if((iRtnValue = DoMathOperation(oneProgramLine, iWarningMsg))< 0)
                        {
                            iRtnValue = -1;
                            error = true;
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_MATHOPR));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                            continue;
                        }
                        //	A = B || C
                        else if((iRtnValue == 0) && (iRtnValue = DoLogicalOperation(oneProgramLine, iWarningMsg)< 0))
                        {
                            iRtnValue = -1;
                            error = true;
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_LOGICOPR));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        //	A = B < C
                        else if ((iRtnValue == 0) && ((iRtnValue = DoRelationalOperation(oneProgramLine, iWarningMsg))< 0))
                        {
                            iRtnValue = -1;
                            error = true;
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_RELAOPR));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        else
                        {
                            if (ExtractOneWord(oneProgramLine,codeWord))
                            {
                                float fValue = 0.0f;

                                if (GetFloatFromWord(codeWord, fValue) )
                                {
                                        error = false;
                                        CCompiledProgramLine a;
                                        a._command = CMD::ID_VALUE;	//	ID_VALUE
                                        a._strLabel[0] = variable;

                                        if (iRtnValue == 1) a._strLabel[1] = rValue;
                                        else a._strLabel[1].clear();

                                        a._strLabel[2] = rValue;
                                        a._intParameter[0] = iRtnValue;
                                        a._floatParameter[0] = fValue;
                                        a._originalUncompiledCode = originalLine;
                                        a._iLineNumber = currentCodeLineNb;
                                        _compiledRobotLanguageProgram.push_back(a);

                                        float fOneValue = 0.0f;
                                        int iTmpValue = 0;
                                        VAR_TYPE iVarType = VAR_TYPE::NONE;
                                        int iWarnMessage = -1;
                                        std::vector<int> DimSizes;
                                        std::string strLvalue, strRvalue;
                                        int height = 0, width = 0, depth = 0;

                                        // 배열에 값 넣기
                                        if (IsInternalArray(variable)) {
                                            if (ExtractDimDefine(variable, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
                                            {
                                                height = (DimSizes[0] == -1) ? 0 : DimSizes[0];
                                                width = (DimSizes[1] == -1) ? 0 : DimSizes[1];
                                                depth = (DimSizes[2] == -1) ? 0 : DimSizes[2];
                                                if (SetDataToArray(strLvalue, height, width, depth, fValue, iWarnMessage))
                                                    continue;	// success case
                                            }
                                        }
                                        else if (GetDataFromMap(variable, fOneValue))
                                            _internalVariable[variable.c_str()].SetValue(fValue);
                                        else if (GetDataFromIO(variable.c_str(), iTmpValue))
                                                SetDataToOut(variable.c_str(), (int) fValue);
                                        else
                                        {
                                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_LABEL));
                                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                                            error = true;
                                        }
                                }
                                else if(IsInternalArray(codeWord) || IsInternalVariable(codeWord) || IsInternalPorts(codeWord.c_str()) )
                                {
                                    error = false;
                                    CCompiledProgramLine a;
                                    a._command = CMD::ID_VALUE;
                                    a._originalUncompiledCode = originalLine;
                                    a._strLabel[0] = variable;
                                    a._strLabel[1] = codeWord;
                                    a._iLineNumber = currentCodeLineNb;
                                    _compiledRobotLanguageProgram.push_back(a);
                                }

                            }
                        }
                        if (error)
                        {
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_LABEL));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                            continue;
                        }

                    }
                }

                //	CMD 값 이외에 별다른 데이터가 필요하지 않음.
                else if (cmd == CMD::ID_GOHOME) // GOHOME
                {
                    CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                    _compiledRobotLanguageProgram.push_back(a);
                }

                else if (cmd == CMD::ID_SPEED) // SPEED
                {
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        float fSpeed = 0.0f;
                        ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                        CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);

                        //	SPEED 80의 80의 값을 취득
                        if (GetFloatFromWord(codeWord, fSpeed))
                        {
                            if (fSpeed>0)
                            {
                                if (!ExtractOneWord(oneProgramLine,codeWord))
                                {
                                    error = false;
                                    a._floatParameter[0]= fSpeed;
                                    a._strLabel[0].clear();
                                    _compiledRobotLanguageProgram.push_back(a);
                                    _fEuclidFeed = fSpeed;
                                }
                            }
                        }
                        //	SPEED A의 변수 A값을 취득
                        else if(GetValueAtOneWord(codeWord.c_str(), fSpeed, eAddr, iWarningMsg))
                        {
                            error = false;
                            a._floatParameter[0]= fSpeed;
                            a._strLabel[2] =codeWord;
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        else
                        {
                            // SPEED A*30와 같이 수학 연산 요구되는 부분
                            int iRtnValue = 0;
                            a._strLabel[0].clear();
                            a._strLabel[2] = codeWord;
                            iRtnValue = DoMathOperation(codeWord, iWarningMsg);

                            if(iRtnValue)
                            {
                                error = false;
                                a._intParameter[0] = 1;
                                a._floatParameter[0]= fSpeed;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_SPEED));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_ROTATE) // ROTATE
                {
                    bool error = true;
                    ADDRESS eAddr = ADDRESS::GENERAL_VAR;
                    float fRotateValue = 0;
                    CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);

                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        //	SPEED 80의 80의 값을 취득
                        if (GetFloatFromWord(codeWord, fRotateValue))
                        {
                            if (fRotateValue>=0)
                            {
                                if (!ExtractOneWord(oneProgramLine, codeWord))
                                {
                                    error = false;
                                    a._floatParameter[0] = fRotateValue;
                                    a._strLabel[0].clear();
                                    _compiledRobotLanguageProgram.push_back(a);
                                }
                            }
                        }
                        else if( GetValueAtOneWord(codeWord, fRotateValue, eAddr, iWarningMsg) )
                        {
                            error = false;
                            a._floatParameter[0]= fRotateValue;
                            a._strLabel[0].clear();

                            if (eAddr != ADDRESS::FLOAT_VALUE)
                                a._strLabel[2] = codeWord;

                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        else
                        {
                            // SPEED A*30와 같이 수학 연산이 요구되는 부분
                            int iRtnValue = 0;
                            a._strLabel[0].clear();
                            a._strLabel[2] = codeWord;
                            iRtnValue = DoMathOperation(codeWord, iWarningMsg);
                            if (iRtnValue)
                            {
                                error = false;
                                a._intParameter[0] = 1;
                                a._floatParameter[0] = fRotateValue;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ROTATE));
                        CCompiledProgramLine b(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(b);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_GRASP) // GRASP
                {
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        ADDRESS eAddr;
                        float fValue = 0.0f;
                        if (GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg)) {
                            if (eAddr == ADDRESS::PORT_IO)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));

                            if (fValue >= 0)
                            {
                                error = false;
                                CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                                a._floatParameter[0] = fValue;
                                a._strLabel[0] = codeWord;
                                a._iLineNumber = currentCodeLineNb;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                        else
                        {
                            // SPEED A*30와 같이 수학 연산 요구되는 부분
                            int iRtnValue = 0;
                            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                            a._strLabel[0].clear();
                            a._strLabel[2] = codeWord;
                            iRtnValue = DoMathOperation(codeWord, iWarningMsg);

                            if (iRtnValue)
                            {
                                error = false;
                                a._intParameter[0] = 1;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_GRASP));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_RELEASE) // RELEASE
                {
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine, codeWord))
                    {
                        ADDRESS eAddr;
                        float fValue = 0.0f;
                        if (GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg)) {
                            if (eAddr == ADDRESS::PORT_IO)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));

                            if (fValue >= 0)
                            {
                                error = false;
                                CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                                a._floatParameter[0] = fValue;
                                a._strLabel[0] = codeWord;
                                a._iLineNumber = currentCodeLineNb;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                        else
                        {
                            // REASE A*30와 같이 수학 연산 요구되는 부분
                            int iRtnValue = 0;
                            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                            a._strLabel[0].clear();
                            a._strLabel[2] = codeWord;
                            iRtnValue = DoMathOperation(codeWord, iWarningMsg);

                            if (iRtnValue)
                            {
                                error = false;
                                a._intParameter[0] = 1;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_RELEASE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_CHANGE) // CHANGE - 변수처리 ok
                {
                    bool error = true;
                    float f;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        int i;

                        if (GetIntegerFromWord(codeWord,i))
                        {
                            if (i>0)
                            {
                                if (!ExtractOneWord(oneProgramLine,codeWord))
                                {
                                    error = false;
                                    CCompiledProgramLine a;
                                    a._command = cmd;
                                    a._originalUncompiledCode = originalLine;
                                    a._intParameter[0]=i;
                                    a._strLabel[0].clear();
                                    a._iLineNumber = currentCodeLineNb;
                                    _compiledRobotLanguageProgram.push_back(a);
                                }
                            }
                        }
                        else if( GetDataFromMap(codeWord, f) )
                        {
                            error = false;
                            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                            a._floatParameter[0]=0;
                            a._strLabel[0] =codeWord;
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_CHANGE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_DRIVE) // DRIVE - 변수처리 ok
                {
                    bool error = true;
                    float f1 = 0.0f, f2 = 0.0f;
                    ADDRESS eAddr = ADDRESS::FLOAT_VALUE;
                    CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);

                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        int iTmpValue = 0;
                        a._strLabel[0] = codeWord;

                        if (GetValueAtOneWord(codeWord, f1, eAddr, iWarningMsg))
                            iTmpValue = (int)f1;

                        // i는 1~3
                        if (ExtractOneWord(oneProgramLine,codeWord))
                        {
                            if (GetValueAtOneWord(codeWord, f2, eAddr, iWarningMsg))
                            {
                                error = false;
                                a._intParameter[0]= iTmpValue;
                                a._floatParameter[0]=f2;
                                a._strLabel[1] = codeWord;
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                            else
                            {
                                int iRtnValue = 0;
                                a._strLabel[1] = codeWord;
                                iRtnValue = DoMathOperation(codeWord, iWarningMsg);

                                if (iRtnValue)
                                {
                                    error = false;
                                    a._intParameter[0] = 1;
                                    _compiledRobotLanguageProgram.push_back(a);
                                }
                            }
                        }

                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DRIVE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_READY) // READY
                {
                    bool error = true;

                    if ( !ExtractOneWord(oneProgramLine,codeWord) )
                    {
                        error = false;
                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._floatParameter[0]=0;
                        a._floatParameter[1]=300;
                        a._floatParameter[2]=0;
                        a._strLabel[0].clear();
                        a._strLabel[1].clear();
                        a._strLabel[2].clear();
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_READY));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_MOVE ) // MOVE
                {
                    bool bErrorCheck = true;
                    int iSpeedValue = UNCONTROLLABLE_VALUE;
                    float fWaitValue = UNCONTROLLABLE_VALUE;
                    float fGripValue = UNCONTROLLABLE_VALUE;
                    CCompiledProgramLine scInsertData;
                    scInsertData.clear();
                    int iTempData = 0;
                    int iTempPoseNo = 0;
                    int iTempMoveNo = 0;
                    float fTempData = 0.0f;
                    std::string strTempData;

                    // 데이터 분해
                    std::list<CCompiledProgramLine> *CurrChainRobotMotion = NULL;
                    std::list<CCompiledProgramLine> RealAllocatedChainBlock;

                    //	Is it chained motion? and need math operation?
                    //	다수의 사용자 지정 데이터(ex P1)를 구분
                    if(IsChainMotionProgram(oneProgramLine, iTempMoveNo, iTempPoseNo) == true)
                    {
                        // MOVE L, P001＋(0, 0, 10), ?＋(20, 0, 0)  처리
                        scInsertData._command = CMD::ID_MOVE2;
                        scInsertData._strLabel[3] = "SAVE_CURRENTPOS";

                        if(iTempPoseNo > 0) {
                            while (oneProgramLine.size() > 0)
                            {
                                //	take (-93.85, 221.73, 68) at first
                                ExtractOneMoveData(oneProgramLine, strTempData);

                                if(strTempData.size()>0)
                                {
                                    if(ExactractPoseData("NULL", strTempData, scInsertData))
                                    {
                                        //	MOVE P1, P2, T=3 안됨.
                                        if (strTempData.compare("ERROR_T") == 0)
                                        {
                                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_POSE_NOT_T));
                                            continue;
                                        }
                                        //	MOVE P1, P2, H=3 안됨.
                                        else if (strTempData.compare("ERROR_H") == 0)
                                        {
                                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_POSE_NOT_H));
                                            continue;
                                        }

                                        if(scInsertData._command == CMD::ID_SPEED)
                                            scInsertData._originalUncompiledCode = "SPEED " + strTempData;
                                        else
                                            scInsertData._originalUncompiledCode = "MOVE " + strTempData;

                                        scInsertData._iLineNumber = currentCodeLineNb;
                                        _compiledRobotLanguageProgram.push_back(scInsertData);

                                        scInsertData.clear();
                                        scInsertData._command = CMD::ID_MOVE2;
                                    }
                                }
                            }

                            if (_compiledRobotLanguageProgram.size() != 0)
                                _compiledRobotLanguageProgram.back()._strLabel[3] = "CLEAR_CURRENTPOS";
                            else
                                bErrorCheck = true;
                        }
                        else
                        {
                            //	MOVE (-93.85, 221.73, 68), (-93.85, 321.01, 68), (-93.85, 221.73, 74)
                            if(_compiledRobotLanguageProgram.size()>0)
                                scInsertData = _compiledRobotLanguageProgram.back();

                            scInsertData._command = CMD::ID_MOVE2;
                            scInsertData._originalUncompiledCode = oneProgramLine;
                            scInsertData._strLabel[0] = oneProgramLine;
                            scInsertData._iLineNumber = currentCodeLineNb;
                            scInsertData.clear();

                            scInsertData.createChainedProgramLine();


                            //	MOVE (-93.85, 221.73, 68)
                            //	MOVE (-93.85, 321.01, 68)
                            //	MOVE (-93.85, 221.73, 74)
                            if(GenerateSerialMotion(oneProgramLine, scInsertData))
                            {
                                CurrChainRobotMotion = (scInsertData._compiledChainedMotionProgram);

                                if(CurrChainRobotMotion)
                                {
                                    oneProgramLine = (*(CurrChainRobotMotion)).front()._originalUncompiledCode;
                                    scInsertData._originalUncompiledCode = oneProgramLine;
                                    RealAllocatedChainBlock.swap(*CurrChainRobotMotion);
                                }
                                scInsertData = RealAllocatedChainBlock.front();
                                RealAllocatedChainBlock.pop_front();
                            }
                            scInsertData.deleteChainedProgramLine();
                            goto CHAINED_MOTION;
                        }
                        // (pose) was done. Chained motion is not..
                        continue;
                    }

                    CHAINED_MOTION : //(X,Y,Z),(X,Y,Z),(X,Y,Z)
                    do{
                        int iDatacount = 0, iWarning = 0;
                        scInsertData._originalUncompiledCode += "MOVE ";
                        scInsertData._originalUncompiledCode += oneProgramLine;
                        scInsertData._iLineNumber = currentCodeLineNb;

                        std::string strGrasp;
                        std::string strWait;
                        std::string strSpeed;

                        ADDRESS eAddr = ADDRESS::FLOAT_VALUE;

                        while (iDatacount < NUM_POSE_PARAM)	// 변수의 갯수 3~5개 가능
                        {
                            if (!ExtractOneAddress(oneProgramLine,codeWord))		// 변수 갯수 에러/완료
                                break;

                            RemoveFrontAndBackSpaces(codeWord);

                            //	S값 처리문 (서상현 구현부 사용)
                            if (codeWord.at(0) == 'S')		// S값 입력 시
                            {
                                if (codeWord.length() > 1)	// 'SXX'로 입력한 경우. 처리해줌(변수와 겹칠 위험이 있어 에러 처리)
                                {
                                    codeWord.erase(0,1);	// 'S'삭제
                                    float fSpeed = 0.0f;
                                    if (!GetValueAtOneWord(codeWord, fSpeed, eAddr, iWarningMsg))
                                        iSpeedValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                    iSpeedValue = (int) fSpeed;
                                    strSpeed = codeWord;
                                }
                                else						// 'S=XX'로 입력한 경우
                                {
                                    if (ExtractOneWord(oneProgramLine,codeWord))
                                    {
                                        float fSpeed = 0.0f;
                                        if (!GetValueAtOneWord(codeWord, fSpeed, eAddr, iWarningMsg))
                                            iSpeedValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                        iSpeedValue = (int) fSpeed;
                                        strSpeed = codeWord;
                                    }
                                }
                                if (!ValidateSpeed((float)iSpeedValue)) // 범위 검토
                                    iSpeedValue = UNCONTROLLABLE_VALUE;	// 값 범위 초과

                                ++iDatacount;
                                //break;
                            }

                            //	T값 처리문 (서상현 구현부 사용)
                            else if (codeWord.at(0) == 'T' && iDatacount >=3 )		// H값 입력 시, 인자 3개 이상 사용 후
                            {
                                if (codeWord.length() > 1)	// 'TXX'로 입력한 경우. 처리해줌(변수와 겹칠 위험이 있어 에러 처리)
                                {
                                    codeWord.erase(0, 1);	// 'T'삭제

                                    float fValue = 0.0f;
                                    if (!GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg))
                                        fWaitValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                    strWait = codeWord;
                                    fWaitValue = fValue;
                                }
                                else						// 'T=XX'로 입력한 경우
                                {
                                    if (ExtractOneWord(oneProgramLine, codeWord))
                                    {
                                        float fValue = 0.0f;
                                        if (!GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg))
                                            fWaitValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                        strWait = codeWord;
                                        fWaitValue = fValue;
                                    }
                                }
                                if (fWaitValue < 0) // 범위 검토
                                    fWaitValue = UNCONTROLLABLE_VALUE;	// 값 범위 초과

                                codeWord.clear();
                            }

                            //	T값 처리문 (서상현 구현부 사용)
                            else if (codeWord.at(0) == 'G' && iDatacount >= 3)		// H값 입력 시, 인자 3개 이상 사용 후
                            {
                                if (codeWord.length() > 1)	// 'GXX'로 입력한 경우. 처리해줌(변수와 겹칠 위험이 있어 에러 처리)
                                {
                                    codeWord.erase(0, 1);	// 'G'삭제

                                    float fValue = 0.0f;
                                    if (!GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg))
                                        fGripValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                    strGrasp = codeWord;
                                    fGripValue = fValue;
                                }
                                else						// 'G=XX'로 입력한 경우
                                {
                                    if (ExtractOneWord(oneProgramLine, codeWord))
                                    {
                                        float fValue = 0.0f;
                                        if (!GetValueAtOneWord(codeWord, fValue, eAddr, iWarningMsg))
                                            fGripValue = UNCONTROLLABLE_VALUE;	// 값을 획득 못하면 에러

                                        strGrasp = codeWord;
                                        fGripValue = fValue;
                                    }
                                }
                                if (!ValidateGrip((float)fGripValue)) // 범위 검토
                                    fGripValue = UNCONTROLLABLE_VALUE;	// 값 범위 초과

                                codeWord.clear();
                            }
                            /*
                            ADDRESS varType;
                            float fValues[TOTAXES] = { 0.0f, };


                            if(GetValueAtOneWord(codeWord, fTempData, varType, iWarningMsg))
                                scInsertData._floatParameter[iDatacount] = fTempData;
                            else if (GetPoseAtOneWord(codeWord, fValues, iWarning))
                            {
                                if (iDatacount > 0)		// P값은 MOVE 명령어의 첫 변수로만 와야 함
                                {
                                    iSpeedValue = -1;
                                    continue;	// Error
                                }
                                scInsertData._strLabel[iDatacount++] = codeWord;
                                scInsertData._strLabel[4] = "POSE";
                            }
                            */
                            if(codeWord.empty() || fGripValue > 0 || iSpeedValue > 0)
                            {
                                continue;
                            }
                            else if (GetFloatFromWord(codeWord, fTempData))
                            {
                                scInsertData._floatParameter[iDatacount] = fTempData;
                                scInsertData._strLabel[iDatacount].clear();
                            }
                            else if (GetDataFromMap(codeWord, fTempData))
                            {
                                scInsertData._strLabel[iDatacount] = codeWord;
                            }
                            else if (GetDataFromArray(codeWord, fTempData, iWarning))
                            {
                                scInsertData._strLabel[iDatacount] = codeWord;
                            }

                            else if (GetDataFromPose(codeWord, scInsertData._floatParameter, iWarning))
                            {
                                if (iDatacount > 0)		// P값은 MOVE 명령어의 첫 변수로만 와야 함
                                {
                                    iSpeedValue = UNCONTROLLABLE_VALUE;
                                    continue;	// Error
                                }
                                scInsertData._strLabel[iDatacount++].clear();
                                scInsertData._strLabel[iDatacount++] = codeWord;
                                scInsertData._strLabel[iDatacount++].clear();
                                scInsertData._strLabel[3].clear();
                                scInsertData._strLabel[4] = "POSE";
                            }
                            else if (GetDataFromUserPose(codeWord, scInsertData._floatParameter))
                            {
                                if (iDatacount > 0)		// P값은 MOVE 명령어의 첫 변수로만 와야 함
                                {
                                    iSpeedValue = UNCONTROLLABLE_VALUE;
                                    continue;	// Error
                                }
                                scInsertData._strLabel[iDatacount++].clear();
                                scInsertData._strLabel[iDatacount++] = codeWord;
                                scInsertData._strLabel[iDatacount++].clear();
                                scInsertData._strLabel[3].clear();
                                scInsertData._strLabel[4] = "POSE";
                            }
                            else if (GetDataFromArray(codeWord, fTempData, iWarning))
                            {
                                scInsertData._strLabel[iDatacount] = codeWord;
                            }

                            else if (codeWord.find_first_of("@") != std::string::npos)
                            {
                                scInsertData._strLabel[0].clear();
                                scInsertData._strLabel[1].clear();
                                scInsertData._strLabel[2].clear();
                                scInsertData._strLabel[3].clear();
                                _compiledRobotLanguageProgram.push_back(scInsertData);
                                goto SKIP_MOTIONVALID_CHECK; // @ 현재좌표값이 없는 가정하에 계산 보류. 런타임시 체크하도록 한다.
                            }
                            else
                            {
                                iSpeedValue = UNCONTROLLABLE_VALUE;
                                _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_INVALD_MOVEPARAM));
                                break;	// Error
                            }
                            ++iDatacount;
                        }


                        // 컴파일 데이터 저장
                        if (iSpeedValue > 0)		// MOVE 명령어에 속도 포함
                        {
                            CCompiledProgramLine stSpeedLineAdd;
                            float fSpeedValue = (float)iSpeedValue;
                            std::stringstream ssConvert;

                            ssConvert << iSpeedValue;
                            strTempData = "SPEED " + ssConvert.str();
                            stSpeedLineAdd._strLabel[0] = strSpeed;

                            if (GetValueAtOneWord(codeWord, fSpeedValue, eAddr, iWarning))
                            {
                                if (eAddr != ADDRESS::GENERAL_VAR)
                                    stSpeedLineAdd._strLabel[0] = codeWord;
                            }

                            stSpeedLineAdd._originalUncompiledCode = strTempData;
                            stSpeedLineAdd._command = CMD::ID_SPEED;	// SPEED 명령어 번호
                            stSpeedLineAdd._floatParameter[0] = fSpeedValue;
                            stSpeedLineAdd._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(stSpeedLineAdd);
                        }

                        if (iDatacount == 5 || (iDatacount == 4 && iSpeedValue == UNCONTROLLABLE_VALUE))	// MOVE2 명령어 사용 시
                        {
                            bErrorCheck = false;

                            scInsertData._command = CMD::ID_MOVE2;	// MOVE2
                            scInsertData._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(scInsertData);
                        }
                        else if (iDatacount == 3 || (iDatacount == 4 && iSpeedValue > 0))	// 기존 MOVE 명령어 사용 시
                        {
                            bErrorCheck = false;

                            scInsertData._command = CMD::ID_MOVE;
                            scInsertData._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(scInsertData);
                        }
                        else	// 에러
                            bErrorCheck = true;

                        // 컴파일 데이터 저장
                        if (fGripValue > (int)UNCONTROLLABLE_VALUE)		// MOVE 명령어에 속도 포함
                        {
                            CCompiledProgramLine grapLineAdd;
                            std::stringstream ssConvert;

                            ssConvert << fGripValue;
                            strTempData = "GRASP " + ssConvert.str();
                            grapLineAdd._strLabel[0] = strGrasp;
                            grapLineAdd._originalUncompiledCode = strTempData;
                            grapLineAdd._command = CMD::ID_GRASP;	// SPEED 명령어 번호
                            grapLineAdd._floatParameter[0] = (float)fGripValue;
                            grapLineAdd._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(grapLineAdd);
                        }

                        if (iSpeedValue > UNCONTROLLABLE_VALUE)	// MOVE 명령어에 속도 포함된 경우 사용 후 원속 복귀
                        {
                            CCompiledProgramLine stSpeedLineAdd;
                            std::stringstream ssTemp;

                            ssTemp << _fEuclidFeed;
                            strTempData = "SPEED " + ssTemp.str();

                            stSpeedLineAdd._originalUncompiledCode = strTempData;
                            stSpeedLineAdd._command = CMD::ID_SPEED;				// SPEED 명령어 번호
                            stSpeedLineAdd._floatParameter[0] = _fEuclidFeed;
                            stSpeedLineAdd._strLabel[0] = strSpeed;
                            stSpeedLineAdd._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(stSpeedLineAdd);
                        }

                        if (fWaitValue > UNCONTROLLABLE_VALUE)	// MOVE 명령어에 속도 포함된 경우 사용 후 원속 복귀
                        {
                            CCompiledProgramLine waitLineAdd;
                            std::stringstream ssTemp;

                            ssTemp << fWaitValue;
                            strTempData = "WAIT " + ssTemp.str();

                            waitLineAdd._strLabel[0] = strWait;
                            waitLineAdd._originalUncompiledCode = strTempData;
                            waitLineAdd._command = CMD::ID_WAIT;				// SPEED 명령어 번호
                            waitLineAdd._floatParameter[0] = fWaitValue;
                            waitLineAdd._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(waitLineAdd);
                        }

                        /*
                        // 범위(에러) 체크
                        if (iSpeedValue == -1)
                            bErrorCheck = true;
                        if( !ValidateMove(scInsertData._floatParameter[0]/1000, scInsertData._floatParameter[1]/1000, scInsertData._floatParameter[2]/1000) )
                            bErrorCheck = true;
                        if (!ValidateRotate(scInsertData._floatParameter[3]) && scInsertData._command == CMD::ID_MOVE2)
                            bErrorCheck = true;
                        */

                        if (bErrorCheck)	// 에러 시 컴파일 종료
                        {
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_INVALID_KINEMATIC_RNG));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                            continue;
                        }

                        SKIP_MOTIONVALID_CHECK:

                        if(RealAllocatedChainBlock.size()>0)
                        {
                            oneProgramLine = RealAllocatedChainBlock.front()._originalUncompiledCode;
                            cmd = RealAllocatedChainBlock.front()._command;
                            iSpeedValue = 0;
                            scInsertData.clear();
                            iTempData = 0;
                            fTempData = 0.0f;
                            strTempData.clear();
                            RealAllocatedChainBlock.pop_front();
                        }

                    } while (RealAllocatedChainBlock.size()>0 && bErrorCheck == false);

                    if (bErrorCheck == true)	//	chain motion 에 대한 대응
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_POSE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }	// MOVE End

                else if (cmd == CMD::ID_VAR) // VAR - 변수 선언, ex) VAR AA
                {
                    bool error = true;
                    std::string strLvalue, strRvalue;

                    //	컴마의 갯수 만큼이 정의되는 변수의 개수가 됨.한개도 없다면 한개의 변수는 정의 된 것으로 본다.
                    unsigned int uNumOfComma = GetCountComma(oneProgramLine)+1;

                    for(int i=0 ; i < (int) uNumOfComma; i++ )
                    {
                        int  iWarnMessage = -1;
                        VAR_TYPE iVarType = VAR_TYPE::NONE;

                        if (ExtractVarDefine(oneProgramLine, iVarType, iWarnMessage, strLvalue, strRvalue))
                        {
                            error = false;
                            VARIABLE varData;

                            auto result = _internalVariable.find(strLvalue.c_str());
                            if (result != _internalVariable.end()) {
                                error = true;
                            }
                            else
                            {
                                int iTmp = 0;
                                float fTemp = 0.0f;

                                switch ((VAR) iVarType)
                                {
                                case VAR::VAR_UINT:	// unsigned int
                                    if (strRvalue.empty())
                                        // notice an warning message
                                        _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));

                                    //iTmp = atoi(strRvalue.c_str());
                                    GetIntegerFromWord(strRvalue, iTmp);
									if (varData.SetValue((float)((unsigned int)iTmp)) != 0)
										_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_OVERFLOW_INT));

                                    varData.SetType(VAR::VAR_UINT);
                                    _internalVariable[strLvalue.c_str()] = varData;
                                    break;

                                case VAR::VAR_INT:	//	int
                                    if (strRvalue.empty())
                                        // notice an warning message
                                        _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));

                                    iTmp = atoi(strRvalue.c_str());
									if (varData.SetValue((float) iTmp) != 0)
										_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_OVERFLOW_INT));

                                    varData.SetType(VAR::VAR_INT);
                                    _internalVariable[strLvalue.c_str()] = varData;
                                    break;

                                default:
                                case VAR::VAR_FLOAT:	//	float
                                    if (strRvalue.empty())
                                        // notice an warning message
                                        _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_NO_VAR_INIT));

                                    fTemp = atof(strRvalue.c_str());
									if(varData.SetValue((float)fTemp) != 0)
										_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_OVERFLOW_FLOAT));

                                    varData.SetType(VAR::VAR_FLOAT);
                                    _internalVariable[strLvalue.c_str()] = varData;
                                    break;
                                }
                            }

                            if (i == 0) {
                                //	 make a compiled robot command (Defining VAR)
                                CCompiledProgramLine a;
                                a._command = cmd;
                                a._originalUncompiledCode = originalLine;
                                a._iLineNumber = currentCodeLineNb;
                                a._floatParameter[0] = varData.GetValue();
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                        else
                            error = true;
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_VAR));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_FN)
                {
                    int iWarnMessage = 0;
                    bool error = false;
                    string strLvalue, strRvalue, strFnName;

                    CCompiledProgramLine scInsertData;
                    scInsertData._originalUncompiledCode = originalLine;
                    scInsertData._iLineNumber = currentCodeLineNb;
                    scInsertData._command = CMD::ID_FN;
                    VAR_TYPE iVarType = VAR_TYPE::NONE;

                    if (ExtractVarDefine(oneProgramLine, iVarType, iWarnMessage, strLvalue, strRvalue))
                    {
                        error = false;
                        auto result = _internalFunction.find(strLvalue.c_str());
                        if (result != _internalFunction.end()) {
                            error = true;
                        }
                        else
                        {
                            CFunctionDefinition fnData;
                            VARIABLE rtnVariableData;
                            rtnVariableData.SetValue((float)UNCONTROLLABLE_VALUE);

                            switch ((VAR)iVarType)
                            {
                            case VAR::VAR_UINT:	// unsigned int
                                rtnVariableData.SetType(VAR::VAR_UINT);
                                fnData._returnValue = rtnVariableData;
                                break;

                            case VAR::VAR_INT:	//	int
                                rtnVariableData.SetType(VAR::VAR_INT);
                                fnData._returnValue = rtnVariableData;
                                break;

                            default:
                            case VAR::VAR_FLOAT:	//	float
                                rtnVariableData.SetType(VAR::VAR_FLOAT);
                                fnData._returnValue = rtnVariableData;
                                break;
                            }

                            strFnName = strLvalue;
                            std::vector<string> extracedWords;
                            ExtractMultiWord(oneProgramLine, extracedWords);
#ifndef QT_COMPIL
                    for each(auto comm in extracedWords)
#else
                    for(auto comm : extracedWords)
#endif
                            {
                                ExtractVarDefine(comm, iVarType, iWarnMessage, strLvalue, strRvalue);
                                fnData._argumentVariable[strLvalue.c_str()]._VarType = iVarType;
                                fnData._argumentVariable[strLvalue.c_str()]._Value = UNCONTROLLABLE_VALUE;
                            }
                            _internalFunction[strFnName] = fnData;
                        }

                    }
                    else
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_FN));
                        error = true;
                    }
                }

                else if (cmd == CMD::ID_DEF)
                {
                    bool error = false;
                    string strLvalue, strRvalue;

                    CCompiledProgramLine scInsertData;
                    scInsertData._originalUncompiledCode = originalLine;
                    scInsertData._iLineNumber = currentCodeLineNb;
                    scInsertData._command = CMD::ID_DEF;

                    if (ExtractOneAddress(oneProgramLine, codeWord))
                    {
                        error = false;
                        auto result = _internalFunction.find(codeWord.c_str());
                        if (result == _internalFunction.end()) {
                            error = true;
                        }
                        else
                        {
                            string fnName(codeWord);
                            if (oneProgramLine.at(0) == '=')
                            {
                                oneProgramLine.erase(0, 1);
                                RemoveFrontAndBackSpaces(oneProgramLine);
                            }

                            _internalFunction[fnName].SetFunctor(oneProgramLine);
                        }
                    }
                    else
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DEFFN));
                        error = true;
                    }
                }

                else if (cmd == CMD::ID_LET)
                {
                    int iWarning = 0;
                    bool error = false;
                    string varPoseName;
                    float fLetValue = 0.0f;
                    unsigned int uAxisNum = 0xFFFF;

                    CCompiledProgramLine scInsertData;
                    scInsertData._originalUncompiledCode += "LET ";
                    scInsertData._originalUncompiledCode += oneProgramLine;
                    scInsertData._iLineNumber = currentCodeLineNb;
                    scInsertData._command = CMD::ID_LET;

                    if (ExtractOneWord(oneProgramLine, codeWord))
                    {
                        scInsertData._strLabel[0] = codeWord;

                        uAxisNum = GetAxisNumberByAxisName(codeWord);
                        bool bRtnType = ExtractOneWord(oneProgramLine, codeWord);
                        ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                        scInsertData._strLabel[1] = codeWord;

                        if (bRtnType)
                        {
                            varPoseName = codeWord;
                            if (!ExtractOneWord(oneProgramLine, codeWord))
                                continue;

                            scInsertData._strLabel[2] = codeWord;

                            //	 문법 체크
                            if (!GetValueAtOneWord(codeWord, fLetValue, eAddr, iWarning))
                                error = true;

                            float fValues[TOTAXES] = { 0.0f, };

                            if (!GetPoseAtOneWord(varPoseName, fValues, iWarning))
                                error = true;
                        }
                        else
                            error = true;

                        if (uAxisNum == 0xFFFF || bRtnType == false)
                            error = true;

                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_LET));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                    _compiledRobotLanguageProgram.push_back(scInsertData);
                }

                else if (cmd == CMD::ID_DEFPOS)
                {
                    bool error = true;
                    std::string strLvalue, strRvalue;

                    VAR_TYPE iVarType = VAR_TYPE::NONE;
                    int iWarnMessage = -1;
                    std::vector<int> DimSizes;
                    std::vector<float> initValues;

                    //	A[2]
                    if (ExtractDeposDefine(oneProgramLine, iVarType, iWarnMessage, strLvalue, strRvalue, DimSizes, initValues))
                    {
						error = false;
						
						auto result = _internalPoseArray.find(strLvalue.c_str());
                        if (result != _internalPoseArray.end()) {
                            error = true;
                        }
                        else
                        {
                            if (AllocateInternalPoseDimension(strLvalue, DimSizes))
                            {
                                if (!strRvalue.empty())
                                    InitInternalPoseDimension(strLvalue, initValues);
                            }
                            else
                            {
                                error = true;
                                break;
                            }

                        }

                        //	 make a compiled robot command (Defining VAR)
                        CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                    }
					else
					{
						if (iWarnMessage == WRN::WRN_DIMINIT_WRONG)
							_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_DIMINIT_WRONG));
						
						_compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DEFPOS));
						CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
						_compiledRobotLanguageProgram.push_back(a);
					}

                }

                else if (cmd == CMD::ID_DIM) // DIM - 배열 변수 선언, ex) VAR AA[20]
                {
                    bool error = true;
                    std::string strLvalue, strRvalue;

                    //	컴마의 갯수 만큼이 정의되는 변수의 개수가 됨.한개도 없다면 한개의 변수는 정의 된 것으로 본다.
                    unsigned int uNumOfComma = GetCountComma(oneProgramLine) + 1;
					bool bNoWarn = true;

                    //	DIM A[2],B[2],C[3] has three iteration no.
                    for (int i = 0; i < (int)uNumOfComma; i++)
                    {
                        VAR_TYPE iVarType = VAR_TYPE::NONE;
                        int iWarnMessage = -1;
                        std::vector<int> DimSizes;

                        //	A[2]
                        if (ExtractDimDefine(oneProgramLine, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
                        {
                            error = false;

							if(iWarnMessage == WRN::WRN_DIMINIT_WRONG)
								_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_DIMINIT_WRONG));
							
                            //	in case of getting same name in Arrary memory
                            auto result = _internalArray.find(strLvalue.c_str());
                            if (result != _internalArray.end()) {
                                error = true;
                                break;
                            }
                            else  // allocatable
                            {
                                if (AllocateInternalVarDimension(strLvalue, DimSizes))
                                {
                                    if (!strRvalue.empty())
										bNoWarn = InitInternalVarDimension(strLvalue, (float)atof(strRvalue.c_str()));
                                }
                                else
                                {
                                    _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DIM_ALLOCATIONFAILED));
                                    break;
                                }
                            }

                            //	 make a compiled robot command (Defining VAR)
                            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                        else
                        {
                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DIM));
                            error = true;
                        }
                    }

					if(bNoWarn == false )
						_compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_OVERFLOW_DIM));

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DIM));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_STOP) // STOP
                {
                    bool error = true;

                    if (!ExtractOneWord(oneProgramLine,codeWord))
                    {
                        error = false;
                        CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_STOP));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }

                }

                if (cmd == CMD::ID_END) // END
                {
                    bool error = true;

                    if (!ExtractOneWord(oneProgramLine,codeWord))
                    {
                        error = false;
                        CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_END));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_IF)
                { //IF
                    bool error = true;

                    firstIfElse.clear();

                    if(oneProgramLine.find("ELSE") != std::string::npos && oneProgramLine.find("THEN") != std::string::npos )
                    {
                        if (ExtractOneWord(oneProgramLine,codeWord))
                        {
                            std::string variable = codeWord;
                            if (ExtractOneWord(oneProgramLine,codeWord))
                            {
                                int v;
                                if (GetIntegerFromWord(codeWord,v))
                                {

                                    if (ExtractOneWord(oneProgramLine,codeWord)) // extract "THEN"
                                    {
                                        if (ExtractOneWord(oneProgramLine,codeWord))  // "LABEL1"
                                        {
                                            std::string label1=codeWord;
                                            if (ExtractOneWord(oneProgramLine,codeWord)) // extract "ELSE"
                                            {
                                                if (ExtractOneWord(oneProgramLine,codeWord)) // "GOTO"
                                                {
                                                    if (ExtractOneWord(oneProgramLine,codeWord)) // "LABEL2"
                                                    {
                                                        std::string label2=codeWord;
                                                        if (!ExtractOneWord(oneProgramLine,codeWord))
                                                        {
                                                            error = false;
                                                            CCompiledProgramLine a;
                                                            a._command = cmd;
                                                            a._originalUncompiledCode = originalLine;
                                                            a._strLabel[1]=label1;
                                                            a._strLabel[2]=label2;
                                                            a._strLabel[0] = variable;
                                                            a._intParameter[0]=v;
                                                            a._intParameter[1]=currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
                                                            a._iLineNumber = currentCodeLineNb;
                                                            _compiledRobotLanguageProgram.push_back(a);
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                    }

                                }
                            }
                        }
                    }
                    else
                    {
                        nofIFstatement.push_back(currentCodeLineNb);

                        //	두줄 이상의 IF_ELSE 지령시 처리하는 부분, 이때 논리 연산자와 관계 연산자 등이 적용된다.
                        if (firstIfElse._bIfElseEnable == false && firstIfElse._bElseEnabled == false) {
                            error = false;
                            std::string::size_type idx;
                            idx = oneProgramLine.find("THEN");
                            oneProgramLine.erase(idx, oneProgramLine.length() - idx);
                            std::string rValue = oneProgramLine;
                            std::string tmpString;

                            // 관계 연산, 논리 연산 추가
                            if (DoRelationalOperation(oneProgramLine, iWarningMsg) < 0) error = true;
                            if (DoLogicalOperation(oneProgramLine, iWarningMsg) < 0) error = true;

                            firstIfElse._bIfElseEnable = true;
                            firstIfElse._ifElsePrevIfLine = _compiledRobotLanguageProgram.size();
                            firstIfElse._elseifLines.push_back(_compiledRobotLanguageProgram.size());

                            CCompiledProgramLine a(cmd, originalLine, currentCodeLineNb);
                            a._strLabel[1].clear();
                            a._strLabel[2] = oneProgramLine;
                            a._strLabel[3] = rValue;
                            a._intParameter[1] = currentCodeLineNb;

                            ExtractOneWord(rValue, tmpString);

                            a._intParameter[0] = atoi(oneProgramLine.c_str());
                            a._intParameter[1] = _compiledRobotLanguageProgram.size();

                            _compiledRobotLanguageProgram.push_back(a);

                            if (atoi(oneProgramLine.c_str()) > 0)
                                firstIfElse._ifElseResultLine = _compiledRobotLanguageProgram.size();
                        }
                        else
                            error = true;
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_IF));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_ELSEIF)
                { //IF
                    bool error = true;

                    if(oneProgramLine.find("THEN") != std::string::npos && firstIfElse._bIfElseEnable == true && firstIfElse._bElseEnabled == false)
                    {
                        error = false;
                        std::string::size_type idx;
                        idx = oneProgramLine.find("THEN");
                        std::string rValue;
                        oneProgramLine.erase(idx, oneProgramLine.length()- idx);
                        rValue = oneProgramLine;

                        if (DoRelationalOperation(oneProgramLine, iWarningMsg) < 0)
                        {
                            if(iWarningMsg == WRN::WRN_ALLOCATE_FAIL)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_ALLOCATE_FAIL));
                            if (iWarningMsg == WRN::WRN_FUNCTIONCALL_FAIL)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_FUNCTIONCALL_FAIL));

                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ELSEIF_AFTER_ELSE));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                            continue;
                        }
                        if (DoLogicalOperation(oneProgramLine, iWarningMsg) < 0 )
                        {
                            if (iWarningMsg == WRN::WRN_ALLOCATE_FAIL)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_ALLOCATE_FAIL));
                            if (iWarningMsg == WRN::WRN_FUNCTIONCALL_FAIL)
                                _compileWarningCode.insert(warningData(currentCodeLineNb, WRN::WRN_FUNCTIONCALL_FAIL));

                            _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ELSEIF_AFTER_ELSE));
                            CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                            _compiledRobotLanguageProgram.push_back(a);
                            continue;
                        }
                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._intParameter[0] = _compiledRobotLanguageProgram.size();
                        a._intParameter[1] = _compiledRobotLanguageProgram.size() + 1;
                        a._iLineNumber = currentCodeLineNb;

                        a._strLabel[1]="";
                        a._strLabel[2]=oneProgramLine;
                        a._strLabel[3]= rValue;

                        if(firstIfElse._elseifLines.size()>0)
                            _compiledRobotLanguageProgram[firstIfElse._elseifLines.back()]._intParameter[0] = _compiledRobotLanguageProgram.size();

                        firstIfElse._elseifLines.push_back(_compiledRobotLanguageProgram.size());
                        _compiledRobotLanguageProgram.push_back(a);

                        if(atoi(oneProgramLine.c_str()) > 0)
                        {
                            firstIfElse._ifElseResultLine = _compiledRobotLanguageProgram.size();
                        }
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ELSEIF_AFTER_ELSE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_ELSE)
                { //IF
                    bool error = true;
                    if(firstIfElse._bIfElseEnable == true && firstIfElse._bElseEnabled == false)
                    {
                        error = false;

                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._intParameter[0] = _compiledRobotLanguageProgram.size();
                        a._intParameter[1] = _compiledRobotLanguageProgram.size() + 1;
                        a._strLabel[1].clear();
                        a._strLabel[2]="";

                        if(firstIfElse._ifElseResultLine == 0)
                        {
                            firstIfElse._ifElseResultLine = _compiledRobotLanguageProgram.size();
                            a._strLabel[3]="1";
                        }
                        else
                            a._strLabel[3]="0";

                        a._iLineNumber = _compiledRobotLanguageProgram.size();
                        _compiledRobotLanguageProgram.push_back(a);
                        firstIfElse._bElseEnabled = true;

                        if(firstIfElse._elseifLines.size()>0)
                        {
                            _compiledRobotLanguageProgram[firstIfElse._elseifLines.back()]._intParameter[0] = _compiledRobotLanguageProgram.size()-1;
                            _compiledRobotLanguageProgram[firstIfElse._elseifLines.back()]._intParameter[1] = _compiledRobotLanguageProgram.size()-1;
                        }

                        firstIfElse._elseifLines.push_back(_compiledRobotLanguageProgram.size()-1);
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ELSE));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_ENDIF)
                { //IF
                    bool error = true;
                    nofENDIFstatement.push_back(currentCodeLineNb);

                    if(firstIfElse._bIfElseEnable == true)
                    {
                        error = false;
                        firstIfElse._bIfElseEnable = false;
                        firstIfElse._ifElseEndLine = _compiledRobotLanguageProgram.size();

                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._intParameter[0] = _compiledRobotLanguageProgram.size();
                        a._intParameter[1] = _compiledRobotLanguageProgram.size() + 1;
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);

                        if(firstIfElse._elseifLines.size()>0)
                            _compiledRobotLanguageProgram[firstIfElse._elseifLines.back()]._intParameter[0] = _compiledRobotLanguageProgram.size()-1;

                        while(firstIfElse._elseifLines.size()>0)
                        {
                            _compiledRobotLanguageProgram[firstIfElse._elseifLines.back()]._intParameter[1] = firstIfElse._ifElseResultLine;
                            firstIfElse._elseifLines.pop_back();
                        }
                        firstIfElse.clear();
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ENDIF));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_WAIT)//WAIT
                {
                    bool error = true;
                    CCompiledProgramLine a;

                    //	WAIT X=1, A*500
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        std::string variable = codeWord;
                        if (ExtractOneWord(oneProgramLine,codeWord))
                        {
                            float fValueforOneWord = 0.0f;
                            ADDRESS pAddr = ADDRESS::GENERAL_VAR;

                            if (GetValueAtOneWord(codeWord, fValueforOneWord, pAddr, iWarningMsg))
                            {
                                a._strLabel[1] = codeWord;

                                if (ExtractOneWord(oneProgramLine,codeWord)) // extract "THEN"
                                {
                                    // 5000
                                    int iWaitTime = 0;
                                    error = false;


                                    a._originalUncompiledCode = originalLine;
                                    a._command = cmd;
                                    a._strLabel[2] = codeWord;	//	 A*500
                                    a._strLabel[0] = variable;
                                    a._iLineNumber = currentCodeLineNb;

                                    a._intParameter[0] = (int) fValueforOneWord;
                                    a._intParameter[1] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass

                                    if(DoMathOperation(codeWord, iWarningMsg) < 0)
                                        continue;

                                    if (GetValueAtOneWord(codeWord, fValueforOneWord, pAddr, iWarningMsg))
                                        a._floatParameter[0]= fValueforOneWord /1000.0f; // convert from ms to s

                                    _compiledRobotLanguageProgram.push_back(a);

                                }

                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_WAIT));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                /*
                if (cmd == CMD::ID_IN)
                { //IN
                    bool error = true;
                    if (ExtractOneWord(codeLine,codeWord)) // 변수
                    {
                        std::string variable = codeWord;
                        if (ExtractOneWord(codeLine,codeWord)) // INTX
                        {
                            std::string input = codeWord;
                            if (!ExtractOneWord(codeLine,codeWord))
                            {
                                error = false;
                                CCompiledProgramLine a;
                                a._command = cmd;
                                a._originalUncompiledCode = originalLine;
                                a._strLabel[0] = variable;
                                a._strLabel[1]=input;
                                a._intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, cmd));
                        break;
                    }
                }

                if (cmd == CMD::ID_OUT)
                { //OUT
                    bool error = true;
                    if (ExtractOneWord(codeLine,codeWord)) // 변수
                    {
                        std::string output = codeWord;
                        if (ExtractOneWord(codeLine,codeWord)) // OUTX
                        {
                            std::string variable = codeWord;
                            if (!ExtractOneWord(codeLine,codeWord))
                            {
                                error = false;
                                CCompiledProgramLine a;
                                a._command = cmd;
                                a._originalUncompiledCode = originalLine;
                                a._strLabel[0]=output;
                                a._strLabel[1] = variable;
                                a._intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
                                _compiledRobotLanguageProgram.push_back(a);
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, cmd));
                        break;
                    }
                }
                */
                else if (cmd == CMD::ID_SET)
                { //SET
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord)) // iNTX, OUTX
                    {
                        std::string variable = codeWord;

                        error = false;
                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._strLabel[0] = variable;
                        a._intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_SET));
                        continue;
                    }
                }

                else if (cmd == CMD::ID_RESET)
                { //RESET
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord)) // iNTX, OUTX
                    {
                        std::string variable = codeWord;
                        if (!ExtractOneWord(oneProgramLine,codeWord))
                        {
                            error = false;
                            CCompiledProgramLine a;
                            a._command = cmd;
                            a._originalUncompiledCode = originalLine;
                            a._strLabel[0] = variable;
                            a._intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);
                        }

                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_RESET));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_FOR)
                { // we have a FOR here

                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord)) // AA
                    {
                        // 변수 확인 ...
                        std::string bk_codeWord = codeWord;

                        float dumm;
                        if( GetDataFromMap(codeWord, dumm) ) // Variable이 정의 되어있는지 확인하는 용도
                        {
                            if (ExtractOneWord(oneProgramLine,codeWord)) // 1
                            {
                                //수치 처리
                                int min;
                                if (GetIntegerFromWord(codeWord,min))
                                {
                                    if (ExtractOneWord(oneProgramLine,codeWord))// TO
                                    {
                                        // TO 인지 확인
                                        if( codeWord.compare("TO") ==0 )
                                        {
                                            if (ExtractOneWord(oneProgramLine,codeWord)) // 2
                                            {
                                                // 수치 처리
                                                int max;
                                                if (GetIntegerFromWord(codeWord,max))
                                                {
                                                    if (ExtractOneWord(oneProgramLine,codeWord))// STEP
                                                    {
                                                        // STEP 인지 확인
                                                        if( codeWord.compare("STEP") ==0 )
                                                        {
                                                            if (ExtractOneWord(oneProgramLine,codeWord)) // 2
                                                            {
                                                                // 수치 처리
                                                                int step;
                                                                if (GetIntegerFromWord(codeWord,step))
                                                                {
                                                                    error = false;
                                                                    //이상함. 수정
                                                                    forLocations.push_back(int(_compiledRobotLanguageProgram.size()));
                                                                    //forLocations.push_back(currentCodeLineNb-1);
#ifndef QT_COMPIL

                                                                    TRACE("FOR - forLocations = %d \r\n", currentCodeLineNb);
                                                                    TRACE("FOR - min, max, step = %d, %d, %d \r\n", min, max, step);
#endif

                                                                    CCompiledProgramLine a;
                                                                    a._command = cmd; //for
                                                                    a._originalUncompiledCode = originalLine;
                                                                    a._intParameter[0] = min; // min
                                                                    a._intParameter[1] = max; // max
                                                                    a._floatParameter[0] = (float) step; // step
                                                                    a._floatParameter[2] = (float) min; // initial value
                                                                    a._strLabel[1] = bk_codeWord; // 변수명
                                                                    a._iLineNumber = currentCodeLineNb;
                                                                    _compiledRobotLanguageProgram.push_back(a);
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        error = false;
                                                        //forLocations.push_back(int(_compiledRobotLanguageProgram.size()));
                                                        forLocations.push_back(currentCodeLineNb);
#ifndef QT_COMPIL
                                                        TRACE("FOR - forLocations = %d \r\n", currentCodeLineNb);
                                                        TRACE("FOR - min, max, step = %d, %d \r\n", min, max);
#endif
                                                        CCompiledProgramLine a;
                                                        a._command = cmd; //for
                                                        a._originalUncompiledCode = originalLine;
                                                        a._intParameter[0] = min; // min
                                                        a._intParameter[1] = max; // max
                                                        a._floatParameter[0]= (float) 1.0f; // step
                                                        a._floatParameter[2] = (float) min;	// initial value
                                                        a._strLabel[1] = bk_codeWord; // 변수명
                                                        a._iLineNumber = currentCodeLineNb;
                                                        _compiledRobotLanguageProgram.push_back(a);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_FOR));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_SWITCH)
                { // we have a SWITCH here

                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord) || firstLvSwitch._switchEnable  == false) // AA
                    {
                        // 변수 확인 ...
                        std::string bk_codeWord = codeWord;

                        float vSwitchValue = 0.0f;
						ADDRESS eAddr = ADDRESS::GENERAL_VAR;
						int iWarningMsg = 0;

						if (GetValueAtOneWord(codeWord, vSwitchValue, eAddr, iWarningMsg))	// Variable이 정의 되어있는지 확인하는 용도
                        //if( GetDataFromMap(codeWord, vSwitchValue) ) 
                        {

                            // 수치 처리
                            error = false;
#ifndef QT_COMPIL
                            TRACE("SWITCH \r\n", currentCodeLineNb);
#endif
                            CCompiledProgramLine a;
                            a._command = cmd; //for
                            a._originalUncompiledCode = originalLine;
                            a._intParameter[0] = (int)vSwitchValue; // min
                            a._intParameter[1] = 0; // 분기될 case 라인 넘버
                            a._strLabel[1] = bk_codeWord; // 변수명
                            a._iLineNumber = currentCodeLineNb;

                            // Switch 처리를 위해 Switch 처리 클래스를 생성한다.
                            a.createSwitchBranch();
                            _compiledRobotLanguageProgram.push_back(a);

                            // Switch문이 입력되었음을 알리는 boolean 을 true로 놓고, ENDSWITH문을 얻을때까지 유지한다..
                            firstLvSwitch._switchEnable = true;
                            firstLvSwitch._switchBegineLine = currentCodeLineNb;
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_SWITCH));
                        firstLvSwitch._switchEnable = false;
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_CASE)
                { // we have a CASE here

                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord) || firstLvSwitch._switchEnable  == true) // AA
                    {
                        // 변수 확인 ...
                        std::string bk_codeWord = codeWord;

                        int t = 0;
                        if (GetIntegerFromWord(codeWord,t))
                        {

                            // 수치 처리
                            error = false;
#ifndef QT_COMPIL
                            TRACE("CASE \r\n", currentCodeLineNb);
#endif
                            CCompiledProgramLine a;
                            a._command = cmd; //for
                            a._originalUncompiledCode = originalLine;
                            a._intParameter[0] = (int) t; // 케이스 번호
                            a._intParameter[1] = 0; // 점프될 라인 넘버
                            a._strLabel[1] = bk_codeWord; // 변수명
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);
                            firstLvSwitch._caseLines.push_back(currentCodeLineNb);

                            if(firstLvSwitch._switchEnable == true)
                            {
                                if (firstLvSwitch._switchBegineLine - 1 >= (int)  _compiledRobotLanguageProgram.size())
                                {
                                    error = true;
                                    _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_CASE));
                                    continue;
                                }
                                // CASE 문 내에 조건이 1,2,3,4 로 나열될 경우 1에 대한 라인, 2에 대한 라인.. 으로 map 데이터를 구축한다.
                                else
                                    _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._switchBranch->insert(make_pair(t, currentCodeLineNb));

                                if(_compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._intParameter[0] == t
                                    && _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._iLineNumber != 0)
                                    {
                                        // 현재 케이스 라인넘버 저장
                                        _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._intParameter[1] = currentCodeLineNb;
                                        firstLvSwitch._switchResultLine = currentCodeLineNb;
                                    }
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_CASE_NOSWITCH));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_DEFAULT)
                { // we have a CASE here

                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord) || firstLvSwitch._switchEnable  == true) // AA
                    {
                        // 변수 확인 ...
                        std::string bk_codeWord = codeWord;

                        int t = 0xffff;
                        error = false;
#ifndef QT_COMPIL
                        TRACE("DEFAULT \r\n", currentCodeLineNb);
#endif
                        CCompiledProgramLine a;
                        a._command = cmd; //for
                        a._originalUncompiledCode = originalLine;
                        a._intParameter[0] = (int) t; // 케이스 번호
                        a._intParameter[1] = currentCodeLineNb+1; // 점프될 라인 넘버
                        a._strLabel[1] = bk_codeWord; // 변수명
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);
                        firstLvSwitch._caseLines.push_back(currentCodeLineNb);

                        if(firstLvSwitch._switchEnable == true && firstLvSwitch._switchResultLine == 0)
                        {
                            // CASE 문 내에 조건이 만족하지 않는 경우이므로, 0xFFFF 값으로 map 데이터를 구축한다.
                            _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._switchBranch->insert(make_pair(t, currentCodeLineNb));

                            if(_compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._iLineNumber != 0)
                            {
                                // 현재 케이스 라인넘버 저장.
                                _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._intParameter[1] = currentCodeLineNb;
                                firstLvSwitch._switchResultLine = currentCodeLineNb;
                            }
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DEFAULT));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_BREAK)
                { // we have a BREAK here

                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord) || firstLvSwitch._switchEnable  == true) // AA
                    {
                        error = false;

                        // 변수 확인 ...
                        std::string bk_codeWord = codeWord;
#ifndef QT_COMPIL
                            TRACE("BREAK \r\n", currentCodeLineNb);
#endif
                            CCompiledProgramLine a;
                            a._command = cmd; //for
                            a._originalUncompiledCode = originalLine;
                            a._intParameter[0] = (int) currentCodeLineNb; // min
                            a._strLabel[1] = bk_codeWord; // 변수명
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);
                            firstLvSwitch._breakLines.push_back(currentCodeLineNb);

                            if(firstLvSwitch._switchEnable == true)
                                if(firstLvSwitch._switchBegineLine > 0
                                    && firstLvSwitch._switchResultLine < currentCodeLineNb
                                    && firstLvSwitch._switchBreakLine > currentCodeLineNb)
                                {
                                    firstLvSwitch._switchBreakLine = currentCodeLineNb;
                                }
                    }
                    if (error)
                    {
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_BREAK));
                        continue;
                    }
                }

                else if (cmd == CMD::ID_ENDSWITCH)
                { // we have a ENDSWITCH here
                    bool error = true;
                    if( firstLvSwitch._switchEnable == true)
                    {
                        error = false;
#ifndef QT_COMPIL
                        TRACE("ENDSWITCH, currentCodeLineNb %d \r\n", currentCodeLineNb);
#endif
                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);

                        firstLvSwitch._switchEnable = false;

                        //	Switch, Break 문에 이곳 ENDSWITH문의 라인넘버를 initParameter[1]에 넣어,
                        //	조건 완료시 ENDSIWTH문의 라인으로 점프하기 위한 용도로 아래의 데이터를 구성
                        while (firstLvSwitch._breakLines.size()>0)
                        {
                            if (firstLvSwitch._switchBegineLine - 1 >= (int) _compiledRobotLanguageProgram.size() ||
                                firstLvSwitch._breakLines.back() - 1>= (int) _compiledRobotLanguageProgram.size())
                            {
                                error = true;
                                _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_SWITCH));
                                break;
                            }
                            _compiledRobotLanguageProgram[firstLvSwitch._switchBegineLine-1]._intParameter[1] = currentCodeLineNb;
                            _compiledRobotLanguageProgram[firstLvSwitch._breakLines.back()-1]._intParameter[1] = currentCodeLineNb;
                            firstLvSwitch._breakLines.pop_back();
                        }
                    }
                    if (error)
                    {
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_ENDSWITCH));
                        continue;
                    }
                }

                else if (cmd == CMD::ID_NEXT)
                { // we have a NEXT instruction

                    bool error = true;

                    if( forLocations.size() > 0)
                    {
                        error = false;

                        int for_pos;
                        for_pos = forLocations.back(); // 주의 ... forLocations이 없을 경우 에러처리 추가해야 함
                        forLocations.pop_back();
#ifndef QT_COMPIL
                        TRACE("NEXT - for_pos = %d, currentCodeLineNb %d \r\n", for_pos, currentCodeLineNb);
#else
                         qDebug() << "NEXT - for_pos = " << currentCodeLineNb << for_pos << currentCodeLineNb<< "\r\n";
#endif
                        _compiledRobotLanguageProgram[for_pos]._floatParameter[1] = (float)_compiledRobotLanguageProgram.size()+1; // for문에 next 위치를 넣는다.

                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._floatParameter[0] = (float)for_pos; // next 문에 for 위치를 넣는다.
                        a._floatParameter[2] = (float)_compiledRobotLanguageProgram.size(); // 이상함.

                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);
                    }

                    if (error)
                    {
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_NEXT));
                        continue;
                    }
                }

                else if (cmd == CMD::ID_DELAY) // DELAY - 변수선언
                {
                    // we have a DELAY here
                    bool error = true;
                    float f;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        int t;
                        ADDRESS eAddr = ADDRESS::GENERAL_VAR;

                        if (GetIntegerFromWord(codeWord,t))
                        {
                            if (t>0)
                            {
                                if (!ExtractOneWord(oneProgramLine,codeWord))
                                {
                                    error = false;
                                    CCompiledProgramLine a;
                                    a._command = cmd;
                                    a._originalUncompiledCode = originalLine;
                                    a._floatParameter[0] = float(t)/1000.0f; // convert from ms to s
                                    a._strLabel[0].clear();
                                    a._iLineNumber = currentCodeLineNb;
                                    _compiledRobotLanguageProgram.push_back(a);
                                }
                            }
                        }
                        else if( GetValueAtOneWord(codeWord, f, eAddr, iWarningMsg) )
                        {
                            error = false;
                            CCompiledProgramLine a;
                            a._command = cmd;
                            a._originalUncompiledCode = originalLine;
                            a._floatParameter[0] = 0;
                            a._strLabel[0] = codeWord;
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_DELAY));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }


                else if (cmd == CMD::ID_GOTO)
                { // we have a GOTO here
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        std::string label(codeWord);
                        if (!ExtractOneWord(oneProgramLine,codeWord))
                        {
                            error = false;
                            CCompiledProgramLine a;
                            a._command = cmd;
                            a._originalUncompiledCode = originalLine;
                            a._strLabel[1] = label;
                            a._intParameter[0] = currentCodeLineNb; // the line number to jump to is set in the second compilation pass
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);
                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_GOTO));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_GOSUB)
                {
                    // we have a GOSUB here
                    bool error = true;
                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        std::string label(codeWord);
                        if (!ExtractOneWord(oneProgramLine,codeWord))
                        {
                            error = false;
                            CCompiledProgramLine a;
                            a._command = cmd;
                            a._originalUncompiledCode = originalLine;
                            a._strLabel[1] = label;
                            a._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(a);

                        }
                    }
                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_GOSUB));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }
                else if (cmd == CMD::ID_RETURN)
                { // we have a RETURN here
                    bool error = true;

                    std::string label(codeWord);
                    if (!ExtractOneWord(oneProgramLine,codeWord))
                    {
                        error = false;
                        CCompiledProgramLine a;
                        a._command = cmd;
                        a._originalUncompiledCode = originalLine;
                        a._iLineNumber = currentCodeLineNb;
                        _compiledRobotLanguageProgram.push_back(a);
                    }

                    if (error)
                    {
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_RETURN));
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        continue;
                    }
                }

                else if (cmd == CMD::ID_LINE_COMMENT)
                { // we have a comment here
                    CCompiledProgramLine a;
                    a._command = cmd;
                    a._originalUncompiledCode = originalLine;
                    a._iLineNumber = currentCodeLineNb;
                    _compiledRobotLanguageProgram.push_back(a);
                }

                else if (cmd == CMD::ID_PRESET) // PRESET
                {
                    bool error = true;
                    int iTempData;
                    CCompiledProgramLine scInsertData;

                    if (ExtractOneWord(oneProgramLine,codeWord))
                    {
                        iTempData = codeWord.find("OFF");

                        if (iTempData != -1)	// OFF
                        {
                            error = false;
                            scInsertData._command = cmd;
                            scInsertData._originalUncompiledCode = originalLine;
                            scInsertData._floatParameter[0] = -1;		// PRESET Cancel 처리
                            scInsertData._strLabel[0].clear();
                            scInsertData._iLineNumber = currentCodeLineNb;
                            _compiledRobotLanguageProgram.push_back(scInsertData);
                        }
                        else					// 4개의 변수 검출
                        {
                            int iDatacount = 0;
                        while (1)
                            //while (iDatacount < 4)
                            {
                                if (GetIntegerFromWord(codeWord,iTempData))
                                {
                                    // 변수 타입 에러
                                    if (iTempData < 0)	{
                                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_PRESET));
                                        break;
                                    }

                                    scInsertData._floatParameter[iDatacount] = (float) iTempData;
                                }
                                else if (codeWord.find("IN") != -1 && iDatacount == 0)
                                {
                                    codeWord.erase(0, 2);
                                    continue;
                                }
                                else if (codeWord.find("OUT") != -1 && iDatacount == 2)
                                {
                                    codeWord.erase(0, 3);
                                    continue;
                                }
                                else
                                {
                                    _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_PRESET));
                                    break;
                                }

                                if (!ExtractOneWord(oneProgramLine,codeWord))		// 변수 갯수 에러/완료
                                {
                                    break;
                                }

                                ++iDatacount;
                            }

                            if (iDatacount == 3)
                            {
                                error = false;
                                scInsertData._command = cmd;
                                scInsertData._originalUncompiledCode = originalLine;
                                scInsertData._strLabel[0].clear();
                                scInsertData._iLineNumber = currentCodeLineNb;
                                _compiledRobotLanguageProgram.push_back(scInsertData);
                            }
                        }
                    }
                    if (error)
                    {
                        CCompiledProgramLine a(CMD::ID_BLANK, originalLine, currentCodeLineNb);
                        _compiledRobotLanguageProgram.push_back(a);
                        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ERR_PRESET));
                        break;
                    }
                }	// End 'PRESET'
            }
        }
        _commModules->NotifyToProgressStatusToAllSubscribers((double)(_robotLanguageProgram.size() - tempCopyedProgram.size()) / _robotLanguageProgram.size() * 100);

        if (tempCopyedProgram.length()==0)
            break;
    }

    // we have a second pass where we need to set the line number where to jump to!
    for (unsigned int i=0;i<_compiledRobotLanguageProgram.size();i++)
    {
        if (_compiledRobotLanguageProgram[i]._command == CMD::ID_GOTO)
        { // this is a GOTO command.
            bool found=false;
            int nLabels = 0;

            for (unsigned int j=0;j<_labels.size();j++)
            {
                if (_labels[j].compare(_compiledRobotLanguageProgram[i]._strLabel[1])==0)
                {
                    _compiledRobotLanguageProgram[i]._intParameter[0]=_labelLocations[j];
                    nLabels++;
                    found=true;
                }
            }
            if (!found || nLabels == 0)
            {
                errline = _compiledRobotLanguageProgram[i]._iLineNumber;
                std::string retString;
                retString.assign("Error on line: %d", errline);
                _compileErrorCode.insert(errorData(errline, CRobotMessages::ERR_UNSIGNED_LABEL));
                retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i]._originalUncompiledCode);
                iFirstErrorLine = errline;
                break;
            }
        }

        if (_compiledRobotLanguageProgram[i]._command == CMD::ID_GOSUB)
        { // this is a GOSUB command.

            bool found=false;
            for (unsigned int j=0;j<_labels.size();j++)
            {
                if (_labels[j].compare(_compiledRobotLanguageProgram[i]._strLabel[1])==0)
                {
                    _compiledRobotLanguageProgram[i]._intParameter[0]=_labelLocations[j];
                    found=true;
                }
            }
            if (!found)
            {
                std::string retString;
                retString.assign("Error on line: %d", _compiledRobotLanguageProgram[i]._iLineNumber);
                errline = _compiledRobotLanguageProgram[i]._iLineNumber;
                _compileErrorCode.insert(errorData(errline, CRobotMessages::ERR_GOSUB));
                retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i]._iLineNumber);
                iFirstErrorLine = currentCodeLineNb;
            }
        }

        if (_compiledRobotLanguageProgram[i]._command == CMD::ID_IF && _compiledRobotLanguageProgram[i]._strLabel[3].empty())
        { // this is a IF command
            bool  bLeft=false, bRight=false;
            for (unsigned int j=0;j<_labels.size();j++)
            {
                if (_labels[j].compare(_compiledRobotLanguageProgram[i]._strLabel[1])==0)
                {
#ifndef QT_COMPIL
                    TRACE("1 i=%d, j=%d \r\n", i, j, _labelLocations[j]);
#endif
                    _compiledRobotLanguageProgram[i]._intParameter[1]=_labelLocations[j];
                    bLeft =true;
                }

                if (_labels[j].compare(_compiledRobotLanguageProgram[i]._strLabel[2])==0)
                {
#ifndef QT_COMPIL
                    TRACE("2 i=%d, j=%d \r\n", i, j, _labelLocations[j]);
#endif
                    _compiledRobotLanguageProgram[i]._intParameter[2]=_labelLocations[j];
                    bRight =true;
                }
            }
            if (!bLeft || !bRight)
            {
                std::string retString;
                retString.assign("Error on line: %d", _compiledRobotLanguageProgram[i]._iLineNumber);

                retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i]._intParameter[1]);
                iFirstErrorLine = _compiledRobotLanguageProgram[i]._iLineNumber;
                break;
            }
        }
    }

    //	IF/ENDIF 개수가 맞지 않을 경우 체크
    if (nofIFstatement.size() != nofENDIFstatement.size())
    {
        if(nofIFstatement.size() - nofENDIFstatement.size() > 0)
            _compileErrorCode.insert(errorData(nofIFstatement.back(), CRobotMessages::ECompileError::ERR_MISMATCH_NOOF_IFENDIF));
        else
            _compileErrorCode.insert(errorData(nofENDIFstatement.back(), CRobotMessages::ECompileError::ERR_MISMATCH_NOOF_IFENDIF));
    }

    if (forLocations.size() > 0)
        _compileErrorCode.insert(errorData(currentCodeLineNb, CRobotMessages::ECompileError::ERR_FOR));

    nTotalError = _compileErrorCode.size();

    if (_compileErrorCode.size() > 0)
    { // we return an error message
        GetFirstError(iFirstErrorLine, nTotalError);
        return;
    }
    else
        iFirstErrorLine = 0;
}


bool CKss1500Interprerter::GetErrorLogFromMap(int& variable, int &value)
{	//	Get a data from map
    auto result = _compileErrorCode.find(variable);

    if (result != _compileErrorCode.end())
    {
        value = result->second;
        return true;
    }
    else
        return false;
}

bool CKss1500Interprerter::GetTotalErrorCodes(std::vector<int>& variable, std::vector<int>& value)
{
    variable.clear();
    //	Get a data from map
#ifndef QT_COMPIL
    for each(auto result in _compileErrorCode)
#else
    for(auto result : _compileErrorCode)
#endif
    {
        variable.push_back(result.first);
        value.push_back(result.second);
    }

    if (variable.size() > 0)
        return true;
    else
        false;
}

bool CKss1500Interprerter::GetTotalWarningMessages(std::vector<int>& variable, std::vector<int>& value)
{
    variable.clear();
    //	Get a data from map
#ifndef QT_COMPIL
        for each(auto result in _compileWarningCode)
#else
        for(auto result : _compileWarningCode)
#endif
    {
        variable.push_back(result.first);
        value.push_back(result.second);
    }

    if (variable.size() > 0)
        return true;
    else
        false;
}


bool CKss1500Interprerter::GetFirstError(int& variable, int &value)
{	//	Get a data from map
    auto result = _compileErrorCode.begin();
    if (result != _compileErrorCode.end())
    {
        variable = result->first;
        value = result->second;
        return true;
    }
    else
        return false;
}

void CKss1500Interprerter::ClearCompliedErrorLog(void)
{
    _compileErrorCode.clear();
    _compileWarningCode.clear();
}

bool CKss1500Interprerter::OnelineSyntaxCheckinCompileTime(const string& oneLine, int lineNo, CMD command, std::list<ECompileError>& errorNo, std::list<EWarningID>& warningNo)
{
    std::string line(oneLine);
    std::vector<string> extracedWords;
	
	char cDimChar = ' ';
    int iPos = 0;
	int ilBracePos = -0xFFFF;
	int irBracePos = 0xFFFF;
	int ilMBracePos = -0xFFFF;
	int irMBracePos = 0xFFFF;
	int ilSBracePos = -0xFFFF;
	int irSBracePos = 0xFFFF;

    std::string	Arg = "";
    bool blBrace = false;
    bool brBrace = false;
    bool blMBrace = false;
    bool brMBrace = false;
    bool blSBrace = false;
    bool brSBrace = false;

    int nlBrace = 0;
    int nrBrace = 0;
    int nlMBrace = 0;
    int nrMBrace = 0;
    int nlSBrace = 0;
    int nrSBrace = 0;
    int nComman = 0;

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == '[')
        {
			ilBracePos = iPos;
            nlBrace++;
            blBrace = true;        //   [a][b][c]
        }
        else if (c == ']')
        {
			irBracePos = iPos;
            nrBrace++;
            brBrace = true;
        }
        else if (c == '(')
        {
			ilSBracePos = iPos;
            nlSBrace++;
            blSBrace = true;        //   (a)(b)
        }
        else if (c == ')')
        {
			irSBracePos = iPos;
            nrSBrace++;
            brSBrace = true;        //   [a][b][c]
        }
        else if (c == '{')
        {
			irMBracePos = iPos;
            nlMBrace++;
            blMBrace = true;        //   [a][b][c]
        }
        else if (c == '}')
        {
			irMBracePos = iPos;
            nrMBrace++;
            brMBrace = true;
        }
        else if (c == ',')
        {
            nComman++;
        }
        else if (c == '\\' || c == '^' || c == '~' || c == '“' || c== '"' || c =='♥')
        {
            errorNo.push_back(ECompileError::ERR_SPECIAL_CHARACTER);
        }
        else
            Arg += c;

        if (blBrace == true && brBrace == true)
        {
            if (c == NULL)
                break;

            blBrace = false;
            brBrace = false;
			int idxL = 0, idxR;
			
			idxL = Arg.find_first_of("{");
			idxR = Arg.find_last_of("}");

			//	A[], A[0] 에러체크
			if(irBracePos-ilBracePos == 1)
				errorNo.push_back(ECompileError::ERR_NOVALUE_WITHINBRACE);
			else
				cDimChar = Arg.at(Arg.size()-1);

			if(!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
                 errorNo.push_back(ECompileError::ERR_INVALID_MBRACE);

			idxL = Arg.find_first_of("(");
			idxR = Arg.find_last_of(")");

			if (!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
                errorNo.push_back(ECompileError::ERR_INVALID_SBRACE);
			
			Arg.clear();
        }
        else if (blMBrace == true && brMBrace == true)
        {
			if (c == NULL)
				break;

			blBrace = false;
			brBrace = false;
			int idxL = 0, idxR;

			idxL = Arg.find_first_of("[");
			idxR = Arg.find_last_of("]");

			if (!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
				errorNo.push_back(ECompileError::ERR_INVALID_LBRACE);

			idxL = Arg.find_first_of("(");
			idxR = Arg.find_last_of(")");

			if (!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
				errorNo.push_back(ECompileError::ERR_INVALID_SBRACE);

			Arg.clear();
        }
		else if (blSBrace == true && brSBrace == true)
		{
			if (c == NULL)
				break;

			blBrace = false;
			brBrace = false;
			int idxL = 0, idxR;

			idxL = Arg.find_first_of("[");
			idxR = Arg.find_last_of("]");

			if (irSBracePos - ilSBracePos == 1)
				errorNo.push_back(ECompileError::ERR_DIMINIT_NOVALUE_WRONG);


			if (!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
				errorNo.push_back(ECompileError::ERR_INVALID_LBRACE);

			idxL = Arg.find_first_of("{");
			idxR = Arg.find_last_of("}");

			if (!(idxL != std::string::npos && idxR != std::string::npos
				|| idxL == std::string::npos && idxR == std::string::npos))
				errorNo.push_back(ECompileError::ERR_INVALID_MBRACE);

			Arg.clear();
		}
		iPos++;
    }

//	if (nlBrace != 1 && nrBrace != 1 && cDimChar == '0' )
//		errorNo.push_back(ECompileError::ERR_ZERO_INDIM);

    if (nlBrace != nrBrace)
        errorNo.push_back(ECompileError::ERR_LBRACE_NO);

    if (nlMBrace != nrMBrace)
        errorNo.push_back(ECompileError::ERR_MBRACE_NO);

    if (nlSBrace != nrSBrace)
        errorNo.push_back(ECompileError::ERR_INVALID_SBRACE);

    if (warningNo.size() > 0 || errorNo.size() > 0)
        return false;

    return true;
}
