#include "Capture.h"
#include "../../common/VideoSpringCommon.h"

HRESULT getVideoCaptureDevice(IBaseFilter **ret)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	
	HRESULT hr;

	if(FAILED(hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum)))
	{
		return hr;
	}

	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	
	if(FAILED(hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0)))
	{
		pSysDevEnum->Release();
		return hr;
	}
	else
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;

		system("cls");
		printf("CHOOSE CAPTURE DEVICE\n\n");

		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;

			if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag)))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);

				if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &varName, 0)))
				{
					char buff[1024];
					// Display the name in your UI somehow.
					wprintf(L"%s y/n: ", varName.bstrVal);
					scanf("%s", buff);

					if(buff[0] == 'y')
					{
						pPropBag->Release();
						pEnumCat->Release();
						pSysDevEnum->Release();

						HRESULT hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)ret);

						pMoniker->Release();

						return hr;
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

	return -1;
}

Capture::Capture(const char *ip)
{
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		printf("Error creating socket.\n");
		return;
	}
	
	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;
 	serveraddr.sin_addr.s_addr = inet_addr(ip);
	serveraddr.sin_port = htons(1234);

	if(connect(server, (SOCKADDR*)&serveraddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("Error connecting to server.\n");
		closesocket(server);
		return;
	}

	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph)))
	{
		printf("Failed to create graph!\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IMediaControl, (void **)&control)))
	{
		printf("Failed to create media control\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IMediaEvent, (void **)&event)))
	{
		printf("Failed to create media event\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IVideoWindow, (void **)&video)))
	{
		printf("Failed to create video window\n");
		return;
	}

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
	system("cls");

	printf("Starting Graph...");
	
	if(FAILED(control->Run()))
	{
		printf("Failed to run graph!\n");
		return 1;
	}

	return 0;
}

int Capture::createGraph()
{
	IBaseFilter *send, *cap, *encoder;
	IVideoSpringSend *sendControl;
	IPin *sendIn, *capOut, *encIn, *encOut, *colorIn, *colorOut, *pinOut;
	IEnumPins *pins;
	HRESULT hr;
	PIN_INFO pi;

	IAMStreamConfig *config;

	int deadline = 1;
	int bitrate = 500;
	int threadcount = 1;

	system("cls");

	printf("ENCODER SETTINGS\n\n");

//	printf("Enter deadline: ");
//	scanf("%d", &deadline);
	printf("Enter Bit Rate: ");
	scanf("%d", &bitrate);
//	printf("Enter Thread Count: ");
//	scanf("%d", &threadcount);

	if(getVideoCaptureDevice(&cap))
	{
		printf("No video capture devices found!\n");
		return 1;
	}

	if(FAILED(graph->AddFilter(cap, NULL)))
	{
		printf("Failed to add capture filter to graph!\n");
		return 1;
	}

	if(FAILED(cap->EnumPins(&pins)))
	{
		printf("Failed to enumerate capture filter pins!\n");
		return 1;
	}

	do
	{
		if(FAILED(pins->Next(1, &capOut, NULL)))
		{
			printf("Failed to get next pin!\n");
			continue;
		}

		if(FAILED(capOut->QueryPinInfo(&pi)))
		{
			printf("Failed to get pin info!\n");
			continue;
		}
	}
	while(pi.dir != PINDIR_OUTPUT);

	if(FAILED(pins->Release()))
	{
		printf("Failed to release pins!\n");
		return 1;
	}

	if(FAILED(CoCreateInstance(CLSID_VideoSpringSend, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&send)))
	{
		printf("Failed to create send filter!\n");
		return 1;
	}

	if(FAILED(graph->AddFilter(send, NULL)))
	{
		printf("Failed to add send filter to graph!\n");
		return 1;
	}

	if(FAILED(CoCreateInstance(CLSID_VP8Encoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&encoder)))
	{
		printf("Failed to load encoding filter!\n");
		return 1;
	}

	if(FAILED(encoder->QueryInterface(IID_IVP8Encoder, (void**)&encoderControl)))
	{
		printf("Failed to get encoder control!\n");
		return 1;
	}
	
	if(FAILED(graph->AddFilter(encoder, NULL)))
	{
		printf("Failed to add encoder to graph!\n");
		return 1;
	}

	if(FAILED(encoder->EnumPins(&pins)))
	{
		printf("Failed to enumerate encoder pins!\n");
		return 1;
	}

	if(FAILED(pins->Next(1, &encIn, NULL)))
	{
		printf("Failed to get next encoder pin!");
		return 1;
	}

	if(FAILED(pins->Next(1, &encOut, NULL)))
	{
		printf("Failed to get next encoder pin!");
		return 1;
	}

	if(FAILED(send->FindPin(L"VideoSpringSend Input Pin", &sendIn)))
	{
		printf("Failed to find input pin!\n");
		return 1;
	}

	if(FAILED(capOut->QueryInterface(IID_IAMStreamConfig, (void**)&config)))
	{
		printf("Failed to get capture pin config!\n");
		return 1;
	}

	AM_MEDIA_TYPE *format;
	VIDEO_STREAM_CONFIG_CAPS caps;

	int numCaps, capSize;

	if(FAILED(config->GetNumberOfCapabilities(&numCaps, &capSize)))
	{
		printf("Failed to get number of capabilities!\n");
		return 1;
	}

	system("cls");

	printf("CHOOSE OUTPUT FORMAT\n\n");

	printf("Framerate: ");
	int frameRate;
	scanf("%d", &frameRate);

	for(int i = 0; i < numCaps; i++)
	{
		if(FAILED(config->GetStreamCaps(i, &format, (BYTE*)&caps)))
		{
			printf("Failed to get stream caps!\n");
			continue;
		}

		VIDEOINFOHEADER *video = (VIDEOINFOHEADER*)format->pbFormat;

		char buff[1024];

		printf("%dx%d @ %d-bit y/n: ", video->bmiHeader.biWidth, video->bmiHeader.biHeight, video->bmiHeader.biBitCount, video->AvgTimePerFrame);

		scanf("%s", buff);
		if(buff[0] == 'y')
		{
			video->AvgTimePerFrame = (LONGLONG)(10000000 / frameRate);

			if(FAILED(config->SetFormat(format)))
			{
				printf("Failed to set format!\n");
				continue;
			}
			break;
		}
	}

	encoderControl->ResetSettings();
	encoderControl->SetDropframeThreshold(25);
	encoderControl->SetTokenPartitions(3);
	encoderControl->SetUndershootPct(95);
	encoderControl->SetDeadline(deadline);
	encoderControl->SetTargetBitrate(bitrate);
	encoderControl->SetThreadCount(threadcount);
	encoderControl->SetEndUsage(kEndUsageCBR);
	encoderControl->SetErrorResilient(1);
	encoderControl->SetKeyframeMode(kKeyframeModeAuto);
//	encoderControl->SetKeyframeMinInterval(0);
//	encoderControl->SetKeyframeMaxInterval(30);
//	encoderControl->SetMaxQuantizer(4);
//	encoderControl->SetMinQuantizer(56);
//	encoderControl->SetDecoderBufferSize(6);
//	encoderControl->SetDecoderBufferInitialSize(4);
//	encoderControl->SetDecoderBufferOptimalSize(5);
	encoderControl->SetResizeAllowed(1);
//	encoderControl->SetForceKeyframe();
	encoderControl->ApplySettings();
	
	IBaseFilter *colorConvert;

	if(SUCCEEDED(CoCreateInstance(CLSID_DMOWrapperFilter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&colorConvert)))
	{
		IDMOWrapperFilter *colorConvertIface;
		
		if(FAILED(colorConvert->QueryInterface(IID_IDMOWrapperFilter, (void **)&colorConvertIface)))
		{
			printf("Failed to get color convert interface!\n");
			goto standard;
			return 1;
		}

		if(FAILED(colorConvertIface->Init(CLSID_CColorConvertDMO, DMOCATEGORY_VIDEO_EFFECT)))
		{
			printf("Failed to init color converter!\n");
			goto standard;
			return 1;
		}

		if(FAILED(graph->AddFilter(colorConvert, NULL)))
		{
			printf("Failed to add color filter to graph!\n");
			goto standard;
			return 1;
		}

		if(FAILED(colorConvert->FindPin(L"in0",  &colorIn)))
		{
			printf("Failed to find input pin!\n");
			goto standard;
			return 1;
		}

		if(FAILED(colorConvert->FindPin(L"out0",  &colorOut)))
		{
			printf("Failed to find output pin!\n");
			goto standard;
			return 1;
		}

		if(FAILED(graph->Connect(capOut, colorIn)))
		{
			printf("Failed to connect capture filter to color converter!\n");
			goto standard;
			return 1;
		}

		if(FAILED(graph->Connect(colorOut, encIn)))
		{
			printf("Failed to connect color converter to encoder!\n");
			return 1;
		}
	}
	else
	{
		standard:
		if(FAILED(graph->Connect(capOut, encIn)))
		{
			printf("Failed to connect capture filter to encoder!\n");
			return 1;
		}
	}
	
	if(FAILED(graph->Connect(encOut, sendIn)))
	{
		printf("Failed to connect encoder to sender!\n");
	}

	if(FAILED(sendIn->QueryInterface(IID_IVideoSpringSend, (void**)&sendControl)))
	{
		printf("Failed to get send control diff!\n");
		return 1;
	}

	sendControl->SetServerSocket(server);


	return 0;
}
