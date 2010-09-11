//------------------------------------------------------------------------------
// File: Setup.cpp
//
// Desc: DirectShow sample code - implementation of PushSource sample filters
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "VideoSpringRecv.h"
#include <time.h>

// Filter setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_VP8      // Minor type
};


const AMOVIESETUP_PIN sudOutputPinVideoSpringRecv = 
{
    L"Output",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudOpPinTypes  // Pointer to media types.
};

const AMOVIESETUP_FILTER sudVideoSpringRecv =
{
    &CLSID_VideoSpringRecv,// Filter CLSID
    L"VideoSpringRecv",        // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOutputPinVideoSpringRecv     // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[1] = 
{
    { 
      L"VideoSpringRecv",                // Name
      &CLSID_VideoSpringRecv,        // CLSID
      CVideoSpringRecv::CreateInstance,  // Method to create an instance of MyComponent
      NULL,                           // Initialization function
      &sudVideoSpringRecv            // Set-up information (for filters)
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    




/********** BEGIN other file ****************/

/*
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};
*/

/**********************************************
 *
 *  CVideoSpringRecvPin Class
 *  
 *
 **********************************************/

CVideoSpringRecvPin::CVideoSpringRecvPin(HRESULT *phr, CSource *pFilter)
      : CSourceStream(NAME("Output"), phr, pFilter, L"Out"), IVideoSpringRecv()
{
	frame = 1;
}


CVideoSpringRecvPin::~CVideoSpringRecvPin()
{
}

STDMETHODIMP CVideoSpringRecvPin::SetServerSocket(SOCKET s)
{
	server = s;

	return S_OK;
}

STDMETHODIMP CVideoSpringRecvPin::SetPresenterId(long id)
{
	presenterId = id;

	return S_OK;
}

STDMETHODIMP CVideoSpringRecvPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_IVideoSpringRecv)
	{
		return GetInterface(static_cast<IVideoSpringRecv*>(this), ppv);
	}
	else
	{
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
	}
}

HRESULT CVideoSpringRecvPin::Active(void)
{
	Message m;

	m.header.command = C_RECEIVE;
	m.header.length = 0;

	if(sendMessage(server, &m) == -1)
	{
		printf("Error sending message to server.\n");
		return -1;
	}

	return CSourceStream::Active();

} // Active


//
// Inactive
//
// Called when the filter is stopped
//
HRESULT CVideoSpringRecvPin::Inactive(void)
{
   	return CSourceStream::Inactive();
} // Inactive

// GetMediaType: This method tells the downstream pin what types we support.

// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when (a) our filter proposes a media type, (b) the other filter proposes a
// type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int,CMediaType*) AND CheckMediaType.
//
// In this case we support only one type, which we obtain from the bitmap file.

HRESULT CVideoSpringRecvPin::GetMediaType(CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);

	BYTE *format;
	uint32_t formatLength;

	Message m;

	m.header.command = C_SET_CLIENT_RECV;
	m.header.length = sizeof(uint32_t);

	m.data = (BYTE*)&presenterId;

	if(sendMessage(server, &m) == -1)
	{
		printf("Error sending message to server.\n");
		return -1;
	}

	if(receiveMessage(server, m) == -1)
	{
		printf("Error receiving format from server.\n");
		deleteMessage(m);
		return -1;
	}

	format = new BYTE[m.header.length];
	formatLength = m.header.length;

	printf("Got format length %ld\n", formatLength);

	memcpy(format, m.data, formatLength);

	deleteMessage(m);
	
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->AllocFormatBuffer(formatLength);
	memcpy(pMediaType->Format(), format, formatLength);

	pMediaType->SetTemporalCompression(true);
	pMediaType->SetType(&MEDIATYPE_Video);
	pMediaType->SetSubtype(&MEDIASUBTYPE_VP8);
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetSampleSize(0);
	pMediaType->lSampleSize = 0;
	pMediaType->pUnk = 0;
	pMediaType->cbFormat = formatLength;
	pMediaType->SetVariableSize();
	//pMediaType->IsFixedSize();

	delete(format);

	return S_OK;
}

HRESULT CVideoSpringRecvPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();
    
    // Ensure a minimum number of buffers
    if (pRequest->cBuffers == 0)
    {
        pRequest->cBuffers = 1;
    }
    pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr)) 
    {
		MessageBox(NULL, "Failed hr", "", MB_OK);
        return hr;
    }

    // Is this allocator unsuitable?
    if (Actual.cbBuffer < pRequest->cbBuffer) 
    {
		MessageBox(NULL, "Allocator unsuitable", "", MB_OK);
        return E_FAIL;
    }

/*    AM_MEDIA_TYPE mt;
    HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
    if (FAILED(hr))
    {
        return hr;
    }
	*/
	return S_OK;
}

//
// DoBufferProcessingLoop
//
// Grabs a buffer and calls the users processing function.
// Overridable, so that different delivery styles can be catered for.
HRESULT CVideoSpringRecvPin::DoBufferProcessingLoop(void) {

    Command com;

    OnThreadStartPlay();

    do {
	while (!CheckRequest(&com)) {

	    IMediaSample *pSample;

	    HRESULT hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
	    if (FAILED(hr)) {
                Sleep(1);
		continue;	// go round again. Perhaps the error will go away
			    // or the allocator is decommited & we will be asked to
			    // exit soon.
	    }

		// Virtual function user will override.
	    hr = FillBuffer(pSample);

	    if (hr == S_OK) {
		hr = Deliver(pSample);
                pSample->Release();

                // downstream filter returns S_FALSE if it wants us to
                // stop or an error if it's reporting an error.
                if(hr != S_OK)
                {
				  printf("Error deliverying sample!\n");
                  DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
                  return S_OK;
                }

	    } 
		else
		{
			pSample->Release();
		}
		/*else if (hr == S_FALSE) {
                // derived class wants us to stop pushing data
		pSample->Release();
		DeliverEndOfStream();
		return S_OK;
	    } else {
                // derived class encountered an error
                pSample->Release();
		DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
                DeliverEndOfStream();
                m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                return hr;
	    }*/

            // all paths release the sample

	}

        // For all commands sent to us there must be a Reply call!

	if (com == CMD_RUN || com == CMD_PAUSE) {
	    Reply(NOERROR);
	} else if (com != CMD_STOP) {
	    Reply((DWORD) E_UNEXPECTED);
	    DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
		printf("Unexpected command!!!\n");
	}
    } while (com != CMD_STOP);
    return S_FALSE;
}

// This is where we insert the DIB bits into the video stream.
// FillBuffer is called once for every sample in the stream.
HRESULT CVideoSpringRecvPin::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;

    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

    // Access the sample's data buffer
    pSample->GetPointer(&pData);

	// Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

/*	if(waiting == 0)
	{
		Message m;
		m.header.command = C_RECEIVE;
		m.header.length = 0;
		sendMessage(server, &m);
		waiting = 1;
	}
	else
	{
		fd_set read_set;

		FD_ZERO(&read_set);
		FD_SET(server, &read_set);
		if(select(0, &read_set, NULL, NULL, 0) < 1)
		{
			return S_FALSE;
		}
	
		if(FD_ISSET(server, &read_set) || frame == 1)
		{
			Message m;
			pSample->GetSize();
			receiveMessage(server, m);
			waiting = 0;

			Message m2;
			m2.header.command = C_RECEIVE;
			m2.header.length = 0;
			sendMessage(server, &m2);
			waiting = 1;

			if(m.header.length > 0)
			{
				memcpy(pData, m.data, m.header.length);
				frame++;
				return S_OK;
			}
		}
	}
	*/
		fd_set read_set;

		FD_ZERO(&read_set);
		FD_SET(server, &read_set);
		if(select(0, &read_set, NULL, NULL, 0) < 1)
		{
			return S_FALSE;
		}
	
		if(FD_ISSET(server, &read_set) || frame == 1)
		{
			Message m;
			pSample->GetSize();
			receiveMessage(server, m);

			if(m.header.length > 0)
			{
				memcpy(pData, m.data, m.header.length);
				frame++;
				return S_OK;
			}
		}
		
	
	return S_FALSE;
}


/**********************************************
 *
 *  CVideoSpringRecv Class
 *
 **********************************************/

CVideoSpringRecv::CVideoSpringRecv(IUnknown *pUnk, HRESULT *phr)
           : CSource(NAME("VideoSpringRecv"), pUnk, CLSID_VideoSpringRecv)
{
    // The pin magically adds itself to our pin array.
    m_pPin = new CVideoSpringRecvPin(phr, this);

    if (phr)
    {
        if (m_pPin == NULL)
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }  
}


CVideoSpringRecv::~CVideoSpringRecv()
{
    delete m_pPin;
}


CUnknown * WINAPI CVideoSpringRecv::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CVideoSpringRecv *pNewFilter = new CVideoSpringRecv(pUnk, phr );

    if (phr)
    {
        if (pNewFilter == NULL) 
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }

    return pNewFilter;
}

/********** END other file ****************/










////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

