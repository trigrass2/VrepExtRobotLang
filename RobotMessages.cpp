#include "RobotMessages.h"
#include "StringTableEng.h"

typedef CRobotMessages::eRunTimeReturnCode RTN_CODE;
typedef CRobotMessages::ECompileError CERR;
typedef CRobotMessages::EWarningID WRN;

typedef std::pair<CRobotMessages::eRunTimeReturnCode, std::string> pairRtnMessage;
typedef std::pair<CRobotMessages::ECompileError, std::string> pairCmpMessage;

CRobotMessages::CRobotMessages()
{
	_errorCompileMessage.resize(CERR::ERR_VALUE+1);
	_errorRuntimeMessage.resize(RTN_CODE::LINE_AT_PROGRAMEND+1);
	_warningMessage.resize(WRN::WRN_DOUBLE_LABEL+1);

	InitializeMessageMapping();
}

CRobotMessages::~CRobotMessages()
{
}

void CRobotMessages::InitializeMessageMapping(void)
{
	_warningMessage[WRN::WRN_NO_VAR_INIT] = WARN_01;
	_warningMessage[WRN::WRN_DOUBLE_LABEL] = WARN_02;
	_warningMessage[WRN::WRN_INVALID_IO_USE] = WARN_03;
	_warningMessage[WRN::WRN_FAIL_TO_MATHOP] = WARN_04;
	_warningMessage[WRN::WRN_FAIL_TO_RELOP] = WARN_05;
	_warningMessage[WRN::WRN_FAIL_TO_LOGICOP] = WARN_06;
	_warningMessage[WRN::WRN_ALLOCATE_FAIL] = WARN_07;
	_warningMessage[WRN::WRN_FUNCTIONCALL_FAIL] = WARN_08;
	_warningMessage[WRN::WRN_DIMINIT_WRONG] = WARN_09;
	_warningMessage[WRN::WRN_OVERFLOW_INT] = WARN_10;
	_warningMessage[WRN::WRN_OVERFLOW_UINT] = WARN_11;
	_warningMessage[WRN::WRN_OVERFLOW_FLOAT] = WARN_12;
	_warningMessage[WRN::WRN_OVERFLOW_DIM] = WARN_13;
		
	_errorRuntimeMessage[RTN_CODE::LINE_AT_PROGRAMEND] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::ABNORMAL_RUNTIME_ERROR] = RTERR_07;
	_errorRuntimeMessage[RTN_CODE::PROGRAM_TERMINATED] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::PROGRAM_NORMALLY_PROGRESSING] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::KINEMATIC_ERROR] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::INVALID_ROTATION_RANGE] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::INVALUD_SPEED_RANGE] = RTERR_02;
	_errorRuntimeMessage[RTN_CODE::INVALUD_GRABBING_RANGE] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::NUMERICAL_CALCULATE_ERROR] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::UNLIMITED_INTERPOLATION] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::INVALED_LET_USE] = RTERR_08;
	_errorRuntimeMessage[RTN_CODE::WRONG_ALLOCATION] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::LINE_AT_PROGRAMEND] = RTERR_01;
	_errorRuntimeMessage[RTN_CODE::INVALID_DRIVE_VALUE] = RTERR_06;

	_errorCompileMessage[CERR::ERR_LABELASSIGN] = CTERR_01;
	_errorCompileMessage[CERR::ERR_LABEL_DEFAULT] = CTERR_09;
	_errorCompileMessage[CERR::ERR_DELAY] = CTERR_35;
	_errorCompileMessage[CERR::ERR_GOTO] = CTERR_06;
	_errorCompileMessage[CERR::ERR_LINE_COMMENT] = CTERR_09;
	_errorCompileMessage[CERR::ERR_GOHOME] = CTERR_36;
	_errorCompileMessage[CERR::ERR_SPEED] = CTERR_37;
	_errorCompileMessage[CERR::ERR_ROTATE] = CTERR_12;
	_errorCompileMessage[CERR::ERR_GRASP] = CTERR_11;
	_errorCompileMessage[CERR::ERR_RELEASE] = CTERR_10;
	_errorCompileMessage[CERR::ERR_CHANGE] = CTERR_09;
	_errorCompileMessage[CERR::ERR_DRIVE] = CTERR_39;
	_errorCompileMessage[CERR::ERR_MOVE] = CTERR_38;
	_errorCompileMessage[CERR::ERR_POSEOPR] = CTERR_40;
	_errorCompileMessage[CERR::ERR_VAR] = CTERR_09;
	_errorCompileMessage[CERR::ERR_DIM] = CTERR_15;
	_errorCompileMessage[CERR::ERR_END] = CTERR_09;
	_errorCompileMessage[CERR::ERR_GOSUB] = RTERR_09;
	_errorCompileMessage[CERR::ERR_RETURN] = CTERR_09;
	_errorCompileMessage[CERR::ERR_IF] = CTERR_22;
	_errorCompileMessage[CERR::ERR_FOR] = CTERR_21;
	_errorCompileMessage[CERR::ERR_NEXT] = CTERR_09;
	_errorCompileMessage[CERR::ERR_WAIT] = CTERR_19;
	_errorCompileMessage[CERR::ERR_IN] = CTERR_09;
	_errorCompileMessage[CERR::ERR_OUT] = CTERR_09;
	_errorCompileMessage[CERR::ERR_SET] = CTERR_20;
	_errorCompileMessage[CERR::ERR_RESET] = CTERR_09;
	_errorCompileMessage[CERR::ERR_READY] = CTERR_09;
	_errorCompileMessage[CERR::ERR_PRESET] = CTERR_09;
	_errorCompileMessage[CERR::ERR_MOVE2] = CTERR_09;
	_errorCompileMessage[CERR::ERR_SWITCH] = CTERR_23;
	_errorCompileMessage[CERR::ERR_BREAK] = CTERR_26;
	_errorCompileMessage[CERR::ERR_CASE] = CTERR_24;
	_errorCompileMessage[CERR::ERR_CASE_NOSWITCH] = CTERR_25;
	_errorCompileMessage[CERR::ERR_DEFAULT] = CTERR_09;
	_errorCompileMessage[CERR::ERR_ENDSWITCH] = CTERR_09;
	_errorCompileMessage[CERR::ERR_ELSE] = CTERR_09;
	_errorCompileMessage[CERR::ERR_ENDIF] = CTERR_18;
	_errorCompileMessage[CERR::ERR_POSE] = CTERR_09;
	_errorCompileMessage[CERR::ERR_INVALD_MOVEPARAM] = CTERR_09;
	_errorCompileMessage[CERR::ERR_LABEL ] = CTERR_09;
	_errorCompileMessage[CERR::ERR_DOUBLE_LABLE] = CTERR_09;
	_errorCompileMessage[CERR::ERR_MATHOPR] = CTERR_28;
	_errorCompileMessage[CERR::ERR_LOGICOPR] = CTERR_30;
	_errorCompileMessage[CERR::ERR_RELAOPR] = CTERR_29;
	_errorCompileMessage[CERR::ERR_POSE_LABEL] = CTERR_09;
	_errorCompileMessage[CERR::ERR_INVALID_KINEMATIC_RNG] = CTERR_09;
	_errorCompileMessage[CERR::ERR_BLANK] = CTERR_09;
	_errorCompileMessage[CERR::ERR_VALUE] = CTERR_09;
	_errorCompileMessage[CERR::ERR_INVALID_MBRACE] = CTERR_02;
	_errorCompileMessage[CERR::ERR_INVALID_LBRACE] = CTERR_03;
	_errorCompileMessage[CERR::ERR_MBRACE_NO] = CTERR_02;
	_errorCompileMessage[CERR::ERR_LBRACE_NO] = CTERR_03;
	_errorCompileMessage[CERR::ERR_MISMATCH_NOOF_IFENDIF] = CTERR_07;
	_errorCompileMessage[CERR::ERR_SPECIAL_CHARACTER] = CTERR_05;
	_errorCompileMessage[CERR::ERR_ELSEIF_AFTER_ELSE] = CTERR_08;
	_errorCompileMessage[CERR::ERR_DEFFN] = CTERR_13;
	_errorCompileMessage[CERR::ERR_FN] = CTERR_13;
	_errorCompileMessage[CERR::ERR_DIM_ALLOCATIONFAILED] = CTERR_16;
	_errorCompileMessage[CERR::ERR_STOP] = CTERR_17; 
	_errorCompileMessage[CERR::ERR_UNSIGNED_LABEL] = CTERR_27;
	_errorCompileMessage[CERR::ERR_INVALID_SBRACE] = CTERR_31;
	_errorCompileMessage[CERR::ERR_POSE_NOT_H] = CTERR_32;
	_errorCompileMessage[CERR::ERR_POSE_NOT_T] = CTERR_33;
	_errorCompileMessage[CERR::ERR_LABEL_NODEFINE] = CTERR_34;
	_errorCompileMessage[CERR::ERR_DEFPOS] = CTERR_41;
	_errorCompileMessage[CERR::ERR_DIMINIT_NOVALUE_WRONG] = CTERR_42;
	_errorCompileMessage[CERR::ERR_NOVALUE_WITHINBRACE] = CTERR_43;
	_errorCompileMessage[CERR::ERR_ZERO_INDIM] = CTERR_44;
} 

std::string CRobotMessages::GetCompileTimeErrorMessage(CERR err)
{
	return _errorCompileMessage[err];
}

std::string CRobotMessages::GetRunTimeErrorMessage(eRunTimeReturnCode err)
{
	return _errorRuntimeMessage[err];
}

std::string CRobotMessages::GetWarningMessage(EWarningID warn)
{
	return _warningMessage[warn];
}