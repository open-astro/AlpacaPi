//**************************************************************************
//*	Name:			eventlogging.c
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May 21,	2019	<MLS> Created eventlogging.c
//*	May 22,	2019	<MLS> Added SendHtmlLog()
//*****************************************************************************


//#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
//#include	<ctype.h>
//#include	<stdint.h>
#include	<time.h>
//#include	<unistd.h>



#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_


#include	"eventlogging.h"
#include	"html_common.h"

#include	"alpacadriver_helper.h"


#define	kDescriptionLen	96
#define	kResultStrLen	96
#define	kErrorStrLen	96
//**************************************************************************
typedef struct
{
	time_t				eventTime;
	char				eventName[64];
	char				eventDescription[kDescriptionLen];
	char				resultString[kResultStrLen];
	TYPE_ASCOM_STATUS	alpacaErrCode;
	char				errorString[kErrorStrLen];
} TYPE_EVENTLOG;

#define	kMaxLogEntries	300

TYPE_EVENTLOG	gEventLog[kMaxLogEntries];
int				gLogIndex	=	0;

//**************************************************************************
//*	if the log files up, we will dump the first half and continue
static void	FlushHalfLog(void)
{
int		iii;
int		halfSize;

	halfSize	=	kMaxLogEntries / 2;
	for (iii=0; iii<halfSize; iii++)
	{
		gEventLog[iii]	=	gEventLog[iii + halfSize];
	}
	gLogIndex	=	halfSize;

}

//**************************************************************************
void	LogEvent(	const char				*eventName,
					const char				*eventDescription,
					const char				*resultString,
					const TYPE_ASCOM_STATUS	alpacaErrCode,
					const char				*errorString)
{
	if (gLogIndex < kMaxLogEntries)
	{
		memset(&gEventLog[gLogIndex], 0, sizeof(TYPE_EVENTLOG));
		gEventLog[gLogIndex].eventTime		=	time(NULL);
		gEventLog[gLogIndex].alpacaErrCode	=	alpacaErrCode;

		if (eventName != NULL)
		{
			strcpy(gEventLog[gLogIndex].eventName,	eventName);
		}
		if (eventDescription != NULL)
		{
			strncpy(gEventLog[gLogIndex].eventDescription,	eventDescription, (kDescriptionLen -1));
			gEventLog[gLogIndex].eventDescription[kDescriptionLen -1]	=	0;
		}
		if (resultString != NULL)
		{
			strcpy(gEventLog[gLogIndex].resultString,	resultString);
		}
		if (errorString != NULL)
		{
			strcpy(gEventLog[gLogIndex].errorString,	errorString);
		}
		gLogIndex++;

		//*	check to see if the log is full
		if (gLogIndex >= kMaxLogEntries)
		{
			FlushHalfLog();
		}
	}
}

//**************************************************************************
void	PrintLog(void)
{
int			ii;
struct tm	*linuxTime;

	for (ii=0; ii<gLogIndex; ii++)
	{
		linuxTime		=	localtime(&gEventLog[ii].eventTime);
		printf("%d/%d/%d %02d:%02d:%02d\t",
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								(1900 + linuxTime->tm_year),
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);
		printf("%-20s\t",	gEventLog[ii].eventName);
		printf("%-20s\t",	gEventLog[ii].eventDescription);
		printf("%-20s\t",	gEventLog[ii].resultString);
		printf("%-20s\t",	gEventLog[ii].errorString);
		printf("\r\n");

	}
}


#pragma mark -

//*****************************************************************************
const char	gHtmlHeaderLog[]	=
{
	"HTTP/1.0 200 \r\n"
//	"Server: alpaca\r\n"
//	"Mime-Version: 1.0\r\n"
	"User-Agent: AlpacaPi\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"Access-Control-Allow-Origin: *\r\n"
	"Access-Control-Allow-Headers: *\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<html lang=\"en\">\r\n"
	"<head>\r\n"
	"<meta charset=\"UTF-8\">\r\n"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
	"<style>\r\n"
	"* { box-sizing: border-box; margin: 0; padding: 0; }\r\n"
	"body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif; background: #0a0a0a; color: #e0e0e0; line-height: 1.6; padding: 20px; }\r\n"
	"h1, h2, h3 { color: #4a9eff; margin: 1em 0 0.5em 0; }\r\n"
	"h1 { font-size: 2em; border-bottom: 2px solid #4a9eff; padding-bottom: 0.3em; }\r\n"
	"h2 { font-size: 1.5em; }\r\n"
	"h3 { font-size: 1.2em; color: #6bb6ff; }\r\n"
	"a { color: #4a9eff; text-decoration: none; transition: color 0.2s; }\r\n"
	"a:hover { color: #6bb6ff; text-decoration: underline; }\r\n"
	"a:visited { color: #8a7fff; }\r\n"
	"table { border-collapse: collapse; width: 100%; margin: 1em 0; background: #1a1a1a; border: 1px solid #333; }\r\n"
	"th { background: #2a2a2a; color: #ffd700; padding: 12px; text-align: left; border: 1px solid #444; font-weight: 600; }\r\n"
	"td { padding: 10px; border: 1px solid #333; }\r\n"
	"tr:nth-child(even) { background: #151515; }\r\n"
	"tr:hover { background: #252525; }\r\n"
	".text-center { text-align: center; }\r\n"
	".text-right { text-align: right; }\r\n"
	".status-enabled { color: #4caf50; font-weight: 600; }\r\n"
	".status-disabled { color: #f44336; font-weight: 600; }\r\n"
	".container { max-width: 1200px; margin: 0 auto; }\r\n"
	".section { margin: 2em 0; padding: 1.5em; background: #151515; border-radius: 8px; border: 1px solid #333; }\r\n"
	"hr { border: none; border-top: 2px solid #4a9eff; margin: 2em 0; }\r\n"
	"ul, ol { margin: 1em 0; padding-left: 2em; }\r\n"
	"li { margin: 0.5em 0; }\r\n"
	"p { margin: 1em 0; }\r\n"
	".info-text { color: #aaa; font-size: 0.9em; }\r\n"
	".badge { display: inline-block; padding: 4px 8px; border-radius: 4px; font-size: 0.85em; font-weight: 600; }\r\n"
	".badge-success { background: #4caf50; color: #fff; }\r\n"
	".badge-error { background: #f44336; color: #fff; }\r\n"
	".badge-info { background: #2196f3; color: #fff; }\r\n"
	"code { background: #2a2a2a; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; color: #ffd700; }\r\n"
	"</style>\r\n"

};

//*****************************************************************************
const char	gHtmlTitleLog[]	=
{
	"<title>Alpaca command log</title>\r\n"
	"</head>\r\n"
	"<body>\r\n"
	"<div class=\"container\">\r\n"
	"<header>\r\n"
	"<h1>Alpaca command log</h1>\r\n"
	"</header>\r\n"

};


#define	kMaxErrors	256
//*****************************************************************************
void	SendHtmlLog(int mySocketFD)
{
char		lineBuff[256];
int			iii;
struct tm	*linuxTime;
int			errorTotal;
int			errorCounts[kMaxErrors];
int			errIndx;

	for (errIndx=0; errIndx<kMaxErrors; errIndx++)
	{
		errorCounts[errIndx]	=	0;
	}
	errorTotal	=	0;

	SocketWriteData(mySocketFD,	gHtmlHeaderLog);
	SocketWriteData(mySocketFD,	gHtmlTitleLog);

	//-------------------------------------------------------------------
	SocketWriteData(mySocketFD,	"<section class=\"section\">\r\n");
	SocketWriteData(mySocketFD,	"<table>\r\n");
	SocketWriteData(mySocketFD,	"<thead><tr>\r\n");
	SocketWriteData(mySocketFD,	"<th>Date/Time</th>\r\n");
	SocketWriteData(mySocketFD,	"<th>Device</th>\r\n");
	SocketWriteData(mySocketFD,	"<th>Command</th>\r\n");
	SocketWriteData(mySocketFD,	"<th class=\"text-center\">Err #</th>\r\n");
	SocketWriteData(mySocketFD,	"<th>Error/Comment</th>\r\n");
	SocketWriteData(mySocketFD,	"</tr></thead>\r\n");
	SocketWriteData(mySocketFD,	"<tbody>\r\n");
	for (iii=0; iii<gLogIndex; iii++)
	{
		SocketWriteData(mySocketFD,	"<tr>\r\n");
		linuxTime		=	localtime(&gEventLog[iii].eventTime);
		sprintf(lineBuff, "\t<td>%d/%d/%d %02d:%02d:%02d</td>",
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								(1900 + linuxTime->tm_year),
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);
		SocketWriteData(mySocketFD,	lineBuff);

		sprintf(lineBuff, "<td>%s</td>",	gEventLog[iii].eventName);
		SocketWriteData(mySocketFD,	lineBuff);


		sprintf(lineBuff, "<td>%s</td>",	gEventLog[iii].eventDescription);
		SocketWriteData(mySocketFD,	lineBuff);

		if (gEventLog[iii].alpacaErrCode != 0)
		{
			sprintf(lineBuff, "<td class=\"text-center\">0x%03X/%d</td>",	gEventLog[iii].alpacaErrCode, gEventLog[iii].alpacaErrCode);

			errorTotal++;
			errIndx	=	gEventLog[iii].alpacaErrCode - kASCOM_Err_NotImplemented;
			if ((errIndx >= 0) && (errIndx < kMaxErrors))
			{
				errorCounts[errIndx]++;
			}
		}
		else
		{
			strcpy(lineBuff, "<td class=\"text-center\">-</td>");
		}
		SocketWriteData(mySocketFD,	lineBuff);

		sprintf(lineBuff, "<td>%s</td>",	gEventLog[iii].errorString);
		SocketWriteData(mySocketFD,	lineBuff);


		SocketWriteData(mySocketFD,	"</tr>\r\n");
	}

	SocketWriteData(mySocketFD,	"<tr>\r\n");
	sprintf(lineBuff, "<td colspan=\"5\" class=\"info-text\">Total entries %d, max=%d</td>",	gLogIndex, kMaxLogEntries);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</tr>\r\n");

	SocketWriteData(mySocketFD,	"</tbody>\r\n");
	SocketWriteData(mySocketFD,	"</table>\r\n");
	SocketWriteData(mySocketFD,	"</section>\r\n");
	SocketWriteData(mySocketFD,	"<p></p>\r\n");

	//--------------------------------------------------------------------
	//*	print out the error code meanings
	SocketWriteData(mySocketFD,	"<section class=\"section\">\r\n");
	SocketWriteData(mySocketFD,	"<h3>Error Counts</h3>\r\n");
	SocketWriteData(mySocketFD,	"<table>\r\n");
	SocketWriteData(mySocketFD,	"<thead><tr>\r\n");
	SocketWriteData(mySocketFD,	"<th>Error Code</th>\r\n");
	SocketWriteData(mySocketFD,	"<th>Error Name</th>\r\n");
	SocketWriteData(mySocketFD,	"<th class=\"text-center\">Count</th>\r\n");
	SocketWriteData(mySocketFD,	"</tr></thead>\r\n");
	SocketWriteData(mySocketFD,	"<tbody>\r\n");

		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_NotImplemented, kASCOM_Err_NotImplemented);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>NotImplemented</td>");

		errIndx	=	kASCOM_Err_NotImplemented - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");

		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_InvalidValue, kASCOM_Err_InvalidValue);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>InvalidValue</td>");
		errIndx	=	kASCOM_Err_InvalidValue - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");


		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_ValueNotSet, kASCOM_Err_ValueNotSet);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>ValueNotSet</td>");
		errIndx	=	kASCOM_Err_ValueNotSet - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");


		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_NotConnected, kASCOM_Err_NotConnected);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>NotConnected</td>");
		errIndx	=	kASCOM_Err_NotConnected - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");


		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_InvalidWhileParked, kASCOM_Err_InvalidWhileParked);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>InvalidWhileParked</td>");
		errIndx	=	kASCOM_Err_InvalidWhileParked - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");


		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_InvalidWhileSlaved, kASCOM_Err_InvalidWhileSlaved);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>InvalidWhileSlaved</td>");
		errIndx	=	kASCOM_Err_InvalidWhileSlaved - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");


		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_InvalidOperation, kASCOM_Err_InvalidOperation);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>InvalidOperation</td>");
		errIndx	=	kASCOM_Err_InvalidOperation - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");

		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_ActionNotImplemented, kASCOM_Err_ActionNotImplemented);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>ActionNotImplemented</td>");
		errIndx	=	kASCOM_Err_ActionNotImplemented - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");

		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_NotInCacheException, kASCOM_Err_NotInCacheException);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>NotInCacheException</td>");
		errIndx	=	kASCOM_Err_NotInCacheException - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");

		SocketWriteData(mySocketFD,	"<tr>\r\n");
		sprintf(lineBuff, "<td>0x%03X/%d</td>",	kASCOM_Err_UnspecifiedError, kASCOM_Err_UnspecifiedError);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<td>UnspecifiedError</td>");
		errIndx	=	kASCOM_Err_UnspecifiedError - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<td class=\"text-center\">%d</td>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</tr>\r\n");

	SocketWriteData(mySocketFD,	"</tbody>\r\n");
	SocketWriteData(mySocketFD,	"</table>\r\n");
	SocketWriteData(mySocketFD,	"</section>\r\n");

	SocketWriteData(mySocketFD,	"</div>\r\n");
	SocketWriteData(mySocketFD,	"</body></html>\r\n");

}
