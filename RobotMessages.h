#pragma once
#include <map>
#include <vector>

#include "VrepExtExport.h"

class VREP_ROBOTLANG_API CRobotMessages
{
public:
	
	enum ECompileError {
		ERR_LABEL_DEFAULT = 0,
		ERR_DELAY, ERR_GOTO, ERR_LINE_COMMENT, ERR_GOHOME,
		ERR_SPEED, ERR_ROTATE, ERR_GRASP, ERR_RELEASE, ERR_CHANGE,
		ERR_DRIVE, ERR_MOVE, ERR_POSEOPR, ERR_VAR, ERR_DIM, ERR_STOP, ERR_END,
		ERR_GOSUB, ERR_RETURN, ERR_IF, ERR_FOR, ERR_NEXT,
		ERR_WAIT, ERR_IN, ERR_OUT, ERR_SET, ERR_RESET,
		ERR_READY, ERR_PRESET, ERR_MOVE2, ERR_SWITCH, ERR_BREAK,
		ERR_CASE, ERR_DEFAULT, ERR_ENDSWITCH, ERR_ELSE,
		ERR_ENDIF, ERR_POSE, ERR_INVALD_MOVEPARAM, ERR_LET,
		ERR_LABEL, ERR_DOUBLE_LABLE, ERR_MATHOPR, ERR_LOGICOPR, ERR_RELAOPR, 
		ERR_LABELASSIGN, ERR_INVALID_MBRACE, ERR_INVALID_LBRACE, ERR_MBRACE_NO, ERR_LBRACE_NO,
		ERR_SPECIAL_CHARACTER, ERR_UNSIGNED_LABEL, ERR_MISMATCH_NOOF_IFENDIF, ERR_ELSEIF_AFTER_ELSE, ERR_INVALID_SBRACE,
		ERR_DEFFN, ERR_FN, ERR_DIM_ALLOCATIONFAILED, ERR_CASE_NOSWITCH, ERR_POSE_NOT_H, 
		ERR_POSE_NOT_T, ERR_LABEL_NODEFINE, ERR_DEFPOS, ERR_DIMINIT_NOVALUE_WRONG, ERR_NOVALUE_WITHINBRACE,
		ERR_ZERO_INDIM,
		ERR_POSE_LABEL, ERR_INVALID_KINEMATIC_RNG, ERR_BLANK, ERR_VALUE,
	};

	//	Program flow control
	enum eRunTimeReturnCode {
		PROGRAM_TERMINATED = 0,
		PROGRAM_NORMALLY_PROGRESSING = 1,
		ABNORMAL_RUNTIME_ERROR,
		KINEMATIC_ERROR,
		INVALID_ROTATION_RANGE,
		INVALID_DRIVE_VALUE,
		INVALUD_SPEED_RANGE,
		INVALUD_GRABBING_RANGE,
		NUMERICAL_CALCULATE_ERROR,
		UNLIMITED_INTERPOLATION,
		INVALED_LET_USE,
		WRONG_ALLOCATION,
		LINE_AT_PROGRAMEND
	};
	
	// Warning ID
	enum EWarningID
	{
		WRN_NO_VAR_INIT = 0,
		WRN_INVALID_IO_USE,
		WRN_FAIL_TO_RELOP,
		WRN_FAIL_TO_MATHOP,
		WRN_FAIL_TO_LOGICOP,
		WRN_ALLOCATE_FAIL,
		WRN_FUNCTIONCALL_FAIL,
		WRN_DIMINIT_WRONG,
		WRN_OVERFLOW_INT,
		WRN_OVERFLOW_UINT,
		WRN_OVERFLOW_FLOAT,
		WRN_OVERFLOW_DIM,
		WRN_DOUBLE_LABEL,
	};

	std::vector<std::string> _errorRuntimeMessage;
	std::vector<std::string> _errorCompileMessage;
	std::vector<std::string> _warningMessage;
	std::map<int, CRobotMessages::ECompileError> _compileErrorCode;
	std::map<int, CRobotMessages::EWarningID> _compileWarningCode;

	void InitializeMessageMapping(void);
	virtual std::string GetCompileTimeErrorMessage(ECompileError);
	virtual std::string GetWarningMessage(EWarningID);
	virtual std::string GetRunTimeErrorMessage(eRunTimeReturnCode);
	CRobotMessages();
    ~CRobotMessages();
};
