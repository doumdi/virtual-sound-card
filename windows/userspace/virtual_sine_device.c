/**
 * Virtual Sine Wave Device for Windows
 * 
 * Generates a continuous sine wave that outputs to a specified audio device.
 * This program is designed to be used with a virtual audio device like VB-Cable,
 * allowing other applications to read the sine wave as an input source.
 * 
 * Usage: ./virtual_sine_device.exe [options]
 *   -f <frequency>   Sine wave frequency in Hz (default: 440.0)
 *   -d <device>      Output device name (default: system default)
 *   -r <rate>        Sample rate in Hz (default: 48000)
 *   -c <channels>    Number of channels (default: 2)
 *   -a <amplitude>   Amplitude 0.0-1.0 (default: 0.5)
 *   -l               List available audio devices
 *   -h               Show this help message
 * 
 * Examples:
 *   ./virtual_sine_device.exe -f 440 -d "CABLE Input"
 *   ./virtual_sine_device.exe -f 880 -a 0.3
 */

#ifdef _WIN32

#define COBJMACROS
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <mmreg.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* WAVEFORMATEXTENSIBLE is not always defined */
#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_CHANNELS 2
#define DEFAULT_AMPLITUDE 0.5

/* COM GUIDs */
const CLSID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
const IID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
const IID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
const IID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}};

/* Audio format GUIDs for WAVEFORMATEXTENSIBLE */
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID KSDATAFORMAT_SUBTYPE_PCM = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

typedef struct {
    double phase;
    double frequency;
    double sample_rate;
    double amplitude;
} sine_generator_t;

static volatile int g_running = 1;

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

static void sine_generator_process_pcm16(sine_generator_t *gen, int16_t *buffer,
                                         size_t num_samples, int channels)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

    for (size_t i = 0; i < num_samples; i++) {
        double sample_float = gen->amplitude * sin(gen->phase);
        /* Convert to 16-bit PCM: scale from [-1.0, 1.0] to [-32768, 32767] */
        int16_t sample = (int16_t)(sample_float * 32767.0);
        
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

static void sine_generator_process_pcm24(sine_generator_t *gen, uint8_t *buffer,
                                         size_t num_samples, int channels)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

    for (size_t i = 0; i < num_samples; i++) {
        double sample_float = gen->amplitude * sin(gen->phase);
        /* Convert to 24-bit PCM: scale from [-1.0, 1.0] to [-8388608, 8388607] */
        int32_t sample = (int32_t)(sample_float * 8388607.0);
        
        /* Write to all channels (24-bit stored as 3 bytes, little-endian) */
        for (int ch = 0; ch < channels; ch++) {
            size_t offset = (i * channels + ch) * 3;
            buffer[offset + 0] = (uint8_t)(sample & 0xFF);
            buffer[offset + 1] = (uint8_t)((sample >> 8) & 0xFF);
            buffer[offset + 2] = (uint8_t)((sample >> 16) & 0xFF);
        }
        
        gen->phase += phase_increment;
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

static void sine_generator_process_pcm32(sine_generator_t *gen, int32_t *buffer,
                                         size_t num_samples, int channels)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

    for (size_t i = 0; i < num_samples; i++) {
        double sample_float = gen->amplitude * sin(gen->phase);
        /* Convert to 32-bit PCM: scale from [-1.0, 1.0] to [-2147483648, 2147483647] */
        int32_t sample = (int32_t)(sample_float * 2147483647.0);
        
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

/* Helper function to check if two GUIDs are equal */
static int guid_equals(const GUID *a, const GUID *b)
{
    return memcmp(a, b, sizeof(GUID)) == 0;
}

/* Get actual audio format from WAVEFORMATEX or WAVEFORMATEXTENSIBLE */
static void get_audio_format_info(const WAVEFORMATEX *pwfx, 
                                  int *is_float, int *bits_per_sample,
                                  const char **format_name)
{
    *is_float = 0;
    *bits_per_sample = pwfx->wBitsPerSample;
    *format_name = "Unknown";

    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && pwfx->cbSize >= 22) {
        /* Cast to WAVEFORMATEXTENSIBLE */
        const WAVEFORMATEXTENSIBLE *pwfex = (const WAVEFORMATEXTENSIBLE*)pwfx;
        
        if (guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
            *is_float = 1;
            *format_name = "IEEE Float (via EXTENSIBLE)";
        } else if (guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM)) {
            *is_float = 0;
            *format_name = "PCM (via EXTENSIBLE)";
        } else {
            *format_name = "Unknown EXTENSIBLE SubFormat";
        }
    } else if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        *is_float = 1;
        *format_name = "IEEE Float";
    } else if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
        *is_float = 0;
        *format_name = "PCM";
    }
}

static BOOL WINAPI console_handler(DWORD signal)
{
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT) {
        g_running = 0;
        printf("\nShutting down...\n");
        return TRUE;
    }
    return FALSE;
}

static void list_audio_devices(void)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDeviceCollection *pCollection = NULL;
    UINT count = 0;

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create device enumerator\n");
        return;
    }

    hr = IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eRender,
                                                DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to enumerate audio endpoints\n");
        IMMDeviceEnumerator_Release(pEnumerator);
        return;
    }

    hr = IMMDeviceCollection_GetCount(pCollection, &count);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to get device count\n");
        IMMDeviceCollection_Release(pCollection);
        IMMDeviceEnumerator_Release(pEnumerator);
        return;
    }

    printf("\nAvailable audio output devices:\n");
    printf("================================\n");

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pDevice = NULL;
        IPropertyStore *pProps = NULL;
        PROPVARIANT varName;

        PropVariantInit(&varName);

        hr = IMMDeviceCollection_Item(pCollection, i, &pDevice);
        if (SUCCEEDED(hr)) {
            hr = IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &pProps);
            if (SUCCEEDED(hr)) {
                hr = IPropertyStore_GetValue(pProps, &PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
                    wprintf(L"  %d: %s\n", i + 1, varName.pwszVal);
                }
                PropVariantClear(&varName);
                IPropertyStore_Release(pProps);
            }
            IMMDevice_Release(pDevice);
        }
    }

    printf("\n");
    IMMDeviceCollection_Release(pCollection);
    IMMDeviceEnumerator_Release(pEnumerator);
}

static IMMDevice* find_device_by_name(IMMDeviceEnumerator *pEnumerator, const wchar_t *device_name)
{
    HRESULT hr;
    IMMDeviceCollection *pCollection = NULL;
    IMMDevice *pFoundDevice = NULL;
    UINT count = 0;

    hr = IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eRender,
                                                DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        return NULL;
    }

    hr = IMMDeviceCollection_GetCount(pCollection, &count);
    if (FAILED(hr)) {
        IMMDeviceCollection_Release(pCollection);
        return NULL;
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pDevice = NULL;
        IPropertyStore *pProps = NULL;
        PROPVARIANT varName;

        PropVariantInit(&varName);

        hr = IMMDeviceCollection_Item(pCollection, i, &pDevice);
        if (SUCCEEDED(hr)) {
            hr = IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &pProps);
            if (SUCCEEDED(hr)) {
                hr = IPropertyStore_GetValue(pProps, &PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
                    if (wcsstr(varName.pwszVal, device_name) != NULL) {
                        pFoundDevice = pDevice;
                        IMMDevice_AddRef(pFoundDevice);
                    }
                }
                PropVariantClear(&varName);
                IPropertyStore_Release(pProps);
            }
            IMMDevice_Release(pDevice);
            
            if (pFoundDevice) {
                break;
            }
        }
    }

    IMMDeviceCollection_Release(pCollection);
    return pFoundDevice;
}

static void print_usage(const char *program_name)
{
    printf("Usage: %s [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  -f <frequency>   Sine wave frequency in Hz (default: %.1f)\n", DEFAULT_FREQUENCY);
    printf("  -d <device>      Output device name (e.g., \"CABLE Input\" for VB-Cable)\n");
    printf("  -r <rate>        Sample rate in Hz (default: %d)\n", DEFAULT_SAMPLE_RATE);
    printf("  -c <channels>    Number of channels (default: %d)\n", DEFAULT_CHANNELS);
    printf("  -a <amplitude>   Amplitude 0.0-1.0 (default: %.1f)\n", DEFAULT_AMPLITUDE);
    printf("  -l               List available audio devices\n");
    printf("  -h               Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s -f 440 -d \"CABLE Input\"\n", program_name);
    printf("  %s -f 880 -a 0.3\n", program_name);
    printf("\nNote: For VB-Cable loopback:\n");
    printf("  1. Install VB-Cable from https://vb-audio.com/Cable/\n");
    printf("  2. Run: %s -d \"CABLE Input\"\n", program_name);
    printf("  3. Applications can now record from \"CABLE Output\"\n");
    printf("\n");
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
    sine_generator_t gen;
    double frequency = DEFAULT_FREQUENCY;
    int sample_rate = DEFAULT_SAMPLE_RATE;
    int channels = DEFAULT_CHANNELS;
    double amplitude = DEFAULT_AMPLITUDE;
    char *device_name = NULL;
    int list_devices = 0;

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            frequency = atof(argv[++i]);
            if (frequency <= 0 || frequency > 20000) {
                fprintf(stderr, "Invalid frequency: %.2f Hz\n", frequency);
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            device_name = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            sample_rate = atoi(argv[++i]);
            if (sample_rate < 8000 || sample_rate > 192000) {
                fprintf(stderr, "Invalid sample rate: %d Hz\n", sample_rate);
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            channels = atoi(argv[++i]);
            if (channels < 1 || channels > 32) {
                fprintf(stderr, "Invalid channel count: %d\n", channels);
                return 1;
            }
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            amplitude = atof(argv[++i]);
            if (amplitude < 0.0 || amplitude > 1.0) {
                fprintf(stderr, "Invalid amplitude: %.2f (must be 0.0-1.0)\n", amplitude);
                return 1;
            }
        } else if (strcmp(argv[i], "-l") == 0) {
            list_devices = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    printf("Windows Virtual Sine Wave Device\n");
    printf("=================================\n");

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

    /* List devices if requested */
    if (list_devices) {
        list_audio_devices();
        IMMDeviceEnumerator_Release(pEnumerator);
        CoUninitialize();
        return 0;
    }

    /* Get audio device */
    if (device_name) {
        wchar_t wdevice_name[256];
        MultiByteToWideChar(CP_UTF8, 0, device_name, -1, wdevice_name, 256);
        pDevice = find_device_by_name(pEnumerator, wdevice_name);
        if (!pDevice) {
            fprintf(stderr, "Could not find device: %s\n", device_name);
            fprintf(stderr, "Use -l to list available devices\n");
            IMMDeviceEnumerator_Release(pEnumerator);
            CoUninitialize();
            return 1;
        }
        printf("Target device: %s\n", device_name);
    } else {
        hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnumerator, eRender,
                                                        eConsole, &pDevice);
        if (FAILED(hr)) {
            fprintf(stderr, "Failed to get default audio endpoint: 0x%lx\n", hr);
            IMMDeviceEnumerator_Release(pEnumerator);
            CoUninitialize();
            return 1;
        }
        printf("Target device: Default\n");
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

    printf("Frequency: %.2f Hz\n", frequency);
    printf("Sample Rate: %ld Hz\n", pwfx->nSamplesPerSec);
    printf("Channels: %d\n", pwfx->nChannels);
    printf("Bits per Sample: %d\n", pwfx->wBitsPerSample);
    printf("Format Tag: 0x%04X\n", pwfx->wFormatTag);
    
    int is_float;
    int bits_per_sample;
    const char *format_name;
    get_audio_format_info(pwfx, &is_float, &bits_per_sample, &format_name);
    printf("Format: %s\n", format_name);
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
    sine_generator_init(&gen, frequency, pwfx->nSamplesPerSec, amplitude);

    /* Set up console handler for Ctrl+C */
    SetConsoleCtrlHandler(console_handler, TRUE);

    printf("Generating continuous sine wave...\n");
    printf("Press Ctrl+C to stop\n\n");

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

    /* Generate and play audio continuously */
    while (g_running) {
        UINT32 numFramesPadding;
        UINT32 numFramesAvailable;
        BYTE *pData;

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

            /* Generate audio based on format */
            int is_float;
            int bits_per_sample;
            const char *format_name;
            get_audio_format_info(pwfx, &is_float, &bits_per_sample, &format_name);
            
            if (is_float) {
                /* IEEE Float format */
                sine_generator_process_float(&gen, (float*)pData,
                                            numFramesAvailable,
                                            pwfx->nChannels);
            } else {
                /* PCM format - handle different bit depths */
                if (bits_per_sample == 16) {
                    sine_generator_process_pcm16(&gen, (int16_t*)pData,
                                                 numFramesAvailable,
                                                 pwfx->nChannels);
                } else if (bits_per_sample == 24) {
                    sine_generator_process_pcm24(&gen, (uint8_t*)pData,
                                                 numFramesAvailable,
                                                 pwfx->nChannels);
                } else if (bits_per_sample == 32) {
                    sine_generator_process_pcm32(&gen, (int32_t*)pData,
                                                 numFramesAvailable,
                                                 pwfx->nChannels);
                } else {
                    /* Unsupported bit depth - output silence */
                    memset(pData, 0, numFramesAvailable * pwfx->nBlockAlign);
                }
            }

            /* Release buffer */
            hr = IAudioRenderClient_ReleaseBuffer(pRenderClient,
                                                 numFramesAvailable, 0);
            if (FAILED(hr)) {
                fprintf(stderr, "Failed to release buffer: 0x%lx\n", hr);
                break;
            }
        }

        /* Sleep to avoid busy-waiting */
        Sleep(10);
    }

    printf("\nStopping audio...\n");

    /* Stop audio client */
    IAudioClient_Stop(pAudioClient);

    /* Cleanup */
    IAudioRenderClient_Release(pRenderClient);
    CoTaskMemFree(pwfx);
    IAudioClient_Release(pAudioClient);
    IMMDevice_Release(pDevice);
    IMMDeviceEnumerator_Release(pEnumerator);
    CoUninitialize();

    printf("Done.\n");
    return 0;
}

#else

#include <stdio.h>

int main(void)
{
    fprintf(stderr, "This program is designed for Windows only.\n");
    fprintf(stderr, "On Linux, use the ALSA implementation in ../linux/\n");
    fprintf(stderr, "On macOS, use the CoreAudio implementation in ../macos/\n");
    return 1;
}

#endif /* _WIN32 */
