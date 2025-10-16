/**
 * Test Loopback Read (Windows/WASAPI)
 * 
 * Reads audio from the default WASAPI capture device
 * This is a demonstration of Windows audio capture using WASAPI.
 * 
 * For a complete virtual sound card loopback, use the WDM driver implementation
 * which creates a virtual audio device with loopback capabilities.
 */

#ifdef _WIN32

#define COBJMACROS
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define READ_DURATION 2
#define EXPECTED_FREQUENCY 440.0
#define FREQUENCY_TOLERANCE 5.0

/* COM GUIDs */
const CLSID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
const IID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
const IID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
const IID IID_IAudioCaptureClient = {0xC8ADBD64, 0xE71E, 0x48a0, {0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17}};

/**
 * Simple zero-crossing frequency detector
 */
static double detect_frequency(float *buffer, size_t num_samples, int sample_rate)
{
	int zero_crossings = 0;
	float prev_sample = buffer[0];

	for (size_t i = 1; i < num_samples; i++) {
		float curr_sample = buffer[i];
		if ((prev_sample < 0 && curr_sample >= 0) ||
		    (prev_sample >= 0 && curr_sample < 0)) {
			zero_crossings++;
		}
		prev_sample = curr_sample;
	}

	/* Frequency = (zero crossings / 2) / duration */
	double duration = (double)num_samples / sample_rate;
	double frequency = (zero_crossings / 2.0) / duration;

	return frequency;
}

/**
 * Check if signal has reasonable amplitude
 */
static int check_amplitude(float *buffer, size_t num_samples)
{
	double sum = 0.0;
	double sum_sq = 0.0;

	for (size_t i = 0; i < num_samples; i++) {
		sum += buffer[i];
		sum_sq += buffer[i] * buffer[i];
	}

	double mean = sum / num_samples;
	double rms = sqrt(sum_sq / num_samples);

	/* RMS should be significant (not silent) */
	if (rms < 0.01) {
		fprintf(stderr, "Signal too quiet (RMS: %.4f)\n", rms);
		return 0;
	}

	/* Mean should be close to zero (no DC offset) */
	if (fabs(mean) > 0.1) {
		fprintf(stderr, "Signal has DC offset (mean: %.4f)\n", mean);
		return 0;
	}

	printf("Signal amplitude OK (RMS: %.4f, mean: %.4f)\n", rms, mean);
	return 1;
}

int main(void)
{
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioCaptureClient *pCaptureClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	UINT32 bufferFrameCount;
	UINT32 packetLength = 0;
	BYTE *pData;
	DWORD flags;
	int test_passed = 1;
	float *all_samples = NULL;
	size_t samples_collected = 0;
	size_t samples_capacity = 0;

	printf("Windows WASAPI Loopback Read Test\n");
	printf("===================================\n");
	printf("Reading from default capture device...\n");
	printf("Expected frequency: %.2f Hz\n", EXPECTED_FREQUENCY);
	printf("Duration: %d seconds\n", READ_DURATION);
	printf("\n");
	printf("Note: Make sure audio is playing through your system,\n");
	printf("      or use a virtual audio cable for loopback testing.\n");
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

	/* Get default audio capture endpoint */
	hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eCapture,
							eConsole, &pDevice);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get default audio capture endpoint: 0x%lx\n", hr);
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

	/* Get capture client */
	hr = IAudioClient_GetService(pAudioClient, &IID_IAudioCaptureClient,
				    (void**)&pCaptureClient);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to get capture client: 0x%lx\n", hr);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Allocate buffer for analysis */
	samples_capacity = READ_DURATION * pwfx->nSamplesPerSec;
	all_samples = (float*)malloc(samples_capacity * sizeof(float));
	if (!all_samples) {
		fprintf(stderr, "Failed to allocate analysis buffer\n");
		IAudioCaptureClient_Release(pCaptureClient);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	printf("Reading audio...\n");

	/* Start audio client */
	hr = IAudioClient_Start(pAudioClient);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to start audio client: 0x%lx\n", hr);
		free(all_samples);
		IAudioCaptureClient_Release(pCaptureClient);
		CoTaskMemFree(pwfx);
		IAudioClient_Release(pAudioClient);
		IMMDevice_Release(pDevice);
		IMMDeviceEnumerator_Release(pEnumerator);
		CoUninitialize();
		return 1;
	}

	/* Capture audio for specified duration */
	DWORD startTime = GetTickCount();
	DWORD duration_ms = READ_DURATION * 1000;

	while ((GetTickCount() - startTime) < duration_ms) {
		/* Sleep a bit before checking for data */
		Sleep(10);

		/* Check if there's data available */
		hr = IAudioCaptureClient_GetNextPacketSize(pCaptureClient, &packetLength);
		if (FAILED(hr)) {
			fprintf(stderr, "Failed to get packet size: 0x%lx\n", hr);
			break;
		}

		while (packetLength != 0) {
			UINT32 numFramesToRead;

			/* Get the captured data */
			hr = IAudioCaptureClient_GetBuffer(pCaptureClient, &pData,
							  &numFramesToRead, &flags, NULL, NULL);
			if (FAILED(hr)) {
				fprintf(stderr, "Failed to get buffer: 0x%lx\n", hr);
				break;
			}

			/* Extract mono samples (left channel only) */
			if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
				float *floatData = (float*)pData;
				for (UINT32 i = 0; i < numFramesToRead && samples_collected < samples_capacity; i++) {
					all_samples[samples_collected++] = floatData[i * pwfx->nChannels];
				}
			}

			/* Release buffer */
			hr = IAudioCaptureClient_ReleaseBuffer(pCaptureClient, numFramesToRead);
			if (FAILED(hr)) {
				fprintf(stderr, "Failed to release buffer: 0x%lx\n", hr);
				break;
			}

			/* Check for next packet */
			hr = IAudioCaptureClient_GetNextPacketSize(pCaptureClient, &packetLength);
			if (FAILED(hr)) {
				fprintf(stderr, "Failed to get packet size: 0x%lx\n", hr);
				break;
			}
		}

		/* Print progress */
		DWORD elapsed = GetTickCount() - startTime;
		float progress = (float)elapsed / duration_ms * 100.0f;
		printf("\rProgress: %.1f%%", progress);
		fflush(stdout);
	}

	printf("\rProgress: 100.0%%\n");
	printf("Read complete. Analyzing...\n\n");

	/* Stop audio client */
	IAudioClient_Stop(pAudioClient);

	/* Analyze the captured audio */
	printf("=== Analysis Results ===\n");
	printf("Samples collected: %zu\n", samples_collected);

	if (samples_collected > 0) {
		/* Check amplitude */
		if (!check_amplitude(all_samples, samples_collected)) {
			test_passed = 0;
		}

		/* Detect frequency */
		double detected_freq = detect_frequency(all_samples, samples_collected,
						       pwfx->nSamplesPerSec);
		printf("Detected frequency: %.2f Hz\n", detected_freq);

		/* Verify frequency is close to expected */
		double freq_diff = fabs(detected_freq - EXPECTED_FREQUENCY);
		if (freq_diff > FREQUENCY_TOLERANCE) {
			fprintf(stderr,
				"FAIL: Frequency mismatch (expected %.2f ± %.2f Hz, got %.2f Hz)\n",
				EXPECTED_FREQUENCY, FREQUENCY_TOLERANCE, detected_freq);
			test_passed = 0;
		} else {
			printf("PASS: Frequency within tolerance (%.2f Hz)\n", freq_diff);
		}
	} else {
		fprintf(stderr, "FAIL: No audio data captured\n");
		test_passed = 0;
	}

	printf("\n");

	/* Cleanup */
	free(all_samples);
	IAudioCaptureClient_Release(pCaptureClient);
	CoTaskMemFree(pwfx);
	IAudioClient_Release(pAudioClient);
	IMMDevice_Release(pDevice);
	IMMDeviceEnumerator_Release(pEnumerator);
	CoUninitialize();

	if (test_passed) {
		printf("=== TEST PASSED ===\n");
		return 0;
	} else {
		printf("=== TEST FAILED ===\n");
		return 1;
	}
}

#else

#include <stdio.h>

int main(void)
{
	fprintf(stderr, "This program is designed for Windows only.\n");
	fprintf(stderr, "On Linux, use the ALSA implementation in ../../linux/tests/\n");
	return 1;
}

#endif /* _WIN32 */
