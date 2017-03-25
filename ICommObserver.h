#pragma once

#include "CommonH.h"

using namespace std;

typedef CCompiledProgramLine::PGM_CMD CMD;

//	RS-232C, iRoDi, Internal communication
class VREP_ROBOTLANG_API ICommObserver
{
public:
	virtual int GetID(void) = 0;
	virtual const std::string& GetName(void) = 0;
	virtual void SetName(const std::string&) = 0;
	virtual void SetID(int) = 0;
	virtual int Update(void) = 0;
	virtual int UpdateProgressStatus(double) = 0;
	virtual int Update(const int& INport, const int& OUTport, int OutportType = 0) = 0;
};
