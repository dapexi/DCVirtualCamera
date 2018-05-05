#include "VirtualCamera.h"

using namespace std;

CVirtualCamera::CVirtualCamera(IUnknown *pUnk, HRESULT *phr)
	: CSource(g_wszVirtualCamera, pUnk, CLSID_VIRTUALCAMERAFILTER)
{
	this->m_phr = phr;
	//
	m_paStreams = (CSourceStream **) new CVirtualCameraPin*[1];
	m_paStreams[0] = new CVirtualCameraPin(phr, this);
}

CVirtualCamera::~CVirtualCamera()
{
	HRESULT hr = S_OK;
	try
	{	
		hr = m_paStreams[0]->Disconnect();
		//m_pPin->Inactive();
		hr = this->RemovePin(m_paStreams[0]);
		Stop();
	}
	catch (...)
	{
	}
}

CUnknown * WINAPI CVirtualCamera::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	CVirtualCamera *pNewFilter = new CVirtualCamera(pUnk, phr);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
	return pNewFilter;

}

STDMETHODIMP CVirtualCamera::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = S_OK;
	try
	{
		CheckPointer(ppv, E_POINTER);
		//
		char msg[255];
		sprintf(msg, "REFIID=%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
		riid.Data1, riid.Data2, riid.Data3,
		riid.Data4[0], riid.Data4[1],
		riid.Data4[2], riid.Data4[3],
		riid.Data4[4], riid.Data4[5],
		riid.Data4[6], riid.Data4[7] ); 
		Logger::LogMessage( msg);
		//
		/*if (riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet) || riid == _uuidof(IAMVideoControl))
			return m_paStreams[0]->QueryInterface(riid, ppv);
		else*/ if (riid == _uuidof(IAMVideoControl))
			*ppv = (IAMVideoControl*)this;
		else
			return CSource::NonDelegatingQueryInterface(riid, ppv);
		//
		AddRef();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
STDMETHODIMP_(ULONG) CVirtualCamera::NonDelegatingAddRef()
{
	ULONG ret = CSource::NonDelegatingAddRef();
	return ret;
}
STDMETHODIMP_(ULONG) CVirtualCamera::NonDelegatingRelease()
{
	ULONG ret = CSource::NonDelegatingRelease();
	return ret;
}

STDMETHODIMP CVirtualCamera::Stop(void)
{
	HRESULT hr = S_OK;
	hr = CSource::Stop();
	Logger::LogMessage("Filter Stopped.");
	return hr;
}

STDMETHODIMP CVirtualCamera::Run(REFERENCE_TIME tStart) {
	return CBaseFilter::Run(tStart); 
}

STDMETHODIMP CVirtualCamera::GetState(DWORD dw, FILTER_STATE *pState)
{
	CheckPointer(pState, E_POINTER);
	*pState = m_State;
	if (m_State == State_Paused)
	{
		return VFW_S_CANT_CUE;
	}
	else
	{
		return S_OK;
	}
}


//Implement IAmVideoControl
HRESULT STDMETHODCALLTYPE CVirtualCamera::GetCaps(IPin *pPin, long *pCapsFlags)
{
	Logger::LogMessage("GetCaps");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCamera::GetCurrentActualFrameRate(IPin *pPin, LONGLONG *ActualFrameRate)
{
	Logger::LogMessage("GetCurrentActualFrameRate");
	ActualFrameRate = &((CVirtualCameraPin*)this->m_paStreams[0])->m_out_framerate;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCamera::GetFrameRateList(IPin *pPin, long iIndex, SIZE Dimensions, long *ListSize, LONGLONG **FrameRates)
{
	Logger::LogMessage("GetFrameRateList");
	*ListSize = 1;
	*FrameRates = &((CVirtualCameraPin*)this->m_paStreams[0])->m_out_framerate;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCamera::GetMaxAvailableFrameRate(IPin *pPin, long iIndex, SIZE Dimensions, LONGLONG *MaxAvailableFrameRate)
{
	Logger::LogMessage("GetMaxAvailableFrameRate");
	MaxAvailableFrameRate = &((CVirtualCameraPin*)this->m_paStreams[0])->m_out_framerate;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCamera::GetMode(IPin *pPin, long *Mode)
{
	Logger::LogMessage("GetMode");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCamera::SetMode(IPin *pPin, long Mode)
{
	Logger::LogMessage("SetMode");
	return S_OK;
}