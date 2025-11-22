//**************************************************************************
//*	Name:			rotatordriver_CAA.h
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<JT>	=	Joey Troy
//*****************************************************************************
//*	Nov 16,	2025	<JT> Created rotatordriver_CAA.h
//**************************************************************************
//#include	"rotatordriver_CAA.h"


#ifndef _ROTATOR_DRIVER_CAA_H_
#define	_ROTATOR_DRIVER_CAA_H_

#ifndef	_ROTOR_DRIVER_H_
	#include	"rotatordriver.h"
#endif

#ifndef CAA_API_H
	#include	"CAA_API.h"		//*	ZWO header file for CAA
#endif


int	CreateRotatorObjects_CAA(void);


//**************************************************************************************
class RotatorDriverCAA: public RotatorDriver
{
	public:

		//
		// Construction
		//
						RotatorDriverCAA(const int caa_ID_num);
		virtual			~RotatorDriverCAA(void);
		virtual	int32_t	RunStateMachine(void);
//
		virtual	int32_t				ReadCurrentPoisiton_steps(void);
		virtual	double				ReadCurrentPoisiton_degs(void);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_steps(const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_degs(const double newPosition);

		virtual	TYPE_ASCOM_STATUS	HaltMovement(void);
		virtual	bool				IsRotatorMoving(void);

		TYPE_ASCOM_STATUS	Get_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		bool			OpenRotatorConnection(void);		//*	returns true if open succeeded.
		void			ProcessCAAerror(const int caa_ErrorCode, const char *functionName, const char *errorMssg);
//
//
		int				cCAA_ID_num;
		CAA_INFO		cCAAInfo;
		bool			cCAAconnectionIsOpen;
		float			cMinDegree;
		float			cMaxDegree;


};


#endif	//	_ROTATOR_DRIVER_CAA_H_

