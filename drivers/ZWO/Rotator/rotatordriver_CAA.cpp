//**************************************************************************
//*	Name:			rotatordriver_CAA.cpp
//*
//*	Author:			Joey Troy (C) 2025
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<JT>	=	Joey Troy
//*****************************************************************************
//*	Nov 16,	2025	<JT> Created rotatordriver_CAA.cpp
//*****************************************************************************

#ifdef _ENABLE_ROTATOR_CAA_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>
#include	<dirent.h>
#include	<unistd.h>		//*	for usleep()
#include	<mutex>


#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"CAA_API.h"		//*	ZWO header file for CAA

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"rotatordriver.h"
#include	"rotatordriver_CAA.h"
#include	"eventlogging.h"


//*****************************************************************************
//*	CAA Error Code Messages
//*****************************************************************************
const char	*gZWO_CAA_errorMsgs[]	=
{
	"CAA_SUCCESS",
	"CAA_ERROR_INVALID_INDEX",
	"CAA_ERROR_INVALID_ID",
	"CAA_ERROR_INVALID_VALUE",
	"CAA_ERROR_REMOVED",		//failed to find the caa, maybe the caa has been removed
	"CAA_ERROR_MOVING",			//caa is moving
	"CAA_ERROR_ERROR_STATE",	//caa is in error state
	"CAA_ERROR_GENERAL_ERROR",	//other error
	"CAA_ERROR_NOT_SUPPORTED",
	"CAA_ERROR_CLOSED",
	"CAA_ERROR_OUT_RANGE",		//超过 0- 360范围
	"CAA_ERROR_OVER_LIMIT",		//超过限位
	"CAA_ERROR_STALL",			//堵转
	"CAA_ERROR_TIMEOUT",		//超时
	"CAA_ERROR_END"
};

static std::recursive_mutex	gCAASDKmutex;

//*****************************************************************************
int		CreateRotatorObjects_CAA(void)
{
int		iii;
int		caa_count;
char	rulesFileName[]	=	"caa.rules";
bool	rulesFileOK;
char	driverVersionString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK == false)
	{
		LogEvent(	"rotator",
					"Problem with ZWO CAA rules file",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	strcpy(driverVersionString,	CAAGetSDKVersion());
	LogEvent(	"rotator",
				"Library version (ZWO-CAA)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("rotator", "ZWO-CAA", driverVersionString);
	AddSupportedDevice(kDeviceType_Rotator, "ZWO", "CAA", driverVersionString);

	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_count	=	CAAGetNum();
	}
	CONSOLE_DEBUG_W_NUM("caa_count\t=", caa_count);
	if (caa_count > 0)
	{
		char	countMsg[64];
		sprintf(countMsg, "ZWO CAA devices found: %d", caa_count);
		LogEvent(	"rotator",
					countMsg,
					NULL,
					kASCOM_Err_Success,
					NULL);
		for (iii=0; iii < caa_count; iii++)
		{
			CONSOLE_DEBUG_W_NUM("Creating CAA rotator driver for device index\t=", iii);
			new RotatorDriverCAA(iii);
		}
	}
	else
	{
		LogEvent(	"rotator",
					"No ZWO CAA devices found",
					NULL,
					kASCOM_Err_Success,
					NULL);
		CONSOLE_DEBUG("No ZWO CAA devices detected");
	}

	return(1);
}

//**************************************************************************************
RotatorDriverCAA::RotatorDriverCAA(const int caa_ID_num)
	:RotatorDriver(0)
{
	CONSOLE_DEBUG_W_NUM(__FUNCTION__, caa_ID_num);

	strcpy(cCommonProp.Name,		"AlpacaPi Rotator ZWO CAA");
	strcpy(cCommonProp.Description,	"AlpacaPi Rotator ZWO CAA");
	strcpy(cRotatorManufacturer,	"ZWO");
	strcpy(cRotatorModel,			"CAA");
	cRotatorProp.CanReverse		=	true;		//*	CAA supports reverse direction
	cRotatorProp.Reverse			=	false;
	cRotatorProp.Position			=	0.0;
	cRotatorProp.MechanicalPosition	=	0.0;
	cRotatorProp.StepSize			=	0.1;		//*	Step size in degrees
	cRotatorProp.TargetPosition		=	0.0;
	cRotatorProp.IsMoving			=	false;
	cRotatorProp.SyncOffset			=	0.0;

	cRotatorStepsPerRev				=	3600;		//*	360 degrees * 10 steps per degree
	strcpy(cDeviceManufacturer,	"ZWO");

	cUUID.part1						=	'ZWO_';					//*	4 byte manufacturer code

	cCAAconnectionIsOpen			=	false;
	cCAA_ID_num						=	caa_ID_num;
	cMinDegree						=	0.0;
	cMaxDegree						=	360.0;
	
	//*	Attempt to open connection during construction
	//*	Per SDK manual: CAAGetNum() -> CAAGetID() -> CAAGetProperty() -> CAAOpen()
	if (!OpenRotatorConnection())
	{
		CONSOLE_DEBUG("Failed to open CAA connection during construction");
		LogEvent(	"rotator",
					"ZWO CAA connection failed during initialization",
					NULL,
					kASCOM_Err_NotConnected,
					NULL);
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
RotatorDriverCAA::~RotatorDriverCAA(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		CAAClose(cCAAInfo.ID);
		cCAAconnectionIsOpen	=	false;
	}
}

//*****************************************************************************
//*	returns true if open succeeded.
bool	RotatorDriverCAA::OpenRotatorConnection(void)
{
int		caa_RetCode;
float	currentPos;
float	maxDegree;

//	CONSOLE_DEBUG(__FUNCTION__);

	std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);

	//*	if connection was previously open, try to close it first
	if (cCAAconnectionIsOpen)
	{
		CAAClose(cCAAInfo.ID);
		cCAAconnectionIsOpen	=	false;
	}

	//*	Follow SDK recommended call flow:
	//*	1. CAAGetID() - Get device ID
	//*	2. CAAGetProperty() - Get device properties (can be called before open per SDK manual)
	//*	3. CAAOpen() - Open device
	caa_RetCode	=	CAAGetID(cCAA_ID_num, &cCAAInfo.ID);
	if (caa_RetCode != CAA_SUCCESS)
	{
		CONSOLE_DEBUG_W_NUM("CAAGetID() failed, caa_RetCode\t=",	caa_RetCode);
		LogEvent(	"rotator",
					"ZWO CAA CAAGetID() failed",
					NULL,
					kASCOM_Err_UnspecifiedError,
					NULL);
		cCAAconnectionIsOpen	=	false;
		return(false);
	}

	CONSOLE_DEBUG_W_NUM("CAAGetID() succeeded, device ID\t=", cCAAInfo.ID);

	//*	Try to get property before opening (per SDK manual recommendation)
	//*	Note: This may fail if property requires device to be open, but we'll try anyway
	caa_RetCode	=	CAAGetProperty(cCAAInfo.ID, &cCAAInfo);
	if (caa_RetCode == CAA_SUCCESS)
	{
		CONSOLE_DEBUG_W_STR("CAAGetProperty() succeeded, Name\t=", cCAAInfo.Name);
		CONSOLE_DEBUG_W_NUM("CAAGetProperty() MaxStep\t=", cCAAInfo.MaxStep);
		strncpy(cRotatorModel, cCAAInfo.Name, sizeof(cRotatorModel) - 1);
		cRotatorModel[sizeof(cRotatorModel) - 1]	=	'\0';
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("CAAGetProperty() before open returned\t=",	caa_RetCode);
		//*	Property might require device to be open, we'll try again after opening
	}

	//*	Open the device
	caa_RetCode	=	CAAOpen(cCAAInfo.ID);
	if (caa_RetCode == CAA_SUCCESS)
	{
		cCAAconnectionIsOpen			=	true;
		CONSOLE_DEBUG_W_NUM("CAA device opened successfully, ID\t=", cCAAInfo.ID);
		LogEvent(	"rotator",
					"ZWO CAA device opened",
					NULL,
					kASCOM_Err_Success,
					NULL);

		//*	If property wasn't retrieved before opening, try again now
		if (cRotatorModel[0] == '\0')
		{
			caa_RetCode	=	CAAGetProperty(cCAAInfo.ID, &cCAAInfo);
			if (caa_RetCode == CAA_SUCCESS)
			{
				CONSOLE_DEBUG_W_STR("CAAGetProperty() after open, Name\t=", cCAAInfo.Name);
				strncpy(cRotatorModel, cCAAInfo.Name, sizeof(cRotatorModel) - 1);
				cRotatorModel[sizeof(cRotatorModel) - 1]	=	'\0';
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("CAAGetProperty() after open returned\t=",	caa_RetCode);
			}
		}

		//*	Get min and max degree limits
		//*	Note: CAAMinDegree may not be available in all SDK versions, so we use default 0.0
		cMinDegree	=	0.0;	//*	Default minimum degree

		caa_RetCode	=	CAAGetMaxDegree(cCAAInfo.ID, &maxDegree);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cMaxDegree	=	maxDegree;
		}
		else
		{
			cMaxDegree	=	360.0;	//*	Default maximum degree if call fails
		}

		//*	Get current position
		caa_RetCode	=	CAAGetDegree(cCAAInfo.ID, &currentPos);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cRotatorProp.Position			=	currentPos;
			cRotatorProp.MechanicalPosition	=	currentPos;
			cRotatorProp.TargetPosition		=	currentPos;
			CONSOLE_DEBUG_W_DBL("Initial position (degrees)\t=", currentPos);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("CAAGetDegree() returned\t=",	caa_RetCode);
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetDegree() failed:");
		}

		//*	Check if device is already moving (shouldn't be, but verify)
		bool	bMoving;
		bool	pbHandControl;
		caa_RetCode	=	CAAIsMoving(cCAAInfo.ID, &bMoving, &pbHandControl);
		if (caa_RetCode == CAA_SUCCESS)
		{
			CONSOLE_DEBUG_W_NUM("Device moving status\t=", bMoving);
			CONSOLE_DEBUG_W_NUM("Hand control active\t=", pbHandControl);
			if (pbHandControl)
			{
				CONSOLE_DEBUG("WARNING: Device is under hand control - software commands may not work!");
			}
		}

		//*	Get reverse direction setting
		bool	reverseValue;
		caa_RetCode	=	CAAGetReverse(cCAAInfo.ID, &reverseValue);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cRotatorProp.Reverse	=	reverseValue;
		}
	}
	else
	{
		cCAAconnectionIsOpen			=	false;
		ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAOpen() failed:");
		CONSOLE_DEBUG_W_NUM("caa_RetCode\t=",	caa_RetCode);
	}
	return(cCAAconnectionIsOpen);
}

//*****************************************************************************
int32_t	RotatorDriverCAA::RunStateMachine(void)
{
bool		bMoving			=	false;
bool		pbHandControl	=	false;
int			caa_RetCode;
float		currentPos;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCAAconnectionIsOpen == false)
	{
		//*	something has happened and we lost connection
		int		caa_count;
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_count	=	CAAGetNum();
		if (caa_count > 0)
		{
			OpenRotatorConnection();
		}
		return(1000 * 1000);	//*	return early if connection not open
	}

	//---------------------------------------------------------------------
	//*	only call CAA functions if connection is open
	//*	Note: During movement, the device may not respond to queries immediately
	//*	so we handle errors gracefully
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAIsMoving(cCAAInfo.ID, &bMoving, &pbHandControl);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cRotatorProp.IsMoving	=	bMoving;
		}
		else
		{
			//*	If device is moving, errors might be due to device being busy
			//*	Don't mark connection as closed unless it's a critical error
			if (caa_RetCode == CAA_ERROR_GENERAL_ERROR || caa_RetCode == CAA_ERROR_TIMEOUT)
			{
				//*	Device might be busy moving - don't treat as fatal
				//*	Keep IsMoving state as-is
			}
			else
			{
				ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAIsMoving() failed:");
			}
		}
	}

	//---------------------------------------------------------------------
	//*	only call CAA functions if connection is open
	//*	Try to get position, but handle errors gracefully during movement
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAGetDegree(cCAAInfo.ID, &currentPos);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cRotatorProp.MechanicalPosition	=	currentPos;
			cRotatorProp.Position			=	currentPos + cRotatorProp.SyncOffset;
			if (cRotatorProp.IsMoving == false)
			{
				//*	When idle the target must represent the actual position so relative
				//*	commands base their offset on the true hardware angle.
				cRotatorProp.TargetPosition	=	cRotatorProp.Position;
			}
		}
		else
		{
			//*	If device is moving, errors might be due to device being busy
			//*	Don't mark connection as closed unless it's a critical error
			if (caa_RetCode == CAA_ERROR_GENERAL_ERROR || caa_RetCode == CAA_ERROR_TIMEOUT)
			{
				//*	Device might be busy moving - don't treat as fatal
				//*	Position will be updated on next successful read
			}
			else
			{
				ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetDegree() failed:");
			}
		}
	}

	return(1000 * 1000);
}

//*****************************************************************************
int32_t	RotatorDriverCAA::ReadCurrentPoisiton_steps(void)
{
float		currentPos;
int32_t		position_steps;
int			caa_RetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAGetDegree(cCAAInfo.ID, &currentPos);
		if (caa_RetCode == CAA_SUCCESS)
		{
			//*	Convert degrees to steps (assuming 10 steps per degree)
			position_steps	=	(int32_t)(currentPos * 10.0);
		}
		else
		{
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetDegree() failed:");
			position_steps	=	0;
		}
	}
	else
	{
		position_steps	=	0;
	}

	return(position_steps);
}

//*****************************************************************************
double	RotatorDriverCAA::ReadCurrentPoisiton_degs(void)
{
float		currentPos;
int			caa_RetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAGetDegree(cCAAInfo.ID, &currentPos);
		if (caa_RetCode == CAA_SUCCESS)
		{
			return((double)currentPos);
		}
		else
		{
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetDegree() failed:");
		}
	}

	return(0.0);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriverCAA::SetCurrentPoisiton_steps(const int32_t newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
float				newPosition_degrees;
int					caa_count;

	CONSOLE_DEBUG_W_INT32("newPosition\t=", newPosition);

	//*	ensure connection is open before attempting move
	if (cCAAconnectionIsOpen == false)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_count	=	CAAGetNum();
		if (caa_count > 0)
		{
			OpenRotatorConnection();
		}
		if (cCAAconnectionIsOpen == false)
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			return(alpacaErrCode);
		}
	}

	//*	Convert steps to degrees (assuming 10 steps per degree)
	newPosition_degrees	=	(float)newPosition / 10.0;
	alpacaErrCode			=	SetCurrentPoisiton_degs((double)newPosition_degrees);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriverCAA::SetCurrentPoisiton_degs(const double newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
float				newPosition_float;
int					caa_RetCode;
int					caa_count;

	CONSOLE_DEBUG_W_DBL("newPosition\t=", newPosition);

	//*	ensure connection is open before attempting move
	if (cCAAconnectionIsOpen == false)
	{
		CONSOLE_DEBUG("CAA connection not open, attempting to open...");
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_count	=	CAAGetNum();
		CONSOLE_DEBUG_W_NUM("CAAGetNum() returned\t=", caa_count);
		if (caa_count > 0)
		{
			OpenRotatorConnection();
		}
		if (cCAAconnectionIsOpen == false)
		{
			CONSOLE_DEBUG("Failed to open CAA connection");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			LogEvent(	"rotator",
						"ZWO CAA device not connected",
						NULL,
						kASCOM_Err_NotConnected,
						NULL);
			return(alpacaErrCode);
		}
	}

	//*	Get current position before move (per INDI driver pattern)
	float	currentPosBeforeMove	=	0.0;
	std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
	caa_RetCode	=	CAAGetDegree(cCAAInfo.ID, &currentPosBeforeMove);
	if (caa_RetCode != CAA_SUCCESS)
	{
		CONSOLE_DEBUG_W_NUM("Failed to read current angle before move, code\t=", caa_RetCode);
		ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetDegree() failed:");
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		return(alpacaErrCode);
	}
	CONSOLE_DEBUG_W_DBL("Current position before move\t=", currentPosBeforeMove);
	
	//*	Check if target is very close to current position (per INDI driver: THRESHOLD = 0.1°)
	//*	If so, no movement needed
	const double	THRESHOLD	=	0.1;
	if (fabs((double)currentPosBeforeMove - newPosition) <= THRESHOLD)
	{
		CONSOLE_DEBUG("Target position is very close to current position, no move needed");
		alpacaErrCode	=	kASCOM_Err_Success;
		cRotatorProp.Position			=	currentPosBeforeMove;
		cRotatorProp.MechanicalPosition	=	currentPosBeforeMove;
		return(alpacaErrCode);
	}
	
	//*	Normalize angle to 0-360° range (per SDK: angles are 0-360°)
	//*	The SDK manual states angles are 0-360°, so we normalize here
	double	normalizedPosition	=	newPosition;
	while (normalizedPosition < 0.0)
	{
		normalizedPosition	+=	360.0;
	}
	while (normalizedPosition >= 360.0)
	{
		normalizedPosition	-=	360.0;
	}

	//*	Validate position is within device limits (per INDI driver pattern)
	if (normalizedPosition > cMaxDegree)
	{
		CONSOLE_DEBUG_W_DBL("Target angle exceeds max limit, normalizedPosition\t=", normalizedPosition);
		CONSOLE_DEBUG_W_DBL("cMaxDegree\t=", cMaxDegree);
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		LogEvent(	"rotator",
					"ZWO CAA position exceeds max limit",
					NULL,
					kASCOM_Err_InvalidValue,
					NULL);
		return(alpacaErrCode);
	}

	newPosition_float	=	(float)normalizedPosition;
	
	CONSOLE_DEBUG_W_DBL("Calling CAAMoveTo(), target position (degrees)\t=", newPosition_float);
	CONSOLE_DEBUG_W_NUM("CAA connection open\t=", cCAAconnectionIsOpen);
	CONSOLE_DEBUG_W_NUM("CAA device ID\t=", cCAAInfo.ID);
	CONSOLE_DEBUG_W_NUM("CAA MaxStep\t=", cCAAInfo.MaxStep);
	
	//*	Check device status before move (per INDI driver pattern)
	bool	bMoving;
	bool	pbHandControl;
	caa_RetCode	=	CAAIsMoving(cCAAInfo.ID, &bMoving, &pbHandControl);
	if (caa_RetCode == CAA_SUCCESS)
	{
		CONSOLE_DEBUG_W_NUM("Device moving status before move\t=", bMoving);
		CONSOLE_DEBUG_W_NUM("Hand control active before move\t=", pbHandControl);
		
		if (pbHandControl)
		{
			CONSOLE_DEBUG("WARNING: Device is under hand control - move command may fail!");
			LogEvent(	"rotator",
						"ZWO CAA device under hand control",
						NULL,
						kASCOM_Err_UnspecifiedError,
						NULL);
			//*	Don't return error - try the move anyway, device might accept it
		}
		
		if (bMoving)
		{
			CONSOLE_DEBUG("Device is already moving - stopping current movement before new command");
			//*	Stop any existing movement before starting new one
			//*	This ensures clean state for the new movement command
			CAAStop(cCAAInfo.ID);
			usleep(100000);	//*	100ms delay to allow stop to complete
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("CAAIsMoving() before move returned\t=", caa_RetCode);
		//*	Continue anyway - the move command might still work
	}
	
	//*	Call CAAMoveTo() with the target angle in degrees
	//*	Per INDI driver: passes angle directly as double (converted to float by SDK)
	//*	SDK manual confirms angles are 0-360°, and INDI driver confirms this works
	//*	Per ASCOM Alpaca API: MoveAbsolute(Position) moves to absolute position in degrees
	CONSOLE_DEBUG_W_DBL("Calling CAAMoveTo() with angle (degrees)\t=", newPosition_float);
	CONSOLE_DEBUG_W_DBL("Original newPosition (before normalization)\t=", newPosition);
	caa_RetCode	=	CAAMoveTo(cCAAInfo.ID, newPosition_float);
	CONSOLE_DEBUG_W_NUM("CAAMoveTo() returned\t=", caa_RetCode);
	
	if (caa_RetCode == CAA_SUCCESS)
	{
		alpacaErrCode			=   kASCOM_Err_Success;
		cRotatorProp.IsMoving	=	true;
		cRotatorProp.TargetPosition	=	normalizedPosition;	//*	Use normalized position for target
		CONSOLE_DEBUG("CAAMoveTo() succeeded, rotator should be moving");
		CONSOLE_DEBUG_W_DBL("Target position set to\t=", cRotatorProp.TargetPosition);
		LogEvent(	"rotator",
					"ZWO CAA move command sent",
					NULL,
					kASCOM_Err_Success,
					NULL);
		
		//*	Note: Don't query device immediately after move command
		//*	The device may be busy and USB communication can timeout
		//*	The RunStateMachine() will check status on next cycle
		//*	Per ASCOM: After MoveAbsolute, IsMoving should be true until movement completes
	}
	else
	{
		alpacaErrCode	=   kASCOM_Err_UnspecifiedError;
		ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAMoveTo() failed:");
		CONSOLE_DEBUG_W_NUM("CAAMoveTo() failed, caa_RetCode\t=",	caa_RetCode);
		
		LogEvent(	"rotator",
					"ZWO CAA move command failed",
					NULL,
					kASCOM_Err_UnspecifiedError,
					NULL);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriverCAA::HaltMovement(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					caa_RetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAStop(cCAAInfo.ID);
		if (caa_RetCode == CAA_SUCCESS)
		{
			alpacaErrCode			=	kASCOM_Err_Success;
			cRotatorProp.IsMoving	=	false;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAStop() failed:");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
bool	RotatorDriverCAA::IsRotatorMoving(void)
{
bool		bMoving			=	false;
bool		pbHandControl	=	false;
int			caa_RetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAIsMoving(cCAAInfo.ID, &bMoving, &pbHandControl);
		if (caa_RetCode == CAA_SUCCESS)
		{
			return(bMoving);
		}
		else
		{
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAIsMoving() failed:");
		}
	}

	return(false);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriverCAA::Get_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					caa_RetCode;
bool				reverseValue;

	//*	Use SDK function to get current reverse direction setting
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAAGetReverse(cCAAInfo.ID, &reverseValue);
		if (caa_RetCode == CAA_SUCCESS)
		{
			cRotatorProp.Reverse	=	reverseValue;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAAGetReverse() failed:");
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Failed to get reverse direction");
			reverseValue	=	false;	//*	Return false on error
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Rotator not connected");
		reverseValue	=	false;
	}

	//*	Return the value
	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							reverseValue,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriverCAA::Put_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
char				argumentString[32];
bool				newReverseValue;
int					caa_RetCode;

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Reverse",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		//*	Parse boolean value using helper function
		newReverseValue	=	IsTrueFalse(argumentString);
	if (cCAAconnectionIsOpen)
	{
		std::lock_guard<std::recursive_mutex>	sdkLock(gCAASDKmutex);
		caa_RetCode	=	CAASetReverse(cCAAInfo.ID, newReverseValue);
			if (caa_RetCode == CAA_SUCCESS)
			{
				cRotatorProp.Reverse	=	newReverseValue;
				alpacaErrCode			=	kASCOM_Err_Success;
			}
			else
			{
				ProcessCAAerror(caa_RetCode, __FUNCTION__, "CAASetReverse() failed:");
				alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Failed to set reverse direction");
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Rotator not connected");
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Reverse' not specified");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
void	RotatorDriverCAA::ProcessCAAerror(const int caa_ErrorCode, const char *functionName, const char *errorMssg)
{

	switch(caa_ErrorCode)
	{
		case CAA_SUCCESS:
			break;

		case CAA_ERROR_REMOVED:			//failed to find the caa, maybe the caa has been removed
		case CAA_ERROR_INVALID_ID:
		case CAA_ERROR_CLOSED:			//connection is closed, need to reopen
			cCAAconnectionIsOpen	=	false;
			break;


		case CAA_ERROR_INVALID_INDEX:
		case CAA_ERROR_INVALID_VALUE:
		case CAA_ERROR_MOVING:			//caa is moving
		case CAA_ERROR_ERROR_STATE:		//caa is in error state
		case CAA_ERROR_GENERAL_ERROR:	//other error
		case CAA_ERROR_NOT_SUPPORTED:
		case CAA_ERROR_OUT_RANGE:		//超过 0- 360范围
		case CAA_ERROR_OVER_LIMIT:		//超过限位
		case CAA_ERROR_STALL:			//堵转
		case CAA_ERROR_TIMEOUT:			//超时
		case CAA_ERROR_END:
		default:
			CONSOLE_DEBUG_W_NUM("CAA error code\t=", caa_ErrorCode);
			if ((caa_ErrorCode >= 0) && (caa_ErrorCode <= CAA_ERROR_TIMEOUT))
			{
				CONSOLE_DEBUG_W_STR("CAA error code\t=", gZWO_CAA_errorMsgs[caa_ErrorCode]);
			}
			break;
	}
}


#endif	//	_ENABLE_ROTATOR_CAA_
