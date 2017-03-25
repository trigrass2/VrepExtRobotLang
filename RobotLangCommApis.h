#ifndef CROBOTLANGCOMMRESOURCES_H
#define CROBOTLANGCOMMRESOURCES_H

#include <list>
#include <vector>
#include <map>
#include "VrepExtExport.h"

class VREP_ROBOTLANG_API CPoint3D  // position vector
{

public:
    float X, Y, Z;

public:
    CPoint3D();
    CPoint3D(float x, float y, float z);
    friend CPoint3D operator + (class CPoint3D, class CPoint3D);
    friend CPoint3D operator - (class CPoint3D, class CPoint3D);
    friend CPoint3D operator * (class CPoint3D, float);
    friend CPoint3D operator / (class CPoint3D, float);
    virtual ~CPoint3D();
};



void SetVarBit(int &var, int bitno, bool bset);
bool GetVarBit(int &var, int bitno);


//	 Switch keyword for DMBH Robot
class  CSwitch
{
public:
    bool _switchEnable;		//	 Switch keyword was given
    int _switchBegineLine;	//	 SWITCH (..)  - Line number
    int _switchBreakLine;	//	 BREAK - Line number
    int _switchResultLine;	//	 CASE - Result line number

    std::vector<int> _caseLines;		//	 Case line numbers
    std::vector<int> _breakLines;	//	 Break line numbers
    void clear(void ){
        _switchEnable = false;	// switch, switchend
        _switchBegineLine = 0;
        _switchBreakLine = 0xFFFF;
        _switchResultLine = 0;
    }
    CSwitch() {
        clear();
    }
};

class  CIfElseThen
{
public:
    bool _bIfElseEnable;	//	IF keyword was given
    int _ifElsePrevIfLine;	//	Previous line number of ELSSIF
    int _ifElseEndLine;		//	Line number of ENDIF
    int _ifElseResultLine;	//	IF/ELSE - Result line number
    bool _bIgnoreCondition;	//	If given condition was matched, then this boolean variable will be true.
    bool _bElseEnabled;		//	ELSE keyword was given

    std::vector<int> _elseifLines;

    CIfElseThen(void) {
        clear();
    }
    void clear() {
        _bIfElseEnable = false;
        _ifElsePrevIfLine = 0;
        _ifElseEndLine = 0xFFFF;
        _ifElseResultLine = 0;
        _bElseEnabled = false;
        _bIgnoreCondition = false;
    }
};

class VREP_ROBOTLANG_API CCompiledProgramLine
{
public:

    // Command ID
    typedef enum eCommandID
    {
        ID_LABEL_DEFAULT = -1,
        ID_DELAY = 1,
        ID_GOTO,
        ID_LINE_COMMENT = 4,
        ID_GOHOME = 17,
        ID_SPEED, ID_ROTATE, ID_GRASP, ID_RELEASE, ID_CHANGE,
        ID_DRIVE, ID_MOVE, ID_VAR, ID_STOP, ID_END,
        ID_GOSUB, ID_RETURN, ID_IF, ID_FOR, ID_NEXT,
        ID_WAIT, ID_IN, ID_OUT, ID_SET, ID_RESET,
        ID_READY, ID_PRESET, ID_MOVE2, ID_SWITCH, ID_BREAK,
        ID_CASE, ID_DEFAULT, ID_ENDSWITCH, ID_ELSEIF, ID_ELSE,
        ID_ENDIF, ID_POSE, ID_DIM, ID_LABEL, ID_DEFPOS,
        ID_LET, ID_FN, ID_DEF,
        ID_BLANK = 99,
        ID_VALUE,
    } PGM_CMD;

    std::string _originalUncompiledCode;
    int _command;						// 0=DRAW, 1=DELAY, 2=GOTO, 3=SPEED, etc.
    int _intParameter[2];				// _intParameter[0]: If command==GOTO, then compiled prg line to jump to. if command==SETBIT or CLEARBIT, then the bit number to set/clear (1-32)
                                        // _intParameter[1]: If command==IFBITGOTO or IFNBITGOTO, then compiled prg line to jump to
    float _floatParameter[7];			// if command==DRIVE or JDRIVE, then target joint x values, if command==SPEED or RSPEED, then desired cartesian or joint velocity (m/s / rad/s), if command==DELAY, then time to wait (s).
    int _iLineNumber;					// Current Line Number
    std::string _strLabel[5];			// 기존 tmp/tmpLabel/tmpLabel2 대체
    std::map<int, int>* _switchBranch;	//	 Switch reserved word was given, then this will be created

    CCompiledProgramLine();
    CCompiledProgramLine(int cmd, const std::string& originaData, int lineNumber);
    ~CCompiledProgramLine();
    void createSwitchBranch(void);		//	Line numbers to jump for switch
    void delete_switchBranch(void);		//  Destruction
    void deleteChainedProgramLine(void);//	For multiple CCompiledProgramLine
    void createChainedProgramLine(void);//				"
    std::list<CCompiledProgramLine> *_compiledChainedMotionProgram;		//	If one command has serial motion commands, then this will be created.
    bool IsINcommand(void) const;
    bool IsOUTcommand(void) const;
    void clear();						//	Reset data members
};

#endif // CROBOTLANGCOMMRESOURCES_H
