#pragma once

#include "stdafx.h"
#include "IRobotLang.h"
#include "CommonH.h"
#include "atlstr.h"
#include "VrepExtExport.h"

#include "Kss1500MoveRange.h"
#include "Kss1500Auxiliary.h"
#include <vector>
#include <list>
#include <afxtempl.h>
#include <map>

using namespace std;

class CSwitch
{
public:
	bool bSwitchEnable_;
	int switchBegineLine_;
	int switchBreakLine_;
	int switchResultLine_;

	std::vector<int> caseLines;
	std::vector<int> breakLines;

	CSwitch(){
		bSwitchEnable_ = false;	// switch, switchend
		switchBegineLine_ = 0;
		switchBreakLine_ = 0xFFFF;
		switchResultLine_ = 0;
	}
};

class CIfElseThen
{
public:
	bool bIfElseEnable_;
	int ifElsePrevIfLine_;
	int ifElseEndLine_;
	int ifElseResultLine_;
	bool bElseEnabled_;

	std::vector<int> elseifLines;

	CIfElseThen(void){
		clear();
	}
	void clear(){
		bIfElseEnable_ = false;
		ifElsePrevIfLine_ = 0;
		ifElseEndLine_ = 0xFFFF;
		ifElseResultLine_ = 0;
		bElseEnabled_ = false;
	}
};


class SCompiledProgramLine
{
public:	
	std::string correspondingUncompiledCode;
	int command;				// 0=DRAW, 1=DELAY, 2=GOTO, 3=SPEED, etc. 
	int intParameter[2];		// intParameter[0]: If command==GOTO, then compiled prg line to jump to. if command==SETBIT or CLEARBIT, then the bit number to set/clear (1-32)  
								// intParameter[1]: If command==IFBITGOTO or IFNBITGOTO, then compiled prg line to jump to  
	float floatParameter[7];	// if command==DRIVE or JDRIVE, then target joint x values, if command==SPEED or RSPEED, then desired cartesian or joint velocity (m/s / rad/s), if command==DELAY, then time to wait (s). 
	int iLineNumber;			// Current Line Number
	std::string strLabel[5];	// 기존 tmp/tmpLabel/tmpLabel2 대체
	map<int, int> *switchBranch;
	void createSwitchBranch(void);
	void deleteSwitchBranch(void);
	void deleteChainedProgramLine(void);
	void createChainedProgramLine(void);
	std::list<SCompiledProgramLine> *_compiledChainedMotionProgram;
	void clear();
	SCompiledProgramLine();
	~SCompiledProgramLine();
};


class VREP_ROBOTLANG_API CKss1500Interprerter :public IDMBHRobot, protected CKss1500MoveRange, public CKss1500Auxiliary
{

protected:
	std::string _robotLanguageProgram; // 여기에 text 구문이 들어 감. _robotLanguageProgram
	std::vector<SCompiledProgramLine> _compiledRobotLanguageProgram;
	int currentProgramLine; // The current program counter (number is relative to the compiled program!)
	CMap<CString, LPCTSTR, float, float> m_mIntVar;// 2014_0206 add 
//	CUIntArray _arrReturn; // GOSUB-RETURN
	std::vector<int> _arrReturn;
	bool currentProgramLineNotYetPartiallyProcessed;

public:
	int _nInputM, _nOutputM; // shseo 주의: input + output 처리부 확인 필요 

//	CScriptText::lineVector _robotLanguageCommentColor;		// 주석문 삭제 활용 데이터로 주석 칼라로 삭제 글자 비교용 - 2015.08.10
	
public:
	void setProgram(const std::string& prg);
	std::string getProgram();
	int  GetCountComma(std::string line); 
	bool GetDataFromMap(CString variable, float &value); 
	bool GetDataFromIO(CString io, int &value); 
	void SetDataToOut(CString io, int &value); 
	bool GetDataFromINI(CString variable, float *fPos);		// 임시 TP Save - 2015.09.15
	bool SetDataToINI(CString variable, float *fPos);

	bool _extractOneLine(std::string& inputString,std::string& extractedLine);
	bool _extractOneWord(std::string& line,std::string& extractedWord, const char* opr = " ");
	bool _extractWordRelationOpr(std::string& line,std::string& lExtWord, std::string& extOpr, std::string& rExtWord,std::string& wholeWord);
	bool _extractWordMathOpr(std::string& line,std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord);
	bool _extractWordLogicOpr(std::string& line,std::string& lExtWord, std::string& extOpr,std::string& rExtWord, std::string& wholeWord);
	bool _extractWordPoseOpr(std::string& line,std::string& lExtWord, std::string& extOpr,std::string& rExtWord);
	bool _getCommandFromWord(const std::string& word,int& command);
	bool _getIntegerFromWord(const std::string& word,int& theInteger);
	bool _getFloatFromWord(const std::string& word,float& theFloat);
	bool _extractMultiWord(std::string& str,vector<string>& extracedWords);
	bool _removeFrontAndBackSpaces(std::string& word, bool bCommentLine = FALSE);
	bool IsChainMotionProgram(std::string &strStatement, int& nMoveCount, int& nPoseInMoveCount );
	bool _extractXyzPosition(std::string &strStatement);
	bool _extractXyzrgPosition(std::string &strStatement, float val[]);
	bool _generateSerialMotion(std::string &strStatement, SCompiledProgramLine&);
	int _exactractPoseData(const std::string&, std::string &strStatement, SCompiledProgramLine&);
	bool _exactractOneMoveData(std::string &lExtWord, std::string &rExtWord);
	void compileCode(int &errline, std::string& );
	int DoRelationalOperation(std::string &strStatement);
	int DoLogicalOperation(std::string &strStatement);
	int DoMathOperation(std::string &strStatement);
	void GetCurrentRobotMotionInfo(const SCompiledProgramLine&, GEO& currPos, ROT& currRos, float& currGrpPos,bool UpdateSavedPosALpha = false);	//	Get current motion information 
	int DoPoseOperation(const SCompiledProgramLine& currProgramLine , float fTargValue[], std::string& rtnPose, const GEO& currPos, const ROT& currRos, const float& currGrpPos);
//	void boostAction(vector<string> &v, int &lastlen, const char *begin, const char * end);
	CKss1500Interprerter(void);
	~CKss1500Interprerter(void);
};

