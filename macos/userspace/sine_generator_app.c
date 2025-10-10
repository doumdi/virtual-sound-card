/**
 * Sine Wave Generator Application (macOS)
 * 
 * Generates a sine wave and plays it to a CoreAudio device
 * Usage: ./sine_generator_app [frequency] [duration_seconds]
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

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_DURATION 5
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define BUFFER_SIZE 1024

typedef struct {
	double phase;
	double frequency;
	double sample_rate;
	double amplitude;
} sine_generator_t;

typedef struct {
	sine_generator_t generator;
	int frames_remaining;
	int total_frames;
} audio_context_t;

static void sine_generator_init(sine_generator_t *gen, double frequency,
				double sample_rate, double amplitude)
{
	gen->phase = 0.0;
	gen->frequency = frequency;
	gen->sample_rate = sample_rate;
	gen->amplitude = amplitude;
}

static void sine_generator_process_i16(sine_generator_t *gen, int16_t *buffer,
				       size_t num_samples)
{
	double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
	const double scale = 32767.0;

	for (size_t i = 0; i < num_samples; i++) {
		double sample = gen->amplitude * sin(gen->phase);
		buffer[i] = (int16_t)(sample * scale);
		gen->phase += phase_increment;

		if (gen->phase >= 2.0 * M_PI) {
			gen->phase -= 2.0 * M_PI;
		}
	}
}

// Audio callback function
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
	
	if (context->frames_remaining <= 0) {
		// Silence when done
		for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
			memset(ioData->mBuffers[i].mData, 0,
			       ioData->mBuffers[i].mDataByteSize);
		}
		return noErr;
	}
	
	int16_t *buffer = (int16_t *)ioData->mBuffers[0].mData;
	UInt32 frames_to_generate = inNumberFrames;
	
	if (frames_to_generate > (UInt32)context->frames_remaining) {
		frames_to_generate = context->frames_remaining;
	}
	
	// Generate interleaved stereo samples
	for (UInt32 i = 0; i < frames_to_generate; i++) {
		int16_t mono_sample;
		sine_generator_process_i16(&context->generator, &mono_sample, 1);
		// Duplicate for stereo
		buffer[i * CHANNELS] = mono_sample;
		buffer[i * CHANNELS + 1] = mono_sample;
	}
	
	// Fill rest with silence if needed
	if (frames_to_generate < inNumberFrames) {
		size_t silence_start = frames_to_generate * CHANNELS * sizeof(int16_t);
		size_t silence_size = (inNumberFrames - frames_to_generate) * CHANNELS * sizeof(int16_t);
		memset((uint8_t *)buffer + silence_start, 0, silence_size);
	}
	
	context->frames_remaining -= frames_to_generate;
	
	return noErr;
}

int main(int argc, char *argv[])
{
	double frequency = DEFAULT_FREQUENCY;
	int duration = DEFAULT_DURATION;
	OSStatus err;
	AudioComponentInstance audio_unit;
	audio_context_t context;

	/* Parse command line arguments */
	if (argc >= 2) {
		frequency = atof(argv[1]);
		if (frequency <= 0 || frequency > 20000) {
			fprintf(stderr, "Invalid frequency: %.2f Hz\n",
				frequency);
			return 1;
		}
	}
	if (argc >= 3) {
		duration = atoi(argv[2]);
		if (duration <= 0 || duration > 60) {
			fprintf(stderr, "Invalid duration: %d seconds\n",
				duration);
			return 1;
		}
	}

	printf("Sine Wave Generator (macOS)\n");
	printf("============================\n");
	printf("Frequency: %.2f Hz\n", frequency);
	printf("Duration: %d seconds\n", duration);
	printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
	printf("Channels: %d\n", CHANNELS);
	printf("\n");

	// Initialize sine generator
	sine_generator_init(&context.generator, frequency, SAMPLE_RATE, 0.5);
	context.total_frames = duration * SAMPLE_RATE;
	context.frames_remaining = context.total_frames;

	// Find the default output audio unit
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;

	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	if (component == NULL) {
		fprintf(stderr, "Error: Could not find default audio output component\n");
		return 1;
	}

	err = AudioComponentInstanceNew(component, &audio_unit);
	if (err != noErr) {
		fprintf(stderr, "Error: Could not create audio unit instance (error: %d)\n", (int)err);
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

	printf("Playing sine wave...\n");

	// Start playback
	err = AudioOutputUnitStart(audio_unit);
	if (err != noErr) {
		fprintf(stderr, "Error: Could not start audio output (error: %d)\n", (int)err);
		AudioUnitUninitialize(audio_unit);
		AudioComponentInstanceDispose(audio_unit);
		return 1;
	}

	// Monitor progress
	for (int i = 0; i <= duration; i++) {
		sleep(1);
		float progress = (float)(duration - i) / duration * 100.0f;
		if (i < duration) {
			printf("\rProgress: %.1f%%", 100.0f - progress);
			fflush(stdout);
		}
	}

	printf("\rProgress: 100.0%%\n");
	printf("Playback complete!\n");

	// Cleanup
	AudioOutputUnitStop(audio_unit);
	AudioUnitUninitialize(audio_unit);
	AudioComponentInstanceDispose(audio_unit);

	return 0;
}
