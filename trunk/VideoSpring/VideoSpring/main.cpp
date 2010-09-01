#include "Capture.h"
#include "Player.h"

int main(int argc, char** argv)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
	    printf("Error.\n");
		exit(1);
	}

	Capture c;
	Player p;

	system("pause");

	return 0;
}
	/*
	printf("done.\n");

	printf("Creating Filter Graph Manager...");

	IGraphBuilder *graph;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);

	printf("done.\n");

    printf("Creating Media Control...");

	IMediaControl *pControl;
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);

	printf("done.\n");

	printf("Creating Media Event...");

	IMediaEvent   *pEvent;
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	printf("done.\n");

	IVideoWindow *pVideo;
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVideo);

/*	printf("Selecting capture device...\n");
	
	IBaseFilter *pCaptureFilter;
	hr = getVideoCaptureDevice(&pCaptureFilter);
	if(FAILED(hr))
	{
		printf("failed.\n");
	}
	else
	{
		printf("done.\n");
	}
	printf("Loading VideoSpringRecv filter...");
	IBaseFilter *recv;
	CoCreateInstance(CLSID_VideoSpringRecv, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&recv);
	printf("done.\n");

/*	printf("Loading VMR9...");
	IVMRFilterConfig9 *vmr9Config;
	IBaseFilter *vmr9;
	CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vmr9);
	vmr9->QueryInterface(IID_IVMRFilterConfig9, (void**)&vmr9Config);
	printf("done.\n");
	
	printf("Building Filter Graph...");

	IEnumPins *pins;
	IPin *pinOut;
	recv->EnumPins(&pins);
	pins->Next(1, &pinOut, NULL);
	pins->Release();

/*	IPin *pinIn;
	vmr9->EnumPins(&pins);
	pins->Next(1, &pinIn, NULL);
	
	pGraph->AddFilter(recv, NULL);
	//pGraph->AddFilter(vmr9, NULL);
	//pGraph->Connect(pinOut, pinIn); 
	pGraph->Render(pinOut);

	pinOut->Release();
	//pinIn->Release();
	//hr = pGraph->RenderFile(L"C:\\119_0093.MOV", NULL);

	printf("done.\n");

	printf("Running Filter Graph...");

	hr = pControl->Run();
//	vmr9Config->SetRenderingMode(
	//pVideo->put_FullScreenMode(-1);
	//pVideo->
	printf("done.\n");

	printf("Waiting for graph to end...");

	long evCode = 0;
	pEvent->WaitForCompletion(INFINITE, &evCode);

	printf("done.\n");

	printf("Cleaning up...");

	recv->Release();
/*	vmr9Config->Release();
	vmr9->Release();
	//pCaptureFilter->Release();
	pVideo->Release();
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();

	printf("done.\n");


	printf("\n");
	system("pause");
	return 0;
}*/
