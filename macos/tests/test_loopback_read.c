/**
 * Test Loopback Read (macOS)
 * 
 * Reads audio from CoreAudio input device and verifies it contains a sine wave
 * This program should be run while sine_generator_app is playing
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define BUFFER_SIZE 1024
#define READ_DURATION 2
#define EXPECTED_FREQUENCY 440.0
#define FREQUENCY_TOLERANCE 5.0

typedef struct {
	int16_t *buffer;
	size_t buffer_size;
	size_t samples_collected;
	size_t target_samples;
} capture_context_t;

/**
 * Simple zero-crossing frequency detector
 */
static double detect_frequency(int16_t *buffer, size_t num_samples,
			       int sample_rate)
{
	int zero_crossings = 0;
	int16_t prev_sample = buffer[0];

	for (size_t i = 1; i < num_samples; i++) {
		int16_t curr_sample = buffer[i];
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
static int check_amplitude(int16_t *buffer, size_t num_samples)
{
	int32_t sum = 0;
	int32_t sum_sq = 0;

	for (size_t i = 0; i < num_samples; i++) {
		sum += buffer[i];
		sum_sq += (int32_t)buffer[i] * buffer[i];
	}

	double mean = (double)sum / num_samples;
	double rms = sqrt((double)sum_sq / num_samples);

	/* RMS should be significant (not silent) */
	if (rms < 1000) {
		fprintf(stderr, "Signal too quiet (RMS: %.2f)\n", rms);
		return 0;
	}

	/* Mean should be close to zero (no DC offset) */
	if (fabs(mean) > 1000) {
		fprintf(stderr, "Signal has DC offset (mean: %.2f)\n", mean);
		return 0;
	}

	printf("Signal amplitude OK (RMS: %.2f, mean: %.2f)\n", rms, mean);
	return 1;
}

// Audio input callback
static OSStatus input_callback(void *inRefCon,
			       AudioUnitRenderActionFlags *ioActionFlags,
			       const AudioTimeStamp *inTimeStamp,
			       UInt32 inBusNumber,
			       UInt32 inNumberFrames,
			       AudioBufferList *ioData)
{
	(void)ioData;
	
	capture_context_t *context = (capture_context_t *)inRefCon;
	OSStatus err;
	
	// Allocate buffer list for rendering
	AudioBufferList bufferList;
	bufferList.mNumberBuffers = 1;
	bufferList.mBuffers[0].mNumberChannels = CHANNELS;
	bufferList.mBuffers[0].mDataByteSize = inNumberFrames * CHANNELS * sizeof(int16_t);
	bufferList.mBuffers[0].mData = malloc(bufferList.mBuffers[0].mDataByteSize);
	
	if (bufferList.mBuffers[0].mData == NULL) {
		return memFullErr;
	}
	
	// Render audio
	err = AudioUnitRender((AudioUnit)inRefCon,
			      ioActionFlags,
			      inTimeStamp,
			      inBusNumber,
			      inNumberFrames,
			      &bufferList);
	
	if (err == noErr) {
		int16_t *samples = (int16_t *)bufferList.mBuffers[0].mData;
		
		// Extract mono (left channel) and store
		for (UInt32 i = 0; i < inNumberFrames && context->samples_collected < context->target_samples; i++) {
			context->buffer[context->samples_collected++] = samples[i * CHANNELS];
		}
	}
	
	free(bufferList.mBuffers[0].mData);
	
	return err;
}

static AudioDeviceID get_default_input_device(void)
{
	AudioDeviceID device_id;
	UInt32 size = sizeof(device_id);
	AudioObjectPropertyAddress property_address = {
		kAudioHardwarePropertyDefaultInputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	
	OSStatus err = AudioObjectGetPropertyData(kAudioObjectSystemObject,
						  &property_address,
						  0,
						  NULL,
						  &size,
						  &device_id);
	
	if (err != noErr) {
		return kAudioDeviceUnknown;
	}
	
	return device_id;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	
	OSStatus err;
	AudioComponentInstance audio_unit;
	capture_context_t context;
	int test_passed = 1;

	printf("Loopback Read Test (macOS)\n");
	printf("===========================\n");
	printf("Reading from virtual sound card...\n");
	printf("Expected frequency: %.2f Hz\n", EXPECTED_FREQUENCY);
	printf("Duration: %d seconds\n", READ_DURATION);
	printf("\n");

	// Allocate capture buffer
	context.target_samples = READ_DURATION * SAMPLE_RATE;
	context.buffer_size = context.target_samples;
	context.samples_collected = 0;
	context.buffer = malloc(context.buffer_size * sizeof(int16_t));
	
	if (!context.buffer) {
		fprintf(stderr, "Error: Could not allocate capture buffer\n");
		return 1;
	}

	// Check if default input device exists
	AudioDeviceID input_device = get_default_input_device();
	if (input_device == kAudioDeviceUnknown) {
		fprintf(stderr, "Error: No input device found\n");
		fprintf(stderr, "Make sure you have an audio input device configured.\n");
		free(context.buffer);
		return 1;
	}

	// Find the default input audio unit
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;

	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	if (component == NULL) {
		fprintf(stderr, "Error: Could not find HAL output component\n");
		free(context.buffer);
		return 1;
	}

	err = AudioComponentInstanceNew(component, &audio_unit);
	if (err != noErr) {
		fprintf(stderr, "Error: Could not create audio unit instance (error: %d)\n", (int)err);
		free(context.buffer);
		return 1;
	}

	// Enable input on the HAL unit
	UInt32 enable_io = 1;
	err = AudioUnitSetProperty(audio_unit,
				   kAudioOutputUnitProperty_EnableIO,
				   kAudioUnitScope_Input,
				   1,
				   &enable_io,
				   sizeof(enable_io));
	if (err != noErr) {
		fprintf(stderr, "Error: Could not enable input (error: %d)\n", (int)err);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	// Disable output on the HAL unit
	enable_io = 0;
	err = AudioUnitSetProperty(audio_unit,
				   kAudioOutputUnitProperty_EnableIO,
				   kAudioUnitScope_Output,
				   0,
				   &enable_io,
				   sizeof(enable_io));
	if (err != noErr) {
		fprintf(stderr, "Error: Could not disable output (error: %d)\n", (int)err);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	// Set audio format
	AudioStreamBasicDescription format;
	format.mSampleRate = SAMPLE_RATE;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mBytesPerPacket = CHANNELS * sizeof(int16_t);
	format.mFramesPerPacket = 1;
	format.mBytesPerFrame = CHANNELS * sizeof(int16_t);
	format.mChannelsPerFrame = CHANNELS;
	format.mBitsPerChannel = 16;

	err = AudioUnitSetProperty(audio_unit,
				   kAudioUnitProperty_StreamFormat,
				   kAudioUnitScope_Output,
				   1,
				   &format,
				   sizeof(format));
	if (err != noErr) {
		fprintf(stderr, "Error: Could not set audio format (error: %d)\n", (int)err);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	// Set input callback
	AURenderCallbackStruct callback;
	callback.inputProc = input_callback;
	callback.inputProcRefCon = &context;

	err = AudioUnitSetProperty(audio_unit,
				   kAudioOutputUnitProperty_SetInputCallback,
				   kAudioUnitScope_Global,
				   0,
				   &callback,
				   sizeof(callback));
	if (err != noErr) {
		fprintf(stderr, "Error: Could not set input callback (error: %d)\n", (int)err);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	// Initialize audio unit
	err = AudioUnitInitialize(audio_unit);
	if (err != noErr) {
		fprintf(stderr, "Error: Could not initialize audio unit (error: %d)\n", (int)err);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	printf("Reading audio...\n");

	// Start capture
	err = AudioOutputUnitStart(audio_unit);
	if (err != noErr) {
		fprintf(stderr, "Error: Could not start audio input (error: %d)\n", (int)err);
		AudioUnitUninitialize(audio_unit);
		AudioComponentInstanceDispose(audio_unit);
		free(context.buffer);
		return 1;
	}

	// Monitor progress
	for (int i = 0; i <= READ_DURATION; i++) {
		sleep(1);
		float progress = (float)context.samples_collected / context.target_samples * 100.0f;
		printf("\rProgress: %.1f%%", progress);
		fflush(stdout);
	}

	printf("\rProgress: 100.0%%\n");
	printf("Read complete. Analyzing...\n\n");

	// Stop capture
	AudioOutputUnitStop(audio_unit);
	AudioUnitUninitialize(audio_unit);
	AudioComponentInstanceDispose(audio_unit);

	/* Analyze the captured audio */
	printf("=== Analysis Results ===\n");

	/* Check amplitude */
	if (!check_amplitude(context.buffer, context.samples_collected)) {
		test_passed = 0;
	}

	/* Detect frequency */
	double detected_freq = detect_frequency(context.buffer,
						context.samples_collected,
						SAMPLE_RATE);
	printf("Detected frequency: %.2f Hz\n", detected_freq);

	/* Verify frequency is close to expected */
	double freq_diff = fabs(detected_freq - EXPECTED_FREQUENCY);
	if (freq_diff > FREQUENCY_TOLERANCE) {
		fprintf(stderr,
			"FAIL: Frequency mismatch (expected %.2f ± %.2f Hz, got %.2f Hz)\n",
			EXPECTED_FREQUENCY, FREQUENCY_TOLERANCE, detected_freq);
		test_passed = 0;
	} else {
		printf("PASS: Frequency within tolerance (%.2f Hz)\n",
		       freq_diff);
	}

	printf("\n");

	/* Cleanup */
	free(context.buffer);

	if (test_passed) {
		printf("=== TEST PASSED ===\n");
		return 0;
	} else {
		printf("=== TEST FAILED ===\n");
		return 1;
	}
}
