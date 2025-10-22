/**
 * Test Format Handling for Windows Virtual Sine Device
 * 
 * Tests the WAVEFORMATEXTENSIBLE handling and format detection functions
 */

#ifdef _WIN32

#define COBJMACROS
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

/* Audio format GUIDs for WAVEFORMATEXTENSIBLE */
static const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {
    0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID KSDATAFORMAT_SUBTYPE_PCM = {
    0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};

/**
 * Helper function to compare GUIDs
 */
static int guid_equals(const GUID *a, const GUID *b)
{
    return memcmp(a, b, sizeof(GUID)) == 0;
}

/**
 * Check if format is IEEE float (supports both simple and extensible formats)
 */
static int is_format_ieee_float(WAVEFORMATEX *pwfx)
{
    if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        return 1;
    }
    
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        WAVEFORMATEXTENSIBLE *pwfex = (WAVEFORMATEXTENSIBLE*)pwfx;
        return guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
    }
    
    return 0;
}

/**
 * Check if format is PCM (supports both simple and extensible formats)
 */
static int is_format_pcm(WAVEFORMATEX *pwfx)
{
    if (pwfx->wFormatTag == WAVE_FORMAT_PCM) {
        return 1;
    }
    
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        WAVEFORMATEXTENSIBLE *pwfex = (WAVEFORMATEXTENSIBLE*)pwfx;
        return guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM);
    }
    
    return 0;
}

/**
 * Sine generator for testing
 */
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
        
        for (int ch = 0; ch < channels; ch++) {
            buffer[i * channels + ch] = sample;
        }
        
        gen->phase += phase_increment;
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

static void sine_generator_process_pcm(sine_generator_t *gen, BYTE *buffer,
                                       size_t num_samples, int channels,
                                       int bits_per_sample)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    int bytes_per_sample = bits_per_sample / 8;
    
    for (size_t i = 0; i < num_samples; i++) {
        double sample_value = gen->amplitude * sin(gen->phase);
        
        for (int ch = 0; ch < channels; ch++) {
            BYTE *sample_ptr = buffer + (i * channels + ch) * bytes_per_sample;
            
            if (bits_per_sample == 16) {
                int16_t pcm_sample = (int16_t)(sample_value * 32767.0);
                memcpy(sample_ptr, &pcm_sample, sizeof(int16_t));
            } else if (bits_per_sample == 24) {
                int32_t pcm_sample = (int32_t)(sample_value * 8388607.0);
                sample_ptr[0] = (BYTE)(pcm_sample & 0xFF);
                sample_ptr[1] = (BYTE)((pcm_sample >> 8) & 0xFF);
                sample_ptr[2] = (BYTE)((pcm_sample >> 16) & 0xFF);
            } else if (bits_per_sample == 32) {
                int32_t pcm_sample = (int32_t)(sample_value * 2147483647.0);
                memcpy(sample_ptr, &pcm_sample, sizeof(int32_t));
            }
        }
        
        gen->phase += phase_increment;
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

int main(void)
{
    int test_passed = 1;
    
    printf("Windows Format Handling Test\n");
    printf("=============================\n\n");
    
    /* Test 1: Simple WAVE_FORMAT_IEEE_FLOAT */
    printf("Test 1: Detect simple IEEE Float format... ");
    WAVEFORMATEX wfx_float = {0};
    wfx_float.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wfx_float.nChannels = 2;
    wfx_float.nSamplesPerSec = 48000;
    wfx_float.wBitsPerSample = 32;
    
    if (is_format_ieee_float(&wfx_float) && !is_format_pcm(&wfx_float)) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    /* Test 2: Simple WAVE_FORMAT_PCM */
    printf("Test 2: Detect simple PCM format... ");
    WAVEFORMATEX wfx_pcm = {0};
    wfx_pcm.wFormatTag = WAVE_FORMAT_PCM;
    wfx_pcm.nChannels = 2;
    wfx_pcm.nSamplesPerSec = 48000;
    wfx_pcm.wBitsPerSample = 16;
    
    if (is_format_pcm(&wfx_pcm) && !is_format_ieee_float(&wfx_pcm)) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    /* Test 3: WAVEFORMATEXTENSIBLE with IEEE Float SubFormat */
    printf("Test 3: Detect EXTENSIBLE IEEE Float format... ");
    WAVEFORMATEXTENSIBLE wfex_float = {0};
    wfex_float.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wfex_float.Format.nChannels = 2;
    wfex_float.Format.nSamplesPerSec = 48000;
    wfex_float.Format.wBitsPerSample = 32;
    wfex_float.Format.cbSize = 22;
    wfex_float.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    
    if (is_format_ieee_float(&wfex_float.Format) && !is_format_pcm(&wfex_float.Format)) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    /* Test 4: WAVEFORMATEXTENSIBLE with PCM SubFormat */
    printf("Test 4: Detect EXTENSIBLE PCM format... ");
    WAVEFORMATEXTENSIBLE wfex_pcm = {0};
    wfex_pcm.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wfex_pcm.Format.nChannels = 2;
    wfex_pcm.Format.nSamplesPerSec = 48000;
    wfex_pcm.Format.wBitsPerSample = 16;
    wfex_pcm.Format.cbSize = 22;
    wfex_pcm.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    
    if (is_format_pcm(&wfex_pcm.Format) && !is_format_ieee_float(&wfex_pcm.Format)) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    /* Test 5: Generate float samples */
    printf("Test 5: Generate float samples... ");
    sine_generator_t gen;
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    
    float float_buffer[100 * 2];  /* 100 frames, 2 channels */
    sine_generator_process_float(&gen, float_buffer, 100, 2);
    
    /* Check that samples are in valid range */
    int float_samples_ok = 1;
    for (int i = 0; i < 100 * 2; i++) {
        if (float_buffer[i] < -1.0f || float_buffer[i] > 1.0f) {
            float_samples_ok = 0;
            break;
        }
    }
    
    if (float_samples_ok) {
        printf("PASS\n");
    } else {
        printf("FAIL (samples out of range)\n");
        test_passed = 0;
    }
    
    /* Test 6: Generate 16-bit PCM samples */
    printf("Test 6: Generate 16-bit PCM samples... ");
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    
    BYTE pcm16_buffer[100 * 2 * 2];  /* 100 frames, 2 channels, 2 bytes per sample */
    sine_generator_process_pcm(&gen, pcm16_buffer, 100, 2, 16);
    
    /* Check that we generated non-zero data */
    int pcm16_ok = 0;
    for (size_t i = 0; i < sizeof(pcm16_buffer); i++) {
        if (pcm16_buffer[i] != 0) {
            pcm16_ok = 1;
            break;
        }
    }
    
    if (pcm16_ok) {
        printf("PASS\n");
    } else {
        printf("FAIL (all zeros)\n");
        test_passed = 0;
    }
    
    /* Test 7: Generate 24-bit PCM samples */
    printf("Test 7: Generate 24-bit PCM samples... ");
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    
    BYTE pcm24_buffer[100 * 2 * 3];  /* 100 frames, 2 channels, 3 bytes per sample */
    sine_generator_process_pcm(&gen, pcm24_buffer, 100, 2, 24);
    
    /* Check that we generated non-zero data */
    int pcm24_ok = 0;
    for (size_t i = 0; i < sizeof(pcm24_buffer); i++) {
        if (pcm24_buffer[i] != 0) {
            pcm24_ok = 1;
            break;
        }
    }
    
    if (pcm24_ok) {
        printf("PASS\n");
    } else {
        printf("FAIL (all zeros)\n");
        test_passed = 0;
    }
    
    /* Test 8: Generate 32-bit PCM samples */
    printf("Test 8: Generate 32-bit PCM samples... ");
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    
    BYTE pcm32_buffer[100 * 2 * 4];  /* 100 frames, 2 channels, 4 bytes per sample */
    sine_generator_process_pcm(&gen, pcm32_buffer, 100, 2, 32);
    
    /* Check that we generated non-zero data */
    int pcm32_ok = 0;
    for (size_t i = 0; i < sizeof(pcm32_buffer); i++) {
        if (pcm32_buffer[i] != 0) {
            pcm32_ok = 1;
            break;
        }
    }
    
    if (pcm32_ok) {
        printf("PASS\n");
    } else {
        printf("FAIL (all zeros)\n");
        test_passed = 0;
    }
    
    printf("\n");
    
    if (test_passed) {
        printf("=== ALL TESTS PASSED ===\n");
        return 0;
    } else {
        printf("=== SOME TESTS FAILED ===\n");
        return 1;
    }
}

#else

#include <stdio.h>

int main(void)
{
    fprintf(stderr, "This test is designed for Windows only.\n");
    return 1;
}

#endif /* _WIN32 */
