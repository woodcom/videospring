#include "VideoSpringSend.h"


// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,           // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins  =
{
    L"Input",                   // Pin string name
    FALSE,                      // Is it rendered
    FALSE,                      // Is it an output
    FALSE,                      // Allowed zero pins
    FALSE,                      // Allowed many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of pins types
    &sudPinTypes				// Pin information
};

const AMOVIESETUP_FILTER sudVideoSpringSend =
{
    &CLSID_VideoSpringSend,     // Filter CLSID
    L"VideoSpringSend",         // String name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};



// List of class IDs and creator functions for class factory

CFactoryTemplate g_Templates[] = {
    { L"OscilloVideoSpringSend"
    , &CLSID_VideoSpringSend
    , CVideoSpringSendFilter::CreateInstance
    , NULL
    , &sudVideoSpringSend }
};
int g_cTemplates = 1;





//
// Constructor
//
// Create the filter, VideoSpringSend window, and input pin
//

CVideoSpringSendFilter::CVideoSpringSendFilter(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseFilter(NAME("VideoSpringSend"), pUnk, (CCritSec *) this, CLSID_VideoSpringSend)
{
    ASSERT(phr);
	
	m_pInputPin = NULL;

    // Create the single input pin
    m_pInputPin = new CVideoSpringSendInputPin(this,phr,L"VideoSpringSend Input Pin");
    if(m_pInputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

} // (Constructor)


//
// Destructor
//
CVideoSpringSendFilter::~CVideoSpringSendFilter()
{
    // Delete the contained interfaces
printf("Deconstructing VideSpringSendFilter...");
    ASSERT(m_pInputPin);
	delete(m_pInputPin);
printf("done.\n");

} // (Destructor)

//
// CreateInstance
//
// This goes in the factory template table to create new instances
//
CUnknown * WINAPI CVideoSpringSendFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CVideoSpringSendFilter(pUnk, phr);

} // CreateInstance

//
// GetPinCount
//
// Return the number of input pins we support
//
int CVideoSpringSendFilter::GetPinCount()
{
    return 1;

} // GetPinCount


//
// GetPin
//
// Return our single input pin - not addrefed
//
CBasePin *CVideoSpringSendFilter::GetPin(int n)
{
    // We only support one input pin and it is numbered zero

    ASSERT(n == 0);
    if(n != 0)
    {
        return NULL;
    }

    return m_pInputPin;

} // GetPin


//
// Stop
//
// Switch the filter into stopped mode.
//
STDMETHODIMP CVideoSpringSendFilter::Stop()
{
    CAutoLock lock(this);

	return CBaseFilter::Stop();
} // Stop


//
// Pause
//
// Override Pause to stop the window streaming
//
STDMETHODIMP CVideoSpringSendFilter::Pause()
{
    CAutoLock lock(this);

    // tell the pin to go inactive and change state
    return CBaseFilter::Pause();

} // Pause


//
// Run
//
// Overriden to start the window streaming
//
STDMETHODIMP CVideoSpringSendFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(this);

	return CBaseFilter::Run(tStart);
} // Run


//
// Constructor
//
CVideoSpringSendInputPin::CVideoSpringSendInputPin(CVideoSpringSendFilter *pFilter,
                               HRESULT *phr,
                               LPCWSTR pPinName) :
    CRenderedInputPin(NAME("VideoSpringSend Input Pin"), pFilter, pFilter, phr, pPinName),
	IVideoSpringSend()
{
	frame = 0;
    m_pFilter = pFilter;
} // (Constructor)


//
// Destructor does nothing
//
CVideoSpringSendInputPin::~CVideoSpringSendInputPin()
{
} // (Destructor)


STDMETHODIMP CVideoSpringSendInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_IVideoSpringSend)
	{
		return GetInterface(static_cast<IVideoSpringSend*>(this), ppv);
	}
	else
	{
		return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
	}
}


STDMETHODIMP CVideoSpringSendInputPin::SetServerSocket(SOCKET s)
{
	server = s;

	return S_OK;
}

//
// CheckMediaType
//
// Check that we can support a given proposed type
//
HRESULT CVideoSpringSendInputPin::CheckMediaType(const CMediaType *pmt)
{
    return S_OK;

} // CheckMediaType

//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and allows us to reset the connection media type to be invalid so that
// we can always use that to determine whether we are connected or not. We
// leave the format block alone as it will be reallocated if we get another
// connection or alternatively be deleted if the filter is finally released
//
HRESULT CVideoSpringSendInputPin::BreakConnect()
{
    // Check we have a valid connection
	if(m_mt.IsValid() == FALSE)
    {
        // Don't return an error here, because it could lead to 
        // ASSERT failures when rendering media files in GraphEdit.
        return S_FALSE;
    }

    m_pFilter->Stop();

    // Reset the CLSIDs of the connected media type

    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);
    return CBaseInputPin::BreakConnect();

} // BreakConnect

//
// ReceiveCanBlock
//
// We don't hold up source threads on Receive
//
STDMETHODIMP CVideoSpringSendInputPin::ReceiveCanBlock()
{
    return S_FALSE;
}



//
// SetMediaType
//
// Actually set the format of the input pin
//
HRESULT CVideoSpringSendInputPin::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);
    CAutoLock lock(m_pFilter);

	this->format = new BYTE[pmt->cbFormat];
	memcpy(this->format, pmt->Format(), pmt->cbFormat);
	this->formatLength = pmt->cbFormat;

    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    
    return hr;

} // SetMediaType


//
// Active
//
// Implements the remaining IMemInputPin virtual methods
//
HRESULT CVideoSpringSendInputPin::Active(void)
{
	Message m;
	m.header.command = C_SET_PRESENTER_SEND;
	m.header.length = sizeof(uint32_t);

	DWORD pid = GetCurrentProcessId();
	m.data = (BYTE*)&pid;

	if(sendMessage(server, &m) == -1)
	{
		printf("VideoSpringSend.dll::CVideoSpringSendInputPin::Active Failed to send message\n");
		return -1;
	}

	return S_OK;
} // Active


//
// Inactive
//
// Called when the filter is stopped
//
HRESULT CVideoSpringSendInputPin::Inactive(void)
{
    return NOERROR;

} // Inactive


//
// Receive
//
// Here's the next block of data from the stream
//
HRESULT CVideoSpringSendInputPin::Receive(IMediaSample * pSample)
{
	// Lock this with the filter-wide lock
    CAutoLock lock(m_pFilter);

	Message m;
	
	if(frame == 0) // Send format info if first frame
	{
		printf("%ld sending format\n", formatLength);
		m.header.command = C_SET_FORMAT;
		m.header.length = formatLength;
		m.data = format;

		sendMessage(server, &m);
		printf("Sent format\n");
	}

	m.header.command = C_BROADCAST;
	m.header.length = pSample->GetActualDataLength();
	pSample->GetPointer(&(m.data));

	if(sendMessage(server, &m) == -1)
	{
		printf("Error sending frame.\n");
		return -1;
	}

	frame++;

	return NOERROR;
} // Receive


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////
#include <io.h>
//
// DllRegisterServer
//
// Handles DLL registry
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);

} // DllUnregisterServer


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

