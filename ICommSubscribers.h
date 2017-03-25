#pragma once
#include "ICommObserver.h"

typedef CCompiledProgramLine::PGM_CMD CMD;

class VREP_ROBOTLANG_API ICommSubject
{
public:
	virtual int Attach(ICommObserver*) = 0;
	virtual int Detach(ICommObserver*) = 0;
	virtual int DetachAll(void) = 0;
	virtual int FindObjectID(std::string) = 0;
	virtual int NotifyToAllSubscribers(void) = 0;
	virtual int NotifyToSpecificSubscriber(int) = 0;
	virtual int NotifyToProgressStatusToAllSubscribers(double) = 0;
	virtual int NotifyIOportsToAllSubscribers(int inport, int outport, int iSendingIoSignalType = 0) = 0;
};

