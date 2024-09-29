#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Winuser.h>
#include <Commctrl.h>
#include <stdio.h>

#include <shlobj.h>
#include <shlwapi.h>
 
#include <Objbase.h>
#include <oleauto.h>

#include <string.h>
#include <wchar.h>

#include <Bthsdpdef.h>
#include <BluetoothAPIs.h>
#include <bthdef.h> 
 
#include <initguid.h>
#include <mmreg.h>
#include <string>
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <iostream>
#include <Mmiscapi.h>
#include <avrt.h>
#include <Mmiscapi.h>
#include <vfw.h>
#include <Dshow.h>
#include <d3d9.h>
#include <Vmr9.h>
#include <Evr.h>
#include <dbt.h> 
 

#include <richedit.h>

#include <bcrypt.h>

#include <Wmcodecdsp.h>

#include <mferror.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Bthprops.lib") 
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Avrt.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Quartz.lib") 
#pragma comment(lib, "Kernel32.lib")

#pragma comment(lib, "Gdi32.lib")

#pragma comment(lib, "Bcrypt.lib")


#define ID_BUTTON_List_rules  12
#define ID_BUTTON_user_manual  14
#define ID_BUTTON_about_us  16
#define ID_BUTTON_reset_firewall  18
#define ID_BUTTON_show_list_of_songs  20
#define ID_BUTTON_pause_resume  22
#define ID_BUTTON_stop  24


#define IDT_TIMER1  1000

HWND btn;
HWND btn2;
HWND btn3;
HWND btn4;
HWND btn5;
HWND btn6;
HWND btn7;
 
 
HANDLE folder;

HWND hWndListView;
HANDLE thread;
HANDLE thread2;
 
/*********** EXPERIMENT ************/
PWSTR argument;
/**********************************/

FILE *stream;

bool isPlayingAudio = false;

IMediaSeeking *pMS = NULL;

bool isPlayingVideo = false;
bool isVideoPaused = false;

IGraphBuilder *pGraph;
IVMRWindowlessControl* pWc;
IMediaControl *pControl = NULL;
IMediaEvent *pEvent;
IBaseFilter* pVmr;
IVMRFilterConfig* pConfig;
long lWidth, lHeight;
long evCode = 0;


IAudioClient *pAudioClient = NULL;
HANDLE hTask = NULL;
IMMDeviceEnumerator *pEnumerator = NULL;
IMMDevice *pDevice = NULL;
IAudioRenderClient *pRenderClient = NULL;
BYTE *pData;
UINT32 nFramesInBuffer;
HANDLE hNeedDataEvent;
IMFSample *pSample = NULL;
IMFMediaBuffer *pBuffer = NULL;
BYTE *pAudioData = NULL;
HMMIO hFile2;
IMFSourceReader *pReader = NULL;
IMFMediaType *pUncompressedAudioType = NULL;
IMFMediaType *pPartialType = NULL;
HRESULT hr;

IPropertyStore *pProps;
WAVEFORMATEX *pWfx;
WAVEFORMATEX *pWfxx;

DWORD cbWritten;
DWORD cbBuffer;
DWORD cbMaxAudioData;
DWORD cbAudioData;

/******************* EXPERIMENT WORKS ***************************/
long bytePositionToSeek = 0;

/**********************************************************/
// UINT_PTR IDT_TIMER1;


HANDLE firstBluetoothRadio;
BLUETOOTH_FIND_RADIO_PARAMS findRadioParams;

typedef struct MyData {
	wchar_t urlOfSong[260];
	HWND windowHandle;
} MYDATA, *PMYDATA;

PMYDATA pDataStr;

void showError(LPWSTR msg);


DWORD WINAPI playVideo(LPVOID hwnd);



IMFPMediaPlayer *g_pPlayer = NULL;


class MediaPlayerCallback : public IMFPMediaPlayerCallback
{
	long m_cRef; // Reference count

public:

	MediaPlayerCallback() : m_cRef(1)
	{
	}

	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MediaPlayerCallback, IMFPMediaPlayerCallback),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
		 
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		ULONG count = InterlockedDecrement(&m_cRef);
		if (count == 0)
		{
			delete this;
			return 0;
		}
		return count;
	}

	IFACEMETHODIMP_(void) OnMediaPlayerEvent(MFP_EVENT_HEADER *pEventHeader);
};


void MediaPlayerCallback::OnMediaPlayerEvent(MFP_EVENT_HEADER * pEventHeader)
{
	switch (pEventHeader->eEventType)
	{
		case MFP_EVENT_TYPE_MEDIAITEM_SET:
		{
			hr = g_pPlayer->SetAspectRatioMode(MFVideoARMode_None);
										if(FAILED(hr))
										{
											showError(L"FAILED g_pPlayer->SetAspectRatioMode");
											if(hr == MF_E_INVALIDREQUEST)
											{
												showError(L"FAILED g_pPlayer->SetAspectRatioMode hr == MF_E_INVALIDREQUEST");
											}
										}

			g_pPlayer->Play(); 
			break;
		}
	}
}



 
DWORD WINAPI extractVideoOnly(LPVOID hWnd)
{
	HWND hwnd = (HWND) hWnd;

	IMFSourceReader *pReader;
	const WCHAR *pszURL = L"1215epi.mp4";
	// const WCHAR *pszURL = L"sia.mp4";

	hr = MFCreateSourceReaderFromURL(pszURL, NULL, &pReader);
	if(FAILED(hr))
	{
	 	// printf("FAILED CoCreateInstance\n");
	 	showError(L"FAILED MFCreateSourceReaderFromURL\n");
	}

	IMFMediaType *pType = NULL;
	hr = pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, MF_SOURCE_READER_CURRENT_TYPE_INDEX, &pType);
	if(FAILED(hr))
	{
	 	// printf("FAILED CoCreateInstance\n");
	 	showError(L"FAILED GetNativeMediaType \n");
	}

	hr = pReader->SetStreamSelection(
		(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
	if(FAILED(hr))
	{
	 	// printf("FAILED CoCreateInstance\n");
	 	showError(L"FAILED SetStreamSelection 2\n");
	}

	IMFSinkWriter *pSinkWriter = NULL;
	hr = MFCreateSinkWriterFromURL(L"sinkwriter.wmv", NULL, NULL, &pSinkWriter);  // THIS WORKS
	if(FAILED(hr))
	{
		printf("FAILED MFCreateSinkWriterFromURL\n");
		showError(L"FAILED MFCreateSinkWriterFromURL\n");
	}

	DWORD streamIndex;
	hr = pSinkWriter->AddStream(pType, &streamIndex);
	if(FAILED(hr))
	{
		printf("FAILED pSinkWriter->AddStream\n");
		showError(L"FAILED pSinkWriter->AddStream\n");
	}

	 hr = MFTRegisterLocalByCLSID(
            __uuidof(CColorConvertDMO),
            MFT_CATEGORY_VIDEO_PROCESSOR,
            L"",
            MFT_ENUM_FLAG_SYNCMFT,
            0,
            NULL,
            0,
            NULL
            );
	if(FAILED(hr))
	{
		printf("FAILED MFTRegisterLocalByCLSID\n");
		showError(L"FAILED MFTRegisterLocalByCLSID\n");
	} 

	hr = pSinkWriter->SetInputMediaType(streamIndex, pType, NULL);
	if(FAILED(hr))
	{
		printf("FAILED pSinkWriter->SetInputMediaType\n");
		showError(L"FAILED SetInputMediaType\n");
	}

	hr = pSinkWriter->BeginWriting();
	if(FAILED(hr))
	{
		printf("FAILED pSinkWriter->BeginWriting\n");
		showError(L"FAILED pSinkWriter->BeginWriting\n");
	} 

	hr = pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL , pType);
	if(FAILED(hr))
	{
		printf("FAILED pReader->SetCurrentMediaType\n");
		showError(L"FAILED pReader->SetCurrentMediaType\n");
	}



	bool quit = false;
	DWORD streamIndexx, flags;
	LONGLONG llTimeStamp;
	IMFSample *pSample = NULL;
	BYTE *pVideoData = NULL;
	DWORD cbBuffer = 0;
	while (!quit)
	{
		hr = pReader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM, // Stream index.
			0, // Flags.
			&streamIndexx, // Receives the actual stream index.
			&flags, // Receives status flags.
			&llTimeStamp, // Receives the time stamp.
			&pSample // Receives the sample or NULL.
			);

		if(FAILED(hr))
		{
			printf("FAILED pReader->ReadSample\n");
			showError(L"FAILED pReader->ReadSample\n");
		}

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			printf("End of stream\n");
			showError(L"End of stream\n");
			quit = true;
			break;
		}

		hr = pSample->ConvertToContiguousBuffer(&pBuffer);		 
		if(FAILED(hr))
		{
			printf("FAILED pSample->ConvertToContiguousBuffer\n");
			showError(L"FAILED pSample->ConvertToContiguousBuffer\n");
		}

		hr = pBuffer->Lock(&pVideoData, NULL, &cbBuffer);
		if(FAILED(hr))
		{
			printf("FAILED pBuffer->Lock\n");
			showError(L"FAILED pBuffer->Lock\n");
		}

		hr = pSinkWriter->WriteSample(streamIndex, pSample);
		if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pSinkWriter->WriteSample\n");
		}

		hr = pBuffer->Unlock(); 
		if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pBuffer->Unlock\n");
		}
		 

		pVideoData = NULL;

		pBuffer->Release();
		if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pBuffer->Release\n");
		}

		pSample->Release();
		if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pSample->Release\n");
		}



	}

	pReader->Release();
	if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pReader->Release\n");
		}

	pType->Release();
	if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pType->Release\n");
		}

	pSinkWriter->Release();
	if(FAILED(hr))
		{
			printf("FAILED pSinkWriter->WriteSample\n");
			showError(L"FAILED pSinkWriter->Release\n");
		}


	  DWORD threadIdentifier; 
						     thread2 = CreateThread(
							NULL,
							0,
							playVideo,
							  hwnd,
							0,
							&threadIdentifier);

							if(thread2 == NULL)
							{
								showError(L"FAILED CreateThread\n");
							}  

 


	return 0;
}

 

wchar_t * decryptFile(wchar_t * fileName)
{
	BCRYPT_ALG_HANDLE hAlg = NULL;
	if(ERROR_SUCCESS != BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0))
	{
		printf("FAILED BCryptOpenAlgorithmProvider");
	}

	DWORD cbData = 0, cbKeyObject = 0;
	if(ERROR_SUCCESS != BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0))
	{
		printf("FAILED BCryptGetProperty BCRYPT_OBJECT_LENGTH");
	}

	PBYTE pbKeyObject = NULL;
	pbKeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbKeyObject);
	if(NULL == pbKeyObject)
	{
		printf("**** memory allocation failed\n");		 
	}

	DWORD cbBlockLen = 0;
	if(ERROR_SUCCESS != BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbBlockLen, sizeof(DWORD), &cbData, 0))
	{
		printf("FAILED BCryptGetProperty BCRYPT_BLOCK_LENGTH");
	}

	FILE *stream;
	FILE *stream2;
	if((stream = _wfopen(fileName, L"rb" )) == NULL)
	// if((stream = fopen( "siaAudioOnly.encrypted", "rb" )) == NULL)
	{
		printf("FAILED fopen");
	}

	if((stream2 = _wfopen( L"siaAudioOnly.decrypted", L"w+b" )) == NULL)
	{
		printf("FAILED fopen");
	}

	BYTE key[16];
	BYTE iv[16];

	FILE *stream3;
	FILE *stream4;
	if((stream3 = fopen( "key.key", "rb" )) == NULL)
	{
		printf("FAILED fopen");
	}

	if((stream4 = fopen( "iv.iv", "rb" )) == NULL)
	{
		printf("FAILED fopen");
	}

	int numread;
		numread = fread( key, sizeof( char ), 16, stream3 );
		if(numread == 0)
		{
			printf("FAILED fread");
		}
		printf("key: %d bytes\n", numread);


		numread = fread( iv, sizeof( char ), 16, stream4 );
		if(numread == 0)
		{
			printf("FAILED fread");
		}
		printf("iv: %d bytes\n", numread);

		if (cbBlockLen > sizeof (iv))
		{
			printf("**** block length is longer than the provided IV length\n");
		}

		PBYTE pbIV = NULL;
		pbIV= (PBYTE) HeapAlloc (GetProcessHeap (), 0, cbBlockLen);
		if(NULL == pbIV)
		{
			printf("**** memory allocation failed\n");
		}

		memcpy(pbIV, iv, cbBlockLen);

		if(ERROR_SUCCESS !=  BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0) )
		{
			printf("FAILED BCryptSetProperty");
		}

		BCRYPT_KEY_HANDLE hKey = NULL;
		if(ERROR_SUCCESS != BCryptGenerateSymmetricKey(hAlg, &hKey, pbKeyObject, cbKeyObject, (PBYTE)key, sizeof(key), 0) )
		{
			printf("FAILED BCryptGenerateSymmetricKey");
		}

		BYTE message[1024];
		DWORD cbPlainText = 0;
		PBYTE pbPlainText = NULL;
		do{
			numread = fread( message, sizeof( char ), 1024, stream );
			printf("Encrypted bytes read: %d\n", numread);
			if(ERROR_SUCCESS != BCryptDecrypt(hKey, message, numread, NULL, pbIV, cbBlockLen, NULL, 0, &cbPlainText, BCRYPT_BLOCK_PADDING) )
			{
				printf("FAILED BCryptDecrypt 1 \n");
				return 0;
			}

			printf("cbPlainText %d\n", cbPlainText);

			pbPlainText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbPlainText);
			if(NULL == pbPlainText)
			{
				printf("**** memory allocation failed\n");
			}

			if(numread < 1024)
			{
				if(ERROR_SUCCESS != BCryptDecrypt(hKey, message, numread, NULL, pbIV, cbBlockLen, pbPlainText, cbPlainText, &cbPlainText, BCRYPT_BLOCK_PADDING) )
				{
					printf("FAILED BCryptDecrypt 2 \n");
					return 0;
				}

				fwrite( pbPlainText, sizeof( char ), cbPlainText, stream2 );

				HeapFree(GetProcessHeap(), 0, pbPlainText);
			}else{  
				
				// THE BELOW BCryptDecrypt won't work correctly unless used with the above if(numread < 1024) code AND BCryptDecrypt(..., BCRYPT_BLOCK_PADDING)
				// THE BELOW BCryptDecrypt will fail only this is used BCryptDecrypt(hKey, message, numread, NULL, pbIV, cbBlockLen, pbPlainText, cbPlainText, &cbPlainText, BCRYPT_BLOCK_PADDING)
				// I think BCRYPT_BLOCK_PADDING is supposed to be used on the final decryption block. The documentation doesn't show how it's supposed to be used if a file was encrypted not using WINDOWS APIs eg. OPENSSL on Ubuntu.
				// I discovered this flaw on my own.
				if(ERROR_SUCCESS != BCryptDecrypt(hKey, message, numread, NULL, pbIV, cbBlockLen, pbPlainText, cbPlainText, &cbPlainText, 0) )
				{
					printf("FAILED BCryptDecrypt 2 \n");
					return 0;
				}

				fwrite( pbPlainText, sizeof( char ), cbPlainText, stream2 );

				HeapFree(GetProcessHeap(), 0, pbPlainText);

			}

		}while(!feof(stream));



		BCryptCloseAlgorithmProvider(hAlg, 0);
		BCryptDestroyKey(hKey);
		HeapFree(GetProcessHeap(), 0, pbKeyObject);
		HeapFree(GetProcessHeap(), 0, pbIV);

	 _fcloseall();




	return L"siaAudioOnly.decrypted";
}

// int pauseResume()
int pauseResume(HWND hwnd)
{
	/***************** EXPERIMENT ***************************/
				if(pAudioClient != NULL && pControl == NULL)
				{
					if(isPlayingAudio)
					{
						 hr = pAudioClient->Stop();
							if(FAILED(hr))
							{
									printf("FAILED pAudioClient->Stop pausePlayback\n");	
							}else{
								isPlayingAudio = false;

								/****************** EXPERIMENT *****************/
										KillTimer(hwnd, IDT_TIMER1);
								/**********************************************/

								return 0;
							} 
					}

					if(!isPlayingAudio)
					{
						  hr = pAudioClient->Start();
							if(FAILED(hr))
							{
									printf("Failed pAudioClient->Start resumePlayback\n");	
							}else{
								isPlayingAudio = true;

								/************** EXPERIMENT ***************************/
									SetTimer(hwnd, // handle to main window
												IDT_TIMER1, // timer identifier
												60000, // 60-second interval
												(TIMERPROC) NULL); // no timer callback
								/*****************************************************/

								return 0;
							}  
					}
				}

			/*******************************************************/ 		

			/************** EXPERIMENT WORKS************************/
				if(pControl != NULL && pAudioClient != NULL)
				{

					if(isPlayingVideo || isPlayingAudio) // I USED || FOR TESTING PURPOSES. MODIFY WHEN REQUIRED
					{
						
						hr = pControl->Pause();
						if(FAILED(hr))
						{
							showError(L"FAILED pControl->Pause\n");
						}else{
							  isVideoPaused = true;
							  isPlayingVideo = false;
							//  return 0;
						} 

							/************ THIS CODE WORKS ************************
							LONGLONG pCurrent = 100 * 10000000;
							hr = pMS->SetPositions(&pCurrent, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
							if(FAILED(hr))
							{
								showError(L"pMS->SetPositions\n");
							} ***********************/

						/**************** EXPERIMENT WORKS***************************/
							
							 hr = pAudioClient->Stop();
							if(FAILED(hr))
							{
									printf("FAILED pAudioClient->Stop pausePlayback\n");	
							}else{
								isPlayingAudio = false;
								return 0;
							} 

							/********* THIS AUDIO SEEKING CODE WORKS ****************
							bytePositionToSeek = 176400 * 100; // THIS VARIABLE WORKS ALONGSIDE mmioSeek() inside the Core Audio rendering for-loop
							return 0;
							********************/

						/******************************************************/

					}

					if(isVideoPaused || !isPlayingAudio) // I USED || FOR TESTING PURPOSES. MODIFY WHEN REQUIRED
					{
						hr = pControl->Run();

						if(FAILED(hr))
						{
							showError(L"FAILED pControl->Run\n");
						}else{
							  isVideoPaused = false;
							  isPlayingVideo = true;
							//  return 0;
						}

						/*************** EXPERIMENT ************************/
							
							hr = pAudioClient->Start();
							if(FAILED(hr))
							{
									printf("Failed pAudioClient->Start resumePlayback\n");	
							}else{
								isPlayingAudio = true;
								return 0;
							}

						/***************************************************/
					}
				}
			/**************************************************/

	return 0;
}

int showListOfSongs(HWND hwnd)
{
	INITCOMMONCONTROLSEX icex; // Structure for control initialization.
							icex.dwICC = ICC_LISTVIEW_CLASSES;
							InitCommonControlsEx(&icex);

							
							RECT rcClient; // The parent window's client area.
							GetClientRect (hwnd, &rcClient);

							  hWndListView = CreateWindow(WC_LISTVIEW,
																L"",
																WS_OVERLAPPEDWINDOW | LVS_REPORT | LVS_NOCOLUMNHEADER, 
																0, 0,
																600,
																600,
																// rcClient.right - rcClient.left,
																// rcClient.bottom - rcClient.top,
																hwnd,
																NULL, // Menu
																(HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
																NULL // Additional application data
																);
							 

							LVITEM lvI2;							
							lvI2.mask = LVIF_TEXT;
							lvI2.stateMask = 0;
							lvI2.iSubItem = 0;
							lvI2.state = 0;							
							int i = 0;

							/********************* EXPERIMENT ***************************/
									WCHAR szText[256]; // Temporary buffer.
										StringCchCopy(szText, 256, L"List of active rules.");
										LVCOLUMN lvc;
										int iCol = 0;
										lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
										lvc.iSubItem = iCol;
										lvc.pszText = szText;
										lvc.cx = 500;
										ListView_InsertColumn(hWndListView, iCol, &lvc); 

								COLORREF rgbGreen = 0x0000ffad;
							    COLORREF rgbWhite = 0x00000000;
								ListView_SetBkColor(hWndListView, rgbGreen);
								ListView_SetTextBkColor(hWndListView, rgbGreen);

								LOGFONT lf; 
							    ZeroMemory(&lf, sizeof(lf));
							    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Ink Free");
							      StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Gabriola");
							    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Calibri");
							    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"AR CARTER");
							    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Times New Roman");
							    lf.lfWeight = FW_NORMAL;
							    HFONT hFont = CreateFontIndirect(&lf);
							    SendMessage(hWndListView, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
							/***********************************************************/
							
							

							WIN32_FIND_DATA ffd;
							HANDLE hFind;
							DWORD dwError = 0;

							TCHAR szDir[MAX_PATH];
							WCHAR * pth = L"C:\\Users\\hp\\Desktop\\ProgrammingPrac\\Media Player final version";

							 StringCchCopy(szDir, MAX_PATH, pth);
							// StringCchCat(szDir, MAX_PATH, TEXT("\\*.txt"));
							 StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

							hFind = FindFirstFile(szDir, &ffd);
							if (INVALID_HANDLE_VALUE == hFind)
							{		 
								MessageBox(NULL, L"FAILED FindFirstFile\n", L"ERROR", MB_OK);
							}else{
								do{									
 									 if(! (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
									 { 									
										if(NULL != wcsstr(ffd.cFileName, L".encrypted"))
										// if(NULL != wcsstr(ffd.cFileName, L".mp3")) THIS PART WORKS
										{
											lvI2.iItem = i;
											lvI2.pszText = ffd.cFileName;

											if (ListView_InsertItem(hWndListView, &lvI2) == -1)
											{								
												MessageBox(NULL, L"FAILED ListView_InsertItem\n", L"ERROR", MB_OK);
											}

											i++;
										}
																				
									 }

									

								}while (FindNextFile(hFind, &ffd) != 0);


								dwError = GetLastError();
								if (dwError == ERROR_NO_MORE_FILES)
								{ 
									MessageBox(NULL, L"NO MORE FILES\n", L"ALERT", MB_OK);
								}

							}		

							ShowWindow(hWndListView, SW_SHOW);	

	return 0;
}


void showError(LPWSTR msg)
{
	 

					/*************************** EXPERIMENT ************************/
						if((stream = fopen( "errorLog.txt", "a" )) == NULL)
							{
									int msgboxID = MessageBox(
									NULL,
									(LPCWSTR)L"FAILED fopen(...",
									(LPCWSTR)L"ERROR",
									MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
									); 
							}

							 	
							  if(0 == fprintf_s(stream, "%S\n",  msg))
							  {
							  	int msgboxID = MessageBox(
									NULL,
									(LPCWSTR)L"FAILED fprintf_s(...",
									(LPCWSTR)L"ERROR",
									MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
									); 
							  }
							  fclose(stream);
							  
							  // fprintf(stream, "%S\n", msg);
					/*************************************************************/			    

}

void freeResources(HWND hwnd)
{
	/****************** EXPERIMENT *****************/						
		if(isPlayingAudio)
		{
			KillTimer(hwnd, IDT_TIMER1);
		}
	/**********************************************/
	

	if(pAudioClient != NULL &&  pControl == NULL)
				{				
					pAudioClient->Stop();
					pAudioClient->Release();
					pAudioClient = NULL;
					pEnumerator->Release();
					pEnumerator = NULL;
					pDevice->Release();
					pDevice = NULL;
					isPlayingAudio = false;
					pBuffer->Release();
					pBuffer = NULL;
					pReader->Release();
					pReader = NULL;
					pUncompressedAudioType->Release();
					pUncompressedAudioType = NULL;
					pPartialType->Release();
					pPartialType = NULL; 
					pAudioData = NULL;
					cbAudioData = 0;  
					cbMaxAudioData = 0;
					cbBuffer = NULL;
					cbWritten = 0;
					pProps->Release(); 					  
					pProps = NULL;
					mmioClose(hFile2, NULL);
					pRenderClient->Release();
					pSample = NULL;

					

					if(hFile2 != NULL)
					{
						mmioClose(hFile2, NULL);
					}

					CloseHandle(folder); 


					HeapFree(GetProcessHeap(), 0, pDataStr);
					pDataStr = NULL;



				}

			  /**********************************************************/


			/*************** EXPERIMENT WORKS *******************/
				if(pControl != NULL && pAudioClient != NULL)
				{
					pControl->Stop();
					pAudioClient->Stop();
					pControl->Release();
					pAudioClient->Release();
					pControl = NULL;
					pAudioClient = NULL;
					pGraph->Release();
					pGraph = NULL;
					pEvent->Release();
					pEvent = NULL;
					pEnumerator->Release();
					pEnumerator = NULL;
					pDevice->Release();
					pDevice = NULL;	
					isPlayingVideo = false;
					isPlayingAudio = false;
					pWc->Release();
					pWc = NULL;
					pVmr->Release();
					pVmr = NULL;
					pConfig->Release();																																							
					pConfig = NULL;
					pBuffer->Release();
					pBuffer = NULL;
					pReader->Release();
					pReader = NULL;
					pUncompressedAudioType->Release();
					pUncompressedAudioType = NULL;
					pPartialType->Release();
					pPartialType = NULL; 
					pAudioData = NULL;
					cbAudioData = 0;  
					cbMaxAudioData = 0;
					cbBuffer = NULL;
					cbWritten = 0;
					pProps->Release(); 					  
					pProps = NULL;
					mmioClose(hFile2, NULL);

					// TerminateThread(thread, 0);
					// thread = NULL;


						  pRenderClient->Release();
						 // pRenderClient = NULL;

						// pMS->Release();
						  pMS = NULL;

						
						// pSample->Release();
						pSample = NULL;


					 

					 CloseHandle(folder); 

					  

					  InvalidateRect(hwnd, NULL, TRUE);
					  ShowWindow(hWndListView, SW_SHOW);
					 				
 
				}
				
				
				
}
 





class CMMNotificationClient : public IMMNotificationClient
{
	LONG _cRef;

	public:
		CMMNotificationClient() :
		_cRef(1)
		{

		}

	~CMMNotificationClient(void)		
	{

	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ulRef = InterlockedDecrement(&_cRef);
		if (0 == ulRef)
		{
			delete this;
		}

		return ulRef;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
	{
		if (IID_IUnknown == riid)
		{
			AddRef();
			*ppvInterface = (IUnknown*)this;
		}
		else if (__uuidof(IMMNotificationClient) == riid)
		{
			AddRef();
			*ppvInterface = (IMMNotificationClient*)this;
		}
		else
		{
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
	{
		int msgboxID = MessageBox(
						NULL,
						pwstrDeviceId,
						(LPCWSTR)L"OnDefaultDeviceChanged",
						MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
						); 

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
	{
		int msgboxID = MessageBox(
						NULL,
						pwstrDeviceId,
						(LPCWSTR)L"OnDeviceAdded",
						MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
						);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
	{
		int msgboxID = MessageBox(
						NULL,
						pwstrDeviceId,
						(LPCWSTR)L"OnDeviceRemoved",
						MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
						);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
	{
		int msgboxID = MessageBox(
						NULL,
						pwstrDeviceId,
						(LPCWSTR)L"OnDeviceStateChanged",
						MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
						);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
	{
		int msgboxID = MessageBox(
						NULL,
						pwstrDeviceId,
						(LPCWSTR)L"OnPropertyValueChanged",
						MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
						);
		return S_OK;
	}
};



DWORD WINAPI playVideo(LPVOID hwnd)
// DWORD playVideo(HWND hWnd)
{
	isPlayingVideo = true;

	/*************** EXPERIMENT **********************/
		
		folder = CreateFile(
			 TEXT("lockIt"), // open One.txt
			GENERIC_READ , // open for reading			 
			  0, // do not share
			NULL, // no security
			OPEN_EXISTING, // existing file only
			  FILE_FLAG_BACKUP_SEMANTICS,  
			NULL);

		if (folder == INVALID_HANDLE_VALUE)
		{
			showError(L"FAILED CreateFile\n");
		}
	/************************************************/


	 
	  
				HWND hWnd = (HWND)hwnd;
		

				

				hr = CoCreateInstance(CLSID_FilterGraph, NULL,
					CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);

				if(FAILED(hr))
				{
				 	// printf("FAILED CoCreateInstance\n");
				 	showError(L"FAILED CoCreateInstance\n");
				}

				hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
				if(FAILED(hr))
				{
				 	// printf("FAILED QueryInterface IID_IMediaControl\n");
				 	showError(L"FAILED QueryInterface IID_IMediaControl\n");
				}

				hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
				if(FAILED(hr))
				{
				 	// printf("FAILED QueryInterface IID_IMediaEvent\n");
				 	showError(L"FAILED QueryInterface IID_IMediaEvent\n");
				}

				// WINDOW-LESS VIDEO RENDER PART
				hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
					CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
				if(FAILED(hr))
				{
				 	// printf("FAILED CoCreateInstance(CLSID_VideoMixingRenderer\n");
				 	showError(L"FAILED CoCreateInstance(CLSID_VideoMixingRenderer\n");
				}

				/******************** EXPERIMENT NOT PRODUCING WANTED RESULTS***************************/
						/* IEnumFilters *pEnum = NULL;
						IBaseFilter *pFilter;
						ULONG cFetched;
						HRESULT hr = pGraph->EnumFilters(&pEnum);

						while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
						{
							FILTER_INFO FilterInfo;
							hr = pFilter->QueryFilterInfo(&FilterInfo);

							MessageBox(NULL, FilterInfo.achName, TEXT("Filter Name"), MB_OK);

							 
								hr = pGraph->RemoveFilter(pFilter);
								if(FAILED(hr))
								{
								 	 showError(L"FAILED pGraph->RemoveFilter\n");
								} 
							 
							 	break;


							pFilter->Release();
						} */


						 

				/**********************************************************/

				hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer");
				if(FAILED(hr))
				{
				 	// printf("FAILED pGraph->AddFilter\n");
				 	showError(L"FAILED pGraph->AddFilter\n");
				}



				 




				hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
				if (SUCCEEDED(hr))
				{
					hr = pConfig->SetRenderingMode(VMRMode_Windowless);
					pConfig->Release();
					
				}else{
					 // printf("FAILED pVmr->QueryInterface(IID_IVMRFilterConfig\n");
					 showError(L"FAILED pVmr->QueryInterface(IID_IVMRFilterConfig\n");
				}

				if (SUCCEEDED(hr))
				{
					hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWc);
					if( SUCCEEDED(hr))
					{
						hr = pWc->SetVideoClippingWindow(hWnd);
						if (SUCCEEDED(hr))
						{
							
						}
						else
						{
							// An error occurred, so release the interface.
							// printf("FAILED pVmr->QueryInterface(IID_IVMRWindowlessControl\n");
							showError(L"FAILED pVmr->QueryInterface(IID_IVMRWindowlessControl\n");
							pWc->Release();
						}
					}
				}

				hr = pWc->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
				if(FAILED(hr))
				{
				 	// printf("FAILED pWc->GetNativeVideoSize\n");
				 	showError(L"FAILED pWc->GetNativeVideoSize\n");
				}

			
			 	RECT rcSrc, rcDest;
				SetRect(&rcSrc, 0, 0, lWidth, lHeight);
				GetClientRect(hWnd, &rcDest);
				SetRect(&rcDest, 0, 0, rcDest.right, rcDest.bottom);
				hr = pWc->SetVideoPosition(&rcSrc, &rcDest);
				if(FAILED(hr))
				{
				 	// printf("FAILED pWc->SetVideoPosition\n");
				 	showError(L"FAILED pWc->SetVideoPosition\n");
				} 



				    //  hr = pGraph->RenderFile(L"vid.wmv", NULL);  
 					   hr = pGraph->RenderFile(L"sinkwriter.wmv", NULL);  
				 // hr = pGraph->RenderFile(L"lockIt/siaNoSound.avi", NULL); // THIS CODE WORKS
				// hr = pGraph->RenderFile(L"C:\\Users\\hp\\Desktop\\ProgrammingPrac\\audioAndVideoFilesForProgrammingPrac\\siaNoSound.avi", NULL); // THIS CODE WORKS
				if (FAILED(hr))
				{
					// printf("FAILED pGraph->RenderFile\n");
					showError(L"FAILED pGraph->RenderFile\n");
				}



				/******************************** THIS CODE WORKS ************************************/
					
					hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
					if (FAILED(hr))
					{
						// printf("FAILED pGraph->RenderFile\n");
						showError(L"FAILED pGraph->QueryInterface\n");
					}

					DWORD			m_seekCaps;		// Caps bits for IMediaSeeking
					hr = pMS->GetCapabilities(&m_seekCaps);
					if (FAILED(hr))
					{
						// printf("FAILED pGraph->RenderFile\n");
						showError(L"FAILED pMS->GetCapabilities\n");
					}

					/********************** EXPERIMENT ****************************/
						if (AM_SEEKING_CanSeekAbsolute & m_seekCaps)
						{
							showError(L"AM_SEEKING_CanSeekAbsolute\n");
						}else{
							showError(L"!!!! CANNOT AM_SEEKING_CanSeekAbsolute\n");
						}

					/****************************************************************/

					LONGLONG rtDuration = 0;
					hr = pMS->GetDuration(&rtDuration);
					if (FAILED(hr))
					{
						// printf("FAILED pGraph->RenderFile\n");
						showError(L"FAILED pMS->GetDuration\n");
					}

				  LONGLONG durationInSeconds = rtDuration / 10000000;
				 

				    FILE *stream2;
					if((stream2 = fopen( "durationInSeconds.txt", "w" )) == NULL)
					{
						printf("FAILED fopen");
					}

					fprintf(stream2, "durationInSeconds: %lld\n", durationInSeconds);
					fclose(stream2); 		

				/********************************************************************************/

			 	hr = pControl->Run();
				if (FAILED(hr))
				{
					// printf("FAILED pControl->Run\n");
					showError(L"FAILED pControl->Run\n");
				}

				
				pEvent->WaitForCompletion(INFINITE, &evCode);   // // IF pEvent->WaitForCompletion(INFINITE, &evCode) is not used video display fails
			 	 

	return 0;
						
}

 
// DWORD WINAPI startPlayback(LPVOID hwnd)
DWORD WINAPI startPlayback(LPVOID dataStruct)
{
	PMYDATA pDataStruct = (PMYDATA)dataStruct;
	HWND hwnd = pDataStruct->windowHandle;


	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	 const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


	

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL, 
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);

	if(FAILED(hr))
	{
		// printf("Failed CoCreateInstance\n");
		showError(L"Failed CoCreateInstance\n");
	}

	hr = pEnumerator->GetDefaultAudioEndpoint(
		eRender, eConsole, &pDevice);

	if(FAILED(hr))
	{
		// printf("FAILED GetDefaultAudioEndpoint\n");		
		showError(L"FAILED GetDefaultAudioEndpoint\n");		
	}


	/************************************* EXPERIMENT *****************************************/
		
	hr = pEnumerator->RegisterEndpointNotificationCallback(new CMMNotificationClient());
	if(FAILED(hr))
	{
		// printf("FAILED GetDefaultAudioEndpoint\n");		
		showError(L"FAILED pEnumerator->RegisterEndpointNotificationCallback\n");		
	}

	/*****************************************************************************************/

	hr = pDevice->Activate(
		IID_IAudioClient, CLSCTX_ALL, NULL,
		(void**)&pAudioClient);

	if(FAILED(hr))
	{
		// printf("FAILED Activate\n");
		showError(L"FAILED Activate\n");
	}

	
	/********************* EXPERIMENT ***************************/		
		pWfx = NULL;
		pWfxx = NULL;
		hr = pAudioClient->GetMixFormat(&pWfxx);
		if(FAILED(hr))
		{
			showError(L"FAILED pAudioClient->GetMixFormat\n");
		}

		 

		PROPVARIANT audioEngineDeviceFormat;	
		pProps = NULL;
		hr = pDevice->OpenPropertyStore(
					STGM_READ, &pProps);
		PropVariantInit(&audioEngineDeviceFormat);
		hr = pProps->GetValue(
					PKEY_AudioEngine_DeviceFormat, &audioEngineDeviceFormat);
		if(FAILED(hr))
		{
			showError(L"FAILED pProps->GetValue\n");
		}
		pWfx = (WAVEFORMATEX *) audioEngineDeviceFormat.blob.pBlobData;
	/******************************************************************/

	/************* EXPERIMENT **************/
		 	folder = CreateFile(
								 L"lockIt", // open One.txt
								GENERIC_READ | GENERIC_WRITE, // open for reading			 
								  0, // do not share
								NULL, // no security
								OPEN_EXISTING, // existing file only
								  FILE_FLAG_BACKUP_SEMANTICS,  
								NULL);

			if (folder == INVALID_HANDLE_VALUE)
			{
				showError(L"FAILED CreateFile folder\n");
			}  

	 /****************************************/
	 
	 
	
	HANDLE hFile;
	BOOL bResult;
	
	 hFile = CreateFile(
			L"lockIt/exclusiveFile.wav",
			GENERIC_WRITE, // open for writing
			0,  
			NULL, // no security
			CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL
			);

	if(NULL == hFile)
	{
		
		// printf("FAILED CreateFile\n");
		showError(L"FAILED CreateFile\n");
	} 

	
	 
 	// hr = MFCreateSourceReaderFromURL(L"1215epi.mp4", NULL, &pReader); // THIS PART WORKS. IT EXTRACTS AN AUDIO STREAM FROM A VIDEO FILE AND CONVERTS IT TO WAV FOR RENDERING USING CORE AUDIO
	  hr = MFCreateSourceReaderFromURL(decryptFile(pDataStruct->urlOfSong), NULL, &pReader); // THIS PART WORKS
	// hr = MFCreateSourceReaderFromURL(pDataStruct->urlOfSong, NULL, &pReader); // THIS PART WORKS
	if(FAILED(hr))
	{
		// printf("FAILED MFCreateSourceReaderFromURL\n");
		showError(L"FAILED MFCreateSourceReaderFromURL\n");
		 		
	}

	  cbAudioData = 0; // Total bytes of PCM audio data written to the file.
	  cbMaxAudioData = 0;
	  cbBuffer = 0;


	
	hr = pReader->SetStreamSelection(
	(DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
	if(FAILED(hr))
	{
		// printf("FAILED pReader->SetStreamSelection\n");
		showError(L"FAILED pReader->SetStreamSelection\n");
	}
	hr = pReader->SetStreamSelection(
		(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
	if(FAILED(hr))
	{
		// printf("FAILED pReader->SetStreamSelection 2\n");
		showError(L"FAILED pReader->SetStreamSelection 2\n");
	}

	// Create a partial media type that specifies uncompressed PCM audio.
	hr = MFCreateMediaType(&pPartialType);
	if(FAILED(hr))
	{
		// printf("FAILED MFCreateMediaType\n");
		showError(L"FAILED MFCreateMediaType\n");
	}
	hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if(FAILED(hr))
	{
		// printf("FAILED pPartialType->SetGUID\n");
		showError(L"FAILED pPartialType->SetGUID\n");
	}
	
	
	hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	if(FAILED(hr))
	{
		// printf("FAILED pPartialType->SetGUID 2\n");
		showError(L"FAILED pPartialType->SetGUID 2\n");
	} 


	/***************** EXPERIMENT WORKS************************/
						 
						/* if(pWfx->wFormatTag == WAVE_FORMAT_PCM)
						 {
						 	showError(L"pWfx->wFormatTag == MFAudioFormat_PCM");
						 }

						 if(pWfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
						 {
						 	showError(L"pWfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT");
						 } 

						stream = fopen( "errorLog.txt", "a" );
						fprintf_s(stream, "%X \n",  pWfx->wFormatTag);
						fclose(stream); */
						 


						hr = pPartialType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, pWfx->nAvgBytesPerSec);
						if(FAILED(hr))
						{
							// printf("FAILED pPartialType->SetGUID 2\n");
							showError(L"FAILED pPartialType->SetGUID(MF_MT_AUDIO_AVG_BYTES_PER_SECOND\n");
						}

						hr = pPartialType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, pWfx->wBitsPerSample);
						if(FAILED(hr))
						{
							// printf("FAILED pPartialType->SetGUID 2\n");
							showError(L"FAILED pPartialType->SetGUID(MF_MT_AUDIO_BITS_PER_SAMPLE\n");
						}

						hr = pPartialType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, pWfx->nBlockAlign);
						if(FAILED(hr))
						{
							// printf("FAILED pPartialType->SetGUID 2\n");
							showError(L"FAILED pPartialType->SetGUID(MF_MT_AUDIO_BLOCK_ALIGNMENT\n");
						}

						hr = pPartialType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pWfx->nChannels);
						if(FAILED(hr))
						{
							// printf("FAILED pPartialType->SetGUID 2\n");
							showError(L"FAILED pPartialType->SetGUID(MF_MT_AUDIO_NUM_CHANNELS\n");
						}

						hr = pPartialType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pWfx->nSamplesPerSec);
						if(FAILED(hr))
						{
							// printf("FAILED pPartialType->SetGUID 2\n");
							showError(L"FAILED pPartialType->SetGUID(MF_MT_AUDIO_SAMPLES_PER_SECOND\n");
						} 
	/*****************************************************/




	hr = pReader->SetCurrentMediaType(
		(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		NULL, pPartialType);
	if(FAILED(hr))
	{
		// printf("FAILED pReader->SetCurrentMediaType\n");
		showError(L"FAILED pReader->SetCurrentMediaType\n");
	}

	hr = pReader->GetCurrentMediaType(
	(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
	&pUncompressedAudioType);
	if(FAILED(hr))
	{
		// printf("FAILED pReader->GetCurrentMediaType\n");
		showError(L"FAILED pReader->GetCurrentMediaType\n");
	}
	hr = pReader->SetStreamSelection(
		(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		TRUE);
	if(FAILED(hr))
	{
		// printf("FAILED pReader->SetStreamSelection 3\n");
		showError(L"FAILED pReader->SetStreamSelection 3\n");
	}

	// WriteWaveData
	 cbAudioData = 0;
	
	
	

	
	while (true)
	{
		
		DWORD dwFlags = 0;
		hr = pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0, NULL, &dwFlags, NULL, &pSample );
		if(FAILED(hr))
		{
			// printf("FAILED pReader->ReadSample\n");
			showError(L"FAILED pReader->ReadSample\n");
		}
		if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
		{
			// printf("Type change - not supported by WAVE file format.\n");
			showError(L"Type change - not supported by WAVE file format.\n");
			break;
			
		}
		if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			 printf("End of input file.\n");
			// showError(L"End of input file.\n");
			break;
			
		}

		if (pSample == NULL)
		{
			// printf("No sample\n");
			showError(L"No sample\n");
			continue;
		}
		hr = pSample->ConvertToContiguousBuffer(&pBuffer);
		if(FAILED(hr))
		{
			// printf("FAILED pSample->ConvertToContiguousBuffer\n");
			showError(L"FAILED pSample->ConvertToContiguousBuffer\n");
		}
		// hr = pBuffer->Lock(&pData, NULL, &cbBuffer);
		 hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);
		// printf("cbBuffer: %d\n", cbBuffer);
		if(FAILED(hr))
		{
			// printf("FAILED pBuffer->Lock\n");
			showError(L"FAILED pBuffer->Lock\n");
		}

		

		
		 bResult = WriteFile(hFile, pAudioData, cbBuffer, &cbWritten, NULL);
		if (!bResult)
		{
			// printf("FAILED WriteFile 4\n");
			showError(L"FAILED WriteFile 4\n");
		}
		

		
		hr = pBuffer->Unlock();
		pAudioData = NULL;
		cbAudioData += cbBuffer;
		pSample->Release();
		pBuffer->Release();

	}  

	printf("Wrote %d bytes of audio data.\n", cbAudioData);
	 

	 CloseHandle(hFile); 

	 

 	printf("HERE HERE\n");

 	

 	  hFile2 = mmioOpen(
		// L"C:\\Users\\hp\\Desktop\\ProgrammingPrac\\exeFile.exe", // THIS LINE WORKS
		  L"lockIt/exclusiveFile.wav",
		NULL,
		MMIO_EXCLUSIVE); 

	 if(NULL == hFile2)
	{
		// printf("Failed to open file\n");
		showError(L"Failed to open file\n");
	}

		

    

	  hr = pAudioClient->IsFormatSupported(
        AUDCLNT_SHAREMODE_EXCLUSIVE,
        pWfx,   
        NULL // can't suggest a "closest match" in exclusive mode
    );
    if (AUDCLNT_E_UNSUPPORTED_FORMAT == hr) {
       // printf("Audio device does not support the requested format.\n");
       showError(L"Audio device does not support the requested format.\n");
        pAudioClient->Release();
        return hr; 
       

    } else if (FAILED(hr)) {
        // printf("IAudioClient::IsFormatSupported failed: hr = 0x%08x.\n", hr);
        showError(L"IAudioClient::IsFormatSupported failed: \n");
        pAudioClient->Release();
        return hr;
    } 

    // get the periodicity of the device
    REFERENCE_TIME hnsPeriod;
    hr = pAudioClient->GetDevicePeriod(
        NULL, // don't care about the engine period
        &hnsPeriod // only the device period
    );
    if (FAILED(hr)) {
       // printf("IAudioClient::GetDevicePeriod failed: hr = 0x%08x.\n", hr);
        showError(L"IAudioClient::GetDevicePeriod failed:");
        pAudioClient->Release();
        return hr;
    }

    // need to know how many frames that is
      nFramesInBuffer = (UINT32)( // frames =
        1.0 * hnsPeriod * // hns *
        pWfx->nSamplesPerSec / // (frames / s) /
        1000 / // (ms / s) /
        10000 // (hns / s) /
        + 0.5 // rounding
    );

     printf("The default period for this device is %I64u hundred-nanoseconds, or %u frames.\n", hnsPeriod, nFramesInBuffer);

     hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_EXCLUSIVE,
		 AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		hnsPeriod, hnsPeriod, pWfx, NULL);

    if(FAILED(hr))
	{
		// printf("pAudioClient->Initialize 1\n");	
		showError(L"pAudioClient->Initialize 1\n");	
	}

	#define AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED      AUDCLNT_ERR(0x019)
	if (AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED == hr) {

        // if the buffer size was not aligned, need to do the alignment dance
       // printf("Buffer size not aligned - doing the alignment dance.\n");
        showError(L"Buffer size not aligned - doing the alignment dance.\n");
        
        // get the buffer size, which will be aligned
        hr = pAudioClient->GetBufferSize(&nFramesInBuffer);
        if (FAILED(hr)) {
           // printf("IAudioClient::GetBufferSize failed: hr = 0x%08x\n", hr);
            showError(L"IAudioClient::GetBufferSize failed:\n");
            return hr;
        }

     	// throw away this IAudioClient
        pAudioClient->Release();

        // calculate the new aligned periodicity
        hnsPeriod = // hns =
            (REFERENCE_TIME)(
                10000.0 * // (hns / ms) *
                1000 * // (ms / s) *
                nFramesInBuffer / // frames /
                pWfx->nSamplesPerSec  // (frames / s)
                + 0.5 // rounding
            );

        // activate a new IAudioClient
        hr = pDevice->Activate(
		IID_IAudioClient, CLSCTX_ALL, NULL,
		(void**)&pAudioClient);
        if (FAILED(hr)) {
           // printf("IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x\n", hr);
            showError(L"IMMDevice::Activate(IAudioClient) failed: ");
            return hr;
        }

        // try initialize again
        printf("Trying again with periodicity of %I64u hundred-nanoseconds, or %u frames.\n", hnsPeriod, nFramesInBuffer);
        hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_EXCLUSIVE,
		 AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		hnsPeriod, hnsPeriod, pWfx, NULL);

        if (FAILED(hr)) {
            // printf("IAudioClient::Initialize failed, even with an aligned buffer: hr = 0x%08x\n", hr);
            showError(L"IAudioClient::Initialize failed, even with an aligned buffer:\n");
            pAudioClient->Release();
            return hr;
        }
    } else if (FAILED(hr)) {
       // printf("IAudioClient::Initialize failed: hr = 0x%08x\n", hr);
        showError(L"IAudioClient::Initialize 2 failed :\n");
        pAudioClient->Release();
        return hr;
    }

     hr = pAudioClient->GetBufferSize(&nFramesInBuffer);
    if (FAILED(hr)) {
       // printf("IAudioClient::GetBufferSize failed: hr = 0x%08x\n", hr);
        showError(L"IAudioClient::GetBufferSize failed:\n");
        pAudioClient->Release();
        return hr;
    }

    // calculate the new period
    hnsPeriod = // hns =
        (REFERENCE_TIME)(
            10000.0 * // (hns / ms) *
            1000 * // (ms / s) *
            nFramesInBuffer / // frames /
            pWfx->nSamplesPerSec  // (frames / s)
            + 0.5 // rounding
        );
    
    printf("We ended up with a period of %I64u hns or %u frames.\n", hnsPeriod, nFramesInBuffer);

    
    
       hNeedDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hNeedDataEvent)
    {
    	// printf("Failed CreateEvent\n");
    	showError(L"Failed CreateEvent\n");
    }

    hr = pAudioClient->SetEventHandle(hNeedDataEvent);
    if (FAILED(hr))
    {
    	// printf("Failed SetEventHandle\n");
    	showError(L"Failed SetEventHandle\n");
    } 

    hr = pAudioClient->GetService(IID_IAudioRenderClient,
		(void**)&pRenderClient);

	if(FAILED(hr))
	{
		// printf("Failed GetService\n");
		showError(L"Failed GetService\n");
	}

	 
	hr = pRenderClient->GetBuffer(nFramesInBuffer, &pData);

	if(FAILED(hr))
	{
		// printf("Failed GetBuffer\n");
		showError(L"Failed GetBuffer\n");
	}

	hr = pRenderClient->ReleaseBuffer(nFramesInBuffer, AUDCLNT_BUFFERFLAGS_SILENT);
    if (FAILED(hr)) {
        // printf("IAudioRenderClient::ReleaseBuffer failed trying to pre-roll silence: hr = 0x%08x\n", hr);
        showError(L"IAudioRenderClient::ReleaseBuffer failed trying to pre-roll silence: ");
        pRenderClient->Release();        
      
        pAudioClient->Release();
        return hr;
    }

     DWORD taskIndex = 0;
	
	hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
	if (hTask == NULL)
	{
		// printf("FAILED AvSetMmThreadCharacteristics\n");
		showError(L"FAILED AvSetMmThreadCharacteristics\n");
	}

    hr = pAudioClient->Start();

	if(FAILED(hr))
	{
			// printf("Failed Start \n");	
			showError(L"Failed Start \n");	
	}

	UINT32 nBytesPerFrame = pWfx->nBlockAlign;
	UINT nFrames = 0;
	nFrames = cbAudioData / nBytesPerFrame; // cbAudioData is the total file size in bytes i.e total bytes written to file when converting from mp3
	

	// ORIGINAL FOR-LOOP for ( UINT32 nFramesPlayed = 0, nFramesThisPass = nFramesInBuffer; nFramesPlayed < nFrames; nFramesPlayed += nFramesThisPass) 

	 LONG nBytesGotten = -1;

	 
	 		

	 		/*****************EXPERIMENT********************************/
	 				 	if(NULL != wcsstr(pDataStruct->urlOfSong, L".mp4"))        // THERE IS VIDEO
						{
							DWORD threadIdentifier; 
						     thread2 = CreateThread(
							NULL,
							0,
							playVideo,
							hwnd,
							0,
							&threadIdentifier);

							if(thread2 == NULL)
							{
								showError(L"FAILED CreateThread\n");
							}
						}   
	 		 /*********************************************************/
	  
	 	/************************ EXPERIMENT WORKS************************************ 
	 		 DWORD threadIdentifier; 
			     thread2 = CreateThread(
				NULL,
				0,
				playVideo,
				hwnd,
				0,
				&threadIdentifier);

				if(thread2 == NULL)
				{
					showError(L"FAILED CreateThread\n");
				}
	************************************************************************/

				isPlayingAudio = true;


				/************** EXPERIMENT ***************************/
					SetTimer(hwnd, // handle to main window
								IDT_TIMER1, // timer identifier
								60000, // 60-second interval
								(TIMERPROC) NULL); // no timer callback
				/*****************************************************/



	 

	for (
        UINT32 nFramesPlayed = 0,
            nFramesThisPass = nFramesInBuffer;
        nBytesGotten != 0;
        nFramesPlayed += nFramesThisPass
    ){
		 
		 WaitForSingleObject(hNeedDataEvent, INFINITE);
 
    	hr = pRenderClient->GetBuffer(nFramesInBuffer, &pData);
    	if(FAILED(hr))
		{
			// printf("Failed GetBuffer 2\n");
			showError(L"Failed GetBuffer 2\n");
		} 

		if (nFramesPlayed + nFramesInBuffer > nFrames) {
            // nope - this is the last buffer
            nFramesThisPass = nFrames - nFramesPlayed;
        }
         UINT32 nBytesThisPass = nFramesThisPass * pWfx->nBlockAlign;
         // printf("nBlockAlign %d\n", pWfx->nBlockAlign);
         // showError(L"RENDERING AUDIO !!!");


        
          	if(bytePositionToSeek != 0)
          	{
          		if(-1 == mmioSeek(hFile2, bytePositionToSeek, SEEK_SET))
          		{
          			// printf("Failed mmioSeek EXPERIMENT\n");
          			showError(L"Failed mmioSeek EXPERIMENT\n");
          		}else{
          			// printf("SEEKING SUCCEEDED\n");
          			showError(L"SEEKING SUCCEEDED\n");
          		}

          		bytePositionToSeek = 0;
          	}

         nBytesGotten = mmioRead(hFile2, (HPSTR)pData, nBytesThisPass);

       

        if (nFramesThisPass < nFramesInBuffer) {
            UINT32 nBytesToZero = (nFramesInBuffer * pWfx->nBlockAlign) - nBytesThisPass;
            ZeroMemory(pData + nBytesGotten, nBytesToZero);
        }
      
        hr = pRenderClient->ReleaseBuffer(nFramesInBuffer, 0); // no flags 

    }

    
   /***********************************************/
	   freeResources(hwnd);
	/*********************************************/

	

	bool avRevert = AvRevertMmThreadCharacteristics(hTask);
	if(avRevert == FALSE)
	{
		// printf("FAILED AvRevertMmThreadCharacteristics\n");
		showError(L"FAILED AvRevertMmThreadCharacteristics\n");
	}

	   

	return 0;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	  

	switch (uMsg)
	{
		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_TIMER1:
				{
					MessageBox(NULL, L"1 minute gone", L"WM_TIMER", MB_OK);
					return 0;
				}
			}
		}

		case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
						case ID_BUTTON_pause_resume:
						{
							pauseResume(hwnd);
							// pauseResume();
							break;
						}

						case ID_BUTTON_stop:
						{
							freeResources(hwnd);


							/********************* EXPERIMENT WORKS**********************/
							 
									/*     DWORD threadIdentifier; 
								     thread2 = CreateThread(
									NULL,
									0,
									extractVideoOnly,
									  hwnd,
									0,
									&threadIdentifier);

									if(thread2 == NULL)
									{
										showError(L"FAILED CreateThread\n");
									}     */  

										MediaPlayerCallback *g_pPlayerCB = NULL;
										g_pPlayerCB = new (std::nothrow) MediaPlayerCallback();  
										IMFPMediaItem *pItem = NULL;										

								    	hr = MFPCreateMediaPlayer(
										NULL, // URL.
										// L"1215epi.mp4", // URL.
										// L"C:\\Users\\hp\\Desktop\\ProgrammingPrac\\video.mp4", // URL.
										FALSE, 
										// TRUE, 
										0, // Option flags.
										g_pPlayerCB, // Callback interface.
										// NULL, // Callback interface.
										hwnd,
										&g_pPlayer
										// NULL
										);   

									    hr = g_pPlayer->CreateMediaItemFromURL(L"vid.wmv", TRUE, 0, &pItem);

									    /**************** EXPERIMENT ************************/
									   			 pItem->SetStreamSelection(1, false);
									   	/****************************************************/

										g_pPlayer->SetMediaItem(pItem);



										

										

				           /******************************************************/

							break;
						}

						case ID_BUTTON_show_list_of_songs:
						{
							showListOfSongs(hwnd);
							break;
						}

						case ID_BUTTON_user_manual:
			 			{
			 				LoadLibrary(TEXT("Msftedit.dll"));
			 				 
			 				HWND hwndEdit= CreateWindowEx(0, MSFTEDIT_CLASS, TEXT("Type here"),
																  ES_MULTILINE | ES_READONLY | WS_VISIBLE |  WS_TABSTOP | WS_SYSMENU,
																0, 0,  												 
																  600,
																  600,
																NULL,
																// hwnd,
																NULL, // Menu
																 (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
																NULL // Additional application data
																);

			 				 COLORREF rgbGreen = 0x0000ffad;
			 				 SendMessage(hwndEdit, EM_SETBKGNDCOLOR, (WPARAM) 0,  (LPARAM) rgbGreen);
			 				 SendMessage(hwndEdit, WM_SETICON, (WPARAM) 1, (LPARAM) ExtractIcon(NULL, L"myLogo.ico", 0));

			 				 CHARFORMAT charFormat;
			 				 charFormat.cbSize = sizeof(CHARFORMAT);
			 				 charFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
			 				 charFormat.yHeight = 300;
			 				 COLORREF rgbWhite = 0x00000000;
			 				 charFormat.crTextColor = rgbWhite;
			 				 // StringCchCopy(charFormat.szFaceName, LF_FACESIZE, L"Calibri");
			 				 StringCchCopy(charFormat.szFaceName, LF_FACESIZE, L"Gabriola");

			 				 SendMessage(hwndEdit, EM_SETCHARFORMAT, (WPARAM) SCF_ALL, (LPARAM) &charFormat);

			 				 TCHAR lpszLatin[] = L"You suck";
			 				 SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) lpszLatin);
			 				  

			 				break;
			 			}

						case ID_BUTTON_about_us:
			 			{
			 				LoadLibrary(TEXT("Msftedit.dll"));
			 				HWND hwndEdit= CreateWindowEx(0, MSFTEDIT_CLASS, TEXT("About us"),
																  ES_MULTILINE | ES_READONLY | WS_VISIBLE |  WS_TABSTOP | WS_SYSMENU,
																0, 0,  												 
																  600,
																  600,
																NULL,
																// hwnd,
																NULL, // Menu
																 (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
																NULL // Additional application data
																);
			 				 COLORREF rgbGreen = 0x0000ffad;
			 				 SendMessage(hwndEdit, EM_SETBKGNDCOLOR, (WPARAM) 0,  (LPARAM) rgbGreen);
			 				 SendMessage(hwndEdit, WM_SETICON, (WPARAM) 1, (LPARAM) ExtractIcon(NULL, L"myLogo.ico", 0));

			 				 CHARFORMAT charFormat;
			 				 charFormat.cbSize = sizeof(CHARFORMAT);
			 				 charFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
			 				 charFormat.yHeight = 300;
			 				 COLORREF rgbWhite = 0x00000000;
			 				 charFormat.crTextColor = rgbWhite;
			 				 // StringCchCopy(charFormat.szFaceName, LF_FACESIZE, L"Calibri");
			 				 StringCchCopy(charFormat.szFaceName, LF_FACESIZE, L"Gabriola");

			 				 SendMessage(hwndEdit, EM_SETCHARFORMAT, (WPARAM) SCF_ALL, (LPARAM) &charFormat);

			 				 TCHAR lpszLatin[] = L"Website:\t www.altogradesoftwares.tk \n"
			 				 					 L"Telegram:\t +263 775 267 926 \n"
			 				 					 L"Twitter:\t\t twitter.com/AltoGrade \n"
			 				 					 L"Facebook:\t facebook.com/AltoGradeSoftwares \n"
			 				 					 L"WhatsApp:\t +263 775 267 926";
			 				 SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) lpszLatin);

			 				break;
			 			}

						default:
			 				 break;
					}
				}
			}


			return 0;
		}
		 
		case WM_NOTIFY:
		{
			  LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;


			switch(lpnmitem->hdr.code)
			{
				case NM_CLICK:
				{
					/*************** EXPERIMENT WORKS***********************/
									freeResources(hwnd);
					/*************************************************/

					wchar_t szText[260];
					LVITEM lvI;
					lvI.iSubItem = 0;
					lvI.pszText = szText;
					lvI.cchTextMax = sizeof(szText);
					lvI.mask = LVIF_TEXT | lParam;

					 
					ListView_GetItemText(lpnmitem->hdr.hwndFrom, lpnmitem->iItem, 0, szText, sizeof(szText));
					 
					

					/**************** EXPERIMENT *******************/ 
						if(NULL != wcsstr(szText, L".encrypted"))
						// if(NULL != wcsstr(szText, L".mp3")) // THIS PART WORKS
						{

							MessageBox(NULL, szText, L"NM_CLICK", MB_OK);

							    pDataStr  = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
																								sizeof(MYDATA));

							if( pDataStr == NULL )
							{
								showError(L"(PMYDATA) HeapAlloc(... FAILED\n");
							}else{				 								
								  wmemcpy(pDataStr->urlOfSong, szText, sizeof szText / sizeof szText[0]);								
								  pDataStr->windowHandle	= hwnd;							
							}

							

								DWORD threadIdentifier;
								  HANDLE thread;
								  thread = CreateThread(
									NULL,
									0,
									startPlayback,
									pDataStr,
									0,
									&threadIdentifier);
						}

						/* if(NULL != wcsstr(szText, L".mp4"))
						{
								 

								ShowWindow(hWndListView, SW_HIDE);
						}  */

					 /**********************************************/


					/*************** EXPERIMENT WORKS*******************/
						/* DWORD threadIdentifier;
								  HANDLE thread;
								  thread = CreateThread(
									NULL,
									0,
									startPlayback,
									hwnd,
									0,
									&threadIdentifier); 

						ShowWindow(hWndListView, SW_HIDE);  */
					/*********************************************/

								  
								   

					return TRUE;
				}

				
			}  			 
				return 0;	 
		}	
 

		case WM_CREATE:
			{
							if(SetWindowDisplayAffinity(hwnd, WDA_MONITOR) != TRUE)
							{
								// printf("SetWindowDisplayAffinity FAILED\n");
								showError(L"SetWindowDisplayAffinity FAILED\n");
							}
							 
							
							 
							hr = CoInitialize(NULL);   // THIS WORKS
							if(hr != S_OK)
							{					
								// printf("FAILED CoInitializeEx\n");
								showError(L"FAILED CoInitializeEx\n");										
							}

							/******************** EXPERIMENT **********************/
							// Initialize the Media Foundation platform.
							hr = MFStartup(MF_VERSION);
							if(FAILED(hr))
							{
								// printf("FAILED MFStartup\n");
								showError(L"FAILED MFStartup\n");
							}														
							/*******************************************************/

							btn = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"Show list of songs", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								10, // x position
								40, // y position
								200, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								(HMENU) ID_BUTTON_show_list_of_songs, // No menu.
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL);

							btn6 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"Check your balance", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								10, // x position
								120, // y position
								250, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								NULL,
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL);

							btn7 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"Recharge your account", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								10, // x position
								200, // y position
								300, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								NULL,
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL);

							btn2 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"User Manual", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								10, // x position
								280, // y position
								250, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								(HMENU) ID_BUTTON_user_manual, // No menu.
								// NULL, // No menu.
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL); 

							btn3 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"About Us", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								10, // x position
								360, // y position
								200, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								(HMENU) ID_BUTTON_about_us, // No menu.
								// NULL, // No menu.
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL);

							btn4 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"Pause/Resume", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								500, // x position
								360, // y position
								200, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								(HMENU) ID_BUTTON_pause_resume, // No menu.
								// NULL, // No menu.
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL); 

							btn5 = CreateWindow(
								 L"BUTTON", // Predefined class; Unicode assumed
								 (LPCWSTR)   L"Stop", // Button text
								WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
								500, // x position
								420, // y position
								200, // Button width
								50, // Button height
								hwnd, // Parent window        
								// hwnd, // Parent window
								(HMENU) ID_BUTTON_stop, // No menu.
								// NULL, // No menu.
								(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
								NULL); 

							LOGFONT lf; 
						    ZeroMemory(&lf, sizeof(lf));
						    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Ink Free");
						      StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Gabriola");
						    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Calibri");
						    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"AR CARTER");
						    // StringCchCopy(lf.lfFaceName, LF_FACESIZE, L"Times New Roman");
						    lf.lfWeight = FW_NORMAL;
						    HFONT hFont = CreateFontIndirect(&lf);
						    SendMessage(btn, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn2, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn3, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn4, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn5, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn6, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
						    SendMessage(btn7, WM_SETFONT, (WPARAM)hFont,  MAKELPARAM(TRUE, 0));
				 
				/********** THIS CODE WORKS. DON'T DELETE. IT SHOWS HOW TO CREATE A DIALOG FOR SELECTING FILES ************************ 
					OPENFILENAME ofn; // common dialog box structure
					wchar_t szFile[260]; // buffer for file name
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hwnd;
					ofn.lpstrFile = szFile;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = L"MUSIC\0*.mp3\0All\0*.*\0Text\0*.TXT\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					GetOpenFileName(&ofn);
				 **********************************************/


				/********************************** THIS CODE WORKS  DON'T DELETE. IT SHOWS HOW TO CREATE A DIALOG FOR SELECTING FOLDERS ********************************** 
				HRESULT hrComInit = S_OK;

				hrComInit = CoInitializeEx(
					0,
					COINIT_APARTMENTTHREADED);

				if(FAILED(hrComInit))
				{
					printf("FAILED CoInitializeEx\n");
				}

				IFileDialog *pfd;
			    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
			    {
			    	DWORD dwOptions;
			        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			        {
			            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
			        }

			        if (SUCCEEDED(pfd->Show(NULL)))
			        {
			        	 IShellItem *psi;
	            		 if (SUCCEEDED(pfd->GetResult(&psi)))
	            		 {
	            		 	 PWSTR pszPath;
			                if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszPath)))
			                {
			                    MessageBox(NULL, pszPath, L"Selected Container", MB_OK);
			                    CoTaskMemFree(pszPath);
			                }
			                psi->Release();
	            		 }

	            		 pfd->Release();
			        }
		    }

		    *******************************************************************/

				/*************** EXPERIMENT *********************** 
					MessageBox(
							NULL,
							(LPCWSTR) argument,
							(LPCWSTR) L"Command line arguments",
							MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
							);	 
			*************************************************/

				


							
				
				/********************* THIS CODE WORKS AND IT'S FOR BLUETOOTH NOTIFICATIONS. TURN BLUETOOTH ON BEFORE USING THIS CODE
				findRadioParams.dwSize = sizeof(findRadioParams);
				if(NULL == BluetoothFindFirstRadio(&findRadioParams, &firstBluetoothRadio))
				{
					printf("BluetoothFindFirstRadio FAILED\n");
				} 

				DEV_BROADCAST_HANDLE NotificationFilter;
				NotificationFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
				NotificationFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
				NotificationFilter.dbch_handle = firstBluetoothRadio;
				
				HDEVNOTIFY hDeviceNotify = RegisterDeviceNotification(
							hwnd, // events recipient
							&NotificationFilter, // type of device
							DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
							);

				if (NULL == hDeviceNotify)
				{
					showError(L"FAILED RegisterDeviceNotification\n");
					return 0;
				} */

 				  

				  /********** EXPERIMENT **********************************************/
				  		 GUID gd = { 0xE6F07B5F, 0xEE97, 0x4a90,0xB0, 0x76, 0x33, 0xF5, 0x7B, 0xF4, 0xEA, 0xA7};

				  		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter2;
				  		NotificationFilter2.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
						NotificationFilter2.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
						NotificationFilter2.dbcc_classguid = gd;

						HDEVNOTIFY hDeviceNotify2 = RegisterDeviceNotification(
							hwnd, // events recipient
							&NotificationFilter2, // type of device
							DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
							);

						if (NULL == hDeviceNotify2)
						{
							showError(L"FAILED RegisterDeviceNotification\n");
							return 0;
						} 



				  /*********************************************************************/



				
				 

				return 0;

			}
			
			
		 			
		
		case WM_SIZE:
			{
				if (pWc)
				{
					RECT rc;
					GetClientRect(hwnd, &rc);
					pWc->SetVideoPosition(NULL, &rc);
				}

				return 0;
			}

		case WM_DEVICECHANGE:
		{
			/* switch (wParam)
			{
				case DBT_DEVICEARRIVAL:
					showError(L"DBT_DEVICEARRIVAL\n");
				 	break;

				case  DBT_DEVICEQUERYREMOVE:
					showError(L"DBT_DEVICEQUERYREMOVE\n");
					break;
			}  */


			 showError(L"WM_DEVICECHANGE\n");

			


			/************************** THIS CODE WORKS AND IT'S FOR BLUETOOTH DEVICE NOTIFICATIONS******
			 PDEV_BROADCAST_HANDLE dbh = (PDEV_BROADCAST_HANDLE) lParam;
			if(dbh->dbch_eventguid == GUID_BLUETOOTH_RADIO_IN_RANGE)
			{
				showError(L"dbh.dbch_eventguid == GUID_BLUETOOTH_RADIO_IN_RANGE\n");

				PBTH_RADIO_IN_RANGE rir = (PBTH_RADIO_IN_RANGE) dbh->dbch_data;
				BTH_DEVICE_INFO deviceInfo = rir->deviceInfo;
				if(GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_PHONE)
				{
					showError(L"GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_PHONE\n");
				}

				if(GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_AUDIO)
				{
					showError(L"GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_AUDIO\n");
				}

				if(GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_PERIPHERAL)
				{
					showError(L"GET_COD_MAJOR(deviceInfo.classOfDevice) == COD_MAJOR_PERIPHERAL\n");
				}

			}

			
			if(dbh->dbch_eventguid == GUID_BLUETOOTH_HCI_EVENT)
			{
				showError(L"dbh->dbch_eventguid == GUID_BLUETOOTH_HCI_EVENT\n");
			}

			if(dbh->dbch_eventguid == GUID_BLUETOOTH_L2CAP_EVENT)
			{
				showError(L"dbh->dbch_eventguid == GUID_BLUETOOTH_L2CAP_EVENT\n");

				PBTH_L2CAP_EVENT_INFO L2CAP_EVENT_INFO = (PBTH_L2CAP_EVENT_INFO) dbh->dbch_data;
				if(L2CAP_EVENT_INFO->connected == 0)
				{
					showError(L"channel has been terminated\n");
				}

				if(L2CAP_EVENT_INFO->connected != 0)
				{
					showError(L"channel has been connected\n");
				}
			}
				*/					



			return 0;
		}			

		 

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				EndPaint(hwnd, &ps);

				if(pWc)
				{
					pWc->RepaintVideo(hwnd, hdc);
				}				

				return 0;
			}


			
		case WM_CLOSE:
			{
				freeResources(hwnd);
				 
			    CoUninitialize();			  
				MFShutdown();
				DestroyWindow(hwnd);
				return 0;
			/**********************************************/

			}
				


	 

			   

		 
		 

		case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pMyCDS;
			pMyCDS = (PCOPYDATASTRUCT) lParam;

			MessageBox(
							NULL,
							(LPCWSTR) (pMyCDS->lpData),
							(LPCWSTR) L"WM_COPYDATA",
							MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
							); 

			return 0;

		}

		/*  // THIS PARTIALLY WORKS. I'M RECEIVING A NOT NULL but BLANK LPARAM ON THE OTHER APP.
		default:
			if(uMsg == RegisterWindowMessage(L"PLAYthisTRACK"))
			{
				SendMessage(hwnd, WM_SETTEXT, 0, lParam); 

				 WCHAR urlOfSong = (WCHAR) (lParam);


				 if(urlOfSong == NULL)
				 // if(lParam == NULL)
				 {
				 	MessageBox(
							NULL,
							(LPCWSTR) L"urlOfSong == NULL",
							(LPCWSTR) L"ERROR ERROR ",
							MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
							); 

				 }else{
				 		

				 		MessageBox(
							NULL,
							  (LPCWSTR) urlOfSong,
							// (LPCWSTR) urlOfSong,
							// (LPCWSTR) L"MESSAGE RECEIVED",
							(LPCWSTR) L"Eeey",
							MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
							); 
				 }

				 
			} */
			  
			
	}


	

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
PWSTR lpCmdLine, INT nCmdShow)
{
	/**************EXPERIMENT *******************/
		argument =  lpCmdLine;
	/*******************************************/

	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	// COLORREF rgbGreen = 0x00d0117b;
	// COLORREF rgbGreen = 0x0000fff8;
	  COLORREF rgbGreen = 0x0000ffad;
	// COLORREF rgbGreen = 0x0003ff37;
	// COLORREF rgbGreen = 0x001d07ff;
	// COLORREF rgbGreen = 0x00241441;
	// COLORREF rgbGreen = 0x008060FF;
	HBRUSH hbrush = CreateSolidBrush(rgbGreen);

	WNDCLASSEX wc = { };
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW; 
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;  


	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName =  CLASS_NAME;
	wc.hbrBackground = hbrush;

	 
	 
	RegisterClassEx(&wc);

	HWND hwnd = CreateWindowEx(
		0, // Optional window styles.
		 CLASS_NAME, // Window class
		 L"Alto Grade Media Player", // Window text
		   WS_OVERLAPPEDWINDOW , // Window style
		 // Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		// CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, // Parent window
		NULL, // Menu
		hInstance, // Instance handle
		NULL // Additional application data
		);

	ShowWindow(hwnd, nCmdShow);



	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}



	return 0;
}

