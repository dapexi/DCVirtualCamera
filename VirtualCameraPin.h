
#pragma once
#include "stdafx.h"
#include "PushGuids.h"
#include "Logger.h"

// Filter name strings


struct VideoData
{
	int FrameRate = 0;
	long FrameIndex = 0;
	int videoWidth = 0;
	int videoHeight = 0;
	byte* buffer = NULL;
};

class CVirtualCameraPin : public CSourceStream, public IAMStreamConfig, public IKsPropertySet, public IAMFilterMiscFlags, public IAMBufferNegotiation
{

private:
	GUID						m_mediaSubType;
	REFERENCE_TIME				m_rtStart;
	bool m_fFirstSampleDelivered;
	////////////////////////////////////////////////////////////
protected:
	HRESULT OnThreadCreate(void);
	HRESULT OnThreadDestroy(void);
	HRESULT OnThreadStartPlay(void);

	CCritSec m_cSharedState;

	const int m_BitCount = 32;

	HANDLE hMapFile = NULL;
	bool readVideoInfo = false;
	int m_VideoWidth = 720;
	int m_VideoHeight = 480;
	long lastFrameIndex = -1;
	int checkBufferCount = 0;
	long m_frameSize = m_VideoWidth*m_VideoHeight * 4;
	LONGLONG m_llSampleMediaTimeStart;
public:
	LONGLONG m_out_framerate = 333333;
	//////////////////////////////////////////////////////////////////////////
	//  IUnknown
	//////////////////////////////////////////////////////////////////////////
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }
	STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }	
	STDMETHODIMP Notify(IBaseFilter * pSelf, Quality q) { return S_OK; }

	//////////////////////////////////////////////////////////////////////////
	//  IAMStreamConfig
	//////////////////////////////////////////////////////////////////////////
	HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
	HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
	HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
	HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

	//////////////////////////////////////////////////////////////////////////
	//  IKsPropertySet
	//////////////////////////////////////////////////////////////////////////
	HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
	HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
	HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

	
	// IAMBufferNegotiation -- never gets called...
	HRESULT STDMETHODCALLTYPE SuggestAllocatorProperties( /* [in] */ const ALLOCATOR_PROPERTIES *pprop);
	HRESULT STDMETHODCALLTYPE GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop);

	HRESULT FillBuffer(IMediaSample *pSample);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	/* don't seem to get called... */
	HRESULT Stop(void);

	CVirtualCameraPin(HRESULT *phr, CSource *pFilter);
	~CVirtualCameraPin();


	// IAMFilterMiscFlags -- never gets called...
	ULONG STDMETHODCALLTYPE GetMiscFlags() { return AM_FILTER_MISC_FLAGS_IS_SOURCE; }

	void ReleaseMemoryUsed();
	LPVOID get_Filter(){ return (LPVOID)(this->m_pFilter); }
private:
	bool ProcessVideo(IMediaSample *pSample);
	
	HRESULT FillVideoBuffer(IMediaSample *pSample);
	std::auto_ptr<VideoData> ReadVideoSamplefromPipe();
	void GetVideoSampleInfo();

};

