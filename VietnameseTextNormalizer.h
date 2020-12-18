#ifndef _QUANGBT_VIETNAMESE_TEXT_NORMALIZER_HEADER_
#define _QUANGBT_VIETNAMESE_TEXT_NORMALIZER_HEADER_
#include "ConfigSystem.h"
#include "SyllableSystem.h"
#include "WordSystem.h"
#include "ContextSystem.h"
#ifdef _DEBUG
#define WordSegmentConfuse printf("\n\nWord segment confuse case!\n");
#else
#define WordSegmentConfuse /*no thing*/
#endif

//#define DEBUG


struct TEXT_NODE_EXTRA
{
	qwchar const *			textLower;
	qwchar const *			textType;
	qvsylidentifier *		vsyllables;
	/************************************************************************/
	/* Alphabet character                                                   */
	/************************************************************************/
	int						textCountTotalAlphabet;			   //Vi-En
	/************************************************************************/
	/* Upper - Lower Character                                              */
	/************************************************************************/
	int						textCountLowerCharacter;		  //Vi-En
	int						textCountUpperCharacter;		  //Vi-En
	/************************************************************************/
	/* Vowel - Consonant Character                                          */
	/************************************************************************/
	int						textCountVowelCharacter;		  //Vi-En
	int						textCountConsonantCharacter;	  //Vi-En
	/************************************************************************/
	/* Vietnamese                                                           */
	/************************************************************************/
	int						textCountVietnameseOnlyCharacter;     //Vi
	/************************************************************************/
	/* English                                                              */
	/************************************************************************/
	int						textCountEnglishVowelCharacter;
	int						textCountEnglishConsonantCharacter;
	/************************************************************************/
	/* Number                                                               */
	/************************************************************************/
	int						textCountNumberCharacter;
	int						textCountZeroCharacter;
	/************************************************************************/
	/* Other                                                                */
	/************************************************************************/
	int						textCountOtherOnKeyBoard;
	int						textCountDotCharacter;
	int						textCountCommaCharacter;
	int						textCountSlashCharacter;
	/************************************************************************/
	/* Unknown Character                                                    */
	/************************************************************************/
	int						textCountUnknownCharacter;
	/************************************************************************/
	/* Number per number                                                    */
	/************************************************************************/
	int						numberValueOfGroup1;
	int						numberValueOfGroup2;
	int						numberValueOfGroup3;
	int						lengthOfGroup1;
	int						lengthOfGroup2;
	int						lengthOfGroup3;
	const qwchar * 			startOfGroup1;
	const qwchar * 			startOfGroup2;
	const qwchar * 			startOfGroup3;
	bool					flagNeedInsertNgafy;
	int						vietnameseUnitIdentifier;
	void *					memText;
	void *					memTextLower;
	void *					memTextType;
};
struct TEXT_NODE
{
	/************************************************************************/
	/* Text infomation                                                      */
	/************************************************************************/
	qwchar const *			text;
	int						textLength;
	qwchar const *			originalText;
	int						originalTextLength;
	TEXT_NODE_CAPITAL		capital;
	char					needSpaceAfter;
	/************************************************************************/
	/* Synthesis Infomation                                                 */
	/************************************************************************/
	//globalidentifier		identifier;
	qvsylidentifier			vietnameseSyllableIdentifier;
	qvwrdidentifier			vietnameseWordIdentifier;
	qvabbsylidentifier		vietnameseAbbreviationIndentifier;
	qvloansylidentifier		vietnameseLoanWordIndentifier;
	qvmissingsylidentifier	vietnameseMissingIndentifiler;
	qesylidentifier			englishSyllableIdentifier;
	qewrdidentifier			englishWordIdentifier;
	qjwrdidentifier			japaneseWordIdentifier;
	TEXT_NODE_TYPE			textNodeType;
	double					silenceTimeInSecond;
	char					leftVietnameseSyllableSure;
	char					vietnameseSyllableSure;
	char					rightVietnameseSyllableSure;
	char					leftVietnameseWordSure;
	char					vietnameseWordSure;
	char					rightVietnameseWordSure;
	char					leftVietnameseAbbreviationSure;
	char					vietnameseAbbreviationSure;
	char					rightVietnameseAbbreviationSure;
	char					leftVietnameseLoanSure;
	char					vietnameseLoanSure;
	char					rightVietnameseLoanSure;
	char					leftEnglishWordSure;
	char					englishWordSure;
	char					rightEnglishWordSure;
	/************************************************************************/
	/* Prosody                                                              */
	/************************************************************************/
	char					prosodyQ;
	char					prosodyE;
	char					prosodyL;
	char					prosodyC;
	char					prosodyB;
	char					retroflex;
	char					viendup;
	char					splittable;
	char					changeable;
	/************************************************************************/
	/* Syllable context information                                         */
	/************************************************************************/
	char					tone;
	char					stress;
	char					sopen;
	/************************************************************************/
	/* Word context information                                             */
	/************************************************************************/
	char					syllablePositionForwardInWord;
	char					syllablePositionBackwardInWord;
	char					countNumberSyllableInLeftWord;
	char					countNumberSyllableInCurrentWord;
	char					countNumberSyllableInRightWord;
	char					joinWithLastNode;
	char					joinWithNextNode;
	/************************************************************************/
	/* Phrase context information                                           */
	/************************************************************************/
	char					syllablePositionForwardInPhrase;
	char					syllablePositionBackwardInPhrase;
	char					countNumberSyllableInPhrase;
	/************************************************************************/
	/* Waveform infomation                                                  */
	/************************************************************************/
	double					waveformLengthInSecond;
	/************************************************************************/
	/* For Deverlopment                                                     */
	/************************************************************************/
	char					step;
	/************************************************************************/
	/* Phoneme context information                                          */
	/************************************************************************/
	char					phonemeLength;
	PHONEME_NODE *			phonemeStart;
	PHONEME_NODE *			phonemeEnd;
	/************************************************************************/
	/* Extra                                                                */
	/************************************************************************/
	TEXT_NODE_EXTRA *		extra;
	/************************************************************************/
	/* Linked list pointer                                                  */
	/************************************************************************/
	TEXT_NODE *				back;						/*pointer to back*/
	TEXT_NODE *				next;						/*pointer to next*/
};
class VietnameseTextNormalizer
{
private:
	/************************************************************************/
	/* Performance Optimization and Safe Return                             */
	/************************************************************************/
	TEXT_NODE				nullTextNodeForStep1Input;
	TEXT_NODE				nullTextNodeForStep2Normalize;
/*protected:*/
public:
	/************************************************************************/
	/* Text information                                                     */
	/************************************************************************/
	TEXT_NODE *				head;
	TEXT_NODE *				tail;
	TEXT_NODE *				uhead;
	TEXT_NODE *				utail;
	qwchar const *			originalText;
	int						originalTextLength;
	int						countTotalNode;
	int						countTotalUnknownNode;
	/************************************************************************/
	/* Configure                                                            */
	/************************************************************************/
	bool					flagStandardTextForTTS;
	bool					flagValidateToolMode;
	/************************************************************************/
	/* Log                                                                  */
	/************************************************************************/
	FILE *					logFile;
	double					logTime;
	/************************************************************************/
	/* Parser tool                                                          */
	/************************************************************************/
	std::fwstringset		trustedAbbreviationSet;
	std::fwstringset		trustedLoanSet;
	/************************************************************************/
	/* Configure : Pause time                                               */
	/************************************************************************/
	double					silenceOtherTime;
	double					silenceNormalTime;
	double					silenceShortTime;
	double					silenceQuotationTime;
	double					silenceColonTime;
	double					silenceCommaTime;
	double					silenceSemicolonTime;
	double					silenceSentenceTime;
	double					silenceNewLineTime;
	double					silenceStartTime;
	double					silenceEndTime;
protected:
	void					Log(const char * format, ...);
	void					Log(const qwchar * wstr, int wstrlen);
#ifdef _DEBUG
	friend void main(void);
#endif
private:
	virtual void			Init(void);
	virtual void			Refresh(void);
private:
	bool					IsValidDate(int day, int month, int year);
	void					UpdateVietnameseTextNodeContext(TEXT_NODE * textNode, TEXT_NODE * leftTextNodeOffset0, TEXT_NODE * leftTextNodeOffset1, TEXT_NODE * leftTextNodeOffset2, TEXT_NODE * leftTextNodeOffset3, TEXT_NODE * leftTextNodeOffset4);
	void					UpdateVietnameseTextNodeContext(TEXT_NODE * textNode);
	double					SignificantScore(TEXT_NODE * textNode, qvsylidentifier vietnameseSyllableIdentifier);
	double					SignificantScoreForMissingEndAndJoinProblem(TEXT_NODE * textNode, qvsylidentifier vietnameseSyllableIdentifier);
	double					PerplexityScore(TEXT_NODE * textNode, qvsylidentifier vietnameseSyllableIdentifier);
	TEXT_NODE *				InsertVietnameseSyllableToTheTail(qvsylidentifier vietnameseSyllableIdentifier, qwchar const * originalText, int originalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE * leftTextNodeOffset0, TEXT_NODE * leftTextNodeOffset1, TEXT_NODE * leftTextNodeOffset2, TEXT_NODE * leftTextNodeOffset3, TEXT_NODE * leftTextNodeOffset4);
	TEXT_NODE *				InsertEnglishWordToTheTail(qvwrdidentifier englishWordIdentifier, qwchar const * originalText, int originalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE * leftTextNodeOffset0, TEXT_NODE * leftTextNodeOffset1, TEXT_NODE * leftTextNodeOffset2, TEXT_NODE * leftTextNodeOffset3, TEXT_NODE * leftTextNodeOffset4);
	TEXT_NODE *				InsertJapaneseWordToTheTail(qjwrdidentifier japaneseWordIdentifier, qwchar const * originalText, int originalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE * leftTextNodeOffset0, TEXT_NODE * leftTextNodeOffset1, TEXT_NODE * leftTextNodeOffset2, TEXT_NODE * leftTextNodeOffset3, TEXT_NODE * leftTextNodeOffset4);
	TEXT_NODE *				InsertUnknownNodeToTail(qwchar const * originalText, int originalTextLength, TEXT_NODE * leftTextNodeOffset0, TEXT_NODE * leftTextNodeOffset1, TEXT_NODE * leftTextNodeOffset2, TEXT_NODE * leftTextNodeOffset3, TEXT_NODE * leftTextNodeOffset4);
	TEXT_NODE*				InsertShortPauseNode(TEXT_NODE* textNode);
public:
	bool					flagValidToStandard;
	qwchar *				standardText;
	int						standardTextLength;
	int						standardTextChange;
public:
	/*constructor*/			VietnameseTextNormalizer();
	void					Input(const qwchar *text);
	void					Normalize(void);
	void					GenStandardText(void);
	/*desstructor*/			~VietnameseTextNormalizer();
};








#endif



