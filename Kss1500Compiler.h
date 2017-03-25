#pragma once

#ifndef QT_COMPIL
    #include "stdafx.h"
    #include "atlstr.h"
#endif
#include "IRobotMotion.h"
#include "ICommObserver.h"
#include "ICommSubscribers.h"
#include "CommonH.h"
#include "VrepExtExport.h"
#include "RobotLangParserOpr.h"
#include "Kss1500MoveRange.h"
#include "Kss1500RobotMotion.h"
#include <vector>
#include <list>
#include "RobotMessages.h"

#ifndef QT_COMPIL
    #include <afxtempl.h>
#endif

#include <map>

using namespace std;

class VREP_ROBOTLANG_API CKss1500Interprerter : public CKss1500RobotMotion, public CRobotLangParserAndOperator, public CRobotMessages
{
public:

	enum eInterpreterStatus
	{
		MSTATUS_ERROR = -1,
		MSTATUS_STANDBY = 0,
		MSTATUS_RUN,
		MSTATUS_STEPDONE,
        MSTATUS_COMMUNCATION,
        MSTATUS_COMMAND,
    } _eInterpreterStatus;

protected:		//	Communicators
    ICommSubject* _commModules;

private:
	std::string _robotSubProgram;			// Part program for TP

protected:
	std::vector<int> _arrReturn;
	std::string _robotLanguageProgram;		// Robot main program
	std::vector<CCompiledProgramLine> _compiledRobotLanguageProgram;

public:
	CKss1500Interprerter(void);
	~CKss1500Interprerter(void);
	const std::string& GetProgram(void);
	const std::string& GetSubProgram(void);
	void AssigningErrorMessages(void);
	void SetProgram(const std::string&);
	void SetSubProgram(const std::string&);
	void AddOneLetterToProgram(const char&);
	void SetProgram(const char*);
	void CompileCode(int& nTotalError, int& iFirstErrorLine);
	unsigned int GetTotCompiledLineNo(void) { return _compiledRobotLanguageProgram.size(); }
	bool GetErrorLogFromMap(int& variable, int &value);
	void ClearCompliedErrorLog(void);
	eInterpreterStatus GetCurrentInterpreterStatus(void) {return _eInterpreterStatus;}
    void SetCurrentInterpreterStatus(eInterpreterStatus mode) {_eInterpreterStatus = mode;}
	bool GetFirstError(int& variable, int &value);
	virtual bool GetTotalErrorCodes(std::vector<int>& variable, std::vector<int>& value);
	virtual bool GetTotalWarningMessages(std::vector<int>& variable, std::vector<int>& value);
    char GetAtLetterFromProgram(int i);
    unsigned int GetProgramSize() {return _robotLanguageProgram.size();}
	bool UpdateIOport(int input, int output, int ChangedOutput = 0);	//	Update IO port data, int ChangedOutput IN=0, OUT=1, IN/OUT=2
	bool UpdateInPort(int input);	//	Update IO port data, int ChangedOutput IN=0, OUT=1, IN/OUT=2
	bool OnelineSyntaxCheckinCompileTime(const string& oneLine, int lineNo, CMD command, std::list<ECompileError>& errorNo, std::list<EWarningID>& warningNo);
};
