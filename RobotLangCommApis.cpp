#include "RobotLangCommApis.h"

void SetVarBit(int &var, int bitno, bool bset)
{
    if( bset )
    {
        var |= ( 1 << bitno );
    }
    else
    {
        var &= ~( 1 << bitno );
    }
}

bool GetVarBit(int &var, int bitno)
{
    bool b;

    b = (var >> bitno) & 0x01;

    return b;
}

CCompiledProgramLine::CCompiledProgramLine()
{
    clear();
}

CCompiledProgramLine::CCompiledProgramLine(int cmdLine, const std::string& originaData, int lineNumber)
{
    clear();
    this->_command = int(cmdLine);
    this->_originalUncompiledCode = originaData;
    this->_iLineNumber = lineNumber;
}


void CCompiledProgramLine::clear()
{
    _switchBranch = nullptr;
    _intParameter[0] = 0;
    _intParameter[1] = 0;

    if (_compiledChainedMotionProgram)
        _compiledChainedMotionProgram = nullptr;

    _originalUncompiledCode.clear();
    _command = ID_BLANK;

    _iLineNumber = 0;

    for (int i = 0; i<7; i++) _floatParameter[i] = 0.0f;
    for (int i = 0; i<5; i++) _strLabel[i].clear();
}

void CCompiledProgramLine::createSwitchBranch(void)
{
    if (!_switchBranch)
        _switchBranch = new std::map<int, int>;
}

void CCompiledProgramLine::delete_switchBranch(void)
{
    if (_switchBranch)
    {
        delete _switchBranch;
        _switchBranch = nullptr;
    }
}
void CCompiledProgramLine::createChainedProgramLine(void)
{
    if (_compiledChainedMotionProgram == NULL)
        _compiledChainedMotionProgram = new std::list<CCompiledProgramLine>;
    else
        _compiledChainedMotionProgram->clear();
}

void CCompiledProgramLine::deleteChainedProgramLine(void)
{
    if (_compiledChainedMotionProgram != NULL)
    {
        delete _compiledChainedMotionProgram;
        _compiledChainedMotionProgram = NULL;
    }
}


bool CCompiledProgramLine::IsINcommand(void) const
{
    if (_command == PGM_CMD::ID_VALUE)
    {
        std::string variable(_strLabel[0]);
        int thisIsIn = variable.find("IN");

        //	in case of IN0 = 1 of IN0
        if (thisIsIn != std::string::npos)
            variable = variable.substr(thisIsIn + 2, variable.size() - thisIsIn);
        else
        {
            //	in case of X = IN0
            thisIsIn = variable.assign(_strLabel[1]).find("IN");

            if (thisIsIn != std::string::npos)
                variable = variable.substr(thisIsIn + 2, variable.size() - thisIsIn);
        }

        if (thisIsIn != std::string::npos)
        {
            int portRange = atoi(variable.c_str());

            if (portRange >= 0 && portRange <= 7)
            {
                return true;
            }
            else
                return false;
        }
    return false;
    }
    return false;
}

bool CCompiledProgramLine::IsOUTcommand(void) const
{
    if (_command == PGM_CMD::ID_VALUE)
    {
        // in case of OUT0 = 1
        std::string variable(_strLabel[0]);
        int thisIsIn = variable.find("OUT");

        if (thisIsIn != std::string::npos)
            variable = variable.substr(thisIsIn + 3, variable.size() - thisIsIn);

        if (thisIsIn != std::string::npos)
        {
            int portRange = atoi(variable.c_str());

            if (portRange >= 0 && portRange <= 7)
            {
                return true;
            }
            else
                return false;
        }
        return false;
    }
    return false;
}

CCompiledProgramLine::~CCompiledProgramLine()
{

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPoint3D::CPoint3D()
{
    X = 0.0f;
    Y = 0.0f;
    Z = 0.0f;
}

CPoint3D::CPoint3D(float x, float y, float z)
{
    this->X = x;
    this->Y = y;
    this->Z = z;
}

////////////////////////////////////////////////////////////

CPoint3D::~CPoint3D()
{

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CPoint3D operator + (CPoint3D A, CPoint3D B)
{
    CPoint3D temp;
    temp.X = A.X + B.X;
    temp.Y = A.Y + B.Y;
    temp.Z = A.Z + B.Z;
    return(temp);
}

CPoint3D operator - (CPoint3D A, CPoint3D B)
{
    CPoint3D temp;
    temp.X = A.X - B.X;
    temp.Y = A.Y - B.Y;
    temp.Z = A.Z - B.Z;
    return(temp);
}

CPoint3D operator * (CPoint3D A, float B)
{
    CPoint3D temp;
    temp.X = A.X*B;
    temp.Y = A.Y*B;
    temp.Z = A.Z*B;
    return(temp);
}

CPoint3D operator / (CPoint3D A, float B)
{
    CPoint3D temp;
    temp.X = A.X / B;
    temp.Y = A.Y / B;
    temp.Z = A.Z / B;
    return(temp);
}

