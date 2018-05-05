
#pragma once
#include "stdafx.h"
#include "VirtualCameraPin.h"

class CVirtualCamera : public CSource,IAMVideoControl
{
public:
	DECLARE_IUNKNOWN;
	static CUnknown *		WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
	STDMETHODIMP			NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	STDMETHODIMP_(ULONG)	NonDelegatingAddRef();
	STDMETHODIMP_(ULONG)	NonDelegatingRelease();
	STDMETHODIMP			Stop(void);
	STDMETHODIMP			SendSample(char * pBuffer, int BufferLen);
	STDMETHODIMP Run(REFERENCE_TIME tStart); // it does call this...
	STDMETHODIMP GetState(DWORD dw, FILTER_STATE *pState);
	~CVirtualCamera();

	//Implement IAmVideoControl
	HRESULT STDMETHODCALLTYPE GetCaps(IPin *pPin,long *pCapsFlags);
	HRESULT STDMETHODCALLTYPE GetCurrentActualFrameRate(IPin *pPin, LONGLONG *ActualFrameRate);
	HRESULT STDMETHODCALLTYPE GetFrameRateList(IPin *pPin, long iIndex, SIZE Dimensions, long *ListSize, LONGLONG **FrameRates);
	HRESULT STDMETHODCALLTYPE GetMaxAvailableFrameRate(IPin *pPin, long iIndex, SIZE Dimensions, LONGLONG *MaxAvailableFrameRate);
	HRESULT STDMETHODCALLTYPE GetMode(IPin *pPin, long *Mode);
	HRESULT STDMETHODCALLTYPE SetMode(IPin *pPin, long Mode);
private:
	CVirtualCamera(LPUNKNOWN punk, HRESULT *phr);
	CCritSec					m_cSharedState;
	HRESULT *m_phr;
public:
};



