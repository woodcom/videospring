#include "../../common/VideoSpringCommon.h"

/**********************************************
 *
 *  Class declarations
 *
 **********************************************/

class CVideoSpringRecvPin : public CSourceStream, public IVideoSpringRecv
{
protected:
/*
    CRefTime m_rtSampleTime;	        // The time stamp for each sample

    int m_iFrameNumber;
    const REFERENCE_TIME m_rtFrameLength;
	*/
    CCritSec m_cSharedState;            // Protects our internal state

private:

	SOCKET server;
	long presenterId;
	long frame;

public:

    CVideoSpringRecvPin(HRESULT *phr, CSource *pFilter);
    ~CVideoSpringRecvPin();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP SetServerSocket(SOCKET s);
	STDMETHODIMP SetPresenterId(long id);

	HRESULT Active(void);
	HRESULT Inactive(void);

    // Override the version that offers exactly one media type
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT	DoBufferProcessingLoop(void);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};




class CVideoSpringRecv : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance
    CVideoSpringRecv(IUnknown *pUnk, HRESULT *phr);
    ~CVideoSpringRecv();

    CVideoSpringRecvPin *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

};
