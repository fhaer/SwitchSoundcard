#include "stdafx.h"
#include "stdio.h"
#include "wchar.h"
#include "tchar.h"
#include "windows.h"
#include "mmdeviceapi.h"
#include "endpointvolume.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);

		LPWSTR defaultWstrID = NULL;
		IMMDevice *defaultDevice = NULL; 
		hr = pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
		if (SUCCEEDED(hr))
		{
			hr = defaultDevice->GetId(&defaultWstrID);
		}
		defaultDevice->Release();

		if (SUCCEEDED(hr))
		{
			IMMDeviceCollection *pDevices;
			// Enumerate the output devices.
			hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
			if (SUCCEEDED(hr))
			{
				UINT count;
				pDevices->GetCount(&count);
				bool notSet = true;
				if (SUCCEEDED(hr))
				{
					bool setOnNextIteration = false;
					for (int idx = 0; notSet && idx < count*2; idx++)
					{
						int i = idx%count;
						IMMDevice *pDevice;
						hr = pDevices->Item(i, &pDevice);
						if (SUCCEEDED(hr))
						{
							LPWSTR wstrID = NULL;
							hr = pDevice->GetId(&wstrID);
							if (SUCCEEDED(hr))
							{
								if (wcscmp(wstrID, defaultWstrID) == 0) 
								{
									setOnNextIteration = true;
								} 
								else if(setOnNextIteration)
								{
									IPropertyStore *pStore;
									hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
									if (SUCCEEDED(hr))
									{
										PROPVARIANT friendlyName;
										PropVariantInit(&friendlyName);
										hr = pStore->GetValue(PKEY_Device_DeviceDesc, &friendlyName);
										if (SUCCEEDED(hr))
										{
											char playCommand[50];
											sprintf (playCommand, "start nircmd mediaplay 1000 \"switchsoundcard.wav\""); 
											printf(playCommand);
											system(playCommand);
											// if no options, print the device
											// otherwise, find the selected device and set it to be default
											char name[50];
											sprintf(name, "%ws", friendlyName.pwszVal);
											char command[50];
											sprintf (command, "start nircmd setdefaultsounddevice \"%s\"", name); 
											printf(command);
											system(command);
											PropVariantClear(&friendlyName);
											notSet = false;

											if (strcmp(name, "Lautsprecher") == 0) 
											{
												IAudioEndpointVolume *endpointVolume = NULL;
												hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
												float currentVolumeLeft = 0;
												float currentVolumeRight = 0;
												hr = endpointVolume->GetChannelVolumeLevelScalar(0, &currentVolumeLeft);
												hr = endpointVolume->GetChannelVolumeLevelScalar(1, &currentVolumeRight);
												float newVolumeLeft  = currentVolumeRight / 1.8f;
												float newVolumeRight = currentVolumeRight;
												hr = endpointVolume->SetChannelVolumeLevelScalar(0, newVolumeLeft, NULL);
												hr = endpointVolume->SetChannelVolumeLevelScalar(1, newVolumeRight, NULL);
												endpointVolume->Release(); 
											}
										}
										pStore->Release();
									}
								}
							}
							pDevice->Release(); 
						}
					}
				}
				pDevices->Release();
			}
			pEnum->Release();
		}
	}
	return 0;
}


