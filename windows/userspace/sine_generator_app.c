/**
 * Sine Wave Generator Application (Windows/WASAPI)
 * 
 * Generates a sine wave and plays it through the default WASAPI audio device
 * Usage: sine_generator_app.exe [frequency] [duration_seconds]
 * 
 * This is a demonstration of Windows audio output using WASAPI.
 * For a complete virtual sound card, use the WDM driver implementation.
 */

#ifdef _WIN32

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_DURATION 5
#define SAMPLE_RATE 48000
#define CHANNELS 2

/* COM GUIDs */
const CLSID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
const IID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
const IID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
const IID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}};

typedef struct {
	double phase;
	double frequency;
	double sample_rate;
	double amplitude;
} sine_generator_t;

static void sine_generator_init(sine_generator_t *gen, double frequency,
				double sample_rate, double amplitude)
{
	gen->phase = 0.0;
	gen->frequency = frequency;
	gen->sample_rate = sample_rate;
	gen->amplitude = amplitude;
}

static void sine_generator_process_float(sine_generator_t *gen, float *buffer,
					 size_t num_samples, int channels)
{
	double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

	for (size_t i = 0; i < num_samples; i++) {
		float sample = (float)(gen->amplitude * sin(gen->phase));
		
		/* Write to all channels */
		for (int ch = 0; ch < channels; ch++) {
			buffer[i * channels + ch] = sample;
		}
		
		gen->phase += phase_increment;
		if (gen->phase >= 2.0 * M_PI) {
			gen->phase -= 2.0 * M_PI;
		}
	}
}

int main(int argc, char *argv[])
{
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	UINT32 bufferFrameCount;
	BYTE *pData;
	sine_generator_t gen;
	double frequency = DEFAULT_FREQUENCY;
	int duration = DEFAULT_DURATION;
	DWORD flags = 0;

	/* Parse command line arguments */
	if (argc >= 2) {
		frequency = atof(argv[1]);
		if (frequency <= 0 || frequency > 20000) {
			fprintf(stderr, "Invalid frequency: %.2f Hz\n", frequency);
			return 1;
		}
	}
	if (argc >= 3) {
		duration = atoi(argv[2]);
		if (duration <= 0 || duration > 60) {
			fprintf(stderr, "Invalid duration: %d seconds\n", duration);
			return 1;
		}
	}

	printf("Windows WASAPI Sine Wave Generator\n");
	printf("===================================\n");
	printf("Frequency: %.2f Hz\n", frequency);
	printf("Duration: %d seconds\n", duration);
	printf("\n");

	/* Initialize COM */
	hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to initialize COM: 0x%lx\n", hr);
		return 1;
	}

	/* Create device enumerator */
	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			      &IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to create device enumerator: 0x%lx\n", hr);
		CoUninitialize();
		return 1;
	}

	/* Get default audio endpoint */
	hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eRender,
							eConsole, &pDevice);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get default audio endpoint: 0x%lx\n", hr);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Activate audio client */
	hr = IMMDevice_Activate(pDevice, &IID_IAudioClient, CLSCTX_ALL,
			       NULL, (void**)&pAudioClient);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to activate audio client: 0x%lx\n", hr);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Get mix format */
	hr = IAudioClient_GetMixFormat(pAudioClient, &pwfx);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get mix format: 0x%lx\n", hr);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	printf("Sample Rate: %ld Hz\n", pwfx->nSamplesPerSec);
	printf("Channels: %d\n", pwfx->nChannels);
	printf("Format: ");
	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		printf("IEEE Float\n");
	} else if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
		printf("PCM %d-bit\n", pwfx->wBitsPerSample);
	} else {
		printf("Unknown (0x%x)\n", pwfx->wFormatTag);
	}
	printf("\n");

	/* Initialize audio client */
	hr = IAudioClient_Initialize(pAudioClient, AUDCLNT_SHAREMODE_SHARED,
				     0, 10000000, 0, pwfx, NULL);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to initialize audio client: 0x%lx\n", hr);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Get buffer size */
	hr = IAudioClient_GetBufferSize(pAudioClient, &bufferFrameCount);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get buffer size: 0x%lx\n", hr);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Get render client */
	hr = IAudioClient_GetService(pAudioClient, &IID_IAudioRenderClient,
				    (void**)&pRenderClient);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get render client: 0x%lx\n", hr);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Initialize sine generator */
	sine_generator_init(&gen, frequency, pwfx->nSamplesPerSec, 0.5);

	printf("Playing sine wave...\n");

	/* Start audio client */
	hr = IAudioClient_Start(pAudioClient);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to start audio client: 0x%lx\n", hr);
		IAudioRenderClient_Release(pRenderClient);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Generate and play audio */
	int total_frames = duration * pwfx->nSamplesPerSec;
	int frames_written = 0;

	while (frames_written < total_frames) {
		UINT32 numFramesPadding;
		UINT32 numFramesAvailable;

		/* Check how much buffer space is available */
		hr = IAudioClient_GetCurrentPadding(pAudioClient, &numFramesPadding);
		if (FAILED(hr)) {
			fprintf(stderr, "Failed to get current padding: 0x%lx\n", hr);
			break;
		}

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		if (numFramesAvailable > 0) {
			/* Get buffer */
			hr = IAudioRenderClient_GetBuffer(pRenderClient, numFramesAvailable, &pData);
			if (FAILED(hr)) {
				fprintf(stderr, "Failed to get buffer: 0x%lx\n", hr);
				break;
			}

			/* Generate audio */
			if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
				sine_generator_process_float(&gen, (float*)pData,
							    numFramesAvailable,
							    pwfx->nChannels);
			} else {
				/* For PCM, we'd need to convert - for now just silence */
				memset(pData, 0, numFramesAvailable * pwfx->nBlockAlign);
			}

			/* Release buffer */
			hr = IAudioRenderClient_ReleaseBuffer(pRenderClient,
							     numFramesAvailable, flags);
			if (FAILED(hr)) {
				fprintf(stderr, "Failed to release buffer: 0x%lx\n", hr);
				break;
			}

			frames_written += numFramesAvailable;

			/* Print progress */
			if (frames_written % (pwfx->nSamplesPerSec / 4) == 0) {
				float progress = (float)frames_written / total_frames * 100.0f;
				printf("\rProgress: %.1f%%", progress);
				fflush(stdout);
			}
		}

		/* Sleep a bit to avoid busy-waiting */
		Sleep(10);
	}

	printf("\rProgress: 100.0%%\n");
	printf("Playback complete!\n");

	/* Stop audio client */
	IAudioClient_Stop(pAudioClient);

	/* Cleanup */
	IAudioRenderClient_Release(pRenderClient);
	CoTaskMemFree(pwfx);
	IAudioClient_Release(pAudioClient);
	IMMDevice_Release(pDevice);
	IMMDeviceEnumerator_Release(pEnumerator);
	CoUninitialize();

	return 0;
}

#else

#include <stdio.h>

int main(void)
{
	fprintf(stderr, "This program is designed for Windows only.\n");
	fprintf(stderr, "On Linux, use the ALSA implementation in ../linux/\n");
	return 1;
}

#endif /* _WIN32 */
