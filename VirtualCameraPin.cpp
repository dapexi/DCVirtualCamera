#pragma once
#include "VirtualCameraPin.h"


#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);


CVirtualCameraPin::CVirtualCameraPin(HRESULT *phr, CSource *pFilter)
	: CSourceStream(NAME("DC Virtual Camera"), phr, pFilter, L"Out")
{
	m_rtStart = 0;
	m_pName = L"Video";
	//
	m_fFirstSampleDelivered = FALSE;
	m_llSampleMediaTimeStart = 0;
	GetMediaType(0, &m_mt);
}

CVirtualCameraPin::~CVirtualCameraPin()
{
	CAutoLock cAutoLock(&m_cSharedState);
	//stop the pipe/viewer first
	ReleaseMemoryUsed();
}

HRESULT CVirtualCameraPin::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == _uuidof(IAMStreamConfig))
		*ppv = (IAMStreamConfig*)this;
	else if (riid == _uuidof(IKsPropertySet))
		*ppv = (IKsPropertySet*)this;
	else if (riid == _uuidof(IAMFilterMiscFlags))
		*ppv = (IAMFilterMiscFlags*)this;
	else if (riid == _uuidof(IAMBufferNegotiation))
		*ppv = (IAMBufferNegotiation*)this;
	else
		return CSourceStream::QueryInterface(riid, ppv);

	AddRef();
	return S_OK;
}

HRESULT CVirtualCameraPin::SetMediaType(const CMediaType *pmt)
{
	DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());
	HRESULT hr = CSourceStream::SetMediaType(pmt);
	return hr;
}

HRESULT CVirtualCameraPin::GetMediaType(int iPosition, CMediaType *pmt)
{
	if (iPosition < 0) {
		return E_INVALIDARG;
	}

	if (iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}
	//
	CAutoLock cAutoLockShared(&m_cSharedState);
	//
	VIDEOINFO *pvi = (VIDEOINFO *)pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
	if (NULL == pvi) return S_OK;
	/* Zero out the memory */
	ZeroMemory(pvi, sizeof(VIDEOINFO));
	//
	GetVideoSampleInfo();
	//
	pvi->bmiHeader.biCompression = BI_RGB;
	pvi->bmiHeader.biBitCount = m_BitCount;
	pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth = m_VideoWidth;
	pvi->bmiHeader.biHeight = m_VideoHeight;
	pvi->bmiHeader.biPlanes = 1;
	pvi->bmiHeader.biSizeImage = m_frameSize;
	pvi->bmiHeader.biClrImportant = 0;
	pvi->AvgTimePerFrame = m_out_framerate;
	/* Set the Major-type of the media for DShow */
	pmt->SetType(&MEDIATYPE_Video);
	pmt->SetFormatType(&FORMAT_VideoInfo);
	/* If our codec uses temporal compression */
	pmt->SetTemporalCompression(FALSE);
	pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
	/* Set the max sample size */
	pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
	//
	/////////////////////////////////////////////////////////////////////////////////////////
	return S_OK;
}

HRESULT CVirtualCameraPin::CheckMediaType(const CMediaType *pMediaType)
{
	int cbFormat = pMediaType->cbFormat;
	if (*pMediaType != m_mt) {
		return E_INVALIDARG;
	}
	return S_OK;
} // CheckMediaType

HRESULT STDMETHODCALLTYPE CVirtualCameraPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
	if (!pmt) {
		return S_OK; 
	}

	if (CheckMediaType((CMediaType *)pmt) != S_OK) {
		return E_FAIL; // just in case :P [FME...]
	}

	m_mt = *pmt;

	IPin* pin;
	ConnectedTo(&pin);
	if (pin)
	{
		IFilterGraph *pGraph = ((CSource*)m_pFilter)->GetFilterGraph();
		pGraph->Reconnect(this);
	}

	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCameraPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
	*ppmt = CreateMediaType(&m_mt);
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCameraPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	*piCount = 1; // only allow one type currently...
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CVirtualCameraPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppMediaType, BYTE *pSCC)
{
	if (iIndex < 0)
		return E_INVALIDARG;
	if (iIndex > 0)
		return S_FALSE;
	if (pSCC == NULL)
		return E_POINTER;

	*ppMediaType = CreateMediaType(&m_mt);
	if (*ppMediaType == NULL) return E_OUTOFMEMORY;
	DECLARE_PTR(VIDEOINFOHEADER, pvi, (*ppMediaType)->pbFormat);
	//
	AM_MEDIA_TYPE * pm = *ppMediaType;
	//
	GetVideoSampleInfo();
	//
	VIDEO_STREAM_CONFIG_CAPS* pASCC = (VIDEO_STREAM_CONFIG_CAPS*)pSCC;
	ZeroMemory(pSCC, sizeof(VIDEO_STREAM_CONFIG_CAPS));
	///////////////////////////////////////////////////
	pvi->bmiHeader.biCompression = BI_RGB;
	pvi->bmiHeader.biBitCount = m_BitCount;
	pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth = m_VideoWidth;
	pvi->bmiHeader.biHeight = m_VideoHeight;
	pvi->bmiHeader.biPlanes = 1;
	pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
	pvi->bmiHeader.biClrImportant = 0;

	(*ppMediaType)->majortype = MEDIATYPE_Video;
	(*ppMediaType)->subtype = MEDIASUBTYPE_RGB32;
	(*ppMediaType)->formattype = FORMAT_VideoInfo;
	(*ppMediaType)->bTemporalCompression = FALSE;
	(*ppMediaType)->bFixedSizeSamples = FALSE;
	(*ppMediaType)->lSampleSize = pvi->bmiHeader.biSizeImage;
	(*ppMediaType)->cbFormat = sizeof(VIDEOINFOHEADER);
	//
	pASCC->guid = FORMAT_VideoInfo;
	pASCC->VideoStandard = AnalogVideo_None;

	pASCC->InputSize.cx = m_VideoWidth;
	pASCC->InputSize.cy = m_VideoHeight;
	pASCC->MinCroppingSize.cx = 80;
	pASCC->MinCroppingSize.cy = 60;
	pASCC->MaxCroppingSize.cx = m_VideoWidth;
	pASCC->MaxCroppingSize.cy = m_VideoHeight;
	pASCC->CropGranularityX = 80;
	pASCC->CropGranularityY = 60;

	pASCC->CropAlignX = 0;
	pASCC->CropAlignY = 0;

	pASCC->MinOutputSize.cx = pASCC->MinCroppingSize.cx;
	pASCC->MinOutputSize.cy = pASCC->MinCroppingSize.cy;
	pASCC->MaxOutputSize.cx = pASCC->MaxCroppingSize.cx;
	pASCC->MaxOutputSize.cy = pASCC->MaxCroppingSize.cy;
	pASCC->OutputGranularityX = 0;
	pASCC->OutputGranularityY = 0;
	pASCC->StretchTapsX = 0;
	pASCC->StretchTapsY = 0;
	pASCC->ShrinkTapsX = 0;
	pASCC->ShrinkTapsY = 0;

	pASCC->MinFrameInterval = 200000;
	pASCC->MaxFrameInterval = UNITS / 1;
	pASCC->MinBitsPerSecond = (pASCC->MinOutputSize.cx * pASCC->MinOutputSize.cy * m_BitCount) / 5;// * 1;
	pASCC->MaxBitsPerSecond = (pASCC->MaxOutputSize.cx * pASCC->MaxOutputSize.cy * m_BitCount) * 50;//* (UNITS / m_out_framerate);
	
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////

HRESULT CVirtualCameraPin::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData,
	DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{// Set: Cannot set any properties.
	return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CVirtualCameraPin::Get(
	REFGUID guidPropSet,   // Which property set.
	DWORD dwPropID,        // Which property in that set.
	void *pInstanceData,   // Instance data (ignore).
	DWORD cbInstanceData,  // Size of the instance data (ignore).
	void *pPropData,       // Buffer to receive the property data.
	DWORD cbPropData,      // Size of the buffer.
	DWORD *pcbReturned     // Return the size of the property.
	)
{
	if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;

	if (pcbReturned) *pcbReturned = sizeof(GUID);
	if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
	if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.

	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}
// QuerySupported: Query whether the pin supports the specified property.
HRESULT CVirtualCameraPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
	
	if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CVirtualCameraPin::SuggestAllocatorProperties( /* [in] */ const ALLOCATOR_PROPERTIES *pprop) {
	int requested = pprop->cbBuffer;
	if (pprop->cBuffers > 0)
		requested *= pprop->cBuffers;
	if (pprop->cbPrefix > 0)
		requested += pprop->cbPrefix;

	if (requested <= m_frameSize*2) {
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}

HRESULT STDMETHODCALLTYPE CVirtualCameraPin::GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop) { return NULL; } // they never call this...

void CVirtualCameraPin::ReleaseMemoryUsed()
{
	try
	{
		
	}
	catch (std::exception ex)
	{

	}
}


HRESULT CVirtualCameraPin::DecideBufferSize(IMemAllocator* pMemAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
	CheckPointer(pMemAlloc, E_POINTER);
	CheckPointer(pProperties, E_POINTER);
	/* Thread-saftey */
	CAutoLock cAutoLockShared(&m_cSharedState);

	pProperties->cbBuffer = m_frameSize;
	pProperties->cBuffers = 2;

	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pMemAlloc->SetProperties(pProperties, &Actual);
	if (FAILED(hr))
	{
		return hr;
	}
	
	if (Actual.cbBuffer < pProperties->cbBuffer)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CVirtualCameraPin::FillBuffer(IMediaSample *pSample)
{
	CheckPointer(pSample, E_POINTER);
	//
	return FillVideoBuffer(pSample);
}

HRESULT CVirtualCameraPin::FillVideoBuffer(IMediaSample *pSample)
{
	HRESULT hr = S_OK;
	bool rc = true;

	try
	{
		rc = ProcessVideo(pSample);
		if (rc)
		{

			REFERENCE_TIME rtStop = m_rtStart + m_out_framerate;

			//CRefTime now;
			//now.m_time = m_rtStart;
			//m_pFilter->StreamTime(now);

			pSample->SetTime(&m_rtStart, &rtStop);

			m_rtStart = rtStop;

			//pSample->SetPreroll(FALSE);
			pSample->SetDiscontinuity(!m_fFirstSampleDelivered);
			pSample->SetSyncPoint(true);

			LONGLONG llMediaTimeStart = m_llSampleMediaTimeStart;
			DWORD dwNumVideoSamplesInPacket = m_out_framerate / 10000;

			LONGLONG llMediaTimeStop = m_llSampleMediaTimeStart + dwNumVideoSamplesInPacket;
			hr = pSample->SetMediaTime(&llMediaTimeStart, &llMediaTimeStop);
			m_llSampleMediaTimeStart = llMediaTimeStop;

			m_fFirstSampleDelivered = TRUE;
		}
		else{
			hr = E_FAIL;
		}
	}
	catch (...)
	{
		hr = E_FAIL;
	}
	return hr;
}


bool CVirtualCameraPin::ProcessVideo(IMediaSample *pSample)
{
	long cbData;
	BYTE *pData;

	// Access the sample's data buffer
	pSample->GetPointer(&pData);
	cbData = pSample->GetSize();
	long bufferSize = cbData;
	///////////////////////////////////////////////////////////
	//
	auto vd = this->ReadVideoSamplefromPipe();
	if (vd->FrameRate == 0 || vd->buffer == NULL)
		return true;
	//
	/*for (int i = 0; i < bufferSize; ++i)
		pData[i] = rand();*/
	memcpy(pData, vd->buffer, m_frameSize);
	//
	//pSample->SetActualDataLength(bufferSize);
	//
	delete[]vd->buffer;
	//
	return true;
}

HRESULT CVirtualCameraPin::OnThreadCreate(void)
{
	HRESULT hr = S_OK;
	hr = CSourceStream::OnThreadCreate();
	m_fFirstSampleDelivered = FALSE;
	m_rtStart = 0;
	return hr;
}

HRESULT CVirtualCameraPin::OnThreadDestroy(void)
{
	CAutoLock cAutoLockShared(&m_cSharedState);
	HRESULT hr = S_OK;
	ReleaseMemoryUsed();
	hr = CSourceStream::OnThreadDestroy();
	return hr;
}

HRESULT CVirtualCameraPin::OnThreadStartPlay(void)
{
	HRESULT hr = S_OK;
	hr = CSourceStream::OnThreadStartPlay();
	//
	return hr;
}

HRESULT CVirtualCameraPin::Stop(void)
{
	CAutoLock cAutoLockShared(&m_cSharedState);
	HRESULT hr = S_OK;
	CSourceStream::Stop();
	ReleaseMemoryUsed();
	return hr;
}


std::auto_ptr<VideoData> CVirtualCameraPin::ReadVideoSamplefromPipe()
{
	std::auto_ptr<VideoData> videoData(new VideoData);
	//Test
	//videoData->FrameIndex = 0;
	//videoData->FrameRate = 333333;
	//videoData->videoWidth = 720;
	//videoData->videoHeight = 480;

	//m_VideoWidth = videoData->videoWidth;
	//m_VideoHeight = videoData->videoHeight;
	//m_frameSize = m_VideoWidth * m_VideoHeight * 4;
	//m_out_framerate = videoData->FrameRate;

	////videoData->buffer = new byte[m_frameSize];
	//return videoData;

	//End Of Test
	//
	try
	{
		std::string mapName = "PlayoutXVCam" + std::to_string(FilterIndex);
		std::wstring wmapName(mapName.begin(), mapName.end());;

		std::string mutextName = "Global\PlayoutXVCammutex" + std::to_string(FilterIndex);
		std::wstring wmutextName(mutextName.begin(), mutextName.end());
		//
		if (hMapFile == NULL)
			hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, wmapName.c_str());
		//
		if (hMapFile == NULL)
			return videoData;
		//
		HANDLE hMutex = CreateMutex(NULL, false, wmutextName.c_str());
		if (hMutex == NULL)
			return videoData;
		//
		void* pBuf = NULL;
		//
		while (true)
		{
			WaitForSingleObject(hMutex, 1);
			
			pBuf = (LPTSTR)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
			if (pBuf == NULL)
			{
				//auto uError = GetLastError();
				break;
			}
			//
			CopyMemory(&videoData->FrameIndex, (PBYTE)pBuf, sizeof(long));
			CopyMemory(&videoData->FrameRate, ((PBYTE)pBuf + 8), sizeof(int));
			CopyMemory(&videoData->videoWidth, ((PBYTE)pBuf + 12), sizeof(int));
			CopyMemory(&videoData->videoHeight, ((PBYTE)pBuf + 16), sizeof(int));
			//
			m_VideoWidth = videoData->videoWidth;
			m_VideoHeight = videoData->videoHeight;
			m_frameSize = m_VideoWidth * m_VideoHeight * 4;
			m_out_framerate = videoData->FrameRate;
			//
			if (videoData->FrameIndex == lastFrameIndex)
			{
				if (checkBufferCount++ > 1000)
				{
					break;
				}	
				//
				if (pBuf != NULL)
				{
					FlushViewOfFile(pBuf, 0);
					UnmapViewOfFile(pBuf);
				}
				//
				ReleaseMutex(hMutex);
				Sleep(1);
				//
				continue;
			}
			//
			checkBufferCount = 0;
			lastFrameIndex = videoData->FrameIndex;
			//
			int bufferLength = 0;
			CopyMemory(&bufferLength, ((PBYTE)pBuf + 20), sizeof(int));
			//
			if (bufferLength > 0)
			{
				videoData->buffer = new byte[bufferLength];
				if (videoData->buffer == NULL)
				{
					//_CrtDumpMemoryLeaks();
					break;
				}
				//
				CopyMemory(videoData->buffer, ((PBYTE)pBuf + 24), bufferLength);
			}
			//
			break;
		}
		//
		if (pBuf != NULL)
		{
			FlushViewOfFile(pBuf, 0);
			UnmapViewOfFile(pBuf);
		}
		//
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		//
		return videoData;
	}
	catch (...)
	{
		return videoData;
	}
}

void CVirtualCameraPin::GetVideoSampleInfo()
{
	if (readVideoInfo)
		return;
	//
	try
	{
		auto vd = this->ReadVideoSamplefromPipe();
		if (vd->FrameRate != 0 && vd->buffer != NULL)
		{
			readVideoInfo = true;
			m_out_framerate = vd->FrameRate;
			m_VideoWidth = vd->videoWidth;
			m_VideoHeight = vd->videoHeight;
			//
			delete[]vd->buffer;
		}
	}
	catch (...)
	{
	}
}

