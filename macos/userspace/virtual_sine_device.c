/**
 * Virtual Sine Wave Device for macOS
 * 
 * Generates a continuous sine wave that outputs to a specified audio device.
 * This program is designed to be used with a virtual audio device like BlackHole,
 * allowing other applications to read the sine wave as an input source.
 * 
 * Usage: ./virtual_sine_device [options]
 *   -f <frequency>   Sine wave frequency in Hz (default: 440.0)
 *   -d <device>      Output device name (default: system default)
 *   -r <rate>        Sample rate in Hz (default: 48000)
 *   -c <channels>    Number of channels (default: 2)
 *   -a <amplitude>   Amplitude 0.0-1.0 (default: 0.5)
 *   -h               Show this help message
 * 
 * Examples:
 *   ./virtual_sine_device -f 440 -d "BlackHole 2ch"
 *   ./virtual_sine_device -f 880 -a 0.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_CHANNELS 2
#define DEFAULT_AMPLITUDE 0.5
#define BUFFER_SIZE 512

typedef struct {
    double phase;
    double frequency;
    double sample_rate;
    double amplitude;
} sine_generator_t;

typedef struct {
    sine_generator_t generator;
    int channels;
} audio_context_t;

static volatile int g_running = 1;

static void sine_generator_init(sine_generator_t *gen, double frequency,
                                double sample_rate, double amplitude)
{
    gen->phase = 0.0;
    gen->frequency = frequency;
    gen->sample_rate = sample_rate;
    gen->amplitude = amplitude;
}

static void sine_generator_process_f32(sine_generator_t *gen, float *buffer,
                                       size_t num_samples)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    
    for (size_t i = 0; i < num_samples; i++) {
        buffer[i] = (float)(gen->amplitude * sin(gen->phase));
        gen->phase += phase_increment;
        
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

static void signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
    printf("\nShutting down...\n");
}

// Audio callback function - generates sine wave
static OSStatus audio_callback(void *inRefCon,
                               AudioUnitRenderActionFlags *ioActionFlags,
                               const AudioTimeStamp *inTimeStamp,
                               UInt32 inBusNumber,
                               UInt32 inNumberFrames,
                               AudioBufferList *ioData)
{
    (void)ioActionFlags;
    (void)inTimeStamp;
    (void)inBusNumber;
    
    audio_context_t *context = (audio_context_t *)inRefCon;
    
    if (!ioData || ioData->mNumberBuffers == 0) {
        return noErr;
    }
    
    float *buffer = (float *)ioData->mBuffers[0].mData;
    
    // Generate mono samples and duplicate for all channels
    for (UInt32 i = 0; i < inNumberFrames; i++) {
        float mono_sample;
        sine_generator_process_f32(&context->generator, &mono_sample, 1);
        
        // Duplicate for all channels
        for (int ch = 0; ch < context->channels; ch++) {
            buffer[i * context->channels + ch] = mono_sample;
        }
    }
    
    return noErr;
}

static AudioDeviceID find_device_by_name(const char *device_name)
{
    AudioObjectPropertyAddress property_address = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    UInt32 size = 0;
    OSStatus err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                   &property_address,
                                                   0,
                                                   NULL,
                                                   &size);
    if (err != noErr) {
        return kAudioDeviceUnknown;
    }
    
    int device_count = size / sizeof(AudioDeviceID);
    AudioDeviceID *devices = (AudioDeviceID *)malloc(size);
    
    err = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                     &property_address,
                                     0,
                                     NULL,
                                     &size,
                                     devices);
    if (err != noErr) {
        free(devices);
        return kAudioDeviceUnknown;
    }
    
    AudioDeviceID found_device = kAudioDeviceUnknown;
    property_address.mSelector = kAudioDevicePropertyDeviceName;
    
    for (int i = 0; i < device_count; i++) {
        char name[256];
        size = sizeof(name);
        
        err = AudioObjectGetPropertyData(devices[i],
                                        &property_address,
                                        0,
                                        NULL,
                                        &size,
                                        name);
        
        if (err == noErr && strcmp(name, device_name) == 0) {
            found_device = devices[i];
            break;
        }
    }
    
    free(devices);
    return found_device;
}

static void list_audio_devices(void)
{
    AudioObjectPropertyAddress property_address = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    UInt32 size = 0;
    OSStatus err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                   &property_address,
                                                   0,
                                                   NULL,
                                                   &size);
    if (err != noErr) {
        fprintf(stderr, "Error getting device list size\n");
        return;
    }
    
    int device_count = size / sizeof(AudioDeviceID);
    AudioDeviceID *devices = (AudioDeviceID *)malloc(size);
    
    err = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                     &property_address,
                                     0,
                                     NULL,
                                     &size,
                                     devices);
    if (err != noErr) {
        fprintf(stderr, "Error getting device list\n");
        free(devices);
        return;
    }
    
    printf("\nAvailable audio output devices:\n");
    printf("================================\n");
    
    property_address.mSelector = kAudioDevicePropertyDeviceName;
    
    for (int i = 0; i < device_count; i++) {
        char name[256];
        size = sizeof(name);
        
        err = AudioObjectGetPropertyData(devices[i],
                                        &property_address,
                                        0,
                                        NULL,
                                        &size,
                                        name);
        
        if (err == noErr) {
            printf("  %d: %s\n", i + 1, name);
        }
    }
    
    printf("\n");
    free(devices);
}

static void print_usage(const char *program_name)
{
    printf("Usage: %s [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  -f <frequency>   Sine wave frequency in Hz (default: %.1f)\n", DEFAULT_FREQUENCY);
    printf("  -d <device>      Output device name\n");
    printf("  -r <rate>        Sample rate in Hz (default: %d)\n", DEFAULT_SAMPLE_RATE);
    printf("  -c <channels>    Number of channels (default: %d)\n", DEFAULT_CHANNELS);
    printf("  -a <amplitude>   Amplitude 0.0-1.0 (default: %.1f)\n", DEFAULT_AMPLITUDE);
    printf("  -l               List available audio devices\n");
    printf("  -h               Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s -f 440 -d \"BlackHole 2ch\"\n", program_name);
    printf("  %s -f 880 -a 0.3\n", program_name);
    printf("\n");
}

int main(int argc, char *argv[])
{
    double frequency = DEFAULT_FREQUENCY;
    int sample_rate = DEFAULT_SAMPLE_RATE;
    int channels = DEFAULT_CHANNELS;
    double amplitude = DEFAULT_AMPLITUDE;
    char *device_name = NULL;
    int list_devices = 0;
    int opt;
    
    // Parse command line options
    while ((opt = getopt(argc, argv, "f:d:r:c:a:lh")) != -1) {
        switch (opt) {
            case 'f':
                frequency = atof(optarg);
                if (frequency <= 0 || frequency > 20000) {
                    fprintf(stderr, "Invalid frequency: %.2f Hz\n", frequency);
                    return 1;
                }
                break;
            case 'd':
                device_name = optarg;
                break;
            case 'r':
                sample_rate = atoi(optarg);
                if (sample_rate < 8000 || sample_rate > 192000) {
                    fprintf(stderr, "Invalid sample rate: %d Hz\n", sample_rate);
                    return 1;
                }
                break;
            case 'c':
                channels = atoi(optarg);
                if (channels < 1 || channels > 32) {
                    fprintf(stderr, "Invalid channel count: %d\n", channels);
                    return 1;
                }
                break;
            case 'a':
                amplitude = atof(optarg);
                if (amplitude < 0.0 || amplitude > 1.0) {
                    fprintf(stderr, "Invalid amplitude: %.2f (must be 0.0-1.0)\n", amplitude);
                    return 1;
                }
                break;
            case 'l':
                list_devices = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (list_devices) {
        list_audio_devices();
        return 0;
    }
    
    printf("Virtual Sine Wave Device for macOS\n");
    printf("===================================\n");
    printf("Frequency: %.2f Hz\n", frequency);
    printf("Sample Rate: %d Hz\n", sample_rate);
    printf("Channels: %d\n", channels);
    printf("Amplitude: %.2f\n", amplitude);
    
    OSStatus err;
    AudioComponentInstance audio_unit;
    audio_context_t context;
    
    // Initialize sine generator
    sine_generator_init(&context.generator, frequency, sample_rate, amplitude);
    context.channels = channels;
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Find the audio output component
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    
    // If specific device is requested, use HAL output
    if (device_name != NULL) {
        AudioDeviceID device_id = find_device_by_name(device_name);
        if (device_id == kAudioDeviceUnknown) {
            fprintf(stderr, "Error: Device '%s' not found\n", device_name);
            printf("\nUse -l option to list available devices.\n");
            return 1;
        }
        
        printf("Output Device: %s (ID: %u)\n", device_name, device_id);
        desc.componentSubType = kAudioUnitSubType_HALOutput;
    } else {
        printf("Output Device: System Default\n");
    }
    
    printf("\n");
    
    AudioComponent component = AudioComponentFindNext(NULL, &desc);
    if (component == NULL) {
        fprintf(stderr, "Error: Could not find audio output component\n");
        return 1;
    }
    
    err = AudioComponentInstanceNew(component, &audio_unit);
    if (err != noErr) {
        fprintf(stderr, "Error: Could not create audio unit instance (error: %d)\n", (int)err);
        return 1;
    }
    
    // If specific device is requested, set it
    if (device_name != NULL) {
        AudioDeviceID device_id = find_device_by_name(device_name);
        err = AudioUnitSetProperty(audio_unit,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &device_id,
                                  sizeof(device_id));
        if (err != noErr) {
            fprintf(stderr, "Error: Could not set output device (error: %d)\n", (int)err);
            AudioComponentInstanceDispose(audio_unit);
            return 1;
        }
    }
    
    // Set audio format
    AudioStreamBasicDescription format;
    format.mSampleRate = sample_rate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mBytesPerPacket = channels * sizeof(float);
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = channels * sizeof(float);
    format.mChannelsPerFrame = channels;
    format.mBitsPerChannel = 32;
    
    err = AudioUnitSetProperty(audio_unit,
                              kAudioUnitProperty_StreamFormat,
                              kAudioUnitScope_Input,
                              0,
                              &format,
                              sizeof(format));
    if (err != noErr) {
        fprintf(stderr, "Error: Could not set audio format (error: %d)\n", (int)err);
        AudioComponentInstanceDispose(audio_unit);
        return 1;
    }
    
    // Set render callback
    AURenderCallbackStruct callback;
    callback.inputProc = audio_callback;
    callback.inputProcRefCon = &context;
    
    err = AudioUnitSetProperty(audio_unit,
                              kAudioUnitProperty_SetRenderCallback,
                              kAudioUnitScope_Input,
                              0,
                              &callback,
                              sizeof(callback));
    if (err != noErr) {
        fprintf(stderr, "Error: Could not set render callback (error: %d)\n", (int)err);
        AudioComponentInstanceDispose(audio_unit);
        return 1;
    }
    
    // Initialize audio unit
    err = AudioUnitInitialize(audio_unit);
    if (err != noErr) {
        fprintf(stderr, "Error: Could not initialize audio unit (error: %d)\n", (int)err);
        AudioComponentInstanceDispose(audio_unit);
        return 1;
    }
    
    printf("Starting sine wave generation...\n");
    printf("Press Ctrl+C to stop\n\n");
    
    // Start playback
    err = AudioOutputUnitStart(audio_unit);
    if (err != noErr) {
        fprintf(stderr, "Error: Could not start audio output (error: %d)\n", (int)err);
        AudioUnitUninitialize(audio_unit);
        AudioComponentInstanceDispose(audio_unit);
        return 1;
    }
    
    printf("Virtual sine wave device is running.\n");
    if (device_name != NULL) {
        printf("Outputting to: %s\n", device_name);
        printf("\nOther applications can now read from '%s' as an input device.\n", device_name);
    } else {
        printf("Outputting to: System Default Output\n");
        printf("\nTo use as a virtual device, specify a loopback device like BlackHole:\n");
        printf("  %s -d \"BlackHole 2ch\" -f %.0f\n", argv[0], frequency);
    }
    
    // Keep running
    while (g_running) {
        sleep(1);
    }
    
    // Cleanup
    printf("\nStopping...\n");
    AudioOutputUnitStop(audio_unit);
    AudioUnitUninitialize(audio_unit);
    AudioComponentInstanceDispose(audio_unit);
    
    printf("Stopped.\n");
    return 0;
}
