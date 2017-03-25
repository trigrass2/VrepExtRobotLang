#include <typeinfo>

#include "CommSubscriberConrete.h"

CCommSubjectConrete::CCommSubjectConrete()
{

}

CCommSubjectConrete::~CCommSubjectConrete()
{
}

int CCommSubjectConrete::GetID(void)
{
	return 1;
}

const std::string& CCommSubjectConrete::GetName(void)
{
	return nullptr;
}

void CCommSubjectConrete::SetID(int IDvalue)
{

}

int CCommSubjectConrete::Update(void)
{
	return 1;
}

int CCommSubjectConrete::Update(int& INport, int& OUTport, int ChangedOutput)
{
	return 1;
}

int CCommSubjectConrete::Notify(CMD, float* fData, int* iData, void* UserDefData)
{
	return 1;
}

int CCommSubjectConrete::GetData(CMD)
{
	return 1;
}

int CCommSubjectConrete::NotifyIOportsToAllSubscribers(int INport, int Outport, int ChangedOutputType)
{
#ifndef QT_COMPIL
	for each (auto comm in _CommSubscribers)
#endif

            for (auto const &comm : _CommSubscribers)
		{
			(ICommObserver*)comm->Update(INport, Outport, ChangedOutputType);
		}
	return _CommSubscribers.size();
}

int CCommSubjectConrete::NotifyToAllSubscribers(void)
{
#ifndef QT_COMPIL
	for each (auto comm in _CommSubscribers)
#endif
    for(auto const &comm : _CommSubscribers)
	{
		(ICommObserver*) comm->Update();
	}
	return _CommSubscribers.size();
}

int CCommSubjectConrete::NotifyToProgressStatusToAllSubscribers(double progressPercent)
{
#ifndef QT_COMPIL
	for each (auto comm in _CommSubscribers)
#endif
		for (auto const &comm : _CommSubscribers)
		{
			(ICommObserver*)comm->UpdateProgressStatus(progressPercent);
		}
	return _CommSubscribers.size();
}

int CCommSubjectConrete::FindObjectID(std::string strObject)
{
	int commID = 0;

#ifndef QT_COMPIL
    for each (auto comm in _CommSubscribers)
#endif
    for(auto const &comm : _CommSubscribers)
	{
		if ((ICommObserver*)comm->GetName().compare(strObject) == 0)
		{
			return comm->GetID();
		}
	}
}

int CCommSubjectConrete::NotifyToSpecificSubscriber(int i)
{
	int commID = 0;

#ifndef QT_COMPIL
    for each (auto comm in _CommSubscribers)
#endif
    for(auto const &comm : _CommSubscribers)
	{
		if ((int) (ICommObserver*)comm->GetID() == i)
		{
			comm->Update();
			return 1;
		}
	}
	return 0;
}

int CCommSubjectConrete::Attach(ICommObserver* observer)
{
	if (observer == nullptr)
		return -1;
	
	observer->SetID(_CommSubscribers.size());
	_CommSubscribers.push_back(observer);

	return 1;
}

int CCommSubjectConrete::Detach(ICommObserver* observer)
{
#ifndef QT_COMPIL
    for each (auto comm in _CommSubscribers)
#endif
    for(auto const &comm : _CommSubscribers)
	{
		if ((ICommObserver*)comm == observer)
		{
			delete comm;
			return 1;
		}
	}
	return 0;
}

int CCommSubjectConrete::DetachAll(void)
{
	int a = _CommSubscribers.size();
	_CommSubscribers.clear();
	return a;
}
