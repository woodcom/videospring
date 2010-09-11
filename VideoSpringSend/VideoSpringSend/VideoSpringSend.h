//------------------------------------------------------------------------------
// File: VideoSpringSend.h
//
// Desc: DirectShow sample code - header file for audio oscilloVideoSpringSend filter.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include "../../common/VideoSpringCommon.h"
#include <streams.h>

class CVideoSpringSendFilter;

// Class supporting the VideoSpringSend input pin

class CVideoSpringSendInputPin : public CBaseInputPin, public IVideoSpringSend
{
    friend class CVideoSpringSendFilter;

public:
	SOCKADDR_IN serveraddr;
	DECLARE_IUNKNOWN;

private:
	WSADATA data;
	SOCKET server;
	long formatLength;
	BYTE *format;
	long frame;
    CVideoSpringSendFilter *m_pFilter;         // The filter that owns us

public:

    CVideoSpringSendInputPin(CVideoSpringSendFilter *pTextOutFilter,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CVideoSpringSendInputPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP SetServerSocket(SOCKET s);

    // Lets us know where a connection ends
    HRESULT BreakConnect();

    // Check that we can support this input type
    HRESULT CheckMediaType(const CMediaType *pmt);

    // Actually set the current format
    HRESULT SetMediaType(const CMediaType *pmt);

    // IMemInputPin virtual methods

    // Override so we can show and hide the window
    HRESULT Active(void);
    HRESULT Inactive(void);

    // Here's the next block of data from the stream.
    // AddRef it if you are going to hold onto it
    STDMETHODIMP Receive(IMediaSample *pSample);

}; // CVideoSpringSendInputPin


// This is the COM object that represents the VideoSpringSend filter

class CVideoSpringSendFilter : public CBaseFilter, public CCritSec
{

public:
    // Implements the IBaseFilter and IMediaFilter interfaces

    DECLARE_IUNKNOWN


    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

public:

    CVideoSpringSendFilter(LPUNKNOWN pUnk,HRESULT *phr);
    virtual ~CVideoSpringSendFilter();

    // Return the pins that we support
    int GetPinCount();
    CBasePin *GetPin(int n);

    // This goes in the factory template table to create new instances
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);

    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

private:

    // The nested classes may access our private state
    friend class CVideoSpringSendInputPin;

    CVideoSpringSendInputPin *m_pInputPin;   // Handles pin interfaces

}; // CVideoSpringSendFilter

