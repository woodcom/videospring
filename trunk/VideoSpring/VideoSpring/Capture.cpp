#include "Capture.h"
#include "../../common/VideoSpringCommon.h"

HRESULT getVideoCaptureDevice(IBaseFilter **ret)
{
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					char buff[1024];
					// Display the name in your UI somehow.
					wprintf(L"\n%s y/n:\n", varName.bstrVal);
					scanf("%s", buff);
					if(buff[0] == 'y')
					{

						// To create an instance of the filter, do the following:
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)ret);
						return 0;
						break;
						// Now add the filter to the graph. 
						//Remember to release pFilter later.
					}
				}
				VariantClear(&varName);

				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

	return 1;
}

Capture::Capture()
{
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph);
	graph->QueryInterface(IID_IMediaControl, (void **)&control);
	graph->QueryInterface(IID_IMediaEvent, (void **)&event);
	graph->QueryInterface(IID_IVideoWindow, (void **)&video);

	if(!createGraph())
	{
		runGraph();
	}
}

Capture::~Capture()
{
	video->Release();
	control->Release();
	event->Release();
	graph->Release();
}

int Capture::runGraph()
{
	HRESULT hr;
	hr = control->Run();
//	long evCode;
//	event->WaitForCompletion(INFINITE, &evCode);
	return 0;
}

int Capture::createGraph()
{
	IBaseFilter *send, *cap;
	IPin *sendIn;
	IPin *capOut;
	IPin *encIn;
	IPin *encOut;
	IPin *colorIn;
	IPin *colorOut;
	IEnumPins *pins;
	HRESULT hr;
	PIN_INFO pi;

	if(getVideoCaptureDevice(&cap))
	{
		printf("No video capture devices found!\n");
		return 1;
	}

	graph->AddFilter(cap, NULL);

	IPin *pinOut;
	cap->EnumPins(&pins);

	do // Find the first output
	{
		pins->Next(1, &capOut, NULL);
		capOut->QueryPinInfo(&pi);
	}
	while(pi.dir != PINDIR_OUTPUT);

	pins->Release();

	hr = CoCreateInstance(CLSID_VideoSpringSend, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&send);
	hr = graph->AddFilter(send, NULL);

	IVP8Encoder *encoderControl;
	IBaseFilter *encoder;

	hr = CoCreateInstance(CLSID_VP8Encoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&encoder);
	encoder->QueryInterface(IID_IVP8Encoder, (void**)&encoderControl);

	encoderControl->SetDeadline(100);
	encoderControl->SetErrorResilient(1);
	encoderControl->SetKeyframeMode(kKeyframeModeDisabled);
	encoderControl->SetTargetBitrate(500);
	encoderControl->SetThreadCount(8);
	encoderControl->SetEndUsage(kEndUsageCBR);
	encoderControl->ApplySettings();

	hr = graph->AddFilter(encoder, NULL);

//	hr = CoCreateInstance(CLSID_CColorConvertDMO, NULL, CLSCTX_INPROC, IID_IMediaObject, (void**)&colorConvert);
//	hr = graph->AddFilter(colorConvert, NULL);

//	hr = colorConvert->FindPin(L"in0",  &colorIn);
//	hr = colorConvert->FindPin(L"out0",  &colorOut);

	hr = encoder->EnumPins(&pins);
	hr = pins->Next(1, &encIn, NULL);
	hr = pins->Next(1, &encOut, NULL);
	hr = send->FindPin(L"VideoSpringSend Input Pin", &sendIn);
	hr = graph->Connect(capOut, encIn);
//	hr = graph->Connect(colorOut, encIn);
	hr = graph->Connect(encOut, sendIn);

	return 0;
}
