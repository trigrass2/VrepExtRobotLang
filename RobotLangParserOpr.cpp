#include "RobotLangParserOpr.h"
#ifndef WIN_VREP
#include <boost/lexical_cast.hpp>
#else
#include <boost\lexical_cast.hpp>
#endif
#include <string>
#include <utility>
#include <cctype>
#include <clocale>
#include <algorithm>
#include <cstring>
#include <limits>
#include <float.h>
#include "RobotMessages.h"

#ifdef QT_COMPIL
#include <QtCore>
#include <QtXml/QtXml>
#include <QDebug>
#else
#include <intsafe.h>
#endif

typedef CCompiledProgramLine::PGM_CMD CMD;
typedef CRobotLangParserAndOperator::VAR_TYPE VAR;
typedef CRobotMessages::EWarningID WRN;
typedef CRobotMessages::eRunTimeReturnCode RTNCODE;
typedef std::pair<std::string, float> pairData;
typedef std::pair<int, CRobotMessages::ECompileError> errorData;
typedef std::pair<int, CRobotMessages::EWarningID> warningData;

CRobotLangParserAndOperator::CFunctionDefinition::CFunctionDefinition()
{

}

CRobotLangParserAndOperator::CFunctionDefinition::~CFunctionDefinition()
{

}

void CRobotLangParserAndOperator::CFunctionDefinition::Init()
{
    _returnValue._Value = UNCONTROLLABLE_VALUE;
    _name.clear();
    _functor.clear();
    _argumentVariable.clear();
}

/*
int CRobotLangParserAndOperator::CFunctionDefinition::MakeArguments(const std::vector<std::string>& args)
{

}
*/


CRobotLangParserAndOperator::CRobotLangParserAndOperator()
{
    _nInputM = 0;
    _nOutputM = 0;
    _iSendingIoSignalType = 0;
    _bGetDataThroughExternalDevice = false;
    _bLockforSignalUpdate = false;
    _bSendingIoSignalBySerialPort = true;
    _iSendingIoSignalType = 0;
    _userPoseData.resize(MAX_POINT_NUM);
#ifndef QT_COMPIL
    _userPoseDataFilePath = "TPSaveData.ini";
    ReadUserPoseFromDisk(_userPoseDataFilePath);
#else
    _userPoseDataFilePath = "TPSaveData.xml";
    QFile file(_userPoseDataFilePath.c_str());
    ReadUserPoseFromDisk(file);
#endif
}


CRobotLangParserAndOperator::~CRobotLangParserAndOperator()
{

#ifndef QT_COMPIL
    _userPoseDataFilePath = "TPSaveData.ini";
    WriteUserPoseToDisk(_userPoseDataFilePath);
#else
    _userPoseDataFilePath = "TPSaveData.xml";
    QFile file(_userPoseDataFilePath.c_str());
    WriteUserPoseToDisk(file);
#endif
}


int CRobotLangParserAndOperator::SVarData::SetValue(float rValue) {
    int iRtnValue = 0;

    if (rValue == UNCONTROLLABLE_VALUE)
        return 0;

    switch (this->_VarType)
    {
    case CRobotLangParserAndOperator::eVartypeID::NONE:
        this->_Value = 0;
        break;
    case CRobotLangParserAndOperator::eVartypeID::VAR_FLOAT:
        this->_Value = (float)rValue;
        if ((long)(rValue/1000000) > 0)
            iRtnValue = 1;
        break;
    case CRobotLangParserAndOperator::eVartypeID::VAR_UINT:
        this->_Value = (float) ((unsigned int)rValue);
        if (0 > rValue)
            iRtnValue = -1;
        else if (INT_MAX  < rValue)
            iRtnValue = 1;
        break;

    case CRobotLangParserAndOperator::eVartypeID::VAR_INT:
        this->_Value = (float) ((int)rValue);
        if (INT_MIN > rValue)
            iRtnValue = -1;
        else if (INT_MAX  < rValue)
            iRtnValue = 1;
        break;
    case CRobotLangParserAndOperator::eVartypeID::VAR_POSE:
        return false;
        break;
    default:
        return false;
        break;
    }
    return iRtnValue;
}


float CRobotLangParserAndOperator::SVarData::GetValue(void)
{
    return this->_Value;
}

void CRobotLangParserAndOperator::SVarData::SetType(eVartypeID eType)
{
    this->_VarType = eType;
}


CRobotLangParserAndOperator::eVartypeID CRobotLangParserAndOperator::SVarData::GetType(void)
{
    return this->_VarType;
}

bool CRobotLangParserAndOperator::ExtractOneLine(std::string& inputString, std::string& extractedLine)
{ // Extracts one line from a multi-line string
    extractedLine = "";
    while (inputString.length() != 0)
    {
        char c = inputString[0];
        inputString.erase(inputString.begin());
        if ((c == char(13)) || (c == char(10)))
        {
            if (c == char(10))
                return(true);
            //				break;
        }
        else
            extractedLine += c;
    }
    return(false);// no more code there! (but the "extractedLine" might be non-empty!)     extractedLine.length()!=0);
}

void CRobotLangParserAndOperator::SetUserPoseDataValue(int no, int axisNo, float data)
{
    if(no >= _userPoseData.size()||axisNo >= TOTAXES)
        return;
    else
        _userPoseData[no]._Data[axisNo] = data;
}
float CRobotLangParserAndOperator::GetUserPoseDataValue(int no, int axisNo)
{

    if(no >= _userPoseData.size()||axisNo >= TOTAXES)
        return UNCONTROLLABLE_VALUE;
    else
        return _userPoseData[no]._Data[axisNo];
}

bool CRobotLangParserAndOperator::ExtractOneAddress(std::string& line, std::string& extractedWord, const char* opr)
{
    extractedWord = "";

    while (line.length() != 0)
    {
        char c = line[0];

        line.erase(line.begin());

        if ((strrchr(opr, c) != NULL || c == ',' || c == '>' || c == '<' || c == '-' || c == '+' || c == '!' || c == '=' || c == '(' || c == ')'))
        {
            if (extractedWord.length() != 0)
            {
                if (c != ' ')
                    line =c + line;

                return(extractedWord.length() != 0);
            }
            else
            {
                //MOVE (-134.90,90,0,20,0), first character is minus
                if ((c == '-' || c == '+') && line.size() > 0)
                {
                    if(std::isalnum(line[1]))
                        extractedWord += c;
                }

            }
        }
        else
            extractedWord += c;
    }
    return(extractedWord.length() != 0);
}

bool CRobotLangParserAndOperator::ExtractOneWord(std::string& line, std::string& extractedWord, const char* opr)
{ // Extracts one word from a string (words are delimited by spaces)
    extractedWord = "";

    while (line.length() != 0)
    {
        char c = line[0];

        line.erase(line.begin());

        if ((strrchr(opr, c) != NULL || c == ',' || c == '>' || c == '<' || c == '!' || c == '=' || c == '(' || c == ')'))
        {
            if (extractedWord.length() != 0)
                return(extractedWord.length() != 0);
        }
        else
            extractedWord += c;
    }
    return(extractedWord.length() != 0);
}

void CRobotLangParserAndOperator::GetListofVariable(char variables[])
{
    std::string eachVariable;

    if (_internalVariable.size() > 0)
        strcpy(variables, "[VAR] ");

#ifndef QT_COMPIL
    for each (auto comm in _internalVariable)
#else
    for (auto comm : _internalVariable)
#endif
    {
        eachVariable = "(";

        switch ((eVartypeID) comm.second.GetType())
        {
        case eVartypeID::NONE:
            eachVariable = eachVariable + "-";
            break;
        case eVartypeID::VAR_FLOAT:
            eachVariable = eachVariable + "F";
            break;
        case eVartypeID::VAR_UINT:
            eachVariable = eachVariable + "U";
            break;
        case eVartypeID::VAR_INT:
            eachVariable = eachVariable + "I";
            break;
        case eVartypeID::VAR_POSE:
            eachVariable = eachVariable + "P";
            break;
        default:
            break;
        }

        eachVariable += ")";
        eachVariable = eachVariable + (std::string) comm.first;
        eachVariable += " ";
        strcat(variables, eachVariable.c_str());
        //variables.push_back(eachVariable);
    }
}

void CRobotLangParserAndOperator::GetListofArray(char variables[])
{
    std::string eachVariable;
    eVartypeID _VarType = eVartypeID::NONE;

    if (_internalArray.size() > 0)
        strcpy(variables, "[DIM] ");

#ifndef QT_COMPIL
    for each (auto comm in _internalArray)
#else
    for (auto const &comm : _internalArray)
#endif
    {
        int HEIGHT = comm.second.size();
        int WIDTH =  comm.second[0].size();
        int DEPTH = comm.second[0][0].size();;
        /*
        char strTmp [10] = "";

        eachVariable = eachVariable + (std::string) comm.first;
        eachVariable += "(";
        itoa(HEIGHT, strTmp, 10);
        eachVariable += strTmp;
        itoa(WIDTH, strTmp, 10);
        eachVariable += strTmp;
        itoa(DEPTH, strTmp, 10);
        eachVariable += strTmp;
        eachVariable += ")";
        */
        eachVariable = eachVariable + (std::string) comm.first;
        eachVariable += " ";
        strcat(variables, eachVariable.c_str());
        eachVariable.clear();
    }
}

void CRobotLangParserAndOperator::GetListofPose(char variables[])
{
    std::map<std::string, std::vector <POSE>> _poseVariable;
}

void CRobotLangParserAndOperator::GetListofLabels(char variables[])
{
    std::string eachVariable;
    eVartypeID _VarType = eVartypeID::NONE;
    char strLineNo[10];

    if (_labels.size() > 0)
        strcpy(variables, "[LABELS] ");

#ifndef QT_COMPIL
    for (auto i=0;i<(int)_labelLocations.size(); i++)
#else
    for (auto i = 0; i<_labelLocations.size(); i++)
#endif
    {
        eachVariable = eachVariable + (std::string) _labels[i];
        eachVariable += "(";


        #ifndef WIN_VREP
           sprintf(strLineNo,"%d",_labelLocations[i]);
        #else
          itoa(_labelLocations[i], strLineNo, 10);
        #endif

        eachVariable += strLineNo;
        eachVariable += ")";
        eachVariable += " ";
        strcat(variables, eachVariable.c_str());
    }
}


bool CRobotLangParserAndOperator::ExtractWordMathOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord)
{ // Extracts one word from a string (words are delimited by spaces)
    if (!lExtWord.empty()) lExtWord = "";
    if (!rExtWord.empty()) rExtWord = "";
    if (!extOpr.empty()) extOpr = "";
    if (!wholeWord.empty()) wholeWord.clear();

    int iPos = 0;
    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == ' ')
        {
            if (!lExtWord.empty())
            {
                wholeWord.push_back(c);
                continue;
            }

            lExtWord.clear();
            wholeWord.clear();
        }
        else if (c == '*' || c == '/' || c == '%')
        {
            extOpr += c;
            wholeWord.push_back(c);
            if (lExtWord.length() != 0)
            {
                break;
            }
        }
        else if (c == '+')
        {
            extOpr += c;
            wholeWord.push_back(c);
            if (lExtWord.length() != 0)
            {
                break;
            }
        }
        else if (c == '-' && iPos > 0)
        {
            extOpr += c;
            wholeWord.push_back(c);
            if (lExtWord.length() != 0)
            {
                break;
            }
        }
        else
        {
            lExtWord += c;
            wholeWord.push_back(c);
        }
        iPos++;
    }
    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == ' ' || c == ',' || c == '>' || c == '<' || c == '!' || c == '(' || c == ')' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/') // shseo - ',' '(' ')' ??? ???
        {
            if (rExtWord.length() != 0)
                break;
        }
        else
        {
            rExtWord += c;
        }
        wholeWord.push_back(c);
    }
    return(lExtWord.length() != 0 && rExtWord.length() != 0 && extOpr.length() != 0);

}
bool CRobotLangParserAndOperator::ExtractWordPoseOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord)
{
    if (!lExtWord.empty()) lExtWord = "";
    if (!rExtWord.empty()) rExtWord = "";
    if (!extOpr.empty()) extOpr = "";

    int iPos = 0;
    int iLBrace = line.find("(");
    int iRBrace = line.find(")");

    if (iLBrace == 0 && iRBrace == line.size() - 1)
    {
        line.erase(0, 1);
        line.erase(line.size() - 2, line.size() - 1);
    }

    //	1) MOVE P01 + (-30,20,20) ?? ??????? ????. lExtWord = P01, Opr = +, rExtWord =  (-30,20,20)
    //	2) -132,192,40 ?? ??????? ????. lExtWord = -132,192,40
    if (line.find(",") != string::npos && iLBrace == string::npos && iRBrace == string::npos)
    {
        string  IsOneBlockValue(line), subStr;
        float fValue = 0;

        ExtractOneAddress(IsOneBlockValue, subStr);
        ADDRESS addType;
        int iWarnMessage = 0;

        if(!GetValueAtOneWord(subStr, fValue, addType, iWarnMessage = 2))
        {
            // ????o?? -3f3f?? ???? ????? ??¡Æ??? ????
            lExtWord = line;
            line.clear();
        }
        lExtWord = line;
        return false;
    }

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == ' ')
        {
            if (!lExtWord.empty())
                continue;
            lExtWord.clear();
        }
        else if (c == '+')
        {
            extOpr += c;
            if (lExtWord.length() != 0)
            {
                break;
            }
        }
        else if (c == '-' && iPos > 0)
        {
            extOpr += c;
            if (lExtWord.length() != 0)
            {
                break;
            }
        }
        else
        {
            lExtWord += c;
        }
        iPos++;
    }
    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == '(' || c == ')')
        {
            if (rExtWord.length() != 0)
            {
                break;
            }
        }
        else
        {
            rExtWord += c;
        }
    }
    return(lExtWord.length() != 0 && rExtWord.length() != 0 && extOpr.length() != 0);

}

// Extracts logical words from a given string
// 1 || 0 -> lExtWord, || -> extOpr, rExtWord -> 0, wholeWord -> 1 || 0
bool CRobotLangParserAndOperator::ExtractWordLogicOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord)
{
    if (!lExtWord.empty()) lExtWord = "";
    if (!rExtWord.empty()) rExtWord = "";
    if (!extOpr.empty()) extOpr = "";
    if (!wholeWord.empty()) wholeWord.clear();

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == '&' || c == '|' || c == '!' || c == '^')
        {
            extOpr += c;
            wholeWord.push_back(c);
            if (lExtWord.length() != 0)
            {
                if (line[0] != NULL)
                    if (line[0] != '&' && line[0] != '|')
                        break;
            }
        }
        else
        {
            lExtWord += c;
            wholeWord.push_back(c);
        }
    }

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == ' ' || c == ',' || c == '>' || c == '<' || c == '!' || c == '(' || c == ')' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/') // shseo - ',' '(' ')' ??? ???
        {
            if (rExtWord.length() != 0)
                break;
        }
        else
        {
            rExtWord += c;
        }
        wholeWord.push_back(c);
    }
    return(lExtWord.length() != 0 && rExtWord.length() != 0 && extOpr.length() != 0);
}

bool CRobotLangParserAndOperator::ExtractWordRelationOpr(std::string& line, std::string& lExtWord, std::string& extOpr, std::string& rExtWord, std::string& wholeWord)
{ // Extracts one word from a string (words are delimited by spaces)

    if (!lExtWord.empty()) lExtWord = "";
    if (!rExtWord.empty()) rExtWord = "";
    if (!extOpr.empty()) extOpr = "";
    if (!wholeWord.empty()) wholeWord.clear();

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());
        if (c == '|' || c == '&' || c == '^') //c == ' ' ||
        {
            lExtWord.clear();
            wholeWord.clear();
        }
        else if (c == '>' || c == '<' || c == '!' || c == '(' || c == ')' || c == '=') // c == ',' ||
        {
            extOpr += c;
            wholeWord.push_back(c);
            if (lExtWord.length() != 0)
            {
                if (line[0] != NULL)
                    if (line[0] != '=')
                        break;
            }
        }
        else
        {
            lExtWord += c;
            wholeWord.push_back(c);
        }
    }

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == ' ' || c == ',' || c == '>' || c == '<' || c == '!' || c == '(' || c == ')' || c == '=' || c == '*' || c == '/') // c == '+' || c == '-' || shseo - ',' '(' ')' ??? ???
        {
            if (rExtWord.length() != 0)
                break;
        }
        else
        {
            rExtWord += c;
        }
        wholeWord.push_back(c);
    }
    return(lExtWord.length() != 0 && rExtWord.length() != 0 && extOpr.length() != 0);
}

bool CRobotLangParserAndOperator::GetCommandFromWord(const std::string& word, int& command)
{ // returns the command index for a string command
    command = CMD::ID_LABEL_DEFAULT;

    // "//" ??? ???
    if (word.size() > 2)
    {
        if (word.substr(0, 2) == "//")
        {
            command = CMD::ID_LINE_COMMENT;
            return(command != CMD::ID_LABEL_DEFAULT);
        }
    }

    if (word.compare("") == 0)
        command = CMD::ID_BLANK;
    if (word.compare("DELAY") == 0)
        command = CMD::ID_DELAY;
    if (word.compare("GOTO") == 0)
        command = CMD::ID_GOTO;
    if (word.compare("//") == 0)
        command = CMD::ID_LINE_COMMENT;
    if (word.compare("GOHOME") == 0)
        command = CMD::ID_GOHOME;
    if (word.compare("SPEED") == 0)
        command = CMD::ID_SPEED;
    if (word.compare("ROTATE") == 0)
        command = CMD::ID_ROTATE;
    if (word.compare("GRASP") == 0)
        command = CMD::ID_GRASP;
    if (word.compare("RELEASE") == 0)
        command = CMD::ID_RELEASE;
    if (word.compare("CHANGE") == 0)
        command = CMD::ID_CHANGE;
    if (word.compare("DRIVE") == 0)
        command = CMD::ID_DRIVE;
    if (word.compare("MOVE") == 0)
        command = CMD::ID_MOVE;
    if (word.compare("VAR") == 0) //  ????
        command = CMD::ID_VAR;
    if (word.compare("STOP") == 0)
        command = CMD::ID_STOP;
    if (word.compare("END") == 0)
        command = CMD::ID_END;
    if (word.compare("GOSUB") == 0)
        command = CMD::ID_GOSUB;
    if (word.compare("RETURN") == 0)
        command = CMD::ID_RETURN;
    if (word.compare("ELSEIF") == 0)
        command = CMD::ID_ELSEIF;
    if (word.compare("IF") == 0)
        command = CMD::ID_IF;
    if (word.compare("ELSE") == 0)
        command = CMD::ID_ELSE;
    if (word.compare("ENDIF") == 0)
        command = CMD::ID_ENDIF;
    if (word.compare("FOR") == 0)
        command = CMD::ID_FOR;
    if (word.compare("NEXT") == 0)
        command = CMD::ID_NEXT;
    if (word.compare("WAIT") == 0)
        command = CMD::ID_WAIT;
    if (word.compare("SET") == 0)
        command = CMD::ID_SET;
    if (word.compare("RESET") == 0)
        command = CMD::ID_RESET;
    if (word.compare("READY") == 0)
        command = CMD::ID_READY;
    if (word.compare("PRESET") == 0)	// PRESET ????? ??? - 2015.08.25
        command = CMD::ID_PRESET;
    if (word.compare("SWITCH") == 0)
        command = CMD::ID_SWITCH;
    if (word.compare("ENDSWITCH") == 0)
        command = CMD::ID_ENDSWITCH;
    if (word.compare("DEFAULT") == 0)
        command = CMD::ID_DEFAULT;
    if (word.compare("CASE") == 0)
        command = CMD::ID_CASE;
    if (word.compare("BREAK") == 0)	// PRESET ????? ??? - 2015.08.25
        command = CMD::ID_BREAK;
    if (word.compare("DIM") == 0)
        command = CMD::ID_DIM;
    if (word.compare("DEFPOS") == 0)
        command = CMD::ID_DEFPOS;
    if (word.compare("LET") == 0)
        command = CMD::ID_LET;
    if (word.compare("FN") == 0)
        command = CMD::ID_FN;
    if (word.compare("DEF") == 0)
        command = CMD::ID_DEF;


    if (command == CMD::ID_LABEL_DEFAULT)
    {
        int c = word.size();
        bool label = true;

        if (!(isalnum(word[0]) || std::string::npos != word.find("=", 0)))
            label = false;
        if (label)
            command = CMD::ID_LABEL;//label
    }

    return(command != -1);
}

void boostAction(vector<string> &v, int &lastlen, const char *begin, const char * end)
{
    string str(begin, end);
    if (!str.empty()) {
        v.clear();
        v.push_back(str);
        lastlen = 0;
    }
    else {
        v.resize(lastlen);
    }
}


bool CRobotLangParserAndOperator::IsChainMotionProgram(std::string &strStatement, int& nMoveCount, int& nPoseInMoveCount)
{
    string tmpString = strStatement;
    string strExtractString;
    int moveCount = 0;
    int iWarningMsg = 0;
    int iSearchIndexBegin = 0;
    int iSearchIndexEnd = 0;
    float fTmpValue[7] = { 0.0f, };
    int poseCount = 0;
    try
    {
        while (tmpString.size()>0)
        {
            iSearchIndexBegin = tmpString.find("(", iSearchIndexBegin);
            iSearchIndexEnd = tmpString.find(")", iSearchIndexBegin);

            if (iSearchIndexBegin != -1)
            {
                //	 Failed to parse. Only exist ( or ).
                if (iSearchIndexEnd == -1)
                    return false;

                tmpString.erase(iSearchIndexBegin, iSearchIndexEnd + 1);
                moveCount++;
            }
            else if (ExtractOneAddress(tmpString, strExtractString))
            {
                if (IsInternalUserdefPose(strExtractString) || strExtractString.find_first_of("@") != std::string::npos)
                {
                    GetDataFromUserPose(strExtractString.c_str(), fTmpValue);
                    moveCount++;
                    poseCount++;
                }
                else if (IsInternalPose(strExtractString) || strExtractString.find_first_of("@") != std::string::npos)
                {
                    GetDataFromPose(strExtractString.c_str(), fTmpValue, iWarningMsg);
                    moveCount++;
                    poseCount++;
                }
            }
        }
        nMoveCount = moveCount;
        nPoseInMoveCount = poseCount;

        if (moveCount > 1)
            return true;
        else
            return false;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool CRobotLangParserAndOperator::ExtractMultiWord(std::string& str, vector<string>& extracedWords)
{

    std::string oneProgramLine;

    while (str.size()>0)
    {
        ExtractOneWord(str, oneProgramLine);
        extracedWords.push_back(oneProgramLine);
    }

    return true;
}


bool CRobotLangParserAndOperator::GenerateSerialMotion(std::string &strStatement, CCompiledProgramLine& currentProgramLine)
{
    std::vector<std::string> parsedWords;

    std::string::size_type iLft, iRht;

    iLft = strStatement.find_first_of("(");
    iRht = strStatement.find_first_of(")");

    std::string strBracket, strValue;

    while (iLft != std::string::npos && iRht != std::string::npos)
    {
        strBracket = strStatement.substr(iLft + 1, iRht - iLft - 1);
        currentProgramLine._command = CMD::ID_MOVE2;
        if (!ExtractMultiWord(strBracket, parsedWords))
            return (false);

        int itmpIOvalue = 0;
        float ftmpValue[7] = { 0.0f, };
        strBracket.clear();

        for (int i = 0; i< (int) parsedWords.size(); strBracket = strBracket + parsedWords[i], strBracket = strBracket + "`", i++)
        {
            if (!(parsedWords[i].front() == 'T' || parsedWords[i].front() == 'S'
                || GetDataFromUserPose(parsedWords[i].c_str(), ftmpValue)
                || GetDataFromMap(parsedWords[i].c_str(), ftmpValue[0])
                || GetFloatFromWord(parsedWords[i].c_str(), ftmpValue[0])
                || GetDataFromIO(parsedWords[i].c_str(), itmpIOvalue)))
                return false;
        }
        strBracket.erase(strBracket.end() - 1);
        strStatement.replace(iLft, iRht - iLft + 1, strBracket);
        iLft = strStatement.find_first_of("(");
        iRht = strStatement.find_first_of(")");
        parsedWords.clear();
        strValue.clear();
    }

    parsedWords.clear();

    CCompiledProgramLine a;
    a._command = CMD::ID_MOVE2;
    a._iLineNumber = currentProgramLine._iLineNumber;
    int iAxisCount = 0;
    int iSerialMoveCount = 0;

    while (strStatement.size()>0)
    {
        ExtractOneWord(strStatement, strBracket);
        std::string subBracket(strBracket);
        std::string subXyzComp(strBracket);
        std::string refinedLine(strBracket);
        std::replace(refinedLine.begin(), refinedLine.end(),'`', ',');
        bool bPuse = false;

        if (ExtractOneWord(subBracket, subXyzComp, "`"))
        {
            a._strLabel[iAxisCount] = subXyzComp;
            while (subBracket.size()>0)
            {
                iAxisCount++;
                if (ExtractOneWord(subBracket, subXyzComp, "`"))
                {
                    float tmpFloat[7] = { 0.0f, };
                    a._strLabel[iAxisCount] = subXyzComp;
                    bPuse = GetDataFromUserPose(subXyzComp.c_str(), tmpFloat);
                }
            }
        }
        if (iAxisCount >= 3 && (!bPuse))
            a._command = CMD::ID_MOVE2;

        a._intParameter[0] = iSerialMoveCount;
        a._intParameter[1] = iAxisCount;
        a._originalUncompiledCode = refinedLine;
        currentProgramLine._compiledChainedMotionProgram->push_back(a);
        iAxisCount = 0;
        iSerialMoveCount++;
    }

    currentProgramLine._compiledChainedMotionProgram->push_back(a);

    return true;
}

// parsing 10,20,30
bool CRobotLangParserAndOperator::ExtractXyzrgPosition(std::string &strStatement, float val[])
{
    std::vector<std::string> parsedWords;

    if (!ExtractMultiWord(strStatement, parsedWords))
        return false;

    for (int i = 0; i<(int) parsedWords.size(); i++)
    {
        GetFloatFromWord(parsedWords[i], val[i]);
    }
    return true;
}

// Not used at this moment
bool CRobotLangParserAndOperator::ExtractXyzPosition(std::string &strStatement)
{
    /*	using namespace boost::spirit;
    using boost::spirit::qi::parse;
    std::vector<string> v;

    char const* l(strStatement.c_str() + strlen(strStatement.c_str()));
    if (boost::spirit::qi::parse(strStatement.c_str(), l,
    '(' >> boost::spirit::qi::float_[boost::phoenix::push_back(boost::phoenix::ref(v),
    boost::spirit::qi::_1)] % ',' >> ')') )
    return true;
    else*/
    return false;
}

//	P2, P3, P4+(X1,10,20) -> P2
bool CRobotLangParserAndOperator::ExtractOneMoveData(std::string &lExtWord, std::string &rExtWord)
{

    rExtWord = "";
    bool blBrace = false;
    bool brBrace = false;

    while (lExtWord.length() != 0)
    {
        char c = lExtWord[0];
        lExtWord.erase(lExtWord.begin());

        if (c == '(')
            blBrace = true;        //   P1+(X,Y,Z)							P2
        else if ((c == ',' && ((blBrace == true && brBrace == true) || (blBrace == false && brBrace == false))) || c == NULL)
        {
            if (rExtWord.length() != 0)
                break;
        }
        else if (c == ')')
            brBrace = true;


        rExtWord += c;
    }
    return(rExtWord.length() != 0);

}

//	P4+(X1,10,20)
int CRobotLangParserAndOperator::ExactractPoseData(const std::string &lPoseLabel, std::string &strStatement, CCompiledProgramLine& posData)
{
    std::string condLine, condWords, strBracket;
    std::string LWord, RWord, Opr;
    std::vector<std::string> parsedWords;
    ADDRESS eAddr = ADDRESS::FLOAT_VALUE;

    float fValue[7] = { 0.0f, };
    float fTmpValue = 0.0f;
    float fSpeedValue = 0.0f;
    int idx, iLft, iRht, iOprPos, iWarningMsg = 0;
    bool bThisisPose = IsInternalPose(lPoseLabel.c_str());
    bool bThisisUserPose = IsInternalUserdefPose(lPoseLabel.c_str());

    condLine = strStatement;

    if (lPoseLabel.size()>0)
        if (lPoseLabel.compare("NULL") != 0) {
            posData._strLabel[0] = lPoseLabel;
            if (!(bThisisPose	//	P1 ?? ???? ????? ???
                || bThisisUserPose))	//	DefPos ?? ???? ????? ???
            {
                return false;
            }
        }
    RemoveFrontAndBackSpaces(condLine);

    posData._strLabel[4] = "POSE";//	????
    posData._intParameter[0] = 0;	// pose operation option. ????? ???

    idx = condLine.find("=", 0);
    if (idx != std::string::npos) // = ?? ???? ???. =????
    {
        // S ?? o?? S = 10 ?? ????
        if (condLine.at(0) == 'S')
            condWords = 'S';

        else if (condLine.at(0) == 'T')
            strStatement = "ERROR_T";

        else if (condLine.at(0) == 'H')
            strStatement = "ERROR_H";

        condLine.erase(0, idx + 1);
        RemoveFrontAndBackSpaces(condLine);

        // ¨¨??? ?????????? S ?? o?? , S = 10 ?? ????.
        if (GetValueAtOneWord(condLine, fTmpValue, eAddr, iWarningMsg))
        {
            condWords = condWords + "=";
            strStatement = condWords + condLine;
            posData._command = CMD::ID_SPEED;
            posData._strLabel[2] = strStatement;
            return true;
        }
    }

    string garString = condLine;
    iLft = condLine.find_first_of("(");
    iRht = condLine.find_last_of(")");
    iOprPos = garString.find_first_of(POS_OP, 0);

    if(iLft != std::string::npos && iOprPos != std::string::npos)
        if(iOprPos < iLft)
        //	P1 + (0,20,100) ???? P1?? ???? ???? ???
        if (iOprPos != std::string::npos)
        {
            //	P1 + (0, 20, 100)->condLine = (0, 20, 100), strBracket = P1
           if(IsInternalPose(garString.c_str()) || IsInternalUserdefPose(garString.c_str()))
               ExtractOneWord(condLine, strBracket, " +-");
        }

    // S ?? o?? S = 10 ?? ????. ?????? ?????? Speed o????  ?? ?? ?? ????? ???.
    if (condLine.empty() && GetFloatFromWord(strBracket, fTmpValue))
    {
        strStatement = condWords + strBracket;
        posData._command = CMD::ID_SPEED;
        return true;
    }

    idx = garString.find_first_of(POS_OP, 0);

    if (strBracket.empty())	{
        bThisisUserPose = false;
        bThisisPose = false;
    }
    else {
        bThisisUserPose = GetDataFromUserPose(strBracket.c_str(), fValue);
        bThisisPose = GetDataFromPose(strBracket.c_str(), fValue, iWarningMsg);
    }

    // P1 + (10,20,0)???? P1?? ???????? ???
    if (bThisisUserPose || bThisisPose  || strBracket.compare("@") == 0)
    {
        posData._strLabel[0] = lPoseLabel; // P1
        posData._strLabel[1] = strBracket; // P2

        if (idx == std::string::npos)	//	P1 = P2
        {
            strStatement = strBracket;
            posData._intParameter[1] = 0; // ??? ????? ?????? ???????

            if(bThisisUserPose)
                GetDataFromUserPose(strBracket.c_str(), posData._floatParameter);	//	P1 ?????? ???? float ???? ??¢¥?. off???? o??.

            else if (bThisisPose)
                GetDataFromPose(strBracket.c_str(), posData._floatParameter, iWarningMsg);	//	P1 ?????? ???? float ???? ??¢¥?. off???? o??.

            return true;
        }
    }

    idx = garString.find_first_of("$");

    if (idx != std::string::npos)	//	Incremental ???. ???? ?????.
    {
        condLine = garString.erase(garString.find_first_of("$"), 1);
        condLine = garString;
        posData._intParameter[0] = 1;	//	1??? ?????, 0??? ?????
    }

    iLft = condLine.find_first_of("(");
    iRht = condLine.find_last_of(")");

    if (condLine.find("(", iLft) == std::string::npos && condLine.find(")", iRht) == std::string::npos)
    {
//		if(IsInternalUserdefPose(condLine.c_str()))
        if (GetDataFromUserPose(condLine.c_str(), fValue))
        {
            float fValues[7] = { 0.0f, };
            if (GetDataFromUserPose(strBracket.c_str(), fValues))	//	P1+P2 ?? ???? return
                return false;

            posData._strLabel[1] = condLine; // P2
            condLine = strBracket + condLine;
            return true;
        }
//		else if (IsInternalPose(condLine.c_str()))
        else if (GetDataFromPose(condLine.c_str(), fValue, iWarningMsg))
        {
            float fValues[7] = { 0.0f, };
            if (GetDataFromPose(strBracket.c_str(), fValues, iWarningMsg))	//	P1+P2 ?? ???? return
                return false;

            posData._strLabel[1] = condLine; // P2
            condLine = strBracket + condLine;
            return true;
        }
    }
    else
        condLine = garString;

    try
    {
        if (!ExtractWordPoseOpr(condLine, LWord, Opr, RWord))
        {
            //	MOVE (126.51, 296.48, 78),P1,P2???? (126.51, 296.48, 78)?? ???? ??? ?????? ????? ??? ????
            if (iRht != -1 && posData._strLabel[0].empty() && posData._strLabel[1].empty())
            {
                posData._strLabel[0] = "NULL";
                posData._strLabel[1] = "#";
                posData._strLabel[2] = LWord;
                posData._intParameter[1] = 1;
                return true;
            }

            //	???? ????? ?¥ê?. P1 = 10,20,30,40
            if(posData._strLabel[1].empty())
                posData._strLabel[1] = LWord;

            return false;
        }

        strBracket = RWord;

        if (iLft != std::string::npos && iRht != std::string::npos)
        {
            if (!ExtractMultiWord(strBracket, parsedWords))
                return (false);

            float ftmpValue[7] = { 0.0f, };
            strBracket.clear();

            posData._strLabel[1] = LWord; // P2
            for (int i = 0; i < (int) parsedWords.size(); strBracket = strBracket + parsedWords[i], strBracket = strBracket + ",", i++)
            {
                std::string tmpWord(parsedWords[i].c_str());

                if (GetDataFromMap(parsedWords[i].c_str(), ftmpValue[i]))
                    posData._floatParameter[i] = ftmpValue[i];
                else if (GetFloatFromWord(parsedWords[i], ftmpValue[i]))
                    posData._floatParameter[i] = ftmpValue[i];
                else if (parsedWords[i].compare("#") == 0)
                    posData._floatParameter[i] = 0.0f;
                else
                    if(!DoMathOperation(tmpWord, iWarningMsg)) // A*30 ?????? ????...
                    return false;
            }
            strBracket.erase(strBracket.end() - 1);
            posData._strLabel[2] = strBracket;

            if (Opr.compare("+") == 0)
            {
                posData._intParameter[1] = 1;
                for (int i = 0; i < 5; i++)
                    posData._floatParameter[i] = fValue[i] + posData._floatParameter[i];
            }
            else if (Opr.compare("-") == 0)
            {
                posData._intParameter[1] = 2;
                for (int i = 0; i < 5; i++)
                    posData._floatParameter[i] = fValue[i] - posData._floatParameter[i];
            }
        }

        parsedWords.clear();
    }

#ifndef QT_COMPIL
    catch (CMemoryException* e) {
    e->Delete();
#else
     catch( const std::exception& e) {
#endif
        return -1;
    }

    return TRUE;
}

// ???? Command line, ???? ???, ???? ????? ????, ???? ????? ?????¥ê???  ??????? ??????? ???? command line?????? ?¥ê??? ???? ?????? ???????, ???? ????? ???? ???????? ???¢¥?.
void CRobotLangParserAndOperator::GetCurrentRobotMotionInfo(const CCompiledProgramLine& currProgrram, std::vector<float>& currPos, float& currGrpPos, bool UpdateSavedPosALpha)
{
    static int MotionLineNoInOneLine = 0;
    static std::vector<float> savedPos = currPos;
    static float savedRos = currPos[3];
    static float savedGrpPos = currGrpPos;

    float fRobotPosData[5] = { 0.0f, };

    if ((currProgrram._strLabel[3].compare("SAVE_CURRENTPOS") == 0 && MotionLineNoInOneLine != currProgrram._iLineNumber) || UpdateSavedPosALpha == true)
    {
        savedPos = currPos;
        savedRos = currPos[3];
        savedGrpPos = currGrpPos;
        MotionLineNoInOneLine = currProgrram._iLineNumber;
    }

    currPos = savedPos;
    currPos[3] = savedRos;
    currGrpPos = savedGrpPos;
}

int CRobotLangParserAndOperator::DoPoseOperation(const CCompiledProgramLine& currProgramLine, float fTargValue[], std::string& rtnPose, const std::vector<float>& currPos, const float& currGrpPos)
{
    std::string LWord, RWord, Opr, condWords;
    std::vector<std::string> parsedWords;
    int iWarningMsg = 0;
    float fCurrValue[TOTAXES] = { 0.0f, };
    float fLvalueLabel[TOTAXES] = { 0.0f, };
    float fFirstLabel[TOTAXES] = { 0.0f, };
    float fSecondLabel[TOTAXES] = { 0.0f, };

    if (rtnPose != "NULL")
        if (rtnPose.empty())
            return -1;
        else if (!(IsInternalPose(currProgramLine._strLabel[0].c_str()) || IsInternalUserdefPose(currProgramLine._strLabel[0].c_str())))
            return -1;

    // Current position
    fCurrValue[0] = currPos[0];
    fCurrValue[1] = currPos[1];
    fCurrValue[2] = currPos[2];
    fCurrValue[3] = currPos[3];
    fCurrValue[4] = currGrpPos;
/*	fCurrValue[0] = currPos[0] * 1000;
    fCurrValue[1] = currPos[1] * 1000;
    fCurrValue[2] = -currPos[2] * 1000;
    fCurrValue[3] = -currPos[3];
    fCurrValue[4] = currGrpPos * 1000;*/

    for (int i = 0; i < TOTAXES; i++) //	o???? Label?? ???? ???????? ????.. ex 20,30,40,40 ?? ???? ????? ???????? ??
    {
        fFirstLabel[i] = fLvalueLabel[i];
    }

    try
    {
        if (currProgramLine._strLabel[1].compare("@") == 0) // Position ???? ????????
        {
            fFirstLabel[0] = fCurrValue[0];
            fFirstLabel[1] = fCurrValue[1];
            fFirstLabel[2] = fCurrValue[2];
            fFirstLabel[3] = fCurrValue[3];
            fFirstLabel[4] = fCurrValue[4];
        }
        else if (currProgramLine._strLabel[1].compare("#") == 0) // Position ???? ????????
        {
            fFirstLabel[0] = 0;
            fFirstLabel[1] = 0;
            fFirstLabel[2] = 0;
            fFirstLabel[3] = 0;
            fFirstLabel[4] = 0;
        }
        else
        {
            bool bPose = false;

            //	P1 ?? ???? User defined Pose data ?? ????
            if (IsInternalUserdefPose(currProgramLine._strLabel[1].c_str()))
                bPose = GetDataFromUserPose(currProgramLine._strLabel[1].c_str(), fFirstLabel);
            //	PoseData ?? ???? DERPOS?? ?????? Pose Data?? ????
            else if (IsInternalPose(currProgramLine._strLabel[1].c_str()))
                bPose = GetDataFromPose(currProgramLine._strLabel[1].c_str(), fFirstLabel, iWarningMsg);
            //	20,30,100,40 ?? ???? Position ??? ?????? ????, ??? ExtraMultiWord?? ?©§? ???? ?????
            else if(currProgramLine._strLabel[1].find_first_of(",") != std::string::npos
                    && currProgramLine._strLabel[1].find_last_of(",") != std::string::npos)
                    bPose = false;
            else
                return -1;

            condWords = currProgramLine._strLabel[1];

            if (!bPose)
            {
                if (!ExtractMultiWord(condWords, parsedWords))
                    return false;
                else
                {
                    for (int i = 0; i < (int) parsedWords.size(); i++)
                    {
                        if (IsInternalVariable(parsedWords[i]))
                            GetDataFromMap(parsedWords[i].c_str(), fFirstLabel[i]);
                        else if (IsInternalArray(parsedWords[i]))
                            GetDataFromArray(parsedWords[i].c_str(), fFirstLabel[i], iWarningMsg);
                        else if (IsInternalFunction(parsedWords[i]))
                            GetDataFromFunction(parsedWords[i].c_str(), fFirstLabel[i], iWarningMsg);
                        else
                            GetFloatFromWord(parsedWords[i], fFirstLabel[i]);
                    }
                }
            }
        }

        //	?¥é?¡Æ ?? a???? P3=P1+P2 ???? P2
        condWords = currProgramLine._strLabel[2];
        bool bPose = false;

        //	P2???? ?????? POSE ??????? ???? ??????? ????????
        if (!condWords.empty())
        {
            if (IsInternalUserdefPose(condWords.c_str()))
                bPose = GetDataFromUserPose(condWords.c_str(), fSecondLabel);
            if (IsInternalPose(condWords.c_str()))
                bPose = GetDataFromPose(condWords.c_str(), fSecondLabel, iWarningMsg);
        }
        //	???? ?????
        if (!bPose)
        {
            //	20, 300, 40, 10, 40?? ???? ??? ?????????
            if (!ExtractMultiWord(condWords, parsedWords))
                return false;
            else
            {
                for (int i = 0; i < (int) parsedWords.size(); i++)
                {
                    std::string tmpWord(parsedWords[i].c_str());
                    float tmpValue = 0.0f;

                    if (GetDataFromMap(parsedWords[i].c_str(), tmpValue))
                        fSecondLabel[i] = tmpValue;
                    else if (GetFloatFromWord(parsedWords[i], tmpValue))
                        fSecondLabel[i] = tmpValue;
                    else if (IsInternalFunction(parsedWords[i]))
                        GetDataFromFunction(parsedWords[i].c_str(), tmpValue, iWarningMsg);
                    else if (DoMathOperation(tmpWord, iWarningMsg)) // A*30 ?????? ????...
                        fSecondLabel[i] = (float) atof(tmpWord.c_str());
                    else
                        return false;

                    // ????????
                    if (currProgramLine._intParameter[0] == 1)
                        fSecondLabel[i] = fCurrValue[i] + fSecondLabel[i];
                }
            }
        }

        float fSign = 1.0f;
        switch (currProgramLine._intParameter[1])  // ?¥é?¡Æ?? ???
        {
        case 1: fSign = 1.0f; // +
            break;
        case 2: fSign = -1.0f; // -
            break;
        default:fSign = 0.0f;  // ?¥é?¡Æ ?? ????.
        }

        for (int i = 0; i<5; i++)
        {
            fTargValue[i] = fFirstLabel[i] + fSign*fSecondLabel[i];
        }
    }
    catch (...)
    {
        return -1;
    }
    return TRUE;
}

//	20+30*2-(30/2)
int CRobotLangParserAndOperator::DoMathOperation(std::string &strStatement, int& iWarnMsg)
{
    std::string LWord, RWord, Opr, condLine, condWords;

    condLine = strStatement;

    std::string::size_type idx, iLft, iRht;

    try
    {
        if (condLine.empty())
            return -1;

            //	???? ??????? ?????? return ???..
        idx = condLine.find_first_of(MATH_OP);

        //	-1 + 40 ?? ???? o??. ????? ?????? ?????
        if(condLine.at(0) == '-' && idx == 0)
            idx = condLine.find_first_of(MATH_OP,1);

        if (idx == std::string::npos)
            return FALSE;

        iLft = condLine.find_first_of("=");
        if (iLft != std::string::npos)
        {
            condLine.erase(iLft, iLft + 1);
            RemoveFrontAndBackSpaces(condLine);
            strStatement = condLine;
        }

        iLft = condLine.find_first_of("(");
        iRht = condLine.find_last_of(")");

    //	1-(30*2)?? ???? ????? ??? ???? ???? ??????? o????? ???.
        //	??, ????????? ????? 1-60???? ?????? ?????? ?????.
        while (iLft != std::string::npos && iRht != std::string::npos)
        {
            std::string strBracket;
            strBracket = condLine.substr(iLft + 1, iRht - iLft - 1);

            // 1-60
            if (DoMathOperation(strBracket, iWarnMsg) >= 0)
            {
                condLine.replace(iLft, iRht - iLft + 1, strBracket);
                strStatement = condLine;
            }
            iLft = condLine.find_first_of("(");
            iRht = condLine.find_last_of(")");
        }

        //	????? ?????? 1-60?? o????? ???? ???
        while (idx != std::string::npos)
        {
            bool bRtnValue = false;

            //	LWord = 1, Opr = -, RWord = 60???? ????
            bRtnValue = ExtractWordMathOpr(condLine, LWord, Opr, RWord, condWords);

            if (!bRtnValue)
#ifndef QT_COMPIL
                throw(new CMemoryException);
#else
            throw 1;
#endif

            RemoveFrontAndBackSpaces(LWord);
            RemoveFrontAndBackSpaces(RWord);

            float fLword = 0.0f;
            float fRword = 0.0f;
            int iLword = 0;
            int iRword = 0;
            ADDRESS eAddr = ADDRESS::FLOAT_VALUE;

            /*
            if( GetDataFromMap(LWord.c_str(), fLword) == false && !LWord.empty())
            fLword = atof (LWord.c_str());
            if( GetDataFromMap(RWord.c_str(), fRword) == false && !RWord.empty())
            fRword = atof (RWord.c_str());;

            if (GetDataFromArray(LWord.c_str(), fLword, iWarnMsg))
                fLword = fLword;
            else if (GetDataFromMap(LWord.c_str(), fLword))
                fLword = fLword;
            else if (GetDataFromIO(LWord.c_str(), iLword))
                fLword = (float)iLword;
            else if (GetDataFromFunction(LWord.c_str(), fLword, iWarnMsg))
                fLword = (float)fLword;
            else
                GetFloatFromWord(LWord, fLword);
            */

            GetValueAtOneWord(LWord, fLword, eAddr, iWarnMsg);

            //	?ò÷??? ??? ?????? ?????? ???? ????? ????
            if (iWarnMsg == 2 || iWarnMsg == 4)
                return false;

            /*
            if (GetDataFromArray(RWord.c_str(), fRword, iWarnMsg))
                fRword = fRword;
            else if (GetDataFromMap(RWord.c_str(), fRword))
                fRword = fRword;
            else if (GetDataFromIO(RWord.c_str(), iRword))
                fRword = (float)iRword;
            else if (GetDataFromFunction(RWord.c_str(), fRword, iWarnMsg))
                fRword = (float)fRword;
            else
                GetFloatFromWord(RWord, fRword);
            */

            GetValueAtOneWord(RWord, fRword, eAddr, iWarnMsg);

            //	?ò÷??? ??? ?????? ?????? ???? ????? ????
            if (iWarnMsg == 2 || iWarnMsg == 4)
                return false;


            float fResult = 0.0f;

            //	???? 4? ???? ????
            if (Opr == "*") fResult = fLword*fRword;
            else if (Opr == "/") fResult = fLword / fRword;
            else if (Opr == "+") fResult = fLword + fRword;
            else if (Opr == "-") fResult = fLword - fRword;
            else if (Opr == "%") fResult = (float) (((int)fLword) % ((int)fRword));

            std::string strResult = std::to_string(fResult);
            strStatement.replace(strStatement.find(condWords.c_str()), condWords.size(), strResult.c_str());

            /* char strResult[11];
            sprintf_s(strResult, "%6.4f", fResult);
            strStatement.replace(strStatement.find(condWords.c_str()), condWords.size(), (const char*)strResult);*/
            condLine = strStatement;

            idx = condLine.find_first_of(MATH_OP);

            //	?????? ?????? -1 + 40?? ???? ????? ?????? ?? ????
            if (idx == 0 && condLine.at(0) == '-')
                idx = condLine.find_first_of(MATH_OP,1);

        }
    }
    catch (...)
    {
        strStatement.clear();
        return -1;
    }
    return TRUE;
}

int CRobotLangParserAndOperator::DoRelationalOperation(std::string &strStatement, int& iWarnMessage)
{
    std::string lWord, RWord, Opr, condLine, condWords;

    condLine = strStatement;

    std::string::size_type idx;

    idx = condLine.find_first_of(RELATIONAL_OP);

    if (idx == std::string::npos)
        return FALSE;
    else if (idx == 0 && strStatement.at(0) != '!')
        return FALSE;

    try {
        while (idx != std::string::npos)
        {
            bool bRtnValue = false;
            bRtnValue = ExtractWordRelationOpr(condLine, lWord, Opr, RWord, condWords);

            RemoveFrontAndBackSpaces(lWord);
            RemoveFrontAndBackSpaces(RWord);

            float fLword = 0.0f;
            float fRword = 0.0f;
            int iLword = 0;
            int iRword = 0;
            float ftmpValue = 0.0f;

            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            if (lWord.length() == 0 || RWord.length() == 0 || Opr.length() == 0)
                return -1;

            if (!GetValueAtOneWord(lWord, fLword, eAddr, iWarnMessage))
                return -1;

            if (!GetValueAtOneWord(RWord, fRword, eAddr, iWarnMessage))
                return -1;

            std::string strResult;

            if (Opr == "=")
                if (fLword == fRword) strResult = "1"; else strResult = "0";
            else if (Opr == ">")
                if (fLword > fRword) strResult = "1"; else strResult = "0";
            else if (Opr == "<")
                if (fLword < fRword) strResult = "1"; else strResult = "0";
            else if (Opr == "<=")
                if (fLword <= fRword) strResult = "1"; else strResult = "0";
            else if (Opr == ">=")
                if (fLword >= fRword) strResult = "1"; else strResult = "0";
            else if (Opr == "!=")
                if (fLword != fRword) strResult = "1"; else strResult = "0";
            else
                return -1;

            strStatement.replace(strStatement.find(condWords.c_str()), condWords.size(), strResult);

            idx = condLine.find_first_of(RELATIONAL_OP);
        }
    }
    catch (...)
    {
        strStatement.clear();
        return -1;
    }
    return TRUE;
}

//	???? ?????? ex. A OR B
int CRobotLangParserAndOperator::DoLogicalOperation(std::string &strStatement, int& iWarningMsg)
{
    std::string lWord, RWord, Opr, condLine, condWords;

    condLine = strStatement;

    std::string::size_type idx;

    // ???? ???? ??????? ?????? return
    idx = condLine.find_first_of(LOGICAL_OP);

    if (idx == std::string::npos)
        return FALSE;

    try {
        while (idx != std::string::npos)
        {
            bool bRtnValue = false;		//				 A ,  OR,   B,    A OR B
            bRtnValue = ExtractWordLogicOpr(condLine, lWord, Opr, RWord, condWords);

            int iLword = 0;
            int iRword = 0;
            float fLword = 0;
            float fRword = 0;

            if (lWord.length() == 0 || RWord.length() == 0 || Opr.length() == 0)
                return -1;

            RemoveFrontAndBackSpaces(lWord);
            RemoveFrontAndBackSpaces(RWord);

            if (GetDataFromMap(lWord.c_str(), fLword))
                fLword = (float)fLword;
            else if (GetDataFromIO(lWord.c_str(), iLword))
                fLword = (float)iLword;
            else if (GetDataFromArray(lWord.c_str(), fLword, iWarningMsg))
                fLword = (float)fLword;
            else if (GetDataFromFunction(lWord.c_str(), fLword, iWarningMsg))
                fRword = (float)fRword;
            else
                GetFloatFromWord(lWord.c_str(), fLword);


            if (GetDataFromMap(RWord.c_str(), fRword))
                fRword = (float)fRword;
            else if (GetDataFromIO(RWord.c_str(), iRword))
                fRword = (float)iRword;
            else if (GetDataFromArray(RWord.c_str(), fRword, iWarningMsg))
                fRword = (float)fRword;
            else if (GetDataFromFunction(RWord.c_str(), fRword, iWarningMsg))
                fRword = (float)fRword;
            else
                GetFloatFromWord(RWord.c_str(), fRword);


            int iResult = 0;

            if (Opr == "&") (iResult = (int)fLword&(int)fRword);
            else if (Opr == "&&") (iResult = (int)fLword && (int)fRword);
            else if (Opr == "|") (iResult = (int)fLword | (int)fRword);
            else if (Opr == "||") (iResult = fLword || (int)fRword);
            else if (Opr == "!") (iResult = ((int)fLword) % ((int)fRword));
            else if (Opr == "^") (iResult = ((int)fLword ^ (int)fRword));
            else return -1;


            char strResult[5];

#ifndef WIN_VREP
 sprintf(strResult, "%d", iResult);
#else
 sprintf_s(strResult, "%d", iResult);
#endif



            int szStart = strStatement.find(condWords.c_str());
            strStatement.replace(szStart, condWords.size(), strResult);
            condLine = strStatement;

            idx = condLine.find_first_of(LOGICAL_OP);
        }
    }
    catch (...)
    {
        strStatement.clear();
        return -1;
    }
    return TRUE;
}

bool CRobotLangParserAndOperator::GetIntegerFromWord(const std::string& word, int& theInteger)
{ // returns the integer value corresponding to a string
    try
    {
        theInteger = boost::lexical_cast<int>(word);
        return(true);
    }
    catch (boost::bad_lexical_cast &)
    {
        return(false);
    }
}

bool CRobotLangParserAndOperator::GetFloatFromWord(const std::string& word, float& theFloat)
{ // returns the float value corresponding to a string
    try
    {
        theFloat = boost::lexical_cast<float>(word);
        return(true);
    }
    catch (boost::bad_lexical_cast &)
    {
        return(false);
    }
}

bool CRobotLangParserAndOperator::RemoveFrontAndBackSpaces(std::string& word, bool bCommentLine)
{ // removes spaces at the front and at the back of a string
    int iFindOneLineComment;		// '//' ??? ?????
    bool bPrevCommentMaintain = bCommentLine;
    int iFindCommentStart, iFindCommentEnd;

    iFindOneLineComment = word.find("//");
    iFindCommentStart = word.find("-*");
    iFindCommentEnd = word.find("*-");

    if (iFindCommentStart != -1)		// ??? ????
    {
        if (iFindOneLineComment != -1)
        {
            if (iFindCommentStart < iFindOneLineComment)
                bCommentLine = TRUE;
            else
                iFindCommentStart = -1;
        }
        else
            bCommentLine = TRUE;
    }
    if (iFindOneLineComment != -1 && iFindCommentEnd > iFindOneLineComment)		// '//' ??????? ??? ??? ???¯I
        iFindCommentEnd = -1;


    if (bCommentLine)
    {
        int iStartCount = 0;
        int iEndCount;
        int iWhileCount = 0;	// ??? ???? ???? ?????

        bool bFindComment = TRUE;
        while (bFindComment)
        {
            ++iWhileCount;
            if (iFindCommentStart != -1)		// ????? ?????? ???? ????
                iStartCount = iFindCommentStart;

            if (iFindCommentEnd != -1)		// ????? ?????? ???? ????
                iEndCount = iFindCommentEnd + 2;		// ??????? ???? 2?????

            else
                iEndCount = word.length();

            // ????? ????/???? ????
            if (((iStartCount - iEndCount) == -1 || (iStartCount - iEndCount) == 1) && iFindCommentStart != -1)		// '-*-', '*-*' ?? ????
            {
                if (bPrevCommentMaintain)	// ???????? ??????? ???? ??? ???? ?¥í?
                {
                    iStartCount = 0;
                    bCommentLine = FALSE;
                }
                else						// ??? ??????? ?¥í?
                {
                    iEndCount = word.length();	// ??? ???? ????
                    bCommentLine = TRUE;
                }
            }
            else if (iFindCommentStart > iFindCommentEnd)	// ??? ???? ????
                bCommentLine = TRUE;
            else if ((iFindCommentStart < iFindCommentEnd) && iFindCommentEnd != -1)
                bCommentLine = FALSE;

            //??? ??? ???
            if (iFindCommentStart != -1)
            {
                if (iStartCount > iEndCount)	// ??? ?????? ???????
                {
                    iFindCommentStart = iStartCount;
                    iStartCount = 0;
                }
                else
                {
                    iFindCommentStart = word.find("-*", iStartCount + 2);
                    if (iFindCommentStart > iFindOneLineComment && iFindOneLineComment != -1)	// '//' ??????? ??? ??? ???¯I
                        iFindCommentStart = -1;
                }
            }
            if (iFindCommentEnd != -1)
            {
                iFindCommentEnd = word.find("*-", iEndCount + 2);
                if (iFindCommentEnd > iFindOneLineComment && iFindOneLineComment != -1)	// '//' ??????? ??? ??? ???¯I
                    iFindCommentEnd = -1;
            }
            else
                iEndCount = word.length();

            // ??? ????
            word.erase(iStartCount, iEndCount - iStartCount);

            // ??? ?????? String ???????? ???? ??? ??? a??
            iFindOneLineComment = word.find("//");
            iFindCommentStart = word.find("-*");
            iFindCommentEnd = word.find("*-");

            if ((iFindCommentStart == -1 && iFindCommentEnd == -1 && iStartCount == 0) ||
                (iFindCommentStart == -1 && (iStartCount < iEndCount)) ||
                iWhileCount > 10)	// ????? ??? o?? ???(???????????)
                bFindComment = FALSE;
        }
    }

    iFindOneLineComment = word.find("//");
    if (word.length() > 2 && iFindOneLineComment >= 0)
    {
        word.erase(iFindOneLineComment, word.length() - iFindOneLineComment);
    }
    // End - 2015.08.12

    while ((word.length() != 0) && ((word[0] == ' ') || word[0] == 0x09))
        word.erase(word.begin());
    while ((word.length() != 0) && ((word[word.length() - 1] == ' ') || (word[word.length() - 1] == 0x09)))
        word.erase(word.begin() + word.length() - 1);

    return bCommentLine;
}


int CRobotLangParserAndOperator::GetCountComma(std::string line)//,std::string& extractedWord
{
    int cnt = 0;

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == ',')
            cnt++;
    }
    return cnt;
}

bool CRobotLangParserAndOperator::IsInternalUserdefPose(const std::string& variable)
{
    auto iPos = 0;

    if (variable.at(0) == 'P')		// P ??? ??
    {
        string pAddr = variable.substr(iPos + 1, variable.size() - iPos);

        int iTempData = atoi(pAddr.c_str());
        int  iWarningMsg = 0;

        if (pAddr.find("[") != string::npos && pAddr.find("]") != string::npos)
        {
            vector<int> dimSizes;
            ExtractValuesFromBigBraces(pAddr, dimSizes);

            if (dimSizes.size() > 1)
                return false;

            iTempData = dimSizes[0];
        }
        else
            iTempData = atoi(variable.substr(iPos + 1, variable.size() - iPos).c_str());

        if (iTempData <= 0 || iTempData > MAX_POINT_NUM)
            return false;
        else
            return true;
    }
    return false;
}

bool CRobotLangParserAndOperator::IsInternalPose(const std::string& variable)
{
    int iLft = variable.find_first_of("[");
    int iRht = variable.find_first_of("]");

    //	ArrayVar[][] ?? ????
    if (iLft != std::string::npos && iRht != std::string::npos)
    {
        std::string strPoseArrayName(variable);
        strPoseArrayName = strPoseArrayName.erase(iLft, strPoseArrayName.size() - iLft);
        if (_internalPoseArray.find(strPoseArrayName.c_str()) != _internalPoseArray.end())
            return true;
        return false;
    }	//	ArrayVar ?? ????
    else if (_internalPoseArray.find(variable.c_str()) != _internalPoseArray.end())	//	 ??????
        return true;
    return false;
}

bool CRobotLangParserAndOperator::IsInternalVariable(const std::string& variable)
{
    if (_internalVariable.find(variable.c_str()) != _internalVariable.end())
        return true;
    return false;
}

bool CRobotLangParserAndOperator::IsInternalFunction(const std::string& variable)
{
    int iLPos = variable.find("{");
    int iRPos = variable.find("}");
    std::string tmpStr = variable;

    if (iLPos != string::npos || iRPos != string::npos)
        tmpStr = tmpStr.erase(iLPos, tmpStr.size() - iLPos);

    if (_internalFunction.find(tmpStr.c_str()) != _internalFunction.end())
        return true;
    return false;
}

bool CRobotLangParserAndOperator::IsInternalArray(const std::string& variable)
{
    int iLPos = variable.find("[");
    int iRPos = variable.find("]");
    std::string tmpStr = variable;

    if (iLPos != string::npos || iRPos != string::npos)
        tmpStr = tmpStr.erase(iLPos, tmpStr.size() - iLPos);

    if (_internalArray.find(tmpStr.c_str()) != _internalArray.end())	//	 ??????
        return true;
    return false;
}

//	????? ??? XYZRH?? ????? 01234??
unsigned int CRobotLangParserAndOperator::GetAxisNumberByAxisName(const std::string& name)
{
    if (name.compare("X") == 0)
        return 0;
    else if (name.compare("Y") == 0)
        return 1;
    else if (name.compare("Z") == 0)
        return 2;
    else if (name.compare("T") == 0)
        return 3;
    else if (name.compare("G") == 0)
        return 4;
    else
        return 0xFFFF;
}

bool CRobotLangParserAndOperator::IsInternalPorts(const std::string& portName)
{
    std::string variable = portName;

    int thisIsIn = variable.find("IN");
    int thisIsOut = variable.find("OUT");

    if(thisIsIn != string::npos)
        variable = variable.substr(thisIsIn + 2, variable.size() - thisIsIn);

    else if (thisIsOut != string::npos)
        variable = variable.substr(thisIsOut + 3, variable.size() - thisIsOut);

    if (thisIsIn != string::npos || thisIsOut != string::npos)
    {
        int portRange = atoi(variable.c_str());

        if (portRange >= 0 && portRange <= 7)
            return true;
        else
            return false;
    }
    return false;
}

bool CRobotLangParserAndOperator::GetValueAtOneWord(const std::string& variable, float& value, ADDRESS& addType, int& iWarnMessage)
{
    try {
        if (IsInternalVariable(variable)) {
            GetDataFromMap(variable.c_str(), value);
            addType = ADDRESS::GENERAL_VAR;
            return true;
        }
        else if (IsInternalArray(variable)) {
            GetDataFromArray(variable.c_str(), value, iWarnMessage);
            addType = ADDRESS::ARRAY_VAR;

            if (iWarnMessage > 0)
                return false;
            else
                return true;
        }
        else if (IsInternalPorts(variable))
        {
            int iFloatCast = (int)value;
            GetDataFromIO(variable.c_str(), iFloatCast);
            value = (float) iFloatCast;
            addType = ADDRESS::PORT_IO;
            return true;
        }
        else if (IsInternalFunction(variable))
        {
            GetDataFromFunction(variable.c_str(), value, iWarnMessage);
            addType = ADDRESS::ARRAY_VAR;

            if (iWarnMessage > 0)
                return false;
            else
                return true;
        }
        else {	//	20*30
            if ((bool)std::isalpha(variable.at(0)) == false)
            {
                string tmpVariable = variable;
                if (!GetFloatFromWord(variable, value)) {
                    if (DoMathOperation(tmpVariable, iWarnMessage))
                        GetFloatFromWord(tmpVariable, value);
                    else
                        return false;
                }
                addType = ADDRESS::FLOAT_VALUE;
                return true;
            }
            else
                false;
        }
        return false;
    }
    catch (...)
    {
        return false;
    }
}

bool CRobotLangParserAndOperator::GetPoseAtOneWord(const std::string& variable, float values[], int& iWarnMessage)
{
    bool bThisIsPoseData = IsInternalPose(variable);
    bool bThisIsUserPoseData = IsInternalUserdefPose(variable);

    if (bThisIsPoseData)
        GetDataFromPose(variable, values, iWarnMessage);
    else if (bThisIsUserPoseData)
        GetDataFromUserPose(variable, values);
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::GetDataFromMap(const std::string& variable, float &value)
{	//	Get a data from map
    auto result = _internalVariable.find(variable.c_str());
    SVarData sData;

    if (result != _internalVariable.end())
    {
        sData = result->second;
        value = sData.GetValue();
        return true;
    }
    else
        return false;
}

// -1 : fail, 0 : float, 1: unsigned int, 2: int         iWarnMessage = 0 (normal)
VAR CRobotLangParserAndOperator::GetVariableTypeAndTrimSymbol(std::string* strVar, int& iWarnMessage)
{
    iWarnMessage = 0;

    if (strVar == nullptr)
        return VAR::NONE;

    VAR iRtnValue = VAR::VAR_FLOAT;

    char cFirstChar = (char)strVar->at(0);

    switch (cFirstChar)
    {
        case '`':	// unsigned int
            iRtnValue = VAR::VAR_UINT;
            strVar->erase(strVar->begin());
            break;

        case '#':	//	int
            iRtnValue = VAR::VAR_INT;
            strVar->erase(strVar->begin());
            break;

        case '$':	//	float
            iRtnValue = VAR::VAR_FLOAT;
            strVar->erase(strVar->begin());
            break;

        default:
            if ((bool)std::isalpha(cFirstChar) == false)
                iRtnValue = VAR::NONE;
            else
            {
                float fTmpValue[TOTAXES] = { 0.0f, };
                if (IsInternalUserdefPose(strVar->c_str()))
                {
                    return VAR::VAR_POSE;
                }
                else
                    iWarnMessage = 1;
            }

            iRtnValue = VAR::VAR_FLOAT;
            break;
    }

    return iRtnValue;
}

bool CRobotLangParserAndOperator::ExtractValuesFromBigBraces(std::string& line, vector<int>& extArgs)
{
    int iPos = 0;

    std::string	Arg = "";
    bool blBrace = false;
    bool brBrace = false;

    if (extArgs.size() > 0)
        return false;

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == '[')
        {
            //	[arrayData[0][0]] ?? ???? ?ò÷ ???? ?±Ø??? ??? ?ò÷?? ??? ????
            if(iPos > 0)
                Arg += c;

            iPos++;
            blBrace = true;        //   [a][b][c]
        }
        else if (c == ']' )
        {
            iPos--;

            //	[arrayData[0][0]] ?? ???? ?ò÷ ???? ?±Ø??? ??? ?ò÷?? ??? ????
            if (iPos > 0)
                Arg += c;

            if (iPos == 0)
                brBrace = true;
        }
        else
            Arg += c;

        if (blBrace == true && brBrace == true)
        {
            if (Arg.length() == 0)
                return false;

            int iWarnMessage = 0;
            float ftmpValue = 0.0f;
            bool bGetValue = false;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            //	Array[pos[0][0]] ?? ???? ?????? ???? ?????.

            DoMathOperation(Arg, iWarnMessage);
            bGetValue = GetValueAtOneWord(Arg, ftmpValue, eAddr, iWarnMessage);

            if (ftmpValue < 0.0f || bGetValue == false)
                return false;

            extArgs.push_back((int)ftmpValue);

            if (c == NULL)
                break;

            Arg.clear();
            iPos = 0;
            blBrace = false;
            brBrace = false;
        }
    }

    return true;
}

bool CRobotLangParserAndOperator::ExtractValuesFromMidBraces(std::string& line, vector<float>& extArgs)
{
    std::string	Arg = "";
    bool blBrace = false;
    bool brBrace = false;
    bool bComma = false;

    if (extArgs.size() > 0)
        return false;

    while (line.length() != 0)
    {
        char c = line[0];
        line.erase(line.begin());

        if (c == '{')
        {
            //	arrayData{0,8,3}
            Arg.clear();
            blBrace = true;        //   [a][b][c]
        }
        else if (c == '}')
        {
            brBrace = true;
        }
        else if (c == ',')
        {
            bComma = true;
        }
        else
            Arg += c;

        if (blBrace == true && bComma == true || brBrace == true)
        {
            if (Arg.length() == 0)
                return false;

            int iWarnMessage = 0;
            float ftmpValue = 0.0f;
            bool bGetValue = false;
            ADDRESS eAddr = ADDRESS::GENERAL_VAR;

            //	Array[pos[0][0]] ?? ???? ?????? ???? ?????.
            bGetValue = GetValueAtOneWord(Arg, ftmpValue, eAddr, iWarnMessage);

            if (bGetValue == false)
                return false;

            extArgs.push_back(ftmpValue);

            if (c == NULL)
                break;

            Arg.clear();
            bComma = false;
        }
    }

    return true;
}

bool CRobotLangParserAndOperator::AllocateInternalPoseDimension(const std::string& arryName, std::vector<int>& extArgs)
{
    if (extArgs.size() < 3)
        return false;
    // a,b,c
    int HEIGHT = (extArgs[0] == -1) ? 1 : extArgs[0];
    int WIDTH = (extArgs[1] == -1) ? 1 : extArgs[1];
    int DEPTH = (extArgs[2] == -1) ? 1 : extArgs[2];

    DimPoseElements array3D;

    // Set up sizes. (HEIGHT x WIDTH)
    array3D.resize(HEIGHT);
    for (int i = 0; i < HEIGHT; ++i) {
        array3D[i].resize(WIDTH);

        for (int j = 0; j < WIDTH; ++j)
            array3D[i][j].resize(DEPTH);
    }
    _internalPoseArray[arryName.c_str()] = array3D;

    return true;
}

bool CRobotLangParserAndOperator::InitInternalPoseDimension(const std::string& arryName, const std::vector<float>& value)
{
    if (!IsInternalPose(arryName))
        return false;

    DimPoseElements& array3D = _internalPoseArray.at(arryName);

    int HEIGHT = array3D.size();
    int WIDTH = array3D[0].size();
    int DEPTH = array3D[0][0].size();

    if (value.size() > TOTAXES)
        return false;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j)
            for (int k = 0; k < DEPTH; ++k)
                for(int l = 0;l< value.size();l++)
                    array3D[i][j][k]._Data[l] = value[l];

    }

    return true;
}


bool CRobotLangParserAndOperator::InitInternalVarDimension(const std::string& arryName, float value)
{
    if (!IsInternalArray(arryName))
        return false;

    DimVarElements& array3D = _internalArray.at(arryName);

    int HEIGHT = array3D.size();
    int WIDTH = array3D[0].size();
    int DEPTH = array3D[0][0].size();
    int iNum = 0;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j)
            for (int k = 0; k < DEPTH; ++k)
                iNum = array3D[i][j][k].SetValue(value);
    }
    if (iNum != 0)
        return false;

    return true;
}

bool CRobotLangParserAndOperator::AllocateInternalVarDimension(const std::string& arryName, std::vector<int>& extArgs)
{
    if (extArgs.size() < 3)
        return false;
    // a,b,c
    int HEIGHT = (extArgs[0] == -1) ? 1 : extArgs[0];
    int WIDTH = (extArgs[1] == -1) ? 1 : extArgs[1];
    int DEPTH = (extArgs[2] == -1) ? 1 : extArgs[2];

    DimVarElements array3D;

    // Set up sizes. (HEIGHT x WIDTH)
    array3D.resize(HEIGHT);
    for (int i = 0; i < HEIGHT; ++i) {
        array3D[i].resize(WIDTH);

        for (int j = 0; j < WIDTH; ++j)
            array3D[i][j].resize(DEPTH);
    }
    _internalArray[arryName.c_str()] = array3D;

    // Put some values like, array3D[1][2][5] = 6.0 orarray3D[3][1][4] = 5.5;
    return true;
}
// PosX[1] = 10,20,30
bool CRobotLangParserAndOperator::ExtractDeposDefine(std::string& strVardef, VAR_TYPE &iVarType, int &iWarnMessage, std::string &lExtWord, std::string &rExtWord, vector<int>& dimSizes, vector<float>& initValues)
{
    //	 no warning
    iWarnMessage = 0;
    bool bRtnValue = false;
    auto iEqualPos = strVardef.find("=");
    std::vector<string> strInitValues;

    // find '=' in  = (10,20,30)
    if (iEqualPos != string::npos)
    {
        rExtWord = strVardef.substr( iEqualPos+1, strVardef.size() - iEqualPos);

        std::string rExtTmp = rExtWord;

        if (!ExtractMultiWord(rExtTmp, strInitValues))
            return false;

        // ????? ??????? ????????
        initValues.resize(strInitValues.size());

        for (int i = 0; i< (int)strInitValues.size(); i++)
        {
            //	???? ???? ?????? ?????? ???
            if (IsInternalVariable(strInitValues[i]) || IsInternalArray(strInitValues[i]))
                return false;

            GetFloatFromWord(strInitValues[i].c_str(), initValues[i]);
        }

        strVardef = strVardef.substr(0, iEqualPos);
    }

    if (ExtractOneWord(strVardef, lExtWord, "["))
    {
        if (lExtWord.at(0) != '[')
        {
            strVardef = strVardef + "1]";
        }

        iVarType = GetVariableTypeAndTrimSymbol(&lExtWord, iWarnMessage);

        strVardef = "[" + strVardef;
        bRtnValue = ExtractValuesFromBigBraces(strVardef, dimSizes);

        if (bRtnValue == false)
            iWarnMessage = WRN::WRN_DIMINIT_WRONG;

    }
    else
        return false;

    //	Change from [2] or [1][2] to [1][1][2]
    int iIterationNo = 3 - dimSizes.size();

    for (int i = 0; i < iIterationNo; i++)
    {
        dimSizes.insert(dimSizes.begin(), -1);
    }

    //	Max 3-dimensional array
    if (dimSizes.size() < 3)
        return false;

    return true;

}

bool CRobotLangParserAndOperator::ExtractDimDefine(std::string& strVardef, VAR &iVarType, int &iWarnMessage, std::string &lExtWord, vector<int>& dimSizes, std::string &rExtWord)
{
    //	 no warning
    iWarnMessage = 0;

    bool bRtnValue = false;

    auto iEqualPos = strVardef.find("=");

    // find '=' in  = (10,20,30)
    if (iEqualPos != string::npos)
    {
        rExtWord = strVardef.substr(iEqualPos + 1, strVardef.size() - iEqualPos);
        strVardef = strVardef.substr(0, iEqualPos);
    }

    // A
    if (ExtractOneWord(strVardef, lExtWord, "["))
    {
        iVarType = GetVariableTypeAndTrimSymbol(&lExtWord, iWarnMessage);

//		if (iVarType == VAR::NONE || iVarType == VAR::VAR_UINT || iVarType == VAR::VAR_INT)
//			return false;

        //	¹è¿­°ª¿¡ [] ¾øÀÌ ÇÒ´çÇÒ¶§, DIM HH
        if (strVardef.empty())
            iWarnMessage = WRN::WRN_DIMINIT_WRONG;

        strVardef = "[" + strVardef;
        bRtnValue = ExtractValuesFromBigBraces(strVardef, dimSizes);

        /* find '='  ???? ?????? ??????...
        if (strVardef.find("=") != string::npos)
            //	10
            if (!ExtractOneWord(strVardef, rExtWord))
                return false;
        */
    }
    else
        return false;

    //	Change from [2] or [1][2] to [1][1][2]
    int iIterationNo = 3 - dimSizes.size();

    for (int i = 0; i < iIterationNo; i++)
    {
        dimSizes.insert(dimSizes.begin(), -1);
    }

    //	Max 3-dimensional array
    if (dimSizes.size() < 3)
        return false;

    return true;
}


// Extracting front command(A=10, B=20 -> B=20), iVarType == -1 (abnormal), iWarnMessage == -1 (abnormal)
bool CRobotLangParserAndOperator::ExtractVarDefine(std::string& strVardef, VAR_TYPE &iVarType, int &iWarnMessage, std::string &lExtWord, std::string &rExtWord)
{
    //	 no warning
    iWarnMessage = 0;

    // A
    if (ExtractOneAddress(strVardef, lExtWord))
    {
        iVarType = GetVariableTypeAndTrimSymbol(&lExtWord, iWarnMessage);

        if (iVarType == VAR::NONE)
            return false;

        // find '=' in A=10,B =30
        int iEqualPos = strVardef.find("=");
        int iCommaPos = strVardef.find(",");

        if (strVardef.find("=") != string::npos && (iEqualPos < iCommaPos || iCommaPos == -1 && strVardef.size()>1))
            //	10
            if (!ExtractOneAddress(strVardef, rExtWord))
                return false;
    }
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::GetDataFromFunction(const std::string& variable, float& value, int& iWarnMessage)
{
    iWarnMessage = -1;

    if (!IsInternalFunction(variable))
        return false;

    int iLPos = variable.find("{");
    int iRPos = variable.find("}");
    std::string nonBrace = variable, withBrace = variable;

    if (iLPos != string::npos || iRPos != string::npos)
        nonBrace = nonBrace.erase(iLPos, nonBrace.size() - iLPos);

    DEFFUNC funcContents = _internalFunction[nonBrace];

    std::string  fnReturnStr = funcContents._functor;

    vector<float> arguments;
    ExtractValuesFromMidBraces(withBrace, arguments);

    if (arguments.size() != funcContents._argumentVariable.size())
        return false;

    int argPos = 0;
    int iIndex = 0;
    try {

#ifndef QT_COMPIL
        for each (auto comm in funcContents._argumentVariable)
#else
        for (auto comm : funcContents._argumentVariable)
#endif
        {
            std::stringstream ssConvert;
            ssConvert << arguments[iIndex];

            argPos = fnReturnStr.find(comm.first.c_str());
            while (argPos != string::npos)
            {
                fnReturnStr.replace(argPos, comm.first.size(), ssConvert.str());
                argPos = fnReturnStr.find(comm.first.c_str());
            }
            iIndex++;
        }

        DoRelationalOperation(fnReturnStr, iWarnMessage);
        DoLogicalOperation(fnReturnStr, iWarnMessage);
        DoMathOperation(fnReturnStr, iWarnMessage);

        value = (float) atof(fnReturnStr.c_str());
        funcContents._returnValue.SetValue(value);
        value = funcContents._returnValue.GetValue();
    }
    catch (...)
    {
        iWarnMessage = WRN::WRN_FUNCTIONCALL_FAIL;
        return false;
    }
    return true;
}

bool CRobotLangParserAndOperator::GetDataFromArray(const std::string& variable, float& value, int& iWarnMessage)
{
    VAR_TYPE iVarType = VAR_TYPE::NONE;
    iWarnMessage = -1;
    std::vector<int> DimSizes;
    std::string strLvalue, strRvalue, strVariable = variable;
    int height = 0, width = 0, depth = 0;

    if (variable.empty())
        return false;

    // ?ò÷?? ?? ???
    if (ExtractDimDefine(strVariable, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
    {
        height = (DimSizes[0] == -1) ? 0 : DimSizes[0];
        width = (DimSizes[1] == -1) ? 0 : DimSizes[1];
        depth = (DimSizes[2] == -1) ? 0 : DimSizes[2];

        if (GetDataFromArray(strLvalue, height, width, depth, value, iWarnMessage))
            return true;
        else
            return false;
    }
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::GetDataFromPose(const std::string& variable, float* value, int& iWarnMessage)
{
    VAR_TYPE iVarType = VAR_TYPE::NONE;
    iWarnMessage = -1;
    std::vector<int> DimSizes;
    std::string strLvalue, strRvalue, strVariable = variable;
    int height = 0, width = 0, depth = 0;

    if (variable.empty())
        return false;

    // ?ò÷?? ?? ???
    if (ExtractDimDefine(strVariable, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
    {
        height = (DimSizes[0] == -1) ? 0 : DimSizes[0];
        width = (DimSizes[1] == -1) ? 0 : DimSizes[1];
        depth = (DimSizes[2] == -1) ? 0 : DimSizes[2];

        if (GetDataFromPose(strLvalue, height, width, depth, value, iWarnMessage))
            return true;
        else
            return false;
    }
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::SetDataToPose(const std::string& variable, const float *fPos, int& iWarnMessage)
{
    VAR_TYPE iVarType = VAR_TYPE::NONE;
    iWarnMessage = -1;
    std::vector<int> DimSizes;
    std::string strLvalue, strRvalue, strVariable = variable;
    int height = 0, width = 0, depth = 0;

    if (variable.empty())
        return false;

    // ?ò÷?? ?? ???
    if (ExtractDimDefine(strVariable, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
    {
        if (!IsInternalPose(strLvalue))
            return false;

        height = (DimSizes[0] == -1) ? 0 : DimSizes[0];
        width = (DimSizes[1] == -1) ? 0 : DimSizes[1];
        depth = (DimSizes[2] == -1) ? 0 : DimSizes[2];

        if (SetDataToPose(strLvalue, height, width, depth, fPos, iWarnMessage))
            return true;
    }
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::SetDataToPose(const std::string& variable, int height, int width, int depth, const float* value, int& iWarnMessage)
{
    iWarnMessage = -1;

    if (variable.empty())
        return false;

    if ((bool)std::isalpha((char)variable.at(0)) == true)
    {
        try {
            auto result = _internalPoseArray.find(variable.c_str());

            if (result != _internalPoseArray.end())
            {
                DimPoseElements& array3D = result->second;

                if ((int)array3D.size() <= height)
                    return false;
                if ((int)array3D[height].size() <= width)
                    return false;
                if ((int)array3D[height][width].size() <= depth)
                    return false;

                for (int i = 0; i < TOTAXES; i++)
                {
                    (array3D[height][width][depth])._Data[i] = *(value + i);
                }

            }
            else
                return false;
        }
        catch (...)
        {
            iWarnMessage = 2;
            return false;
        }

        return true;
    }
    else
        return false;
}

bool CRobotLangParserAndOperator::SetDataToArray(const std::string& variable, float value, int& iWarnMessage)
{
    float fTmpValue = 0.0f;
    VAR_TYPE iVarType = VAR_TYPE::NONE;
    iWarnMessage = -1;
    std::vector<int> DimSizes;
    std::string strLvalue, strRvalue, strVariable = variable;
    int height = 0, width = 0, depth = 0;

    if (variable.empty())
        return false;

    // ?ò÷?? ?? ???
    if (ExtractDimDefine(strVariable, iVarType, iWarnMessage, strLvalue, DimSizes, strRvalue))
    {
        if (!IsInternalArray(strLvalue))
            return false;

        height = (DimSizes[0] == -1) ? 0 : DimSizes[0];
        width = (DimSizes[1] == -1) ? 0 : DimSizes[1];
        depth = (DimSizes[2] == -1) ? 0 : DimSizes[2];

        if(SetDataToArray(strLvalue, height, width, depth, value, iWarnMessage))
            return true;
        //GetDataFromArray(strLvalue, height, width, depth, fValue);
    }
    else
        return false;

    return true;
}

bool CRobotLangParserAndOperator::SetDataToArray(const std::string& variable, int height, int width, int depth, const float& value, int& iWarnMessage)
{
    iWarnMessage = -1;

    if (variable.empty())
        return false;

    if ((bool)std::isalpha((char)variable.at(0)) == true)
    {
        try {
            auto result = _internalArray.find(variable.c_str());

            if (result != _internalArray.end())
            {
                DimVarElements& array3D = result->second;

                if ((int) array3D.size() <= height)
                    return false;
                if ((int) array3D[height].size() <= width)
                    return false;
                if ((int) array3D[height][width].size() <= depth)
                    return false;

                array3D[height][width][depth].SetValue(value);
            }
            else
                return false;
        }
        catch (...)
        {
            iWarnMessage = 2;
            return false;
        }

        return true;
    }
    else
        return false;
}


bool CRobotLangParserAndOperator::GetDataFromPose(const std::string& variable, int height, int width, int depth, float* value, int& iWarningMsg)
{
    iWarningMsg = -1;
    if (_internalPoseArray.size() == 0)
        return false;

    if ((bool)std::isalpha((char)variable.at(0)) == true)
    {
        try {
            DimPoseElements& array3D = _internalPoseArray.at(variable);

            if ((int)array3D[height][width].size() <= depth)
                return false;
            else if ((int)array3D[height].size() <= width)
                return false;
            else if ((int)array3D.size() <= height)
                return false;

            for(int i = 0;i<TOTAXES;i++)
                value[i] = array3D[height][width][depth]._Data[i];

            return true;
        }
        catch (...)
        {
            iWarningMsg = 2;
            return false;
        }
    }
    else
        return false;
}


bool CRobotLangParserAndOperator::GetDataFromArray(const std::string& variable, int height, int width, int depth, float& value, int& iWarningMsg)
{
    iWarningMsg = -1;
    if (variable.empty())
        return false;

    if (_internalArray.size() == 0)
        return false;

    if ((bool)std::isalpha((char)variable.at(0)) == true)
    {
        try {
            DimVarElements& array3D = _internalArray.at(variable);

            if ((int) array3D[height][width].size() <= depth)
                throw depth;
            else if ((int) array3D[height].size() <= width)
                throw width;
            else if ((int) array3D.size() <= height)
                throw height;

            value = array3D[height][width][depth].GetValue();
            return true;
        }
        catch (...)
        {
            //	 Allocation error
            iWarningMsg = WRN::WRN_ALLOCATE_FAIL;
            return false;
        }
    }
    else
        return false;
}

bool CRobotLangParserAndOperator::SetDataToMap(const std::string& variable, float &value)
{	//	Get a data from map
    if (variable.empty())
        return false;

    if ((bool) std::isalpha((char) variable.at(0)) == true)
        _internalVariable[variable].SetValue(value);
    else
        return false;

    auto result = _internalVariable.find(variable);

    if (result != _internalVariable.end())
    {
        SVarData sData = result->second;
        value = sData.GetValue();
        return true;
    }
    else
        return false;
}


#ifndef QT_COMPIL
bool CRobotLangParserAndOperator::WriteUserPoseToDisk(std::string userPoseDataFileName)
{
    std::string strVariable;

    // INI ????? TP?? SAVE ??? ??? ?¥å?
    // ???? ???
    char filename[1024];
    GetModuleFileName(NULL, filename, 1024);
    CString strININame = filename;
    int index = strININame.ReverseFind('\\');
    strININame = strININame.Left(index + 1) + userPoseDataFileName.c_str();

    CFileFind finder;
    if (!finder.FindFile(strININame))
        return false;

    for (int iTempData = 1; iTempData< MAX_POINT_NUM; iTempData++) {
        // ?????? ????????
        DWORD nSize;
        CString strWriteString, strPointNum, strSavePoint;

        strPointNum.Format("P%02d", iTempData);
        strWriteString.Format("%g, %g, %g, %g, %g", *(_userPoseData[iTempData]._Data + 0), *(_userPoseData[iTempData]._Data + 1), *(_userPoseData[iTempData]._Data + 2), *(_userPoseData[iTempData]._Data + 3), *(_userPoseData[iTempData]._Data + 4));
        nSize = WritePrivateProfileString(TPSAVE_POINTLIST, strPointNum, strWriteString, strININame);

        if (nSize <= 0)
            return false;
    }
    return true;
#else

bool CRobotLangParserAndOperator::WriteUserPoseToDisk(QFile& file, std::vector<POSE>* userPoseData)
{
    if( userPoseData == nullptr)
        userPoseData = &_userPoseData;

    //Write XML
    QDomDocument document;
    QDomElement root = document.createElement("UserPoses");
    document.appendChild(root);

    //Add some elements
    for(int h = 0; h < userPoseData->size(); h++)
    {
        QDomElement pose = document.createElement("POSE");

        for(int i = 0; i < TOTAXES; i++)
        {
            pose.setAttribute("ID", QString::number(h));
            pose.setAttribute("Px", QString::number(*((*userPoseData)[h]._Data + 0)));
            pose.setAttribute("Py", QString::number(*((*userPoseData)[h]._Data + 1)));
            pose.setAttribute("Pz", QString::number(*((*userPoseData)[h]._Data + 2)));
            pose.setAttribute("Pt", QString::number(*((*userPoseData)[h]._Data + 3)));
            pose.setAttribute("Ph", QString::number(*((*userPoseData)[h]._Data + 4)));
            root.appendChild(pose);
        }
    }

    //Write to file
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writting";
        return -1;
    }
    else
    {
        QTextStream stream(&file);
        stream << document.toString();
        file.close();
        qDebug() << "Finished";
    }

#endif
}

#ifndef QT_COMPIL
bool CRobotLangParserAndOperator::ReadUserPoseFromDisk(std::string userPoseDataFileName)
{
    _userPoseData.clear();
    _userPoseData.resize(MAX_POINT_NUM);

    for(int iTempData = 1; iTempData < MAX_POINT_NUM; iTempData++)
    {
        // INI ????? TP?? SAVE ??? ??? ?¥å?
        // ???? ???
        char filename[1024];
        GetModuleFileName(NULL, filename, 1024);
        CString strININame = filename;
        int index = strININame.ReverseFind('\\');
        strININame = strININame.Left(index + 1) + userPoseDataFileName.c_str();

        CFileFind finder;
        if (!finder.FindFile(strININame))
            return false;

        // ?????? ????????
        DWORD nSize;
        TCHAR cWriteString[50];
        CString strWriteString, strPointNum, strSavePoint;

        strPointNum.Format("P%02d", iTempData);
        nSize = GetPrivateProfileString(TPSAVE_POINTLIST, strPointNum, _T(""), cWriteString, 50, strININame);

        if (nSize <= 0)
        {
            nSize = iTempData;
            strcpy(cWriteString, "0,0,0,0,0");
        }

        strWriteString = (LPCTSTR)cWriteString;

        int iForCount;

        for (iForCount = 0; iForCount < 5; ++iForCount)
        {
            nSize = strWriteString.Find(_T(","));
            strSavePoint = strWriteString.Left(nSize);
            strWriteString.Delete(0, nSize + 1);
            _userPoseData[iTempData]._Data[iForCount] = (float)_tstof(strSavePoint);
        }
        _userPoseData[iTempData]._Data[iForCount] = (float)_tstof(strWriteString);

    }
    return true;
#else
bool CRobotLangParserAndOperator::ReadUserPoseFromDisk(QFile& file, std::vector<POSE>* userPoseData)
{
    if( userPoseData == nullptr)
    {
        userPoseData = &_userPoseData;
    }

    QDomDocument document;

   //Load the file

   if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       qDebug() << "Failed to open file";
       return false;
   }
   else
   {
       if(!document.setContent(&file))
       {
           qDebug() << "Failed to load document";
           return -1;
       }
       file.close();
   }

   QDomElement root = document.firstChildElement();
   ListElements(root,"POSE", "ID", *userPoseData);

   //Get the chapters
   QDomNodeList poses = root.elementsByTagName("POSE");

   for(int i = 0; i < poses.count(); i++)
   {
       QDomNode posenode = poses.at(i);
       //convert to an element
       if(posenode.isElement())
       {
           QDomElement pose = posenode.toElement();
           qDebug() << "P" << pose.attribute("ID");

       }
   }

#endif
}

#ifdef QT_COMPIL

void CRobotLangParserAndOperator::ListElements(QDomElement root, QString tagname, QString attribute, std::vector<POSE>& userPoseData)
{
    QDomNodeList items = root.elementsByTagName(tagname);

    userPoseData.resize(items.count());

     qDebug() << "Total items = " << items.count();

     float attr[5];
     int idNum = 0;
     for(int i = 0; i < items.count(); i++)
     {
        QDomNode itemnode = items.at(i);

        //convert to element
        if(itemnode.isElement())
        {
            QDomElement itemele = itemnode.toElement();
            qDebug() << itemele.attribute("ID")<< ","<< itemele.attribute("Px")<< ","<< itemele.attribute("Py")<< ","<< itemele.attribute("Pz")<<itemele.attribute("Pt")<< ","<< itemele.attribute("Ph");
            qDebug() << "\r\n";

            idNum = itemele.attribute(attribute).toFloat();

            userPoseData[idNum]._Data[0] = itemele.attribute("Px").toFloat();
            userPoseData[idNum]._Data[1] = itemele.attribute("Py").toFloat();
            userPoseData[idNum]._Data[2] = itemele.attribute("Py").toFloat();
            userPoseData[idNum]._Data[3] = itemele.attribute("Pt").toFloat();
            userPoseData[idNum]._Data[4] = itemele.attribute("Ph").toFloat();
        }
     }
}

#endif

bool CRobotLangParserAndOperator::GetDataFromUserPose(const std::string& strVar, float fPosData[])
{
    int iTempData = 0;

    std::string strVariable = strVar;
    iTempData = strVariable.find("P");

    string pAddr = strVariable.substr(iTempData + 1, strVariable.size() - iTempData);

    if (iTempData != string::npos && iTempData == 0)
    {
        strVariable.erase(0, 1);

        if (pAddr.find("[") != string::npos && pAddr.find("]") != string::npos)
        {
            vector<int> dimSizes;
            ExtractValuesFromBigBraces(pAddr, dimSizes);

            if (dimSizes.size() > 1)
                return false;

            iTempData = dimSizes[0];
        }
        else
            iTempData = atoi(pAddr.c_str());

        if (iTempData <= 0 || iTempData > MAX_POINT_NUM)
            return false;

        // ?????? ????????

        int iForCount;
        for (iForCount = 0; iForCount < NUM_POSE_PARAM; ++iForCount)
        {
            fPosData[iForCount] = _userPoseData[iTempData]._Data[iForCount];
        }

        return true;
    }

    return false;
}

//	P1 = P1 + P2?? ???? ?????? ???? ?????? ??????? ???? ?????.
bool CRobotLangParserAndOperator::SetDataToUserPose(const std::string& strVar, float *fPosData)
{
    int iTempData = 0;
    std::string strVariable = strVar;
    iTempData = strVariable.find("P");

    string pAddr = strVariable.substr(iTempData + 1, strVariable.size() - iTempData);

    if (iTempData != string::npos)		// P ??? ??
    {
        strVariable.erase(0, 1);	// 'P'????

        if (pAddr.find("[") != string::npos && pAddr.find("]") != string::npos)
        {
            vector<int> dimSizes;
            ExtractValuesFromBigBraces(pAddr, dimSizes);

            if (dimSizes.size() > 1)
                return false;

            iTempData = dimSizes[0];
        }
        else
            iTempData = atoi(pAddr.c_str());

        if (iTempData <= 0 || iTempData > MAX_POINT_NUM)
            return false;

        // ?????? ????????

        int iForCount;
        for (iForCount = 0; iForCount < NUM_POSE_PARAM; ++iForCount)
        {
            _userPoseData[iTempData]._Data[iForCount] = *(fPosData + iForCount);
        }
    }
    return true;
}

void CRobotLangParserAndOperator::SetDataFromExternalDevice(bool sw)
{
    _bGetDataThroughExternalDevice = sw;
}


bool CRobotLangParserAndOperator::IsDataPassingByExternalDevice(void)
{
    return _bGetDataThroughExternalDevice;
}


bool CRobotLangParserAndOperator::UpdateIOport(int input, int output,int ChangedOutput)
{
    _nInputM = input;
    _nOutputM = output;
    LockSignalDataUpdate(false);

    return true;
}

bool CRobotLangParserAndOperator::UpdateInPort(int input)
{
    _nInputM = input;
    LockSignalDataUpdate(false);

    return true;
}

void CRobotLangParserAndOperator::SetDataToOut(const char* strIo, int value)
{
    if (!IsInternalPorts(strIo))
        return;

    if (_bGetDataThroughExternalDevice && GetLockSignalDataUpdateStatus())
        return;

    std::string io(strIo);

    if (io == "OUT")
    {
        _nOutputM = value;
        return;
    }

    if (value == 1)
    {
        if (io == "OUT0") _nOutputM |= 0x01;
        else if (io == "OUT1") _nOutputM |= (0x01 << 1);
        else if (io == "OUT2") _nOutputM |= (0x01 << 2);
        else if (io == "OUT3") _nOutputM |= (0x01 << 3);
        else if (io == "OUT4") _nOutputM |= (0x01 << 4);
        else if (io == "OUT5") _nOutputM |= (0x01 << 5);
        else if (io == "OUT6") _nOutputM |= (0x01 << 6);
        else if (io == "OUT7") _nOutputM |= (0x01 << 7);
    }
    else if (value == 0)
    {
        if (io == "OUT0")	   _nOutputM &= ~0x01;
        else if (io == "OUT1") _nOutputM &= ~(0x01 << 1);
        else if (io == "OUT2") _nOutputM &= ~(0x01 << 2);
        else if (io == "OUT3") _nOutputM &= ~(0x01 << 3);
        else if (io == "OUT4") _nOutputM &= ~(0x01 << 4);
        else if (io == "OUT5") _nOutputM &= ~(0x01 << 5);
        else if (io == "OUT6") _nOutputM &= ~(0x01 << 6);
        else if (io == "OUT7") _nOutputM &= ~(0x01 << 7);
    }

    SetSendingIoSignalType(1);	//	OUTPUT = 1
    /*
    if (GetSendingIoSignalToSerialPortStatus())
        LockSignalDataUpdate(true);
    */
}

bool CRobotLangParserAndOperator::GetDataFromIO(const char* strIo, int& value)
{
    std::string io = strIo;

    if (_bGetDataThroughExternalDevice && GetLockSignalDataUpdateStatus())
        return false;

    int thisIsIn = io.find("IN");
    int thisIsOut = io.find("OUT");
    int v = 0;

    if (thisIsIn != string::npos)
    {
        _iSendingIoSignalType = 0;

        int portRange = atoi(io.substr(thisIsIn + 2, io.size() - thisIsIn).c_str());
        if (portRange < 0 && portRange > 7)
            return false;

        if (io == "IN") v = _nInputM & 0xFF;
        else if (io == "IN0") v = _nInputM & 1;
        else if (io == "IN1") v = (_nInputM >> 1) & 1;
        else if (io == "IN2") v = (_nInputM >> 2) & 1;
        else if (io == "IN3") v = (_nInputM >> 3) & 1;
        else if (io == "IN4") v = (_nInputM >> 4) & 1;
        else if (io == "IN5") v = (_nInputM >> 5) & 1;
        else if (io == "IN6") v = (_nInputM >> 6) & 1;
        else if (io == "IN7") v = (_nInputM >> 7) & 1;
        else
            return false;

        if (GetSendingIoSignalToSerialPortStatus() && IsDataPassingByExternalDevice())
            LockSignalDataUpdate(true);
    }

    else if (thisIsOut != string::npos)
    {
        _iSendingIoSignalType = 1;
        int portRange = atoi(io.substr(thisIsIn + 3, io.size() - thisIsIn).c_str());
        if (portRange < 0 && portRange > 7)
            return false;

        if (io == "OUT") v = _nOutputM & 0xFF;
        else if (io == "OUT0") v = _nOutputM & 1;
        else if (io == "OUT1") v = (_nOutputM >> 1) & 1;
        else if (io == "OUT2") v = (_nOutputM >> 2) & 1;
        else if (io == "OUT3") v = (_nOutputM >> 3) & 1;
        else if (io == "OUT4") v = (_nOutputM >> 4) & 1;
        else if (io == "OUT5") v = (_nOutputM >> 5) & 1;
        else if (io == "OUT6") v = (_nOutputM >> 6) & 1;
        else if (io == "OUT7") v = (_nOutputM >> 7) & 1;
        else
            return false;
    }
    else
        return false;

    value = v;
    return true;
}


void CRobotLangParserAndOperator::SetDataToIn(const char* strIo, int value)
{
    if (!IsInternalPorts(strIo))
        return;

    std::string io(strIo);

    if (io == "IN")
    {
        GetRefInputMememory() = value;
        return;
    }

    if (value == 1)
    {
        if (io == "IN0") GetRefInputMememory() |= 0x01;
        else if (io == "IN1") GetRefInputMememory() |= (0x01 << 1);
        else if (io == "IN2") GetRefInputMememory() |= (0x01 << 2);
        else if (io == "IN3") GetRefInputMememory() |= (0x01 << 3);
        else if (io == "IN4") GetRefInputMememory() |= (0x01 << 4);
        else if (io == "IN5") GetRefInputMememory() |= (0x01 << 5);
        else if (io == "IN6") GetRefInputMememory() |= (0x01 << 6);
        else if (io == "IN7") GetRefInputMememory() |= (0x01 << 7);
    }
    else if (value == 0)
    {
        if (io == "IN0")	   GetRefInputMememory() &= ~0x01;
        else if (io == "IN1") GetRefInputMememory() &= ~(0x01 << 1);
        else if (io == "IN2") GetRefInputMememory() &= ~(0x01 << 2);
        else if (io == "IN3") GetRefInputMememory() &= ~(0x01 << 3);
        else if (io == "IN4") GetRefInputMememory() &= ~(0x01 << 4);
        else if (io == "IN5") GetRefInputMememory() &= ~(0x01 << 5);
        else if (io == "IN6") GetRefInputMememory() &= ~(0x01 << 6);
        else if (io == "IN7") GetRefInputMememory() &= ~(0x01 << 7);
    }
}
