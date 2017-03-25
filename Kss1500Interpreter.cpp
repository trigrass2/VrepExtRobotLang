#include "stdafx.h"
#include "Kss1500Interpreter.h"

#include <boost/lexical_cast.hpp>
//#include <boost/spirit/include/phoenix.hpp>
/*
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
//#include <boost/spirit.hpp>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
*/

using namespace std;
using namespace boost::spirit;
SCompiledProgramLine::SCompiledProgramLine()
{
	clear();
}
void SCompiledProgramLine::clear()
{
	switchBranch = NULL;
	intParameter[0] = 0;
	intParameter[1] = 0;
	
	if(_compiledChainedMotionProgram )
	_compiledChainedMotionProgram = NULL;

	for(int i = 0;i<7;i++) floatParameter[i] = 0.0f;
}

void SCompiledProgramLine::createSwitchBranch(void)
{
	if(!switchBranch)
		switchBranch = new map<int, int>;
}

void SCompiledProgramLine::deleteSwitchBranch(void)
{
	if(switchBranch)
	{
		delete switchBranch;
		switchBranch = NULL;
	}
}
void SCompiledProgramLine::createChainedProgramLine(void)
{
	if(_compiledChainedMotionProgram == NULL )
		_compiledChainedMotionProgram  = new std::list<SCompiledProgramLine>;
	else
		_compiledChainedMotionProgram->clear();
}

void SCompiledProgramLine::deleteChainedProgramLine(void)
{
	if(_compiledChainedMotionProgram != NULL)
	{	
		delete _compiledChainedMotionProgram;
		_compiledChainedMotionProgram = NULL;
	}
}
SCompiledProgramLine::~SCompiledProgramLine()
{

}

CKss1500Interprerter::CKss1500Interprerter(void)
{
}


CKss1500Interprerter::~CKss1500Interprerter(void)
{
}


bool CKss1500Interprerter::_extractOneLine(std::string& inputString,std::string& extractedLine)
{ // Extracts one line from a multi-line string
	extractedLine= "";
	while (inputString.length()!=0)
	{
		char c=inputString[0];
		inputString.erase(inputString.begin());
		if ( (c==char(13))||(c==char(10)) )
		{
			if (c==char(10))
				return(true);
			//				break;
		}
		else
			extractedLine+=c;
	}
	return(false);// no more code there! (but the "extractedLine" might be non-empty!)     extractedLine.length()!=0);
}

bool CKss1500Interprerter::_extractOneWord(std::string& line,std::string& extractedWord, const char* opr)
{ // Extracts one word from a string (words are delimited by spaces)
	extractedWord= "";
	
		while (line.length()!=0)
		{
			char c=line[0];
			line.erase(line.begin());

			if((strrchr(opr,c) != NULL || c== ',' || c== '>'|| c== '<'|| c== '!'|| c== '='|| c== '('|| c== ')')) // || c== '+'|| c== '-'|| c== '*'|| c== '/' shseo - ',' '(' ')' 검색 추가
			{
				if (extractedWord.length()!=0)
					return(extractedWord.length()!=0);
			}
			else
				extractedWord+=c;
		}
		return(extractedWord.length()!=0);
}

bool CKss1500Interprerter::_extractWordMathOpr(std::string& line,std::string& lExtWord, std::string& extOpr, std::string& rExtWord,std::string& wholeWord)
{ // Extracts one word from a string (words are delimited by spaces)
	if(!lExtWord.empty()) lExtWord= "";
	if(!rExtWord.empty()) rExtWord= "";
	if(!extOpr.empty()) extOpr = "";
	if(!wholeWord.empty()) wholeWord.clear();

	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if (c== ' ') 
		{
			if(!lExtWord.empty())
			{
				wholeWord.push_back(c);
				continue;	
			}
			
			lExtWord.clear();
			wholeWord.clear();
		}
		else if (c== '*' || c== '/' || c== '%')
		{
			extOpr+=c;
			wholeWord.push_back(c);
			if (lExtWord.length()!=0)
			{
				break;
			}
		}
		else if (c== '-' || c== '+')
		{
			extOpr+=c;
			wholeWord.push_back(c);
			if (lExtWord.length()!=0)
			{
				break;
			}
		}
		else
		{
			lExtWord+=c;
			wholeWord.push_back(c);
		}
	}
	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if (c==' ' || c== ',' || c== '>'|| c== '<'|| c== '!'|| c== '('|| c== ')'|| c== '=' || c== '+'|| c== '-'|| c== '*'|| c== '/') // shseo - ',' '(' ')' 검색 추가
		{
			if (rExtWord.length()!=0)
				break;
		}
		else
		{
			rExtWord+=c;
		}
		wholeWord.push_back(c);
	}
	return(lExtWord.length()!=0 && rExtWord.length()!=0 && extOpr.length()!=0);

}
bool CKss1500Interprerter::_extractWordPoseOpr(std::string& line,std::string& lExtWord, std::string& extOpr,std::string& rExtWord)
{
	if(!lExtWord.empty()) lExtWord= "";
	if(!rExtWord.empty()) rExtWord= "";
	if(!extOpr.empty()) extOpr = "";

	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if (c== ' ') 
		{
			if(!lExtWord.empty())
				continue;	
			lExtWord.clear();
		}
		else if (c== '-' || c== '+')
		{
			extOpr+=c;
			if (lExtWord.length()!=0)
			{
				break;
			}
		}
		else
		{
			lExtWord+=c;
		}
	}
	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if ( c== '('|| c== ')') // shseo - ',' '(' ')' 검색 추가
		{
			if (rExtWord.length()!=0)
			{
				break;
			}
		}
		else
		{
			rExtWord+=c;
		}
	}
	return(lExtWord.length()!=0 && rExtWord.length()!=0 && extOpr.length()!=0);

}

bool CKss1500Interprerter::_extractWordLogicOpr(std::string& line,std::string& lExtWord, std::string& extOpr, std::string& rExtWord,std::string& wholeWord)
{ // Extracts one word from a string (words are delimited by spaces)
	if(!lExtWord.empty()) lExtWord= "";
	if(!rExtWord.empty()) rExtWord= "";
	if(!extOpr.empty()) extOpr = "";
	if(!wholeWord.empty()) wholeWord.clear();

	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if (c== '&' || c== '|'|| c== '!'|| c== '^')
		{
			extOpr+=c;
			wholeWord.push_back(c);
			if (lExtWord.length()!=0)
			{
				if(line[0] != NULL)
					if(line[0] != '&' && line[0] != '|')
						break;
			}
		}
		else
		{
			lExtWord+=c;
			wholeWord.push_back(c);
		}
	}
	
	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		
		if (c==' ' || c== ',' || c== '>'|| c== '<'|| c== '!'|| c== '('|| c== ')'|| c== '=' || c== '+'|| c== '-'|| c== '*'|| c== '/') // shseo - ',' '(' ')' 검색 추가
		{
			if (rExtWord.length()!=0)
				break;
		}
		else
		{			
			rExtWord+=c;
		}
		wholeWord.push_back(c);
	}
	return(lExtWord.length()!=0 && rExtWord.length()!=0 && extOpr.length()!=0);
}

bool CKss1500Interprerter::_extractWordRelationOpr(std::string& line,std::string& lExtWord, std::string& extOpr, std::string& rExtWord,std::string& wholeWord)
{ // Extracts one word from a string (words are delimited by spaces)
	
	if(!lExtWord.empty()) lExtWord= "";
	if(!rExtWord.empty()) rExtWord= "";
	if(!extOpr.empty()) extOpr = "";
	if(!wholeWord.empty()) wholeWord.clear();

	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		if (c== ' ' || c== '|'|| c== '&'|| c== '^') 
		{
			lExtWord.clear();
			wholeWord.clear();
		}
		else if (c== ',' || c== '>'|| c== '<'|| c== '!'|| c== '('|| c== ')'|| c== '=') // shseo - ',' '(' ')' 검색 추가
		{
			extOpr+=c;
			wholeWord.push_back(c);
			if (lExtWord.length()!=0)
			{
				if(line[0] != NULL)
				  if(line[0] != '=')
					  break;
			}
		}
		else
		{
			lExtWord+=c;
			wholeWord.push_back(c);
		}
	}
	
	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());
		
		if (c==' ' || c== ',' || c== '>'|| c== '<'|| c== '!'|| c== '('|| c== ')'|| c== '=' || c== '+'|| c== '-'|| c== '*'|| c== '/') // shseo - ',' '(' ')' 검색 추가
		{
			if (rExtWord.length()!=0)
				break;
		}
		else
		{
			rExtWord+=c;
		}
		wholeWord.push_back(c);
	}
	return(lExtWord.length()!=0 && rExtWord.length()!=0 && extOpr.length()!=0);
}

bool CKss1500Interprerter::_getCommandFromWord(const std::string& word,int& command)
{ // returns the command index for a string command
	command = ID_LABEL_DEFAULT; 
	
	// "//" 추가 검사 
	if( word.size() > 2 )
	{
		if( word.substr(0, 2) == "//" ) 
		{
			command = ID_LINE_COMMENT;
			return(command != ID_LABEL_DEFAULT);
		}
	}
	
	if (word.compare("")==0)
		command = ID_BLANK;
	if (word.compare("DELAY")==0)
		command = ID_DELAY;
	if (word.compare("GOTO")==0)
		command = ID_GOTO;
	if (word.compare("//")==0)
		command = ID_LINE_COMMENT;
	if (word.compare("GOHOME")==0)
		command = ID_GOHOME;
	if (word.compare("SPEED")==0)
		command = ID_SPEED;
	if (word.compare("ROTATE")==0)
		command = ID_ROTATE;
	if (word.compare("GRASP")==0)
		command = ID_GRASP;
	if (word.compare("RELEASE")==0)
		command = ID_RELEASE;
	if (word.compare("CHANGE")==0)
		command = ID_CHANGE;
	if (word.compare("DRIVE")==0)
		command = ID_DRIVE;
	if (word.compare("MOVE")==0)
		command = ID_MOVE;
	if (word.compare("VAR")==0) //  변수 
		command = ID_VAR;
	if (word.compare("STOP")==0) 
		command = ID_STOP;
	if (word.compare("END")==0) 
		command = ID_END;
	if (word.compare("GOSUB")==0) 
		command = ID_GOSUB;
	if (word.compare("RETURN")==0) 
		command = ID_RETURN;
	if (word.compare("ELSEIF")==0) 
		command = ID_ELSEIF;
	if (word.compare("IF")==0) 
		command = ID_IF;
	if (word.compare("ELSE")==0) 
		command = ID_ELSE;
	if (word.compare("ENDIF")==0) 
		command = ID_ENDIF;
	if (word.compare("FOR")==0) 
		command = ID_FOR;
	if (word.compare("NEXT")==0) 
		command = ID_NEXT;
	if (word.compare("WAIT")==0) 
		command = ID_WAIT;
	if (word.compare("SET")==0) 
		command = ID_SET;
	if (word.compare("RESET")==0) 
		command = ID_RESET;
	if (word.compare("READY")==0) 
		command = ID_READY;
	if (word.compare("PRESET")==0)	// PRESET 명령어 추가 - 2015.08.25
		command = ID_PRESET;
	if (word.compare("SWITCH")==0)	
		command = ID_SWITCH;
	if (word.compare("ENDSWITCH")==0)	
		command = ID_ENDSWITCH;
	if (word.compare("DEFAULT")==0)	
		command = ID_DEFAULT;
	if (word.compare("CASE")==0)	
		command = ID_CASE;
	if (word.compare("BREAK")==0)	// PRESET 명령어 추가 - 2015.08.25
		command = ID_BREAK;

	if( command == ID_LABEL_DEFAULT)
	{
		int c = word.size(); 
		bool label = true; 
		for( int i=0; i<c-1; i++)
		{
			if( isalnum(word[i]) || word[i]=='_' ) 
				; 
			else
				label = false; 
		}
		if( label ) 
			command = ID_LABEL;//label
	}

	return(command!=-1);
}

void boostAction(vector<string> &v, int &lastlen, const char *begin, const char * end)
{
	string str(begin,end);
	if (!str.empty()) {
		v.clear();
		v.push_back(str);
		lastlen = 0;
	} else {
		v.resize(lastlen);
	}
}

bool CKss1500Interprerter::IsChainMotionProgram(std::string &strStatement, int& nMoveCount, int& nPoseInMoveCount )
{
	string tmpString = strStatement;
	string strExtractString;
	int moveCount = 0;
	int iSearchIndexBegin = 0;
	int iSearchIndexEnd = 0;
	float fTmpValue[7] = {0.0f,};
	int poseCount = 0;
	try
	{
	while(tmpString.size()>0)
	{
		iSearchIndexBegin = tmpString.find("(", iSearchIndexBegin);
		iSearchIndexEnd = tmpString.find(")", iSearchIndexBegin);
		if(iSearchIndexBegin != -1)
		{
			tmpString.erase(iSearchIndexBegin, iSearchIndexEnd+1);
			moveCount++;
		}
		else if(_extractOneWord(tmpString, strExtractString))
		{
			if(GetDataFromINI(strExtractString.c_str(), fTmpValue) || strExtractString.find_first_of("@") != std::string::npos)
			{
				moveCount++;
				poseCount++;
			}
		}
	}
	nMoveCount = moveCount; 
	nPoseInMoveCount = poseCount;

	if(moveCount > 1)
		return true;
	else
		return false;
	}
	catch(...)
	{
		return false;
	}
}

bool CKss1500Interprerter::_extractMultiWord(std::string& str,vector<string>& extracedWords)
{

	std::string codeLine;
	
	while(str.size()>0)
	{
		_extractOneWord(str,codeLine);
		extracedWords.push_back(codeLine);
	}

	return true;
}


bool CKss1500Interprerter::_generateSerialMotion(std::string &strStatement, SCompiledProgramLine& currentProgramLine)
{
	std::vector<std::string> parsedWords;

	std::string::size_type idx,iLft, iRht;

	iLft = strStatement.find_first_of("(");
	iRht = strStatement.find_first_of(")");

	std::string strBracket, strValue;

	while(iLft != std::string::npos && iRht != std::string::npos)
	{
		strBracket = strStatement.substr(iLft+1, iRht-iLft-1);
		currentProgramLine.command = ID_MOVE2;
		if(!_extractMultiWord(strBracket, parsedWords))
			return (false);

		int itmpIOvalue = 0;
		float ftmpValue[7] = {0.0f,};
		strBracket.clear();

		for( int i = 0; i<parsedWords.size();strBracket=strBracket+parsedWords[i], strBracket=strBracket+"`",i++ )
		{
			if(!( parsedWords[i].front() == 'R' ||  parsedWords[i].front() == 'S' 
				|| GetDataFromINI(parsedWords[i].c_str(), ftmpValue) 
				|| GetDataFromMap(parsedWords[i].c_str(), ftmpValue[0]) 
				|| _getFloatFromWord(parsedWords[i], ftmpValue[0]) 
				|| GetDataFromIO(parsedWords[i].c_str(), itmpIOvalue)))
				return false;
		}
		strBracket.erase(strBracket.end()-1);
		strStatement.replace(iLft,iRht-iLft+1, strBracket);
		iLft = strStatement.find_first_of("(");
		iRht = strStatement.find_first_of(")");
		parsedWords.clear();
		strValue.clear();
	}

	parsedWords.clear();

	SCompiledProgramLine a;
	a.command = ID_MOVE2;
	a.iLineNumber = currentProgramLine.iLineNumber;
	int iAxisCount = 0;
	int iSerialMoveCount = 0;

	while(strStatement.size()>0)
	{
		_extractOneWord(strStatement,strBracket);
		std::string subBracket(strBracket);
		std::string subXyzComp(strBracket);
		CString refinedLine(strBracket.c_str());
		refinedLine.Replace("`",",");
		bool bPuse = false;

		if(_extractOneWord(subBracket,subXyzComp,"`"))
		{
			a.strLabel[iAxisCount] = subXyzComp;
			while(subBracket.size()>0)
			{
				iAxisCount++;
				if(_extractOneWord(subBracket,subXyzComp,"`"))
				{
					float tmpFloat[7] = {0.0f,};
					a.strLabel[iAxisCount] = subXyzComp;
					bPuse = GetDataFromINI(subXyzComp.c_str(), tmpFloat); 
				}
			}
		}
		if(iAxisCount >= 3 && (!bPuse))
			a.command = ID_MOVE2;

		a.intParameter[0] = iSerialMoveCount;
		a.intParameter[1] = iAxisCount;
		a.correspondingUncompiledCode = (LPCTSTR) refinedLine;
		currentProgramLine._compiledChainedMotionProgram->push_back(a);
		iAxisCount = 0;
		iSerialMoveCount++;
	}

	currentProgramLine._compiledChainedMotionProgram->push_back(a);

	return true;
}

// parsing "10,20,30" to float []
bool CKss1500Interprerter::_extractXyzrgPosition(std::string &strStatement, float val[])
{
	std::vector<std::string> parsedWords;

	if(!_extractMultiWord(strStatement, parsedWords))
		return false;

	for( int i = 0; i<parsedWords.size();i++ )
	{
		_getFloatFromWord(parsedWords[i], val[i]);
	}
	return true;
}


bool CKss1500Interprerter::_extractXyzPosition(std::string &strStatement)
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
bool CKss1500Interprerter::_exactractOneMoveData(std::string &lExtWord, std::string &rExtWord)
{
	
	rExtWord= "";
	bool blBrace = false;
	bool brBrace = false;

	while (lExtWord.length()!=0)
	{
		char c=lExtWord[0];
		lExtWord.erase(lExtWord.begin());
		
		if( c== '(')
			blBrace = true;        //   P1+(X,Y,Z)							P2		
		else if((c== ',' && ((blBrace == true && brBrace == true) || (blBrace == false && brBrace == false))) || c == NULL)
		{
			if (rExtWord.length()!=0)
				break;
		}
		else if (c== ')')
			brBrace = true;
 
		
		rExtWord+=c;
	}
	return(rExtWord.length()!=0);

}

//	P4+(X1,10,20)
int CKss1500Interprerter::_exactractPoseData(const std::string &lPoseLabel, std::string &strStatement, SCompiledProgramLine& posData)
{
	std::string condLine, condWords, strBracket;
	std::string LWord, RWord, Opr;
	std::vector<std::string> parsedWords;
	float fValue[7] = {0.0f,};
	float fTmpValue = 0.0f;
	int idx,iLft, iRht;
	
	condLine = strStatement;

	if(lPoseLabel.size()>0)
		if(lPoseLabel.compare("NULL") != 0)
		  if(!GetDataFromINI(lPoseLabel.c_str(), fValue))	//	P1 가 없는 갑이면 반환
		  {
			return false;	
		  }
	_removeFrontAndBackSpaces(condLine);	

	posData.strLabel[4] = "POSE";//	구분
	posData.intParameter[0] = 0;	// pose operation option. 절대치 입력

	idx = condLine.find("=", 0);
	if(idx !=  std::string::npos) // = 가 없어도 반환. =제거
	{
		// S 값 처리 S = 10 일 경우
		if(condLine.at(0) == 'S')
			condWords = 'S';

		condLine.erase(0,idx+1);
		_removeFrontAndBackSpaces(condLine);	
	}

	string garString = condLine;
	
	_extractOneWord(condLine, strBracket, " +-");

	// S 값 처리 S = 10 일 경우. 마지막 라인은 Speed 처리를  해 줄 수 있도록 한다.
	if(condLine.empty() && _getFloatFromWord(strBracket, fTmpValue))
	{
		strStatement = condWords + strBracket;
		posData.command = ID_SPEED;
		return true;
	}

	idx = garString.find_first_of(POS_OP,0);

	// P1 + (10,20,0)에서 P1을 가져오는 루틴
	if(GetDataFromINI(strBracket.c_str(), fValue) || strBracket.compare("@") ==0)
	{
		posData.strLabel[0] = lPoseLabel; // P1
		posData.strLabel[1] = strBracket; // P2
		
		if(idx == std::string::npos)	//	P1 = P2
		{
			strStatement = strBracket;
			posData.intParameter[1] = 0; // 그냥 대입시 아무일도 안하도록
			GetDataFromINI(strBracket.c_str(), posData.floatParameter);	//	P1 포지션 값을 float 값에 넣는다. off라인 처리.
			return true;
		}
	}
		
	idx = garString.find_first_of("$");

	if(idx != std::string::npos)	//	Incremental 일때. 현재 안쓴다.
	{
		condLine = garString.erase(garString.find_first_of("$"), 1);
		condLine = garString;
		posData.intParameter[0] = 1;	//	1일때 증분치, 0일때 절대치
	}

	iLft = condLine.find_first_of("(");
	iRht = condLine.find_last_of(")");

	if(condLine.find("(", iLft) ==  std::string::npos && condLine.find(")", iRht) ==  std::string::npos)
	{
		if(GetDataFromINI(condLine.c_str(), fValue))
		{
			posData.strLabel[1] = condLine; // P2
			condLine = strBracket + condLine;		
			return true;
		}
	}
	else
		condLine = garString;

	try
	{
		if(!_extractWordPoseOpr(condLine, LWord, Opr, RWord))
		{
			//	MOVE (126.51, 296.48, 78),P1,P2에서 (126.51, 296.48, 78)와 같이 아무 연산이 일어나지 않는 경우
			if(iRht != -1 && posData.strLabel[0].empty() && posData.strLabel[1].empty())
			{
				posData.strLabel[0] = "NULL";
				posData.strLabel[1] = "#";
				posData.strLabel[2] = LWord;
				posData.intParameter[1] = 1;
				return true;
			}
			return false; //	P1+P2와 같은 연산을 하는 에러케이스. 연산이 안됨. 
		}

		strBracket = RWord;

		if(iLft != std::string::npos && iRht != std::string::npos)
		{
			if(!_extractMultiWord(strBracket, parsedWords))
				return (false); //	데이터 갱신 o, 프로세스 Ok 

			int itmpIOvalue = 0;
			float ftmpValue[7] = {0.0f,};
			strBracket.clear();

			posData.strLabel[1] = LWord; // P2
			
			//	(10,20,0) 처리부분
			for( int i = 0; i<parsedWords.size();strBracket=strBracket+parsedWords[i], strBracket=strBracket+",",i++ )
			{
				if(!( GetDataFromMap(parsedWords[i].c_str(), ftmpValue[i]) 
					|| _getFloatFromWord(parsedWords[i], ftmpValue[i]) ))
					return false; //	데이터 갱신 o, 프로세스 Ok
			}
			strBracket.erase(strBracket.end()-1);
			posData.strLabel[2] = strBracket;

			if(Opr.compare("+") ==0 )
				posData.intParameter[1] = 1;
			
			else if(Opr.compare("-") ==0 )
				posData.intParameter[1] = 2;
		}

		parsedWords.clear();

		//	runProgram (실시간 연산과정에서 데이터를 연산하되, 유저 프로그램 데이터에는 저장하지 않기 위해...)
		return 2;	//	데이터 갱신 x, 프로세스 Ok
	}
	catch (CMemoryException* e)
	{
		e->Delete();
		return -1; //	데이터 갱신 x, 프로세스 not OK
	}

	return 1;	//	데이터 갱신 o, 프로세스 Ok
}

void CKss1500Interprerter::GetCurrentRobotMotionInfo(const SCompiledProgramLine& currProgrram, GEO& currPos, ROT& currRos, float& currGrpPos, bool UpdateSavedPosALpha)
{
	static int MotionLineNoInOneLine = 	0;
	static GEO savedPos = currPos;
	static ROT savedRos = currRos;
	static float savedGrpPos = currGrpPos;

	float fRobotPosData[5] = {0.0f,};

	if((currProgrram.strLabel[3].compare("SAVE_CURRENTPOS") == 0 && MotionLineNoInOneLine != currProgrram.iLineNumber) || UpdateSavedPosALpha == true)	
	{
		savedPos = currPos;
		savedRos = currRos;
		savedGrpPos = currGrpPos;
		MotionLineNoInOneLine = currProgrram.iLineNumber;
	}	

	currPos = savedPos;
	currRos = savedRos;
	currGrpPos = savedGrpPos;
}

int CKss1500Interprerter::DoPoseOperation(const SCompiledProgramLine& currProgramLine , float fTargValue[], std::string& rtnPose, const GEO& currPos, const ROT& currRos, const float& currGrpPos)
{
	std::string LWord, RWord, Opr, condWords;
	std::vector<std::string> parsedWords;
	float fCurrValue[7] = {0.0f,};
	float fLvalueLabel[7] = {0.0f,};
	float fFirstLabel[7] = {0.0f,};
	float fSecondLabel[7] = {0.0f,};
	float fvValue = 0.0f;

	if(rtnPose != "NULL")
		if(rtnPose.empty() || !GetDataFromINI(currProgramLine.strLabel[0].c_str(), fLvalueLabel))
			return -1;

	// Current position
	fCurrValue[0] = currPos.fX*1000;
	fCurrValue[1] = currPos.fY*1000;
	fCurrValue[2] = currPos.fZ*1000;
	fCurrValue[3] = -currRos.fR3;		//	fR3만 쓴다.
	fCurrValue[4] = currGrpPos*1000;

	for(int i = 0;i < 7;i++) //	처음의 Label이 값이 없을때를 대비.. ex 20,30,40,40 과 같이 네개를 정의했을 때  
	{
		fFirstLabel [i] = fLvalueLabel[i];
	}

	std::string::size_type idx;

	try
	{
		if(currProgramLine.strLabel[1].compare("@") == 0) // Position 값을 불러오거나
		{
			fFirstLabel[0] = fCurrValue[0];
			fFirstLabel[1] = fCurrValue[1];
			fFirstLabel[2] = fCurrValue[2];
			fFirstLabel[3] = fCurrValue[3];		//	fR3만 쓴다.
			fFirstLabel[4] = fCurrValue[4];
		}
		else if(currProgramLine.strLabel[1].compare("#") == 0) // Position 값을 불러오거나
		{
			fFirstLabel[0] = 0;
			fFirstLabel[1] = 0;
			fFirstLabel[2] = 0;
			fFirstLabel[3] = 0;		//	fR3만 쓴다.
			fFirstLabel[4] = 0;
		}
		else
		{
			GetDataFromINI(currProgramLine.strLabel[1].c_str(), fFirstLabel); 
			condWords = currProgramLine.strLabel[1];
			
			bool bPose = false;
			bPose = GetDataFromINI(condWords.c_str(), fFirstLabel);

			if(!bPose)
			{
				if(!_extractMultiWord(condWords, parsedWords))
					return false;
				else
				{
					for(int i = 0;i < parsedWords.size();i++)
					{
						if(!GetDataFromMap(parsedWords[i].c_str(), fFirstLabel[i]))
							_getFloatFromWord(parsedWords[i], fFirstLabel[i]); 
					}
				}
			}
		}
	
		//	두번째 값 채우기
		condWords = currProgramLine.strLabel[2];

		bool bPose = false;

		if(!condWords.empty())
			bPose = GetDataFromINI(condWords.c_str(), fSecondLabel);

		if(!bPose)
		{
			if(!_extractMultiWord(condWords, parsedWords))
				return false;
			else
			{
				for(int i = 0;i < parsedWords.size();i++)
				{
					if(!GetDataFromMap(parsedWords[i].c_str(), fSecondLabel[i]))
						_getFloatFromWord(parsedWords[i], fSecondLabel[i]); 
					
					if(_compiledRobotLanguageProgram[currentProgramLine].intParameter[0] == 1) // 증분치일때
						fSecondLabel[i] = fCurrValue[i] + fSecondLabel[i];
				}
			}
		}

		float fSign = 1.0f;
		switch(_compiledRobotLanguageProgram[currentProgramLine].intParameter[1])  // 두번째는 기호
		{
			case 1: fSign = 1.0f; // +
				break;
			case 2: fSign = -1.0f; // -
				break;
			default:fSign = 0.0f;  // 두번째 항 없음.
		}

		for(int i = 0; i<5;i++)
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

int CKss1500Interprerter::DoMathOperation(std::string &strStatement)
{
	std::string LWord, RWord, Opr, condLine, condWords;

	condLine = strStatement;

	std::string::size_type idx,iLft, iRht;

	idx = condLine.find_first_of(MATH_OP);

	if ( idx == std::string::npos)
		return FALSE;

	iLft = condLine.find_first_of("=");
	if(iLft != std::string::npos)
	{
		condLine.erase(iLft,iLft+1);
		_removeFrontAndBackSpaces(condLine);
		strStatement = condLine;
	}

	iLft = condLine.find_first_of("(");
	iRht = condLine.find_last_of(")");

	try
	{

		while(iLft != std::string::npos && iRht != std::string::npos)
		{
			std::string strBracket;
			strBracket = condLine.substr(iLft+1, iRht-iLft-1);

			if(DoMathOperation(strBracket) >=0 )
			{
				condLine.replace(iLft,iRht-iLft+1, strBracket);
				strStatement = condLine;
			}
			iLft = condLine.find_first_of("(");
			iRht = condLine.find_last_of(")");
		}

		while(idx != std::string::npos)
		{
			bool bRtnValue = false;
			bRtnValue =_extractWordMathOpr(condLine, LWord, Opr,RWord, condWords);
			
			if(!bRtnValue)
				throw(new CMemoryException);

			_removeFrontAndBackSpaces(LWord);
			_removeFrontAndBackSpaces(RWord);

			float fLword = 0.0f;
			float fRword = 0.0f;
			int iLword = 0;
			int iRword = 0;
/*
			if( GetDataFromMap(LWord.c_str(), fLword) == false && !LWord.empty()) 
				fLword = atof (LWord.c_str());
			if( GetDataFromMap(RWord.c_str(), fRword) == false && !RWord.empty()) 
				fRword = atof (RWord.c_str());;
*/

			if(GetDataFromMap(LWord.c_str(), fLword))
				fLword = fLword; 
			else if(GetDataFromIO(LWord.c_str(), iLword))
				fLword = (float) iLword; 
			else
				_getFloatFromWord(LWord, fLword);


			if(GetDataFromMap(RWord.c_str(), fRword))
				fRword = fRword; 
			else if(GetDataFromIO(RWord.c_str(), iRword))
				fRword = (float) iRword; 
			else
				_getFloatFromWord(RWord, fRword);

			char tmpStr[10];
			float fResult = 0.0f;

			if (Opr == "*") fResult = fLword*fRword;
			else if (Opr == "/") fResult = fLword/fRword;
			else if (Opr == "+") fResult = fLword+fRword;
			else if (Opr == "-") fResult = fLword-fRword;
			else if (Opr == "%") fResult = ((int)fLword)%((int) fRword);

			CString strResult;
			strResult.Format("%6.4f", fResult);

			strStatement.replace(strStatement.find(condWords.c_str()),condWords.size(), (LPCTSTR) strResult);
			condLine = strStatement;

			idx = condLine.find_first_of(MATH_OP);
		}
	}
	catch (...)
	{
		strStatement.clear();
		return -1;
	}
	return TRUE;
}

int CKss1500Interprerter::DoRelationalOperation(std::string &strStatement)
{
	std::string lWord, RWord, Opr, condLine, condWords;
	
	condLine = strStatement;
	
	std::string::size_type idx;

	idx = condLine.find_first_of(RELATIONAL_OP);

	if ( idx == std::string::npos )
		return FALSE;
	
	try{
		while(idx != std::string::npos)
		{
			bool bRtnValue = false;
			bRtnValue =_extractWordRelationOpr(condLine, lWord, Opr, RWord, condWords);

			_removeFrontAndBackSpaces(lWord);
			_removeFrontAndBackSpaces(RWord);

			float fLword = 0.0f;
			float fRword = 0.0f;
			int iLword = 0;
			int iRword = 0;

			if(lWord.length() == 0 || RWord.length() == 0 || Opr.length() == 0)
				return -1;

			if(GetDataFromMap(lWord.c_str(), fLword))
				fLword = (float)fLword; 
			else if(GetDataFromIO(lWord.c_str(), iLword))
				fLword = (float)iLword; 
			else
				_getFloatFromWord(lWord.c_str(), fLword);


			if(GetDataFromMap(RWord.c_str(), fRword))
				fRword = (float)fRword; 
			else if(GetDataFromIO(RWord.c_str(), iRword))
				fRword = (float)iRword; 
			else
				_getFloatFromWord(RWord.c_str(), fRword);

			std::string strResult;

			if (Opr == "=")
				if(fLword == fRword) strResult = "1"; else strResult = "0";
			else if (Opr == ">")
				if(fLword > fRword) strResult = "1"; else strResult = "0";
			else if (Opr == "<")
				if(fLword < fRword) strResult = "1"; else strResult = "0";
			else if (Opr == "<=")
				if(fLword <= fRword) strResult = "1"; else strResult = "0";
			else if (Opr == ">=")
				if(fLword >= fRword) strResult = "1"; else strResult = "0";
			else if (Opr == "!=")
				if(fLword != fRword) strResult = "1"; else strResult = "0";
			else
				return -1;

			strStatement.replace(strStatement.find(condWords.c_str()),condWords.size(), strResult);

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
//	논리 연산자
int CKss1500Interprerter::DoLogicalOperation(std::string &strStatement)
{
	std::string lWord, RWord, Opr, condLine, condWords;

	condLine = strStatement;

	std::string::size_type idx;

	idx = condLine.find_first_of(LOGICAL_OP);

	if ( idx == std::string::npos)
		return FALSE;

	try{
		while(idx != std::string::npos)
		{
			bool bRtnValue = false;		//				 A ,  OR,   B,    A OR B
			bRtnValue =_extractWordLogicOpr(condLine, lWord, Opr, RWord, condWords);

			INT iLword = 0;
			INT iRword = 0;
			float fLword = 0;
			float fRword = 0;

			if(lWord.length() == 0 || RWord.length() == 0 || Opr.length() == 0)
				return -1;

			_removeFrontAndBackSpaces(lWord);
			_removeFrontAndBackSpaces(RWord);

			//		if( GetDataFromMap(lWord.c_str(), fLword) == false) iLword = stoi (lWord); else iLword = (int) fLword;
			//		if( GetDataFromMap(RWord.c_str(), fRword) == false) iRword = stoi (RWord); else iRword = (int) fRword;;

			if(GetDataFromMap(lWord.c_str(), fLword))
				fLword = (float)fLword; 
			else if(GetDataFromIO(lWord.c_str(), iLword))
				fLword = (float)iLword; 
			else
				_getFloatFromWord(lWord.c_str(), fLword);


			if(GetDataFromMap(RWord.c_str(), fRword))
				fRword = (float)fRword; 
			else if(GetDataFromIO(RWord.c_str(), iRword))
				fRword = (float)iRword; 
			else
				_getFloatFromWord(RWord.c_str(), fRword);


			int iResult = 0;

			if (Opr == "&") ( iResult =  (int)fLword&(int)fRword);
			else if (Opr == "&&") ( iResult = (int) fLword&&(int)fRword);
			else if (Opr == "|") ( iResult = (int) fLword|(int)fRword);
			else if (Opr == "||") ( iResult =  fLword||(int)fRword);
			else if (Opr == "!") ( iResult =  ((int)fLword)%((int) fRword));
			else if (Opr == "^") ( iResult =  ((int)fLword ^ (int)fRword));
			else return -1;

			CString strResult;
			strResult.Format("%d", iResult);

			int szStart = strStatement.find(condWords.c_str());
			strStatement.replace(szStart,condWords.size(), strResult);
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

bool CKss1500Interprerter::_getIntegerFromWord(const std::string& word,int& theInteger)
{ // returns the integer value corresponding to a string
	try
	{
		theInteger=boost::lexical_cast<int>(word);
		return(true);
	}
	catch (boost::bad_lexical_cast &)
	{
		return(false);
	}
}

bool CKss1500Interprerter::_getFloatFromWord(const std::string& word,float& theFloat)
{ // returns the float value corresponding to a string
	try
	{
		theFloat=boost::lexical_cast<float>(word);
		return(true);
	}
	catch (boost::bad_lexical_cast &)
	{
		return(false);
	}
}

//void CKss1500Interprerter::_removeFrontAndBackSpaces(std::string& word)		// 주석문 '-*', '*-' 처리 - 2015.08.11
bool CKss1500Interprerter::_removeFrontAndBackSpaces(std::string& word, bool bCommentLine)
{ // removes spaces at the front and at the back of a string
	int iFindOneLineComment;		// '//' 주석 판별용
	bool bPrevCommentMaintain = bCommentLine;
	int iFindCommentStart, iFindCommentEnd;

	iFindOneLineComment = word.find("//"); 
	iFindCommentStart = word.find("-*");
	iFindCommentEnd = word.find("*-");

	if (iFindCommentStart != -1)		// 주석 시작
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
	if (iFindOneLineComment != -1 && iFindCommentEnd > iFindOneLineComment)		// '//' 주석보다 앞에 있는 경우만
		iFindCommentEnd = -1;


	if (bCommentLine)
	{
		int iStartCount = 0;
		int iEndCount;
		int iWhileCount = 0;	// 혹시 모를 에러 카운터

		bool bFindComment = TRUE;
		while(bFindComment)
		{
			++iWhileCount;
			if (iFindCommentStart != -1)		// 주석이 시작되는 줄인 경우
				iStartCount = iFindCommentStart;

			if (iFindCommentEnd != -1)		// 주석이 끝나는 줄인 경우
				iEndCount = iFindCommentEnd +2;		// 주석문의 길이 2케릭터

			else
				iEndCount = word.length();

			// 주석문 유지/종료 조건
			if (((iStartCount-iEndCount) == -1 || (iStartCount-iEndCount) == 1) && iFindCommentStart != -1)		// '-*-', '*-*' 인 경우
			{
				if (bPrevCommentMaintain)	// 이전부터 주석문인 경우 주석 종료만 인식
				{
					iStartCount = 0;
					bCommentLine = FALSE;
				}
				else						// 주석 시작문만 인식
				{
					iEndCount = word.length();	// 주석 종료문 무시
					bCommentLine = TRUE;
				}
			}	
			else if (iFindCommentStart > iFindCommentEnd)	// 주석 유지 조건
				bCommentLine = TRUE;
			else if ((iFindCommentStart < iFindCommentEnd) && iFindCommentEnd != -1)
				bCommentLine = FALSE;

			//추가 주석 확인
			if (iFindCommentStart != -1)
			{
				if (iStartCount > iEndCount)	// 주석 시작이 뒤인경우
				{
					iFindCommentStart = iStartCount;
					iStartCount = 0;
				}
				else
				{
					iFindCommentStart = word.find("-*", iStartCount +2);
					if (iFindCommentStart > iFindOneLineComment && iFindOneLineComment != -1)	// '//' 주석보다 앞에 있는 경우만
						iFindCommentStart = -1;
				}
			}
			if (iFindCommentEnd != -1)
			{
				iFindCommentEnd = word.find("*-", iEndCount +2);
				if (iFindCommentEnd > iFindOneLineComment && iFindOneLineComment != -1)	// '//' 주석보다 앞에 있는 경우만
					iFindCommentEnd = -1;
			}
			else
				iEndCount = word.length();

			// 주석 삭제
			word.erase(iStartCount, iEndCount - iStartCount); 

			// 주석 삭제된 String 기준으로 남은 주석 다시 찾기
			iFindOneLineComment = word.find("//"); 
			iFindCommentStart = word.find("-*");
			iFindCommentEnd = word.find("*-");

			if ((iFindCommentStart == -1 && iFindCommentEnd == -1 && iStartCount == 0) ||
				(iFindCommentStart == -1 && (iStartCount < iEndCount)) ||
				iWhileCount > 10)	// 한줄에 주석 처리 횟수(무한루프대비)
				bFindComment = FALSE;
		}
	}

	iFindOneLineComment = word.find("//"); 
	if (word.length() > 2 && iFindOneLineComment >= 0)
	{
		word.erase(iFindOneLineComment, word.length() - iFindOneLineComment); 
	}
	// End - 2015.08.12

	while ( (word.length()!=0)&&( (word[0]==' ') || word[0]== 0x09)) 
		word.erase(word.begin());
	while ( (word.length()!=0)&&( (word[word.length()-1]==' ') || (word[word.length()-1]==0x09) ))
		word.erase(word.begin()+word.length()-1);

	return bCommentLine;
}


// shseo 
int CKss1500Interprerter::GetCountComma(std::string line)//,std::string& extractedWord
{ // Extracts one word from a string (words are delimited by spaces)
	//extractedWord="";

	int cnt = 0; 

	while (line.length()!=0)
	{
		char c=line[0];
		line.erase(line.begin());

		// 2014_0206 shseo change
		//if (c==' ') 
		if (c==',') 
		{
			cnt++; 
			//if (extractedWord.length()!=0)
			//return(extractedWord.length()!=0);
		}
		//else
		//extractedWord+=c;
	}
	return cnt;
}


// shseo 2014_0206 add
bool CKss1500Interprerter::GetDataFromMap(CString variable, float &value) 
{
	if(m_mIntVar.Lookup(variable, value) != 0)
		return true;
	else 
		return false; 
}


bool CKss1500Interprerter::GetDataFromINI(CString strVariable, float *fPosData) 
{
	int iTempData;
	iTempData = strVariable.Find("P");
	if (iTempData != -1)		// P 입력 시
	{
		strVariable.Delete(iTempData, 1);	// 'P'삭제
		iTempData = _ttoi(strVariable);

		if (iTempData <= 0 || iTempData > 30)
			return false;

		// INI 파일에 TP의 SAVE 위치 좌표 로드
		// 파일 확인
		char filename[1024];
		GetModuleFileName(NULL, filename, 1024);
		CString strININame = filename;
		int index = strININame.ReverseFind('\\');
		strININame = strININame.Left(index+1)  + _T(TPSAVE_DATAFILE);

		CFileFind finder;
		if (!finder.FindFile(strININame))
			return false;

		// 데이터 가져오기
		DWORD nSize;
		TCHAR cWriteString[50];
		CString strWriteString, strPointNum, strSavePoint;

		strPointNum.Format("P%02d", iTempData);
		nSize = GetPrivateProfileString(TPSAVE_POINTLIST, strPointNum, _T(""), cWriteString, 50, strININame);

		if (nSize <= 0)
			return false;

		strWriteString = (LPCTSTR)cWriteString;

		int iForCount;
		for (iForCount = 0 ; iForCount < 4 ; ++iForCount)
		{
			nSize = strWriteString.Find(_T(","));
			strSavePoint = strWriteString.Left(nSize);
			strWriteString.Delete(0, nSize+1);
			fPosData[iForCount] = _tstof(strSavePoint);
		}
		fPosData[iForCount] = _tstof(strWriteString);

		return true; 
	}
	return false; 
}

bool CKss1500Interprerter::SetDataToINI(CString strVariable, float *fPosData) 
{
	int iTempData;
	iTempData = strVariable.Find("P");
	if (iTempData != -1)		// P 입력 시
	{
		strVariable.Delete(iTempData, 1);	// 'P'삭제
		iTempData = _ttoi(strVariable);

		if (iTempData <= 0 || iTempData > 30)
			return false;

		// INI 파일에 TP의 SAVE 위치 좌표 로드
		// 파일 확인
		char filename[1024];
		GetModuleFileName(NULL, filename, 1024);
		CString strININame = filename;
		int index = strININame.ReverseFind('\\');
		strININame = strININame.Left(index+1)  + _T(TPSAVE_DATAFILE);

		CFileFind finder;
		if (!finder.FindFile(strININame))
			return false;

		// 데이터 가져오기
		DWORD nSize;
		TCHAR cWriteString[50];
		CString strWriteString, strPointNum, strSavePoint;

		strPointNum.Format("P%02d", iTempData);
		strWriteString.Format("%g, %g, %g, %g, %g", *(fPosData+0),*(fPosData+1),*(fPosData+2),*(fPosData+3),*(fPosData+4));
		nSize = WritePrivateProfileString(TPSAVE_POINTLIST, strPointNum, strWriteString, strININame);

		if (nSize <= 0)
			return false;
	
		return true; 
	}
	return false; 
}

bool CKss1500Interprerter::GetDataFromIO(CString io, int &value) 
{
	int v = 0; 

	if ( io == "IN") v = _nInputM & 0xFF;
	else if( io == "IN0") v = _nInputM & 1;
	else if( io == "IN1") v = ( _nInputM >> 1 ) & 1;
	else if( io == "IN2") v = ( _nInputM >> 2 ) & 1;
	else if( io == "IN3") v = ( _nInputM >> 3 ) & 1;
	else if( io == "IN4") v = ( _nInputM >> 4 ) & 1;
	else if( io == "IN5") v = ( _nInputM >> 5 ) & 1;
	else if( io == "IN6") v = ( _nInputM >> 6 ) & 1;
	else if( io == "IN7") v = ( _nInputM >> 7 ) & 1;
	else if( io == "OUT") v = _nOutputM & 0xFF; 
	else if( io == "OUT0") v = _nOutputM & 1; 
	else if( io == "OUT1") v = ( _nOutputM >> 1 ) & 1; 
	else if( io == "OUT2") v = ( _nOutputM >> 2 ) & 1; 
	else if( io == "OUT3") v = ( _nOutputM >> 3 ) & 1; 
	else if( io == "OUT4") v = ( _nOutputM >> 4 ) & 1; 
	else if( io == "OUT5") v = ( _nOutputM >> 5 ) & 1; 
	else if( io == "OUT6") v = ( _nOutputM >> 6 ) & 1; 
	else if( io == "OUT7") v = ( _nOutputM >> 7 ) & 1; 
	else 
	{
		return false; 
	}

	value = v; 

	return true; 
}


void CKss1500Interprerter::setProgram(const std::string& prg)
{
	_robotLanguageProgram=prg;
}

std::string CKss1500Interprerter::getProgram()
{
	return(_robotLanguageProgram);
}

void CKss1500Interprerter::compileCode(int &errline, std::string& message)
{
	currentProgramLine=0;

	POSITION pos = m_mIntVar.GetStartPosition();
	float nKey = 0;
	CString Mapmember;

	while (pos != NULL)
	{
		m_mIntVar.GetNextAssoc(pos, Mapmember, nKey);
		m_mIntVar.SetAt(Mapmember, 0); 
	}
/*
	_arrReturn2.push_back(1);
	_arrReturn2.push_back(2);
	_arrReturn2.push_back(3);
	_arrReturn2.clear();
*/
//	_arrReturn.RemoveAll();
	_arrReturn.clear();

	CString str; 
	std::vector<std::string> labels;
	std::vector<int> labelLocations;
	std::vector<int> forLocations;	// for, endID_L
	
	CSwitch firstLvSwitch;
	CIfElseThen firstIfElse;

	while(_compiledRobotLanguageProgram.size()>0) // _compiledRobotLanguageProgram.clear();
	{
		_compiledRobotLanguageProgram.back().deleteSwitchBranch();
		_compiledRobotLanguageProgram.pop_back();
	}

	std::string code(_robotLanguageProgram);
	int errorOnLineNumber=-1; // no error for now
	std::string codeLine;
	int currentCodeLineNb=0;
	float fSpeedValue = DEFAULT_ROBOT_SPEED;
	
	std::string originalLine;
	bool bCommentLine = FALSE;	// 첫줄에 코멘트 라인 초기화

	while (true)
	{ //  get one code line
		_extractOneLine(code,codeLine);
		currentCodeLineNb++;
		bCommentLine = _removeFrontAndBackSpaces(codeLine, bCommentLine);

		TRACE("compiling: %d \r\n", currentCodeLineNb); 
		errline = currentCodeLineNb; 
		
		// space만 있는 문장처리
		if( codeLine.length() == 0)
		{
			int cmd;
			_getCommandFromWord(codeLine,cmd); 

			SCompiledProgramLine a;
			a.command = cmd;
			a.correspondingUncompiledCode = originalLine;
			a.iLineNumber = currentCodeLineNb;
			_compiledRobotLanguageProgram.push_back(a);
		}
		else // if (codeLine.length()!=0)
		{
			//std::string originalLine(codeLine);
			originalLine = codeLine; 
			std::string codeWord;
			if (_extractOneWord(codeLine,codeWord))
			{ // get the first word in the code line
				int cmd;
				_getCommandFromWord(codeWord,cmd); // get the equivalent command index for the word

				if (cmd == ID_LABEL_DEFAULT)
				{
					errorOnLineNumber = currentCodeLineNb;
					break;
				}

				if (cmd == ID_LABEL)
				{ // we have a label here
					_removeFrontAndBackSpaces(codeLine);
					if (codeLine.length()==0)
					{ // the line is ok.
						// 2중 label catch
						bool b = false; 
						for( int i =0; i< labels.size(); i++)
						{
							if( labels.at(i) == codeWord )
							{
								errorOnLineNumber = currentCodeLineNb;
								b = true; 
							}
						}

						if ( b ) 
							break; 

						labels.push_back(codeWord);
						labelLocations.push_back(int(_compiledRobotLanguageProgram.size())); // 2015_0306 shseo 여기서 계속 
						
						// 2014_0309 shseo - add- THK 합병
						int tmp = int(_compiledRobotLanguageProgram.size()); 
						TRACE("tmp = %d \r\n", int(_compiledRobotLanguageProgram.size())); 
						// 2014_0309 shseo - add
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);
					}
					else
					{ 	// we have a variable here // 변수 값 할당 ex) AA = 10
						bool error = true;
						std::string variable, rValue; 
						variable = codeWord; 
						str = CA2CT(codeWord.c_str());
						rValue = codeLine;

						// 수치 연산
						int iRtnValue = 0;
						float fTmpValue[7] = {0.0f,};
						//	Pose Data 일 경우, P = P1+P2
						if((iRtnValue = GetDataFromINI(codeWord.c_str(), fTmpValue))>0)
						{
							SCompiledProgramLine a;
							a.iLineNumber = currentCodeLineNb;
							a.correspondingUncompiledCode = originalLine;
							iRtnValue =_exactractPoseData(codeWord, codeLine, a);
							a.command = ID_VALUE;

							if(iRtnValue < 0)
							{
								error = true;
								errorOnLineNumber = currentCodeLineNb;
							}
							else if (iRtnValue == 0)
								_extractXyzrgPosition(codeLine, a.floatParameter);
							
							error = false;
							
							if (iRtnValue >= 0 && iRtnValue < 2)
								SetDataToINI(codeWord.c_str(), a.floatParameter);

							_compiledRobotLanguageProgram.push_back(a);
						}
						else if(iRtnValue = (DoMathOperation(codeLine)< 0 || DoLogicalOperation(codeLine)< 0))
						{
							error = true;
							errorOnLineNumber = currentCodeLineNb;
						}
						else
						//if( m_mIntVar.Lookup(str, dumm) != 0 )
						{
							if (_extractOneWord(codeLine,codeWord))
							{
								float f;
								int i; 
								CString str = CA2CT(codeWord.c_str());
								//CString str2 = CA2CT(variable.c_str());
								
								if (_getFloatFromWord(codeWord, f) ) //GetDataFromIO(str2, i)
								{
										error = false;
										SCompiledProgramLine a;
										a.command = ID_VALUE;	//	ID_VALUE
										a.strLabel[0] = variable; 
										a.strLabel[1] = ""; 
										a.strLabel[2] = rValue; 
										a.intParameter[0] = iRtnValue;
										a.floatParameter[0] = f; 
										a.correspondingUncompiledCode = originalLine;
										a.iLineNumber = currentCodeLineNb;
										_compiledRobotLanguageProgram.push_back(a);

										m_mIntVar.SetAt(variable.c_str(), f); // 12.20.2015

								}
								else if( GetDataFromMap(str, f) || GetDataFromIO(str, i) )
								{
									error = false;
									SCompiledProgramLine a;
									a.command = ID_VALUE;
									a.correspondingUncompiledCode = originalLine;
									//a.strLabel[0] = variable; 
									//a.strLabel[1] = codeWord; 
									a.strLabel[0] = variable; 
									a.strLabel[1] = codeWord; 
									a.iLineNumber = currentCodeLineNb;
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						if (error)
						{
							errorOnLineNumber = currentCodeLineNb;
							break;
						}						
					
					}
				}
	

				// shseo 
				if (cmd == ID_GOHOME) // GOHOME
				{
					SCompiledProgramLine a;
					a.command = cmd;
					a.correspondingUncompiledCode = originalLine;
					a.iLineNumber = currentCodeLineNb;
					_compiledRobotLanguageProgram.push_back(a);

				}

				if (cmd == ID_SPEED) // SPEED - 변수처리 ok
				{
					bool error = true;
					if (_extractOneWord(codeLine,codeWord))
					{
						float v = 0.0f; 
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;

						//	 
						if (_getFloatFromWord(codeWord,v))
						{
							if (v>0)
							{
								if (!_extractOneWord(codeLine,codeWord))
								{
									error = false;
									a.floatParameter[0]=v; 
									a.strLabel[0] = ""; 
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						else if( GetDataFromMap(codeWord.c_str(), v) )
						{
							a.floatParameter[0]=0;
							a.strLabel[0] =codeWord; 
							_compiledRobotLanguageProgram.push_back(a);
						}
						else
						{
							// SPEED A*30. MathOperation 이 추가된 부분
							int iRtnValue = 0;
							a.strLabel[0] = ""; 
							a.strLabel[2] = codeWord;
							iRtnValue = DoMathOperation(codeWord);
							if(iRtnValue)
							{
								error = false;
								a.intParameter[0] = 1;
								a.floatParameter[0]= v; 
								_compiledRobotLanguageProgram.push_back(a);
							}
						}
						fSpeedValue = v;
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_ROTATE) // ROTATE - 변수처리 ok
				{
					bool error = true;
					
					if (_extractOneWord(codeLine,codeWord))
					{
						float f; 
						CString str = CA2CT(codeWord.c_str());
						if( _getFloatFromWord(codeWord,f) )
						{
							if (!_extractOneWord(codeLine,codeWord))
							{
								error = false;
								SCompiledProgramLine a;
								a.command = cmd;
								a.correspondingUncompiledCode = originalLine;
								a.floatParameter[0]=f;
								a.strLabel[0] = ""; 
								a.iLineNumber = currentCodeLineNb;
								_compiledRobotLanguageProgram.push_back(a);
							}
						}
						else if( GetDataFromMap(str, f) )
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0]=0;
							a.strLabel[0] =codeWord; 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}

					}
				
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_GRASP) // GRASP - 변수처리 ok
				{
					bool error = true;
					if (_extractOneWord(codeLine,codeWord))
					{
						float f; 
						CString str = CA2CT(codeWord.c_str());
						if( _getFloatFromWord(codeWord,f) )
						{
							if (f>=0)
							{
								if (!_extractOneWord(codeLine,codeWord))
								{
									error = false;
									SCompiledProgramLine a;
									a.command = cmd;
									a.correspondingUncompiledCode = originalLine;
									a.floatParameter[0]=f; 
									a.strLabel[0] = ""; 
									a.iLineNumber = currentCodeLineNb;
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						else if( GetDataFromMap(str, f) )
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0]=0;
							a.strLabel[0] =codeWord; 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_RELEASE) // RELEASE - 변수처리 ok
				{
					bool error = true;
					if (_extractOneWord(codeLine,codeWord))
					{
						float f; 
						CString str = CA2CT(codeWord.c_str());
						if( _getFloatFromWord(codeWord,f) )
						{
							if (f>=0)
							{
								if (!_extractOneWord(codeLine,codeWord))
								{
									error = false;
									SCompiledProgramLine a;
									a.command = cmd;
									a.correspondingUncompiledCode = originalLine;
									a.floatParameter[0]=f; 
									a.strLabel[0] = ""; 
									a.iLineNumber = currentCodeLineNb;
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						else if( GetDataFromMap(str, f) )
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0]=0;
							a.strLabel[0] =codeWord; 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_CHANGE) // CHANGE - 변수처리 ok
				{
					bool error = true;
					float f; 
					if (_extractOneWord(codeLine,codeWord))
					{
						int i;
						CString str = CA2CT(codeWord.c_str());
						if (_getIntegerFromWord(codeWord,i))
						{
							if (i>0)
							{
								if (!_extractOneWord(codeLine,codeWord))
								{
									error = false;
									SCompiledProgramLine a;
									a.command = cmd;
									a.correspondingUncompiledCode = originalLine;
									a.intParameter[0]=i; 
									a.strLabel[0] = ""; 
									a.iLineNumber = currentCodeLineNb;
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						else if( GetDataFromMap(str, f) )
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0]=0;
							a.strLabel[0] =codeWord; 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_DRIVE) // DRIVE - 변수처리 ok
				{
					bool error=true, b1=false, b2=false;
					float f1, f2; 
					std::string word; 
					CString str;
					if (_extractOneWord(codeLine,codeWord))
					{
						int i; 
						str = CA2CT(codeWord.c_str());
						if( _getIntegerFromWord(codeWord,i) )
							b1= true; 
						if( !b1)
						{
							if( GetDataFromMap(str, f1) )
							{
								word = codeWord; 
								b2=true; 
							}
						}

						if( b1 || b2 )
						{
							b1=false; 
							b2=false;
							// i 는 1~3
							if (_extractOneWord(codeLine,codeWord))
							{
								str = CA2CT(codeWord.c_str());

								if( _getFloatFromWord(codeWord,f2) )
								{
									if (!_extractOneWord(codeLine,codeWord))
									{
										error = false;
										SCompiledProgramLine a;
										a.command = cmd;
										a.correspondingUncompiledCode = originalLine;
										a.intParameter[0]=i;
										a.floatParameter[0]=f2;
										a.strLabel[0] = ""; 
										a.strLabel[1] = ""; 
										a.iLineNumber = currentCodeLineNb;
										_compiledRobotLanguageProgram.push_back(a);
									}
								}
								else if( GetDataFromMap(str, f2) )
								{
										error = false;
										SCompiledProgramLine a;
										a.command = cmd;
										a.correspondingUncompiledCode = originalLine;
										a.floatParameter[0]=0;
										a.strLabel[0] =word; 
										a.strLabel[1] =codeWord; 
										a.iLineNumber = currentCodeLineNb;
										a.iLineNumber = currentCodeLineNb;
										_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_READY) // READY
				{
					bool error = true; 

					if ( !_extractOneWord(codeLine,codeWord) )
					{
						error = false;
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.floatParameter[0]=0;
						a.floatParameter[1]=300;
						a.floatParameter[2]=0;
						a.strLabel[0] = ""; 
						a.strLabel[1] = ""; 
						a.strLabel[2] = ""; 
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);
					}

					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_MOVE ) // MOVE -ok - 변수처리 ok
				{
					bool bErrorCheck = TRUE;
					int iSpeedValue = 0;
					SCompiledProgramLine scInsertData;
					scInsertData.clear();
					int iTempData = 0;
					int iTempPoseNo = 0;
					int iTempMoveNo = 0;
					float fTempData = 0.0f;
					CString cstrTempData;
					std::string strTempData;

					// 데이터 분해
					std::list<SCompiledProgramLine> *CurrChainRobotMotion = NULL;
					std::list<SCompiledProgramLine> RealAllocatedChainBlock;

					//	Is it chained motion? and need math operation?
					if(IsChainMotionProgram(codeLine, iTempMoveNo, iTempPoseNo) == true)
					{
						// MOVE L, P001＋(0, 0, 10), ∗＋(20, 0, 0)  처리
						scInsertData.command = ID_MOVE2;
						scInsertData.strLabel[3] = "SAVE_CURRENTPOS";

						if(iTempPoseNo > 0) {
							while (codeLine.size() > 0)
							{
								_exactractOneMoveData(codeLine, strTempData);
								
								if(strTempData.size()>0)
								{
									if(_exactractPoseData("NULL", strTempData, scInsertData))
									{
										scInsertData.correspondingUncompiledCode = "MOVE " + strTempData;
										scInsertData.iLineNumber = currentCodeLineNb;
										_compiledRobotLanguageProgram.push_back(scInsertData);
										scInsertData.clear();
									}

								}
							}
							_compiledRobotLanguageProgram.back().strLabel[3] = "CLEAR_CURRENTPOS";
						}
					    else
						{
							// 
							scInsertData = _compiledRobotLanguageProgram.back();
							scInsertData.command = ID_MOVE2;
							scInsertData.correspondingUncompiledCode = codeLine;
							scInsertData.strLabel[0] = codeLine;
							scInsertData.iLineNumber = currentCodeLineNb;
							scInsertData.clear();

							scInsertData.createChainedProgramLine();

							if(_generateSerialMotion(codeLine, scInsertData))
							{
								CurrChainRobotMotion = (scInsertData._compiledChainedMotionProgram);

								if(CurrChainRobotMotion)
								{
									codeLine = (*(CurrChainRobotMotion)).front().correspondingUncompiledCode;
									scInsertData.correspondingUncompiledCode = codeLine;
									RealAllocatedChainBlock.swap(*CurrChainRobotMotion);
								}
								scInsertData = RealAllocatedChainBlock.front();
								RealAllocatedChainBlock.pop_front();
							}
							scInsertData.deleteChainedProgramLine();
							goto CHAINED_MOTION;
						}
						// (pose) was done. Chained motion is not..
						continue;
					}
					
					CHAINED_MOTION : //(XYZ),(XYZ),(XYZ)
					
					do{
						int iDatacount = 0;
						scInsertData.correspondingUncompiledCode = codeLine;
						scInsertData.iLineNumber = currentCodeLineNb;

						while (iDatacount < 5)	// 변수의 갯수 3~5개 가능
					{
							if (!_extractOneWord(codeLine,codeWord))		// 변수 갯수 에러/완료
								break;

							iTempData = codeWord.find("S");
							if (iTempData != -1)		// S값 입력 시
							{
								if (codeWord.length() > 1)	// 'SXX'로 입력한 경우. 처리해줌(변수와 겹칠 위험이 있어 에러 처리)
								{
									codeWord.erase(0,1);	// 'S'삭제
									if (!_getIntegerFromWord(codeWord, iSpeedValue))
										iSpeedValue = -1;	// 값을 획득 못하면 에러
								}
								else						// 'S=XX'로 입력한 경우
								{
									if (_extractOneWord(codeLine,codeWord))
									{
										if (!_getIntegerFromWord(codeWord, iSpeedValue))
											iSpeedValue = -1;	// 값을 획득 못하면 에러
									}
								}
								if (!ValidateSpeed((float)iSpeedValue)) // 범위 검토
									iSpeedValue = -1;	// 값 범위 초과

								++iDatacount;
								break;	// S값은 제일 마지막에 입력 조건으로 처리 후 종료
							}

							cstrTempData = CA2CT(codeWord.c_str());
							if (_getFloatFromWord(codeWord, fTempData))
							{
								scInsertData.floatParameter[iDatacount] = fTempData;
								scInsertData.strLabel[iDatacount] = "";
							}
							else if (GetDataFromMap(cstrTempData, fTempData))
							{
								scInsertData.strLabel[iDatacount] = codeWord;
							}
							else if (GetDataFromINI(cstrTempData, scInsertData.floatParameter))
							{
								if (iDatacount > 0)		// P값은 MOVE 명령어의 첫 변수로만 와야 함
								{
									iSpeedValue = -1;
									break;	// Error
								}
								scInsertData.strLabel[0] = "";
								++iDatacount;
								scInsertData.strLabel[1] = "";
								++iDatacount;
								scInsertData.strLabel[2] = "";
								++iDatacount;
								scInsertData.strLabel[3] = "";
							}
							else if (codeWord.find_first_of("@") != std::string::npos)
							{
								scInsertData.strLabel[0] = "";
								scInsertData.strLabel[1] = "";
								scInsertData.strLabel[2] = "";
								scInsertData.strLabel[3] = "";
								goto SKIP_MOTIONVALID_CHECK; // @ 현재좌표값이 없는 가정하에 계산 보류. 런타임시 체크하도록 한다.
							}
							else
							{
								iSpeedValue = -1;
								break;	// Error
							}

							++iDatacount;
						}

						// 컴파일 데이터 저장
						if (iSpeedValue > 0)		// MOVE 명령어에 속도 포함
						{
							SCompiledProgramLine stSpeedLineAdd;
							std::stringstream ssConvert;

							ssConvert << iSpeedValue;
							strTempData = "SPEED " + ssConvert.str();

							//cstrTempData.Format("SPEED %d", iSpeedValue);
							//stSpeedLineAdd.correspondingUncompiledCode = cstrTempData;
							stSpeedLineAdd.correspondingUncompiledCode = strTempData;
							stSpeedLineAdd.command = ID_SPEED;	// SPEED 명령어 번호
							stSpeedLineAdd.floatParameter[0] = (float) iSpeedValue; 
							stSpeedLineAdd.strLabel[0] = ""; 
							stSpeedLineAdd.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(stSpeedLineAdd);
						}

						if (iDatacount == 5 || (iDatacount == 4 && iSpeedValue == 0))	// MOVE2 명령어 사용 시
						{
							bErrorCheck = FALSE;

							scInsertData.command = ID_MOVE2;	// MOVE2
							scInsertData.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(scInsertData);
						}
						else if (iDatacount == 3 || (iDatacount == 4 && iSpeedValue > 0))	// 기존 MOVE 명령어 사용 시
						{
							bErrorCheck = FALSE;

							scInsertData.command = ID_MOVE;
							scInsertData.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(scInsertData);
						}
						else	// 에러
							bErrorCheck = TRUE;

						if (iSpeedValue > 0)	// MOVE 명령어에 속도 포함된 경우 사용 후 원속 복귀
						{
							SCompiledProgramLine stSpeedLineAdd;
							std::stringstream ssTemp;

							ssTemp << fSpeedValue;
							strTempData = "SPEED " + ssTemp.str();

							stSpeedLineAdd.correspondingUncompiledCode = strTempData;
							stSpeedLineAdd.command = ID_SPEED;				// SPEED 명령어 번호
							stSpeedLineAdd.floatParameter[0] = fSpeedValue; 
							stSpeedLineAdd.strLabel[0] = ""; 
							stSpeedLineAdd.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(stSpeedLineAdd);
						}
				
						// 범위(에러) 체크
						if (iSpeedValue == -1)
							bErrorCheck = true;
						if( !ValidateMove(scInsertData.floatParameter[0]/1000, scInsertData.floatParameter[1]/1000, scInsertData.floatParameter[2]/1000) ) 
							bErrorCheck = true; 
						if (!ValidateRotate(scInsertData.floatParameter[3]) && scInsertData.command == ID_MOVE2)
							bErrorCheck = true; 

						if (bErrorCheck)	// 에러 시 컴파일 종료
						{
							errorOnLineNumber = currentCodeLineNb;
							break;
						}

						SKIP_MOTIONVALID_CHECK:

						if(RealAllocatedChainBlock.size()>0)
						{
							codeLine = RealAllocatedChainBlock.front().correspondingUncompiledCode;
							cmd = RealAllocatedChainBlock.front().command;
							iSpeedValue = 0;
							scInsertData.clear();
							iTempData = 0;
							fTempData = 0.0f;
							cstrTempData.Empty();
							strTempData.clear();
							RealAllocatedChainBlock.pop_front();
						}

					} while (RealAllocatedChainBlock.size()>0 && bErrorCheck == false);
					
					if(bErrorCheck == true)	//	chain motion 에 대한 대응
						break;
				}	// MOVE End

				if (cmd == ID_VAR) // VAR - 변수 선언, ex) VAR AA
				{
					bool error = true;
					int c = GetCountComma(codeLine); 

					c++; // 변수는 하나 더 많다.
					for(int i=0 ; i < c; i++ )
					{
						if (_extractOneWord(codeLine,codeWord))
						{
							error = false; 

							str = CA2CT(codeWord.c_str());			// -0.2 

							m_mIntVar.SetAt(codeWord.c_str(), 0); 

							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}
						else
						{
							error = true; 
							break; 
						}
					}
					
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_STOP) // STOP
				{
					bool error = true;

					if (!_extractOneWord(codeLine,codeWord))
					{
						error = false;
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a); 
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}

				}

				if (cmd == ID_END) // END
				{
					bool error = true;

					if (!_extractOneWord(codeLine,codeWord))
					{
						error = false;
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a); 
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}

				}

				if (cmd == ID_IF)
				{ //IF
					bool error = true;
					firstIfElse.clear();

					if(codeLine.find("ELSE") != std::string::npos && codeLine.find("THEN") != std::string::npos )
					{
						if (_extractOneWord(codeLine,codeWord))
						{
							std::string variable = codeWord;
							if (_extractOneWord(codeLine,codeWord))
							{
								int v; 
								if (_getIntegerFromWord(codeWord,v))
								{

									if (_extractOneWord(codeLine,codeWord)) // extract "THEN"
									{
										if (_extractOneWord(codeLine,codeWord))  // "LABEL1"
										{
											std::string label1=codeWord;
											if (_extractOneWord(codeLine,codeWord)) // extract "ELSE"
											{
												if (_extractOneWord(codeLine,codeWord)) // "GOTO"
												{
													if (_extractOneWord(codeLine,codeWord)) // "LABEL2"
													{
														std::string label2=codeWord;
														if (!_extractOneWord(codeLine,codeWord))
														{
															error = false;
															SCompiledProgramLine a;
															a.command = cmd;
															a.correspondingUncompiledCode = originalLine;
															a.strLabel[1]=label1;
															a.strLabel[2]=label2;
															a.strLabel[0] = variable;
															a.intParameter[0]=v;
															a.intParameter[1]=currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
															a.iLineNumber = currentCodeLineNb;
															_compiledRobotLanguageProgram.push_back(a);
														}
													}
												}
											}
										}

									}

								}
							}
						}
					}
					else 
					{
						error = false;
						std::string::size_type idx;
						idx = codeLine.find("THEN");
						codeLine.erase(idx, codeLine.length()- idx);
						std::string rValue = codeLine;
						float fTmpVlaue = 0;
						std::string tmpString;

						// 관계 연산, 논리 연산 추가
						if (DoRelationalOperation(codeLine) < 0) error = true;
						if (DoLogicalOperation(codeLine) < 0 ) error = true;
						
						firstIfElse.bIfElseEnable_ = true;
						firstIfElse.ifElsePrevIfLine_ = currentCodeLineNb;
						firstIfElse.elseifLines.push_back(currentCodeLineNb);

						SCompiledProgramLine a;
						a.command = cmd;	
						a.correspondingUncompiledCode = originalLine;
						a.strLabel[1]="";
						a.strLabel[2]=codeLine;
						a.intParameter[1]=currentCodeLineNb;
						a.iLineNumber = currentCodeLineNb;
						a.strLabel[3]= rValue;
						_extractOneWord(rValue,tmpString);
//						a.strLabel[0]= tmpString;
						a.intParameter[0]= atoi(codeLine.c_str());
						_compiledRobotLanguageProgram.push_back(a);
						
						if(atoi(codeLine.c_str()) > 0)
						{
							firstIfElse.ifElseResultLine_ = currentCodeLineNb;
						}
					}

					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_ELSEIF)
				{ //IF
					bool error = true;
					if(codeLine.find("THEN") != std::string::npos && firstIfElse.bIfElseEnable_ == true && firstIfElse.bElseEnabled_ == false)
					{
						error = false;
						std::string::size_type idx;
						idx = codeLine.find("THEN");
						std::string rValue;
						codeLine.erase(idx, codeLine.length()- idx);
						rValue = codeLine;

						if (DoRelationalOperation(codeLine) < 0) error = true;
						if (DoLogicalOperation(codeLine) < 0 ) error = true;

						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.intParameter[0]=currentCodeLineNb;
						a.intParameter[1]=currentCodeLineNb+1;
						a.iLineNumber = currentCodeLineNb;

						a.strLabel[1]="";
						a.strLabel[2]=codeLine;
						a.strLabel[3]= rValue;

						if(firstIfElse.elseifLines.size()>0)
							_compiledRobotLanguageProgram[firstIfElse.elseifLines.back()-1].intParameter[0] = currentCodeLineNb;

						firstIfElse.elseifLines.push_back(currentCodeLineNb);
						_compiledRobotLanguageProgram.push_back(a);

						if(atoi(codeLine.c_str()) > 0)
						{
							firstIfElse.ifElseResultLine_ = currentCodeLineNb;
						}
					}

					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_ELSE)
				{ //IF
					bool error = true;
					if(firstIfElse.bIfElseEnable_ == true && firstIfElse.bElseEnabled_ == false)
					{
						error = false;
						std::string::size_type idx;

						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.intParameter[0]=currentCodeLineNb;
						a.intParameter[1]=currentCodeLineNb+1;
						a.strLabel[1]="";
						a.strLabel[2]="";
						
						if(firstIfElse.ifElseResultLine_ == 0)
						{
							firstIfElse.ifElseResultLine_ = currentCodeLineNb;
							a.strLabel[3]="1";
						}
						else
							a.strLabel[3]="0";

						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);
						firstIfElse.bElseEnabled_ = true;

						if(firstIfElse.elseifLines.size()>0)
						{
							_compiledRobotLanguageProgram[firstIfElse.elseifLines.back()-1].intParameter[0] = currentCodeLineNb;
							_compiledRobotLanguageProgram[firstIfElse.elseifLines.back()-1].intParameter[1] = currentCodeLineNb;
						}

						firstIfElse.elseifLines.push_back(currentCodeLineNb);
					}

					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_ENDIF)
				{ //IF
					bool error = true;
					if(firstIfElse.bIfElseEnable_ == true)
					{
						error = false;
						firstIfElse.bIfElseEnable_ = false;
						firstIfElse.ifElseEndLine_ = currentCodeLineNb;

						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.intParameter[0]=currentCodeLineNb;
						a.intParameter[1]=currentCodeLineNb+1;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);

						if(firstIfElse.elseifLines.size()>0)
							_compiledRobotLanguageProgram[firstIfElse.elseifLines.back()-1].intParameter[0] = currentCodeLineNb;

						while(firstIfElse.elseifLines.size()>0)
						{
							_compiledRobotLanguageProgram[firstIfElse.elseifLines.back()-1].intParameter[1] = firstIfElse.ifElseResultLine_;
							firstIfElse.elseifLines.pop_back();
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_WAIT)//WAIT
				{ 
					bool error = true;
					//	WAIT X=1, A*500
					if (_extractOneWord(codeLine,codeWord))
					{
						std::string variable = codeWord;
						if (_extractOneWord(codeLine,codeWord))
						{
							int v; 
							if (_getIntegerFromWord(codeWord,v))
							{
								if (_extractOneWord(codeLine,codeWord)) // extract "THEN"
								{
									// 5000
									int t; 
									float f;
									SCompiledProgramLine a;
									error = false;
									a.command = cmd;
									a.strLabel[2] = codeWord;	//	 A*500
									a.correspondingUncompiledCode = originalLine;
									a.strLabel[0] = variable;
									a.intParameter[0]=v;
									a.intParameter[1]=currentCodeLineNb; // the line number to jumo to is set in the second compilation pass

									if(DoMathOperation(codeWord) < 0)
										break;

									CString str = CA2CT(codeWord.c_str());
									if (_getIntegerFromWord(codeWord,t))
									{
										if (!_extractOneWord(codeLine,codeWord))
										{

											a.floatParameter[0]=float(t)/1000.0f; // convert from ms to s
											a.strLabel[1] = ""; 
										}
									}
									else if( GetDataFromMap(str, f) )
									{
										a.strLabel[1] =codeWord; 
									}
									_compiledRobotLanguageProgram.push_back(a);

								}

							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				/*
				if (cmd == ID_IN)
				{ //IN
					bool error = true;
					if (_extractOneWord(codeLine,codeWord)) // 변수
					{
						std::string variable = codeWord;
						if (_extractOneWord(codeLine,codeWord)) // INTX
						{
							std::string input = codeWord;
							if (!_extractOneWord(codeLine,codeWord))
							{
								error = false;
								SCompiledProgramLine a;
								a.command = cmd;
								a.correspondingUncompiledCode = originalLine;
								a.strLabel[0] = variable;
								a.strLabel[1]=input;
								a.intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
								_compiledRobotLanguageProgram.push_back(a);
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_OUT)
				{ //OUT
					bool error = true;
					if (_extractOneWord(codeLine,codeWord)) // 변수
					{
						std::string output = codeWord;
						if (_extractOneWord(codeLine,codeWord)) // OUTX
						{
							std::string variable = codeWord;
							if (!_extractOneWord(codeLine,codeWord))
							{
								error = false;
								SCompiledProgramLine a;
								a.command = cmd;
								a.correspondingUncompiledCode = originalLine;
								a.strLabel[0]=output;
								a.strLabel[1] = variable;
								a.intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
								_compiledRobotLanguageProgram.push_back(a);
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				*/
				if (cmd == ID_SET)
				{ //SET
					bool error = true;
					if (_extractOneWord(codeLine,codeWord)) // iNTX, OUTX
					{
						std::string variable = codeWord;

						error = false;
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.strLabel[0] = variable;
						a.intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);
						

					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_RESET)
				{ //RESET
					bool error = true;
					if (_extractOneWord(codeLine,codeWord)) // iNTX, OUTX
					{
						std::string variable = codeWord;
						if (!_extractOneWord(codeLine,codeWord))
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.strLabel[0] = variable;
							a.intParameter[0] = currentCodeLineNb; // the line number to jumo to is set in the second compilation pass
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}

					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_FOR)
				{ // we have a FOR here

					bool error = true;
					if (_extractOneWord(codeLine,codeWord)) // AA
					{
						// 변수 확인 ... 
						std::string bk_codeWord = codeWord; 
						CString variable = CA2CT(codeWord.c_str());	 
						float dumm; 
						if( GetDataFromMap(variable, dumm) ) // Variable이 정의 되어있는지 확인하는 용도 
						{
							if (_extractOneWord(codeLine,codeWord)) // 1
							{
								//수치 처리
								int min; 
								if (_getIntegerFromWord(codeWord,min))	
								{
									if (_extractOneWord(codeLine,codeWord))// TO
									{
										// TO 인지 확인 
										if( codeWord.compare("TO") ==0 )
										{
											if (_extractOneWord(codeLine,codeWord)) // 2 
											{
												// 수치 처리 
												int max; 
												if (_getIntegerFromWord(codeWord,max))
												{
													if (_extractOneWord(codeLine,codeWord))// STEP
													{
														// STEP 인지 확인 
														if( codeWord.compare("STEP") ==0 )
														{
															if (_extractOneWord(codeLine,codeWord)) // 2 
															{
																// 수치 처리 
																int step; 
																if (_getIntegerFromWord(codeWord,step))
																{
																	error = false;
																	//forLocations.push_back(int(_compiledRobotLanguageProgram.size()));
																	forLocations.push_back(currentCodeLineNb);

																	TRACE("FOR - forLocations = %d \r\n", currentCodeLineNb);
																	TRACE("FOR - min, max, step = %d, %d, %d \r\n", min, max, step);

																	SCompiledProgramLine a;
																	a.command = cmd; //for
																	a.correspondingUncompiledCode = originalLine;
																	a.intParameter[0] = min; // min
																	a.intParameter[1] = max; // max
																	a.floatParameter[0] = (float) step; // step 
																	//a.floatParameter[1] = ?; // next pos
																	a.strLabel[1] = bk_codeWord; // 변수명 
																	a.iLineNumber = currentCodeLineNb;
																	_compiledRobotLanguageProgram.push_back(a);
																}
															}
														}
													}
													else
													{
														error = false;
														//forLocations.push_back(int(_compiledRobotLanguageProgram.size()));
														forLocations.push_back(currentCodeLineNb);

														TRACE("FOR - forLocations = %d \r\n", currentCodeLineNb);
														TRACE("FOR - min, max, step = %d, %d \r\n", min, max);

														SCompiledProgramLine a;
														a.command = cmd; //for
														a.correspondingUncompiledCode = originalLine;
														a.intParameter[0]=min; // min
														a.intParameter[1]=max; // max
														a.floatParameter[0]=1; // step 
														//a.floatParameter[1]=?; // next pos
														a.strLabel[1] = bk_codeWord; // 변수명 
														a.iLineNumber = currentCodeLineNb;
														_compiledRobotLanguageProgram.push_back(a);
													}
												}
											}
										}
									}
								}
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_SWITCH)
				{ // we have a SWITCH here

					bool error = true;
					if (_extractOneWord(codeLine,codeWord) || firstLvSwitch.bSwitchEnable_  == false) // AA
					{
						// 변수 확인 ... 
						std::string bk_codeWord = codeWord; 
						CString variable = CA2CT(codeWord.c_str());	 
						float dumm; 
						if( GetDataFromMap(variable, dumm) ) // Variable이 정의 되어있는지 확인하는 용도 
						{

							// 수치 처리 
							error = false;

							TRACE("SWITCH \r\n", currentCodeLineNb);

							SCompiledProgramLine a;
							a.command = cmd; //for
							a.correspondingUncompiledCode = originalLine;
							a.intParameter[0] = (int) dumm; // min
							a.intParameter[1] = 0; // 분기될 case 라인 넘버
							a.strLabel[1] = bk_codeWord; // 변수명 
							a.iLineNumber = currentCodeLineNb;
							a.createSwitchBranch();
							_compiledRobotLanguageProgram.push_back(a);
							firstLvSwitch.bSwitchEnable_ = true;
							firstLvSwitch.switchBegineLine_ = currentCodeLineNb;
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						firstLvSwitch.bSwitchEnable_ = false; 
						break;
					}
				}

				if (cmd == ID_CASE)
				{ // we have a CASE here

					bool error = true;
					if (_extractOneWord(codeLine,codeWord) || firstLvSwitch.bSwitchEnable_  == true) // AA
					{
						// 변수 확인 ... 
						std::string bk_codeWord = codeWord; 
						CString variable = CA2CT(codeWord.c_str());	
						
						int t = 0;
						if (_getIntegerFromWord(codeWord,t))
						{

							// 수치 처리 
							error = false;

							TRACE("CASE \r\n", currentCodeLineNb);

							SCompiledProgramLine a;
							a.command = cmd; //for
							a.correspondingUncompiledCode = originalLine;
							a.intParameter[0] = (int) t; // 케이스 번호
							a.intParameter[1] = 0; // 점프될 라인 넘버
							a.strLabel[1] = bk_codeWord; // 변수명 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
//							firstLvSwitch.switchResultLine_ = currentCodeLineNb;
							firstLvSwitch.caseLines.push_back(currentCodeLineNb);

							if(firstLvSwitch.bSwitchEnable_ == true)
							{
								_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].switchBranch->insert(make_pair(t, currentCodeLineNb));

								if(_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].intParameter[0] == t
									&& _compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].iLineNumber != 0)
									{
										// 현재 케이스 라인넘버 저장
										_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].intParameter[1] = currentCodeLineNb; 
										firstLvSwitch.switchResultLine_ = currentCodeLineNb;
									}
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_DEFAULT)
				{ // we have a CASE here

					bool error = true;
					if (_extractOneWord(codeLine,codeWord) || firstLvSwitch.bSwitchEnable_  == true) // AA
					{
						// 변수 확인 ... 
						std::string bk_codeWord = codeWord; 
						CString variable = CA2CT(codeWord.c_str());	
						int t = 0xffff;

						// 수치 처리 
						error = false;

						TRACE("DEFAULT \r\n", currentCodeLineNb);

						SCompiledProgramLine a;
						a.command = cmd; //for
						a.correspondingUncompiledCode = originalLine;
						a.intParameter[0] = (int) t; // 케이스 번호
						a.intParameter[1] = currentCodeLineNb+1; // 점프될 라인 넘버
						a.strLabel[1] = bk_codeWord; // 변수명 
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a);
						firstLvSwitch.caseLines.push_back(currentCodeLineNb);

						if(firstLvSwitch.bSwitchEnable_ == true && firstLvSwitch.switchResultLine_ == 0)
						{
							_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].switchBranch->insert(make_pair(t, currentCodeLineNb));

							if(_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].iLineNumber != 0)
							{
								// 현재 케이스 라인넘버 저장
								_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].intParameter[1] = currentCodeLineNb; 
								firstLvSwitch.switchResultLine_ = currentCodeLineNb;
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_BREAK)
				{ // we have a BREAK here

					bool error = true;
					if (_extractOneWord(codeLine,codeWord) || firstLvSwitch.bSwitchEnable_  == true) // AA
					{
						error = false;

						// 변수 확인 ... 
						std::string bk_codeWord = codeWord; 
						CString variable = CA2CT(codeWord.c_str());	

							TRACE("BREAK \r\n", currentCodeLineNb);

							SCompiledProgramLine a;
							a.command = cmd; //for
							a.correspondingUncompiledCode = originalLine;
							a.intParameter[0] = (int) currentCodeLineNb; // min
							a.strLabel[1] = bk_codeWord; // 변수명 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
							firstLvSwitch.breakLines.push_back(currentCodeLineNb);

							if(firstLvSwitch.bSwitchEnable_ == true)
								if(firstLvSwitch.switchBegineLine_ > 0  
									&& firstLvSwitch.switchResultLine_ < currentCodeLineNb
									&& firstLvSwitch.switchBreakLine_ > currentCodeLineNb)
								{
									firstLvSwitch.switchBreakLine_ = currentCodeLineNb;
								}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_ENDSWITCH)
				{ // we have a ENDSWITCH here
					bool error = true;
					if( firstLvSwitch.bSwitchEnable_ == true) 
					{
						error = false;
					
						TRACE("ENDSWITCH, currentCodeLineNb %d \r\n", currentCodeLineNb);

						SCompiledProgramLine a; 
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a); 
						
						firstLvSwitch.bSwitchEnable_ = false;
						
						while (firstLvSwitch.breakLines.size()>0)
						{
							_compiledRobotLanguageProgram[firstLvSwitch.switchBegineLine_-1].intParameter[1] = currentCodeLineNb;
							_compiledRobotLanguageProgram[firstLvSwitch.breakLines.back()-1].intParameter[1] = currentCodeLineNb;
							firstLvSwitch.breakLines.pop_back();
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}

				if (cmd == ID_NEXT)  
				{ // we have a NEXT ㅠhㅍ
					
					bool error = true;
					//if (!_extractOneWord(codeLine,codeWord))
					{
						if( forLocations.size() > 0) 
						{
							error = false;

							int for_pos; 
							for_pos = forLocations.back(); // 주의 ... forLocations이 없을 경우 에러처리 추가해야 함 
							forLocations.pop_back();

							//TRACE("NEXT - for_pos = %d, currentCodeLineNb \ %d \r\n", for_pos, currentCodeLineNb);
							TRACE("NEXT - for_pos = %d, currentCodeLineNb %d \r\n", for_pos, currentCodeLineNb);

							_compiledRobotLanguageProgram[for_pos-1].floatParameter[1] = (float) currentCodeLineNb; // for문에 next 위치를 넣는다. 

							SCompiledProgramLine a; 
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0] = (float)for_pos; // next 문에 for 위치를 넣는다. 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a); 
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				
				if (cmd == ID_DELAY) // DELAY - 변수선언 
				{ // we have a DELAY here
					bool error = true;
					float f; 
					if (_extractOneWord(codeLine,codeWord))
					{
						int t;
						CString str = CA2CT(codeWord.c_str());
						if (_getIntegerFromWord(codeWord,t))
						{
							if (t>0)
							{
								if (!_extractOneWord(codeLine,codeWord))
								{
									error = false;
									SCompiledProgramLine a;
									a.command = cmd;
									a.correspondingUncompiledCode = originalLine;
									a.floatParameter[0] = float(t)/1000.0f; // convert from ms to s
									a.strLabel[0] = ""; 
									a.iLineNumber = currentCodeLineNb;
									_compiledRobotLanguageProgram.push_back(a);
								}
							}
						}
						else if( GetDataFromMap(str, f) )
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.floatParameter[0] = 0;
							a.strLabel[0] = codeWord; 
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a);
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}


				if (cmd == ID_GOTO)
				{ // we have a GOTO here
					bool error = true;
					if (_extractOneWord(codeLine,codeWord))
					{
						std::string label(codeWord);
						if (!_extractOneWord(codeLine,codeWord))
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.strLabel[1] = label;
							a.intParameter[0] = currentCodeLineNb; // the line number to jump to is set in the second compilation pass
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a); 
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_GOSUB)
				{ // we have a GOSUB here
					bool error = true;
					if (_extractOneWord(codeLine,codeWord))
					{
						std::string label(codeWord);
						if (!_extractOneWord(codeLine,codeWord))
						{
							error = false;
							SCompiledProgramLine a;
							a.command = cmd;
							a.correspondingUncompiledCode = originalLine;
							a.strLabel[1] = label;
							//a.intParameter[0] = currentCodeLineNb; // the line number to jump to is set in the second compilation pass
							a.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(a); 
							//_arrReturn.Add(currentCodeLineNb);
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				if (cmd == ID_RETURN)
				{ // we have a RETURN here
					bool error = true;

					std::string label(codeWord);
					if (!_extractOneWord(codeLine,codeWord))
					{
						error = false;
						SCompiledProgramLine a;
						a.command = cmd;
						a.correspondingUncompiledCode = originalLine;
						a.iLineNumber = currentCodeLineNb;
						_compiledRobotLanguageProgram.push_back(a); 
					}

					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}
				
				if (cmd == ID_LINE_COMMENT)
				{ // we have a comment here
					SCompiledProgramLine a;
					a.command = cmd;
					a.correspondingUncompiledCode = originalLine;
					a.iLineNumber = currentCodeLineNb;
					_compiledRobotLanguageProgram.push_back(a);
				}

				if (cmd == ID_PRESET) // PRESET
				{
					bool error = true;
					int iTempData;
					SCompiledProgramLine scInsertData;

					if (_extractOneWord(codeLine,codeWord))
					{
						iTempData = codeWord.find("OFF");

						if (iTempData != -1)	// OFF
						{
							error = false;
							scInsertData.command = cmd;
							scInsertData.correspondingUncompiledCode = originalLine;
							scInsertData.floatParameter[0] = -1;		// PRESET Cancel 처리
							scInsertData.strLabel[0] = ""; 
							scInsertData.iLineNumber = currentCodeLineNb;
							_compiledRobotLanguageProgram.push_back(scInsertData);
						}
						else					// 4개의 변수 검출
						{
							int iDatacount = 0;
              while (TRUE)
							//while (iDatacount < 4)
							{
								if (_getIntegerFromWord(codeWord,iTempData))
								{
									if (iTempData < 0)	// 변수 타입 에러
										break;

									scInsertData.floatParameter[iDatacount] = (float) iTempData;
								}
								else if (codeWord.find("IN") != -1 && iDatacount == 0)
								{
									codeWord.erase(0, 2);
									continue;
								}
								else if (codeWord.find("OUT") != -1 && iDatacount == 2)
								{
									codeWord.erase(0, 3);
									continue;
								}
								else
									break;	// 변수 타입 에러

								if (!_extractOneWord(codeLine,codeWord))		// 변수 갯수 에러/완료
									break;
								++iDatacount;

								//if (iDatacount >= 4)		// 변수 갯수 에러
								//	break;
							}

							if (iDatacount == 3)
							{
								error = false;
								scInsertData.command = cmd;
								scInsertData.correspondingUncompiledCode = originalLine;
								scInsertData.strLabel[0] = ""; 
								scInsertData.iLineNumber = currentCodeLineNb;
								_compiledRobotLanguageProgram.push_back(scInsertData);
							}
						}
					}
					if (error)
					{
						errorOnLineNumber = currentCodeLineNb;
						break;
					}
				}	// End 'PRESET'
			}
		}
		if (code.length()==0)
			break;
	}
	if (errorOnLineNumber!=-1)
	{ // we return an error message
		str.Format("Error on line: %d , %s", currentCodeLineNb, originalLine.c_str()); 
		message = str; 
		return;
	}
	else if ( forLocations.size() > 0 )
	{
		std::string retString("Error: the number of FOR and NEXT do not match");

		message = retString; 
		return;
	}
	else
	{ // we have a second pass where we need to set the line number where to jump to!
		for (unsigned int i=0;i<_compiledRobotLanguageProgram.size();i++)
		{
			if (_compiledRobotLanguageProgram[i].command == ID_GOTO)
			{ // this is a GOTO command.
				bool found=false;
				for (unsigned int j=0;j<labels.size();j++)
				{
					if (labels[j].compare(_compiledRobotLanguageProgram[i].strLabel[1])==0)
					{
						_compiledRobotLanguageProgram[i].intParameter[0]=labelLocations[j];
						found=true;
					}
				}
				if (!found)
				{
					// GOTO문 에러 메세지 처리 오류 수정 - 2015.07.31
					//std::string retString("Error on line: ");
					//str.Format("Error on line: %d, ", currentCodeLineNb); 
					errline = _compiledRobotLanguageProgram[i].intParameter[0];
					str.Format("Error on line: %d, ", errline); 
					std::string retString(str);

					//retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i].intParameter[0]);
					retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i].correspondingUncompiledCode);
					message = retString; 
					return;
				}
			}

			if (_compiledRobotLanguageProgram[i].command == ID_GOSUB)
			{ // this is a GOSUB command.
				bool found=false;
				for (unsigned int j=0;j<labels.size();j++)
				{
					if (labels[j].compare(_compiledRobotLanguageProgram[i].strLabel[1])==0)
					{
						_compiledRobotLanguageProgram[i].intParameter[0]=labelLocations[j];
						found=true;
					}
				}
				if (!found)
				{
					//std::string retString("Error on line: ");
					str.Format("Error on line: %d", currentCodeLineNb); 
					std::string retString(str);

					retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i].intParameter[0]);
					message = retString; 
					return;
				}
			}
			if (_compiledRobotLanguageProgram[i].command == ID_IF && firstIfElse.ifElsePrevIfLine_ == 0 )
			{ // this is a IF command
				bool found=false, b1=false, b2=false;
				for (unsigned int j=0;j<labels.size();j++)
				{
					if (labels[j].compare(_compiledRobotLanguageProgram[i].strLabel[1])==0)
					{
						TRACE("1 i=%d, j=%d \r\n", i, j, labelLocations[j]); 

						_compiledRobotLanguageProgram[i].intParameter[1]=labelLocations[j];
						b1=true;
					}

					if (labels[j].compare(_compiledRobotLanguageProgram[i].strLabel[2])==0)
					{
						TRACE("2 i=%d, j=%d \r\n", i, j, labelLocations[j]); 

						_compiledRobotLanguageProgram[i].intParameter[2]=labelLocations[j];
						b2=true;
					}
				}
				if (!b1 || !b2)
				{
					//std::string retString("Error on line: ");
					str.Format("Error on line: %d", currentCodeLineNb); 
					std::string retString(str);

					retString+=boost::lexical_cast<std::string>(_compiledRobotLanguageProgram[i].intParameter[1]);
					message = retString; 
					return;
				}
			}
		}
		currentProgramLineNotYetPartiallyProcessed=true;
		message = (""); // no error!
	}
	
}

