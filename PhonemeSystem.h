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
#ifndef _QUANGBT_PHONEME_SYSTEM_HEADER_
#define _QUANGBT_PHONEME_SYSTEM_HEADER_
#include "ConfigSystem.h"
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(push, off)
#endif
/************************************************************************/
/* The system of phonemes                                               */
/************************************************************************/
struct PHONEME
{
	const char		name[6];
	const char		code[6];
};
extern const struct PHONEME phonemes[];
void ValidatePhonemeData(void);
#define AM_LUOI_PHONEME_INDEX 4
#ifdef _DEBUG
extern const int phonemeSize;
extern const int phonemeSystemSizeInBytes;
#endif
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(pop)
#endif
#endif
