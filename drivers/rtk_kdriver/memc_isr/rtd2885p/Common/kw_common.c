#if 1
/**
 * @file common.c
 * @brief This file implements common stdio/memory/string library function. These function only support MEMC library.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Kiwi-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifdef WIN32
#define DLL_IMPLEMENT
#endif

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Common/kw_uart.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"

#ifdef WIN32
#include <stdio.h> 
#endif



#if 1//RTK_MEMC_Performance_tunging_from_tv001
#define __unused __attribute__ ((unused))
#endif

/*! Global variable */
static unsigned int		guiErrorNO;			///< error number

/*! Internal function for use in this file. */

#define SUPPORT_FLOAT 0   // fix warning message

/**
 * @brief This internal function convert string to Hex
 * @param pcString		String to convert to hex
 * @return Hex data returned. If input string is wrong, guiErrorNO is set.
*/
static int			 ConverStrToHex(char *pcString)
{
	int					i;
	int					iLen = kw_strlen(pcString);
	int					iVal = 0;

	/* Loop to convert */
	for (i = iLen - 1; i < iLen; i--)
	{
		if (pcString[i] >= '0' && pcString[i] <= '9')
		{
			iVal |= (pcString[i] - 0x30) << (i * 4);
		}
		else if (pcString[i] >= 'A' && pcString[i] <= 'F')
		{
			iVal |= (pcString[i] - 55) << (i * 4);
		}
		else if (pcString[i] >= 'a' && pcString[i] <= 'f')
		{
			iVal |= (pcString[i] - 87) << (i * 4);
		}
		else
		{
			guiErrorNO = ERR_PARA;
			break;
		}
	}

	return iVal;
}

/**
 * @brief This internal function convert input string to a signed decimal number.
 * @param pcString		String to convert to decimal number, no signed byte. 
 * @return Deciaml number converted. If input string is wrong, guiErrorNO is set.
*/
static int			 ConvertStrToSINT(char *pcString)
{
	int					i;
	int					iLen = kw_strlen(pcString);
	int					iVal = 0;

	/* Loop to convert */
	for (i = iLen - 1; i < iLen; i--)
	{
		if (pcString[i] >= '0' && pcString[i] <= '9')
		{
			iVal |= (pcString[i] - 0x30) << (i * 4);
		}
		else
		{
			guiErrorNO = ERR_PARA;
			break;
		}
	}

	return iVal;
}

#ifdef WIN32
static pPrintChar gp = NULL;

VOID kw_SetPrintfCallback(pPrintChar p)
{
	gp = p;
}

#endif

static VOID printch(char ch)
{
//	Uart_Printf(ch);
#ifdef WIN32
	printf("%c", ch);
	if (gp != NULL)
		gp(ch);
#else
	Uart_Printf(ch);
#endif
}

#if 1//RTK_MEMC_Performance_tunging_from_tv001
__unused static VOID printdec(int dec) 
#else
static VOID printdec(int			 dec)
#endif
{
	if(dec == 0)
	{
		return;
	}
	else if (dec < 0)
	{
		printch('-');
		dec *= -1;
	}
	
	printdec(dec / 10);
	printch((char)(dec % 10 + '0'));
}

#if SUPPORT_FLOAT
static VOID printflt(double flt)
{
	int			 tmpint = 0;
    
	tmpint = (int			)flt;
	printdec(tmpint);
	printch('.');
	flt = flt - tmpint;
	tmpint = (int			)(flt * 1000000);
	printdec(tmpint);
}
#endif

static VOID printstr(const char* str)
{
#if RTK_MEMC_Performance_tunging_from_tv001
	if(str == NULL){
		return;
	}
#endif

	while(*str)
	{
		printch(*str++);
	}
}

#if 1//RTK_MEMC_Performance_tunging_from_tv001
__unused static VOID printbin(unsigned int bin) 
#else
static VOID printbin(unsigned int bin)
#endif
{
	if(bin == 0)
	{
		printstr("0");
		return;
	}
	printbin(bin / 2);
	printch((char)(bin % 2 + '0'));
}

#if 1//RTK_MEMC_Performance_tunging_from_tv001
__unused static VOID printhex(unsigned int hex) 
#else
static VOID printhex(unsigned int hex)
#endif

{
	if (hex == 0)
		return;

	printhex(hex / 16);

	if((hex % 16) < 10)
	{
		printch((char)(hex % 16 + '0'));
	}
	else
	{
		printch((char)(hex % 16 - 10 + 'a' ));
	}
}

/*! Exported API */

/**
 * @brief This exported function convert input number string to a signed int			 value.
 * @param [in] pcString			String to convert to signed number
 * @return Value converted from input string. If input string is wrong, guiErrorNO is set.
*/

int			 kw_atoi(char *pcString)
{
	char *pcTemp = pcString;
	BOOL bHex = FALSE;
	BOOL bNeg = FALSE;
	int			 iVal = 0;

	/* check input parameter */
	if (pcTemp == NULL)
	{
		guiErrorNO = ERR_PARA;
		return iVal;
	}

	/* check whether is Hex */
	if (pcTemp[0] == '0' && ((pcTemp[1] == 'x') || (pcTemp[1] == 'X')))
	{
		bHex = TRUE;
	}

	/* check negative number */
	if (pcTemp[0] == '-')
	{
		bNeg = TRUE;
	}

	/* Convert string*/
	if (bHex)
	{
		/* it is hex string like 0x12abcdef */
		pcTemp += 2;
		iVal = ConverStrToHex(pcTemp);
	}
	else if (bNeg)
	{
		/* it is negative number string like -1234 */
		pcTemp++;
		iVal = ConvertStrToSINT(pcTemp);

		iVal *= -1;
	}
	else
	{
		/* it is positive number string */
		iVal = ConvertStrToSINT(pcTemp);
	}

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return iVal;
}

/**
 * @brief This exported function convert input number string to a unsigned value.
 * @param [in] pcString			String to convert to unsigned number
 * @return Value converted from input string. If input string is wrong, guiErrorNO is set.
*/
unsigned int kw_atoui(char *pcString)
{
	char *pcTemp = pcString;
//	BOOL bHex = FALSE;
//	BOOL bNeg = FALSE;
	unsigned int uiVal = 0;

	/* check input parameter */
	if ((pcTemp == NULL) || (pcTemp[0] == '-'))
	{
		guiErrorNO = ERR_PARA;
		return uiVal;
	}

	/* check string validate */
	if (pcTemp[0] == '0' && ((pcTemp[1] == 'x') || (pcTemp[1] == 'X')))
	{
		/* hex number string */
		pcTemp += 2;
		uiVal = (unsigned int)ConverStrToHex(pcTemp);
	}
	else
	{
		uiVal = (unsigned int)ConvertStrToSINT(pcTemp);
	}

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return uiVal;
}

/**
 * @brief This exported function concat the source string into dest string and return the dest string.
 * @param [in] pcDest		Dest string need to concat. The pcDest must have enough space to save source string, otherwise pcSource truncated.
 * @param [in] iDestBufLen	Dest buffer total length including pcDest original string length
 * @param [in] pcSource		Source string
 * @return String merged. If dest string has no enough buffer, source string is truncated.
*/
char *kw_strcat(char *pcDest, int			 iDestBufLen, char *pcSource)
{
	int				iBufSpaceLen;
	char	*pcDestTmp;
	int				i;

	/* check input parameter if Dest string is NULL or Destbuflen is samller than Dest string Length fail strcat */
	if (pcDest == NULL || (kw_strlen(pcDest) >= iDestBufLen))
		return NULL;

	/* if source string is NULL, just return Dest string */
	if (pcSource == NULL)
		return pcDest;

	/* get buf space */
	iBufSpaceLen = iDestBufLen - kw_strlen(pcDest);

	/* minus string can fill in */
	iBufSpaceLen = iBufSpaceLen > kw_strlen(pcSource) ? kw_strlen(pcSource) : (iBufSpaceLen - 1);

	pcDestTmp = (char *)(&pcDest[kw_strlen(pcDest)]);

	for (i = 0; i < iBufSpaceLen; i++)
	{
		pcDestTmp[i] = pcSource[i];
	}

	/* set string end */
	pcDestTmp[i] = 0;

	/* clear errorNO */
	guiErrorNO = SUCCESS;
	
	return pcDest;
}

/**
 * @brief This exported function get input string length. Does not support unicode.
 * @param [in] pcSource string pointer want to get length.
 * @return String length, if the input pointer is NULL, return 0
*/
int			 kw_strlen(char *pcSource)
{
	int				iLen = 0;
	char	*pcTemp = pcSource;

	/* check input parameter */
	if (pcTemp == NULL)
	{
		return iLen;
	}

	/* loop to scan pcSource for length */
	do
	{
		iLen++;
		pcTemp++;
	}while (*pcTemp != 0);

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return iLen;
}

/**
 * @brief This exported function get common function error NO. It is better to get this value after each call.
 * @return ErrorNO of last operation
*/
unsigned int kw_GetErrorNO(VOID)
{
	return guiErrorNO;
}

/**
 * @brief This exported function is almost same with C runtime standard printf function
 * @param [in] pcFormat		format string of this printf. The input parameter is variable.
 * @return Character number print out from pcFormat.
*/
int			 kw_printf(char *pcFormat, ...)
{

#if 1//RTK_MEMC_Performance_tunging_from_tv001
	#if 0
		int					ret = 0;
	va_list		vp;
	char		*pfmt;
//	double		vargflt = 0;
	int					vargint = 0;
	unsigned int		vargunsignedint = 0;
	char		*vargpch = NULL;
	char		vargch = 0;

	va_start(vp, pcFormat);
	pfmt = pcFormat;

	while (*pfmt)
	{
		if(*pfmt == '%')
		{
			switch(*(++pfmt))
			{
				case 'c':
						vargch = (char)va_arg(vp, int			); 
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printch(vargch);
						break;

				case 'd':
				case 'i':
						vargint = va_arg(vp, int			);
						if(vargint == 0)
							printch((char)(0 + '0'));
						else
							printdec(vargint);
						break;

#if SUPPORT_FLOAT	// do not support float value
				case 'f':
						vargflt = va_arg(vp, double);
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printflt(vargflt);
						break;
#endif

				case 's':
						vargpch = va_arg(vp, char*);
						printstr(vargpch);
						break;

				case 'b':
				case 'B':
						vargunsignedint = va_arg(vp, unsigned int);
						printbin(vargunsignedint);
						break;

				case 'x':
				case 'X':
						vargunsignedint = va_arg(vp, unsigned int);
						
						if (vargunsignedint == 0)
							printstr("0");
						else
							printhex(vargunsignedint);
						break;

				case '%':
						printch('%');
						break;
				
				default:
						break;
			}

			pfmt++;
		}
		else
		{
			printch(*pfmt++);
		}
	}

	va_end(vp);

	return ret;
	#endif
        return 0;
#else

	int			ret = 0;
	va_list		vp;
	char		*pfmt;
//	double		vargflt = 0;
	int					vargint = 0;
	unsigned int		vargunsignedint = 0;
	char		*vargpch = NULL;
	char		vargch = 0;

	va_start(vp, pcFormat);
	pfmt = pcFormat;

	while (*pfmt)
	{
		if(*pfmt == '%')
		{
			switch(*(++pfmt))
			{
				case 'c':
						vargch = (char)va_arg(vp, int			); 
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printch(vargch);
						break;

				case 'd':
				case 'i':
						vargint = va_arg(vp, int			);
						if(vargint == 0)
							printch((char)(0 + '0'));
						else
							printdec(vargint);
						break;

#if SUPPORT_FLOAT	// do not support float value
				case 'f':
						vargflt = va_arg(vp, double);
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printflt(vargflt);
						break;
#endif

				case 's':
						vargpch = va_arg(vp, char*);
						printstr(vargpch);
						break;

				case 'b':
				case 'B':
						vargunsignedint = va_arg(vp, unsigned int);
						printbin(vargunsignedint);
						break;

				case 'x':
				case 'X':
						vargunsignedint = va_arg(vp, unsigned int);
						
						if (vargunsignedint == 0)
							printstr("0");
						else
							printhex(vargunsignedint);
						break;

				case '%':
						printch('%');
						break;
				
				default:
						break;
			}

			pfmt++;
		}
		else
		{
			printch(*pfmt++);
		}
	}

	va_end(vp);

	return ret;
#endif




}

/**
 * @brief This exported function covnver input signed number into input buffer based on base and return the buffer.
 * @param [in] iVal			signed number want to convert to string
 * @param [in] *pcBuf		buffer contains the convert string
 * @param [in] iBufLen		buffer length
 * @param [in] iBase			base is 16 for hex, 10 for dec, 2 for binary, not support others currently
 * @return Character number show out
*/
char *kw_itoa(int			 iVal, char *pcBuf, int			 iBufLen, int			 iBase)
{
	char *pcTmp = pcBuf;

	/*! if it is negative number, convert to positive number. Negative number only support decimal string */
	if (((iVal < 0) && (iBase != 10)) || (pcTmp == NULL) || (iBufLen <= 0))
	{
		return NULL;
	}

	if (iVal < 0)
	{
		pcTmp[0] = '-';
		iVal *= -1;			// convert to positive number
		iBufLen--;
	}

	pcTmp = kw_uitoa(iVal, (char *)&pcTmp[1], iBufLen, iBase);

	return pcTmp;
}

/**
 * @brief This exported function covnver input signed number into input buffer based on base and return the buffer.
 * @param [in] iVal			signed number want to convert to string
 * @param [in] *pcBuf		buffer contains the convert string
 * @param [in] iBufLen		buffer length
 * @param [in] iBase			base is 16 for hex, 10 for dec, 2 for binary, not support others currently
 * @return Character number show out
*/
char *kw_uitoa(int			 iVal, char *pcBuf, int			 iBufLen, int			 iBase)
{
	return NULL;
}

/**
 * @brief This exported function copy specified length of character from source string start position into dest string.
 *		  The invoker should guarantee sufficient dest string buffer.
 * @param [in] pcDest			Dest string buffer want to copy
 * @param [in] pcSource			Source string
 * @param [in] iCpyLen			Copy length, it is the invoker responsable to guarrant buffer length
 * @param [in] iStarIndex		Source string start index
 * @return Character number show out
*/
char *kw_strcpy(char *pcDest, char *pcSource, int			 iCpyLen, int			 iStartIndex)
{
	/* check input parameter */
	if (pcDest != NULL && pcSource != NULL && iCpyLen > 0 && iStartIndex >= 0 && iStartIndex < kw_strlen(pcSource))
	{
//		char	*pcTemp = (char *)(&pcSource[iStartIndex]);

		while (iCpyLen && *pcDest)
		{
			*pcDest = *pcSource;
			pcDest++;
			pcSource++;
			iCpyLen--;
		}

		*pcDest = 0;
	}

	return pcDest;
}

#else
/**
 * @file common.c
 * @brief This file implements common stdio/memory/string library function. These function only support MEMC library.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifdef WIN32
#define DLL_IMPLEMENT
#endif

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Common/kw_uart.h"

#ifdef WIN32
#include <stdio.h>
#endif

/*! Global variable */
static unsigned int		guiErrorNO;			///< error number

/*! Internal function for use in this file. */
/**
 * @brief This internal function convert string to Hex
 * @param pcString		String to convert to hex
 * @return Hex data returned. If input string is wrong, guiErrorNO is set.
*/
static int			 ConverStrToHex(char *pcString)
{
	int					i;
	int					iLen = kw_strlen(pcString);
	int					iVal = 0;

	/* Loop to convert */
	for (i = iLen - 1; i < iLen; i--)
	{
		if (pcString[i] >= '0' && pcString[i] <= '9')
		{
			iVal |= (pcString[i] - 0x30) << (i * 4);
		}
		else if (pcString[i] >= 'A' && pcString[i] <= 'F')
		{
			iVal |= (pcString[i] - 55) << (i * 4);
		}
		else if (pcString[i] >= 'a' && pcString[i] <= 'f')
		{
			iVal |= (pcString[i] - 87) << (i * 4);
		}
		else
		{
			guiErrorNO = ERR_PARA;
			break;
		}
	}

	return iVal;
}

/**
 * @brief This internal function convert input string to a signed decimal number.
 * @param pcString		String to convert to decimal number, no signed byte. 
 * @return Deciaml number converted. If input string is wrong, guiErrorNO is set.
*/
static int			 ConvertStrToSINT(char *pcString)
{
	int					i;
	int					iLen = kw_strlen(pcString);
	int					iVal = 0;

	/* Loop to convert */
	for (i = iLen - 1; i < iLen; i--)
	{
		if (pcString[i] >= '0' && pcString[i] <= '9')
		{
			iVal |= (pcString[i] - 0x30) << (i * 4);
		}
		else
		{
			guiErrorNO = ERR_PARA;
			break;
		}
	}

	return iVal;
}

#ifdef WIN32
static pPrintChar gp = NULL;

VOID kw_SetPrintfCallback(pPrintChar p)
{
	gp = p;
}

#endif

static VOID printch(char ch)
{
//	Uart_Printf(ch);
#ifdef WIN32
	printf("%c", ch);
	if (gp != NULL)
		gp(ch);
#else
	Uart_Printf(ch);
#endif
}

static VOID printdec(int			 dec)
{
	if(dec == 0)
	{
		return;
	}
	else if (dec < 0)
	{
		printch('-');
		dec *= -1;
	}
	
	printdec(dec / 10);
	printch((char)(dec % 10 + '0'));
}

#ifdef SUPPORT_FLOAT
static VOID printflt(double flt)
{
	int			 icnt = 0;
	int			 tmpint = 0;
    
	tmpint = (int			)flt;
	printdec(tmpint);
	printch('.');
	flt = flt - tmpint;
	tmpint = (int			)(flt * 1000000);
	printdec(tmpint);
}
#endif

static VOID printstr(const char* str)
{
	while(*str)
	{
		printch(*str++);
	}
}

static VOID printbin(unsigned int bin)
{
	if(bin == 0)
	{
		printstr("0");
		return;
	}
	printbin(bin / 2);
	printch((char)(bin % 2 + '0'));
}

static VOID printhex(unsigned int hex)
{
	if (hex == 0)
		return;

	printhex(hex / 16);

	if((hex % 16) < 10)
	{
		printch((char)(hex % 16 + '0'));
	}
	else
	{
		printch((char)(hex % 16 - 10 + 'a' ));
	}
}

/*! Exported API */

/**
 * @brief This exported function convert input number string to a signed int			 value.
 * @param [in] pcString			String to convert to signed number
 * @return Value converted from input string. If input string is wrong, guiErrorNO is set.
*/

int			 kw_atoi(char *pcString)
{
	char *pcTemp = pcString;
	BOOL bHex = FALSE;
	BOOL bNeg = FALSE;
	int			 iVal = 0;

	/* check input parameter */
	if (pcTemp == NULL)
	{
		guiErrorNO = ERR_PARA;
		return iVal;
	}

	/* check whether is Hex */
	if (pcTemp[0] == '0' && ((pcTemp[1] == 'x') || (pcTemp[1] == 'X')))
	{
		bHex = TRUE;
	}

	/* check negative number */
	if (pcTemp[0] == '-')
	{
		bNeg = TRUE;
	}

	/* Convert string*/
	if (bHex)
	{
		/* it is hex string like 0x12abcdef */
		pcTemp += 2;
		iVal = ConverStrToHex(pcTemp);
	}
	else if (bNeg)
	{
		/* it is negative number string like -1234 */
		pcTemp++;
		iVal = ConvertStrToSINT(pcTemp);

		iVal *= -1;
	}
	else
	{
		/* it is positive number string */
		iVal = ConvertStrToSINT(pcTemp);
	}

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return iVal;
}

/**
 * @brief This exported function convert input number string to a unsigned value.
 * @param [in] pcString			String to convert to unsigned number
 * @return Value converted from input string. If input string is wrong, guiErrorNO is set.
*/
unsigned int kw_atoui(char *pcString)
{
	char *pcTemp = pcString;
//	BOOL bHex = FALSE;
//	BOOL bNeg = FALSE;
	unsigned int uiVal = 0;

	/* check input parameter */
	if ((pcTemp == NULL) || (pcTemp[0] == '-'))
	{
		guiErrorNO = ERR_PARA;
		return uiVal;
	}

	/* check string validate */
	if (pcTemp[0] == '0' && ((pcTemp[1] == 'x') || (pcTemp[1] == 'X')))
	{
		/* hex number string */
		pcTemp += 2;
		uiVal = (unsigned int)ConverStrToHex(pcTemp);
	}
	else
	{
		uiVal = (unsigned int)ConvertStrToSINT(pcTemp);
	}

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return uiVal;
}

/**
 * @brief This exported function concat the source string into dest string and return the dest string.
 * @param [in] pcDest		Dest string need to concat. The pcDest must have enough space to save source string, otherwise pcSource truncated.
 * @param [in] iDestBufLen	Dest buffer total length including pcDest original string length
 * @param [in] pcSource		Source string
 * @return String merged. If dest string has no enough buffer, source string is truncated.
*/
char *kw_strcat(char *pcDest, int			 iDestBufLen, char *pcSource)
{
	int				iBufSpaceLen;
	char	*pcDestTmp;
	int				i;

	/* check input parameter if Dest string is NULL or Destbuflen is samller than Dest string Length fail strcat */
	if (pcDest == NULL || (kw_strlen(pcDest) >= iDestBufLen))
		return NULL;

	/* if source string is NULL, just return Dest string */
	if (pcSource == NULL)
		return pcDest;

	/* get buf space */
	iBufSpaceLen = iDestBufLen - kw_strlen(pcDest);

	/* minus string can fill in */
	iBufSpaceLen = iBufSpaceLen > kw_strlen(pcSource) ? kw_strlen(pcSource) : (iBufSpaceLen - 1);

	pcDestTmp = (char *)(&pcDest[kw_strlen(pcDest)]);

	for (i = 0; i < iBufSpaceLen; i++)
	{
		pcDestTmp[i] = pcSource[i];
	}

	/* set string end */
	pcDestTmp[i] = 0;

	/* clear errorNO */
	guiErrorNO = SUCCESS;
	
	return pcDest;
}

/**
 * @brief This exported function get input string length. Does not support unicode.
 * @param [in] pcSource string pointer want to get length.
 * @return String length, if the input pointer is NULL, return 0
*/
int			 kw_strlen(char *pcSource)
{
	int				iLen = 0;
	char	*pcTemp = pcSource;

	/* check input parameter */
	if (pcTemp == NULL)
	{
		return iLen;
	}

	/* loop to scan pcSource for length */
	do
	{
		iLen++;
		pcTemp++;
	}while (*pcTemp != 0);

	/* clear errorNO */
	guiErrorNO = SUCCESS;

	return iLen;
}

/**
 * @brief This exported function get common function error NO. It is better to get this value after each call.
 * @return ErrorNO of last operation
*/
unsigned int kw_GetErrorNO(VOID)
{
	return guiErrorNO;
}

/**
 * @brief This exported function is almost same with C runtime standard printf function
 * @param [in] pcFormat		format string of this printf. The input parameter is variable.
 * @return Character number print out from pcFormat.
*/
int			 kw_printf(char *pcFormat, ...)
{
	int					ret = 0;
	va_list		vp;
	char		*pfmt;
//	double		vargflt = 0;
	int					vargint = 0;
	unsigned int		vargunsignedint = 0;
	char		*vargpch = NULL;
	char		vargch = 0;

	va_start(vp, pcFormat);
	pfmt = pcFormat;

	while (*pfmt)
	{
		if(*pfmt == '%')
		{
			switch(*(++pfmt))
			{
				case 'c':
						vargch = (char)va_arg(vp, int			); 
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printch(vargch);
						break;

				case 'd':
				case 'i':
						vargint = va_arg(vp, int			);
						if(vargint == 0)
							printch((char)(0 + '0'));
						else
							printdec(vargint);
						break;

#if SUPPORT_FLOAT	// do not support float value
				case 'f':
						vargflt = va_arg(vp, double);
						/*    va_arg(ap, type), if type is narrow type (char, short, float) an error is given in strict ANSI
						mode, or a warning otherwise.In non-strict ANSI mode, 'type' is allowed to be any expression. */
						printflt(vargflt);
						break;
#endif

				case 's':
						vargpch = va_arg(vp, char*);
						printstr(vargpch);
						break;

				case 'b':
				case 'B':
						vargunsignedint = va_arg(vp, unsigned int);
						printbin(vargunsignedint);
						break;

				case 'x':
				case 'X':
						vargunsignedint = va_arg(vp, unsigned int);
						
						if (vargunsignedint == 0)
							printstr("0");
						else
							printhex(vargunsignedint);
						break;

				case '%':
						printch('%');
						break;
				
				default:
						break;
			}

			pfmt++;
		}
		else
		{
			printch(*pfmt++);
		}
	}

	va_end(vp);

	return ret;
}

/**
 * @brief This exported function covnver input signed number into input buffer based on base and return the buffer.
 * @param [in] iVal			signed number want to convert to string
 * @param [in] *pcBuf		buffer contains the convert string
 * @param [in] iBufLen		buffer length
 * @param [in] iBase			base is 16 for hex, 10 for dec, 2 for binary, not support others currently
 * @return Character number show out
*/
char *kw_itoa(int			 iVal, char *pcBuf, int			 iBufLen, int			 iBase)
{
	char *pcTmp = pcBuf;

	/*! if it is negative number, convert to positive number. Negative number only support decimal string */
	if (((iVal < 0) && (iBase != 10)) || (pcTmp == NULL) || (iBufLen <= 0))
	{
		return NULL;
	}

	if (iVal < 0)
	{
		pcTmp[0] = '-';
		iVal *= -1;			// convert to positive number
		iBufLen--;
	}

	pcTmp = kw_uitoa(iVal, (char *)&pcTmp[1], iBufLen, iBase);

	return pcTmp;
}

/**
 * @brief This exported function covnver input signed number into input buffer based on base and return the buffer.
 * @param [in] iVal			signed number want to convert to string
 * @param [in] *pcBuf		buffer contains the convert string
 * @param [in] iBufLen		buffer length
 * @param [in] iBase			base is 16 for hex, 10 for dec, 2 for binary, not support others currently
 * @return Character number show out
*/
char *kw_uitoa(int			 iVal, char *pcBuf, int			 iBufLen, int			 iBase)
{
	return NULL;
}

/**
 * @brief This exported function copy specified length of character from source string start position into dest string.
 *		  The invoker should guarantee sufficient dest string buffer.
 * @param [in] pcDest			Dest string buffer want to copy
 * @param [in] pcSource			Source string
 * @param [in] iCpyLen			Copy length, it is the invoker responsable to guarrant buffer length
 * @param [in] iStarIndex		Source string start index
 * @return Character number show out
*/
char *kw_strcpy(char *pcDest, char *pcSource, int			 iCpyLen, int			 iStartIndex)
{
	/* check input parameter */
	if (pcDest != NULL && pcSource != NULL && iCpyLen > 0 && iStartIndex >= 0 && iStartIndex < kw_strlen(pcSource))
	{
//		char	*pcTemp = (char *)(&pcSource[iStartIndex]);

		while (iCpyLen && *pcDest)
		{
			*pcDest = *pcSource;
			pcDest++;
			pcSource++;
			iCpyLen--;
		}

		*pcDest = 0;
	}

	return pcDest;
}

#endif
