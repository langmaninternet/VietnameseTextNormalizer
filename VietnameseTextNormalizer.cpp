#include "VietnameseTextNormalizer.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <list>



/************************************************************************/
/* Initial                                                              */
/************************************************************************/
/*constructor*/		VietnameseTextNormalizer::VietnameseTextNormalizer()
{
	ValidateSyllableData();
	ValidateWordData();
	flagStandardTextForTTS = false;
	flagValidateToolMode = false;
	Init();

	/************************************************************************/
	/* Performance Optimization and Safe Return                             */
	/************************************************************************/
	memset(&nullTextNodeForStep1Input, 0, sizeof(nullTextNodeForStep1Input));
	memset(&nullTextNodeForStep2Normalize, 0, sizeof(nullTextNodeForStep2Normalize));
}
void				VietnameseTextNormalizer::Init(void)
{
	head = 0;
	tail = 0;
	uhead = 0;
	utail = 0;

	flagValidToStandard = true;
	standardText = 0;
	standardTextLength = 0;
	standardTextChange = 0;
	originalText = 0;
	originalTextLength = 0;
	countTotalNode = 0;
	countTotalUnknownNode = 0;



	silenceOtherTime = 0.0;
	silenceNormalTime = 0.0;
	silenceShortTime = 0.0;
	silenceQuotationTime = 0.0;

	silenceColonTime = 0.0;
	silenceCommaTime = 0.0;
	silenceSemicolonTime = 0.0;
	silenceSentenceTime = 0.0;

	silenceNewLineTime = 0.0;
	silenceStartTime = 0.0;
	silenceEndTime = 0.0;


	/* log */
	logFile = 0;
	logTime = 0.0;


}
/************************************************************************/
/* Log function                                                         */
/************************************************************************/
void				VietnameseTextNormalizer::Log(const char* format, ...)
{
	if (logFile/*!=NULL*/)
	{
		char* buffLog = (char*)qcalloc(2000 + 10/*safe*/, sizeof(char));
		if (buffLog)
		{
			va_list args;
			va_start(args, format);
#ifdef WIN32
			vsnprintf_s(buffLog, 2000, _TRUNCATE, format, args);
#else
			vsnprintf(buffLog, 2000, format, args);
#endif

			qfprintf(logFile, "%s", buffLog);
			fflush(logFile);

#ifdef WIN32
#ifdef _DEBUG
			printf("%s", buffLog);
#endif
#endif
			qfree(buffLog);
		}
		else
		{
			printf("\nVietnameseTextNormalizer : Calloc fail!");
			DungManHinh;
		}
	}
}
void				VietnameseTextNormalizer::Log(const qwchar* wstr, int wstrlen)
{
	if (logFile/*!=NULL*/ && wstr/*!=NULL*/ && wstrlen > 0)
	{
		if (wstrlen == 1 && wstr[0] == 0xA/*\n Line Feed*/) Log("\\n");
		else if (wstrlen == 1 && wstr[0] == 0xD/*\r Line Feed*/) Log("\\r");
		else
		{
			unsigned char* bufferUtf8 = (unsigned char*)qcalloc(wstrlen * 3 + 5/*Safe*/, sizeof(unsigned char));
			if (bufferUtf8)
			{
				unsigned char* bufferUtf8Iterator = bufferUtf8;
				int length = 0;
				for (int counter = 0; counter < wstrlen; counter++, wstr++)
				{
					qwchar temp_ucs2str_value = *wstr;
					if (0x0080 > temp_ucs2str_value)
					{
						/* 1 byte UTF-8 Character.*/
						*bufferUtf8Iterator = (unsigned char)(temp_ucs2str_value);
						length++;
						bufferUtf8Iterator++;
					}
					else if (0x0800 > temp_ucs2str_value)
					{
						if (bufferUtf8Iterator != bufferUtf8 && (*(bufferUtf8Iterator - 1) == 20))
						{
							*bufferUtf8Iterator = (unsigned char)(temp_ucs2str_value);
							length++;
							bufferUtf8Iterator++;
							continue;
						}
						/*2 bytes UTF-8 Character.*/
						*bufferUtf8Iterator = (unsigned char)(((unsigned char)(temp_ucs2str_value >> 6)) | 0xc0);
						*(bufferUtf8Iterator + 1) = (unsigned char)(((unsigned char)(temp_ucs2str_value & 0x003F)) | 0x80);
						length += 2;
						bufferUtf8Iterator += 2;
					}
					else
					{
						/* 3 bytes UTF-8 Character .*/
						*bufferUtf8Iterator = (unsigned char)(((unsigned char)(temp_ucs2str_value >> 12)) | 0xE0);
						*(bufferUtf8Iterator + 1) = (unsigned char)(((unsigned char)((temp_ucs2str_value & 0x0FC0) >> 6)) | 0x80);
						*(bufferUtf8Iterator + 2) = (unsigned char)(((unsigned char)(temp_ucs2str_value & 0x003F)) | 0x80);
						length += 3;
						bufferUtf8Iterator += 3;
					}
				}
				*bufferUtf8Iterator = 0;
				Log("%s", bufferUtf8);
				qfree(bufferUtf8);
			}
			else
			{
				printf("\nVietnameseTextNormalizer : Calloc fail!");
				DungManHinh;
			}
		}
	}
}
/************************************************************************/
/* Score                                                                */
/************************************************************************/
double				VietnameseTextNormalizer::SignificantScore(TEXT_NODE* textNode, qvsylidentifier vietnameseSyllableIdentifier)
{
	TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset5 = &nullTextNodeForStep2Normalize;
	if (textNode->back)
	{
		leftTextNodeOffset0 = textNode->back;
		if (leftTextNodeOffset0->back)
		{
			leftTextNodeOffset1 = leftTextNodeOffset0->back;
			if (leftTextNodeOffset1->back)
			{
				leftTextNodeOffset2 = leftTextNodeOffset1->back;
				if (leftTextNodeOffset2->back)
				{
					leftTextNodeOffset3 = leftTextNodeOffset2->back;
					if (leftTextNodeOffset3->back)
					{
						leftTextNodeOffset4 = leftTextNodeOffset3->back;
						if (leftTextNodeOffset4->back)
						{
							leftTextNodeOffset5 = leftTextNodeOffset4->back;
						}
					}
				}
			}
		}
	}
	TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset5 = &nullTextNodeForStep2Normalize;
	if (textNode->next)
	{
		rightTextNodeOffset0 = textNode->next;
		if (rightTextNodeOffset0->next)
		{
			rightTextNodeOffset1 = rightTextNodeOffset0->next;
			if (rightTextNodeOffset1->next)
			{
				rightTextNodeOffset2 = rightTextNodeOffset1->next;
				if (rightTextNodeOffset2->next)
				{
					rightTextNodeOffset3 = rightTextNodeOffset2->next;
					if (rightTextNodeOffset3->next)
					{
						rightTextNodeOffset4 = rightTextNodeOffset3->next;
						if (rightTextNodeOffset4->next)
						{
							rightTextNodeOffset5 = rightTextNodeOffset4->next;
						}
					}
				}
			}
		}
	}
	qvsylidentifier leftTextNodeOffset0VietnameseSyllableIdentifier = leftTextNodeOffset0->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset1VietnameseSyllableIdentifier = leftTextNodeOffset1->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset2VietnameseSyllableIdentifier = leftTextNodeOffset2->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset3VietnameseSyllableIdentifier = leftTextNodeOffset3->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset4VietnameseSyllableIdentifier = leftTextNodeOffset4->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset5VietnameseSyllableIdentifier = leftTextNodeOffset5->vietnameseSyllableIdentifier;


	if (textNode->capital == TEXT_NODE_CAPITAL_LOWER)
	{
		if (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset5VietnameseSyllableIdentifier = 0;
	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_UPPER)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset3->textLength == 1)))  leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset4->textLength == 1)))  leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset5->textLength == 1)))  leftTextNodeOffset5VietnameseSyllableIdentifier = 0;

	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_CAPITAL)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset3->textLength == 1)))  leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset4->textLength == 1)))  leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset5->textLength == 1)))  leftTextNodeOffset5VietnameseSyllableIdentifier = 0;
	}



	double currentSyllableSure =
		/************************************************************************/
		/* Right Sure                                                           */
		/************************************************************************/
		vnsyllables[vietnameseSyllableIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier)
		+ (vnsyllables[leftTextNodeOffset0VietnameseSyllableIdentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) > 2)
		+ (vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) > 3)
		+ (vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 4)
		+ (vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 5)

		+ (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].RightSure(leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1)
		+ vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier)
		/************************************************************************/
		/* Left Sure                                                            */
		/************************************************************************/
		+vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier)
		+ (vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1)
		+ vnabbreviations[rightTextNodeOffset0->vietnameseAbbreviationIndentifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier)
		+ (vnabbreviations[rightTextNodeOffset1->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[rightTextNodeOffset2->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) > 2)
		+ (vnabbreviations[rightTextNodeOffset3->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier) > 3)
		+ (vnabbreviations[rightTextNodeOffset4->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 4)
		;

	/************************************************************************/
	/* Current node is word                                                 */
	/************************************************************************/


	int currentWordIdentifier = vnsyllables[vietnameseSyllableIdentifier].DetectWord(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier);
	if (vnwords[currentWordIdentifier].significant)
	{
		int currentWordLength = vnwords[currentWordIdentifier].length;
		switch (currentWordLength)
		{
		case 2:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 3:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 4:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset4VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 5:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset4VietnameseSyllableIdentifier, leftTextNodeOffset5VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		}
	}

	/************************************************************************/
	/* rightTextNodeOffset0 node is word                                  */
	/************************************************************************/
	int rightWordOffSet0Indentifier = vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].DetectWord(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet0Indentifier].significant)
	{
		int rightWordOffSet0Length = vnwords[rightWordOffSet0Indentifier].length;
		switch (rightWordOffSet0Length)
		{
		case 2: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 3: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 4: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 5: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset4VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		}
	}


	/************************************************************************/
	/* rightTextNodeOffset1 node is word                                  */
	/************************************************************************/
	int rightWordOffSet1Identifier = vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet1Identifier].significant)
	{
		int rightWordOffSet1Length = vnwords[rightWordOffSet1Identifier].length;
		switch (rightWordOffSet1Length)
		{
		case 2:  currentSyllableSure += vnwords[rightWordOffSet1Identifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);		break;
		case 3:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		case 4:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		case 5:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		}
	}


	/************************************************************************/
	/* rightTextNodeOffset2 node is word                                  */
	/************************************************************************/
	int rightWordOffSet2Identifier = vnsyllables[rightTextNodeOffset2->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet2Identifier].significant)
	{
		int rightWordOffSet2Length = vnwords[rightWordOffSet2Identifier].length;
		switch (rightWordOffSet2Length)
		{
		case 2:  currentSyllableSure += (vnwords[rightWordOffSet2Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1);		break;
		case 3:  currentSyllableSure += vnwords[rightWordOffSet2Identifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);		break;
		case 4:  currentSyllableSure += rightWordOffSet2Length + vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].coefficient; break;
		case 5:  currentSyllableSure += rightWordOffSet2Length + vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].coefficient; break;
		}
	}




	/************************************************************************/
	/* rightTextNodeOffset3 node is word                                  */
	/************************************************************************/
	int rightWordOffSet3Identifier = vnsyllables[rightTextNodeOffset3->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet3Identifier].significant)
	{
		int rightWordOffSet3Length = vnwords[rightWordOffSet3Identifier].length;
		switch (rightWordOffSet3Length)
		{
		case 3: currentSyllableSure += (vnwords[rightWordOffSet3Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1); break;
		case 4: currentSyllableSure += vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier);		break;
		case 5: currentSyllableSure += rightWordOffSet3Length + vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet3Identifier].RightSure(rightTextNodeOffset4->vietnameseSyllableIdentifier, rightTextNodeOffset5->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet3Identifier].coefficient; break;
		}
	}
	return currentSyllableSure;
}
double				VietnameseTextNormalizer::SignificantScoreForMissingEndAndJoinProblem(TEXT_NODE* textNode, qvsylidentifier vietnameseSyllableIdentifier)
{
	TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* leftTextNodeOffset5 = &nullTextNodeForStep2Normalize;
	if (textNode->back)
	{
		leftTextNodeOffset0 = textNode->back;
		if (leftTextNodeOffset0->back)
		{
			leftTextNodeOffset1 = leftTextNodeOffset0->back;
			if (leftTextNodeOffset1->back)
			{
				leftTextNodeOffset2 = leftTextNodeOffset1->back;
				if (leftTextNodeOffset2->back)
				{
					leftTextNodeOffset3 = leftTextNodeOffset2->back;
					if (leftTextNodeOffset3->back)
					{
						leftTextNodeOffset4 = leftTextNodeOffset3->back;
						if (leftTextNodeOffset4->back)
						{
							leftTextNodeOffset5 = leftTextNodeOffset4->back;
						}
					}
				}
			}
		}
	}
	TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
	TEXT_NODE* rightTextNodeOffset5 = &nullTextNodeForStep2Normalize;
	if (textNode->next)
	{
		rightTextNodeOffset0 = textNode->next;
		if (rightTextNodeOffset0->next)
		{
			rightTextNodeOffset0 = textNode->next->next;
		}
		else rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
		if (rightTextNodeOffset0->next)
		{
			rightTextNodeOffset1 = rightTextNodeOffset0->next;
			if (rightTextNodeOffset1->next)
			{
				rightTextNodeOffset2 = rightTextNodeOffset1->next;
				if (rightTextNodeOffset2->next)
				{
					rightTextNodeOffset3 = rightTextNodeOffset2->next;
					if (rightTextNodeOffset3->next)
					{
						rightTextNodeOffset4 = rightTextNodeOffset3->next;
						if (rightTextNodeOffset4->next)
						{
							rightTextNodeOffset5 = rightTextNodeOffset4->next;
						}
					}
				}
			}
		}
	}
	qvsylidentifier leftTextNodeOffset0VietnameseSyllableIdentifier = leftTextNodeOffset0->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset1VietnameseSyllableIdentifier = leftTextNodeOffset1->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset2VietnameseSyllableIdentifier = leftTextNodeOffset2->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset3VietnameseSyllableIdentifier = leftTextNodeOffset3->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset4VietnameseSyllableIdentifier = leftTextNodeOffset4->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset5VietnameseSyllableIdentifier = leftTextNodeOffset5->vietnameseSyllableIdentifier;


	if (textNode->capital == TEXT_NODE_CAPITAL_LOWER)
	{
		if (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset5VietnameseSyllableIdentifier = 0;
	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_UPPER)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset3->textLength == 1)))  leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset4->textLength == 1)))  leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset5->textLength == 1)))  leftTextNodeOffset5VietnameseSyllableIdentifier = 0;

	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_CAPITAL)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset3->textLength == 1)))  leftTextNodeOffset3VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset4->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset4->textLength == 1)))  leftTextNodeOffset4VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset5->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset5->textLength == 1)))  leftTextNodeOffset5VietnameseSyllableIdentifier = 0;
	}



	double currentSyllableSure =
		/************************************************************************/
		/* Right Sure                                                           */
		/************************************************************************/
		vnsyllables[vietnameseSyllableIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier)
		+ (vnsyllables[leftTextNodeOffset0VietnameseSyllableIdentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) > 2)
		+ (vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) > 3)
		+ (vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 4)
		+ (vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 5)

		+ (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].RightSure(leftTextNodeOffset0VietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1)
		+ vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].RightSure(vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier)
		/************************************************************************/
		/* Left Sure                                                            */
		/************************************************************************/
		+vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier)
		+ (vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1)
		+ vnabbreviations[rightTextNodeOffset0->vietnameseAbbreviationIndentifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier)
		+ (vnabbreviations[rightTextNodeOffset1->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) > 1)
		+ (vnabbreviations[rightTextNodeOffset2->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) > 2)
		+ (vnabbreviations[rightTextNodeOffset3->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier) > 3)
		+ (vnabbreviations[rightTextNodeOffset4->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 4)
		;

	/************************************************************************/
	/* Current node is word                                                 */
	/************************************************************************/


	int currentWordIdentifier = vnsyllables[vietnameseSyllableIdentifier].DetectWord(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier);
	if (vnwords[currentWordIdentifier].significant)
	{
		int currentWordLength = vnwords[currentWordIdentifier].length;
		switch (currentWordLength)
		{
		case 2:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 3:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 4:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset4VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		case 5:  currentSyllableSure += currentWordLength + vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset4VietnameseSyllableIdentifier, leftTextNodeOffset5VietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) + vnwords[currentWordIdentifier].coefficient; break;
		}
	}

	/************************************************************************/
	/* rightTextNodeOffset0 node is word                                  */
	/************************************************************************/
	int rightWordOffSet0Indentifier = vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].DetectWord(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet0Indentifier].significant)
	{
		int rightWordOffSet0Length = vnwords[rightWordOffSet0Indentifier].length;
		switch (rightWordOffSet0Length)
		{
		case 2: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 3: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 4: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		case 5: currentSyllableSure += rightWordOffSet0Length + vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset3VietnameseSyllableIdentifier, leftTextNodeOffset4VietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet0Indentifier].coefficient; break;
		}
	}


	/************************************************************************/
	/* rightTextNodeOffset1 node is word                                  */
	/************************************************************************/
	int rightWordOffSet1Identifier = vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet1Identifier].significant)
	{
		int rightWordOffSet1Length = vnwords[rightWordOffSet1Identifier].length;
		switch (rightWordOffSet1Length)
		{
		case 2:  currentSyllableSure += vnwords[rightWordOffSet1Identifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);		break;
		case 3:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		case 4:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		case 5:  currentSyllableSure += rightWordOffSet1Length + vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset2VietnameseSyllableIdentifier, leftTextNodeOffset3VietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet1Identifier].coefficient; break;
		}
	}


	/************************************************************************/
	/* rightTextNodeOffset2 node is word                                  */
	/************************************************************************/
	int rightWordOffSet2Identifier = vnsyllables[rightTextNodeOffset2->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet2Identifier].significant)
	{
		int rightWordOffSet2Length = vnwords[rightWordOffSet2Identifier].length;
		switch (rightWordOffSet2Length)
		{
		case 2:  currentSyllableSure += (vnwords[rightWordOffSet2Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1);		break;
		case 3:  currentSyllableSure += vnwords[rightWordOffSet2Identifier].LeftSure(vietnameseSyllableIdentifier, leftTextNodeOffset0VietnameseSyllableIdentifier);		break;
		case 4:  currentSyllableSure += rightWordOffSet2Length + vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].coefficient; break;
		case 5:  currentSyllableSure += rightWordOffSet2Length + vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet2Identifier].coefficient; break;
		}
	}




	/************************************************************************/
	/* rightTextNodeOffset3 node is word                                  */
	/************************************************************************/
	int rightWordOffSet3Identifier = vnsyllables[rightTextNodeOffset3->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier);
	if (vnwords[rightWordOffSet3Identifier].significant)
	{
		int rightWordOffSet3Length = vnwords[rightWordOffSet3Identifier].length;
		switch (rightWordOffSet3Length)
		{
		case 3: currentSyllableSure += (vnwords[rightWordOffSet3Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier) > 1); break;
		case 4: currentSyllableSure += vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier);		break;
		case 5: currentSyllableSure += rightWordOffSet3Length + vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier) + vnwords[rightWordOffSet3Identifier].RightSure(rightTextNodeOffset4->vietnameseSyllableIdentifier, rightTextNodeOffset5->vietnameseSyllableIdentifier) + vnwords[rightWordOffSet3Identifier].coefficient; break;
		}
	}
	return currentSyllableSure;
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(push, off)
#pragma optimize("", off)
#pragma warning(push)
#pragma warning(disable : 4100)
#else
#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

/************************************************************************/
/* On going                                                             */
/************************************************************************/
double				VietnameseTextNormalizer::PerplexityScore(TEXT_NODE* textNode, qvsylidentifier vietnameseSyllableIdentifier)
{
#ifdef FLAG_HAVE_NGRAM_ENGINE
	int sentenceLength = 0;
	std::list< globalidentifier> sens;
	sens.insert(sens.begin(), vnsyllables[vietnameseSyllableIdentifier].identifier);
	for (TEXT_NODE* inode = textNode->back; inode != NULL && inode->silenceTimeInSecond == 0.0 && inode->textNodeType != TEXT_NODE_TYPE_SILENCE; inode = inode->back)
	{
		sens.insert(sens.begin(), inode->identifier);
	}
	for (TEXT_NODE* inode = textNode->next; inode != NULL && inode->silenceTimeInSecond == 0.0 && inode->textNodeType != TEXT_NODE_TYPE_SILENCE; inode = inode->next)
	{
		sens.insert(sens.end(), inode->identifier);
	}
	double GetPerplexity(const std::list< globalidentifier> &sens);
	double mPerplexity = 200/*kỳ vọng tối đa*/ - GetPerplexity(sens);
	if (mPerplexity < 0.0) mPerplexity = 0;
	return mPerplexity / 20.0 /*To get score 0.0 - 10.0*/;
#else
	return 0.0;
#endif
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma optimize("", on)
#pragma warning(pop)
#pragma managed(pop)
#else
#pragma GCC diagnostic pop
#pragma GCC pop_options
#endif








/************************************************************************/
/* Update                                                               */
/************************************************************************/
void				VietnameseTextNormalizer::UpdateVietnameseTextNodeContext(TEXT_NODE* textNode, TEXT_NODE* leftTextNodeOffset0, TEXT_NODE* leftTextNodeOffset1, TEXT_NODE* leftTextNodeOffset2, TEXT_NODE* leftTextNodeOffset3, TEXT_NODE* leftTextNodeOffset4)
{
	qvsylidentifier					vietnameseSyllableIdentifier = textNode->vietnameseSyllableIdentifier;
	/************************************************************************/
	/* Capital                                                              */
	/************************************************************************/
	if ((textNode->capital == TEXT_NODE_CAPITAL_UPPER || textNode->capital == TEXT_NODE_CAPITAL_CAPITAL) && (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER || leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL)) leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	if (vnsyllables[vietnameseSyllableIdentifier].leftJoin) leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	if (vnsyllables[vietnameseSyllableIdentifier].rightJoin) textNode->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	/************************************************************************/
	/* Left Sure                                                            */
	/************************************************************************/
	textNode->leftVietnameseSyllableSure = vnsyllables[vietnameseSyllableIdentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
	switch (textNode->leftVietnameseSyllableSure)
	{
	case 2:
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		leftTextNodeOffset1->englishWordIdentifier = 0;
		/*case through here*/
	case 1:
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		leftTextNodeOffset0->englishWordIdentifier = 0;
		break;
	}
	/************************************************************************/
	/* Right Sure                                                           */
	/************************************************************************/
	leftTextNodeOffset0->rightVietnameseWordSure = vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].RightSure(vietnameseSyllableIdentifier, 0);
	leftTextNodeOffset1->rightVietnameseWordSure = vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier);
	leftTextNodeOffset0->rightVietnameseSyllableSure = vnsyllables[leftTextNodeOffset0->vietnameseSyllableIdentifier].RightSure(vietnameseSyllableIdentifier, 0);
	leftTextNodeOffset1->rightVietnameseSyllableSure = vnsyllables[leftTextNodeOffset1->vietnameseSyllableIdentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier);
	leftTextNodeOffset0->rightVietnameseAbbreviationSure = vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightSure(vietnameseSyllableIdentifier, 0, 0, 0, 0)
		+ vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightAbbreviationSure(textNode->vietnameseAbbreviationIndentifier, 0, 0, 0, 0);
	leftTextNodeOffset1->rightVietnameseAbbreviationSure = vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, 0, 0, 0)
		+ vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightAbbreviationSure(leftTextNodeOffset0->vietnameseAbbreviationIndentifier, textNode->vietnameseAbbreviationIndentifier, 0, 0, 0);
	leftTextNodeOffset2->rightVietnameseAbbreviationSure = vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, 0, 0)
		+ vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightAbbreviationSure(leftTextNodeOffset1->vietnameseAbbreviationIndentifier, leftTextNodeOffset0->vietnameseAbbreviationIndentifier, textNode->vietnameseAbbreviationIndentifier, 0, 0);
	leftTextNodeOffset3->rightVietnameseAbbreviationSure = vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier, 0)
		+ vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightAbbreviationSure(leftTextNodeOffset2->vietnameseAbbreviationIndentifier, leftTextNodeOffset1->vietnameseAbbreviationIndentifier, leftTextNodeOffset0->vietnameseAbbreviationIndentifier, textNode->vietnameseAbbreviationIndentifier, 0);
	leftTextNodeOffset4->rightVietnameseAbbreviationSure = vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, vietnameseSyllableIdentifier)
		+ vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightAbbreviationSure(leftTextNodeOffset3->vietnameseAbbreviationIndentifier, leftTextNodeOffset2->vietnameseAbbreviationIndentifier, leftTextNodeOffset1->vietnameseAbbreviationIndentifier, leftTextNodeOffset0->vietnameseAbbreviationIndentifier, textNode->vietnameseAbbreviationIndentifier);
	if (leftTextNodeOffset0->rightVietnameseWordSure == 1 || leftTextNodeOffset0->rightVietnameseSyllableSure == 1 || leftTextNodeOffset0->rightVietnameseAbbreviationSure == 1)
	{
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset0->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	}
	if (leftTextNodeOffset1->rightVietnameseWordSure == 2 || leftTextNodeOffset1->rightVietnameseSyllableSure == 2 || leftTextNodeOffset1->rightVietnameseAbbreviationSure == 2)
	{
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset0->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset1->vietnameseSyllableIdentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset1->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset1->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	}
	if (leftTextNodeOffset2->rightVietnameseAbbreviationSure == 3)
	{
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset0->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset1->vietnameseSyllableIdentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset1->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset1->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset2->vietnameseSyllableIdentifier) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset2->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset2->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	}
	if (leftTextNodeOffset3->rightVietnameseAbbreviationSure == 4)
	{
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset0->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset1->vietnameseSyllableIdentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset1->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset1->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset2->vietnameseSyllableIdentifier) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset2->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset2->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset3->vietnameseSyllableIdentifier) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset3->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset3->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	}
	if (leftTextNodeOffset4->rightVietnameseAbbreviationSure == 5)
	{
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset0->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset1->vietnameseSyllableIdentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset1->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset1->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset2->vietnameseSyllableIdentifier) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset2->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset2->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset3->vietnameseSyllableIdentifier) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset3->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset3->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		if (leftTextNodeOffset4->vietnameseSyllableIdentifier) leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset4->vietnameseAbbreviationIndentifier > 0 && leftTextNodeOffset4->rightVietnameseAbbreviationSure > 0) leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		leftTextNodeOffset4->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
	}
	/************************************************************************/
	/* Word detection                                                       */
	/************************************************************************/
	qvsylidentifier leftTextNodeOffset0VietnameseSyllableIdentifier = leftTextNodeOffset0->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset1VietnameseSyllableIdentifier = leftTextNodeOffset1->vietnameseSyllableIdentifier;
	qvsylidentifier leftTextNodeOffset2VietnameseSyllableIdentifier = leftTextNodeOffset2->vietnameseSyllableIdentifier;
	if (textNode->capital == TEXT_NODE_CAPITAL_LOWER)
	{
		if (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL) leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_UPPER)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
	}
	else if (textNode->capital == TEXT_NODE_CAPITAL_CAPITAL)
	{
		if (!(leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset0->textLength == 1)))  leftTextNodeOffset0VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset1->textLength == 1)))  leftTextNodeOffset1VietnameseSyllableIdentifier = 0;
		if (!(leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_CAPITAL || (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_UPPER && leftTextNodeOffset2->textLength == 1)))  leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
	}
	for (int iwordCheckTurn = 0; iwordCheckTurn < 3; iwordCheckTurn++)
	{
		if (iwordCheckTurn == 1) leftTextNodeOffset2VietnameseSyllableIdentifier = 0;
		else if (iwordCheckTurn == 2) leftTextNodeOffset1VietnameseSyllableIdentifier = 0;





		qvwrdidentifier vietnameseWordIdentifier = vnsyllables[vietnameseSyllableIdentifier].DetectWord(leftTextNodeOffset0VietnameseSyllableIdentifier, leftTextNodeOffset1VietnameseSyllableIdentifier, leftTextNodeOffset2VietnameseSyllableIdentifier);
#ifndef ON_SCAN_WORD_FREQUENCY
		if (vnwords[vietnameseWordIdentifier].significant) /*safe break*/iwordCheckTurn = 99;
#endif


		//self - validate
		switch (vnwords[vietnameseWordIdentifier].length)
		{
		case 4:
			if (leftTextNodeOffset2VietnameseSyllableIdentifier == 0)
			{
				printf("?");
				vietnameseWordIdentifier = 0;
			}
			break;
		case 3:
			if (leftTextNodeOffset1VietnameseSyllableIdentifier == 0)
			{
				printf("?");
				vietnameseWordIdentifier = 0;
			}
			break;
		case 2:
			if (leftTextNodeOffset0VietnameseSyllableIdentifier == 0)
			{
				printf("?");
				vietnameseWordIdentifier = 0;
			}
			break;
		}




		if (vietnameseWordIdentifier)
		{
			switch (vnwords[vietnameseWordIdentifier].length)
			{
			case 4:
				if (leftTextNodeOffset2->vietnameseWordIdentifier != 0 || leftTextNodeOffset2->changeable == TEXT_NODE_CAN_NOT_CHANGE)
				{
					textNode->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				}
				leftTextNodeOffset2->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset2->englishWordIdentifier = 0;
				leftTextNodeOffset1->englishWordIdentifier = 0;
				leftTextNodeOffset0->englishWordIdentifier = 0;
#ifndef ON_SCAN_WORD_FREQUENCY
				if (vnwords[vietnameseWordIdentifier].significant)
				{
					if (vnwords[leftTextNodeOffset2->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset2->vietnameseWordIdentifier = 0;
					if (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset1->vietnameseWordIdentifier = 0;
					if (vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset0->vietnameseWordIdentifier = 0;
				}
#endif
				textNode->leftVietnameseWordSure = vnwords[vietnameseWordIdentifier].LeftSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier);
				switch (textNode->leftVietnameseWordSure)
				{
				case 2:
					leftTextNodeOffset4->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset4->englishWordIdentifier = 0;
					/*case through here*/
				case 1:
					leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset3->englishWordIdentifier = 0;
					break;
				}
				break;
			case 3:
				if (leftTextNodeOffset1->vietnameseWordIdentifier != 0 || leftTextNodeOffset1->changeable == TEXT_NODE_CAN_NOT_CHANGE)
				{
					textNode->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				}
				leftTextNodeOffset1->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset1->englishWordIdentifier = 0;
				leftTextNodeOffset0->englishWordIdentifier = 0;
#ifndef ON_SCAN_WORD_FREQUENCY
				if (vnwords[vietnameseWordIdentifier].significant)
				{
					if (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset1->vietnameseWordIdentifier = 0;
					if (vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset0->vietnameseWordIdentifier = 0;
					if (vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].length == 2) leftTextNodeOffset0->vietnameseWordIdentifier = 0;
				}
#endif
				textNode->leftVietnameseWordSure = vnwords[vietnameseWordIdentifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier);
				switch (textNode->leftVietnameseWordSure)
				{
				case 2:
					leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset3->englishWordIdentifier = 0;
					/*case through here*/
				case 1:
					leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset2->englishWordIdentifier = 0;
					break;
				}
				break;
			case 2:
				if (leftTextNodeOffset0->vietnameseWordIdentifier != 0 || leftTextNodeOffset0->changeable == TEXT_NODE_CAN_NOT_CHANGE)
				{
					textNode->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				}
				leftTextNodeOffset0->changeable = TEXT_NODE_CAN_NOT_CHANGE;
				leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
				leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				leftTextNodeOffset0->englishWordIdentifier = 0;
#ifndef ON_SCAN_WORD_FREQUENCY
				if (vnwords[vietnameseWordIdentifier].significant)
				{
					if (vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].significant == 0) leftTextNodeOffset0->vietnameseWordIdentifier = 0;
				}
#endif
				textNode->leftVietnameseWordSure = vnwords[vietnameseWordIdentifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier);
				switch (textNode->leftVietnameseWordSure)
				{
				case 2:
					leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset2->englishWordIdentifier = 0;
					/*case through here*/
				case 1:
					leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					leftTextNodeOffset1->englishWordIdentifier = 0;
					break;
				}
				break;
			}

#ifndef ON_SCAN_WORD_FREQUENCY
			if (vnwords[vietnameseWordIdentifier].significant)
#endif
			{
				textNode->vietnameseWordIdentifier = vietnameseWordIdentifier;
			}
			textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		}
	}





	/************************************************************************/
	/* Đơn vị hành chính các cấp                                            */
	/************************************************************************/
	if (textNode->vietnameseSyllableIdentifier > 0
		&& textNode->vietnameseWordIdentifier > 0
		&& vnwords[textNode->vietnameseWordIdentifier].upper > 0
		&& textNode->changeable != TEXT_NODE_CAN_NOT_CHANGE
		&& (textNode->leftVietnameseWordSure > 0
			|| ((leftTextNodeOffset0->vietnameseAbbreviationIndentifier == VIETNAMESE_ABBREVIATION_THAFNH_PHOOS || leftTextNodeOffset0->vietnameseAbbreviationIndentifier == VIETNAMESE_ABBREVIATION_QUAAJN)
				&& leftTextNodeOffset0->vietnameseAbbreviationSure > 0
				))
		)
	{
		bool flagValidPlaceName = false;
		switch (vnwords[textNode->vietnameseWordIdentifier].length)
		{
		case 5:
			if (leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX
				|| leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_UW_OWF_N_G
				|| leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_Q_U_AAJ_N
				|| leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_U_Y_EEJ_N
				|| leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_IR_N_H
				//	|| (leftTextNodeOffset5->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX)
				//	|| (leftTextNodeOffset5->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_R_AAS_N)
				//	|| (leftTextNodeOffset5->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_AF_N_H && leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_OOS)
				)
			{
				flagValidPlaceName = true;
			}
			break;
		case 4:
			if (leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX
				|| leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_UW_OWF_N_G
				|| leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_Q_U_AAJ_N
				|| leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_U_Y_EEJ_N
				|| leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_IR_N_H
				|| (leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX)
				|| (leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_R_AAS_N)
				|| (leftTextNodeOffset4->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_AF_N_H && leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_OOS)
				)
			{
				flagValidPlaceName = true;
			}
			break;
		case 3:
			if (leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX
				|| leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_UW_OWF_N_G
				|| leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_Q_U_AAJ_N
				|| leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_U_Y_EEJ_N
				|| leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_IR_N_H
				|| (leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX)
				|| (leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_R_AAS_N)
				|| (leftTextNodeOffset3->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_AF_N_H && leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_OOS)
				)
			{
				flagValidPlaceName = true;
			}
			break;
		case 2:
			if (leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX
				|| leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_UW_OWF_N_G
				|| leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_Q_U_AAJ_N
				|| leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_U_Y_EEJ_N
				|| leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_IR_N_H
				|| (leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX)
				|| (leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_R_AAS_N)
				|| (leftTextNodeOffset2->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_AF_N_H && leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_OOS)
				)
			{
				flagValidPlaceName = true;
			}
			break;
		case 1:
			if (leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX
				|| leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_UW_OWF_N_G
				|| leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_Q_U_AAJ_N
				|| leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_U_Y_EEJ_N
				|| leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_IR_N_H
				|| (leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_X_AX)
				|| (leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_IJ && leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_R_AAS_N)
				|| (leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_AF_N_H && leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_P_H_OOS)
				)
			{
				flagValidPlaceName = true;
			}
			break;
		}



		if (flagValidPlaceName)
		{
			switch (vnwords[textNode->vietnameseWordIdentifier].length)
			{
			case 5:
				if (leftTextNodeOffset3->capital == TEXT_NODE_CAPITAL_LOWER)
				{
					leftTextNodeOffset3->text = vnsyllables[leftTextNodeOffset3->vietnameseSyllableIdentifier].upper;
					leftTextNodeOffset3->textLength = vnsyllables[leftTextNodeOffset3->vietnameseSyllableIdentifier].length;
					leftTextNodeOffset3->capital = TEXT_NODE_CAPITAL_UPPER;
				}
			case 4:
				if (leftTextNodeOffset2->capital == TEXT_NODE_CAPITAL_LOWER)
				{
					leftTextNodeOffset2->text = vnsyllables[leftTextNodeOffset2->vietnameseSyllableIdentifier].upper;
					leftTextNodeOffset2->textLength = vnsyllables[leftTextNodeOffset2->vietnameseSyllableIdentifier].length;
					leftTextNodeOffset2->capital = TEXT_NODE_CAPITAL_UPPER;
				}
			case 3:
				if (leftTextNodeOffset1->capital == TEXT_NODE_CAPITAL_LOWER)
				{
					leftTextNodeOffset1->text = vnsyllables[leftTextNodeOffset1->vietnameseSyllableIdentifier].upper;
					leftTextNodeOffset1->textLength = vnsyllables[leftTextNodeOffset1->vietnameseSyllableIdentifier].length;
					leftTextNodeOffset1->capital = TEXT_NODE_CAPITAL_UPPER;
				}
			case 2:
				if (leftTextNodeOffset0->capital == TEXT_NODE_CAPITAL_LOWER)
				{
					leftTextNodeOffset0->text = vnsyllables[leftTextNodeOffset0->vietnameseSyllableIdentifier].upper;
					leftTextNodeOffset0->textLength = vnsyllables[leftTextNodeOffset0->vietnameseSyllableIdentifier].length;
					leftTextNodeOffset0->capital = TEXT_NODE_CAPITAL_UPPER;
				}
			case 1:
				if (textNode->capital == TEXT_NODE_CAPITAL_LOWER)
				{
					textNode->text = vnsyllables[textNode->vietnameseSyllableIdentifier].upper;
					textNode->textLength = vnsyllables[textNode->vietnameseSyllableIdentifier].length;
					textNode->capital = TEXT_NODE_CAPITAL_UPPER;
				}
				break;
			}
		}
	}

#ifndef ON_SCAN_WORD_FREQUENCY
	if (textNode->vietnameseWordIdentifier > 0 && vnwords[textNode->vietnameseWordIdentifier].significant == 0) textNode->vietnameseWordIdentifier = 0;
#endif

	/************************************************************************/
	/* Abbreviation                                                         */
	/************************************************************************/
	if (textNode->vietnameseAbbreviationIndentifier == 0) textNode->vietnameseAbbreviationIndentifier = vnsyllables[vietnameseSyllableIdentifier].abbreviation;
	textNode->leftVietnameseAbbreviationSure = vnabbreviations[textNode->vietnameseAbbreviationIndentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier)
		+ vnabbreviations[textNode->vietnameseAbbreviationIndentifier].LeftAbbreviationSure(leftTextNodeOffset0->vietnameseAbbreviationIndentifier, leftTextNodeOffset1->vietnameseAbbreviationIndentifier, leftTextNodeOffset2->vietnameseAbbreviationIndentifier, leftTextNodeOffset3->vietnameseAbbreviationIndentifier, leftTextNodeOffset4->vietnameseAbbreviationIndentifier);
	if (vnabbreviations[textNode->vietnameseAbbreviationIndentifier].contextRequire == 0 && textNode->capital == TEXT_NODE_CAPITAL_CAPITAL && vnabbreviations[textNode->vietnameseAbbreviationIndentifier].pronoucing == 0)
	{
		if (leftTextNodeOffset0->capital != TEXT_NODE_CAPITAL_LOWER) textNode->vietnameseAbbreviationSure = 1;
		else if (textNode->next && textNode->next->capital != TEXT_NODE_CAPITAL_LOWER) textNode->vietnameseAbbreviationSure = 1;
	}
	switch (textNode->leftVietnameseAbbreviationSure)
	{
	case 5:
		leftTextNodeOffset4->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		if (leftTextNodeOffset4->vietnameseSyllableIdentifier) leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset4->vietnameseAbbreviationIndentifier) leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		/*case through here*/
	case 4:
		leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		if (leftTextNodeOffset3->vietnameseSyllableIdentifier) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset3->vietnameseAbbreviationIndentifier) leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		/*case through here*/
	case 3:
		leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		if (leftTextNodeOffset2->vietnameseSyllableIdentifier) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset2->vietnameseAbbreviationIndentifier) leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		/*case through here*/
	case 2:
		leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		if (leftTextNodeOffset1->vietnameseSyllableIdentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset1->vietnameseAbbreviationIndentifier) leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		/*case through here*/
	case 1:
		leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
		if (leftTextNodeOffset0->vietnameseSyllableIdentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (leftTextNodeOffset0->vietnameseAbbreviationIndentifier) leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
		/*case through here*/
	}
	if (textNode->leftVietnameseAbbreviationSure == 0 && textNode->rightVietnameseAbbreviationSure == 0 && vnabbreviations[textNode->vietnameseAbbreviationIndentifier].pronoucing)
	{
		for (qvsylidentifier nextAbbreviationIndentifier = vnabbreviations[textNode->vietnameseAbbreviationIndentifier].pronoucing; nextAbbreviationIndentifier != 0 && textNode->leftVietnameseAbbreviationSure == 0 && textNode->rightVietnameseAbbreviationSure == 0; nextAbbreviationIndentifier = vnabbreviations[nextAbbreviationIndentifier].pronoucing)
		{
			textNode->leftVietnameseAbbreviationSure = vnabbreviations[nextAbbreviationIndentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier);

			TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep1Input;
			TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep1Input;
			TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep1Input;
			TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep1Input;
			TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep1Input;
			if (textNode->next)
			{
				rightTextNodeOffset0 = textNode->next;
				if (rightTextNodeOffset0->next)
				{
					rightTextNodeOffset1 = rightTextNodeOffset0->next;
					if (rightTextNodeOffset1->next)
					{
						rightTextNodeOffset2 = rightTextNodeOffset1->next;
						if (rightTextNodeOffset2->next)
						{
							rightTextNodeOffset3 = rightTextNodeOffset2->next;
							if (rightTextNodeOffset3->next)
							{
								rightTextNodeOffset4 = rightTextNodeOffset3->next;
							}
						}
					}
				}
			}
			textNode->rightVietnameseAbbreviationSure = vnabbreviations[nextAbbreviationIndentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier);
			if (textNode->leftVietnameseAbbreviationSure || textNode->rightVietnameseAbbreviationSure)
			{
				textNode->vietnameseAbbreviationIndentifier = nextAbbreviationIndentifier;
			}
		}
	}
	/************************************************************************/
	/* Loan word                                                            */
	/************************************************************************/
	if (textNode->vietnameseLoanWordIndentifier == 0 && textNode->retroflex == false) textNode->vietnameseLoanWordIndentifier = vnsyllables[vietnameseSyllableIdentifier].loan;
	if ((leftTextNodeOffset0->textNodeType == TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE || leftTextNodeOffset0->silenceTimeInSecond > 0.0) && textNode->next && (textNode->next->textNodeType == TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE || textNode->next->silenceTimeInSecond > 0.0)) textNode->vietnameseLoanSure = 1;
	/************************************************************************/
	/* English                                                              */
	/************************************************************************/
	if (textNode->englishWordIdentifier)
	{
		if (textNode->vietnameseSyllableIdentifier == 0 && textNode->vietnameseLoanWordIndentifier == 0 && textNode->vietnameseAbbreviationIndentifier == 0) textNode->englishWordSure = 1;
		if (textNode->englishWordSure != 0) textNode->leftEnglishWordSure = 1;
		if (textNode->next && textNode->next->englishWordSure != 0) textNode->rightEnglishWordSure = 1;
	}
	/************************************************************************/
	/* Missing End                                                          */
	/************************************************************************/
	if (textNode->vietnameseMissingIndentifiler != 0)
	{
		qvsylidentifier joinWithNextIndentifiler = ((textNode->next /*!= NULL*/ && textNode->next->text /*!= NULL*/) ? vnmissingends[textNode->vietnameseMissingIndentifiler].JoinWithToken(textNode->next->text) : 0);
		if (joinWithNextIndentifiler != 0)
		{
			if (SignificantScoreForMissingEndAndJoinProblem(textNode, joinWithNextIndentifiler) > 0.0 && (textNode->capital == TEXT_NODE_CAPITAL_LOWER || textNode->capital == TEXT_NODE_CAPITAL_UPPER || textNode->capital == TEXT_NODE_CAPITAL_CAPITAL))
			{
				textNode->vietnameseMissingIndentifiler = joinWithNextIndentifiler;
				textNode->originalTextLength = int((textNode->next->originalText - textNode->originalText) + textNode->next->originalTextLength);
				switch (textNode->capital)
				{
				case TEXT_NODE_CAPITAL_LOWER:
					textNode->text = vnsyllables[joinWithNextIndentifiler].lower;
					textNode->textLength = vnsyllables[joinWithNextIndentifiler].length;
					break;
				case TEXT_NODE_CAPITAL_UPPER:
					textNode->text = vnsyllables[joinWithNextIndentifiler].upper;
					textNode->textLength = vnsyllables[joinWithNextIndentifiler].length;
					break;
				case TEXT_NODE_CAPITAL_CAPITAL:
					textNode->text = vnsyllables[joinWithNextIndentifiler].capital;
					textNode->textLength = vnsyllables[joinWithNextIndentifiler].length;
					break;
				default:
					/*do not change any-thing*/
					break;
				}
				textNode->next->originalText = NULL;
				textNode->next->originalTextLength = 0;
				textNode->next->text = NULL;
				textNode->next->textLength = 0;
			}
			else
			{
				textNode->vietnameseMissingIndentifiler = joinWithNextIndentifiler;
			}
			if (textNode->next->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
			{
				if (countTotalUnknownNode > 0)  countTotalUnknownNode--;
			}
			countTotalUnknownNode--;
			textNode->next->textNodeType = TEXT_NODE_TYPE_IGNORE_NODE;
		}
		else
		{
			int	flagAlreadySureRight = 0;
			if (textNode->next == NULL
				|| textNode->silenceTimeInSecond > 0.0
				|| (textNode->vietnameseSyllableIdentifier && textNode->rightVietnameseSyllableSure)
				|| (textNode->vietnameseAbbreviationIndentifier && textNode->rightVietnameseAbbreviationSure)
				) flagAlreadySureRight = -1;

			qvsylidentifier		vietnameseMissingIndentifiler = textNode->vietnameseMissingIndentifiler;
			double				maxCoefficient = 0.0;
			qvsylidentifier		maxCoefficientIdentifier = 0;
			int					maxIsJoinWithNextNode = 0;
			int					countTotalJoinWithNextNode = 0;
			int					countSureWay = 0;
			for (int iway = 0; iway < vnmissingends[vietnameseMissingIndentifiler].length; iway++)
			{
				//		TEXT_NODE *				leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
				//		TEXT_NODE *				leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
				//		TEXT_NODE *				leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
				//		TEXT_NODE *				leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
				//		TEXT_NODE *				leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
				//		TEXT_NODE *				leftTextNodeOffset5 = &nullTextNodeForStep2Normalize;
				//		if (textNode->back)
				//		{
				//			leftTextNodeOffset0 = textNode->back;
				//			if (leftTextNodeOffset0->back)
				//			{
				//				leftTextNodeOffset1 = leftTextNodeOffset0->back;
				//				if (leftTextNodeOffset1->back)
				//				{
				//					leftTextNodeOffset2 = leftTextNodeOffset1->back;
				//					if (leftTextNodeOffset2->back)
				//					{
				//						leftTextNodeOffset3 = leftTextNodeOffset2->back;
				//						if (leftTextNodeOffset3->back)
				//						{
				//							leftTextNodeOffset4 = leftTextNodeOffset3->back;
				//							if (leftTextNodeOffset4->back)
				//							{
				//								leftTextNodeOffset5 = leftTextNodeOffset4->back;
				//							}
				//						}
				//					}
				//				}
				//			}
				//		}

				TEXT_NODE* leftTextNodeOffset5 = &nullTextNodeForStep2Normalize;
				if (leftTextNodeOffset4->back)
				{
					leftTextNodeOffset5 = leftTextNodeOffset4->back;
				}


				TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* rightTextNodeOffset5 = &nullTextNodeForStep2Normalize;
				if (textNode->next)
				{
					rightTextNodeOffset0 = textNode->next;
					if (rightTextNodeOffset0->next)
					{
						rightTextNodeOffset1 = rightTextNodeOffset0->next;
						if (rightTextNodeOffset1->next)
						{
							rightTextNodeOffset2 = rightTextNodeOffset1->next;
							if (rightTextNodeOffset2->next)
							{
								rightTextNodeOffset3 = rightTextNodeOffset2->next;
								if (rightTextNodeOffset3->next)
								{
									rightTextNodeOffset4 = rightTextNodeOffset3->next;
									if (rightTextNodeOffset4->next)
									{
										rightTextNodeOffset5 = rightTextNodeOffset4->next;
									}
								}
							}
						}
					}
				}
				qvsylidentifier		currentVnSyllableIdentifier = vnmissingends[vietnameseMissingIndentifiler].destinations[iway];
				bool				flagCurrentNodeJoinWithNextNode = false;


				int currentSyllableSure =
					/************************************************************************/
					/* Right Sure                                                           */
					/************************************************************************/
					vnsyllables[currentVnSyllableIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier)
					+ (vnsyllables[leftTextNodeOffset0->vietnameseSyllableIdentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 1)
					+ (vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) > 1 + flagAlreadySureRight)
					+ (vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) > 2 + flagAlreadySureRight)
					+ (vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) > 3 + flagAlreadySureRight)
					+ (vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 4 + flagAlreadySureRight)
					+ (vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 5 + flagAlreadySureRight)
					+ (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1)
					+ vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier)
					/************************************************************************/
					/* Left Sure                                                            */
					/************************************************************************/
					+vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier)
					+ (vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1)
					+ vnabbreviations[rightTextNodeOffset0->vietnameseAbbreviationIndentifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier)
					+ (vnabbreviations[rightTextNodeOffset1->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier) > 1 + flagAlreadySureRight)
					+ (vnabbreviations[rightTextNodeOffset2->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier) > 2 + flagAlreadySureRight)
					+ (vnabbreviations[rightTextNodeOffset3->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier) > 3 + flagAlreadySureRight)
					+ (vnabbreviations[rightTextNodeOffset4->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 4 + flagAlreadySureRight)
					;

				//------current node is word
				int currentWordIdentifier = vnsyllables[currentVnSyllableIdentifier].DetectWord(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier);
				int currentWordLength = vnwords[currentWordIdentifier].length;
				int currentWordRightSure = vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier);
				int currentWordLeftSure = 0;
				switch (currentWordLength)
				{
				case 2: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
				case 3: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
				case 4: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier); break;
				case 5: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset4->vietnameseSyllableIdentifier, leftTextNodeOffset5->vietnameseSyllableIdentifier); break;
				}
				if (currentWordRightSure) flagCurrentNodeJoinWithNextNode = true;


				//------rightTextNodeOffset0 is word
				int rightWordOffSet0Indentifier = vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].DetectWord(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
				int rightWordOffSet0Length = vnwords[rightWordOffSet0Indentifier].length;
				int rightWordOffSet0RightSure = vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier);
				int rightWordOffSet0LeftSure = 0;
				switch (rightWordOffSet0Length)
				{
				case 2: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
				case 3: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
				case 4: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
				case 5: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier); break;
				}
				if (rightWordOffSet0Length > 1 + flagAlreadySureRight) flagCurrentNodeJoinWithNextNode = true;


				//------rightTextNodeOffset1 is word
				int rightWordOffSet1Identifier = vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
				int rightWordOffSet1Length = vnwords[rightWordOffSet1Identifier].length;
				int rightWordOffSet1RightSure = vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier);
				int rightWordOffSet1LeftSure = 0;
				switch (rightWordOffSet1Length)
				{
				case 2:
					rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
					rightWordOffSet1Identifier = 0;
					rightWordOffSet1Length = 0;
					rightWordOffSet1RightSure = 0;
					break;
				case 3: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
				case 4: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
				case 5: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
				}
				if (rightWordOffSet1Length > 2 + flagAlreadySureRight) flagCurrentNodeJoinWithNextNode = true;


				//------rightTextNodeOffset2 is word
				int rightWordOffSet2Identifier = vnsyllables[rightTextNodeOffset2->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier);
				int rightWordOffSet2Length = vnwords[rightWordOffSet2Identifier].length;
				int rightWordOffSet2RightSure = vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier);
				int rightWordOffSet2LeftSure = 0;
				switch (rightWordOffSet2Length)
				{
				case 2:
					rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1;
					rightWordOffSet2Identifier = 0;
					rightWordOffSet2Length = 0;
					rightWordOffSet2RightSure = 0;
					break;
				case 3:
					rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
					rightWordOffSet2Identifier = 0;
					rightWordOffSet2Length = 0;
					rightWordOffSet2RightSure = 0;
					break;
				case 4: rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
				case 5: rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
				}
				if (rightWordOffSet2Length > 3 + flagAlreadySureRight) flagCurrentNodeJoinWithNextNode = true;


				//------rightTextNodeOffset3 is word
				int rightWordOffSet3Identifier = vnsyllables[rightTextNodeOffset3->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier);
				int rightWordOffSet3Length = vnwords[rightWordOffSet3Identifier].length;
				int rightWordOffSet3RightSure = vnwords[rightWordOffSet3Identifier].RightSure(rightTextNodeOffset4->vietnameseSyllableIdentifier, rightTextNodeOffset5->vietnameseSyllableIdentifier);
				int rightWordOffSet3LeftSure = 0;
				switch (rightWordOffSet3Length)
				{
				case 2:
					rightWordOffSet3LeftSure = 0;
					rightWordOffSet3Identifier = 0;
					rightWordOffSet3Length = 0;
					rightWordOffSet3RightSure = 0;
					break;
				case 3:
					rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1;
					rightWordOffSet3Identifier = 0;
					rightWordOffSet3Length = 0;
					rightWordOffSet3RightSure = 0;
					break;
				case 4:
					rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
					rightWordOffSet3Identifier = 0;
					rightWordOffSet3Length = 0;
					rightWordOffSet3RightSure = 0;
					break;
				case 5: rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
				}
				if (rightWordOffSet3Length > 4 + flagAlreadySureRight) flagCurrentNodeJoinWithNextNode = true;


				if (currentSyllableSure) flagCurrentNodeJoinWithNextNode = true;
				double currentCoefficient = vnsyllables[currentVnSyllableIdentifier].coefficient
					+ currentSyllableSure
					+ vnwords[currentWordIdentifier].coefficient
					+ currentWordLeftSure
					+ currentWordRightSure
					+ vnwords[rightWordOffSet0Indentifier].coefficient
					+ rightWordOffSet0LeftSure
					+ rightWordOffSet0RightSure
					+ vnwords[rightWordOffSet1Identifier].coefficient
					+ rightWordOffSet1LeftSure
					+ rightWordOffSet1RightSure
					+ vnwords[rightWordOffSet2Identifier].coefficient
					+ rightWordOffSet2LeftSure
					+ rightWordOffSet2RightSure
					+ vnwords[rightWordOffSet3Identifier].coefficient
					+ rightWordOffSet3LeftSure
					+ rightWordOffSet3RightSure;

				if (currentCoefficient > maxCoefficient)
				{
					maxCoefficient = currentCoefficient;
					maxCoefficientIdentifier = currentVnSyllableIdentifier;
					maxIsJoinWithNextNode = flagCurrentNodeJoinWithNextNode;
				}
				if (flagCurrentNodeJoinWithNextNode) countTotalJoinWithNextNode++;

				if (currentSyllableSure
					|| currentWordIdentifier || currentWordLeftSure || currentWordRightSure
					|| rightWordOffSet0Indentifier || rightWordOffSet0LeftSure || rightWordOffSet0RightSure
					|| rightWordOffSet1Identifier || rightWordOffSet1LeftSure || rightWordOffSet1RightSure
					|| rightWordOffSet2Identifier || rightWordOffSet2LeftSure || rightWordOffSet2RightSure
					)
				{
					countSureWay++;
				}
			}
			if (maxCoefficientIdentifier && (maxIsJoinWithNextNode || textNode->next == NULL))
			{
				textNode->vietnameseSyllableIdentifier = maxCoefficientIdentifier;
				textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
				if (countSureWay == 1 && countTotalJoinWithNextNode == 1)
				{
					textNode->vietnameseMissingIndentifiler = 0;
					switch (textNode->capital)
					{
					case TEXT_NODE_CAPITAL_LOWER:
						textNode->text = vnsyllables[maxCoefficientIdentifier].lower;
						textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_UPPER:
						textNode->text = vnsyllables[maxCoefficientIdentifier].upper;
						textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_CAPITAL:
						textNode->text = vnsyllables[maxCoefficientIdentifier].capital;
						textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
						break;
					default:
						/*do not change any-thing*/
						break;
					}
				}
				countTotalUnknownNode--;
			}
			else textNode->vietnameseMissingIndentifiler = vietnameseMissingIndentifiler;
		}
	}
	/************************************************************************/
	/* Final                                                                */
	/************************************************************************/
	if (textNode->vietnameseAbbreviationIndentifier || textNode->vietnameseLoanWordIndentifier)
	{
		if (textNode->text)
		{
			std::wstring currentTextNodeText;
			currentTextNodeText.reserve(textNode->textLength + 1);
			for (int iChar = 0, nChar = textNode->textLength; iChar < nChar; iChar++)
			{
				currentTextNodeText += (wchar_t(textNode->text[iChar]));
			}
			if (textNode->vietnameseAbbreviationIndentifier)
			{
				if (textNode->leftVietnameseAbbreviationSure > 0 || textNode->rightVietnameseAbbreviationSure > 0)
				{
					textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
					trustedAbbreviationSet.insert(currentTextNodeText);
				}
				else if (trustedAbbreviationSet.find(currentTextNodeText) != trustedAbbreviationSet.end())
				{
					textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
				}
			}
			if (textNode->vietnameseLoanWordIndentifier)
			{
				if (textNode->leftVietnameseLoanSure > 0 || textNode->rightVietnameseLoanSure > 0)
				{
					textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD;
					trustedLoanSet.insert(currentTextNodeText);
				}
				else if (trustedLoanSet.find(currentTextNodeText) != trustedLoanSet.end())
				{
					textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD;
				}
			}
		}
	}
	if (vietnameseSyllableIdentifier > 0 && (textNode->leftVietnameseSyllableSure > 0 || textNode->vietnameseSyllableSure > 0 || textNode->rightVietnameseSyllableSure > 0)) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
	else if (textNode->vietnameseAbbreviationIndentifier > 0 && (textNode->leftVietnameseAbbreviationSure > 0 || textNode->vietnameseAbbreviationSure > 0 || textNode->rightVietnameseAbbreviationSure > 0 || (vnabbreviations[textNode->vietnameseAbbreviationIndentifier].contextRequire == 0 && textNode->capital == TEXT_NODE_CAPITAL_CAPITAL))) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
	else if (textNode->englishWordIdentifier > 0 && (textNode->leftEnglishWordSure > 0 || textNode->englishWordSure > 0 || textNode->rightEnglishWordSure > 0)) textNode->textNodeType = TEXT_NODE_TYPE_ENGLISH_WORD;
	else if (textNode->vietnameseLoanWordIndentifier > 0 && (textNode->leftVietnameseLoanSure > 0 || textNode->vietnameseLoanSure > 0 || textNode->rightVietnameseLoanSure > 0)) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD;
	else if (textNode->japaneseWordIdentifier > 0) textNode->textNodeType = TEXT_NODE_TYPE_JAPANESE_WORD;
	else if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
	{
		if (vnsyllables[vietnameseSyllableIdentifier].significant && vnsyllables[vietnameseSyllableIdentifier].vietnamese) textNode->vietnameseSyllableSure = 1;
		if (textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
		else if (textNode->vietnameseAbbreviationIndentifier > 0)
		{
			if (vnabbreviations[vietnameseSyllableIdentifier].vietnamese) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
			else textNode->textNodeType = TEXT_NODE_TYPE_SPELL_IN_VENGLISH;
		}
		else if (textNode->englishWordIdentifier > 0) textNode->textNodeType = TEXT_NODE_TYPE_ENGLISH_WORD;
		else if (textNode->vietnameseLoanWordIndentifier > 0)  textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD;
	}
	if (flagValidateToolMode && textNode->vietnameseSyllableIdentifier) textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
}
void				VietnameseTextNormalizer::UpdateVietnameseTextNodeContext(TEXT_NODE* textNode)
{
	for (int isyllable = 0, maxWordLength = 4/*word max length*/ + 2/*left sure max length*/; isyllable < maxWordLength && textNode; isyllable++, textNode = textNode->next)
	{
		TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
		//TEXT_NODE *				leftTextNodeOffset5 = &nullTextNodeForStep2;
		if (textNode->back)
		{
			leftTextNodeOffset0 = textNode->back;
			if (leftTextNodeOffset0->back)
			{
				leftTextNodeOffset1 = leftTextNodeOffset0->back;
				if (leftTextNodeOffset1->back)
				{
					leftTextNodeOffset2 = leftTextNodeOffset1->back;
					if (leftTextNodeOffset2->back)
					{
						leftTextNodeOffset3 = leftTextNodeOffset2->back;
						if (leftTextNodeOffset3->back)
						{
							leftTextNodeOffset4 = leftTextNodeOffset3->back;
							if (leftTextNodeOffset4->back)
							{
								//leftTextNodeOffset5 = leftTextNodeOffset4->back;
							}
						}
					}
				}
			}
		}
		//				TEXT_NODE *				rightTextNodeOffset0 = &nullTextNodeForStep2;
		//				TEXT_NODE *				rightTextNodeOffset1 = &nullTextNodeForStep2;
		//				TEXT_NODE *				rightTextNodeOffset2 = &nullTextNodeForStep2;
		//				TEXT_NODE *				rightTextNodeOffset3 = &nullTextNodeForStep2;
		//				TEXT_NODE *				rightTextNodeOffset4 = &nullTextNodeForStep2;
		//				//TEXT_NODE *				rightTextNodeOffset5 = &nullTextNodeForStep2;
		//				if (textNode->next)
		//				{
		//					rightTextNodeOffset0 = textNode->next;
		//					if (rightTextNodeOffset0->next)
		//					{
		//						rightTextNodeOffset1 = rightTextNodeOffset0->next;
		//						if (rightTextNodeOffset1->next)
		//						{
		//							rightTextNodeOffset2 = rightTextNodeOffset1->next;
		//							if (rightTextNodeOffset2->next)
		//							{
		//								rightTextNodeOffset3 = rightTextNodeOffset2->next;
		//								if (rightTextNodeOffset3->next)
		//								{
		//									rightTextNodeOffset4 = rightTextNodeOffset3->next;
		//									if (rightTextNodeOffset4->next)
		//									{
		//										//rightTextNodeOffset5 = rightTextNodeOffset4->next;
		//									}
		//								}
		//							}
		//						}
		//					}
		//				}
		UpdateVietnameseTextNodeContext(textNode, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
	}
}
/************************************************************************/
/* Step 1 : Division                                                    */
/************************************************************************/
bool				VietnameseTextNormalizer::IsValidDate(int day, int month, int year)
{
	const int songaytrongthang[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	return (day > 0 && month > 0 && year > 0 && month < 13 && (day <= songaytrongthang[month - 1] + (month == 2) * ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)));
}
TEXT_NODE* VietnameseTextNormalizer::InsertVietnameseSyllableToTheTail(qvsylidentifier vietnameseSyllableIdentifier, qwchar const* nodeOriginalText, int nodeOriginalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE* leftTextNodeOffset0, TEXT_NODE* leftTextNodeOffset1, TEXT_NODE* leftTextNodeOffset2, TEXT_NODE* leftTextNodeOffset3, TEXT_NODE* leftTextNodeOffset4)
{
#ifdef _DEBUG
	if (capital == TEXT_NODE_CAPITAL_UNKNOWN)
	{
		printf("Code sai capital==TEXT_NODE_CAPITAL_UNKNOWN\n");
#ifdef WIN32
		_getch();
#endif
	}
#endif
	TEXT_NODE* textNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
	if (textNode/*!=NULL*/)
	{
		VIETNAMESE_SYLLABLE const* vietnameseSyllableNode = vnsyllables + vietnameseSyllableIdentifier;
		textNode->vietnameseSyllableIdentifier = vietnameseSyllableIdentifier;
		if (vietnameseSyllableNode->english) textNode->englishWordIdentifier = vietnameseSyllableNode->english;
		textNode->originalText = nodeOriginalText;
		textNode->originalTextLength = nodeOriginalTextLength;
		switch (capital)
		{
		case TEXT_NODE_CAPITAL_LOWER:textNode->text = vietnameseSyllableNode->lower; textNode->textLength = vietnameseSyllableNode->length; break;
		case TEXT_NODE_CAPITAL_UPPER:textNode->text = vietnameseSyllableNode->upper; textNode->textLength = vietnameseSyllableNode->length; break;
		case TEXT_NODE_CAPITAL_CAPITAL:textNode->text = vietnameseSyllableNode->capital; textNode->textLength = vietnameseSyllableNode->length; break;
		default:textNode->text = nodeOriginalText; textNode->textLength = nodeOriginalTextLength; break;
		}
		textNode->capital = capital;
		UpdateVietnameseTextNodeContext(textNode, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
		/************************************************************************/
		/* Insert                                                               */
		/************************************************************************/
		if (head/*!=NULL*/)
		{
			textNode->back = tail;
			tail->next = textNode;
			tail = textNode;
			countTotalNode++;
		}
		else
		{
			head = textNode;
			tail = textNode;
			countTotalNode = 1;
		}
	}
	else
	{
		printf("\nVietnameseTextNormalizer : Calloc fail!");
		DungManHinh;
		return &nullTextNodeForStep1Input;
	}
	return textNode;
}
TEXT_NODE* VietnameseTextNormalizer::InsertEnglishWordToTheTail(qvwrdidentifier englishWordIdentifier, qwchar const* nodeOriginalText, int nodeOriginalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE* leftTextNodeOffset0, TEXT_NODE* leftTextNodeOffset1, TEXT_NODE* leftTextNodeOffset2, TEXT_NODE* leftTextNodeOffset3, TEXT_NODE* leftTextNodeOffset4)
{
#ifdef _DEBUG
	if (capital == TEXT_NODE_CAPITAL_UNKNOWN)
	{
		printf("Code sai capital==TEXT_NODE_CAPITAL_UNKNOWN\n");
#ifdef WIN32
		_getch();
#endif
	}
#endif
	TEXT_NODE* textNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
	if (textNode/*!=NULL*/)
	{
		ENGLISH_WORD const* englishwordNode = enwords + englishWordIdentifier;
		/* TextNode infomation */
		textNode->originalText = nodeOriginalText;
		textNode->originalTextLength = nodeOriginalTextLength;
		textNode->text = nodeOriginalText;
		textNode->textLength = nodeOriginalTextLength;
		textNode->capital = capital;
		textNode->englishWordIdentifier = englishWordIdentifier;
		textNode->vietnameseLoanWordIndentifier = englishwordNode->loan;
		textNode->vietnameseAbbreviationIndentifier = englishwordNode->abbreviation;
		UpdateVietnameseTextNodeContext(textNode, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
		/*fix some english word*/
		if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
		{
			textNode->textNodeType = TEXT_NODE_TYPE_ENGLISH_WORD;
			if (leftTextNodeOffset0->englishWordIdentifier && (
				leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_T_H_E
				|| leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_I
				|| leftTextNodeOffset0->textNodeType == TEXT_NODE_TYPE_UNKNOWN))
			{
				leftTextNodeOffset0->vietnameseSyllableIdentifier = 0;
				leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_ENGLISH_WORD;

			}
		}
		/* insert */
		if (head/*!=NULL*/)
		{
			textNode->back = tail;
			tail->next = textNode;
			tail = textNode;
			countTotalNode++;
		}
		else
		{
			head = textNode;
			tail = textNode;
			countTotalNode = 1;
		}
	}
	else
	{
		printf("\nVietnameseTextNormalizer : Calloc fail!");
		DungManHinh;
		return &nullTextNodeForStep1Input;
	}
	return textNode;
}
TEXT_NODE* VietnameseTextNormalizer::InsertJapaneseWordToTheTail(qjwrdidentifier japaneseWordIdentifier, qwchar const* nodeOriginalText, int nodeOriginalTextLength, TEXT_NODE_CAPITAL capital, TEXT_NODE* leftTextNodeOffset0, TEXT_NODE* leftTextNodeOffset1, TEXT_NODE* leftTextNodeOffset2, TEXT_NODE* leftTextNodeOffset3, TEXT_NODE* leftTextNodeOffset4)
{
#ifdef _DEBUG
	if (capital == TEXT_NODE_CAPITAL_UNKNOWN)
	{
		printf("Code sai capital==TEXT_NODE_CAPITAL_UNKNOWN\n");
#ifdef WIN32
		_getch();
#endif
	}
#endif
	TEXT_NODE* textNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
	if (textNode/*!=NULL*/)
	{
		/* TextNode infomation */
		textNode->originalText = nodeOriginalText;
		textNode->originalTextLength = nodeOriginalTextLength;
		textNode->text = nodeOriginalText;
		textNode->textLength = nodeOriginalTextLength;
		textNode->capital = capital;
		textNode->japaneseWordIdentifier = japaneseWordIdentifier;
		UpdateVietnameseTextNodeContext(textNode, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
		/* insert */
		if (head/*!=NULL*/)
		{
			textNode->back = tail;
			tail->next = textNode;
			tail = textNode;
			countTotalNode++;
		}
		else
		{
			head = textNode;
			tail = textNode;
			countTotalNode = 1;
		}
	}
	else
	{
		printf("\nVietnameseTextNormalizer : Calloc fail!");
		DungManHinh;
		return &nullTextNodeForStep1Input;
	}
	return textNode;
}
TEXT_NODE* VietnameseTextNormalizer::InsertUnknownNodeToTail(qwchar const* nodeOriginalText, int nodeOriginalTextLength, TEXT_NODE* leftTextNodeOffset0, TEXT_NODE* leftTextNodeOffset1, TEXT_NODE* leftTextNodeOffset2, TEXT_NODE* leftTextNodeOffset3, TEXT_NODE* leftTextNodeOffset4)
{
	TEXT_NODE* textNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
	if (textNode/*!=NULL*/)
	{
		/* TextNode infomation */
		if (nodeOriginalTextLength == 1)
		{
			switch (*nodeOriginalText)
			{
			case 0x2C/*,*/:
				textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
				textNode->silenceTimeInSecond = silenceCommaTime;
				break;

			case 0x21/*!*/:
			case 0x3F/*?*/:
			case 0x2E/*.*/:
				textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
				textNode->silenceTimeInSecond = silenceSentenceTime;
				break;

			case 0xD/*\r Carriage Return*/:
			case 0xA/*\n Line Feed*/:
				textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
				textNode->silenceTimeInSecond = silenceNewLineTime;
				break;

			case 0x3B/*;*/:
				textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
				textNode->silenceTimeInSecond = silenceSemicolonTime;
				break;


			case 0x2019/*’ right single quotation mark*/:
			case 0x201D/*” right double quotation mark*/:

			case 0x28/*(*/:
			case 0x29/*)*/:
			case 0x2018/*‘ left single quotation mark*/:

			case 0x201C/*“ left double quotation mark*/:

			case 0x27/*' single quotation mark*/:
			case 0x22/*" double quotation mark*/:
				if (leftTextNodeOffset0->textNodeType != TEXT_NODE_TYPE_SILENCE
					&& leftTextNodeOffset0->silenceTimeInSecond == 0.0
					&& leftTextNodeOffset1->textNodeType != TEXT_NODE_TYPE_SILENCE
					&& leftTextNodeOffset1->silenceTimeInSecond == 0.0
					)
				{
					textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
					textNode->silenceTimeInSecond = silenceQuotationTime;
				}
				else
				{
					//textNode->textNodeType = TEXT_NODE_TYPE_IGNORE_NODE;
					textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
					textNode->silenceTimeInSecond = silenceQuotationTime;
				}
				break;


			case 0x3A/*:*/:
				if (leftTextNodeOffset0->textNodeType == TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE
					|| leftTextNodeOffset0->textNodeType == TEXT_NODE_TYPE_ENGLISH_WORD
					|| leftTextNodeOffset0->textNodeType == TEXT_NODE_TYPE_SILENCE
					|| leftTextNodeOffset0->vietnameseSyllableIdentifier > 0
					|| leftTextNodeOffset0->englishWordIdentifier > 0
					|| leftTextNodeOffset0->vietnameseAbbreviationIndentifier > 0
					|| leftTextNodeOffset0->vietnameseLoanWordIndentifier > 0
					|| leftTextNodeOffset0->englishSyllableIdentifier > 0
					)
				{
					textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
					textNode->silenceTimeInSecond = silenceColonTime;
				}
				break;
			case 0x2A/***/: textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; textNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_S_A_O; break;
			case 0x23/*#*/: textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; textNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_T_H_AW_N_G; break;
			case 0x3D/*=*/:textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; textNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_B_AWF_N_G; break;

			case 0x2D/*-*/:
			case  0x2013/*–*/:
				if (flagValidateToolMode)
				{
					textNode->textNodeType = TEXT_NODE_TYPE_IGNORE_NODE;
				}
				else
				{
					textNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
					textNode->silenceTimeInSecond = silenceShortTime;
				}

				break;
			}
		}
		if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
		{
			textNode->vietnameseAbbreviationIndentifier = CapitalAbbreviationDetection(nodeOriginalText);
			if (textNode->vietnameseAbbreviationIndentifier)
			{
				textNode->capital = TEXT_NODE_CAPITAL_CAPITAL;
				textNode->leftVietnameseAbbreviationSure = vnabbreviations[textNode->vietnameseAbbreviationIndentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier);
				switch (textNode->leftVietnameseAbbreviationSure)
				{
				case 5:
					leftTextNodeOffset4->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset4->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					/*case through here*/
				case 4:
					leftTextNodeOffset3->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset3->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					/*case through here*/
				case 3:
					leftTextNodeOffset2->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset2->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					/*case through here*/
				case 2:
					leftTextNodeOffset1->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset1->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					/*case through here*/
				case 1:
					leftTextNodeOffset0->splittable = TEXT_NODE_CAN_NOT_SPLIT;
					leftTextNodeOffset0->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					/*case through here*/
					textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_ABBREVIATION;
				}
			}
		}
		if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
		{
			TEXT_NODE_CAPITAL			capital = TEXT_NODE_CAPITAL_UNKNOWN;
			textNode->vietnameseLoanWordIndentifier = LoanWordDetection(nodeOriginalText, &capital);
			if (textNode->vietnameseLoanWordIndentifier)
			{
				textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_LOAN_WORD;
				if (capital == TEXT_NODE_CAPITAL_UNKNOWN) textNode->capital = TEXT_NODE_CAPITAL_LOWER;
				else textNode->capital = capital;
			}
		}
		if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
		{
			if (UniformResourceIdentifierScheme(nodeOriginalText)) textNode->textNodeType = TEXT_NODE_TYPE_URL;
		}
		if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
		{
			TEXT_NODE_CAPITAL capital = TEXT_NODE_CAPITAL_UNKNOWN;
			textNode->japaneseWordIdentifier = JapaneseRomanjiWordDetection(nodeOriginalText, &capital);
			if (textNode->japaneseWordIdentifier > 0)
			{
				if (capital == TEXT_NODE_CAPITAL_UNKNOWN) textNode->capital = TEXT_NODE_CAPITAL_LOWER;
				textNode->capital = capital;
			}
		}
		if (flagValidateToolMode && textNode->textNodeType == TEXT_NODE_TYPE_IGNORE_NODE)
		{
			qfree(textNode);
			return &nullTextNodeForStep1Input;
		}
		else
		{
			textNode->text = nodeOriginalText;
			textNode->textLength = nodeOriginalTextLength;
			textNode->originalText = nodeOriginalText;
			textNode->originalTextLength = nodeOriginalTextLength;
			/* Insert*/
			if (head/*!=NULL*/)
			{//nếu danh sách đã có một node nào đó
				textNode->back = tail;
				tail->next = textNode;
				tail = textNode;
				countTotalNode++;
			}
			else
			{//nếu danh sách rỗng
				head = textNode;
				tail = textNode;
				countTotalNode = 1;
			}
			if (textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
			{
				countTotalUnknownNode++;
				if (uhead == 0) uhead = textNode;
				utail = textNode;
			}
		}
	}
	else
	{
		printf("\nVietnameseTextNormalizer : Calloc fail!");
		DungManHinh;
		return &nullTextNodeForStep1Input;
	}
	return textNode;
}
TEXT_NODE* VietnameseTextNormalizer::InsertShortPauseNode(TEXT_NODE* textNode)
{
	//const qwchar		insertPauseText[4] = { 0x20,0x7C,0x20,0x0 }; // dấu |
	const qwchar		insertPauseText[4] = { 0x20,0x2C,0x20,0x0 }; // dấu ,
	const int 			insertPauseLength = 3;
	TEXT_NODE* insertNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
	if (insertNode)
	{
		insertNode->text = insertPauseText;
		insertNode->textLength = insertPauseLength;
		insertNode->originalText = textNode->originalText + textNode->originalTextLength;
		insertNode->originalTextLength = 0;
		insertNode->silenceTimeInSecond = silenceShortTime;
		insertNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
		//insert
		if (textNode/*!=NULL*/)
		{
			insertNode->next = textNode->next;
			insertNode->back = textNode;
			if (textNode->next) textNode->next->back = insertNode;
			textNode->next = insertNode;
			if (tail == textNode) tail = insertNode;
		}
		else /*if(head==NULL)*/
		{
			head = insertNode;
			tail = insertNode;
		}
		return insertNode;
	}
	else
	{
		printf("\nVietnameseTextNormalizer : Calloc fail!");
		DungManHinh;
	}
	return textNode;
}
void				VietnameseTextNormalizer::Input(const qwchar* text)
{
	FILE* backupLogFile = logFile;
	Refresh();
	logFile = backupLogFile;
	/************************************************************************/
	/*                                                                      */
	/* Bước 1 : Tách sơ bộ dựa theo cách, table , null và xuống dòng:       */
	/*                                                                      */
	/************************************************************************/
	if (text/*!=NULL*/)
	{
		qtime						step1StartTime = { 0 };
		static TEXT_NODE			nullTextNode/*For Optimization Performance*/;
		TEXT_NODE* leftTextNodeOffset0 = &nullTextNode;
		TEXT_NODE* leftTextNodeOffset1 = &nullTextNode;
		TEXT_NODE* leftTextNodeOffset2 = &nullTextNode;
		TEXT_NODE* leftTextNodeOffset3 = &nullTextNode;
		TEXT_NODE* leftTextNodeOffset4 = &nullTextNode;
		qwchar const* currentOriginalSyllable = text;
		int							currentOriginalSyllableLength = 0;
		originalText = text;
		if (logFile) qGetTime(&step1StartTime);
		for (char flagNeedMoreLoop = ((*text) != 0), startOfNewNode = 1, currentIsUrl = 0; flagNeedMoreLoop;)
		{
			TEXT_NODE_CAPITAL			capital = TEXT_NODE_CAPITAL_UNKNOWN;
			int							combiningTone = 0;
			qvsylidentifier				leftMatchingVietnameseIdentifier = 0;
			qewrdidentifier				leftMatchingEnglishIdentifier = 0;
			int							leftMatchingCombiningTone = 0;
			int							leftMatchingVietnameseLength = 0;
			int							leftMatchingEnglishLength = 0;
			int							pronoucingIndex = 0;
			int							preloadSize = 0;
			bool						preloadTagVi = false;
			bool						preloadTagEn = false;
			bool						preloadTagB = false;
			bool						preloadTagC = false;
			//bool						preloadTagS = false;
			bool						preloadTagL = false;
			bool						preloadTagQ = false;
			//bool						preloadTagE = false;
			bool						preloadTagRetroflex = false;
			bool						preloadViEnDup = false;
			if (startOfNewNode && text[0] == 0x3C/*<*/)
			{
				qwchar const* backupText = text;
				bool flagNeedMoreLoopForCheckPreloadTag = true;
				while (flagNeedMoreLoopForCheckPreloadTag)
				{

					if (text[0] == 0x3C/*<*/)/*start with [<..] */
					{
						switch (text[1])/*start with [<..] : 13 way and 7 case. Cost 1*/
						{
						case 0x62/*b*/:/*text [<b>] cost 3*/if (text[2] == 0x3E/*>*/) { text += 3; preloadSize += 3; preloadTagB = true; }break;
						case 0x63/*c*/:/*text [<c>] cost 4*/if (text[2] == 0x3E/*>*/) { text += 3; preloadSize += 3; preloadTagC = true; }break;
						case 0x65/*e*/:
							switch (text[2])/*start with [<e..] : 5 way and 2 case 1 return. Cost 4*/
							{
							case 0x6E/*n*/:
								switch (text[3])/*start with [<en..] : 4 way and 4 case 1 return. Cost 5*/
								{
								case 0x3E/*>*/:/*text [<en>] cost 6*/text += 4; preloadSize += 4; preloadTagEn = true; break;
								case 0x31/*1*/:/*text [<en1>] cost 8*/if (text[4] == 0x3E/*>*/) { text += 5; preloadSize += 5; preloadTagEn = true; pronoucingIndex = 1; }break;
								case 0x32/*2*/:/*text [<en2>] cost 9*/if (text[4] == 0x3E/*>*/) { text += 5; preloadSize += 5; preloadTagEn = true; pronoucingIndex = 2; }break;
								case 0x33/*3*/:/*text [<en3>] cost 10*/if (text[4] == 0x3E/*>*/) { text += 5; preloadSize += 5; preloadTagEn = true; pronoucingIndex = 3; }break;
								}//end of switch (text[3])/*start with [<en..]*/
								break;
							case 0x3E/*>*/:/*text [<e>] cost 6*/text += 3; preloadSize += 3;/* preloadTagE = true;*/ break;
							}//end of switch (text[2])/*start with [<e..]*/
							break;
						case 0x6C/*l*/:/*text [<l>] cost 6*/if (text[2] == 0x3E/*>*/) { text += 3; preloadSize += 3; preloadTagL = true; }break;
							//case 0x71/*q*/:/*text [<q>] cost 7*/if (text[2] == 0x3E/*>*/) { text += 3; preloadSize += 3; preloadTagQ = true; }break;
						case 0x73/*s*/:/*text [<s>] cost 8*/if (text[2] == 0x3E/*>*/) { text += 3; preloadSize += 3;/* preloadTagS = true;*/ }break;
						case 0x76/*v*/:
							if (text[2] == 0x69/*i*/)/*start with [<vi..] */
							{
								switch (text[3])/*start with [<vi..] : 3 way and 3 case 1 return. Cost 9*/
								{
								case 0x65/*e*/:/*text [<viendup>] cost 15*/if (text[4] == 0x6E/*n*/ && text[5] == 0x64/*d*/ && text[6] == 0x75/*u*/ && text[7] == 0x70/*p*/ && text[8] == 0x3E/*>*/) { text += 9; preloadSize += 9; preloadViEnDup = true; }break;
								case 0x3E/*>*/:/*text [<vi>] cost 11*/text += 4; preloadSize += 4; preloadTagVi = true; break;
								case 0x5F/*_*/:/*text [<vi_r>] cost 14*/if (text[4] == 0x72/*r*/ && text[5] == 0x3E/*>*/) { text += 6; preloadSize += 6; preloadTagRetroflex = true; preloadTagVi = true; }break;
								}//end of switch (text[3])/*start with [<vi..]*/

							}/*end of if start with [<vi..] */
							break;
						}//end of switch (text[1])/*start with [<..]*/

					}/*end of if start with [<..] */














					flagNeedMoreLoopForCheckPreloadTag = (backupText != text);
					backupText = text;
				}
			}
			qvsylidentifier			vietnameseSyllableIdentifier = (startOfNewNode && preloadTagEn == false) ? VietnameseSyllableDetection(text, &capital, &combiningTone, &leftMatchingVietnameseIdentifier, &leftMatchingCombiningTone) : 0;
			if (vietnameseSyllableIdentifier == 0 && startOfNewNode && preloadTagEn == false) VietnameseSyllableWithHTMLEncodeDetection(text, &vietnameseSyllableIdentifier, &currentOriginalSyllableLength, &leftMatchingVietnameseIdentifier, &leftMatchingVietnameseLength, &capital);
			qvwrdidentifier			englishWordIdentifier = (startOfNewNode && vietnameseSyllableIdentifier == 0 && preloadTagVi == false) ? EnglishWordDetection(text, &capital, &leftMatchingEnglishIdentifier) : 0;
			qvloansylidentifier		vietnameseLoanWordIndentifier = (startOfNewNode && vietnameseSyllableIdentifier == 0 && englishWordIdentifier == 0) ? LoanWordDetection(text, &capital) : 0;
			qvabbsylidentifier		vietnameseAbbreviationIndentifier = CapitalAbbreviationDetection(text);
			qjwrdidentifier			japaneseWordIdentifier = (startOfNewNode && vietnameseSyllableIdentifier == 0 && englishWordIdentifier == 0 && vietnameseLoanWordIndentifier == 0) ? JapaneseRomanjiWordDetection(text, &capital) : 0;
			/************************************************************************/
			/* currentOriginalSyllableLength                                        */
			/************************************************************************/
			if (currentOriginalSyllableLength == 0)
			{
				if (vietnameseSyllableIdentifier != 0) currentOriginalSyllableLength = vnsyllables[vietnameseSyllableIdentifier].length + combiningTone;
				else if (englishWordIdentifier != 0) currentOriginalSyllableLength = enwords[englishWordIdentifier].textLength;
				else if (japaneseWordIdentifier != 0) currentOriginalSyllableLength = japaneseWords[japaneseWordIdentifier].romanjiTextLength;
			}
			/************************************************************************/
			/* Left matching                                                        */
			/************************************************************************/
			if (leftMatchingVietnameseIdentifier != 0 && leftMatchingVietnameseLength == 0) leftMatchingVietnameseLength = vnsyllables[leftMatchingVietnameseIdentifier].length + leftMatchingCombiningTone;
			if (leftMatchingEnglishIdentifier && leftMatchingEnglishLength == 0) leftMatchingEnglishLength = enwords[leftMatchingEnglishIdentifier].textLength;
			/************************************************************************/
			/* Preload tag                                                          */
			/************************************************************************/
			if (preloadTagEn && englishWordIdentifier == 0 && (text[1] == 0x20/*Space*/ || text[1] == 0x0/*NULL*/))
			{
				switch (text[0])
				{
				case 0x41/*A*/:case 0x61/*a*/:englishWordIdentifier = ENGLISH_WORD_A; break;
				case 0x42/*B*/:case 0x62/*b*/:englishWordIdentifier = ENGLISH_WORD_B; break;
				case 0x43/*C*/:case 0x63/*c*/:englishWordIdentifier = ENGLISH_WORD_C; break;
				case 0x44/*D*/:case 0x64/*d*/:englishWordIdentifier = ENGLISH_WORD_D; break;
				case 0x45/*E*/:case 0x65/*e*/:englishWordIdentifier = ENGLISH_WORD_E; break;
				case 0x46/*F*/:case 0x66/*f*/:englishWordIdentifier = ENGLISH_WORD_F; break;
				case 0x47/*G*/:case 0x67/*g*/:englishWordIdentifier = ENGLISH_WORD_G; break;
				case 0x48/*H*/:case 0x68/*h*/:englishWordIdentifier = ENGLISH_WORD_H; break;
				case 0x49/*I*/:case 0x69/*i*/:englishWordIdentifier = ENGLISH_WORD_I; break;
				case 0x4A/*J*/:case 0x6A/*j*/:englishWordIdentifier = ENGLISH_WORD_J; break;
				case 0x4B/*K*/:case 0x6B/*k*/:englishWordIdentifier = ENGLISH_WORD_K; break;
				case 0x4C/*L*/:case 0x6C/*l*/:englishWordIdentifier = ENGLISH_WORD_L; break;
				case 0x4D/*M*/:case 0x6D/*m*/:englishWordIdentifier = ENGLISH_WORD_M; break;
				case 0x4E/*N*/:case 0x6E/*n*/:englishWordIdentifier = ENGLISH_WORD_N; break;
				case 0x4F/*O*/:case 0x6F/*o*/:englishWordIdentifier = ENGLISH_WORD_O; break;
				case 0x50/*P*/:case 0x70/*p*/:englishWordIdentifier = ENGLISH_WORD_P; break;
				case 0x51/*Q*/:case 0x71/*q*/:englishWordIdentifier = ENGLISH_WORD_Q; break;
				case 0x52/*R*/:case 0x72/*r*/:englishWordIdentifier = ENGLISH_WORD_R; break;
				case 0x53/*S*/:case 0x73/*s*/:englishWordIdentifier = ENGLISH_WORD_S; break;
				case 0x54/*T*/:case 0x74/*t*/:englishWordIdentifier = ENGLISH_WORD_T; break;
				case 0x55/*U*/:case 0x75/*u*/:englishWordIdentifier = ENGLISH_WORD_U; break;
				case 0x56/*V*/:case 0x76/*v*/:englishWordIdentifier = ENGLISH_WORD_V; break;
				case 0x57/*W*/:case 0x77/*w*/:englishWordIdentifier = ENGLISH_WORD_W; break;
				case 0x58/*X*/:case 0x78/*x*/:englishWordIdentifier = ENGLISH_WORD_X; break;
				case 0x59/*Y*/:case 0x79/*y*/:englishWordIdentifier = ENGLISH_WORD_Y; break;
				case 0x5A/*Z*/:case 0x7A/*z*/:englishWordIdentifier = ENGLISH_WORD_Z; break;
				}
			}
			if (englishWordIdentifier == 0 && preloadTagEn)
			{
				Log(originalText, originalTextLength);
				Log(" : preload tag <en> - but (englishWordIdentifier == 0) !\r\n");
				if (pronoucingIndex) text -= 5;
				else text -= 4;
			}
			if (englishWordIdentifier && pronoucingIndex && pronoucingIndex <= enwords[englishWordIdentifier].pronoucing) englishWordIdentifier += pronoucingIndex;
			if (englishWordIdentifier == 0 && leftMatchingEnglishIdentifier && pronoucingIndex && pronoucingIndex <= enwords[leftMatchingEnglishIdentifier].pronoucing) leftMatchingEnglishIdentifier += pronoucingIndex;
			if (vietnameseSyllableIdentifier != 0 && preloadTagEn == false && (preloadTagB || preloadTagL || preloadTagQ || preloadTagRetroflex)) englishWordIdentifier = 0;
			/************************************************************************/
			/*                                                                      */
			/************************************************************************/
			if (capital == TEXT_NODE_CAPITAL_UNKNOWN)
			{
				if (vietnameseSyllableIdentifier > 0 || englishWordIdentifier > 0 || japaneseWordIdentifier > 0) capital = TEXT_NODE_CAPITAL_LOWER;
				else if (vietnameseAbbreviationIndentifier > 0) capital = TEXT_NODE_CAPITAL_CAPITAL;
				else if (leftMatchingVietnameseIdentifier > 0 || leftMatchingEnglishLength > 0) capital = TEXT_NODE_CAPITAL_LOWER;
			}


			if (preloadTagVi && vietnameseSyllableIdentifier == 0)
			{
				flagValidToStandard = false;
				//		#ifdef WIN32
				//		#ifdef _DEBUG
				//						/************************************************************************/
				//						/* Debug                                                                */
				//						/************************************************************************/
				//						wchar_t bufferError[100] = { 0 };
				//						for (int iChar = 0; iChar < currentOriginalSyllableLength + preloadSize && iChar < 99; iChar++)
				//						{
				//							bufferError[iChar] = currentOriginalSyllable[iChar];
				//						}
				//						MessageBoxW(0, currentOriginalSyllable, L"Dữ liệu input vào không chuẩn!!", MB_OK);
				//		#endif
				//		#endif
			}


			/************************************************************************/
			/* Nếu là từ tiếng việt kết thúc bởi dấu cách                           */
			/************************************************************************/
			if (vietnameseSyllableIdentifier)
			{
				TEXT_NODE* backupTextNode = InsertVietnameseSyllableToTheTail(vietnameseSyllableIdentifier, currentOriginalSyllable + preloadSize, currentOriginalSyllableLength, capital, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
				if (preloadTagVi)
				{
					backupTextNode->englishWordIdentifier = 0;
					backupTextNode->vietnameseLoanWordIndentifier = 0;
					if (vietnameseSyllableIdentifier)
					{
						backupTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					}
				}
				if (preloadTagRetroflex)
				{
					backupTextNode->englishWordIdentifier = 0;
					backupTextNode->vietnameseLoanWordIndentifier = 0;
					if (vietnameseSyllableIdentifier)
					{
						backupTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
					}
				}
				leftTextNodeOffset4 = leftTextNodeOffset3;
				leftTextNodeOffset3 = leftTextNodeOffset2;
				leftTextNodeOffset2 = leftTextNodeOffset1;
				leftTextNodeOffset1 = leftTextNodeOffset0;
				leftTextNodeOffset0 = backupTextNode;
				text += leftTextNodeOffset0->originalTextLength + 1/*cho dấu cách*/;
				currentOriginalSyllable = text;
				currentOriginalSyllableLength = 0;
				if (preloadTagB) backupTextNode->prosodyB = 0x31/*1*/;
				if (preloadTagC) backupTextNode->prosodyC = 0x31/*1*/;
				if (preloadTagL) backupTextNode->prosodyL = 0x31/*1*/;
				if (preloadTagQ) backupTextNode->prosodyQ = 0x31/*1*/;
				if (preloadTagVi) backupTextNode->englishWordIdentifier = 0;
				if (preloadTagRetroflex) backupTextNode->retroflex = true;
				if (preloadViEnDup) backupTextNode->viendup = true;
			}
			/************************************************************************/
			/* Nếu là từ tiếng anh kết thúc bởi dấu cách                            */
			/************************************************************************/
			else if (englishWordIdentifier)
			{
				TEXT_NODE* backupTextNode = InsertEnglishWordToTheTail(englishWordIdentifier, currentOriginalSyllable + preloadSize, currentOriginalSyllableLength, capital, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
				if (preloadTagEn) backupTextNode->vietnameseLoanWordIndentifier = 0;
				leftTextNodeOffset4 = leftTextNodeOffset3;
				leftTextNodeOffset3 = leftTextNodeOffset2;
				leftTextNodeOffset2 = leftTextNodeOffset1;
				leftTextNodeOffset1 = leftTextNodeOffset0;
				leftTextNodeOffset0 = backupTextNode;
				text += leftTextNodeOffset0->originalTextLength + 1/*cho dấu cách*/;
				currentOriginalSyllable = text;
				currentOriginalSyllableLength = 0;
			}
			/************************************************************************/
			/* Nếu là từ viết tắt tiếng việt                                        */
			/************************************************************************/
			else if (vietnameseAbbreviationIndentifier)
			{
				TEXT_NODE* backupTextNode = InsertUnknownNodeToTail(text, vnabbreviations[vietnameseAbbreviationIndentifier].textLength, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
				leftTextNodeOffset4 = leftTextNodeOffset3;
				leftTextNodeOffset3 = leftTextNodeOffset2;
				leftTextNodeOffset2 = leftTextNodeOffset1;
				leftTextNodeOffset1 = leftTextNodeOffset0;
				leftTextNodeOffset0 = backupTextNode;
				text += leftTextNodeOffset0->originalTextLength + 1/*cho dấu cách*/;
				currentOriginalSyllable = text;
				currentOriginalSyllableLength = 0;
			}
			/************************************************************************/
			/* Nếu là từ viết mượn tiếng việt                                        */
			/************************************************************************/
			else if (vietnameseLoanWordIndentifier)
			{
				TEXT_NODE* backupTextNode = InsertUnknownNodeToTail(text, vnloans[vietnameseLoanWordIndentifier].textLength, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
				leftTextNodeOffset4 = leftTextNodeOffset3;
				leftTextNodeOffset3 = leftTextNodeOffset2;
				leftTextNodeOffset2 = leftTextNodeOffset1;
				leftTextNodeOffset1 = leftTextNodeOffset0;
				leftTextNodeOffset0 = backupTextNode;
				text += leftTextNodeOffset0->originalTextLength + 1/*cho dấu cách*/;
				currentOriginalSyllable = text;
				currentOriginalSyllableLength = 0;
			}
			/************************************************************************/
			/* Nếu là tiếng nhật                                                    */
			/************************************************************************/
			else if (japaneseWordIdentifier)
			{
				TEXT_NODE* backupTextNode = InsertJapaneseWordToTheTail(japaneseWordIdentifier, currentOriginalSyllable + preloadSize, currentOriginalSyllableLength, capital, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
				leftTextNodeOffset4 = leftTextNodeOffset3;
				leftTextNodeOffset3 = leftTextNodeOffset2;
				leftTextNodeOffset2 = leftTextNodeOffset1;
				leftTextNodeOffset1 = leftTextNodeOffset0;
				leftTextNodeOffset0 = backupTextNode;
				text += leftTextNodeOffset0->originalTextLength + 1/*cho dấu cách*/;
				currentOriginalSyllable = text;
				currentOriginalSyllableLength = 0;
			}
			/************************************************************************/
			/* Nếu chưa rõ                                                          */
			/************************************************************************/
			else
			{
				bool								needSplitLeftMatchingVietnameseSyllable = false;
				bool								needSplitLeftMatchingEnglishWord = false;
				bool								needSplitLeftCharacter = false;
				bool								needIgnoreLeftCharacter = false;
				char								needIgnoreSeparateCharacter = false;
				int									leftMatchingEnglishWordLength = enwords[leftMatchingEnglishIdentifier].textLength;
				struct VIETNAMESE_SYLLABLE const* leftMatchingVietnameseSyllableNode = vnsyllables + leftMatchingVietnameseIdentifier;
				int									leftMatchingVietnameseWordIdentifier = leftMatchingVietnameseSyllableNode->DetectWord(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier);
				int									leftMatchingVietnameseSyllableSure = leftMatchingVietnameseSyllableNode->LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier)
					+ vnwords[leftMatchingVietnameseWordIdentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
				;

				TEXT_NODE_CAPITAL 					rightCapital = TEXT_NODE_CAPITAL_UNKNOWN;
				int									rightCombiningTone = 0;
				qvsylidentifier						rightTempLeftMatchingVietnameseIdentifier = 0;
				int									rightTempLeftMatchingCombiningTone = 0;
				qvsylidentifier						rightMathchingVietnameseIdentifier = VietnameseSyllableDetection(currentOriginalSyllable + leftMatchingVietnameseLength, &rightCapital, &rightCombiningTone, &rightTempLeftMatchingVietnameseIdentifier, &rightTempLeftMatchingCombiningTone);
				struct VIETNAMESE_SYLLABLE const* rightMatchingVietnameseSyllableNode = vnsyllables + rightMathchingVietnameseIdentifier;

				int									rightMatchingVietnameseWordIdentifier = rightMatchingVietnameseSyllableNode->DetectWord(leftMatchingVietnameseIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
				int									rightMatchingVietnameseSyllableSure = rightMatchingVietnameseSyllableNode->LeftSure(leftMatchingVietnameseIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier)
					+ vnwords[rightMatchingVietnameseWordIdentifier].LeftSure(leftMatchingVietnameseIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
				bool								flagNeedSpace = false;
				/*Rule 1 : */
				if (leftMatchingVietnameseSyllableNode->length > 1
					&& rightMatchingVietnameseSyllableNode->length > 1
					&& capital != TEXT_NODE_CAPITAL_CAPITAL
					&& rightCapital == TEXT_NODE_CAPITAL_UPPER
					&& (
						(leftMatchingVietnameseWordIdentifier && vnwords[leftMatchingVietnameseWordIdentifier].significant)
						|| leftMatchingVietnameseSyllableSure
						|| rightMatchingVietnameseSyllableSure
						|| (rightMatchingVietnameseWordIdentifier && vnwords[rightMatchingVietnameseWordIdentifier].significant)
						|| (leftMatchingVietnameseSyllableNode->vietnamese && rightMatchingVietnameseSyllableNode->vietnamese)
						)
					)
				{
					needSplitLeftMatchingVietnameseSyllable = true;
					flagNeedSpace = true;
				}
				/*Rule 2 : */
				if (leftMatchingVietnameseLength > 0 && leftMatchingVietnameseIdentifier > 0)
				{
					switch (text[leftMatchingVietnameseLength])
					{
					case 0x201C/*“ left double quotation mark*/:
					case 0x2018/*‘ left single quotation mark*/:
						needSplitLeftMatchingVietnameseSyllable = true;
						flagNeedSpace = true;
						break;
					}
				}



				if (leftMatchingVietnameseIdentifier && preloadTagEn == false)
				{
					qwchar characterNextFormLeftMatching = text[leftMatchingVietnameseLength];
					if ((characterNextFormLeftMatching & 0xFF00) == 0xFF00 && (characterNextFormLeftMatching & 0x00FF) <= 0x60)
					{
						characterNextFormLeftMatching = (qwchar)((characterNextFormLeftMatching & 0x00FF) + 0x20);
					}
					switch (characterNextFormLeftMatching)
					{
					case 0xD/*\r Carriage Return*/:
						needSplitLeftMatchingVietnameseSyllable = true;
						if (text[leftMatchingVietnameseLength + 1] == 0xA/*\n Line Feed*/) needIgnoreSeparateCharacter = 1;
						break;
					case 0xA/*\n Line Feed*/:
						needSplitLeftMatchingVietnameseSyllable = true;
						break;
					case 0x9/*Tab*/:
						needSplitLeftMatchingVietnameseSyllable = true;
						needIgnoreSeparateCharacter = 1;
						break;
					case 0x0/*NULL*/:
						originalTextLength = int(text - originalText) + leftMatchingVietnameseLength;
						flagNeedMoreLoop = 0;
						needSplitLeftMatchingVietnameseSyllable = true;
						break;
					case 0x111/*đ*/:case 0xf0/*đ*/:
					case 0x110/*Đ*/:case 0xD0/*Đ*/:
						needSplitLeftMatchingVietnameseSyllable = true;
						break;
					case 0x30/*0*/:case 0x31/*1*/:case 0x32/*2*/:case 0x33/*3*/:case 0x34/*4*/:
					case 0x35/*5*/:case 0x36/*6*/:case 0x37/*7*/:case 0x38/*8*/:case 0x39/*9*/:
						if (leftMatchingVietnameseWordIdentifier || leftMatchingVietnameseSyllableNode->vietnamese)
						{
							needSplitLeftMatchingVietnameseSyllable = true;
						}
						break;
					case 0x62/*b*/:case 0x64/*d*/:case 0x6C/*l*/:case 0x71/*q*/:case 0x76/*v*/:
					case 0x42/*B*/:case 0x44/*D*/:case 0x4C/*L*/:case 0x51/*Q*/:case 0x56/*V*/:
					case 0x72/*r*/:case 0x73/*s*/:case 0x78/*x*/:
					case 0x52/*R*/:case 0x53/*S*/:case 0x58/*X*/:
					case 0x43/*C*/:case 0x4B/*K*/:case 0x4D/*M*/:case 0x4E/*N*/:case 0x50/*P*/:case 0x54/*T*/:case 0x57/*W*/:
					case 0x63/*c*/:case 0x6B/*k*/:case 0x6D/*m*/:case 0x6E/*n*/:case 0x70/*p*/:case 0x74/*t*/:case 0x77/*w*/:
					case 0x46/*F*/:case 0x4A/*J*/:case 0x5A/*Z*/:
					case 0x66/*f*/:case 0x6A/*j*/:case 0x7A/*z*/:
					case 0x47/*G*/:case 0x67/*g*/:
					case 0x48/*H*/:case 0x68/*h*/:
					case 0x41/*A*/:case 0xC0/*À*/:case 0xC1/*Á*/:case 0x1EA2/*Ả*/:case 0xC3/*Ã*/:case 0x1EA0/*Ạ*/:
					case 0x102/*Ă*/:case 0x1EB0/*Ằ*/:case 0x1EAE/*Ắ*/:case 0x1EB2/*Ẳ*/:case 0x1EB4/*Ẵ*/:case 0x1EB6/*Ặ*/:
					case 0xC2/*Â*/:case 0x1EA6/*Ầ*/:case 0x1EA4/*Ấ*/:case 0x1EA8/*Ẩ*/:case 0x1EAA/*Ẫ*/:case 0x1EAC/*Ậ*/:
					case 0x45/*E*/:case 0xC8/*È*/:case 0xC9/*É*/:case 0x1EBA/*Ẻ*/:case 0x1EBC/*Ẽ*/:case 0x1EB8/*Ẹ*/:
					case 0xCA/*Ê*/:case 0x1EC0/*Ề*/:case 0x1EBE/*Ế*/:case 0x1EC2/*Ể*/:case 0x1EC4/*Ễ*/:case 0x1EC6/*Ệ*/:
					case 0x49/*I*/:case 0xCC/*Ì*/:case 0xCD/*Í*/:case 0x1EC8/*Ỉ*/:case 0x128/*Ĩ*/:case 0x1ECA/*Ị*/:
					case 0x4F/*O*/:case 0xD2/*Ò*/:case 0xD3/*Ó*/:case 0x1ECE/*Ỏ*/:case 0xD5/*Õ*/:case 0x1ECC/*Ọ*/:
					case 0xD4/*Ô*/:case 0x1ED2/*Ồ*/:case 0x1ED0/*Ố*/:case 0x1ED4/*Ổ*/:case 0x1ED6/*Ỗ*/:case 0x1ED8/*Ộ*/:
					case 0x1A0/*Ơ*/:case 0x1EDC/*Ờ*/:case 0x1EDA/*Ớ*/:case 0x1EDE/*Ở*/:case 0x1EE0/*Ỡ*/:case 0x1EE2/*Ợ*/:
					case 0x55/*U*/:case 0xD9/*Ù*/:case 0xDA/*Ú*/:case 0x1EE6/*Ủ*/:case 0x168/*Ũ*/:case 0x1EE4/*Ụ*/:
					case 0x1AF/*Ư*/:case 0x1EEA/*Ừ*/:case 0x1EE8/*Ứ*/:case 0x1EEC/*Ử*/:case 0x1EEE/*Ữ*/:case 0x1EF0/*Ự*/:
					case 0x59/*Y*/:case 0x1EF2/*Ỳ*/:case 0xDD/*Ý*/:case 0x1EF6/*Ỷ*/:case 0x1EF8/*Ỹ*/:case 0x1EF4/*Ỵ*/:
					case 0x61/*a*/:case 0xE0/*à*/:case 0xE1/*á*/:case 0x1EA3/*ả*/:case 0xE3/*ã*/:case 0x1EA1/*ạ*/:
					case 0x103/*ă*/:case 0x1EB1/*ằ*/:case 0x1EAF/*ắ*/:case 0x1EB3/*ẳ*/:case 0x1EB5/*ẵ*/:case 0x1EB7/*ặ*/:
					case 0xE2/*â*/:case 0x1EA7/*ầ*/:case 0x1EA5/*ấ*/:case 0x1EA9/*ẩ*/:case 0x1EAB/*ẫ*/:case 0x1EAD/*ậ*/:
					case 0x65/*e*/:case 0xE8/*è*/:case 0xE9/*é*/:case 0x1EBB/*ẻ*/:case 0x1EBD/*ẽ*/:case 0x1EB9/*ẹ*/:
					case 0xEA/*ê*/:case 0x1EC1/*ề*/:case 0x1EBF/*ế*/:case 0x1EC3/*ể*/:case 0x1EC5/*ễ*/:case 0x1EC7/*ệ*/:
					case 0x69/*i*/:case 0xEC/*ì*/:case 0xED/*í*/:case 0x1EC9/*ỉ*/:case 0x129/*ĩ*/:case 0x1ECB/*ị*/:
					case 0x6F/*o*/:case 0xF2/*ò*/:case 0xF3/*ó*/:case 0x1ECF/*ỏ*/:case 0xF5/*õ*/:case 0x1ECD/*ọ*/:
					case 0xF4/*ô*/:case 0x1ED3/*ồ*/:case 0x1ED1/*ố*/:case 0x1ED5/*ổ*/:case 0x1ED7/*ỗ*/:case 0x1ED9/*ộ*/:
					case 0x1A1/*ơ*/:case 0x1EDD/*ờ*/:case 0x1EDB/*ớ*/:case 0x1EDF/*ở*/:case 0x1EE1/*ỡ*/:case 0x1EE3/*ợ*/:
					case 0x75/*u*/:case 0xF9/*ù*/:case 0xFA/*ú*/:case 0x1EE7/*ủ*/:case 0x169/*ũ*/:case 0x1EE5/*ụ*/:
					case 0x1B0/*ư*/:case 0x1EEB/*ừ*/:case 0x1EE9/*ứ*/:case 0x1EED/*ử*/:case 0x1EEF/*ữ*/:case 0x1EF1/*ự*/:
					case 0x79/*y*/:case 0x1EF3/*ỳ*/:case 0xFD/*ý*/:case 0x1EF7/*ỷ*/:case 0x1EF9/*ỹ*/:case 0x1EF5/*ỵ*/:


					case 0x300/*VIETNAMESE_TONE_HUYEN*/:
					case 0x340/*VIETNAMESE_TONE_HUYEN*/:
					case 0x2CB/*VIETNAMESE_TONE_HUYEN*/:

					case 0x303/*VIETNAMESE_TONE_NGA*/:
					case 0x309/*VIETNAMESE_TONE_HOI*/:


					case 0x301/*VIETNAMESE_TONE_SAC*/:
					case 0xB4/*VIETNAMESE_TONE_SAC*/:
					case 0x341/*VIETNAMESE_TONE_SAC*/:
					case 0x2CA/*VIETNAMESE_TONE_SAC*/:
					case 0x1FFD/*VIETNAMESE_TONE_SAC*/:



					case 0x323/*VIETNAMESE_TONE_NANG*/:






					case 0x302/*Combining Circumflex Accent*/:
					case 0x311/*Combining Inverted Breve*/:
					case 0x306/*Combining Breve*/:
					case 0x30C/*Combining Caron*/:
					case 0x31B/*Combining Horn*/:


					case 0x200B/*Zero width space*/:
					case 0xFEFF/*Zero width no-break space*/:
						break;
					default:
						needSplitLeftMatchingVietnameseSyllable = true;
						break;
					}
				}
				if (needSplitLeftMatchingVietnameseSyllable == false && leftMatchingEnglishIdentifier)
				{
					qwchar characterNextFormLeftMatching = text[leftMatchingEnglishWordLength];
					if ((characterNextFormLeftMatching & 0xFF00) == 0xFF00 && (characterNextFormLeftMatching & 0x00FF) <= 0x60)
					{
						characterNextFormLeftMatching = (qwchar)((characterNextFormLeftMatching & 0x00FF) + 0x20);
					}
					switch (text[leftMatchingEnglishWordLength])
					{
					case 0xD/*\r Carriage Return*/:
						needSplitLeftMatchingEnglishWord = true;
						if (text[leftMatchingEnglishWordLength] == 0xA/*\n Line Feed*/) needIgnoreSeparateCharacter = 1;
						break;
					case 0xA/*\n Line Feed*/:
						needSplitLeftMatchingEnglishWord = true;
						break;
					case 0x9/*Tab*/:
						needSplitLeftMatchingEnglishWord = true;
						needIgnoreSeparateCharacter = 1;
						break;
					case 0x0/*NULL*/:
						originalTextLength = int(text - originalText) + leftMatchingEnglishWordLength;
						flagNeedMoreLoop = 0;
						needSplitLeftMatchingEnglishWord = true;
						break;
					case 0x111/*đ*/:case 0xf0/*đ*/:
					case 0x110/*Đ*/:case 0xD0/*Đ*/:
						needSplitLeftMatchingEnglishWord = true;
						break;
					case 0x30/*0*/:case 0x31/*1*/:case 0x32/*2*/:case 0x33/*3*/:case 0x34/*4*/:
					case 0x35/*5*/:case 0x36/*6*/:case 0x37/*7*/:case 0x38/*8*/:case 0x39/*9*/:
						needSplitLeftMatchingEnglishWord = true;
						break;
					case 0x62/*b*/:case 0x64/*d*/:case 0x6C/*l*/:case 0x71/*q*/:case 0x76/*v*/:
					case 0x42/*B*/:case 0x44/*D*/:case 0x4C/*L*/:case 0x51/*Q*/:case 0x56/*V*/:
					case 0x72/*r*/:case 0x73/*s*/:case 0x78/*x*/:
					case 0x52/*R*/:case 0x53/*S*/:case 0x58/*X*/:
					case 0x43/*C*/:case 0x4B/*K*/:case 0x4D/*M*/:case 0x4E/*N*/:case 0x50/*P*/:case 0x54/*T*/:case 0x57/*W*/:
					case 0x63/*c*/:case 0x6B/*k*/:case 0x6D/*m*/:case 0x6E/*n*/:case 0x70/*p*/:case 0x74/*t*/:case 0x77/*w*/:
					case 0x46/*F*/:case 0x4A/*J*/:case 0x5A/*Z*/:
					case 0x66/*f*/:case 0x6A/*j*/:case 0x7A/*z*/:
					case 0x47/*G*/:case 0x67/*g*/:
					case 0x48/*H*/:case 0x68/*h*/:
					case 0x41/*A*/:case 0xC0/*À*/:case 0xC1/*Á*/:case 0x1EA2/*Ả*/:case 0xC3/*Ã*/:case 0x1EA0/*Ạ*/:
					case 0x102/*Ă*/:case 0x1EB0/*Ằ*/:case 0x1EAE/*Ắ*/:case 0x1EB2/*Ẳ*/:case 0x1EB4/*Ẵ*/:case 0x1EB6/*Ặ*/:
					case 0xC2/*Â*/:case 0x1EA6/*Ầ*/:case 0x1EA4/*Ấ*/:case 0x1EA8/*Ẩ*/:case 0x1EAA/*Ẫ*/:case 0x1EAC/*Ậ*/:
					case 0x45/*E*/:case 0xC8/*È*/:case 0xC9/*É*/:case 0x1EBA/*Ẻ*/:case 0x1EBC/*Ẽ*/:case 0x1EB8/*Ẹ*/:
					case 0xCA/*Ê*/:case 0x1EC0/*Ề*/:case 0x1EBE/*Ế*/:case 0x1EC2/*Ể*/:case 0x1EC4/*Ễ*/:case 0x1EC6/*Ệ*/:
					case 0x49/*I*/:case 0xCC/*Ì*/:case 0xCD/*Í*/:case 0x1EC8/*Ỉ*/:case 0x128/*Ĩ*/:case 0x1ECA/*Ị*/:
					case 0x4F/*O*/:case 0xD2/*Ò*/:case 0xD3/*Ó*/:case 0x1ECE/*Ỏ*/:case 0xD5/*Õ*/:case 0x1ECC/*Ọ*/:
					case 0xD4/*Ô*/:case 0x1ED2/*Ồ*/:case 0x1ED0/*Ố*/:case 0x1ED4/*Ổ*/:case 0x1ED6/*Ỗ*/:case 0x1ED8/*Ộ*/:
					case 0x1A0/*Ơ*/:case 0x1EDC/*Ờ*/:case 0x1EDA/*Ớ*/:case 0x1EDE/*Ở*/:case 0x1EE0/*Ỡ*/:case 0x1EE2/*Ợ*/:
					case 0x55/*U*/:case 0xD9/*Ù*/:case 0xDA/*Ú*/:case 0x1EE6/*Ủ*/:case 0x168/*Ũ*/:case 0x1EE4/*Ụ*/:
					case 0x1AF/*Ư*/:case 0x1EEA/*Ừ*/:case 0x1EE8/*Ứ*/:case 0x1EEC/*Ử*/:case 0x1EEE/*Ữ*/:case 0x1EF0/*Ự*/:
					case 0x59/*Y*/:case 0x1EF2/*Ỳ*/:case 0xDD/*Ý*/:case 0x1EF6/*Ỷ*/:case 0x1EF8/*Ỹ*/:case 0x1EF4/*Ỵ*/:
					case 0x61/*a*/:case 0xE0/*à*/:case 0xE1/*á*/:case 0x1EA3/*ả*/:case 0xE3/*ã*/:case 0x1EA1/*ạ*/:
					case 0x103/*ă*/:case 0x1EB1/*ằ*/:case 0x1EAF/*ắ*/:case 0x1EB3/*ẳ*/:case 0x1EB5/*ẵ*/:case 0x1EB7/*ặ*/:
					case 0xE2/*â*/:case 0x1EA7/*ầ*/:case 0x1EA5/*ấ*/:case 0x1EA9/*ẩ*/:case 0x1EAB/*ẫ*/:case 0x1EAD/*ậ*/:
					case 0x65/*e*/:case 0xE8/*è*/:case 0xE9/*é*/:case 0x1EBB/*ẻ*/:case 0x1EBD/*ẽ*/:case 0x1EB9/*ẹ*/:
					case 0xEA/*ê*/:case 0x1EC1/*ề*/:case 0x1EBF/*ế*/:case 0x1EC3/*ể*/:case 0x1EC5/*ễ*/:case 0x1EC7/*ệ*/:
					case 0x69/*i*/:case 0xEC/*ì*/:case 0xED/*í*/:case 0x1EC9/*ỉ*/:case 0x129/*ĩ*/:case 0x1ECB/*ị*/:
					case 0x6F/*o*/:case 0xF2/*ò*/:case 0xF3/*ó*/:case 0x1ECF/*ỏ*/:case 0xF5/*õ*/:case 0x1ECD/*ọ*/:
					case 0xF4/*ô*/:case 0x1ED3/*ồ*/:case 0x1ED1/*ố*/:case 0x1ED5/*ổ*/:case 0x1ED7/*ỗ*/:case 0x1ED9/*ộ*/:
					case 0x1A1/*ơ*/:case 0x1EDD/*ờ*/:case 0x1EDB/*ớ*/:case 0x1EDF/*ở*/:case 0x1EE1/*ỡ*/:case 0x1EE3/*ợ*/:
					case 0x75/*u*/:case 0xF9/*ù*/:case 0xFA/*ú*/:case 0x1EE7/*ủ*/:case 0x169/*ũ*/:case 0x1EE5/*ụ*/:
					case 0x1B0/*ư*/:case 0x1EEB/*ừ*/:case 0x1EE9/*ứ*/:case 0x1EED/*ử*/:case 0x1EEF/*ữ*/:case 0x1EF1/*ự*/:
					case 0x79/*y*/:case 0x1EF3/*ỳ*/:case 0xFD/*ý*/:case 0x1EF7/*ỷ*/:case 0x1EF9/*ỹ*/:case 0x1EF5/*ỵ*/:
					case 0x300/*VIETNAMESE_TONE_HUYEN*/:
					case 0x340/*VIETNAMESE_TONE_HUYEN*/:
					case 0x2CB/*VIETNAMESE_TONE_HUYEN*/:


					case 0x303/*VIETNAMESE_TONE_NGA*/:
					case 0x309/*VIETNAMESE_TONE_HOI*/:


					case 0x301/*VIETNAMESE_TONE_SAC*/:
					case 0xB4/*VIETNAMESE_TONE_SAC*/:
					case 0x341/*VIETNAMESE_TONE_SAC*/:
					case 0x2CA/*VIETNAMESE_TONE_SAC*/:
					case 0x1FFD/*VIETNAMESE_TONE_SAC*/:



					case 0x323/*VIETNAMESE_TONE_NANG*/:
					case 0x200B/*Zero width space*/:
					case 0xFEFF/*Zero width no-break space*/:
					case 0x302/*Combining Circumflex Accent*/:
					case 0x311/*Combining Inverted Breve*/:
					case 0x306/*Combining Breve*/:
					case 0x30C/*Combining Caron*/:
					case 0x31B/*Combining Horn*/:
						break;
					default:
						needSplitLeftMatchingEnglishWord = true;
						break;
					}
				}
				if (startOfNewNode && needSplitLeftMatchingVietnameseSyllable == false && needSplitLeftMatchingEnglishWord == false)
				{
					qwchar currentCharacter = *text;
					if ((currentCharacter & 0xFF00) == 0xFF00 && (currentCharacter & 0x00FF) <= 0x60)
					{
						currentCharacter = (qwchar)((currentCharacter & 0x00FF) + 0x20);
					}
					switch (currentCharacter)
					{
					case 0x30/*0*/:case 0x31/*1*/:case 0x32/*2*/:case 0x33/*3*/:case 0x34/*4*/:
					case 0x35/*5*/:case 0x36/*6*/:case 0x37/*7*/:case 0x38/*8*/:case 0x39/*9*/:
					case 0x111/*đ*/:case 0xf0/*đ*/:
					case 0x110/*Đ*/:case 0xD0/*Đ*/:
					case 0x62/*b*/:case 0x64/*d*/:case 0x6C/*l*/:case 0x71/*q*/:case 0x76/*v*/:
					case 0x42/*B*/:case 0x44/*D*/:case 0x4C/*L*/:case 0x51/*Q*/:case 0x56/*V*/:
					case 0x72/*r*/:case 0x73/*s*/:case 0x78/*x*/:
					case 0x52/*R*/:case 0x53/*S*/:case 0x58/*X*/:
					case 0x43/*C*/:case 0x4B/*K*/:case 0x4D/*M*/:case 0x4E/*N*/:case 0x50/*P*/:case 0x54/*T*/:case 0x57/*W*/:
					case 0x63/*c*/:case 0x6B/*k*/:case 0x6D/*m*/:case 0x6E/*n*/:case 0x70/*p*/:case 0x74/*t*/:case 0x77/*w*/:
					case 0x46/*F*/:case 0x4A/*J*/:case 0x5A/*Z*/:case 0x66/*f*/:case 0x6A/*j*/:
					case 0x7A/*z*/:
					case 0x47/*G*/:case 0x67/*g*/:
					case 0x48/*H*/:case 0x68/*h*/:
					case 0x41/*A*/:case 0xC0/*À*/:case 0xC1/*Á*/:case 0x1EA2/*Ả*/:case 0xC3/*Ã*/:case 0x1EA0/*Ạ*/:
					case 0x102/*Ă*/:case 0x1EB0/*Ằ*/:case 0x1EAE/*Ắ*/:case 0x1EB2/*Ẳ*/:case 0x1EB4/*Ẵ*/:case 0x1EB6/*Ặ*/:
					case 0xC2/*Â*/:case 0x1EA6/*Ầ*/:case 0x1EA4/*Ấ*/:case 0x1EA8/*Ẩ*/:case 0x1EAA/*Ẫ*/:case 0x1EAC/*Ậ*/:
					case 0x45/*E*/:case 0xC8/*È*/:case 0xC9/*É*/:case 0x1EBA/*Ẻ*/:case 0x1EBC/*Ẽ*/:case 0x1EB8/*Ẹ*/:
					case 0xCA/*Ê*/:case 0x1EC0/*Ề*/:case 0x1EBE/*Ế*/:case 0x1EC2/*Ể*/:case 0x1EC4/*Ễ*/:case 0x1EC6/*Ệ*/:
					case 0x49/*I*/:case 0xCC/*Ì*/:case 0xCD/*Í*/:case 0x1EC8/*Ỉ*/:case 0x128/*Ĩ*/:case 0x1ECA/*Ị*/:
					case 0x4F/*O*/:case 0xD2/*Ò*/:case 0xD3/*Ó*/:case 0x1ECE/*Ỏ*/:case 0xD5/*Õ*/:case 0x1ECC/*Ọ*/:
					case 0xD4/*Ô*/:case 0x1ED2/*Ồ*/:case 0x1ED0/*Ố*/:case 0x1ED4/*Ổ*/:case 0x1ED6/*Ỗ*/:case 0x1ED8/*Ộ*/:
					case 0x1A0/*Ơ*/:case 0x1EDC/*Ờ*/:case 0x1EDA/*Ớ*/:case 0x1EDE/*Ở*/:case 0x1EE0/*Ỡ*/:case 0x1EE2/*Ợ*/:
					case 0x55/*U*/:case 0xD9/*Ù*/:case 0xDA/*Ú*/:case 0x1EE6/*Ủ*/:case 0x168/*Ũ*/:case 0x1EE4/*Ụ*/:
					case 0x1AF/*Ư*/:case 0x1EEA/*Ừ*/:case 0x1EE8/*Ứ*/:case 0x1EEC/*Ử*/:case 0x1EEE/*Ữ*/:case 0x1EF0/*Ự*/:
					case 0x59/*Y*/:case 0x1EF2/*Ỳ*/:case 0xDD/*Ý*/:case 0x1EF6/*Ỷ*/:case 0x1EF8/*Ỹ*/:case 0x1EF4/*Ỵ*/:
					case 0x61/*a*/:case 0xE0/*à*/:case 0xE1/*á*/:case 0x1EA3/*ả*/:case 0xE3/*ã*/:case 0x1EA1/*ạ*/:
					case 0x103/*ă*/:case 0x1EB1/*ằ*/:case 0x1EAF/*ắ*/:case 0x1EB3/*ẳ*/:case 0x1EB5/*ẵ*/:case 0x1EB7/*ặ*/:
					case 0xE2/*â*/:case 0x1EA7/*ầ*/:case 0x1EA5/*ấ*/:case 0x1EA9/*ẩ*/:case 0x1EAB/*ẫ*/:case 0x1EAD/*ậ*/:
					case 0x65/*e*/:case 0xE8/*è*/:case 0xE9/*é*/:case 0x1EBB/*ẻ*/:case 0x1EBD/*ẽ*/:case 0x1EB9/*ẹ*/:
					case 0xEA/*ê*/:case 0x1EC1/*ề*/:case 0x1EBF/*ế*/:case 0x1EC3/*ể*/:case 0x1EC5/*ễ*/:case 0x1EC7/*ệ*/:
					case 0x69/*i*/:case 0xEC/*ì*/:case 0xED/*í*/:case 0x1EC9/*ỉ*/:case 0x129/*ĩ*/:case 0x1ECB/*ị*/:
					case 0x6F/*o*/:case 0xF2/*ò*/:case 0xF3/*ó*/:case 0x1ECF/*ỏ*/:case 0xF5/*õ*/:case 0x1ECD/*ọ*/:
					case 0xF4/*ô*/:case 0x1ED3/*ồ*/:case 0x1ED1/*ố*/:case 0x1ED5/*ổ*/:case 0x1ED7/*ỗ*/:case 0x1ED9/*ộ*/:
					case 0x1A1/*ơ*/:case 0x1EDD/*ờ*/:case 0x1EDB/*ớ*/:case 0x1EDF/*ở*/:case 0x1EE1/*ỡ*/:case 0x1EE3/*ợ*/:
					case 0x75/*u*/:case 0xF9/*ù*/:case 0xFA/*ú*/:case 0x1EE7/*ủ*/:case 0x169/*ũ*/:case 0x1EE5/*ụ*/:
					case 0x1B0/*ư*/:case 0x1EEB/*ừ*/:case 0x1EE9/*ứ*/:case 0x1EED/*ử*/:case 0x1EEF/*ữ*/:case 0x1EF1/*ự*/:
					case 0x79/*y*/:case 0x1EF3/*ỳ*/:case 0xFD/*ý*/:case 0x1EF7/*ỷ*/:case 0x1EF9/*ỹ*/:case 0x1EF5/*ỵ*/:
					case 0x20/* */:
					case 0x9/*Tab*/:
						break;
					case 0x200B/*Zero width space*/:
					case 0xFEFF/*Zero width no-break space*/:
					case 0xA0/*Non-breaking space*/:
						needIgnoreLeftCharacter = true;
						break;
					case 0xD/*\r Carriage Return*/:
						if (text[1] == 0xA/*\n Line Feed*/) needIgnoreLeftCharacter = true;
						break;
					case 0x0/*NULL*/:
						originalTextLength = int(text - originalText);
						flagNeedMoreLoop = 0;
						break;
					default:
						needSplitLeftCharacter = true;
						break;
					}
				}

				if (needSplitLeftMatchingVietnameseSyllable == false
					&& leftMatchingVietnameseLength > 0
					&& leftMatchingVietnameseIdentifier > 0
					&& vnsyllables[leftMatchingVietnameseIdentifier].vietnamese
					&& vnsyllables[leftMatchingVietnameseIdentifier].significant
					&& vnsyllables[leftMatchingVietnameseIdentifier].tone > '0'
					&& rightMathchingVietnameseIdentifier > 0
					&& vnsyllables[rightMathchingVietnameseIdentifier].vietnamese
					&& vnsyllables[rightMathchingVietnameseIdentifier].significant
					&& vnsyllables[rightMathchingVietnameseIdentifier].tone > '0'
					)
				{
					needSplitLeftMatchingVietnameseSyllable = true;
				}

				if (needSplitLeftMatchingVietnameseSyllable)
				{
					TEXT_NODE* backupTextNode = InsertVietnameseSyllableToTheTail(leftMatchingVietnameseIdentifier, currentOriginalSyllable, leftMatchingVietnameseLength, capital, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
					if (preloadTagVi)
					{
						backupTextNode->englishWordIdentifier = 0;
						backupTextNode->vietnameseLoanWordIndentifier = 0;
						if (vietnameseSyllableIdentifier)
						{
							backupTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
						}
					}
					if (preloadTagRetroflex)
					{
						backupTextNode->englishWordIdentifier = 0;
						backupTextNode->vietnameseLoanWordIndentifier = 0;
						if (vietnameseSyllableIdentifier)
						{
							backupTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
						}
					}
					leftTextNodeOffset4 = leftTextNodeOffset3;
					leftTextNodeOffset3 = leftTextNodeOffset2;
					leftTextNodeOffset2 = leftTextNodeOffset1;
					leftTextNodeOffset1 = leftTextNodeOffset0;
					leftTextNodeOffset0 = backupTextNode;
					text += leftMatchingVietnameseLength + needIgnoreSeparateCharacter;
					currentOriginalSyllable = text;
					currentOriginalSyllableLength = 0;
					if (preloadTagB) backupTextNode->prosodyB = 0x31/*1*/;
					if (preloadTagC) backupTextNode->prosodyC = 0x31/*1*/;
					if (preloadTagL) backupTextNode->prosodyL = 0x31/*1*/;
					if (preloadTagQ)
					{
						backupTextNode->prosodyQ = 0x31/*1*/;
						if (backupTextNode->back /*!= NULL*/ && backupTextNode->back->silenceTimeInSecond == 0.0) backupTextNode->back->prosodyQ = 0x32/*2*/;
					}
					if (flagNeedSpace) backupTextNode->needSpaceAfter = 1;
					if (preloadTagRetroflex) backupTextNode->retroflex = true;
					if (preloadViEnDup) backupTextNode->viendup = true;
				}
				else if (needSplitLeftMatchingEnglishWord)
				{
					TEXT_NODE* backupTextNode = InsertEnglishWordToTheTail(leftMatchingEnglishIdentifier, currentOriginalSyllable, leftMatchingEnglishLength, capital, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
					if (preloadTagEn) backupTextNode->vietnameseLoanWordIndentifier = 0;
					leftTextNodeOffset4 = leftTextNodeOffset3;
					leftTextNodeOffset3 = leftTextNodeOffset2;
					leftTextNodeOffset2 = leftTextNodeOffset1;
					leftTextNodeOffset1 = leftTextNodeOffset0;
					leftTextNodeOffset0 = backupTextNode;
					text += leftMatchingEnglishWordLength + needIgnoreSeparateCharacter;
					currentOriginalSyllable = text;
					currentOriginalSyllableLength = 0;
				}
				else if (needSplitLeftCharacter)
				{
					TEXT_NODE* backupTextNode = InsertUnknownNodeToTail(text, 1, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
					leftTextNodeOffset4 = leftTextNodeOffset3;
					leftTextNodeOffset3 = leftTextNodeOffset2;
					leftTextNodeOffset2 = leftTextNodeOffset1;
					leftTextNodeOffset1 = leftTextNodeOffset0;
					leftTextNodeOffset0 = backupTextNode;
					if (text[1] != 0x20/*space*/ && (text[0] == 0x201D/*”*/ || text[0] == 0x2019/*’*/)) backupTextNode->needSpaceAfter = 1;

					text++;
					currentOriginalSyllable = text;
					currentOriginalSyllableLength = 0;
				}
				else if (needIgnoreLeftCharacter)
				{
					text++;
					currentOriginalSyllable = text;
					currentOriginalSyllableLength = 0;
				}
				else
				{
					text += leftMatchingVietnameseLength;
					currentOriginalSyllableLength += leftMatchingVietnameseLength;
					char needInsertCurrentCharacter = 0;
					qwchar currentCharacter = *text;
					if ((currentCharacter & 0xFF00) == 0xFF00 && (currentCharacter & 0x00FF) <= 0x60) currentCharacter = (qwchar)((currentCharacter & 0x00FF) + 0x20);
					switch (currentCharacter)
					{
					case 0xA0/*Non-breaking space*/:
					case 0x20/*Space*/:
					case 0x9/*Tab*/:
						startOfNewNode = 1;
						currentIsUrl = 0;
						break;
					case 0xD/*\r Carriage Return*/:
						startOfNewNode = 1;
						currentIsUrl = 0;
						if (text[1] != 0xA/*\n Line Feed*/) needInsertCurrentCharacter = 1;
						break;
					case 0xA/*\n Line Feed*/:
						startOfNewNode = 1;
						currentIsUrl = 0;
						needInsertCurrentCharacter = 1;
						break;
					case 0x0/*NULL*/:
						originalTextLength = int(text - originalText);
						flagNeedMoreLoop = 0;
						startOfNewNode = 1;
						currentIsUrl = 0;
						break;
					case 0x30/*0*/:case 0x31/*1*/:case 0x32/*2*/:case 0x33/*3*/:case 0x34/*4*/:
					case 0x35/*5*/:case 0x36/*6*/:case 0x37/*7*/:case 0x38/*8*/:case 0x39/*9*/:
					case 0x111/*đ*/:case 0xf0/*đ*/:
					case 0x110/*Đ*/:case 0xD0/*Đ*/:
					case 0x62/*b*/:case 0x64/*d*/:case 0x6C/*l*/:case 0x71/*q*/:case 0x76/*v*/:
					case 0x42/*B*/:case 0x44/*D*/:case 0x4C/*L*/:case 0x51/*Q*/:case 0x56/*V*/:
					case 0x72/*r*/:case 0x73/*s*/:case 0x78/*x*/:
					case 0x52/*R*/:case 0x53/*S*/:case 0x58/*X*/:
					case 0x43/*C*/:case 0x4B/*K*/:case 0x4D/*M*/:case 0x4E/*N*/:case 0x50/*P*/:case 0x54/*T*/:case 0x57/*W*/:
					case 0x63/*c*/:case 0x6B/*k*/:case 0x6D/*m*/:case 0x6E/*n*/:case 0x70/*p*/:case 0x74/*t*/:case 0x77/*w*/:
					case 0x46/*F*/:case 0x4A/*J*/:case 0x5A/*Z*/:case 0x66/*f*/:case 0x6A/*j*/:
					case 0x7A/*z*/:
					case 0x47/*G*/:case 0x67/*g*/:
					case 0x48/*H*/:case 0x68/*h*/:
					case 0x41/*A*/:case 0xC0/*À*/:case 0xC1/*Á*/:case 0x1EA2/*Ả*/:case 0xC3/*Ã*/:case 0x1EA0/*Ạ*/:
					case 0x102/*Ă*/:case 0x1EB0/*Ằ*/:case 0x1EAE/*Ắ*/:case 0x1EB2/*Ẳ*/:case 0x1EB4/*Ẵ*/:case 0x1EB6/*Ặ*/:
					case 0xC2/*Â*/:case 0x1EA6/*Ầ*/:case 0x1EA4/*Ấ*/:case 0x1EA8/*Ẩ*/:case 0x1EAA/*Ẫ*/:case 0x1EAC/*Ậ*/:
					case 0x45/*E*/:case 0xC8/*È*/:case 0xC9/*É*/:case 0x1EBA/*Ẻ*/:case 0x1EBC/*Ẽ*/:case 0x1EB8/*Ẹ*/:
					case 0xCA/*Ê*/:case 0x1EC0/*Ề*/:case 0x1EBE/*Ế*/:case 0x1EC2/*Ể*/:case 0x1EC4/*Ễ*/:case 0x1EC6/*Ệ*/:
					case 0x49/*I*/:case 0xCC/*Ì*/:case 0xCD/*Í*/:case 0x1EC8/*Ỉ*/:case 0x128/*Ĩ*/:case 0x1ECA/*Ị*/:
					case 0x4F/*O*/:case 0xD2/*Ò*/:case 0xD3/*Ó*/:case 0x1ECE/*Ỏ*/:case 0xD5/*Õ*/:case 0x1ECC/*Ọ*/:
					case 0xD4/*Ô*/:case 0x1ED2/*Ồ*/:case 0x1ED0/*Ố*/:case 0x1ED4/*Ổ*/:case 0x1ED6/*Ỗ*/:case 0x1ED8/*Ộ*/:
					case 0x1A0/*Ơ*/:case 0x1EDC/*Ờ*/:case 0x1EDA/*Ớ*/:case 0x1EDE/*Ở*/:case 0x1EE0/*Ỡ*/:case 0x1EE2/*Ợ*/:
					case 0x55/*U*/:case 0xD9/*Ù*/:case 0xDA/*Ú*/:case 0x1EE6/*Ủ*/:case 0x168/*Ũ*/:case 0x1EE4/*Ụ*/:
					case 0x1AF/*Ư*/:case 0x1EEA/*Ừ*/:case 0x1EE8/*Ứ*/:case 0x1EEC/*Ử*/:case 0x1EEE/*Ữ*/:case 0x1EF0/*Ự*/:
					case 0x59/*Y*/:case 0x1EF2/*Ỳ*/:case 0xDD/*Ý*/:case 0x1EF6/*Ỷ*/:case 0x1EF8/*Ỹ*/:case 0x1EF4/*Ỵ*/:
					case 0x61/*a*/:case 0xE0/*à*/:case 0xE1/*á*/:case 0x1EA3/*ả*/:case 0xE3/*ã*/:case 0x1EA1/*ạ*/:
					case 0x103/*ă*/:case 0x1EB1/*ằ*/:case 0x1EAF/*ắ*/:case 0x1EB3/*ẳ*/:case 0x1EB5/*ẵ*/:case 0x1EB7/*ặ*/:
					case 0xE2/*â*/:case 0x1EA7/*ầ*/:case 0x1EA5/*ấ*/:case 0x1EA9/*ẩ*/:case 0x1EAB/*ẫ*/:case 0x1EAD/*ậ*/:
					case 0x65/*e*/:case 0xE8/*è*/:case 0xE9/*é*/:case 0x1EBB/*ẻ*/:case 0x1EBD/*ẽ*/:case 0x1EB9/*ẹ*/:
					case 0xEA/*ê*/:case 0x1EC1/*ề*/:case 0x1EBF/*ế*/:case 0x1EC3/*ể*/:case 0x1EC5/*ễ*/:case 0x1EC7/*ệ*/:
					case 0x69/*i*/:case 0xEC/*ì*/:case 0xED/*í*/:case 0x1EC9/*ỉ*/:case 0x129/*ĩ*/:case 0x1ECB/*ị*/:
					case 0x6F/*o*/:case 0xF2/*ò*/:case 0xF3/*ó*/:case 0x1ECF/*ỏ*/:case 0xF5/*õ*/:case 0x1ECD/*ọ*/:
					case 0xF4/*ô*/:case 0x1ED3/*ồ*/:case 0x1ED1/*ố*/:case 0x1ED5/*ổ*/:case 0x1ED7/*ỗ*/:case 0x1ED9/*ộ*/:
					case 0x1A1/*ơ*/:case 0x1EDD/*ờ*/:case 0x1EDB/*ớ*/:case 0x1EDF/*ở*/:case 0x1EE1/*ỡ*/:case 0x1EE3/*ợ*/:
					case 0x75/*u*/:case 0xF9/*ù*/:case 0xFA/*ú*/:case 0x1EE7/*ủ*/:case 0x169/*ũ*/:case 0x1EE5/*ụ*/:
					case 0x1B0/*ư*/:case 0x1EEB/*ừ*/:case 0x1EE9/*ứ*/:case 0x1EED/*ử*/:case 0x1EEF/*ữ*/:case 0x1EF1/*ự*/:
					case 0x79/*y*/:case 0x1EF3/*ỳ*/:case 0xFD/*ý*/:case 0x1EF7/*ỷ*/:case 0x1EF9/*ỹ*/:case 0x1EF5/*ỵ*/:
					case 0x2F/*/ slash*/:
					case 0x2D/*-*/:
					case  0x2013/*–*/:
					case 0x2C/*,*/:
					case 0x2E/*.*/:
					case 0x200B/*Zero width space*/:
					case 0xFEFF/*Zero width no-break space*/:
					case 0x300/*` VIETNAMESE_TONE_HUYEN*/:
					case 0x340/*VIETNAMESE_TONE_HUYEN*/:
					case 0x2CB/*VIETNAMESE_TONE_HUYEN*/:

					case 0x303/*~ VIETNAMESE_TONE_NGA*/:
					case 0x309/*? VIETNAMESE_TONE_HOI*/:

					case 0x301/*' VIETNAMESE_TONE_SAC*/:
					case 0xB4/*' VIETNAMESE_TONE_SAC*/:
					case 0x341/*VIETNAMESE_TONE_SAC*/:
					case 0x2CA/*VIETNAMESE_TONE_SAC*/:
					case 0x1FFD/*VIETNAMESE_TONE_SAC*/:


					case 0x323/*. VIETNAMESE_TONE_NANG*/:
					case 0x302/* ̂ */:
					case 0x306/* ̆ */:
					case 0x30C/* ̌ */:
					case 0x311/* ̑ */:
					case 0x31B/* ̛ */:
					case 0x3A1/*P*/:
					case 0x3C1/*p*/:



						text++;
						currentOriginalSyllableLength++;
						startOfNewNode = 0;
						break;
					default:
						if (currentIsUrl == 1 || UniformResourceIdentifierScheme(currentOriginalSyllable))
						{
							text++;
							currentOriginalSyllableLength++;
							startOfNewNode = 0;
							currentIsUrl = 1;
						}
						else
						{
							startOfNewNode = 1;
							currentIsUrl = 0;
							needInsertCurrentCharacter = 1;
						}
						break;
					}
					if (startOfNewNode/* != 0*/)
					{
						if (currentOriginalSyllableLength/* != 0*/)
						{
							TEXT_NODE* backupTextNode = InsertUnknownNodeToTail(currentOriginalSyllable + preloadSize, currentOriginalSyllableLength, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
							leftTextNodeOffset4 = leftTextNodeOffset3;
							leftTextNodeOffset3 = leftTextNodeOffset2;
							leftTextNodeOffset2 = leftTextNodeOffset1;
							leftTextNodeOffset1 = leftTextNodeOffset0;
							leftTextNodeOffset0 = backupTextNode;
						}

						if (needInsertCurrentCharacter)
						{
							TEXT_NODE* backupTextNode = InsertUnknownNodeToTail(text, 1, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
							leftTextNodeOffset4 = leftTextNodeOffset3;
							leftTextNodeOffset3 = leftTextNodeOffset2;
							leftTextNodeOffset2 = leftTextNodeOffset1;
							leftTextNodeOffset1 = leftTextNodeOffset0;
							leftTextNodeOffset0 = backupTextNode;
						}

						/* Reset variable */
						text++;
						currentOriginalSyllable = text;
						currentOriginalSyllableLength = 0;
					}//end of if start new node
				}//end of if không cần cắt tại vị trí hiện tại
			}//end of else significant syllable
		}//end of for -- flagNeedMoreLoop
		if (logFile/*!=NULL*/)
		{//Log for developer
			qtime step1EndTime = { 0 };
			qGetTime(&step1EndTime);
			double step1Time = qGetDiffTime(step1StartTime, step1EndTime);
			logTime += step1Time;
			double processLeft = countTotalUnknownNode * 100.00 / countTotalNode;
			Log("\n\n\n\\****************************************Start****************************************/\n");
			Log("Process input text\n");
			Log("Labeling:\n\tStep 1 : Divide to %d text node by Space, New line, Null (Process %.2lf%%, exist %.2lf%%, time %lfs, %d unknown): \n", countTotalNode, 100.0 - processLeft, processLeft, step1Time, countTotalUnknownNode);
			for (TEXT_NODE* textNode = head; textNode/*!=NULL*/; textNode = textNode->next)
			{
				if (textNode->vietnameseSyllableIdentifier
					|| textNode->englishWordIdentifier
					|| textNode->vietnameseAbbreviationIndentifier
					|| textNode->vietnameseLoanWordIndentifier
					|| textNode->textNodeType == TEXT_NODE_TYPE_SILENCE
					|| textNode->textNodeType == TEXT_NODE_TYPE_URL)
				{

					Log("\t\t+\"");
					Log(textNode->text, textNode->textLength);
					Log("\" - %d character", textNode->textLength);
					if (textNode->vietnameseSyllableIdentifier)
					{
						Log(", vnsyllable id %d", textNode->vietnameseSyllableIdentifier);
						if (textNode->leftVietnameseSyllableSure || textNode->rightVietnameseSyllableSure) Log(" (ssure l%d-r%d)", textNode->leftVietnameseSyllableSure, textNode->rightVietnameseSyllableSure);
					}
					if (textNode->vietnameseAbbreviationIndentifier)
					{
						Log(", abbreviation id %d", textNode->vietnameseAbbreviationIndentifier);
						if (textNode->leftVietnameseAbbreviationSure || textNode->rightVietnameseAbbreviationSure) Log(" (asure l%d-r%d)", textNode->leftVietnameseAbbreviationSure, textNode->rightVietnameseAbbreviationSure);
					}
					if (textNode->vietnameseLoanWordIndentifier) Log(", loan id %d ", textNode->vietnameseLoanWordIndentifier);
					if (textNode->vietnameseWordIdentifier)
					{
						Log(", left word length %d - id %d ", vnwords[textNode->vietnameseWordIdentifier].length, textNode->vietnameseWordIdentifier);
						if (textNode->leftVietnameseWordSure || textNode->rightVietnameseWordSure) Log(" (wsure l%d-r%d)", textNode->leftVietnameseWordSure, textNode->rightVietnameseWordSure);
					}
					if (textNode->englishWordIdentifier) Log(", enword id %d", textNode->englishWordIdentifier);
					if (textNode->textNodeType == TEXT_NODE_TYPE_SILENCE) Log(", Silence Time %lfs ", textNode->silenceTimeInSecond);
					if (textNode->splittable == TEXT_NODE_CAN_NOT_SPLIT) Log(", j+");

					if (textNode->changeable) Log("	changeable=%d", textNode->changeable);

					Log("\n");
					textNode->step = 1;
				}
				else
				{

					Log("\t\t+[");
					Log(textNode->text, textNode->textLength);
					Log("] - %d character", textNode->textLength);
					if (textNode->textNodeType == TEXT_NODE_TYPE_URL) Log(" URL ", textNode->textNodeType);
					if (uhead == textNode) Log("(*****uhead*****)");
					else if (utail == textNode) Log("(*****utail*****)");
					Log("\n");

				}
			}
		}
	}
}
/************************************************************************/
/* Step 2 : Normalization                                               */
/************************************************************************/
void				VietnameseTextNormalizer::Normalize(void)
{
	/************************************************************************/
	/*                                                                      */
	/* Bước 2 : Chuyển các encode đặc biệt về các encode mặc định           */
	/*                                                                      */
	/************************************************************************/
	Log("\tStep 2 : Normalization %d unknown node ", countTotalUnknownNode);
	qtime			step2StartTime;
	qGetTime(&step2StartTime);
	const int		limitCharacterLength = 250;
	/************************************************************************/
	/* Normalization loop                                                   */
	/************************************************************************/
	for (TEXT_NODE* textNode = uhead; textNode/*!=NULL*/; )
	{
		//Normalization
		if ((textNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN || textNode->textNodeType == TEXT_NODE_TYPE_URL)
			&& textNode->vietnameseSyllableIdentifier == 0
			&& textNode->vietnameseAbbreviationIndentifier == 0
			&& textNode->vietnameseLoanWordIndentifier == 0
			&& textNode->englishWordIdentifier == 0
			&& textNode->silenceTimeInSecond == 0.0
			&& textNode->originalTextLength < limitCharacterLength)
		{//nếu cần chuẩn hóa ở bước này
			qwchar* textMemory = (qwchar*)qcalloc(textNode->originalTextLength + 10/*for safe*/, sizeof(qwchar));
			qwchar* textLowerMemory = (qwchar*)qcalloc(textNode->originalTextLength + 10/*for safe*/, sizeof(qwchar));
			qwchar* textTypeMemory = (qwchar*)qcalloc(textNode->originalTextLength + 10/*for safe*/, sizeof(qwchar));
			qwchar* textIndexMemory = (qwchar*)qcalloc(textNode->originalTextLength + 10/*for safe*/, sizeof(qwchar));



			qwchar* text = textMemory;
			qwchar* textLower = textLowerMemory;
			qwchar* textType = textTypeMemory;
			qwchar* textIndex = textIndexMemory;
			if (textMemory /*!= NULL*/ && textLowerMemory /*!= NULL*/ && textTypeMemory /*!= NULL*/ && textIndex /*!= NULL*/)
			{
				/************************************************************************/
				/* Text Properties                                                      */
				/************************************************************************/
				const qwchar* backupOriginalText = textNode->originalText;
				int								textLength = 0;
				int								textErrorCombiningTone = 0;
				int								textCountTotalChange = 0;
				/*Alphabet character*/
				int								textCountTotalAlphabet = 0;

				/*Upper - Lower Character*/
				int								textCountLowerCharacter = 0;
				int								textCountUpperCharacter = 0;

				/*Vowel - Consonant Character*/
				int								textCountVowelCharacter = 0;
				int								textCountConsonantCharacter = 0;

				/*Vietnamese*/
				int								textCountVietnameseOnlyCharacter = 0;

				/*English*/
				int								textCountEnglishVowelCharacter = 0;
				int								textCountEnglishConsonantCharacter = 0;

				/*Number*/
				int								textCountNumberCharacter = 0;
				int								textCountZeroCharacter = 0;

				/*Other*/
				int								textCountOtherOnKeyBoard = 0;
				int								textCountDotCharacter = 0;
				int								textCountCommaCharacter = 0;
				int								textCountSlashCharacter = 0;

				/*Unknown Character*/
				int								textCountUnknownCharacter = 0;

				/************************************************************************/
				/* Bước 1 : Duyệt sơ bộ từ đầu đến cuối                                 */
				/************************************************************************/
				for (int iChar = 0, iCharMaxLength = textNode->originalTextLength; iChar < iCharMaxLength; iChar++, backupOriginalText++)
				{
					qwchar currentCharacter = *backupOriginalText;
					if ((currentCharacter & 0xFF00) == 0xFF00 && (currentCharacter & 0x00FF) <= 0x60)
					{
						currentCharacter = (qwchar)((currentCharacter & 0x00FF) + 0x20);
						textCountTotalChange++;
					}
					switch (currentCharacter)
					{
						/************************************************************************/
						/* Nhóm các chữ cái chỉ có trong tiếng việt, viết thường                */
						/************************************************************************/
					case 0xE0/*af*/:text[textLength] = (qwchar)0xE0/*af*/; textLower[textLength] = (qwchar)0xE0/*af*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xE1/*as*/:text[textLength] = (qwchar)0xE1/*as*/; textLower[textLength] = (qwchar)0xE1/*as*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA3/*ar*/:text[textLength] = (qwchar)0x1EA3/*ar*/; textLower[textLength] = (qwchar)0x1EA3/*ar*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xE3/*ax*/:text[textLength] = (qwchar)0xE3/*ax*/; textLower[textLength] = (qwchar)0xE3/*ax*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA1/*aj*/:text[textLength] = (qwchar)0x1EA1/*aj*/; textLower[textLength] = (qwchar)0x1EA1/*aj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x103/*aw*/:text[textLength] = (qwchar)0x103/*aw*/; textLower[textLength] = (qwchar)0x103/*aw*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB1/*awf*/:text[textLength] = (qwchar)0x1EB1/*awf*/; textLower[textLength] = (qwchar)0x1EB1/*awf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAF/*aws*/:text[textLength] = (qwchar)0x1EAF/*aws*/; textLower[textLength] = (qwchar)0x1EAF/*aws*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB3/*awr*/:text[textLength] = (qwchar)0x1EB3/*awr*/; textLower[textLength] = (qwchar)0x1EB3/*awr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB5/*awx*/:text[textLength] = (qwchar)0x1EB5/*awx*/; textLower[textLength] = (qwchar)0x1EB5/*awx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB7/*awj*/:text[textLength] = (qwchar)0x1EB7/*awj*/; textLower[textLength] = (qwchar)0x1EB7/*awj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xE2/*aa*/:text[textLength] = (qwchar)0xE2/*aa*/; textLower[textLength] = (qwchar)0xE2/*aa*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA7/*aaf*/:text[textLength] = (qwchar)0x1EA7/*aaf*/; textLower[textLength] = (qwchar)0x1EA7/*aaf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA5/*aas*/:text[textLength] = (qwchar)0x1EA5/*aas*/; textLower[textLength] = (qwchar)0x1EA5/*aas*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA9/*aar*/:text[textLength] = (qwchar)0x1EA9/*aar*/; textLower[textLength] = (qwchar)0x1EA9/*aar*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAB/*aax*/:text[textLength] = (qwchar)0x1EAB/*aax*/; textLower[textLength] = (qwchar)0x1EAB/*aax*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAD/*aaj*/:text[textLength] = (qwchar)0x1EAD/*aaj*/; textLower[textLength] = (qwchar)0x1EAD/*aaj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xE8/*ef*/:text[textLength] = (qwchar)0xE8/*ef*/; textLower[textLength] = (qwchar)0xE8/*ef*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xE9/*es*/:text[textLength] = (qwchar)0xE9/*es*/; textLower[textLength] = (qwchar)0xE9/*es*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBB/*er*/:text[textLength] = (qwchar)0x1EBB/*er*/; textLower[textLength] = (qwchar)0x1EBB/*er*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBD/*ex*/:text[textLength] = (qwchar)0x1EBD/*ex*/; textLower[textLength] = (qwchar)0x1EBD/*ex*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB9/*ej*/:text[textLength] = (qwchar)0x1EB9/*ej*/; textLower[textLength] = (qwchar)0x1EB9/*ej*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xEA/*ee*/:text[textLength] = (qwchar)0xEA/*ee*/; textLower[textLength] = (qwchar)0xEA/*ee*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC1/*eef*/:text[textLength] = (qwchar)0x1EC1/*eef*/; textLower[textLength] = (qwchar)0x1EC1/*eef*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBF/*ees*/:text[textLength] = (qwchar)0x1EBF/*ees*/; textLower[textLength] = (qwchar)0x1EBF/*ees*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC3/*eer*/:text[textLength] = (qwchar)0x1EC3/*eer*/; textLower[textLength] = (qwchar)0x1EC3/*eer*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC5/*eex*/:text[textLength] = (qwchar)0x1EC5/*eex*/; textLower[textLength] = (qwchar)0x1EC5/*eex*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC7/*eej*/:text[textLength] = (qwchar)0x1EC7/*eej*/; textLower[textLength] = (qwchar)0x1EC7/*eej*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xEC/*if*/:text[textLength] = (qwchar)0xEC/*if*/; textLower[textLength] = (qwchar)0xEC/*if*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xED/*is*/:text[textLength] = (qwchar)0xED/*is*/; textLower[textLength] = (qwchar)0xED/*is*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC9/*ir*/:text[textLength] = (qwchar)0x1EC9/*ir*/; textLower[textLength] = (qwchar)0x1EC9/*ir*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x129/*ix*/:text[textLength] = (qwchar)0x129/*ix*/; textLower[textLength] = (qwchar)0x129/*ix*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECB/*ij*/:text[textLength] = (qwchar)0x1ECB/*ij*/; textLower[textLength] = (qwchar)0x1ECB/*ij*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF2/*of*/:text[textLength] = (qwchar)0xF2/*of*/; textLower[textLength] = (qwchar)0xF2/*of*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF3/*os*/:text[textLength] = (qwchar)0xF3/*os*/; textLower[textLength] = (qwchar)0xF3/*os*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECF/*or*/:text[textLength] = (qwchar)0x1ECF/*or*/; textLower[textLength] = (qwchar)0x1ECF/*or*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF5/*ox*/:text[textLength] = (qwchar)0xF5/*ox*/; textLower[textLength] = (qwchar)0xF5/*ox*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECD/*oj*/:text[textLength] = (qwchar)0x1ECD/*oj*/; textLower[textLength] = (qwchar)0x1ECD/*oj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF4/*oo*/:text[textLength] = (qwchar)0xF4/*oo*/; textLower[textLength] = (qwchar)0xF4/*oo*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED3/*oof*/:text[textLength] = (qwchar)0x1ED3/*oof*/; textLower[textLength] = (qwchar)0x1ED3/*oof*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED1/*oos*/:text[textLength] = (qwchar)0x1ED1/*oos*/; textLower[textLength] = (qwchar)0x1ED1/*oos*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED5/*oor*/:text[textLength] = (qwchar)0x1ED5/*oor*/; textLower[textLength] = (qwchar)0x1ED5/*oor*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED7/*oox*/:text[textLength] = (qwchar)0x1ED7/*oox*/; textLower[textLength] = (qwchar)0x1ED7/*oox*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED9/*ooj*/:text[textLength] = (qwchar)0x1ED9/*ooj*/; textLower[textLength] = (qwchar)0x1ED9/*ooj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1A1/*ow*/:text[textLength] = (qwchar)0x1A1/*ow*/; textLower[textLength] = (qwchar)0x1A1/*ow*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDD/*owf*/:text[textLength] = (qwchar)0x1EDD/*owf*/; textLower[textLength] = (qwchar)0x1EDD/*owf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDB/*ows*/:text[textLength] = (qwchar)0x1EDB/*ows*/; textLower[textLength] = (qwchar)0x1EDB/*ows*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDF/*owr*/:text[textLength] = (qwchar)0x1EDF/*owr*/; textLower[textLength] = (qwchar)0x1EDF/*owr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE1/*owx*/:text[textLength] = (qwchar)0x1EE1/*owx*/; textLower[textLength] = (qwchar)0x1EE1/*owx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE3/*owj*/:text[textLength] = (qwchar)0x1EE3/*owj*/; textLower[textLength] = (qwchar)0x1EE3/*owj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF9/*uf*/:text[textLength] = (qwchar)0xF9/*uf*/; textLower[textLength] = (qwchar)0xF9/*uf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xFA/*us*/:text[textLength] = (qwchar)0xFA/*us*/; textLower[textLength] = (qwchar)0xFA/*us*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE7/*ur*/:text[textLength] = (qwchar)0x1EE7/*ur*/; textLower[textLength] = (qwchar)0x1EE7/*ur*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x169/*ux*/:text[textLength] = (qwchar)0x169/*ux*/; textLower[textLength] = (qwchar)0x169/*ux*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE5/*uj*/:text[textLength] = (qwchar)0x1EE5/*uj*/; textLower[textLength] = (qwchar)0x1EE5/*uj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1B0/*uw*/:text[textLength] = (qwchar)0x1B0/*uw*/; textLower[textLength] = (qwchar)0x1B0/*uw*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EEB/*uwf*/:text[textLength] = (qwchar)0x1EEB/*uwf*/; textLower[textLength] = (qwchar)0x1EEB/*uwf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE9/*uws*/:text[textLength] = (qwchar)0x1EE9/*uws*/; textLower[textLength] = (qwchar)0x1EE9/*uws*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EED/*uwr*/:text[textLength] = (qwchar)0x1EED/*uwr*/; textLower[textLength] = (qwchar)0x1EED/*uwr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EEF/*uwx*/:text[textLength] = (qwchar)0x1EEF/*uwx*/; textLower[textLength] = (qwchar)0x1EEF/*uwx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF1/*uwj*/:text[textLength] = (qwchar)0x1EF1/*uwj*/; textLower[textLength] = (qwchar)0x1EF1/*uwj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF3/*yf*/:text[textLength] = (qwchar)0x1EF3/*yf*/; textLower[textLength] = (qwchar)0x1EF3/*yf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xFD/*ys*/:text[textLength] = (qwchar)0xFD/*ys*/; textLower[textLength] = (qwchar)0xFD/*ys*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF7/*yr*/:text[textLength] = (qwchar)0x1EF7/*yr*/; textLower[textLength] = (qwchar)0x1EF7/*yr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF9/*yx*/:text[textLength] = (qwchar)0x1EF9/*yx*/; textLower[textLength] = (qwchar)0x1EF9/*yx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF5/*yj*/:text[textLength] = (qwchar)0x1EF5/*yj*/; textLower[textLength] = (qwchar)0x1EF5/*yj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xF0/*ð*/:textCountTotalChange++; case 0x111/*dd*/:text[textLength] = (qwchar)0x111/*dd*/; textLower[textLength] = (qwchar)0x111/*dd*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
						/************************************************************************/
						/* Nhóm các chữ cái có cả trong tiếng việt lẫn tiếng anh viết thường    */
						/************************************************************************/
					case 0x61/*a*/:text[textLength] = (qwchar)0x61/*a*/; textLower[textLength] = (qwchar)0x61/*a*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x62/*b*/:text[textLength] = (qwchar)0x62/*b*/; textLower[textLength] = (qwchar)0x62/*b*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x63/*c*/:text[textLength] = (qwchar)0x63/*c*/; textLower[textLength] = (qwchar)0x63/*c*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x64/*d*/:text[textLength] = (qwchar)0x64/*d*/; textLower[textLength] = (qwchar)0x64/*d*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x65/*e*/:text[textLength] = (qwchar)0x65/*e*/; textLower[textLength] = (qwchar)0x65/*e*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x66/*f*/:text[textLength] = (qwchar)0x66/*f*/; textLower[textLength] = (qwchar)0x66/*f*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x67/*g*/:text[textLength] = (qwchar)0x67/*g*/; textLower[textLength] = (qwchar)0x67/*g*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x68/*h*/:text[textLength] = (qwchar)0x68/*h*/; textLower[textLength] = (qwchar)0x68/*h*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x69/*i*/:text[textLength] = (qwchar)0x69/*i*/; textLower[textLength] = (qwchar)0x69/*i*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x6A/*j*/:text[textLength] = (qwchar)0x6A/*j*/; textLower[textLength] = (qwchar)0x6A/*j*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x6B/*k*/:text[textLength] = (qwchar)0x6B/*k*/; textLower[textLength] = (qwchar)0x6B/*k*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x6C/*l*/:text[textLength] = (qwchar)0x6C/*l*/; textLower[textLength] = (qwchar)0x6C/*l*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x6D/*m*/:text[textLength] = (qwchar)0x6D/*m*/; textLower[textLength] = (qwchar)0x6D/*m*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x6E/*n*/:text[textLength] = (qwchar)0x6E/*n*/; textLower[textLength] = (qwchar)0x6E/*n*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x6F/*o*/:text[textLength] = (qwchar)0x6F/*o*/; textLower[textLength] = (qwchar)0x6F/*o*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x3C1/*ρ*/:textCountTotalChange++; case 0x70/*p*/:text[textLength] = (qwchar)0x70/*p*/; textLower[textLength] = (qwchar)0x70/*p*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x71/*q*/:text[textLength] = (qwchar)0x71/*q*/; textLower[textLength] = (qwchar)0x71/*q*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x72/*r*/:text[textLength] = (qwchar)0x72/*r*/; textLower[textLength] = (qwchar)0x72/*r*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x73/*s*/:text[textLength] = (qwchar)0x73/*s*/; textLower[textLength] = (qwchar)0x73/*s*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x74/*t*/:text[textLength] = (qwchar)0x74/*t*/; textLower[textLength] = (qwchar)0x74/*t*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x75/*u*/:text[textLength] = (qwchar)0x75/*u*/; textLower[textLength] = (qwchar)0x75/*u*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x76/*v*/:text[textLength] = (qwchar)0x76/*v*/; textLower[textLength] = (qwchar)0x76/*v*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x77/*w*/:text[textLength] = (qwchar)0x77/*w*/; textLower[textLength] = (qwchar)0x77/*w*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x78/*x*/:text[textLength] = (qwchar)0x78/*x*/; textLower[textLength] = (qwchar)0x78/*x*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
					case 0x79/*y*/:text[textLength] = (qwchar)0x79/*y*/; textLower[textLength] = (qwchar)0x79/*y*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; textCountEnglishVowelCharacter++; break;
					case 0x7A/*z*/:text[textLength] = (qwchar)0x7A/*z*/; textLower[textLength] = (qwchar)0x7A/*z*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_LOWER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountLowerCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; textCountEnglishConsonantCharacter++; break;
						/************************************************************************/
						/* Nhóm các chữ cái chỉ có trong tiếng việt, viết hoa                   */
						/************************************************************************/
					case 0xC0/*AF*/:text[textLength] = (qwchar)0xC0/*AF*/; textLower[textLength] = (qwchar)0xE0/*af*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xC1/*AS*/:text[textLength] = (qwchar)0xC1/*AS*/; textLower[textLength] = (qwchar)0xE1/*as*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA2/*AR*/:text[textLength] = (qwchar)0x1EA2/*AR*/; textLower[textLength] = (qwchar)0x1EA3/*ar*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xC3/*AX*/:text[textLength] = (qwchar)0xC3/*AX*/; textLower[textLength] = (qwchar)0xE3/*ax*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA0/*AJ*/:text[textLength] = (qwchar)0x1EA0/*AJ*/; textLower[textLength] = (qwchar)0x1EA1/*aj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x102/*AW*/:text[textLength] = (qwchar)0x102/*AW*/; textLower[textLength] = (qwchar)0x103/*aw*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB0/*AWF*/:text[textLength] = (qwchar)0x1EB0/*AWF*/; textLower[textLength] = (qwchar)0x1EB1/*awf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAE/*AWS*/:text[textLength] = (qwchar)0x1EAE/*AWS*/; textLower[textLength] = (qwchar)0x1EAF/*aws*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB2/*AWR*/:text[textLength] = (qwchar)0x1EB2/*AWR*/; textLower[textLength] = (qwchar)0x1EB3/*awr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB4/*AWX*/:text[textLength] = (qwchar)0x1EB4/*AWX*/; textLower[textLength] = (qwchar)0x1EB5/*awx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB6/*AWJ*/:text[textLength] = (qwchar)0x1EB6/*AWJ*/; textLower[textLength] = (qwchar)0x1EB7/*awj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xC2/*AA*/:text[textLength] = (qwchar)0xC2/*AA*/; textLower[textLength] = (qwchar)0xE2/*aa*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA6/*AAF*/:text[textLength] = (qwchar)0x1EA6/*AAF*/; textLower[textLength] = (qwchar)0x1EA7/*aaf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA4/*AAS*/:text[textLength] = (qwchar)0x1EA4/*AAS*/; textLower[textLength] = (qwchar)0x1EA5/*aas*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EA8/*AAR*/:text[textLength] = (qwchar)0x1EA8/*AAR*/; textLower[textLength] = (qwchar)0x1EA9/*aar*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAA/*AAX*/:text[textLength] = (qwchar)0x1EAA/*AAX*/; textLower[textLength] = (qwchar)0x1EAB/*aax*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EAC/*AAJ*/:text[textLength] = (qwchar)0x1EAC/*AAJ*/; textLower[textLength] = (qwchar)0x1EAD/*aaj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xC8/*EF*/:text[textLength] = (qwchar)0xC8/*EF*/; textLower[textLength] = (qwchar)0xE8/*ef*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xC9/*ES*/:text[textLength] = (qwchar)0xC9/*ES*/; textLower[textLength] = (qwchar)0xE9/*es*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBA/*ER*/:text[textLength] = (qwchar)0x1EBA/*ER*/; textLower[textLength] = (qwchar)0x1EBB/*er*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBC/*EX*/:text[textLength] = (qwchar)0x1EBC/*EX*/; textLower[textLength] = (qwchar)0x1EBD/*ex*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EB8/*EJ*/:text[textLength] = (qwchar)0x1EB8/*EJ*/; textLower[textLength] = (qwchar)0x1EB9/*ej*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xCA/*EE*/:text[textLength] = (qwchar)0xCA/*EE*/; textLower[textLength] = (qwchar)0xEA/*ee*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC0/*EEF*/:text[textLength] = (qwchar)0x1EC0/*EEF*/; textLower[textLength] = (qwchar)0x1EC1/*eef*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EBE/*EES*/:text[textLength] = (qwchar)0x1EBE/*EES*/; textLower[textLength] = (qwchar)0x1EBF/*ees*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC2/*EER*/:text[textLength] = (qwchar)0x1EC2/*EER*/; textLower[textLength] = (qwchar)0x1EC3/*eer*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC4/*EEX*/:text[textLength] = (qwchar)0x1EC4/*EEX*/; textLower[textLength] = (qwchar)0x1EC5/*eex*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC6/*EEJ*/:text[textLength] = (qwchar)0x1EC6/*EEJ*/; textLower[textLength] = (qwchar)0x1EC7/*eej*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xCC/*IF*/:text[textLength] = (qwchar)0xCC/*IF*/; textLower[textLength] = (qwchar)0xEC/*if*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xCD/*IS*/:text[textLength] = (qwchar)0xCD/*IS*/; textLower[textLength] = (qwchar)0xED/*is*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EC8/*IR*/:text[textLength] = (qwchar)0x1EC8/*IR*/; textLower[textLength] = (qwchar)0x1EC9/*ir*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x128/*IX*/:text[textLength] = (qwchar)0x128/*IX*/; textLower[textLength] = (qwchar)0x129/*ix*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECA/*IJ*/:text[textLength] = (qwchar)0x1ECA/*IJ*/; textLower[textLength] = (qwchar)0x1ECB/*ij*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD2/*OF*/:text[textLength] = (qwchar)0xD2/*OF*/; textLower[textLength] = (qwchar)0xF2/*of*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD3/*OS*/:text[textLength] = (qwchar)0xD3/*OS*/; textLower[textLength] = (qwchar)0xF3/*os*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECE/*OR*/:text[textLength] = (qwchar)0x1ECE/*OR*/; textLower[textLength] = (qwchar)0x1ECF/*or*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD5/*OX*/:text[textLength] = (qwchar)0xD5/*OX*/; textLower[textLength] = (qwchar)0xF5/*ox*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ECC/*OJ*/:text[textLength] = (qwchar)0x1ECC/*OJ*/; textLower[textLength] = (qwchar)0x1ECD/*oj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD4/*OO*/:text[textLength] = (qwchar)0xD4/*OO*/; textLower[textLength] = (qwchar)0xF4/*oo*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED2/*OOF*/:text[textLength] = (qwchar)0x1ED2/*OOF*/; textLower[textLength] = (qwchar)0x1ED3/*oof*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED0/*OOS*/:text[textLength] = (qwchar)0x1ED0/*OOS*/; textLower[textLength] = (qwchar)0x1ED1/*oos*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED4/*OOR*/:text[textLength] = (qwchar)0x1ED4/*OOR*/; textLower[textLength] = (qwchar)0x1ED5/*oor*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED6/*OOX*/:text[textLength] = (qwchar)0x1ED6/*OOX*/; textLower[textLength] = (qwchar)0x1ED7/*oox*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1ED8/*OOJ*/:text[textLength] = (qwchar)0x1ED8/*OOJ*/; textLower[textLength] = (qwchar)0x1ED9/*ooj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1A0/*OW*/:text[textLength] = (qwchar)0x1A0/*OW*/; textLower[textLength] = (qwchar)0x1A1/*ow*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDC/*OWF*/:text[textLength] = (qwchar)0x1EDC/*OWF*/; textLower[textLength] = (qwchar)0x1EDD/*owf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDA/*OWS*/:text[textLength] = (qwchar)0x1EDA/*OWS*/; textLower[textLength] = (qwchar)0x1EDB/*ows*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EDE/*OWR*/:text[textLength] = (qwchar)0x1EDE/*OWR*/; textLower[textLength] = (qwchar)0x1EDF/*owr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE0/*OWX*/:text[textLength] = (qwchar)0x1EE0/*OWX*/; textLower[textLength] = (qwchar)0x1EE1/*owx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE2/*OWJ*/:text[textLength] = (qwchar)0x1EE2/*OWJ*/; textLower[textLength] = (qwchar)0x1EE3/*owj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD9/*UF*/:text[textLength] = (qwchar)0xD9/*UF*/; textLower[textLength] = (qwchar)0xF9/*uf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xDA/*US*/:text[textLength] = (qwchar)0xDA/*US*/; textLower[textLength] = (qwchar)0xFA/*us*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE6/*UR*/:text[textLength] = (qwchar)0x1EE6/*UR*/; textLower[textLength] = (qwchar)0x1EE7/*ur*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x168/*UX*/:text[textLength] = (qwchar)0x168/*UX*/; textLower[textLength] = (qwchar)0x169/*ux*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE4/*UJ*/:text[textLength] = (qwchar)0x1EE4/*UJ*/; textLower[textLength] = (qwchar)0x1EE5/*uj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1AF/*UW*/:text[textLength] = (qwchar)0x1AF/*UW*/; textLower[textLength] = (qwchar)0x1B0/*uw*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EEA/*UWF*/:text[textLength] = (qwchar)0x1EEA/*UWF*/; textLower[textLength] = (qwchar)0x1EEB/*uwf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EE8/*UWS*/:text[textLength] = (qwchar)0x1EE8/*UWS*/; textLower[textLength] = (qwchar)0x1EE9/*uws*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EEC/*UWR*/:text[textLength] = (qwchar)0x1EEC/*UWR*/; textLower[textLength] = (qwchar)0x1EED/*uwr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EEE/*UWX*/:text[textLength] = (qwchar)0x1EEE/*UWX*/; textLower[textLength] = (qwchar)0x1EEF/*uwx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF0/*UWJ*/:text[textLength] = (qwchar)0x1EF0/*UWJ*/; textLower[textLength] = (qwchar)0x1EF1/*uwj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF2/*YF*/:text[textLength] = (qwchar)0x1EF2/*YF*/; textLower[textLength] = (qwchar)0x1EF3/*yf*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xDD/*YS*/:text[textLength] = (qwchar)0xDD/*YS*/; textLower[textLength] = (qwchar)0xFD/*ys*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF6/*YR*/:text[textLength] = (qwchar)0x1EF6/*YR*/; textLower[textLength] = (qwchar)0x1EF7/*yr*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF8/*YX*/:text[textLength] = (qwchar)0x1EF8/*YX*/; textLower[textLength] = (qwchar)0x1EF9/*yx*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x1EF4/*YJ*/:text[textLength] = (qwchar)0x1EF4/*YJ*/; textLower[textLength] = (qwchar)0x1EF5/*yj*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0xD0/*Ð*/:textCountTotalChange++; case 0x110/*DD*/:text[textLength] = (qwchar)0x110/*DD*/; textLower[textLength] = (qwchar)0x111/*dd*/; textType[textLength] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountVietnameseOnlyCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
						/************************************************************************/
						/* Nhóm các chữ cái có cả trong tiếng việt lẫn tiếng anh viết hoa       */
						/************************************************************************/
					case 0x41/*A*/:text[textLength] = (qwchar)0x41/*A*/; textLower[textLength] = (qwchar)0x61/*a*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x42/*B*/:text[textLength] = (qwchar)0x42/*B*/; textLower[textLength] = (qwchar)0x62/*b*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x43/*C*/:text[textLength] = (qwchar)0x43/*C*/; textLower[textLength] = (qwchar)0x63/*c*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x44/*D*/:text[textLength] = (qwchar)0x44/*D*/; textLower[textLength] = (qwchar)0x64/*d*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x45/*E*/:text[textLength] = (qwchar)0x45/*E*/; textLower[textLength] = (qwchar)0x65/*e*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x46/*F*/:text[textLength] = (qwchar)0x46/*F*/; textLower[textLength] = (qwchar)0x66/*f*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x47/*G*/:text[textLength] = (qwchar)0x47/*G*/; textLower[textLength] = (qwchar)0x67/*g*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x48/*H*/:text[textLength] = (qwchar)0x48/*H*/; textLower[textLength] = (qwchar)0x68/*h*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x49/*I*/:text[textLength] = (qwchar)0x49/*I*/; textLower[textLength] = (qwchar)0x69/*i*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x4A/*J*/:text[textLength] = (qwchar)0x4A/*J*/; textLower[textLength] = (qwchar)0x6A/*j*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x4B/*K*/:text[textLength] = (qwchar)0x4B/*K*/; textLower[textLength] = (qwchar)0x6B/*k*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x4C/*L*/:text[textLength] = (qwchar)0x4C/*L*/; textLower[textLength] = (qwchar)0x6C/*l*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x4D/*M*/:text[textLength] = (qwchar)0x4D/*M*/; textLower[textLength] = (qwchar)0x6D/*m*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x4E/*N*/:text[textLength] = (qwchar)0x4E/*N*/; textLower[textLength] = (qwchar)0x6E/*n*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x4F/*O*/:text[textLength] = (qwchar)0x4F/*O*/; textLower[textLength] = (qwchar)0x6F/*o*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x3A1/*Ρ*/:textCountTotalChange++; case 0x50/*P*/:text[textLength] = (qwchar)0x50/*P*/; textLower[textLength] = (qwchar)0x70/*p*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x51/*Q*/:text[textLength] = (qwchar)0x51/*Q*/; textLower[textLength] = (qwchar)0x71/*q*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x52/*R*/:text[textLength] = (qwchar)0x52/*R*/; textLower[textLength] = (qwchar)0x72/*r*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x53/*S*/:text[textLength] = (qwchar)0x53/*S*/; textLower[textLength] = (qwchar)0x73/*s*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x54/*T*/:text[textLength] = (qwchar)0x54/*T*/; textLower[textLength] = (qwchar)0x74/*t*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x55/*U*/:text[textLength] = (qwchar)0x55/*U*/; textLower[textLength] = (qwchar)0x75/*u*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x56/*V*/:text[textLength] = (qwchar)0x56/*V*/; textLower[textLength] = (qwchar)0x76/*v*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x57/*W*/:text[textLength] = (qwchar)0x57/*W*/; textLower[textLength] = (qwchar)0x77/*w*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x58/*X*/:text[textLength] = (qwchar)0x58/*X*/; textLower[textLength] = (qwchar)0x78/*x*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
					case 0x59/*Y*/:text[textLength] = (qwchar)0x59/*Y*/; textLower[textLength] = (qwchar)0x79/*y*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountVowelCharacter++; break;
					case 0x5A/*Z*/:text[textLength] = (qwchar)0x5A/*Z*/; textLower[textLength] = (qwchar)0x7A/*z*/; textType[textLength] = CHARACTER_TYPE_ALPHABET_UPPER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountUpperCharacter++; textCountTotalAlphabet++; textCountConsonantCharacter++; break;
						/************************************************************************/
						/* Nhóm các kí tự số                                                    */
						/************************************************************************/
					case 0x30/*0*/:text[textLength] = (qwchar)0x30/*0*/; textLower[textLength] = (qwchar)0x30/*0*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; textCountZeroCharacter++; break;
					case 0x31/*1*/:text[textLength] = (qwchar)0x31/*1*/; textLower[textLength] = (qwchar)0x31/*1*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x32/*2*/:text[textLength] = (qwchar)0x32/*2*/; textLower[textLength] = (qwchar)0x32/*2*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x33/*3*/:text[textLength] = (qwchar)0x33/*3*/; textLower[textLength] = (qwchar)0x33/*3*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x34/*4*/:text[textLength] = (qwchar)0x34/*4*/; textLower[textLength] = (qwchar)0x34/*4*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x35/*5*/:text[textLength] = (qwchar)0x35/*5*/; textLower[textLength] = (qwchar)0x35/*5*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x36/*6*/:text[textLength] = (qwchar)0x36/*6*/; textLower[textLength] = (qwchar)0x36/*6*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x37/*7*/:text[textLength] = (qwchar)0x37/*7*/; textLower[textLength] = (qwchar)0x37/*7*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x38/*8*/:text[textLength] = (qwchar)0x38/*8*/; textLower[textLength] = (qwchar)0x38/*8*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
					case 0x39/*9*/:text[textLength] = (qwchar)0x39/*9*/; textLower[textLength] = (qwchar)0x39/*9*/; textType[textLength] = CHARACTER_TYPE_NUMBER; textIndex[textLength] = (qwchar)iChar; textLength++; textCountNumberCharacter++; break;
						/************************************************************************/
						/* Các dấu câu có trên bàn phím                                         */
						/************************************************************************/
					case 0x21/*! exclamation mark*/:text[textLength] = (qwchar)0x21/*! exclamation mark*/; textLower[textLength] = (qwchar)0x21/*! exclamation mark*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x22/*" quotation mark   (entity name is quot;)*/:text[textLength] = (qwchar)0x22/*" quotation mark   (entity name is quot;)*/; textLower[textLength] = (qwchar)0x22/*" quotation mark   (entity name is quot;)*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x23/*# number sign*/:text[textLength] = (qwchar)0x23/*# number sign*/; textLower[textLength] = (qwchar)0x23/*# number sign*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x24/*$ dollar sign*/:text[textLength] = (qwchar)0x24/*$ dollar sign*/; textLower[textLength] = (qwchar)0x24/*$ dollar sign*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x25/*% percent sign*/:text[textLength] = (qwchar)0x25/*% percent sign*/; textLower[textLength] = (qwchar)0x25/*% percent sign*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x26/*& ampersand   (entity name is &amp;)*/:text[textLength] = (qwchar)0x26/*& ampersand   (entity name is &amp;)*/; textLower[textLength] = (qwchar)0x26/*& ampersand   (entity name is &amp;)*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x27/*' apostrophe*/:text[textLength] = (qwchar)0x27/*' apostrophe*/; textLower[textLength] = (qwchar)0x27/*' apostrophe*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x28/*( left parenthesis*/:text[textLength] = (qwchar)0x28/*( left parenthesis*/; textLower[textLength] = (qwchar)0x28/*( left parenthesis*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x29/*) right parenthesis*/:text[textLength] = (qwchar)0x29/*) right parenthesis*/; textLower[textLength] = (qwchar)0x29/*) right parenthesis*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x2A/** asterisk*/:text[textLength] = (qwchar)0x2A/** asterisk*/; textLower[textLength] = (qwchar)0x2A/** asterisk*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x2B/*+ plus sign*/:text[textLength] = (qwchar)0x2B/*+ plus sign*/; textLower[textLength] = (qwchar)0x2B/*+ plus sign*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x2C/*, comma*/:text[textLength] = (qwchar)0x2C/*, comma*/; textLower[textLength] = (qwchar)0x2C/*, comma*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; textCountCommaCharacter++; break;
					case 0x2D/*- hyphen-minus*/:text[textLength] = (qwchar)0x2D/*- hyphen-minus*/; textLower[textLength] = (qwchar)0x2D/*- hyphen-minus*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x2E/*. dot*/:text[textLength] = (qwchar)0x2E/*. dot*/; textLower[textLength] = (qwchar)0x2E/*. dot*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; textCountDotCharacter++; break;
					case 0x2F/*/ slash*/:text[textLength] = (qwchar)0x2F/*/ slash*/; textLower[textLength] = (qwchar)0x2F/*/ slash*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; textCountSlashCharacter++; break;
					case 0x3A/*"; colon*/:text[textLength] = (qwchar)0x3A/*"; colon*/; textLower[textLength] = (qwchar)0x3A/*"; colon*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x3B/*; semicolon*/:text[textLength] = (qwchar)0x3B/*; semicolon*/; textLower[textLength] = (qwchar)0x3B/*; semicolon*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x3C/*< less-than sign   (entity name is &lt;)*/:text[textLength] = (qwchar)0x3C/*< less-than sign   (entity name is &lt;)*/; textLower[textLength] = (qwchar)0x3C/*< less-than sign   (entity name is &lt;)*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x3D/*= equals sign*/:text[textLength] = (qwchar)0x3D/*= equals sign*/; textLower[textLength] = (qwchar)0x3D/*= equals sign*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x3E/*> greater-than sign   (entity name is &gt;)*/:text[textLength] = (qwchar)0x3E/*> greater-than sign   (entity name is &gt;)*/; textLower[textLength] = (qwchar)0x3E/*> greater-than sign   (entity name is &gt;)*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x3F/*? question mark*/:text[textLength] = (qwchar)0x3F/*? question mark*/; textLower[textLength] = (qwchar)0x3F/*? question mark*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x40/*@ commercial at*/:text[textLength] = (qwchar)0x40/*@ commercial at*/; textLower[textLength] = (qwchar)0x40/*@ commercial at*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x5B/*[ left square bracket*/:text[textLength] = (qwchar)0x5B/*[ left square bracket*/; textLower[textLength] = (qwchar)0x5B/*[ left square bracket*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x5C/*\ reverse solidus*/:text[textLength] = (qwchar)0x5C/*\ reverse solidus*/; textLower[textLength] = (qwchar)0x5C/*\ reverse solidus*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x5D/*] right square bracket*/:text[textLength] = (qwchar)0x5D/*] right square bracket*/; textLower[textLength] = (qwchar)0x5D/*] right square bracket*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x5E/*^ circumflex accent*/:text[textLength] = (qwchar)0x5E/*^ circumflex accent*/; textLower[textLength] = (qwchar)0x5E/*^ circumflex accent*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x5F/*_ low line*/:text[textLength] = (qwchar)0x5F/*_ low line*/; textLower[textLength] = (qwchar)0x5F/*_ low line*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x60/*` grave accent*/:text[textLength] = (qwchar)0x60/*` grave accent*/; textLower[textLength] = (qwchar)0x60/*` grave accent*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x7B/*{ left curly bracket*/:text[textLength] = (qwchar)0x7B/*{ left curly bracket*/; textLower[textLength] = (qwchar)0x7B/*{ left curly bracket*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x7C/*| vertical line*/:text[textLength] = (qwchar)0x7C/*| vertical line*/; textLower[textLength] = (qwchar)0x7C/*| vertical line*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x7D/*} right curly bracket*/:text[textLength] = (qwchar)0x7D/*} right curly bracket*/; textLower[textLength] = (qwchar)0x7D/*} right curly bracket*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
					case 0x7E/*~ tilde*/:text[textLength] = (qwchar)0x7E/*~ tilde*/; textLower[textLength] = (qwchar)0x7E/*~ tilde*/; textType[textLength] = CHARACTER_TYPE_OTHER_ON_KEYBOARD; textIndex[textLength] = (qwchar)iChar; textLength++; textCountOtherOnKeyBoard++; break;
						/************************************************************************/
						/* Nhóm các kí tự dấu đặc biệt                                          */
						/************************************************************************/
					case 0x300/*` VIETNAMESE_TONE_HUYEN*/:
					case 0x340/*VIETNAMESE_TONE_HUYEN*/:
					case 0x2CB/*VIETNAMESE_TONE_HUYEN*/:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x61/*af*/:text[lastIndexOfOutput] = (qwchar)0xE0/*af*/; textLower[lastIndexOfOutput] = (qwchar)0xE0/*af*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x41/*AF*/:text[lastIndexOfOutput] = (qwchar)0xC0/*AF*/; textLower[lastIndexOfOutput] = (qwchar)0xE0/*af*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x103/*awf*/:text[lastIndexOfOutput] = (qwchar)0x1EB1/*awf*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB1/*awf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x102/*AWF*/:text[lastIndexOfOutput] = (qwchar)0x1EB0/*AWF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB1/*awf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xE2/*aaf*/:text[lastIndexOfOutput] = (qwchar)0x1EA7/*aaf*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA7/*aaf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC2/*AAF*/:text[lastIndexOfOutput] = (qwchar)0x1EA6/*AAF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA7/*aaf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*ef*/:text[lastIndexOfOutput] = (qwchar)0xE8/*ef*/; textLower[lastIndexOfOutput] = (qwchar)0xE8/*ef*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x45/*EF*/:text[lastIndexOfOutput] = (qwchar)0xC8/*EF*/; textLower[lastIndexOfOutput] = (qwchar)0xE8/*ef*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xEA/*eef*/:text[lastIndexOfOutput] = (qwchar)0x1EC1/*eef*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC1/*eef*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xCA/*EEF*/:text[lastIndexOfOutput] = (qwchar)0x1EC0/*EEF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC1/*eef*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x69/*if*/:text[lastIndexOfOutput] = (qwchar)0xEC/*if*/; textLower[lastIndexOfOutput] = (qwchar)0xEC/*if*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x49/*IF*/:text[lastIndexOfOutput] = (qwchar)0xCC/*IF*/; textLower[lastIndexOfOutput] = (qwchar)0xEC/*if*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*of*/:text[lastIndexOfOutput] = (qwchar)0xF2/*of*/; textLower[lastIndexOfOutput] = (qwchar)0xF2/*of*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x4F/*OF*/:text[lastIndexOfOutput] = (qwchar)0xD2/*OF*/; textLower[lastIndexOfOutput] = (qwchar)0xF2/*of*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xF4/*oof*/:text[lastIndexOfOutput] = (qwchar)0x1ED3/*oof*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED3/*oof*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD4/*OOF*/:text[lastIndexOfOutput] = (qwchar)0x1ED2/*OOF*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED3/*oof*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A1/*owf*/:text[lastIndexOfOutput] = (qwchar)0x1EDD/*owf*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDD/*owf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A0/*OWF*/:text[lastIndexOfOutput] = (qwchar)0x1EDC/*OWF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDD/*owf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x75/*uf*/:text[lastIndexOfOutput] = (qwchar)0xF9/*uf*/; textLower[lastIndexOfOutput] = (qwchar)0xF9/*uf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x55/*UF*/:text[lastIndexOfOutput] = (qwchar)0xD9/*UF*/; textLower[lastIndexOfOutput] = (qwchar)0xF9/*uf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1B0/*uwf*/:text[lastIndexOfOutput] = (qwchar)0x1EEB/*uwf*/; textLower[lastIndexOfOutput] = (qwchar)0x1EEB/*uwf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1AF/*UWF*/:text[lastIndexOfOutput] = (qwchar)0x1EEA/*UWF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EEB/*uwf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x79/*yf*/:text[lastIndexOfOutput] = (qwchar)0x1EF3/*yf*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF3/*yf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x59/*YF*/:text[lastIndexOfOutput] = (qwchar)0x1EF2/*YF*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF3/*yf*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							default:textErrorCombiningTone = 1/*` VIETNAMESE_TONE_HUYEN*/; textCountTotalChange++; break;
							}
						}
						break;
					case 0x303/*~ VIETNAMESE_TONE_NGA*/:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x61/*ax*/:text[lastIndexOfOutput] = (qwchar)0xE3/*ax*/; textLower[lastIndexOfOutput] = (qwchar)0xE3/*ax*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x41/*AX*/:text[lastIndexOfOutput] = (qwchar)0xC3/*AX*/; textLower[lastIndexOfOutput] = (qwchar)0xE3/*ax*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x103/*awx*/:text[lastIndexOfOutput] = (qwchar)0x1EB5/*awx*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB5/*awx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x102/*AWX*/:text[lastIndexOfOutput] = (qwchar)0x1EB4/*AWX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB5/*awx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xE2/*aax*/:text[lastIndexOfOutput] = (qwchar)0x1EAB/*aax*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAB/*aax*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC2/*AAX*/:text[lastIndexOfOutput] = (qwchar)0x1EAA/*AAX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAB/*aax*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*ex*/:text[lastIndexOfOutput] = (qwchar)0x1EBD/*ex*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBD/*ex*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x45/*EX*/:text[lastIndexOfOutput] = (qwchar)0x1EBC/*EX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBD/*ex*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xEA/*eex*/:text[lastIndexOfOutput] = (qwchar)0x1EC5/*eex*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC5/*eex*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xCA/*EEX*/:text[lastIndexOfOutput] = (qwchar)0x1EC4/*EEX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC5/*eex*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x69/*ix*/:text[lastIndexOfOutput] = (qwchar)0x129/*ix*/; textLower[lastIndexOfOutput] = (qwchar)0x129/*ix*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x49/*IX*/:text[lastIndexOfOutput] = (qwchar)0x128/*IX*/; textLower[lastIndexOfOutput] = (qwchar)0x129/*ix*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*ox*/:text[lastIndexOfOutput] = (qwchar)0xF5/*ox*/; textLower[lastIndexOfOutput] = (qwchar)0xF5/*ox*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x4F/*OX*/:text[lastIndexOfOutput] = (qwchar)0xD5/*OX*/; textLower[lastIndexOfOutput] = (qwchar)0xF5/*ox*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xF4/*oox*/:text[lastIndexOfOutput] = (qwchar)0x1ED7/*oox*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED7/*oox*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD4/*OOX*/:text[lastIndexOfOutput] = (qwchar)0x1ED6/*OOX*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED7/*oox*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A1/*owx*/:text[lastIndexOfOutput] = (qwchar)0x1EE1/*owx*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE1/*owx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A0/*OWX*/:text[lastIndexOfOutput] = (qwchar)0x1EE0/*OWX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE1/*owx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x75/*ux*/:text[lastIndexOfOutput] = (qwchar)0x169/*ux*/; textLower[lastIndexOfOutput] = (qwchar)0x169/*ux*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x55/*UX*/:text[lastIndexOfOutput] = (qwchar)0x168/*UX*/; textLower[lastIndexOfOutput] = (qwchar)0x169/*ux*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1B0/*uwx*/:text[lastIndexOfOutput] = (qwchar)0x1EEF/*uwx*/; textLower[lastIndexOfOutput] = (qwchar)0x1EEF/*uwx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1AF/*UWX*/:text[lastIndexOfOutput] = (qwchar)0x1EEE/*UWX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EEF/*uwx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x79/*yx*/:text[lastIndexOfOutput] = (qwchar)0x1EF9/*yx*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF9/*yx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x59/*YX*/:text[lastIndexOfOutput] = (qwchar)0x1EF8/*YX*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF9/*yx*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							default:textErrorCombiningTone = 2/*~ VIETNAMESE_TONE_NGA*/; textCountTotalChange++; break;
							}
						}
						break;
					case 0x309/*? VIETNAMESE_TONE_HOI*/:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x61/*ar*/:text[lastIndexOfOutput] = (qwchar)0x1EA3/*ar*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA3/*ar*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x41/*AR*/:text[lastIndexOfOutput] = (qwchar)0x1EA2/*AR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA3/*ar*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x103/*awr*/:text[lastIndexOfOutput] = (qwchar)0x1EB3/*awr*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB3/*awr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x102/*AWR*/:text[lastIndexOfOutput] = (qwchar)0x1EB2/*AWR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB3/*awr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xE2/*aar*/:text[lastIndexOfOutput] = (qwchar)0x1EA9/*aar*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA9/*aar*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC2/*AAR*/:text[lastIndexOfOutput] = (qwchar)0x1EA8/*AAR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA9/*aar*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*er*/:text[lastIndexOfOutput] = (qwchar)0x1EBB/*er*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBB/*er*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x45/*ER*/:text[lastIndexOfOutput] = (qwchar)0x1EBA/*ER*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBB/*er*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xEA/*eer*/:text[lastIndexOfOutput] = (qwchar)0x1EC3/*eer*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC3/*eer*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xCA/*EER*/:text[lastIndexOfOutput] = (qwchar)0x1EC2/*EER*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC3/*eer*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x69/*ir*/:text[lastIndexOfOutput] = (qwchar)0x1EC9/*ir*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC9/*ir*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x49/*IR*/:text[lastIndexOfOutput] = (qwchar)0x1EC8/*IR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC9/*ir*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*or*/:text[lastIndexOfOutput] = (qwchar)0x1ECF/*or*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECF/*or*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x4F/*OR*/:text[lastIndexOfOutput] = (qwchar)0x1ECE/*OR*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECF/*or*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xF4/*oor*/:text[lastIndexOfOutput] = (qwchar)0x1ED5/*oor*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED5/*oor*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD4/*OOR*/:text[lastIndexOfOutput] = (qwchar)0x1ED4/*OOR*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED5/*oor*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A1/*owr*/:text[lastIndexOfOutput] = (qwchar)0x1EDF/*owr*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDF/*owr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A0/*OWR*/:text[lastIndexOfOutput] = (qwchar)0x1EDE/*OWR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDF/*owr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x75/*ur*/:text[lastIndexOfOutput] = (qwchar)0x1EE7/*ur*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE7/*ur*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x55/*UR*/:text[lastIndexOfOutput] = (qwchar)0x1EE6/*UR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE7/*ur*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1B0/*uwr*/:text[lastIndexOfOutput] = (qwchar)0x1EED/*uwr*/; textLower[lastIndexOfOutput] = (qwchar)0x1EED/*uwr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1AF/*UWR*/:text[lastIndexOfOutput] = (qwchar)0x1EEC/*UWR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EED/*uwr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x79/*yr*/:text[lastIndexOfOutput] = (qwchar)0x1EF7/*yr*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF7/*yr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x59/*YR*/:text[lastIndexOfOutput] = (qwchar)0x1EF6/*YR*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF7/*yr*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							default:textErrorCombiningTone = 3/*? VIETNAMESE_TONE_HOI*/; textCountTotalChange++; break;
							}
						}
						break;
					case 0x301/*' VIETNAMESE_TONE_SAC*/:
					case 0xB4/*' VIETNAMESE_TONE_SAC*/:
					case 0x341/*VIETNAMESE_TONE_SAC*/:
					case 0x2CA/*VIETNAMESE_TONE_SAC*/:
					case 0x1FFD/*VIETNAMESE_TONE_SAC*/:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x61/*as*/:text[lastIndexOfOutput] = (qwchar)0xE1/*as*/; textLower[lastIndexOfOutput] = (qwchar)0xE1/*as*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x41/*AS*/:text[lastIndexOfOutput] = (qwchar)0xC1/*AS*/; textLower[lastIndexOfOutput] = (qwchar)0xE1/*as*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x103/*aws*/:text[lastIndexOfOutput] = (qwchar)0x1EAF/*aws*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAF/*aws*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x102/*AWS*/:text[lastIndexOfOutput] = (qwchar)0x1EAE/*AWS*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAF/*aws*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xE2/*aas*/:text[lastIndexOfOutput] = (qwchar)0x1EA5/*aas*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA5/*aas*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC2/*AAS*/:text[lastIndexOfOutput] = (qwchar)0x1EA4/*AAS*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA5/*aas*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*es*/:text[lastIndexOfOutput] = (qwchar)0xE9/*es*/; textLower[lastIndexOfOutput] = (qwchar)0xE9/*es*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x45/*ES*/:text[lastIndexOfOutput] = (qwchar)0xC9/*ES*/; textLower[lastIndexOfOutput] = (qwchar)0xE9/*es*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xEA/*ees*/:text[lastIndexOfOutput] = (qwchar)0x1EBF/*ees*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBF/*ees*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xCA/*EES*/:text[lastIndexOfOutput] = (qwchar)0x1EBE/*EES*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBF/*ees*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x69/*is*/:text[lastIndexOfOutput] = (qwchar)0xED/*is*/; textLower[lastIndexOfOutput] = (qwchar)0xED/*is*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x49/*IS*/:text[lastIndexOfOutput] = (qwchar)0xCD/*IS*/; textLower[lastIndexOfOutput] = (qwchar)0xED/*is*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*os*/:text[lastIndexOfOutput] = (qwchar)0xF3/*os*/; textLower[lastIndexOfOutput] = (qwchar)0xF3/*os*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x4F/*OS*/:text[lastIndexOfOutput] = (qwchar)0xD3/*OS*/; textLower[lastIndexOfOutput] = (qwchar)0xF3/*os*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xF4/*oos*/:text[lastIndexOfOutput] = (qwchar)0x1ED1/*oos*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED1/*oos*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD4/*OOS*/:text[lastIndexOfOutput] = (qwchar)0x1ED0/*OOS*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED1/*oos*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A1/*ows*/:text[lastIndexOfOutput] = (qwchar)0x1EDB/*ows*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDB/*ows*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A0/*OWS*/:text[lastIndexOfOutput] = (qwchar)0x1EDA/*OWS*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDB/*ows*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x75/*us*/:text[lastIndexOfOutput] = (qwchar)0xFA/*us*/; textLower[lastIndexOfOutput] = (qwchar)0xFA/*us*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x55/*US*/:text[lastIndexOfOutput] = (qwchar)0xDA/*US*/; textLower[lastIndexOfOutput] = (qwchar)0xFA/*us*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1B0/*uws*/:text[lastIndexOfOutput] = (qwchar)0x1EE9/*uws*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE9/*uws*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1AF/*UWS*/:text[lastIndexOfOutput] = (qwchar)0x1EE8/*UWS*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE9/*uws*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x79/*ys*/:text[lastIndexOfOutput] = (qwchar)0xFD/*ys*/; textLower[lastIndexOfOutput] = (qwchar)0xFD/*ys*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x59/*YS*/:text[lastIndexOfOutput] = (qwchar)0xDD/*YS*/; textLower[lastIndexOfOutput] = (qwchar)0xFD/*ys*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							default:textErrorCombiningTone = 4/*' VIETNAMESE_TONE_SAC*/; textCountTotalChange++; break;
							}
						}
						break;
					case 0x323/*. VIETNAMESE_TONE_NANG*/:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x61/*aj*/:text[lastIndexOfOutput] = (qwchar)0x1EA1/*aj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA1/*aj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x41/*AJ*/:text[lastIndexOfOutput] = (qwchar)0x1EA0/*AJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA1/*aj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x103/*awj*/:text[lastIndexOfOutput] = (qwchar)0x1EB7/*awj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB7/*awj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x102/*AWJ*/:text[lastIndexOfOutput] = (qwchar)0x1EB6/*AWJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB7/*awj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xE2/*aaj*/:text[lastIndexOfOutput] = (qwchar)0x1EAD/*aaj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAD/*aaj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC2/*AAJ*/:text[lastIndexOfOutput] = (qwchar)0x1EAC/*AAJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAD/*aaj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*ej*/:text[lastIndexOfOutput] = (qwchar)0x1EB9/*ej*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB9/*ej*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x45/*EJ*/:text[lastIndexOfOutput] = (qwchar)0x1EB8/*EJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB9/*ej*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xEA/*eej*/:text[lastIndexOfOutput] = (qwchar)0x1EC7/*eej*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC7/*eej*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xCA/*EEJ*/:text[lastIndexOfOutput] = (qwchar)0x1EC6/*EEJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC7/*eej*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x69/*ij*/:text[lastIndexOfOutput] = (qwchar)0x1ECB/*ij*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECB/*ij*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x49/*IJ*/:text[lastIndexOfOutput] = (qwchar)0x1ECA/*IJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECB/*ij*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*oj*/:text[lastIndexOfOutput] = (qwchar)0x1ECD/*oj*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECD/*oj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x4F/*OJ*/:text[lastIndexOfOutput] = (qwchar)0x1ECC/*OJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ECD/*oj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xF4/*ooj*/:text[lastIndexOfOutput] = (qwchar)0x1ED9/*ooj*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED9/*ooj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD4/*OOJ*/:text[lastIndexOfOutput] = (qwchar)0x1ED8/*OOJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED9/*ooj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A1/*owj*/:text[lastIndexOfOutput] = (qwchar)0x1EE3/*owj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE3/*owj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1A0/*OWJ*/:text[lastIndexOfOutput] = (qwchar)0x1EE2/*OWJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE3/*owj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x75/*uj*/:text[lastIndexOfOutput] = (qwchar)0x1EE5/*uj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE5/*uj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x55/*UJ*/:text[lastIndexOfOutput] = (qwchar)0x1EE4/*UJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE5/*uj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1B0/*uwj*/:text[lastIndexOfOutput] = (qwchar)0x1EF1/*uwj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF1/*uwj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x1AF/*UWJ*/:text[lastIndexOfOutput] = (qwchar)0x1EF0/*UWJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF1/*uwj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x79/*yj*/:text[lastIndexOfOutput] = (qwchar)0x1EF5/*yj*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF5/*yj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x59/*YJ*/:text[lastIndexOfOutput] = (qwchar)0x1EF4/*YJ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EF5/*yj*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							default:textErrorCombiningTone = 5/*. VIETNAMESE_TONE_NANG*/; textCountTotalChange++; break;
							}
						}
						break;
					case 0x302/* ̂ */:case 0x311/* ̑ */:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x41/*A*/:text[lastIndexOfOutput] = (qwchar)0xC2/*Â*/; textLower[lastIndexOfOutput] = (qwchar)0xE2/*â*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x61/*a*/:text[lastIndexOfOutput] = (qwchar)0xE2/*â*/; textLower[lastIndexOfOutput] = (qwchar)0xE2/*â*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC0/*À*/:text[lastIndexOfOutput] = (qwchar)0x1EA6/*Ầ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA7/*ầ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE0/*à*/:text[lastIndexOfOutput] = (qwchar)0x1EA7/*ầ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA7/*ầ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xC1/*Á*/:text[lastIndexOfOutput] = (qwchar)0x1EA4/*Ấ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA5/*ấ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE1/*á*/:text[lastIndexOfOutput] = (qwchar)0x1EA5/*ấ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA5/*ấ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA2/*Ả*/:text[lastIndexOfOutput] = (qwchar)0x1EA8/*Ẩ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA9/*ẩ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA3/*ả*/:text[lastIndexOfOutput] = (qwchar)0x1EA9/*ẩ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EA9/*ẩ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xC3/*Ã*/:text[lastIndexOfOutput] = (qwchar)0x1EAA/*Ẫ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAB/*ẫ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE3/*ã*/:text[lastIndexOfOutput] = (qwchar)0x1EAB/*ẫ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAB/*ẫ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA0/*Ạ*/:text[lastIndexOfOutput] = (qwchar)0x1EAC/*Ậ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAD/*ậ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA1/*ạ*/:text[lastIndexOfOutput] = (qwchar)0x1EAD/*ậ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAD/*ậ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x45/*E*/:text[lastIndexOfOutput] = (qwchar)0xCA/*Ê*/; textLower[lastIndexOfOutput] = (qwchar)0xEA/*ê*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x65/*e*/:text[lastIndexOfOutput] = (qwchar)0xEA/*ê*/; textLower[lastIndexOfOutput] = (qwchar)0xEA/*ê*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC8/*È*/:text[lastIndexOfOutput] = (qwchar)0x1EC0/*Ề*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC1/*ề*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE8/*è*/:text[lastIndexOfOutput] = (qwchar)0x1EC1/*ề*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC1/*ề*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xC9/*É*/:text[lastIndexOfOutput] = (qwchar)0x1EBE/*Ế*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBF/*ế*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE9/*é*/:text[lastIndexOfOutput] = (qwchar)0x1EBF/*ế*/; textLower[lastIndexOfOutput] = (qwchar)0x1EBF/*ế*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EBA/*Ẻ*/:text[lastIndexOfOutput] = (qwchar)0x1EC2/*Ể*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC3/*ể*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EBB/*ẻ*/:text[lastIndexOfOutput] = (qwchar)0x1EC3/*ể*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC3/*ể*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EBC/*Ẽ*/:text[lastIndexOfOutput] = (qwchar)0x1EC4/*Ễ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC5/*ễ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EBD/*ẽ*/:text[lastIndexOfOutput] = (qwchar)0x1EC5/*ễ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC5/*ễ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EB8/*Ẹ*/:text[lastIndexOfOutput] = (qwchar)0x1EC6/*Ệ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC7/*ệ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EB9/*ẹ*/:text[lastIndexOfOutput] = (qwchar)0x1EC7/*ệ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EC7/*ệ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x4F/*O*/:text[lastIndexOfOutput] = (qwchar)0xD4/*Ô*/; textLower[lastIndexOfOutput] = (qwchar)0xF4/*ô*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*o*/:text[lastIndexOfOutput] = (qwchar)0xF4/*ô*/; textLower[lastIndexOfOutput] = (qwchar)0xF4/*ô*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD2/*Ò*/:text[lastIndexOfOutput] = (qwchar)0x1ED2/*Ồ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED3/*ồ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF2/*ò*/:text[lastIndexOfOutput] = (qwchar)0x1ED3/*ồ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED3/*ồ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xD3/*Ó*/:text[lastIndexOfOutput] = (qwchar)0x1ED0/*Ố*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED1/*ố*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF3/*ó*/:text[lastIndexOfOutput] = (qwchar)0x1ED1/*ố*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED1/*ố*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECE/*Ỏ*/:text[lastIndexOfOutput] = (qwchar)0x1ED4/*Ổ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED5/*ổ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECF/*ỏ*/:text[lastIndexOfOutput] = (qwchar)0x1ED5/*ổ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED5/*ổ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xD5/*Õ*/:text[lastIndexOfOutput] = (qwchar)0x1ED6/*Ỗ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED7/*ỗ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF5/*õ*/:text[lastIndexOfOutput] = (qwchar)0x1ED7/*ỗ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED7/*ỗ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECC/*Ọ*/:text[lastIndexOfOutput] = (qwchar)0x1ED8/*Ộ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED9/*ộ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECD/*ọ*/:text[lastIndexOfOutput] = (qwchar)0x1ED9/*ộ*/; textLower[lastIndexOfOutput] = (qwchar)0x1ED9/*ộ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							default:textCountTotalChange++; break;
							}
						}
						break;
					case 0x306/* ̆ */:case 0x30C/* ̌ */:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x41/*A*/:text[lastIndexOfOutput] = (qwchar)0x102/*Ă*/; textLower[lastIndexOfOutput] = (qwchar)0x103/*ă*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x61/*a*/:text[lastIndexOfOutput] = (qwchar)0x103/*ă*/; textLower[lastIndexOfOutput] = (qwchar)0x103/*ă*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xC0/*À*/:text[lastIndexOfOutput] = (qwchar)0x1EB0/*Ằ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB1/*ằ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE0/*à*/:text[lastIndexOfOutput] = (qwchar)0x1EB1/*ằ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB1/*ằ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xC1/*Á*/:text[lastIndexOfOutput] = (qwchar)0x1EAE/*Ắ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAF/*ắ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE1/*á*/:text[lastIndexOfOutput] = (qwchar)0x1EAF/*ắ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EAF/*ắ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA2/*Ả*/:text[lastIndexOfOutput] = (qwchar)0x1EB2/*Ẳ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB3/*ẳ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA3/*ả*/:text[lastIndexOfOutput] = (qwchar)0x1EB3/*ẳ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB3/*ẳ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xC3/*Ã*/:text[lastIndexOfOutput] = (qwchar)0x1EB4/*Ẵ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB5/*ẵ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xE3/*ã*/:text[lastIndexOfOutput] = (qwchar)0x1EB5/*ẵ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB5/*ẵ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA0/*Ạ*/:text[lastIndexOfOutput] = (qwchar)0x1EB6/*Ặ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB7/*ặ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1EA1/*ạ*/:text[lastIndexOfOutput] = (qwchar)0x1EB7/*ặ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EB7/*ặ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							default:textCountTotalChange++; break;
							}
						}
						break;
					case 0x31B/* ̛ */:
						if (textLength/*>0*/)
						{
							textType[textLength] = CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE;
							int lastIndexOfOutput = textLength - 1;
							switch (text[lastIndexOfOutput])
							{
							case 0x4F/*O*/:text[lastIndexOfOutput] = (qwchar)0x1A0/*Ơ*/; textLower[lastIndexOfOutput] = (qwchar)0x1A1/*ơ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0x6F/*o*/:text[lastIndexOfOutput] = (qwchar)0x1A1/*ơ*/; textLower[lastIndexOfOutput] = (qwchar)0x1A1/*ơ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;
							case 0xD2/*Ò*/:text[lastIndexOfOutput] = (qwchar)0x1EDC/*Ờ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDD/*ờ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF2/*ò*/:text[lastIndexOfOutput] = (qwchar)0x1EDD/*ờ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDD/*ờ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xD3/*Ó*/:text[lastIndexOfOutput] = (qwchar)0x1EDA/*Ớ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDB/*ớ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF3/*ó*/:text[lastIndexOfOutput] = (qwchar)0x1EDB/*ớ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDB/*ớ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECE/*Ỏ*/:text[lastIndexOfOutput] = (qwchar)0x1EDE/*Ở*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDF/*ở*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECF/*ỏ*/:text[lastIndexOfOutput] = (qwchar)0x1EDF/*ở*/; textLower[lastIndexOfOutput] = (qwchar)0x1EDF/*ở*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xD5/*Õ*/:text[lastIndexOfOutput] = (qwchar)0x1EE0/*Ỡ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE1/*ỡ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0xF5/*õ*/:text[lastIndexOfOutput] = (qwchar)0x1EE1/*ỡ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE1/*ỡ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECC/*Ọ*/:text[lastIndexOfOutput] = (qwchar)0x1EE2/*Ợ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE3/*ợ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;
							case 0x1ECD/*ọ*/:text[lastIndexOfOutput] = (qwchar)0x1EE3/*ợ*/; textLower[lastIndexOfOutput] = (qwchar)0x1EE3/*ợ*/; textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER; textIndex[textLength] = (qwchar)iChar; textCountTotalChange++; break;


							case 0x55/*U*/:text[lastIndexOfOutput] = (qwchar)0x1AF/*Ư*/;
								textLower[lastIndexOfOutput] = (qwchar)0x1B0/*ư*/;
								textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER;
								textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++;
								textCountEnglishVowelCharacter--; textCountTotalChange++; break;



							case 0x75/*u*/:text[lastIndexOfOutput] = (qwchar)0x1B0/*ư*/;
								textLower[lastIndexOfOutput] = (qwchar)0x1B0/*ư*/;
								textType[lastIndexOfOutput] = CHARACTER_TYPE_VIETNAMESE_ONLY_LOWER;
								textIndex[textLength] = (qwchar)iChar; textCountVietnameseOnlyCharacter++; textCountEnglishVowelCharacter--; textCountTotalChange++; break;



							default:textCountTotalChange++; break;
							}
						}
						break;

						/************************************************************************/
						/* Các kí tự đặc biệt khác ngoài bảng mã                                */
						/************************************************************************/
					default:
						text[textLength] = currentCharacter;
						textLower[textLength] = currentCharacter;
						textType[textLength] = CHARACTER_TYPE_UNKNOWN;
						textIndex[textLength] = (qwchar)iChar;
						textLength++;
						textCountUnknownCharacter++;
						if (flagStandardTextForTTS)
						{
							text[textLength] = 0x20/*space*/;
							textLower[textLength] = 0x20/*space*/;
						}
						break;
					}//kết thúc của switch
				}//kết thúc của for duyệt từ đầu chuỗi originalText đến cuối
				text[textLength] = 0x20/*space*/;
				textLower[textLength] = 0x20/*space*/;
				/*------------------------   Kết thúc bước 1 -------------------------*/


				/************************************************************************/
				/* Bước 2 : Loại bỏ các kí tự lạ ở phần đầu                             */
				/************************************************************************/
				while (textLength > 0 && textType[0] == CHARACTER_TYPE_UNKNOWN)
				{
					text++;
					textLower++;
					textType++;
					textIndex++;
					textLength--;
					textCountUnknownCharacter--;
				}
				while ((textLength > 0 && textType[textLength - 1] == CHARACTER_TYPE_UNKNOWN) || (textLength > 1 && textType[textLength - 1] == CHARACTER_TYPE_OTHER_ON_KEYBOARD))
				{
					textLength--;
					qwchar removeCharacter = text[textLength];
					text[textLength] = 0x20/*space*/;
					textLower[textLength] = 0x20/*space*/;
					if (textLength > 0 && textType[textLength] == CHARACTER_TYPE_OTHER_ON_KEYBOARD)
					{
						TEXT_NODE* insertTextNode = (TEXT_NODE*)qcalloc(1, sizeof(TEXT_NODE));
						if (insertTextNode)
						{
							qwchar const* insertText = textNode->originalText + textIndex[textLength];
							insertTextNode->originalText = insertText;
							insertTextNode->originalTextLength = 1;
							insertTextNode->text = insertText;
							insertTextNode->textLength = 1;
							switch (*insertText)
							{
							case 0x2C/*,*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceCommaTime;
								break;

							case 0x21/*!*/:
							case 0x3F/*?*/:
							case 0x2E/*.*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceSentenceTime;
								break;

							case 0xD/*\r Carriage Return*/:
							case 0xA/*\n Line Feed*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceNewLineTime;
								break;

							case 0x3B/*;*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceSemicolonTime;
								break;

							case 0x28/*(*/:
							case 0x29/*)*/:
							case 0x2018/*‘ left single quotation mark*/:
							case 0x2019/*’ right single quotation mark*/:
							case 0x201C/*“ left double quotation mark*/:
							case 0x201D/*” right double quotation mark*/:
							case 0x27/*' single quotation mark*/:
							case 0x22/*" double quotation mark*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceQuotationTime;
								break;


							case 0x3A/*:*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceColonTime;
								break;
							case 0x2A/***/: insertTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; insertTextNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_S_A_O; break;
							case 0x23/*#*/: insertTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; insertTextNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_T_H_AW_N_G; break;
							case 0x3D/*=*/:insertTextNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE; insertTextNode->vietnameseSyllableIdentifier = VIETNAMESE_SYLLABLE_B_AWF_N_G; break;

							case 0x2D/*-*/:
							case  0x2013/*–*/:
								insertTextNode->textNodeType = TEXT_NODE_TYPE_SILENCE;
								insertTextNode->silenceTimeInSecond = silenceShortTime;
								break;
							}


							//insert
							insertTextNode->next = textNode->next;
							insertTextNode->back = textNode;
							if (textNode->next) textNode->next->back = insertTextNode;
							textNode->next = insertTextNode;
							if (tail == textNode) tail = insertTextNode;
							countTotalNode++;


							//update text list info
							if (insertTextNode->textNodeType == TEXT_NODE_TYPE_UNKNOWN)
							{
								countTotalUnknownNode++;
								if (utail == textNode) utail = insertTextNode;
							}
						}
						else
						{
							printf("\nVietnameseTextNormalizer : Calloc fail!");
							DungManHinh;
						}
						textCountOtherOnKeyBoard--;
						switch (removeCharacter)
						{
						case 0x2C/*,*/:textCountCommaCharacter--; break;
						case 0x2E/*.*/:textCountDotCharacter--; break;
						case 0x2F/*/*/:textCountSlashCharacter--; break;
						}
					}
					else textCountUnknownCharacter--;
				}
				if (textLength == 0)
				{
					textNode->originalTextLength = 0;
					textNode->textLength = 0;
				}
				else
				{
					textNode->originalText += textIndex[0];
					textNode->originalTextLength = textIndex[textLength - 1] + 1 - textIndex[0] + (textType[textLength] == CHARACTER_TYPE_VIETNAMESE_COMBINING_TONE);
					textNode->text = textNode->originalText;
					textNode->textLength = textNode->originalTextLength;
				}
				/*------------------------   Kết thúc bước 2 -------------------------*/


				/************************************************************************/
				/* Bước 3 : Xử lý các case tốt, không cần check context                 */
				/************************************************************************/
				bool	flagNeedMoreChecking = (textLength > 0);
				bool	flagNeedStoreTextMemory = (textLength > 0 /*&& textCountTotalChange > 0*/);
				bool	flagNeedStoreCombiningTone = (textLength > 0 && textErrorCombiningTone > 0 && textCountTotalAlphabet > 0);
				if (textCountUpperCharacter == 0 && textCountLowerCharacter > 0) textNode->capital = TEXT_NODE_CAPITAL_LOWER;
				else if (textCountLowerCharacter == 0 && textCountUpperCharacter > 0) textNode->capital = TEXT_NODE_CAPITAL_CAPITAL;
				else if (textCountUpperCharacter == 1 && (textType[0] == CHARACTER_TYPE_ALPHABET_UPPER || textType[0] == CHARACTER_TYPE_VIETNAMESE_ONLY_UPPER) && textCountLowerCharacter > 0) textNode->capital = TEXT_NODE_CAPITAL_UPPER;
				if (textCountTotalAlphabet > 0)
				{//nếu có kí tự chữ
					TEXT_NODE_CAPITAL					tempCapital /*Không có ý nghĩa vì đã cố tình dùng text lower để detect*/ = TEXT_NODE_CAPITAL_UNKNOWN;
					int									combiningTone = 0;
					qvsylidentifier						leftMatchingVietnameseIdentifier = 0;
					qewrdidentifier						leftMatchingEnglishIdentifier = 0;
					int									leftMatchingCombiningTone = 0;
					qvsylidentifier						vietnameseSyllableIdentifier = VietnameseSyllableDetection(textLower, &tempCapital, &combiningTone, &leftMatchingVietnameseIdentifier, &leftMatchingCombiningTone);
					struct VIETNAMESE_SYLLABLE const* vietnameseSyllableNode = vnsyllables + vietnameseSyllableIdentifier;
					qvwrdidentifier						englishWordIdentifier = (vietnameseSyllableIdentifier == 0 ? EnglishWordDetection(textLower, &tempCapital, &leftMatchingEnglishIdentifier) : vietnameseSyllableNode->english);
					textNode->vietnameseLoanWordIndentifier = LoanWordDetection(textLower, &tempCapital);
					textNode->vietnameseAbbreviationIndentifier = LowerAbbreviationDetection(textLower);
					textNode->japaneseWordIdentifier = JapaneseRomanjiWordDetection(textLower, &tempCapital);
					qvmissingsylidentifier				vietnameseMissingIndentifiler = (vietnameseSyllableIdentifier == 0 && englishWordIdentifier == 0 ? MissingEndDetection(textLower) : 0);
					if (vietnameseSyllableIdentifier)
					{
						textNode->vietnameseSyllableIdentifier = vietnameseSyllableIdentifier;
						switch (textNode->capital)
						{
						case TEXT_NODE_CAPITAL_LOWER:
							textNode->text = vietnameseSyllableNode->lower;
							textNode->textLength = vietnameseSyllableNode->length;
							flagNeedStoreTextMemory = false;
							break;

						case TEXT_NODE_CAPITAL_UPPER:
							textNode->text = vietnameseSyllableNode->upper;
							textNode->textLength = vietnameseSyllableNode->length;
							flagNeedStoreTextMemory = false;
							break;

						case TEXT_NODE_CAPITAL_CAPITAL:
							textNode->text = vietnameseSyllableNode->capital;
							textNode->textLength = vietnameseSyllableNode->length;
							flagNeedStoreTextMemory = false;
							break;

						default:
							textNode->text = text;
							textNode->textLength = textLength;
							flagNeedStoreTextMemory = true;
							break;
						}
						if (textErrorCombiningTone != 0 && vietnameseSyllableNode->tone == 0x30/*0*/) flagNeedStoreCombiningTone = true;
						flagNeedMoreChecking = false;
						UpdateVietnameseTextNodeContext(textNode);
					}
					else if (englishWordIdentifier)
					{
						textNode->englishWordIdentifier = englishWordIdentifier;
						textNode->textNodeType = TEXT_NODE_TYPE_ENGLISH_WORD;
						textNode->text = text;
						textNode->textLength = textLength;
						flagNeedStoreTextMemory = true;
						flagNeedMoreChecking = false;
						UpdateVietnameseTextNodeContext(textNode);
					}
					else if (textNode->vietnameseLoanWordIndentifier > 0 || textNode->vietnameseAbbreviationIndentifier > 0 || textNode->japaneseWordIdentifier > 0)
					{
						UpdateVietnameseTextNodeContext(textNode);
						flagNeedMoreChecking = false;
					}
					else if (vietnameseMissingIndentifiler)
					{ // bắt buộc chỉ chọn cái có kết nối với đằng sau, nếu ko phải sang bước tiếp
						double				maxCoefficient = 0.0;
						qvsylidentifier		maxCoefficientIdentifier = 0;
						int					maxIsJoinWithNextNode = 0;
						int					countTotalJoinWithNextNode = 0;
						int					countSureWay = 0;
						for (int iway = 0; iway < vnmissingends[vietnameseMissingIndentifiler].length; iway++)
						{
							TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* leftTextNodeOffset5 = &nullTextNodeForStep2Normalize;
							if (textNode->back)
							{
								leftTextNodeOffset0 = textNode->back;
								if (leftTextNodeOffset0->back)
								{
									leftTextNodeOffset1 = leftTextNodeOffset0->back;
									if (leftTextNodeOffset1->back)
									{
										leftTextNodeOffset2 = leftTextNodeOffset1->back;
										if (leftTextNodeOffset2->back)
										{
											leftTextNodeOffset3 = leftTextNodeOffset2->back;
											if (leftTextNodeOffset3->back)
											{
												leftTextNodeOffset4 = leftTextNodeOffset3->back;
												if (leftTextNodeOffset4->back)
												{
													leftTextNodeOffset5 = leftTextNodeOffset4->back;
												}
											}
										}
									}
								}
							}
							TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
							TEXT_NODE* rightTextNodeOffset5 = &nullTextNodeForStep2Normalize;
							if (textNode->next)
							{
								rightTextNodeOffset0 = textNode->next;
								if (rightTextNodeOffset0->next)
								{
									rightTextNodeOffset1 = rightTextNodeOffset0->next;
									if (rightTextNodeOffset1->next)
									{
										rightTextNodeOffset2 = rightTextNodeOffset1->next;
										if (rightTextNodeOffset2->next)
										{
											rightTextNodeOffset3 = rightTextNodeOffset2->next;
											if (rightTextNodeOffset3->next)
											{
												rightTextNodeOffset4 = rightTextNodeOffset3->next;
												if (rightTextNodeOffset4->next)
												{
													rightTextNodeOffset5 = rightTextNodeOffset4->next;
												}
											}
										}
									}
								}
							}
							qvsylidentifier		currentVnSyllableIdentifier = vnmissingends[vietnameseMissingIndentifiler].destinations[iway];
							bool				flagCurrentNodeJoinWithNextNode = false;


							int currentSyllableSure =
								/************************************************************************/
								/* Right Sure                                                           */
								/************************************************************************/
								vnsyllables[currentVnSyllableIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier)
								+ (vnsyllables[leftTextNodeOffset0->vietnameseSyllableIdentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 1)
								+ (vnabbreviations[leftTextNodeOffset0->vietnameseAbbreviationIndentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier) > 1)
								+ (vnabbreviations[leftTextNodeOffset1->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier) > 2)
								+ (vnabbreviations[leftTextNodeOffset2->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier) > 3)
								+ (vnabbreviations[leftTextNodeOffset3->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier) > 4)
								+ (vnabbreviations[leftTextNodeOffset4->vietnameseAbbreviationIndentifier].RightSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 5)

								+ (vnwords[leftTextNodeOffset1->vietnameseWordIdentifier].RightSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1)
								+ vnwords[leftTextNodeOffset0->vietnameseWordIdentifier].RightSure(currentVnSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier)
								/************************************************************************/
								/* Left Sure                                                            */
								/************************************************************************/
								+vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier)
								+ (vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1)
								+ vnabbreviations[rightTextNodeOffset0->vietnameseAbbreviationIndentifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier)
								+ (vnabbreviations[rightTextNodeOffset1->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier) > 1)
								+ (vnabbreviations[rightTextNodeOffset2->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier) > 2)
								+ (vnabbreviations[rightTextNodeOffset3->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier) > 3)
								+ (vnabbreviations[rightTextNodeOffset4->vietnameseAbbreviationIndentifier].LeftSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 4)
								;

							//------current node is word
							int currentWordIdentifier = vnsyllables[currentVnSyllableIdentifier].DetectWord(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier);
							int currentWordLength = vnwords[currentWordIdentifier].length;
							int currentWordRightSure = vnwords[currentWordIdentifier].RightSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier);
							int currentWordLeftSure = 0;
							switch (currentWordLength)
							{
							case 2: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
							case 3: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
							case 4: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier); break;
							case 5: currentWordLeftSure = vnwords[currentWordIdentifier].LeftSure(leftTextNodeOffset4->vietnameseSyllableIdentifier, leftTextNodeOffset5->vietnameseSyllableIdentifier); break;
							}
							if (currentWordRightSure) flagCurrentNodeJoinWithNextNode = true;


							//------rightTextNodeOffset0 is word
							int rightWordOffSet0Indentifier = vnsyllables[rightTextNodeOffset0->vietnameseSyllableIdentifier].DetectWord(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
							int rightWordOffSet0Length = vnwords[rightWordOffSet0Indentifier].length;
							int rightWordOffSet0RightSure = vnwords[rightWordOffSet0Indentifier].RightSure(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier);
							int rightWordOffSet0LeftSure = 0;
							switch (rightWordOffSet0Length)
							{
							case 2: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
							case 3: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
							case 4: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
							case 5: rightWordOffSet0LeftSure = vnwords[rightWordOffSet0Indentifier].LeftSure(leftTextNodeOffset3->vietnameseSyllableIdentifier, leftTextNodeOffset4->vietnameseSyllableIdentifier); break;
							}
							if (rightWordOffSet0Length > 1) flagCurrentNodeJoinWithNextNode = true;


							//------rightTextNodeOffset1 is word
							int rightWordOffSet1Identifier = vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
							int rightWordOffSet1Length = vnwords[rightWordOffSet1Identifier].length;
							int rightWordOffSet1RightSure = vnwords[rightWordOffSet1Identifier].RightSure(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset3->vietnameseSyllableIdentifier);
							int rightWordOffSet1LeftSure = 0;
							switch (rightWordOffSet1Length)
							{
							case 2:
								rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
								rightWordOffSet1Identifier = 0;
								rightWordOffSet1Length = 0;
								rightWordOffSet1RightSure = 0;
								break;
							case 3: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
							case 4: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
							case 5: rightWordOffSet1LeftSure = vnwords[rightWordOffSet1Identifier].LeftSure(leftTextNodeOffset2->vietnameseSyllableIdentifier, leftTextNodeOffset3->vietnameseSyllableIdentifier); break;
							}
							if (rightWordOffSet1Length > 2) flagCurrentNodeJoinWithNextNode = true;


							//------rightTextNodeOffset2 is word
							int rightWordOffSet2Identifier = vnsyllables[rightTextNodeOffset2->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier);
							int rightWordOffSet2Length = vnwords[rightWordOffSet2Identifier].length;
							int rightWordOffSet2RightSure = vnwords[rightWordOffSet2Identifier].RightSure(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset4->vietnameseSyllableIdentifier);
							int rightWordOffSet2LeftSure = 0;
							switch (rightWordOffSet2Length)
							{
							case 2:
								rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1;
								rightWordOffSet2Identifier = 0;
								rightWordOffSet2Length = 0;
								rightWordOffSet2RightSure = 0;
								break;
							case 3:
								rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(currentVnSyllableIdentifier, leftTextNodeOffset0->vietnameseSyllableIdentifier);
								rightWordOffSet2Identifier = 0;
								rightWordOffSet2Length = 0;
								rightWordOffSet2RightSure = 0;
								break;
							case 4: rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier); break;
							case 5: rightWordOffSet2LeftSure = vnwords[rightWordOffSet2Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
							}
							if (rightWordOffSet2Length > 3) flagCurrentNodeJoinWithNextNode = true;


							//------rightTextNodeOffset3 is word
							int rightWordOffSet3Identifier = vnsyllables[rightTextNodeOffset3->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier);
							int rightWordOffSet3Length = vnwords[rightWordOffSet3Identifier].length;
							int rightWordOffSet3RightSure = vnwords[rightWordOffSet3Identifier].RightSure(rightTextNodeOffset4->vietnameseSyllableIdentifier, rightTextNodeOffset5->vietnameseSyllableIdentifier);
							int rightWordOffSet3LeftSure = 0;
							switch (rightWordOffSet3Length)
							{
							case 2:
								rightWordOffSet3LeftSure = 0;
								rightWordOffSet3Identifier = 0;
								rightWordOffSet3Length = 0;
								rightWordOffSet3RightSure = 0;
								break;
							case 3:
								rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(rightTextNodeOffset0->vietnameseSyllableIdentifier, currentVnSyllableIdentifier) > 1;
								rightWordOffSet3Identifier = 0;
								rightWordOffSet3Length = 0;
								rightWordOffSet3RightSure = 0;
								break;
							case 4:
								rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset0->vietnameseSyllableIdentifier, leftTextNodeOffset1->vietnameseSyllableIdentifier);
								rightWordOffSet3Identifier = 0;
								rightWordOffSet3Length = 0;
								rightWordOffSet3RightSure = 0;
								break;
							case 5: rightWordOffSet3LeftSure = vnwords[rightWordOffSet3Identifier].LeftSure(leftTextNodeOffset1->vietnameseSyllableIdentifier, leftTextNodeOffset2->vietnameseSyllableIdentifier); break;
							}
							if (rightWordOffSet3Length > 4) flagCurrentNodeJoinWithNextNode = true;


							if (currentSyllableSure) flagCurrentNodeJoinWithNextNode = true;
							double currentCoefficient = vnsyllables[currentVnSyllableIdentifier].coefficient
								+ currentSyllableSure
								+ vnwords[currentWordIdentifier].coefficient
								+ currentWordLeftSure
								+ currentWordRightSure
								+ vnwords[rightWordOffSet0Indentifier].coefficient
								+ rightWordOffSet0LeftSure
								+ rightWordOffSet0RightSure
								+ vnwords[rightWordOffSet1Identifier].coefficient
								+ rightWordOffSet1LeftSure
								+ rightWordOffSet1RightSure
								+ vnwords[rightWordOffSet2Identifier].coefficient
								+ rightWordOffSet2LeftSure
								+ rightWordOffSet2RightSure
								+ vnwords[rightWordOffSet3Identifier].coefficient
								+ rightWordOffSet3LeftSure
								+ rightWordOffSet3RightSure;

							if (currentCoefficient > maxCoefficient)
							{
								maxCoefficient = currentCoefficient;
								maxCoefficientIdentifier = currentVnSyllableIdentifier;
								maxIsJoinWithNextNode = flagCurrentNodeJoinWithNextNode;
							}
							if (flagCurrentNodeJoinWithNextNode) countTotalJoinWithNextNode++;

							if (currentSyllableSure
								|| currentWordIdentifier || currentWordLeftSure || currentWordRightSure
								|| rightWordOffSet0Indentifier || rightWordOffSet0LeftSure || rightWordOffSet0RightSure
								|| rightWordOffSet1Identifier || rightWordOffSet1LeftSure || rightWordOffSet1RightSure
								|| rightWordOffSet2Identifier || rightWordOffSet2LeftSure || rightWordOffSet2RightSure
								)
							{
								countSureWay++;
							}
						}
						if (maxCoefficientIdentifier && (maxIsJoinWithNextNode || textNode->next == NULL))
						{
							textNode->vietnameseSyllableIdentifier = maxCoefficientIdentifier;
							textNode->textNodeType = TEXT_NODE_TYPE_VIETNAMESE_SYLLABLE;
							if (countSureWay == 1 && countTotalJoinWithNextNode == 1)
							{
								switch (textNode->capital)
								{
								case TEXT_NODE_CAPITAL_LOWER:
									textNode->text = vnsyllables[maxCoefficientIdentifier].lower;
									textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
									flagNeedStoreTextMemory = false;
									break;
								case TEXT_NODE_CAPITAL_UPPER:
									textNode->text = vnsyllables[maxCoefficientIdentifier].upper;
									textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
									flagNeedStoreTextMemory = false;
									break;
								case TEXT_NODE_CAPITAL_CAPITAL:
									textNode->text = vnsyllables[maxCoefficientIdentifier].capital;
									textNode->textLength = vnsyllables[maxCoefficientIdentifier].length;
									flagNeedStoreTextMemory = false;
									break;
								default:
									textNode->text = text;
									textNode->textLength = textLength;
									flagNeedStoreTextMemory = true;
									break;
								}
							}
							if (textErrorCombiningTone != 0 && vnsyllables[maxCoefficientIdentifier].tone == 0x30/*0*/) flagNeedStoreCombiningTone = true;
							flagNeedMoreChecking = false;
							countTotalUnknownNode--;
							UpdateVietnameseTextNodeContext(textNode);
						}
						else
						{
							textNode->vietnameseMissingIndentifiler = vietnameseMissingIndentifiler;
						}
					}
				}
				if (flagNeedMoreChecking && textLength == 1)
				{
					switch (text[0])
					{
					case 0x25/*%*/:
						textNode->textNodeType = TEXT_NODE_TYPE_PERCENT_SIGN;
						flagNeedMoreChecking = false;
						flagNeedStoreCombiningTone = false;
						break;
					case 0x2030/*‰*/:
						textNode->textNodeType = TEXT_NODE_TYPE_PERMILLE_SIGN;
						flagNeedMoreChecking = false;
						flagNeedStoreCombiningTone = false;
						break;
					case 0x2031/*‱*/:
						textNode->textNodeType = TEXT_NODE_TYPE_PERMYRIAD_SIGN;
						flagNeedMoreChecking = false;
						flagNeedStoreCombiningTone = false;
						break;
					}
				}
				/*------------------------   Kết thúc bước 3 -------------------------*/


				/************************************************************************/
				/* Bước 4 : Lưu lại thông tin cho bước sau                              */
				/************************************************************************/
				if (flagNeedMoreChecking || flagNeedStoreTextMemory || flagNeedStoreCombiningTone)
				{
					textNode->extra = (TEXT_NODE_EXTRA*)qcalloc(1, sizeof(TEXT_NODE_EXTRA));
					if (textNode->extra)
					{
						if (flagNeedStoreTextMemory)
						{
							textNode->extra->memText = textMemory;
							textNode->text = text;
							textNode->textLength = textLength;
						}
						if (flagNeedMoreChecking)
						{
							//lưu text lower, text type
							if (textCountUpperCharacter > 0)
							{
								textNode->extra->textLower = textLower;
								textNode->extra->memTextLower = textLowerMemory;
							}
							else
							{
								textNode->extra->textLower = textNode->text;
							}
							textNode->extra->textType = textType;
							textNode->extra->memTextType = textTypeMemory;
						}
						/*Alphabet character*/
						textNode->extra->textCountTotalAlphabet = textCountTotalAlphabet;

						/*Upper - Lower Character*/
						textNode->extra->textCountLowerCharacter = textCountLowerCharacter;
						textNode->extra->textCountUpperCharacter = textCountUpperCharacter;
						/*Vowel - Consonant Character*/
						textNode->extra->textCountVowelCharacter = textCountVowelCharacter;
						textNode->extra->textCountConsonantCharacter = textCountConsonantCharacter;

						/*Vietnamese*/
						textNode->extra->textCountVietnameseOnlyCharacter = textCountVietnameseOnlyCharacter;	//Vietnamese only

						/*English*/
						textNode->extra->textCountEnglishVowelCharacter = textCountEnglishVowelCharacter;
						textNode->extra->textCountEnglishConsonantCharacter = textCountEnglishConsonantCharacter;
						/*Number*/
						textNode->extra->textCountNumberCharacter = textCountNumberCharacter;
						//textNode->extra->textCountZeroCharacter = textCountZeroCharacter;

						/*Other*/
						textNode->extra->textCountOtherOnKeyBoard = textCountOtherOnKeyBoard;
						textNode->extra->textCountDotCharacter = textCountDotCharacter;
						textNode->extra->textCountCommaCharacter = textCountCommaCharacter;
						textNode->extra->textCountSlashCharacter = textCountSlashCharacter;
						/*Unknown Character*/
						textNode->extra->textCountUnknownCharacter = textCountUnknownCharacter;
					}
					else
					{
						printf("\nVietnameseTextNormalizer : Calloc fail!");
						DungManHinh;
					}
				}
				//free
				if (textNode->extra)
				{
					if (textNode->extra->memText == 0) qfree(textMemory);
					if (textNode->extra->memTextLower == 0) qfree(textLowerMemory);
					if (textNode->extra->memTextType == 0) qfree(textTypeMemory);
				}
				else
				{
					if (textMemory /*!=NULL*/)	qfree(textMemory);
					if (textLowerMemory /*!= NULL*/) qfree(textLowerMemory);
					if (textTypeMemory /*!= NULL*/) qfree(textTypeMemory);
				}
				if (textIndexMemory /*!= NULL*/) qfree(textIndexMemory);
				/*------------------------   Kết thúc bước 4 -------------------------*/
			}
			else
			{
				if (textMemory /*!=NULL*/)	qfree(textMemory);
				if (textLowerMemory /*!= NULL*/) qfree(textLowerMemory);
				if (textTypeMemory /*!= NULL*/) qfree(textTypeMemory);
				if (textIndexMemory /*!= NULL*/) qfree(textIndexMemory);
				printf("\nVietnameseTextNormalizer : Calloc fail!");
				DungManHinh;
			}
		}//kết thúc của nếu cần xử ở bước này
		 //Go to Next
		if (textNode->textLength == 0 || textNode->originalTextLength > limitCharacterLength)
		{//remove if null or very long
			if (textNode == head)
			{
				TEXT_NODE* nextTextNode = textNode->next;
				if (uhead == textNode)  uhead = nextTextNode;
				if (utail == textNode)  utail = nextTextNode;
				/*if ( head == textNode)*/  head = nextTextNode;
				if (tail == textNode)  tail = nextTextNode;
				if (nextTextNode) nextTextNode->back = NULL;
				if (textNode->extra /*!= NULL*/)
				{
					if (textNode->extra->memText) qfree(textNode->extra->memText);
					if (textNode->extra->memTextLower) qfree(textNode->extra->memTextLower);
					if (textNode->extra->memTextType) qfree(textNode->extra->memTextType);
					if (textNode->extra->vsyllables) qfree(textNode->extra->vsyllables);
					qfree(textNode->extra);
				}
				qfree(textNode);
				textNode = nextTextNode;
			}
			else if (textNode == tail)
			{
				TEXT_NODE* lastTextNode = textNode->back;
				if (uhead == textNode)  uhead = lastTextNode;
				if (utail == textNode)  utail = lastTextNode;
				if (head == textNode)  head = lastTextNode;
				/*if ( tail == textNode)*/  tail = lastTextNode;
				if (lastTextNode) lastTextNode->next = NULL;
				if (textNode->extra /*!= NULL*/)
				{
					if (textNode->extra->memText) qfree(textNode->extra->memText);
					if (textNode->extra->memTextLower) qfree(textNode->extra->memTextLower);
					if (textNode->extra->memTextType) qfree(textNode->extra->memTextType);
					if (textNode->extra->vsyllables) qfree(textNode->extra->vsyllables);
					qfree(textNode->extra);
				}
				qfree(textNode);
				textNode = NULL;
			}
			else
			{
				TEXT_NODE* nextTextNode = textNode->next;
				TEXT_NODE* lastTextNode = textNode->back;
				if (uhead == textNode)  uhead = lastTextNode;
				if (utail == textNode)  utail = nextTextNode;
				lastTextNode->next = nextTextNode;
				nextTextNode->back = lastTextNode;
				if (textNode->extra /*!= NULL*/)
				{
					if (textNode->extra->memText) qfree(textNode->extra->memText);
					if (textNode->extra->memTextLower) qfree(textNode->extra->memTextLower);
					if (textNode->extra->memTextType) qfree(textNode->extra->memTextType);
					if (textNode->extra->vsyllables) qfree(textNode->extra->vsyllables);
					qfree(textNode->extra);
				}
				qfree(textNode);
				textNode = nextTextNode;
			}
			countTotalUnknownNode--;
			countTotalNode--;
		}
		else
		{
			/* Go to next node */
			if (textNode == utail) textNode = NULL;
			else textNode = textNode->next;
		}
	}//kết thúc của vòng lặp từ đầu danh sách đến cuối  for ....

	/************************************************************************/
	/* Update Node                                                           */
	/************************************************************************/
	for (TEXT_NODE* textNode = head; textNode/*!=NULL*/; textNode = textNode->next)
	{
		TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
		TEXT_NODE* leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
		if (textNode->back)
		{
			leftTextNodeOffset0 = textNode->back;
			if (leftTextNodeOffset0->back)
			{
				leftTextNodeOffset1 = leftTextNodeOffset0->back;
				if (leftTextNodeOffset1->back)
				{
					leftTextNodeOffset2 = leftTextNodeOffset1->back;
					if (leftTextNodeOffset2->back)
					{
						leftTextNodeOffset3 = leftTextNodeOffset2->back;
						if (leftTextNodeOffset3->back)
						{
							leftTextNodeOffset4 = leftTextNodeOffset3->back;
						}
					}
				}
			}
		}
		UpdateVietnameseTextNodeContext(textNode, leftTextNodeOffset0, leftTextNodeOffset1, leftTextNodeOffset2, leftTextNodeOffset3, leftTextNodeOffset4);
	}




	/************************************************************************/
	/* Correct                                                              */
	/************************************************************************/
	for (TEXT_NODE* textNode = head; textNode/*!=NULL*/; textNode = textNode->next)
	{
		if (flagValidateToolMode == false && textNode->changeable != TEXT_NODE_CAN_NOT_CHANGE && ((textNode->vietnameseSyllableIdentifier > 0
			&& textNode->englishWordIdentifier == 0
			&& textNode->vietnameseAbbreviationIndentifier == 0
			&& textNode->vietnameseLoanWordIndentifier == 0
			&& textNode->vietnameseMissingIndentifiler == 0
			&& (vnsyllables[textNode->vietnameseSyllableIdentifier].significant == 0)
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].correctLength > 0
			)
			|| (textNode->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_K_O)))
		{
			int					countTotalSureWay = 0;
			double				maxWayScore = 0.0;
			qvsylidentifier		maxWayIdentifier = 0;
			for (int iway = 0; iway < vnsyllables[textNode->vietnameseSyllableIdentifier].correctLength; iway++)
			{
				qvsylidentifier currentVietnameseSyllableIdentifier = vnsyllables[textNode->vietnameseSyllableIdentifier].correct[iway];
				double currentScore = SignificantScore(textNode, currentVietnameseSyllableIdentifier) + PerplexityScore(textNode, currentVietnameseSyllableIdentifier);
				if (currentScore > 0)
				{
					countTotalSureWay++;
					if (maxWayScore < currentScore)
					{
						maxWayScore = currentScore;
						maxWayIdentifier = currentVietnameseSyllableIdentifier;
					}
				}
			}
			if (maxWayScore > 0 && maxWayIdentifier > 0)
			{
				textNode->vietnameseSyllableIdentifier = maxWayIdentifier;
				if (countTotalSureWay == 1)
				{
					switch (textNode->capital)
					{
					case TEXT_NODE_CAPITAL_LOWER:
						textNode->text = vnsyllables[maxWayIdentifier].lower;
						textNode->textLength = vnsyllables[maxWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_UPPER:
						textNode->text = vnsyllables[maxWayIdentifier].upper;
						textNode->textLength = vnsyllables[maxWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_CAPITAL:
						textNode->text = vnsyllables[maxWayIdentifier].capital;
						textNode->textLength = vnsyllables[maxWayIdentifier].length;
						break;
					default:
						/*do not change any-thing*/
						break;
					}
					UpdateVietnameseTextNodeContext(textNode);
				}
			}
		}


		if (textNode->vietnameseSyllableIdentifier > 0
			&& textNode->englishWordIdentifier == 0
			&& textNode->vietnameseAbbreviationIndentifier == 0
			&& textNode->vietnameseLoanWordIndentifier == 0
			&& textNode->vietnameseMissingIndentifiler == 0
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier
			&& vnsyllables[vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier].significant)
		{
			qvsylidentifier otherWayIdentifier = vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier;
			double currentSure = SignificantScore(textNode, textNode->vietnameseSyllableIdentifier) + PerplexityScore(textNode, textNode->vietnameseSyllableIdentifier);
			double otherSure = SignificantScore(textNode, otherWayIdentifier) + PerplexityScore(textNode, otherWayIdentifier);
			if (currentSure == 0.0 && otherSure > 0.0)
			{
				textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
				switch (textNode->capital)
				{
				case TEXT_NODE_CAPITAL_LOWER:
					textNode->text = vnsyllables[otherWayIdentifier].lower;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_UPPER:
					textNode->text = vnsyllables[otherWayIdentifier].upper;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_CAPITAL:
					textNode->text = vnsyllables[otherWayIdentifier].capital;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				default:
					/*do not change any-thing*/
					break;
				}
				UpdateVietnameseTextNodeContext(textNode);
			}
			else if (flagStandardTextForTTS)
			{
				if (currentSure > 0.0 && otherSure > 0.0 && currentSure < otherSure)
				{
					textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
					switch (textNode->capital)
					{
					case TEXT_NODE_CAPITAL_LOWER:
						textNode->text = vnsyllables[otherWayIdentifier].lower;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_UPPER:
						textNode->text = vnsyllables[otherWayIdentifier].upper;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_CAPITAL:
						textNode->text = vnsyllables[otherWayIdentifier].capital;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					default:
						/*do not change any-thing*/
						break;
					}
					UpdateVietnameseTextNodeContext(textNode);
				}
				else if (currentSure == otherSure && vnsyllables[textNode->vietnameseSyllableIdentifier].coefficient < vnsyllables[otherWayIdentifier].coefficient)
				{
					textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
					switch (textNode->capital)
					{
					case TEXT_NODE_CAPITAL_LOWER:
						textNode->text = vnsyllables[otherWayIdentifier].lower;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_UPPER:
						textNode->text = vnsyllables[otherWayIdentifier].upper;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					case TEXT_NODE_CAPITAL_CAPITAL:
						textNode->text = vnsyllables[otherWayIdentifier].capital;
						textNode->textLength = vnsyllables[otherWayIdentifier].length;
						break;
					default:
						/*do not change any-thing*/
						break;
					}
					UpdateVietnameseTextNodeContext(textNode);
				}
			}

		}


		/************************************************************************/
		/* Bị dính liền theo lỗi typing                                         */
		/************************************************************************/
		if (textNode->next
			&& textNode->originalText
			&& textNode->next->originalText
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].vietnamese
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].significant
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].tone > '0'
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].vietnamese
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].significant
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].tone > '0'
			&& textNode->next->capital == TEXT_NODE_CAPITAL_LOWER
			&& textNode->capital == TEXT_NODE_CAPITAL_LOWER
			&& textNode->originalText + textNode->originalTextLength == textNode->next->originalText)
		{
			textNode->needSpaceAfter = 1;
		}
		if (flagStandardTextForTTS
			&& textNode->next
			&& textNode->originalText
			&& textNode->next->originalText
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].vietnamese
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].significant
			&& vnsyllables[textNode->vietnameseSyllableIdentifier].tone > '0'
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].vietnamese
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].significant
			&& vnsyllables[textNode->next->vietnameseSyllableIdentifier].tone > '0'
			&& textNode->originalText + textNode->originalTextLength == textNode->next->originalText)
		{
			textNode->needSpaceAfter = 1;
		}
		/************************************************************************/
		/* Convert Y - I                                                        */
		/************************************************************************/
		//			if (textNode->vietnameseSyllableIdentifier > 0
		//				&& textNode->englishWordIdentifier == 0
		//				&& textNode->vietnameseAbbreviationIndentifier == 0
		//				&& textNode->vietnameseLoanWordIndentifier == 0
		//				&& textNode->vietnameseMissingIndentifiler == 0
		//				&& vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier
		//				&& vnsyllables[vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier].significant
		//				&& vnsyllables[textNode->vietnameseSyllableIdentifier].significant
		//				&& vnsyllables[textNode->vietnameseSyllableIdentifier].length > 1
		//				&& vnsyllables[textNode->vietnameseSyllableIdentifier].otherisi)
		//			{
		//				qvsylidentifier otherWayIdentifier = vnsyllables[textNode->vietnameseSyllableIdentifier].iyidentifier;
		//				textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
		//				switch (textNode->capital)
		//				{
		//				case TEXT_NODE_CAPITAL_LOWER:
		//					textNode->text = vnsyllables[otherWayIdentifier].lower;
		//					textNode->textLength = vnsyllables[otherWayIdentifier].length;
		//					break;
		//				case TEXT_NODE_CAPITAL_UPPER:
		//					textNode->text = vnsyllables[otherWayIdentifier].upper;
		//					textNode->textLength = vnsyllables[otherWayIdentifier].length;
		//					break;
		//				case TEXT_NODE_CAPITAL_CAPITAL:
		//					textNode->text = vnsyllables[otherWayIdentifier].capital;
		//					textNode->textLength = vnsyllables[otherWayIdentifier].length;
		//					break;
		//				default:
		//					/*do not change any-thing*/
		//					break;
		//				}
		//				UpdateVietnameseTextNodeContext(textNode);
		//			}


		/************************************************************************/
		/* Một vài case đặc biệt                                                */
		/************************************************************************/
		if (flagValidateToolMode == false
			&& textNode->changeable != TEXT_NODE_CAN_NOT_CHANGE
			&& textNode->vietnameseSyllableIdentifier > 0
			&& textNode->englishWordIdentifier == 0
			&& textNode->vietnameseAbbreviationIndentifier == 0
			&& textNode->vietnameseLoanWordIndentifier == 0
			&& textNode->vietnameseMissingIndentifiler == 0
			&& SignificantScore(textNode, textNode->vietnameseSyllableIdentifier) == 0
			)
		{
			TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* leftTextNodeOffset3 = &nullTextNodeForStep2Normalize;
			//TEXT_NODE *				leftTextNodeOffset4 = &nullTextNodeForStep2Normalize;
			if (textNode->back)
			{
				leftTextNodeOffset0 = textNode->back;
				if (leftTextNodeOffset0->back)
				{
					leftTextNodeOffset1 = leftTextNodeOffset0->back;
					if (leftTextNodeOffset1->back)
					{
						leftTextNodeOffset2 = leftTextNodeOffset1->back;
						if (leftTextNodeOffset2->back)
						{
							leftTextNodeOffset3 = leftTextNodeOffset2->back;
							//if (leftTextNodeOffset3->back)
							//{
							//	leftTextNodeOffset4 = leftTextNodeOffset3->back;
							//}
						}
					}
				}
			}
			TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
			if (textNode->next)
			{
				rightTextNodeOffset0 = textNode->next;
				if (rightTextNodeOffset0->next)
				{
					rightTextNodeOffset1 = rightTextNodeOffset0->next;
					if (rightTextNodeOffset1->next)
					{
						rightTextNodeOffset2 = rightTextNodeOffset1->next;
						if (rightTextNodeOffset2->next)
						{
							rightTextNodeOffset3 = rightTextNodeOffset2->next;
							if (rightTextNodeOffset3->next)
							{
								rightTextNodeOffset4 = rightTextNodeOffset3->next;
							}
						}
					}
				}
			}




			/* .. vô hình chung .. -> .. vô hình trung .. */
			if (textNode->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_C_H_U_N_G
				&& leftTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_H_IF_N_H
				&& leftTextNodeOffset1->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_V_OO
				)
			{
				qvsylidentifier otherWayIdentifier = VIETNAMESE_SYLLABLE_T_R_U_N_G;
				textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
				switch (textNode->capital)
				{
				case TEXT_NODE_CAPITAL_LOWER:
					textNode->text = vnsyllables[otherWayIdentifier].lower;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_UPPER:
					textNode->text = vnsyllables[otherWayIdentifier].upper;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_CAPITAL:
					textNode->text = vnsyllables[otherWayIdentifier].capital;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				default:
					/*do not change any-thing*/
					break;
				}
				UpdateVietnameseTextNodeContext(textNode);
			}


			/* .. chuẩn đoán {bệnh, mắc,  bị bệnh, ung thư, phát hiện bệnh, phát hiện ung thư} .. -> .. chẩn đoán .. */
			if (textNode->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_C_H_U_AAR_N
				&& rightTextNodeOffset0->vietnameseSyllableIdentifier == VIETNAMESE_SYLLABLE_DD_O_AS_N
				&& SignificantScore(textNode, VIETNAMESE_SYLLABLE_C_H_AAR_N) > 0)
			{
				qvsylidentifier otherWayIdentifier = VIETNAMESE_SYLLABLE_C_H_AAR_N;
				textNode->vietnameseSyllableIdentifier = otherWayIdentifier;
				switch (textNode->capital)
				{
				case TEXT_NODE_CAPITAL_LOWER:
					textNode->text = vnsyllables[otherWayIdentifier].lower;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_UPPER:
					textNode->text = vnsyllables[otherWayIdentifier].upper;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				case TEXT_NODE_CAPITAL_CAPITAL:
					textNode->text = vnsyllables[otherWayIdentifier].capital;
					textNode->textLength = vnsyllables[otherWayIdentifier].length;
					break;
				default:
					/*do not change any-thing*/
					break;
				}
				UpdateVietnameseTextNodeContext(textNode);
			}





		}

	}



	/************************************************************************/
	/* Word segment                                                         */
	/************************************************************************/
#ifndef ON_SCAN_WORD_FREQUENCY
	for (TEXT_NODE* textNode = head; textNode/*!=NULL*/; textNode = textNode->next)
	{
		if (textNode->vietnameseWordIdentifier)
		{
			TEXT_NODE* rightTextNodeOffset0 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset1 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset2 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset3 = &nullTextNodeForStep2Normalize;
			TEXT_NODE* rightTextNodeOffset4 = &nullTextNodeForStep2Normalize;
			if (textNode->next)
			{
				rightTextNodeOffset0 = textNode->next;
				if (rightTextNodeOffset0->next)
				{
					rightTextNodeOffset1 = rightTextNodeOffset0->next;
					if (rightTextNodeOffset1->next)
					{
						rightTextNodeOffset2 = rightTextNodeOffset1->next;
						if (rightTextNodeOffset2->next)
						{
							rightTextNodeOffset3 = rightTextNodeOffset2->next;
							if (rightTextNodeOffset3->next)
							{
								rightTextNodeOffset4 = rightTextNodeOffset3->next;
							}
						}
					}
				}
			}
			double currentWordPoint = (textNode->leftVietnameseWordSure + textNode->rightVietnameseWordSure + 1) * vnwords[textNode->vietnameseWordIdentifier].coefficient;
			double rightO0WordPoint = (vnwords[rightTextNodeOffset0->vietnameseWordIdentifier].length > 1) * ((rightTextNodeOffset0->leftVietnameseWordSure + rightTextNodeOffset0->rightVietnameseWordSure + 1) * vnwords[rightTextNodeOffset0->vietnameseWordIdentifier].coefficient);
			double rightO1WordPoint = (vnwords[rightTextNodeOffset1->vietnameseWordIdentifier].length > 2) * ((rightTextNodeOffset1->leftVietnameseWordSure + rightTextNodeOffset1->rightVietnameseWordSure + 1) * vnwords[rightTextNodeOffset1->vietnameseWordIdentifier].coefficient);
			double rightO2WordPoint = (vnwords[rightTextNodeOffset2->vietnameseWordIdentifier].length > 3) * ((rightTextNodeOffset2->leftVietnameseWordSure + rightTextNodeOffset2->rightVietnameseWordSure + 1) * vnwords[rightTextNodeOffset2->vietnameseWordIdentifier].coefficient);
			double rightO3WordPoint = (vnwords[rightTextNodeOffset3->vietnameseWordIdentifier].length > 4) * ((rightTextNodeOffset3->leftVietnameseWordSure + rightTextNodeOffset3->rightVietnameseWordSure + 1) * vnwords[rightTextNodeOffset3->vietnameseWordIdentifier].coefficient);
			double rightO4WordPoint = (vnwords[rightTextNodeOffset4->vietnameseWordIdentifier].length > 5) * ((rightTextNodeOffset4->leftVietnameseWordSure + rightTextNodeOffset4->rightVietnameseWordSure + 1) * vnwords[rightTextNodeOffset4->vietnameseWordIdentifier].coefficient);


			if (rightTextNodeOffset0->vietnameseWordIdentifier && vnwords[rightTextNodeOffset0->vietnameseWordIdentifier].length > 1)
			{
				if (currentWordPoint > rightO0WordPoint) rightTextNodeOffset0->vietnameseWordIdentifier = 0;
				else textNode->vietnameseWordIdentifier = 0;
			}
			if (rightTextNodeOffset1->vietnameseWordIdentifier && vnwords[rightTextNodeOffset1->vietnameseWordIdentifier].length > 2)
			{
				if (currentWordPoint > rightO1WordPoint)
				{
					rightTextNodeOffset1->vietnameseWordIdentifier = vnsyllables[rightTextNodeOffset1->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset0->vietnameseSyllableIdentifier, 0, 0);
				}
				else textNode->vietnameseWordIdentifier = 0;
			}
			if (rightTextNodeOffset2->vietnameseWordIdentifier && vnwords[rightTextNodeOffset2->vietnameseWordIdentifier].length > 3)
			{
				if (currentWordPoint > rightO2WordPoint)
				{
					rightTextNodeOffset2->vietnameseWordIdentifier = vnsyllables[rightTextNodeOffset2->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier, 0);
				}
				else textNode->vietnameseWordIdentifier = 0;
			}

			if (rightTextNodeOffset3->vietnameseWordIdentifier && vnwords[rightTextNodeOffset3->vietnameseWordIdentifier].length > 4)
			{
				if (currentWordPoint > rightO3WordPoint)
				{
					rightTextNodeOffset3->vietnameseWordIdentifier = vnsyllables[rightTextNodeOffset3->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier, rightTextNodeOffset0->vietnameseSyllableIdentifier);
				}
				else textNode->vietnameseWordIdentifier = 0;
			}

			if (rightTextNodeOffset4->vietnameseWordIdentifier && vnwords[rightTextNodeOffset4->vietnameseWordIdentifier].length > 5)
			{
				if (currentWordPoint > rightO4WordPoint)
				{
					rightTextNodeOffset4->vietnameseWordIdentifier = vnsyllables[rightTextNodeOffset4->vietnameseSyllableIdentifier].DetectWord(rightTextNodeOffset3->vietnameseSyllableIdentifier, rightTextNodeOffset2->vietnameseSyllableIdentifier, rightTextNodeOffset1->vietnameseSyllableIdentifier);
				}
				else textNode->vietnameseWordIdentifier = 0;
			}
			if (textNode->vietnameseWordIdentifier)
			{

				TEXT_NODE* leftTextNodeOffset0 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* leftTextNodeOffset1 = &nullTextNodeForStep2Normalize;
				TEXT_NODE* leftTextNodeOffset2 = &nullTextNodeForStep2Normalize;
				//TEXT_NODE *				leftTextNodeOffset3 = &nullTextNodeForStep2;
				//TEXT_NODE *				leftTextNodeOffset4 = &nullTextNodeForStep2;
				if (textNode->back)
				{
					leftTextNodeOffset0 = textNode->back;
					if (leftTextNodeOffset0->back)
					{
						leftTextNodeOffset1 = leftTextNodeOffset0->back;
						if (leftTextNodeOffset1->back)
						{
							leftTextNodeOffset2 = leftTextNodeOffset1->back;
							//	if (leftTextNodeOffset2->back)
							//	{
							//		leftTextNodeOffset3 = leftTextNodeOffset2->back;
							//		if (leftTextNodeOffset3->back)
							//		{
							//			leftTextNodeOffset4 = leftTextNodeOffset3->back;
							//		}
							//	}
						}
					}
				}
				switch (vnwords[textNode->vietnameseWordIdentifier].length)
				{
				case 2:
					if (leftTextNodeOffset0->countNumberSyllableInCurrentWord != 0)
					{
						WordSegmentConfuse;
					}
					textNode->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[1];
					textNode->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[1];
					textNode->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[1];

					leftTextNodeOffset0->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[0];
					leftTextNodeOffset0->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[0];
					leftTextNodeOffset0->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[0];

					textNode->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset0->joinWithNextNode = 0x31/*1*/;
					break;

				case 3:
					if (leftTextNodeOffset0->countNumberSyllableInCurrentWord != 0 || leftTextNodeOffset1->countNumberSyllableInCurrentWord)
					{
						WordSegmentConfuse;
					}

					textNode->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[2];
					textNode->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[2];
					textNode->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[2];

					leftTextNodeOffset0->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[1];
					leftTextNodeOffset0->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[1];
					leftTextNodeOffset0->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[1];

					leftTextNodeOffset1->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[0];
					leftTextNodeOffset1->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[0];
					leftTextNodeOffset1->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[0];

					textNode->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset0->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset0->joinWithNextNode = 0x31/*1*/;
					leftTextNodeOffset1->joinWithNextNode = 0x31/*1*/;
					break;

				case 4:
					if (leftTextNodeOffset0->countNumberSyllableInCurrentWord != 0 || leftTextNodeOffset1->countNumberSyllableInCurrentWord || leftTextNodeOffset2->countNumberSyllableInCurrentWord)
					{
						WordSegmentConfuse;
					}

					textNode->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[3];
					textNode->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[3];
					textNode->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[3];

					leftTextNodeOffset0->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[2];
					leftTextNodeOffset0->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[2];
					leftTextNodeOffset0->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[2];

					leftTextNodeOffset1->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[1];
					leftTextNodeOffset1->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[1];
					leftTextNodeOffset1->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[1];

					leftTextNodeOffset2->syllablePositionForwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionForward[0];
					leftTextNodeOffset2->syllablePositionBackwardInWord = vnwords[textNode->vietnameseWordIdentifier].viSyllablePositionReverse[0];
					leftTextNodeOffset2->countNumberSyllableInCurrentWord = vnwords[textNode->vietnameseWordIdentifier].viCountNumberSyllable[0];

					textNode->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset0->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset0->joinWithNextNode = 0x31/*1*/;
					leftTextNodeOffset1->joinWithLastNode = 0x31/*1*/;
					leftTextNodeOffset1->joinWithNextNode = 0x31/*1*/;
					leftTextNodeOffset2->joinWithNextNode = 0x31/*1*/;
					break;

				default:
					printf("Chua ho tro wordlength = %d", vnwords[textNode->vietnameseWordIdentifier].length);
					DungManHinh;
					textNode->syllablePositionForwardInWord = 0x31/*1*/;
					textNode->syllablePositionBackwardInWord = 0x31/*1*/;
					textNode->countNumberSyllableInCurrentWord = 0x31/*1*/;
					break;
				}
			}
			else
			{
				textNode->vietnameseWordIdentifier = 0;
				textNode->syllablePositionForwardInWord = 0x31/*1*/;
				textNode->syllablePositionBackwardInWord = 0x31/*1*/;
				textNode->countNumberSyllableInCurrentWord = 0x31/*1*/;
			}
		}
		//else
		//{
		//	textNode->syllablePositionForwardInWord = 0x31/*1*/;
		//	textNode->syllablePositionReverseInWord = 0x31/*1*/;
		//	textNode->countNumberSyllableInWord = 0x31/*1*/;
		//}
	}
#endif
	/************************************************************************/
	/* Log                                                                  */
	/************************************************************************/
	qtime step2EndTime;
	qGetTime(&step2EndTime);
	double step2Time = qGetDiffTime(step2StartTime, step2EndTime);
	Log("in %lfs second\n", step2Time);
	logTime += step2Time;
	if (logFile)
	{
		for (TEXT_NODE* textNode = uhead; textNode/*!=NULL*/; )
		{
			if (textNode->step != 1)
			{
				if (textNode->vietnameseSyllableIdentifier
					|| textNode->vietnameseAbbreviationIndentifier
					|| textNode->vietnameseLoanWordIndentifier
					|| textNode->englishWordIdentifier
					|| textNode->textNodeType == TEXT_NODE_TYPE_SILENCE
					)
				{

					bool flagHaveSomeChanged = (textNode->originalTextLength != textNode->textLength);
					if (flagHaveSomeChanged == false)
					{
						for (int iChar = 0; iChar < textNode->originalTextLength; iChar++)
						{
							if (textNode->originalText[iChar] != textNode->text[iChar])
							{
								flagHaveSomeChanged = true;
								//soft break
								iChar = textNode->originalTextLength;
							}
						}
					}
					if (flagHaveSomeChanged)
					{
						Log("\t\t+original [\"");
						Log(textNode->originalText, textNode->originalTextLength);
						Log("\" - %d character] --- text [\"", textNode->originalTextLength);
						Log(textNode->text, textNode->textLength);
						Log("\" - %d character] ", textNode->textLength);
					}
					else
					{
						Log("\t\t+ \"");
						Log(textNode->text, textNode->textLength);
						Log("\" - %d character ", textNode->textLength);
					}
					if (textNode->vietnameseSyllableIdentifier)  Log(", vietnamese syllable indentifier %d", textNode->vietnameseSyllableIdentifier);
					if (textNode->englishWordIdentifier)  Log(", english word indentifier %d", textNode->englishWordIdentifier);
					if (textNode->textNodeType == TEXT_NODE_TYPE_SILENCE)  Log(", Silence Time %lfs ", textNode->silenceTimeInSecond);
					Log("\n");
					textNode->step = 2;
				}
				else
				{
					Log("\t\t+[");
					Log(textNode->text, textNode->textLength);
					Log("] - %d character", textNode->textLength);
					if (uhead == textNode)  Log("(*****uhead*****)");
					else if (utail == textNode)  Log("(*****utail*****)");
					Log("\n");
				}
			}
			/* Go to next node */
			if (textNode == utail) textNode = 0;
			else textNode = textNode->next;
		}
	}
}
/************************************************************************/
/* Step 3 : Gen Standard Text                                           */
/************************************************************************/
void				VietnameseTextNormalizer::GenStandardText(void)
{
	if (flagValidToStandard)
	{
		/************************************************************************/
		/*                                                                      */
		/* Tạo ra một chuỗi chuẩn hóa từ chuỗi ban đầu                          */
		/*                                                                      */
		/************************************************************************/
		standardTextLength = originalTextLength;
		for (TEXT_NODE* textNode = head; textNode != 0; textNode = textNode->next)
		{
			if (textNode->originalTextLength < textNode->textLength)
			{
				standardTextLength += textNode->textLength - textNode->originalTextLength;
			}
			else if (textNode->originalTextLength > textNode->textLength)
			{
				standardTextLength -= textNode->originalTextLength - textNode->textLength;
			}
			if (textNode->textLength == 1
				&& (textNode->text[0] == 0x2E/*.*/ || textNode->text[0] == 0x2C/*,*/)
				&& textNode->next /*!= NULL*/
				&& textNode->originalText + 1 == textNode->next->originalText
				)
			{
				if ((//
					textNode->next->rightVietnameseSyllableSure != 0
					|| textNode->next->rightVietnameseWordSure != 0
					|| textNode->next->rightVietnameseAbbreviationSure != 0

					|| (textNode->next->capital == TEXT_NODE_CAPITAL_UPPER && textNode->back/* != NULL*/
						&& (
							textNode->back->leftVietnameseSyllableSure != 0
							|| textNode->back->leftVietnameseWordSure != 0
							|| textNode->back->leftVietnameseAbbreviationSure != 0
							))

					) && (textNode->next->vietnameseSyllableIdentifier || textNode->next->vietnameseLoanWordIndentifier || textNode->next->englishWordIdentifier || textNode->vietnameseAbbreviationIndentifier)) textNode->needSpaceAfter = 1;
				else if (textNode->next->vietnameseSyllableIdentifier && vnsyllables[textNode->next->vietnameseSyllableIdentifier].vietnamese)  textNode->needSpaceAfter = 1;
				else if (textNode->next->capital == TEXT_NODE_CAPITAL_CAPITAL && textNode->vietnameseAbbreviationIndentifier) textNode->needSpaceAfter = 1;
			}
			if (textNode->needSpaceAfter) standardTextLength++;
		}
		Log("\tStep 5 : Gen Standard Text %d characters from original text %d characters\n", standardTextLength, originalTextLength);
		standardText = (qwchar*)qcalloc(standardTextLength + 100/*safe*/, sizeof(qwchar));
		if (standardText)
		{
			bool				validateStandardText = true;
			int					countStandardTextSlot = standardTextLength;
			int					countOriginalTextSlot = originalTextLength;
			qwchar* standardTextPtr = standardText;
			const qwchar* originalTextPtr = originalText;
			/************************************************************************/
			/* Chèn đoạn đầu                                                        */
			/************************************************************************/
			if (head)
			{
				int	preparationLength = int(head->originalText - originalTextPtr);
				if (preparationLength >= 0 && preparationLength <= countStandardTextSlot && preparationLength <= countOriginalTextSlot)
				{
					for (int iCharacter = 0; iCharacter < preparationLength; iCharacter++)
					{
						*standardTextPtr = *originalTextPtr;
						standardTextPtr++;
						originalTextPtr++;
					}
					countStandardTextSlot -= preparationLength;
					countOriginalTextSlot -= preparationLength;
				}
				else if (preparationLength)
				{
					validateStandardText = false;
				}
			}
			/************************************************************************/
			/* Chèn đoạn chính                                                      */
			/************************************************************************/
			if (validateStandardText)
			{
				for (TEXT_NODE* textNode = head; textNode != 0 && validateStandardText/* == true*/; textNode = textNode->next)
				{
					if (textNode->originalText)
					{
						bool flagCurrentNodeIsChange = (textNode->textLength != textNode->originalTextLength);
						if (flagCurrentNodeIsChange == false)
						{
							for (int ichar = 0; ichar < textNode->textLength; ichar++)
							{
								if (textNode->text[ichar] != textNode->originalText[ichar])
								{
									flagCurrentNodeIsChange = true;
									ichar = textNode->textLength/*soft break*/;
								}
							}
						}
						if (textNode->needSpaceAfter) standardTextChange++;
						if (flagCurrentNodeIsChange)
						{
							Log("\t\t+Fix \"");
							Log(textNode->originalText, textNode->originalTextLength);
							Log("\" - %d character ----> \"", textNode->originalTextLength);
							Log(textNode->text, textNode->textLength);
							Log("\" - %d character\n", textNode->textLength);
							standardTextChange++;
						}
						/************************************************************************/
						/* Phần cắt                                                             */
						/************************************************************************/
						if (validateStandardText)
						{
							int seperationLength = int(textNode->originalText - originalTextPtr);
							if (seperationLength >= 0 && seperationLength <= countStandardTextSlot && seperationLength <= countOriginalTextSlot)
							{
								for (int ichar = 0; ichar < seperationLength; ichar++)
								{
									*standardTextPtr = *originalTextPtr;
									standardTextPtr++;
									originalTextPtr++;
								}
								countStandardTextSlot -= seperationLength;
								countOriginalTextSlot -= seperationLength;
							}
							//else if (seperationLength) validateStandardText = false;
						}
						/************************************************************************/
						/* Phần node                                                            */
						/************************************************************************/
						int textLength = textNode->textLength;
						if (textLength >= 0 && textLength <= countStandardTextSlot && textLength <= countOriginalTextSlot)
						{
							for (int ichar = 0; ichar < textLength; ichar++)
							{
								*standardTextPtr = textNode->text[ichar];
								standardTextPtr++;
							}
							countStandardTextSlot -= textLength;
							countOriginalTextSlot -= textNode->originalTextLength;
							originalTextPtr += textNode->originalTextLength;
							if (textNode->needSpaceAfter)
							{
								*standardTextPtr = 0x20/*space*/;
								standardTextPtr++;
								countStandardTextSlot -= 1;
							}
						}
						else if (textLength)
						{
							validateStandardText = false;
						}
					}
				}
			}
			/************************************************************************/
			/* Chèn đoạn cuối                                                       */
			/************************************************************************/
			if (validateStandardText && countOriginalTextSlot)
			{
				if (countOriginalTextSlot >= 0 && countOriginalTextSlot <= countStandardTextSlot)
				{
					for (int ichar = 0; ichar < countOriginalTextSlot; ichar++)
					{
						*standardTextPtr = *originalTextPtr;
						standardTextPtr++;
						originalTextPtr++;
					}
					countStandardTextSlot -= countOriginalTextSlot;
					countOriginalTextSlot -= countOriginalTextSlot;
				}
				else if (countOriginalTextSlot)
				{
					validateStandardText = false;
				}
			}
			if (validateStandardText == false)
			{
				qfree(standardText);
				standardText = 0;
				standardTextLength = 0;
			}
			else
			{
				Log("\t=========> GenStandardText : %d change syllable\n", standardTextChange);
				Log(standardText, standardTextLength);
				Log("\n\n");
			}
			/************************************************************************/
			/* Chuyển đổi các kí tự đặc biệt nằm ngoài âm tiết                      */
			/************************************************************************/
			for (int iChar = 0; iChar < standardTextLength; iChar++)
			{
				switch (standardText[iChar])
				{
				case 0x200B/*Zero width space*/:
				case 0xFEFF/*Zero width no-break space*/:
				case 0xA0/*Non-breaking space*/:
					standardText[iChar] = L' ';
					standardTextChange++;
					break;
				}
			}
		}
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
void				VietnameseTextNormalizer::Refresh(void)
{
	/************************************************************************/
	/* Free all text node                                                 */
	/************************************************************************/
	for (TEXT_NODE* textNode = head; textNode /*!= NULL*/;)
	{
		if (textNode->extra /*!= NULL*/)
		{
			if (textNode->extra->memText) qfree(textNode->extra->memText);
			if (textNode->extra->memTextLower) qfree(textNode->extra->memTextLower);
			if (textNode->extra->memTextType) qfree(textNode->extra->memTextType);
			if (textNode->extra->vsyllables) qfree(textNode->extra->vsyllables);
			qfree(textNode->extra);
		}
		TEXT_NODE* backupTextNode = textNode;
		for (PHONEME_NODE* phonemeNode = textNode->phonemeStart; phonemeNode/*!=0*/;)
		{
			PHONEME_NODE* backupPhonemeNode = phonemeNode;
			phonemeNode = phonemeNode->next;
			qfree(backupPhonemeNode);
		}
		textNode = textNode->next;
		qfree(backupTextNode);
	}
	/************************************************************************/
	/* Log                                                                  */
	/************************************************************************/
	if (this->logFile)
	{
		qfprintf(this->logFile, "\\****************************************End****************************************/\n\n");
		fflush(this->logFile);
	}
	/************************************************************************/
	/* Free Standard Text                                                   */
	/************************************************************************/
	if (standardText) qfree(standardText);
	standardText = 0;
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	trustedAbbreviationSet.clear();
	trustedLoanSet.clear();
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	Init();
}
/*Desstructor*/		VietnameseTextNormalizer::~VietnameseTextNormalizer()
{
	Refresh();
}

