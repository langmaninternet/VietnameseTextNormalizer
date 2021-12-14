/* ----------------------------------------------------------------- */
/*              The Vietnamese Speech Synthesis Engine               */
/*      Developed by Bui Tan Quang - langmaninternet@gmail.com       */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/* ----------------------------------------------------------------- */
#ifndef _QUANGBT_CONFIG_SYSTEM_
#define _QUANGBT_CONFIG_SYSTEM_
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string>
#include <map>
#include <set>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(push, off)
#endif
#ifdef _MSC_VER
#include <windows.h>
#include <conio.h>
#include <time.h>
#ifdef _DEBUG
#define DungManHinh _getch()
#else
#define DungManHinh /*nothing*/
#endif
#else
#include <sys/time.h>
#define DungManHinh /*nothing*/
#endif
#if defined(QBT_VALIDATE_TOOL)
#undef DungManHinh
#define DungManHinh _getch()
#endif



/************************************************************************/
/* General config                                                       */
/************************************************************************/


#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#define qwchar wchar_t
#else

#define qwchar unsigned short int
#endif


//#define QUANGBT_SPEECH_SYNTHESIS_TRACE_MEMORY_ALLOCATION


#define MAX_SYLLABLE_IN_PHRASE	40


#define NO_ENGLISH_PHONEME 

/************************************************************************/
/* Base Data Type                                                       */
/************************************************************************/


namespace std
{
	class				wstringslesscmp { public: bool operator()(const std::wstring& lhs, const std::wstring& rhs)const; };
	typedef				std::set<std::wstring, std::wstringslesscmp>						fwstringset;
}


#ifndef globalidentifier
#define globalidentifier int
#endif


#ifndef qvsylidentifier
#define qvsylidentifier short int
#endif


#ifndef qvwrdidentifier
#define qvwrdidentifier int
#endif


#ifndef qvabbsylidentifier
#define qvabbsylidentifier short int
#endif


#ifndef qvloansylidentifier
#define qvloansylidentifier  int
#endif


#ifndef qvmissingsylidentifier
#define qvmissingsylidentifier short int
#endif


#ifndef qesylidentifier
#define qesylidentifier short int
#endif


#ifndef qewrdidentifier
#define qewrdidentifier int
#endif


#ifndef qjsylidentifier
#define qjsylidentifier short int
#endif


#ifndef qjwrdidentifier
#define qjwrdidentifier int
#endif


#ifndef qinput
#define qinput	/*input*/
#endif


#ifndef qoutput
#define qoutput	/*output*/
#endif


#ifndef qexport
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)  
#define qexport extern "C"	__declspec(dllexport)
#else
#define qexport
#endif
#endif




/************************************************************************/
/* Structure Data Type                                                  */
/************************************************************************/
#ifndef WAVHEADER_DEFINITION
#define WAVHEADER_DEFINITION
#pragma pack(push,1)
typedef struct _WAVHEADE
{
	char			chunkID[4];
	int				chunkSize;
	char			format[4];
	char			subchunk1ID[4];
	int				subchunk1Size;
	unsigned short	audioFormat;
	unsigned short	numberOfChannels;
	int				sampleRate;
	int				byteRate;
	unsigned short	blockAlign;
	unsigned short	bitsPerSample;
	char			subchunk2ID[4];
	unsigned int	subchunk2Size;
} WAVHEADER;
#pragma pack(pop)
#endif


#ifndef CHARACTER_TYPE_DEFINITION
#define CHARACTER_TYPE_DEFINITION
enum CHARACTER_TYPE
{
	CHARACTER_TYPE_UNKNOWN = 0,
	CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER = 1,
	CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER = 2,
	CHARACTER_TYPE_ALPHABET_LOWER = 3,
	CHARACTER_TYPE_ALPHABET_UPPER = 4,
	CHARACTER_TYPE_NUMBER = 5,
	CHARACTER_TYPE_OTHER_ON_KEYBOARD = 6,
	CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE = 7
};
#endif
#ifndef TEXT_NODE_TYPE_DEFINITION
#define TEXT_NODE_TYPE_DEFINITION
enum TEXT_NODE_TYPE
{
	TEXT_NODE_TYPE_UNKNOWN = 0						/**/,
	TEXT_NODE_TYPE_SILENCE							/**/, /*khoảng nghỉ*/
	TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE				/**/, /*âm tiết tiếng việt*/
	TEXT_NODE_TYPE_ENGLISH_SYLLABLE					/**/, /*âm tiết tiếng anh*/
	TEXT_NODE_TYPE_ENGLISH_WORD						/**/, /*từ tiếng anh*/
	TEXT_NODE_TYPE_VENGLISH_WORD						/**/, /*từ tiếng anh nhưng đọc bằng âm tiết tiếng việt iphone, ipad, obama*/
	TEXT_NODE_TYPE_JAPANESE_WORD						/**/, /*từ tiếng nhật*/
	TEXT_NODE_TYPE_PERCENT_SIGN						/**/, /*phần trăm*/
	TEXT_NODE_TYPE_PERMILLE_SIGN						/**/, /*phần nghìn*/
	TEXT_NODE_TYPE_PERMYRIAD_SIGN						/**/, /*phần vạn*/
	TEXT_NODE_TYPE_NATURAL_NUMBER						/**/, /*số tự nhiên, đọc số*/
	TEXT_NODE_TYPE_NATURAL_NUMBER_WITH_DOT			/**/, /*số tự nhiên, đọc số*/
	TEXT_NODE_TYPE_REAL_NUMBER_WITH_COMMA				/**/, /*số tự nhiên, đọc số*/
	TEXT_NODE_TYPE_REAL_NUMBER_WITH_COMMA_DOT			/**/, /*số tự nhiên, đọc số*/
	TEXT_NODE_TYPE_NATURAL_NUMBER_X_NATURAL_NUMBER	/**/, /*số tự nhiên, đọc số*/
	TEXT_NODE_TYPE_NATURAL_NUMBER_WITH_SI_UNIT		/**/, /*Số và đơn vị đo chuẩn*/
	TEXT_NODE_TYPE_ROMAN_NUMBER						/**/, /*số la mã, đọc số*/
	TEXT_NODE_TYPE_DIGIT								/**/, /*dãy số, đánh vần*/
	TEXT_NODE_TYPE_LICENSE_PLATE_HEADER				/**/, /*phần đầu biển số xe*/
	TEXT_NODE_TYPE_LICENSE_PLATE_TAILER				/**/, /*phần đuôi biển số xe, check tứ quý*/
	TEXT_NODE_TYPE_SI_UNIT							/**/, /*Đơn vị đo chuẩn quốc tế*/
	TEXT_NODE_TYPE_DAY_PER_MONTH						/**/, /*ngày trên tháng*/
	TEXT_NODE_TYPE_MONTH_PER_YEAR						/**/, /*tháng trên năm*/
	TEXT_NODE_TYPE_DAY_PER_MONTH_PER_YEAR				/**/, /*ngày trên tháng trên năm*/
	TEXT_NODE_TYPE_FRACTION							/**/, /*phân số*/
	TEXT_NODE_TYPE_SPELL_IN_VIETNAMESE				/**/, /*Đánh vần tiếng việt*/
	TEXT_NODE_TYPE_SPELL_IN_VENGLISH					/**/, /*Đánh vần tiếng anh*/
	TEXT_NODE_TYPE_CHARACTER_AND_NUMBER				/**/, /*kí tự và số , dạng A9 C4*/
	TEXT_NODE_TYPE_HOUR								/**/, /*Số giờ*/
	TEXT_NODE_TYPE_HOUR_AND_MIN						/**/, /*Số giờ và phút*/
	TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION			/**/, /*Từ viết tắt tiếng việt*/
	TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD				/**/, /*Từ viết mượn tiếng nước ngoài*/
	TEXT_NODE_TYPE_VIETNAMESE_SPLIT_TO_READ_WORD		/**/, /*Từ viết mượn tiếng nước ngoài*/
	TEXT_NODE_TYPE_URL								/**/, /* Uniform Resource Locator */
	TEXT_NODE_TYPE_IGNORE_NODE						/**/
};
#endif


#ifndef TEXT_NODE_CAPITAL_DEFINITION
#define TEXT_NODE_CAPITAL_DEFINITION
enum TEXT_NODE_CAPITAL
{
	TEXT_NODE_CAPITAL_UNKNOWN = 0,
	TEXT_NODE_CAPITAL_LOWER = 1,
	TEXT_NODE_CAPITAL_UPPER = 2,
	TEXT_NODE_CAPITAL_CAPITAL = 4
};
#endif


#ifndef TEXT_NODE_SPLITTABLE_DEFINITION
#define TEXT_NODE_SPLITTABLE_DEFINITION
enum TEXT_NODE_SPLITTABLE
{
	TEXT_NODE_CAN_NOT_SPLIT = -1,
	TEXT_NODE_SPLITTABLE_UNKNOWN = 0,
	TEXT_NODE_ABLE_TO_SPLIT = 1
};
#endif


#ifndef TEXT_NODE_CHANGETABLE_DEFINITION
#define TEXT_NODE_CHANGETABLE_DEFINITION
enum TEXT_NODE_CHANGETABLE
{
	TEXT_NODE_CAN_NOT_CHANGE = -1,
	TEXT_NODE_CHANGETABLE_UNKNOWN = 0,
	TEXT_NODE_ABLE_TO_CHANGE = 1
};
#endif
#ifndef PHONEME_NODE_DEFINITION
#define PHONEME_NODE_DEFINITION
struct PHONEME_NODE
{
	/************************************************************************/
	/* Phoneme information                                                  */
	/************************************************************************/
	char *label;
	char leftOfLeftOfLeftPhoneme;
	char leftOfLeftPhoneme;
	char leftPhoneme;
	char currentPhoneme;
	char rightPhoneme;
	char rightOfRightPhoneme;
	char rightOfRightOfRightPhoneme;
	char phonemePositionForwardInSyllable;
	char phonemePositionReverseInSyllable;
	/************************************************************************/
	/* Linked list pointer                                                  */
	/************************************************************************/
	PHONEME_NODE * back;
	PHONEME_NODE * next;
};
#endif


/************************************************************************/
/* Macro                                                                */
/************************************************************************/
#ifndef qfprintf
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#define qfprintf fprintf_s
#else
#define qfprintf fprintf
#endif
#endif



#ifdef QUANGBT_SPEECH_SYNTHESIS_TRACE_MEMORY_ALLOCATION


#undef qcalloc
#undef qfree
void *			qdebugcalloc(int nodeCount, int nodeSize);
void			qdebugfree(void * mem);
extern int		countTotalCalloc;
extern int		countTotalFree;
#define qcalloc	qdebugcalloc
#define qfree	qdebugfree


#else


#ifndef qcalloc
#define qcalloc calloc
#endif

#ifndef qfree
#define qfree free
#endif


#endif





/************************************************************************/
/* Debug                                                                */
/************************************************************************/
#ifndef qtime
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#define qtime SYSTEMTIME
#define qGetTime(X)	GetSystemTime(X)
#define	qGetDiffTime(S,E) ((E.wHour - S.wHour)*3600. + (E.wMinute - S.wMinute)*60. + E.wSecond - S.wSecond + (E.wMilliseconds - S.wMilliseconds) / 1000.)
#else
#define qtime timeval
#define qGetTime(X)	gettimeofday(X,0)
#define	qGetDiffTime(S,E) (E.tv_sec - S.tv_sec + (E.tv_usec - S.tv_usec) / 1000000.0)
#endif
#endif







#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(pop)
#endif
#endif
