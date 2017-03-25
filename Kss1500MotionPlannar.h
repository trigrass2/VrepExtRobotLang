#pragma once

#include "VrepExtExport.h"
#include "Kss1500Compiler.h"

class VREP_ROBOTLANG_API CKss1500MotionPlannar : public CKss1500Interprerter
{
private:
	
	//	Motion information
	float  _fEuclidVelocity;
	int _currentProgramLine;				// The current program counter (number is relative to the compiled program)
	int _previousProgramLine;				// The porevious program counter 
	bool m_bStepRun = false;				// For single step on/off. If it works.
	float _timeAlreadySpentAtCurrentProgramLine;	//	Total running time.
	bool _bStopAtMotion;
	bool _bCalRelativeValueForDrive;
    float _fTarJointValByRelJointVal;

public:
	int RunProgram(float deltaTime = 0xffff);
    const CCompiledProgramLine* GetCurrentCompiledProgramLine(void);		//	Get compiled line at current line position
	const CCompiledProgramLine* GetPreviousCompiledProgramLine(void);		//	Get compiled line at previous line position 
	void SetStepRunCheck(bool sw) { m_bStepRun = sw; }						//	Set single block on/off
	bool GetStepRunStatus(void) { return m_bStepRun; }						//	Get single block status
    int  GetRobotRunStatus(void);                                           //	Get interpreter status. it determines when it go through the rotine with it.
    void SetRobotRunMode(int);                                              //  Set interpreter status. inhereied from pure virual function.
	void SetCurrentLineNumber(int);											//	Set current position number (complied positoin)
	int GetCurrentLineNumber(void);											//	Get current position number (complied positoin)
	int GetPreviousLineNumber(void);										//	Get previous position number (complied positoin)
	void SetPreviousLineNumber(int lineNO);									//	Set previous position number (complied positoin)
	int GetTotalLineNumber(void);											//	Get total number of compiled line.
	void SetStopAtMotionSwitch(bool);										//	Set a switch to stop at motion
	bool GetStopAtMotionStatus(void);										//	Get a switch status for stopping at motion 
    void SetCommModules(ICommSubject* modules) { _commModules = modules; }  //	Register a observer to the subsriber list
    ICommSubject* GetCommModules(void) const { return _commModules; }       //	Get a observer handle from the subsriber list
    void Reset(void);														//	Reset internal data
    void SynchronizeCurrToTarget(void);                                     //  Set all current data to target data to communicate by CubicNote 1st version.
    bool MakeOnelineProgramforCommandMode(const std::string&);              //  Set all input command data to target command data to communicate by CubicNote 2nd version.
    bool SetCustomMotionData(std::string& data);
    float GetEuclidSpeed(void)   {return _fEuclidVelocity;}                 // Get feedrate of system
    void SetEuclidSpeed(float speed)   {_fEuclidVelocity = speed;}          // Set feedrate of system
    int GetInputPort(void) { return GetInputMemory(); }
    int GetOutputPort(void) { return GetOutputMemory(); }
    virtual int GetSendingIoSignalType(void);
	void ResetRelativeJointValue(void);
    bool GetCalRelativeValueForDriveCmdStatus(void) {return _bCalRelativeValueForDrive;}
    void SetCalRelativeValueForDriveCmdStatus(bool sw) {_bCalRelativeValueForDrive = sw;}
    void SetRelTargetJointVal (float val) {_fTarJointValByRelJointVal = val;}
    float GetRelTargetJointVal (void) {return _fTarJointValByRelJointVal;}
    CKss1500MotionPlannar();
	~CKss1500MotionPlannar();
};

