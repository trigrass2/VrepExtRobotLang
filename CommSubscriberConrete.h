#pragma once
#include <vector>
#include <map>
#include "ICommSubscribers.h"

//	Concrete Subscriber
class CCommSubjectConrete : public ICommSubject
{
private:
	std::vector<ICommObserver*> _CommSubscribers;
	std::map<std::string, int> _ObserverID;

public:
	int GetID(void);
	const std::string& GetName(void);
	void SetID(int);
	int Update(void);
	int Update(int& INport, int& OUTport, int ChangedOutput = 0);
	int Notify(CMD, float* fData = nullptr, int* iData = nullptr, void* UserDefData = nullptr);
	int GetData(CMD);
	int NotifyToAllSubscribers(void);			// Notify all message to subsribers in case of STEPRUN
	int NotifyToProgressStatusToAllSubscribers(double);// Notify all IO informations to subsribers in case of STEPRUN	
	int NotifyIOportsToAllSubscribers(int input, int output, int ChangedOutputType = 0);// Notify all IO informations to subsribers in case of STEPRUN	
	int NotifyToSpecificSubscriber(int);		// Notify message to specific subsribers in case of MSTATUS_RUN
	
	// Notify all message to subsribers in case of STEPRUN
	int NotifyToSpecificSubscriber(std::string observerName, CMD, int* iData = nullptr, float* fData = nullptr, void* UserDefData = nullptr);
	int Attach(ICommObserver*);					//	Add to observer to subscriber
	int Detach(ICommObserver*);					//	Detatch observer from subscriber
	int DetachAll(void);						//	Detatch all observer from subscriber
	int FindObjectID(std::string);				//	Return observer ID to get a observer handle
        CCommSubjectConrete();
        ~CCommSubjectConrete();
};

