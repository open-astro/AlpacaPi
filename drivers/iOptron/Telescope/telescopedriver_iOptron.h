//**************************************************************************
//*	Name:			telescopedriver_iOptron.h
//*
//*	Author:			Mark Sproul (C) 2024
//*					Joey Troy (C) 2025
//*
//*	Description:	iOptron Mount Telescope Driver
//*					Supports all ASCOM Alpaca Telescope API methods
//*					Based on iOptron RS232 Command Language
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:		https://ascom-standards.org/api/
//*					https://www.ioptron.com/Articles.asp?ID=295
//*					INDIGO iOptron driver reference
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<JT>	=	Joey Troy
//*****************************************************************************
//*	Dec 20,	2024	<MLS> Created telescopedriver_iOptron.h
//*	Nov 11,	2025	<JT>  Initial implementation based on LX200 pattern
//*	Nov 11,	2025	<JT>  Adapted for iOptron command protocol
//*****************************************************************************
//#include	"telescopedriver_iOptron.h"

#ifndef _TELESCOPE_DRIVER_IOPTRON_H_
#define	_TELESCOPE_DRIVER_IOPTRON_H_

#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

#ifndef _TELESCOPE_DRIVER_COMM_H_
	#include	"telescopedriver_comm.h"
#endif

void	CreateTelescopeObjects_iOptron(void);

//**************************************************************************************
class TelescopeDriveriOptron: public TelescopeDriverComm
{
	public:

		//
		// Construction
		//
								TelescopeDriveriOptron(	DeviceConnectionType	connectionType,
														const char				*devicePath);
		virtual					~TelescopeDriveriOptron(void);
		virtual	int32_t			RunStateMachine(void);
		virtual	bool			AlpacaDisConnect(void);

		virtual	bool			SendCmdsFromQueue(void);
		virtual	bool			SendCmdsPeriodic(void);

		//*************************************************************************
		//*	DO NOT IMPLEMENT THE SYNCHRONOUS METHODS
		//*		Use the ASYNC methods instead
		//*		Alpaca cannot do synchronous and ASCOM/ALPACA are trying to eliminate all SYNC commands
		//*************************************************************************

		//--------------------------------------------------------------------------------------------------
		//*	these routines should be implemented by the sub-classes
		//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(	char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_FindHome(		char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_MoveAxis(		const int axisNum,
															const double moveRate_degPerSec,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_Park(			char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SetPark(		char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToAltAz(	const double	newAlt_Degrees,
															const double	newAz_Degrees,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingOnOff(const bool newTrackingState,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingRate(	TYPE_DriveRates newTrackingRate,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_UnPark(		char *alpacaErrMsg);

		//*	iOptron specific command processing
		bool					Process_iOptronResponse(char *dataBuffer);
		bool					Process_RA_Response(char *dataBuffer);
		bool					Process_DEC_Response(char *dataBuffer);
		bool					Process_Status_Response(char *dataBuffer);
		bool					Process_GEP_Response(char *dataBuffer);	//*	Process :GEP# response (RA and DEC)
		bool					Process_GLS_Response(char *dataBuffer);	//*	Process :GLS# response (status)

		//*	Setup support
		virtual	bool			Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString);
		virtual	bool			Setup_ProcessKeyword(const char *keyword, const char *valueString);
		virtual	void			Setup_SaveInit(void);
		virtual	void			Setup_SaveFinish(void);
		
		//*	Config file support
		void					ReadIOptronConfig(void);
		void					WriteIOptronConfig(void);

		//*	communication state
		bool					cTelescopeInfoValid;
		int						cIOptron_CommErrCnt;
		char					cTelescopeRA_String[32];
		char					cTelescopeDEC_String[32];
		char					cTelescopeStatus_String[64];
		bool					cWaitingForResponse;
		int						cLastCommandID;
		bool					cSetupChangeOccured;

};

#endif // _TELESCOPE_DRIVER_IOPTRON_H_

