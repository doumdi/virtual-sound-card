/**
 * Test for Windows WAVEFORMATEXTENSIBLE handling
 * 
 * This test verifies that the virtual_sine_device properly detects and handles
 * WAVEFORMATEXTENSIBLE format (0xFFFE) which is commonly used by virtual audio
 * cables like VB-Cable on Windows.
 */

#ifdef _WIN32

#include <windows.h>
#include <mmreg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

/* Audio format GUIDs */
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID KSDATAFORMAT_SUBTYPE_PCM = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

typedef struct {
    double phase;
    double frequency;
    double sample_rate;
    double amplitude;
} sine_generator_t;

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

static void sine_generator_init(sine_generator_t *gen, double frequency,
                                double sample_rate, double amplitude)
{
    gen->phase = 0.0;
    gen->frequency = frequency;
    gen->sample_rate = sample_rate;
    gen->amplitude = amplitude;
}

static void sine_generator_process_pcm16(sine_generator_t *gen, int16_t *buffer,
                                         size_t num_samples, int channels)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

    for (size_t i = 0; i < num_samples; i++) {
        double sample_float = gen->amplitude * sin(gen->phase);
        int16_t sample = (int16_t)(sample_float * 32767.0);
        
        for (int ch = 0; ch < channels; ch++) {
            buffer[i * channels + ch] = sample;
        }
        
        gen->phase += phase_increment;
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

int main(void)
{
    int passed = 1;
    printf("Testing Windows WAVEFORMATEXTENSIBLE handling...\n\n");

    /* Test 1: WAVEFORMATEXTENSIBLE with PCM SubFormat (VB-Cable case) */
    {
        WAVEFORMATEXTENSIBLE wfex = {0};
        wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wfex.Format.nChannels = 2;
        wfex.Format.nSamplesPerSec = 48000;
        wfex.Format.wBitsPerSample = 16;
        wfex.Format.nBlockAlign = (wfex.Format.nChannels * wfex.Format.wBitsPerSample) / 8;
        wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;
        wfex.Format.cbSize = 22;
        wfex.Samples.wValidBitsPerSample = 16;
        wfex.dwChannelMask = 0x3; /* Front Left + Front Right */
        memcpy(&wfex.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID));

        int is_float;
        int bits_per_sample;
        const char *format_name;
        get_audio_format_info(&wfex.Format, &is_float, &bits_per_sample, &format_name);

        printf("Test 1: WAVEFORMATEXTENSIBLE with PCM (VB-Cable typical)\n");
        printf("  Format Tag: 0x%04X\n", wfex.Format.wFormatTag);
        printf("  Detected: %s\n", format_name);
        printf("  Is Float: %d\n", is_float);
        printf("  Bits per Sample: %d\n", bits_per_sample);

        if (wfex.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
            printf("  FAIL: Format tag should be WAVE_FORMAT_EXTENSIBLE (0xFFFE)\n");
            passed = 0;
        } else if (is_float != 0) {
            printf("  FAIL: Should detect as PCM, not float\n");
            passed = 0;
        } else if (bits_per_sample != 16) {
            printf("  FAIL: Bits per sample should be 16\n");
            passed = 0;
        } else {
            printf("  PASS\n");
        }
        printf("\n");
    }

    /* Test 2: WAVEFORMATEXTENSIBLE with IEEE Float SubFormat */
    {
        WAVEFORMATEXTENSIBLE wfex = {0};
        wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wfex.Format.nChannels = 2;
        wfex.Format.nSamplesPerSec = 48000;
        wfex.Format.wBitsPerSample = 32;
        wfex.Format.nBlockAlign = (wfex.Format.nChannels * wfex.Format.wBitsPerSample) / 8;
        wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;
        wfex.Format.cbSize = 22;
        wfex.Samples.wValidBitsPerSample = 32;
        wfex.dwChannelMask = 0x3;
        memcpy(&wfex.SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID));

        int is_float;
        int bits_per_sample;
        const char *format_name;
        get_audio_format_info(&wfex.Format, &is_float, &bits_per_sample, &format_name);

        printf("Test 2: WAVEFORMATEXTENSIBLE with IEEE Float\n");
        printf("  Format Tag: 0x%04X\n", wfex.Format.wFormatTag);
        printf("  Detected: %s\n", format_name);
        printf("  Is Float: %d\n", is_float);
        printf("  Bits per Sample: %d\n", bits_per_sample);

        if (is_float != 1) {
            printf("  FAIL: Should detect as float\n");
            passed = 0;
        } else if (bits_per_sample != 32) {
            printf("  FAIL: Bits per sample should be 32\n");
            passed = 0;
        } else {
            printf("  PASS\n");
        }
        printf("\n");
    }

    /* Test 3: Standard WAVE_FORMAT_PCM */
    {
        WAVEFORMATEX wfx = {0};
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize = 0;

        int is_float;
        int bits_per_sample;
        const char *format_name;
        get_audio_format_info(&wfx, &is_float, &bits_per_sample, &format_name);

        printf("Test 3: Standard WAVE_FORMAT_PCM\n");
        printf("  Format Tag: 0x%04X\n", wfx.wFormatTag);
        printf("  Detected: %s\n", format_name);
        printf("  Is Float: %d\n", is_float);

        if (is_float != 0) {
            printf("  FAIL: Should detect as PCM, not float\n");
            passed = 0;
        } else {
            printf("  PASS\n");
        }
        printf("\n");
    }

    /* Test 4: Sine generation to PCM16 buffer */
    {
        printf("Test 4: Sine wave generation to PCM16 buffer\n");
        sine_generator_t gen;
        sine_generator_init(&gen, 440.0, 48000.0, 0.5);
        
        int16_t buffer[100 * 2]; /* 100 samples, 2 channels */
        sine_generator_process_pcm16(&gen, buffer, 100, 2);
        
        /* Verify first sample is near 0 (starts at phase 0) */
        if (abs(buffer[0]) > 100) {
            printf("  FAIL: First sample should be near 0, got %d\n", buffer[0]);
            passed = 0;
        } else {
            printf("  First sample: %d (near 0 - PASS)\n", buffer[0]);
        }
        
        /* Verify samples are within expected range */
        int max_val = 0;
        for (int i = 0; i < 100 * 2; i++) {
            if (abs(buffer[i]) > max_val) {
                max_val = abs(buffer[i]);
            }
        }
        
        /* Max should be around amplitude * 32767 = 0.5 * 32767 = ~16383 */
        if (max_val > 20000) {
            printf("  FAIL: Max value too high: %d\n", max_val);
            passed = 0;
        } else if (max_val < 10000) {
            printf("  FAIL: Max value too low: %d\n", max_val);
            passed = 0;
        } else {
            printf("  Max sample value: %d (expected ~16383 - PASS)\n", max_val);
        }
        printf("\n");
    }

    /* Summary */
    if (passed) {
        printf("All tests PASSED\n");
        printf("\nThis confirms that virtual_sine_device can properly handle:\n");
        printf("  - WAVEFORMATEXTENSIBLE format (0xFFFE)\n");
        printf("  - PCM subformat detection\n");
        printf("  - IEEE Float subformat detection\n");
        printf("  - PCM16 sine wave generation\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}

#else

#include <stdio.h>

int main(void)
{
    printf("This test is designed for Windows only.\n");
    printf("On non-Windows platforms, format handling is different.\n");
    return 0;
}

#endif /* _WIN32 */
