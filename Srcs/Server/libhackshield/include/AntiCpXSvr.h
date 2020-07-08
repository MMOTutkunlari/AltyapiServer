#ifndef _ANTICPX_SERVER_H_
#define _ANTICPX_SERVER_H_

// ��ȿ���� ���� �ڵ��� �ǹ��Ѵ�
#define ANTICPX_INVALID_HANDLE_VALUE		(0)

//////////////////////////////////////////////////////////////////////////
// ���� �ڵ�
//////////////////////////////////////////////////////////////////////////

#define ANTICPXSVR_BASECODE_ERROR							0xE9040000

// ���Ͽ� ���� ������ �����ϴ�
#define ERROR_ANTICPXSVR_FILE_ACCESS_DENIED					(ANTICPXSVR_BASECODE_ERROR + 0x1)
// ������ ã�� ���� �����ϴ�
#define ERROR_ANTICPXSVR_FILE_NOT_FOUND						(ANTICPXSVR_BASECODE_ERROR + 0x2)
// �Է°��� �ùٸ��� �ʽ��ϴ�
#define ERROR_ANTICPXSVR_INVALID_PARAMETER					(ANTICPXSVR_BASECODE_ERROR + 0x3)
// �ùٸ� ������ �ƴմϴ�
#define ERROR_ANTICPXSVR_BAD_FORMAT							(ANTICPXSVR_BASECODE_ERROR + 0x4)
// ��û�� ���� ������ ���� �ʾҽ��ϴ�
#define ERROR_ANTICPXSVR_NOT_YET_RECEIVED_RESPONSE			(ANTICPXSVR_BASECODE_ERROR + 0x5)
// ����ϰ� ���� �ʽ��ϴ�
#define ERROR_ANTICPXSVR_NO_WAITING							(ANTICPXSVR_BASECODE_ERROR + 0x6)
// �޸𸮰� �����մϴ�
#define ERROR_ANTICPXSVR_NOT_ENOUGH_MEMORY					(ANTICPXSVR_BASECODE_ERROR + 0x7)
// �ۼ��ŵ� �޽����� �ùٸ��� �ʽ��ϴ�
#define ERROR_ANTICPXSVR_BAD_MESSAGE						(ANTICPXSVR_BASECODE_ERROR + 0x8)
// ��Ŷ �м��� ���� ������ ������ �����Ǿ����ϴ�
#define ERROR_ANTICPXSVR_REPLY_ATTACK						(ANTICPXSVR_BASECODE_ERROR + 0x9)
// �ٽ��� ��� ������ �����Ǿ����ϴ�
#define ERROR_ANTICPXSVR_HSHIELD_FILE_ATTACK				(ANTICPXSVR_BASECODE_ERROR + 0xA)
// Ŭ���̾�Ʈ ���� ������ �����Ǿ����ϴ�
#define ERROR_ANTICPXSVR_CLIENT_FILE_ATTACK					(ANTICPXSVR_BASECODE_ERROR + 0xB)
// �޸� ������ �����Ǿ����ϴ�
#define ERROR_ANTICPXSVR_MEMORY_ATTACK						(ANTICPXSVR_BASECODE_ERROR + 0xC)
// �� ���� Ŭ���̾�Ʈ ������ �����Ǿ����ϴ�
// (���� ���� ���� HSB ������ ������Ʈ�Ͽ��� ��� HSB ��å�� �� ���� Ŭ���̾�Ʈ�� �������� �ʵ���
// �����Ǿ� ���� ��� �� ������ �߻��� �� �ִ�. ��å�� ���� ���̹Ƿ� ����� ������ �ƴϴ�.)
#define ERROR_ANTICPXSVR_OLD_VERSION_CLIENT_EXPIRED			(ANTICPXSVR_BASECODE_ERROR + 0xD)
// HSB ������ ������ �� ������ �ٽ��� ���� ¦�� ���� �ʽ��ϴ�
#define ERROR_ANTICPXSVR_UNKNOWN_CLIENT						(ANTICPXSVR_BASECODE_ERROR + 0xE)
// V3 Small Engine ������ ������ �����Ǿ����ϴ�.
#define ERROR_ANTICPXSVR_V3SENGINE_FILE_ATTACK				(ANTICPXSVR_BASECODE_ERROR + 0xF)
// Nano Engine ������ ������ �����Ǿ����ϴ�.
#define ERROR_ANTICPXSVR_NANOENGINE_FILE_ATTACK				(ANTICPXSVR_BASECODE_ERROR + 0x10)
// HackShield Version�� �������� �ʴ´�.
#define ERROR_ANTICPXSVR_INVALID_HACKSHIELD_VERSION			(ANTICPXSVR_BASECODE_ERROR + 0x11)
// Engine Version�� �������� �ʴ´�.
#define ERROR_ANTICPXSVR_INVALID_ENGINE_VERSION				(ANTICPXSVR_BASECODE_ERROR + 0x12)
// ���� ������Ʈ ������ ���ܹ߻�.
#define ERROR_ANTICPXSVR_CREATE_SVROBJ_EXCEPTION			(ANTICPXSVR_BASECODE_ERROR + 0x13)
// �䱸�޼��� ������ ���ܹ߻�.
#define ERROR_ANTICPXSVR_MAKEREQ_EXCEPTION					(ANTICPXSVR_BASECODE_ERROR + 0x14)
// ����޼��� Ȯ�ν� ���ܹ߻�.
#define ERROR_ANTICPXSVR_VERIFY_EXCEPTION					(ANTICPXSVR_BASECODE_ERROR + 0x15)
//Tracing Information �ۼ� �߿� ���ܹ߻�.
#define ERROR_ANTICPXSVR_TRACE_EXCEPTION					(ANTICPXSVR_BASECODE_ERROR + 0x16)
//CreateClientObject �ۼ� �߿� ���� �߻�.
#define ERROR_ANTICPXSVR_CREATE_CLIENT_OBJECT_EXCEPTION		(ANTICPXSVR_BASECODE_ERROR + 0x17)
//�ٽ��� ���� ���°� ���������� �ʽ��ϴ�.
#define ERROR_ANTICPXSVR_ABNORMAL_HACKSHIELD_STATUS			(ANTICPXSVR_BASECODE_ERROR + 0x18)
//�ٽ��� ���� ���°� ���������� �ʽ��ϴ�.
#define ERROR_ANTICPXSVR_DETECT_CALLBACK_IS_NOTIFIED		(ANTICPXSVR_BASECODE_ERROR + 0x19)
// ����޼���_EX Ȯ�ν� ���ܹ߻�.
#define ERROR_ANTICPXSVR_VERIFYEX_EXCEPTION					(ANTICPXSVR_BASECODE_ERROR + 0x1A)

// ���ǵ��� ���� ����
#define ERROR_ANTICPXSVR_UNKNOWN							(ANTICPXSVR_BASECODE_ERROR + 0xFF)

#if defined (WIN32) || defined(WIN64)
#ifdef AHNHS_EXPORTS
#define AHNHS_API  __declspec(dllexport)
#else
#define AHNHS_API  __declspec(dllimport)	
#endif
#else
#define IN
#define OUT
#define __stdcall
#define AHNHS_API
typedef unsigned long * ULONG_PTR;
#endif

typedef ULONG_PTR AHNHS_SERVER_HANDLE;
typedef ULONG_PTR AHNHS_CLIENT_HANDLE;

#ifndef __ANTICPX_TRANS_BUFFER__
#define __ANTICPX_TRANS_BUFFER__

#define ANTICPX_TRANS_BUFFER_MAX			400

#define ANTICPX_RECOMMAND_CLOSE_SESSION		101
#define ANTICPX_RECOMMAND_KEEP_SESSION		102

// Client-side CallBack Code
#define AHNHS_ACTAPC_DETECT_ALREADYHOOKED			0x010101			
#define AHNHS_ACTAPC_DETECT_AUTOMOUSE				0x010102			
#define AHNHS_ACTAPC_DETECT_AUTOMACRO				0x010104			
#define AHNHS_ACTAPC_DETECT_HOOKFUNCTION			0x010301			 
#define AHNHS_ACTAPC_DETECT_DRIVERFAILED			0x010302			
#define AHNHS_ACTAPC_DETECT_SPEEDHACK				0x010303		//���ǵ��ٷ��� ���α׷��� ���� �ý��� �ð��� ����Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_SPEEDHACK_APP			0x010304		//���ǵ��ٷ��� ���α׷��� ���� �ý��� �ð��� ����Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_MESSAGEHOOK				0x010306			
#define AHNHS_ACTAPC_DETECT_KDTRACE					0x010307		//����� Ʈ���̽��� �߻��߽��ϴ�.(Ŀ�� ����� Ȱ��ȭ, ���� �극��ũ ������ ó��)
#define AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED			0x010308		//[����͸�] ��ġ�� ����� Ʈ���̽��� ����Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_SPEEDHACK_RATIO			0x01030B		
#define AHNHS_ACTAPC_DETECT_INJECTDLL_UNLOADED		0x010401		//�ٽ��� ��ȣ ����� ������ ��ε�Ǿ����ϴ�.
#define AHNHS_ENGINE_DETECT_GAME_HACK				0x010501			
#define AHNHS_ENGINE_DETECT_GENERAL_HACK			0x010502			
#define AHNHS_ENGINE_DETECT_WINDOWED_HACK			0x010503		// â��� ��ŷ��	
#define AHNHS_ACTAPC_DETECT_MULTI_LOADING			0x010504		// Ŭ���̾�Ʈ ��Ƽ �ε� ����.
#define AHNHS_ACTAPC_DETECT_ABNORMAL_MEMORY_ACCESS	0x010601		// �ڵ� ��ġ �õ��� �����Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_MODULE_CHANGE			0x010701		//�ٽ��� ���ø���� ����Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_NANOENGINEFAILED		0x010702		// ���뿣���� ���������� �ʽ��ϴ�.
#define AHNHS_ACTAPC_DETECT_CODEMISMATCH			0x010703		// HookFreeCode�� ���� ������ ���� �Ǿ����ϴ�.
#define AHNHS_ACTAPC_DETECT_PROTECTSCREENFAILED		0x010704		// ProtectScreen(Ex)�� ���������� �ʽ��ϴ�.
#define AHNHS_ACTAPC_DETECT_MEM_MODIFY_FROM_LMP		0x010705
#define AHNHS_ACTAPC_DETECT_LMP_FAILED				0x010706
#define AHNHS_ACTAPC_DETECT_HACKING_PROCESS			0x010707		// Code Signature�� ��ġ�ϴ� ��ŷ ����̹��� ã���� ���
#define AHNHS_ACTAPC_DETECT_THREAD_STOPPING			0x010708		// ��ȣ�Ǵ� �����尡 ���� ����Ǿ��� ���
#define AHNHS_ACTAPC_DETECT_ABNORMAL_FUNCTION_CALL	0x010801
#define AHNHS_ACTAPC_DETECT_ANTIFREESERVER			0x010901		// Game�� �ùٸ� ������ ������ ���� �ʾҽ��ϴ�.
#define AHNHS_ACTAPC_DETECT_ABNORMAL_HACKSHIELD_STATUS	0x010A01	// HackShield ���� ���� Ȯ�� ���, ���� ������ �ƴ�.
#define AHNHS_ACTAPC_DETECT_SELF_DESTRUCTION			0x010A02	// HackShield ���� ������, ��ŷ���� �����ϰ�, ������� ���� ���, ���� ���� ��Ų��.	



//[AntiCpXSvr.dll 1.0.0.10 ���� ���� ����]
// ���ӻ翡�� �⺻ bytealigment�� ������� ������ �ٽ��� �������� ��� ���� �� ���� 
// bytealignment�� ����ǹǷ� ���� ������ ���ÿ� ����.
#pragma pack(push) //���� alignment ����
#pragma pack(8)

typedef struct _AHNHS_TRANS_BUFFER
{
	unsigned char byBuffer[ANTICPX_TRANS_BUFFER_MAX/* �ۼ��� ��Ŷ�� �ִ� ũ�� */];
	unsigned short nLength;
} AHNHS_TRANS_BUFFER, *PAHNHS_TRANS_BUFFER;

#pragma pack(pop) // ���� alignment ����.

#endif // __ANTICPX_TRANS_BUFFER__

#if defined(__cplusplus)
extern "C"
{
#endif

// MACRO Version

#if defined (_NO_HACKSHIELD)
	
	#define	AHNHS_NOT_NULL	(1)	// �����Ͱ��� �����ϴ� ���, NOT NULL�� ó��

	#define	_AhnHS_CreateServerObject(arg1)		AHNHS_NOT_NULL
	#define	_AhnHS_CloseServerHandle(arg1)		;
	#define	_AhnHS_CreateClientObject(arg1)		AHNHS_NOT_NULL
	#define _AhnHS_CloseClientHandle(arg1)		;
	#define _AhnHS_MakeRequest(arg1,arg2)					ERROR_SUCCESS
	#define _AhnHS_VerifyResponse(arg1,arg2,arg3)			ERROR_SUCCESS
	#define _AhnHS_VerifyResponseEx(arg1,arg2,arg3,arg4)	ERROR_SUCCESS
	#define _AhnHS_VerifyResponseEx_WithInfo(arg1,arg2,arg3,arg4,arg5)	ERROR_SUCCESS

#else	//!_NO_HACKSHIELD

/**
	���� �ڵ�(Server Handle)�� �����Ѵ�

	@param
		const char *pszFilePath ; [�Է°�] HackShield Briefcase (.hsb) ���� ���

	@retval
		���� �ڵ�(Server Handle)

	@remarks
		�����ڵ��� �ùٸ��� �������� ������ ��� NULL ���� �����Ѵ�. ���� HackShield Briefcase
		(.hsb) ���� ��ΰ� �ùٸ��� ���� ��쳪 �ý��� ���ҽ�(�޸�)�� ������ ��� �߻��Ѵ�.

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
AHNHS_SERVER_HANDLE
__stdcall
_AhnHS_CreateServerObject (
	IN const char *pszFilePath
	);


/**
	���� �ڵ�(Server Handle)�� �ݴ´�

	@param
		AHNHS_SERVER_HANDLE hServer ; [�Է°�] ���� �ڵ�(Server Handle)

	@retval
		����

	@remarks
		������ �����ڵ��� ���� ���μ����� ���� ���μ����� ������ �� �ݾ��־��
		�����ڵ� ������ ���Ǿ��� �޸𸮳� �ý��� �ڿ��� �����ϰԵȴ�.

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
void
__stdcall
_AhnHS_CloseServerHandle (
	IN AHNHS_SERVER_HANDLE hServer
	);


/**
	Ŭ���̾�Ʈ �ڵ�(Client Handle)�� �����Ѵ�

	@param
		AHNHS_SERVER_HANDLE hServer ; [�Է°�] ���� �ڵ�(Server Handle)

	@retval
		Ŭ���̾�Ʈ �ڵ�(Client Handle)

	@remarks
		���� �ڵ��� �Է� �޾� Ŭ���̾�Ʈ �ڵ��� �����Ѵ�. Ŭ���̾�Ʈ �ڵ���
		Ŭ���̾�Ʈ�� ������ �� ���� �����ϸ�, ������ �����Ǵ� ���� �ڵ��� ���� �ʰ�
		�����Ѵ�.

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
AHNHS_CLIENT_HANDLE
__stdcall
_AhnHS_CreateClientObject (
	IN AHNHS_SERVER_HANDLE hServer
	);


/**
	Ŭ���̾�Ʈ �ڵ�(Client Handle)�� �ݴ´�

	@param
		AHNHS_CLIENT_HANDLE hClient ; [�Է°�] Ŭ���̾�Ʈ �ڵ�(Client Handle)

	@retval
		����

	@remarks
		������ Ŭ���̾�Ʈ �ڵ��� Ŭ���̾�Ʈ ������ ����� �� �����Ǿ�� �Ѵ�.
		�̶� Ŭ���̾�Ʈ �ڵ� ������ ���Ǿ��� �޸𸮳� �ý��� �ڿ��� �����ϰԵȴ�.

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
void
__stdcall
_AhnHS_CloseClientHandle (
	IN AHNHS_CLIENT_HANDLE hClient
	);


/**
	Ŭ���̾�Ʈ�� ��ŷ �õ��� �����ϱ� ���� ��û(�޽���)�� �����Ѵ�.

	@param
		AHNHS_CLIENT_HANDLE hClient ; [�Է°�] Ŭ���̾�Ʈ �ڵ�
		PAHNHS_TRANS_BUFFER pRequestBuffer ; [��°�] ���� ������ ����/����

	@retval
		ERROR_ANTICPXSVR_xxx �������� ���ǵǾ� �ִ� ���� �ڵ� 

	@remarks
		���� ���ǿ� �´� Ŭ���̾�Ʈ �ڵ��� �Է��Ͽ� ��û �޽����� �����Ѵ�.
		��û �޽����� AHNHS_TRANS_BUFFER ����ü ���·� ��µǸ�, ��� ���� ����
		������ ����.

		typedef struct _AHNHS_TRANS_BUFFER
		{
			unsigned short nLength;
			unsigned char byBuffer[ANTICPX_TRANS_BUFFER_MAX]; // �ۼ��� ��Ŷ�� �ִ� ���� ũ��
			
		} AHNHS_TRANS_BUFFER, *PAHNHS_TRANS_BUFFER;

		nLength ; ��û �޽��� ������ ���� ���� ����
		byBuffer ; ��û �޽��� ������ ���� �� �ִ� �ִ� ����Ʈ ����

		���� ����!!! byBuffer�� ��û �޽��� ������ ���� �� �ִ� �ִ� ���� ũ���̹Ƿ� ��Ʈ��ũ�� 
		������ �� nLength ��ŭ�� �����ؾ��Ѵ�.

		Example:
		
			AHNHS_TRANS_BUFFER stReqTransBuf;

			ulRet = _AhnHS_MakeRequest ( hClient, &stReqTransBuf );

			if ( ulRet != ERROR_SUCCESS )
				return ulRet;

			bytesSent = send( ConnectSocket, stReqTransBuf.byBuffer, stReqTransBuf.nLength, 0 );

			...

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
unsigned long
__stdcall
_AhnHS_MakeRequest (
	IN AHNHS_CLIENT_HANDLE hClient,
	OUT PAHNHS_TRANS_BUFFER pRequestBuffer
	);


/**
	Ŭ���̾�Ʈ�� ����(�޽���)�� �ùٸ��� �˻��Ѵ�.

	@param
		AHNHS_CLIENT_HANDLE hClient ; [�Է°�] Ŭ���̾�Ʈ �ڵ�
		unsigned char *pbyResponse ; Ŭ���̾�Ʈ�� ���� ���� ������ ����
		unsigned long nResponseLength ; Ŭ���̾�Ʈ�� ���� ���� ������ ����

	@retval
		ERROR_ANTICPXSVR_xxx �������� ���ǵǾ� �ִ� ���� �ڵ� 

	@remarks
		_AhnHS_MakeRequest �Լ��� ���� ��û(�޽���)�� ���� Ŭ���̾�Ʈ�� ������
		�ùٸ��� �˻��ϴ� �Լ��̴�.

	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
unsigned long
__stdcall
_AhnHS_VerifyResponse (
	IN AHNHS_CLIENT_HANDLE hClient,
	IN unsigned char *pbyResponse,
	IN unsigned long nResponseLength
	);

/**
	Ŭ���̾�Ʈ�� ���� �޽����� �����Ͽ� Ŭ���̾�Ʈ ���� ���Ῡ�θ� 
	�����Ѵ�. 

	@param
		AHNHS_CLIENT_HANDLE hClient ; [�Է°�] Ŭ���̾�Ʈ �ڵ�
		unsigned char *pbyResponse ; Ŭ���̾�Ʈ�� ���� ���� ������ ����
		unsigned long nResponseLength ; Ŭ���̾�Ʈ�� ���� ���� ������ ����
		unsigned long *pnErrorCode	; ���� �ڵ� 
	@retval
		ANTICPX_RECOMMAND_CLOSE_SESSION : Ŭ���̾�Ʈ ���� ���� ���� ����
		ANTICPX_RECOMMAND_KEEP_SESSION  : Ŭ���̾�Ʈ ���� ���� ���� ����
	@remarks
		���������� _AhnHS_VerifyResponse �Լ��� ȣ���Ѵ�.
	
	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
unsigned long
__stdcall
_AhnHS_VerifyResponseEx (
	IN AHNHS_CLIENT_HANDLE hClient,
	IN unsigned char *pbyResponse,
	IN unsigned long nResponseLength,
	OUT unsigned long *pulErrorCode
	);


/**
	Ŭ���̾�Ʈ�� ���� �޽����� �����Ͽ� Ŭ���̾�Ʈ ���� ���Ῡ�θ� 
	�����Ѵ�. 

	@param
		AHNHS_CLIENT_HANDLE hClient ; [�Է°�] Ŭ���̾�Ʈ �ڵ�
		unsigned char *pbyResponse ; Ŭ���̾�Ʈ�� ���� ���� ������ ����
		unsigned long nResponseLength ; Ŭ���̾�Ʈ�� ���� ���� ������ ����
		unsigned long *pulErrorCode	; ���� �ڵ� 
		unsigned long *pulSpecificErr ; pnErrorCode ���� ���� �����ϴ� �����ڵ� 
	@retval
		ANTICPX_RECOMMAND_CLOSE_SESSION : Ŭ���̾�Ʈ ���� ���� ���� ����
		ANTICPX_RECOMMAND_KEEP_SESSION  : Ŭ���̾�Ʈ ���� ���� ���� ����
	@remarks
		���������� _AhnHS_VerifyResponse �Լ��� ȣ���Ѵ�.
	
	@see
		Ȯ�� �������� Quick Guide�� ������ �ּ���.
*/
AHNHS_API
unsigned long
__stdcall
_AhnHS_VerifyResponseEx_WithInfo(
	IN AHNHS_CLIENT_HANDLE hClient,
	IN unsigned char *pbyResponse,
	IN unsigned long nResponseLength,
	OUT unsigned long *pulErrorCode,
	OUT unsigned long *pulSpecificError
	);


#endif	// _NO_HACKSHIELD

#if defined(__cplusplus)
}
#endif

#endif // _ANTICPX_SERVER_H_
