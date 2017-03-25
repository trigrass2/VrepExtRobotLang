#pragma once

#ifndef QT_COMPIL
    #include "stdafx.h"
#else
    #include <QtCore>
    #include <QtXml/QtXml>
    #include <QDebug>
#endif

#include "VrepExtExport.h"
#include <string>
#include "CommonH.h"

using namespace std;

class VREP_ROBOTLANG_API CRobotLangParserAndOperator
{
//	Pose data group
public:

	// Variable ID
	typedef enum class eVartypeID
	{
		NONE = -1, VAR_FLOAT = 0, VAR_UINT, VAR_INT, VAR_POSE,
	} VAR_TYPE;

	typedef enum class eAddress
	{
		GENERAL_VAR = 0, ARRAY_VAR, PORT_IO, FLOAT_VALUE, 
	} ADDRESS;

	typedef struct SPoseData {
		float _Data[TOTAXES] = { 0.0f, };

        bool operator ==(const SPoseData& Joints) const
        {
            return (Joints._Data[0] == this->_Data[0] && Joints._Data[1] == this->_Data[1] &&Joints._Data[2] == this->_Data[2]
                    && Joints._Data[3] == this->_Data[3] && Joints._Data[4] == this->_Data[4]);
        }

	} POSE;

	typedef class SVarData {
		public:
			float _Value = 0.0f;
			eVartypeID _VarType = eVartypeID::VAR_FLOAT;
		public:		
			int SetValue(float rValue);	//	i<0 under flow, 0 = normal, 1>overflow
			float GetValue(void);
			void SetType(eVartypeID);
			eVartypeID GetType(void);
	} VARIABLE;


	typedef class CFunctionDefinition 	{
	public:
		VARIABLE _returnValue;
		std::string _name;
		std::string _functor;
		std::map<std::string, VARIABLE> _argumentVariable;

		CFunctionDefinition();
		~CFunctionDefinition();
		void Init(void);
		int MakeArguments(const std::vector<std::string>& args);
		void SetFunctor(const std::string& contents) { _functor = contents; }
	} DEFFUNC;

	typedef std::vector <std::vector < std::vector<SVarData>>> DimVarElements;
	typedef std::vector <std::vector < std::vector<POSE>>> DimPoseElements;

	std::map<std::string, DEFFUNC> _internalFunction;
	std::map<std::string, VARIABLE> _internalVariable;
	std::map<std::string, DimPoseElements> _internalPoseArray;
	std::map<std::string, DimVarElements> _internalArray;
	std::vector<int> _labelLocations;
	std::vector<std::string> _labels;

	std::vector<POSE> _userPoseData;
    std::string _userPoseDataFilePath;

	//	IO data group
private:
	int _nInputM, _nOutputM; 
	bool _bLockforSignalUpdate;
	bool _bSendingIoSignalBySerialPort;
	int _iSendingIoSignalType;
	bool _bGetDataThroughExternalDevice;

public:
	//	I/O Set
	void SetInputMemory(int i) { _nInputM = i; }
	int GetInputMemory(void) { return _nInputM; }
	void SetOutputMemory(int i) { _nOutputM = i; }
	int GetOutputMemory(void) { return _nOutputM; }
	int& GetRefInputMememory(void) { return _nInputM; }
	int& GetRefOutputMememory(void) { return _nOutputM; }
	bool GetLockSignalDataUpdateStatus(void) { return _bLockforSignalUpdate; }
	void LockSignalDataUpdate(bool status) { _bLockforSignalUpdate = status; }
	void SetSendingIoSignalToSerialPort(bool status) { _bSendingIoSignalBySerialPort = status; }
	bool GetSendingIoSignalToSerialPortStatus(void) { return _bSendingIoSignalBySerialPort; }
	void SetDataFromExternalDevice(bool sw);		  //	Get data like IO passing through serial comm.
	bool IsDataPassingByExternalDevice(void);		  //	Get status to be turn or not pass-through switch.
	void SetSendingIoSignalType(int type) { _iSendingIoSignalType = type; }
	bool UpdateIOport(int input, int output, int ChangedOutput = 0);
	bool UpdateInPort(int input);

public: 
	//	Parsing 
	bool ExtractOneLine(std::string& inputString, std::string& extractedLine);
	bool ExtractWordRelationOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord);
	bool ExtractWordMathOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord);
	bool ExtractWordLogicOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord);
	bool ExtractWordPoseOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord);
	bool ExtractMultiWord(std::string& str, vector<string>& extracedWords);
	int ExactractPoseData(const std::string&, std::string &strStatement, CCompiledProgramLine&);
	bool ExtractDimDefine(std::string& strVardef, VAR_TYPE &iVarType, int &iWarnMessage, std::string &lExtWord, vector<int>& dimSizes, std::string &rExtWord);
	bool ExtractDeposDefine(std::string& strVardef, VAR_TYPE &iVarType, int &iWarnMessage, std::string &lExtWord, std::string &rExtWord, vector<int>& dimSizes, vector<float>& initValues);
	bool ExtractValuesFromBigBraces(std::string& strDim, vector<int>& extArgs);	//	[][]
	bool ExtractValuesFromMidBraces(std::string& strDim, vector<float>& extArgs);	//	{}{} for function define
	bool ExtractOneMoveData(std::string &lExtWord, std::string &rExtWord);
	bool GetCommandFromWord(const std::string& word, int& command);
	bool RemoveFrontAndBackSpaces(std::string& word, bool bCommentLine = false);
	int  GetCountComma(std::string line);
	bool ExtractOneWord(std::string& line, std::string& extractedWord, const char* opr = " ");
	bool ExtractOneAddress(std::string& line, std::string& extractedWord, const char* opr = " ");
	bool ExtractXyzPosition(std::string &strStatement);
	bool ExtractXyzrgPosition(std::string &strStatement, float val[]);
	bool ExtractVarDefine(std::string& strVardef, VAR_TYPE &iVarType, int &iWarnMessage, std::string &lExtWord, std::string &rExtWord);

public:	
	//	Properties	
	unsigned int GetAxisNumberByAxisName(const std::string&);
    virtual int GetSendingIoSignalType(void) { return _iSendingIoSignalType; }
	bool GetDataFromMap(const std::string& variable, float &value);
	bool SetDataToMap(const std::string& variable, float &value);
	bool SetDataToArray(const std::string& variable, int depth, int height, int width, const float& value, int& iWarnMessage);
	bool GetDataFromArray(const std::string& variable, int depth, int height, int width, float& value, int& iWarnMessage);
	bool GetDataFromArray(const std::string& variable, float& value, int& iWarnMessage);
	bool SetDataToArray(const std::string& variable, float value, int& iWarnMessage);
	bool GetDataFromPose(const std::string& variable, float* value, int& iWarnMessage);
	bool GetDataFromPose(const std::string& variable, int depth, int height, int width, float* value, int& iWarnMessage);
	bool SetDataToPose(const std::string& variable, int depth, int height, int width, const float* value, int& iWarnMessage);
	bool SetDataToPose(const std::string& variable, const float* value, int& iWarnMessage);
	bool GetDataFromUserPose(const std::string& variable, float fPos[]);
	bool SetDataToUserPose(const std::string& strVariable, float *fPos);
	bool GetDataFromFunction(const std::string& variable, float& value, int& iWarnMessage);
	bool InitInternalVarDimension(const std::string& arryName, float value);
	bool InitInternalPoseDimension(const std::string& arryName, const std::vector<float>& value);
	bool GetValueAtOneWord(const std::string& variable, float& value, ADDRESS& addType, int& iWarnMessage);
	bool GetPoseAtOneWord(const std::string& variable, float values[], int& iWarnMessage);
    void SetUserPoseDataValue(int no, int axisNo, float data);
    float GetUserPoseDataValue(int no, int axisNo);

	// Operation
	bool IsInternalVariable(const std::string& variable);
	bool IsInternalArray(const std::string& variable);
	bool IsInternalUserdefPose(const std::string& variable);
	bool IsInternalPose(const std::string& variable);
	bool IsInternalPorts(const std::string& variable);
	bool IsInternalFunction(const std::string& variable);
	bool GetErrorLogFromMap(int& variable, int &value);
	bool GetFirstError(int& variable, int &value);
	bool GetDataFromIO(const char*, int& value);
	void SetDataToOut(const char*, int value);
	void SetDataToIn(const char*, int value);
	bool GetIntegerFromWord(const std::string& word, int& theInteger);
	bool GetFloatFromWord(const std::string& word, float& theFloat);
	bool IsChainMotionProgram(std::string &strStatement, int& nMoveCount, int& nPoseInMoveCount);
	bool GenerateSerialMotion(std::string &strStatement, CCompiledProgramLine&);
	VAR_TYPE GetVariableTypeAndTrimSymbol(std::string *, int& iWarnMessage);
	int DoRelationalOperation(std::string &strStatement, int& iWarnMessage);
	int DoLogicalOperation(std::string &strStatement, int& iWarnMessage);
	int DoMathOperation(std::string &strStatement, int& iWarnMessage);
	int DoPoseOperation(const CCompiledProgramLine& currProgramLine, float fTargValue[], std::string& rtnPose, const std::vector<float>& currPos, const float& currGrpPos);
	bool AllocateInternalVarDimension(const std::string& arryName, vector<int>& extArgs);
	bool AllocateInternalPoseDimension(const std::string& arryName, vector<int>& extArgs);
	void GetCurrentRobotMotionInfo(const CCompiledProgramLine& currProgrram, std::vector<float>& currPos, float& currGrpPos, bool UpdateSavedPosALpha = false);
	void GetListofVariable(char[]);
	void GetListofArray(char[]);
	void GetListofLabels(char[]);
	void GetListofPose(char[]);
#ifdef QT_COMPIL
    bool WriteUserPoseToDisk(QFile& file, std::vector<POSE>* userPoseData = nullptr);
    bool ReadUserPoseFromDisk(QFile& file, std::vector<POSE>* userPoseData = nullptr);
    void ListElements(QDomElement root, QString tagname, QString attribute, std::vector<POSE>& userPoseData);
#else
    bool WriteUserPoseToDisk(std::string filename = "TPSaveData.ini");
    bool ReadUserPoseFromDisk(std::string filename = "TPSaveData.ini");
#endif
	//	Construction for loading user pose data and destruction for saving user pose data 
	CRobotLangParserAndOperator();
	~CRobotLangParserAndOperator();

};

