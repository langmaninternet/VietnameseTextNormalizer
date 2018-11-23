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
#include "ContextSystem.h"
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma managed(push, off)
#pragma optimize("", off)
#else
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif

/************************************************************************/
/* Uniform Resource Identifier                                          */
/************************************************************************/
int UniformResourceIdentifierScheme(qinput const qwchar * text)
{

/*Nếu bắt đầu là 'http://' hoặc 'https://' hoặc 'mms://' */
	return (text && (
		(text[0] == 0x68/*h*/ && text[1] == 0x74/*t*/ && text[2] == 0x74/*t*/ && text[3] == 0x70/*p*/ && text[4] == 0x3A/*:*/ && text[5] == 0x2F/*/*/ && text[6] == 0x2F/*/*/)
		|| (text[0] == 0x68/*h*/ && text[1] == 0x74/*t*/ && text[2] == 0x74/*t*/ && text[3] == 0x70/*p*/ && text[4] == 0x73/*s*/ && text[5] == 0x3A/*:*/ && text[6] == 0x2F/*/*/ && text[7] == 0x2F/*/*/)
		|| (text[0] == 0x6D/*m*/ && text[1] == 0x6D/*m*/ && text[2] == 0x73/*s*/ && text[3] == 0x3A/*:*/ && text[4] == 0x2F/*/*/ && text[5] == 0x2F/*/*/)
		));

}



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
#pragma optimize("", on)
#pragma managed(pop)
#else
#pragma GCC pop_options
#endif

