# Virtual Sound Card API Documentation

This document describes the programming interfaces for interacting with the Virtual Sound Card driver.

## Overview

The Virtual Sound Card provides APIs at multiple levels:
- **System Audio API**: Standard OS audio APIs (ALSA, WASAPI, CoreAudio)
- **Control API**: Driver-specific configuration and control
- **MIDI API**: MIDI device interaction

## Control API

### Device Management

#### Create Virtual Device

```c
/**
 * Creates a new virtual audio device
 *
 * @param config Device configuration parameters
 * @param device_id Output parameter for created device ID
 * @return 0 on success, error code on failure
 */
int vcard_create_device(const vcard_config_t *config, int *device_id);
```

**Configuration Structure:**

```c
typedef struct {
    char name[64];              // Device name
    uint32_t channels_in;       // Number of input channels (1-32)
    uint32_t channels_out;      // Number of output channels (1-32)
    uint32_t sample_rate;       // Sample rate in Hz
    uint32_t buffer_size;       // Buffer size in frames
    uint32_t bit_depth;         // 16, 24, or 32
    uint32_t midi_ports_in;     // MIDI input ports (0-16)
    uint32_t midi_ports_out;    // MIDI output ports (0-16)
} vcard_config_t;
```

**Example:**

```c
vcard_config_t config = {
    .name = "Virtual Card 1",
    .channels_in = 2,
    .channels_out = 2,
    .sample_rate = 48000,
    .buffer_size = 512,
    .bit_depth = 24,
    .midi_ports_in = 1,
    .midi_ports_out = 1
};

int device_id;
int result = vcard_create_device(&config, &device_id);
if (result == 0) {
    printf("Device created with ID: %d\n", device_id);
}
```

#### Destroy Virtual Device

```c
/**
 * Destroys a virtual audio device
 *
 * @param device_id Device ID to destroy
 * @return 0 on success, error code on failure
 */
int vcard_destroy_device(int device_id);
```

#### List Devices

```c
/**
 * Lists all virtual audio devices
 *
 * @param devices Array to store device information
 * @param max_devices Maximum number of devices to return
 * @param count Output parameter for actual number of devices
 * @return 0 on success, error code on failure
 */
int vcard_list_devices(vcard_device_info_t *devices, 
                       int max_devices, 
                       int *count);
```

### Device Configuration

#### Get Device Configuration

```c
/**
 * Gets current device configuration
 *
 * @param device_id Device ID
 * @param config Output parameter for configuration
 * @return 0 on success, error code on failure
 */
int vcard_get_config(int device_id, vcard_config_t *config);
```

#### Update Device Configuration

```c
/**
 * Updates device configuration (may require device restart)
 *
 * @param device_id Device ID
 * @param config New configuration
 * @return 0 on success, error code on failure
 */
int vcard_set_config(int device_id, const vcard_config_t *config);
```

### Audio Routing

#### Set Routing

```c
/**
 * Configures audio routing between channels
 *
 * @param device_id Device ID
 * @param routing Routing configuration
 * @return 0 on success, error code on failure
 */
int vcard_set_routing(int device_id, const vcard_routing_t *routing);
```

**Routing Structure:**

```c
typedef struct {
    int num_routes;
    struct {
        uint32_t source_channel;    // Source channel index
        uint32_t dest_channel;      // Destination channel index
        float gain;                  // Gain multiplier (0.0-1.0)
    } routes[128];
} vcard_routing_t;
```

### Status and Monitoring

#### Get Device Status

```c
/**
 * Gets current device status
 *
 * @param device_id Device ID
 * @param status Output parameter for status
 * @return 0 on success, error code on failure
 */
int vcard_get_status(int device_id, vcard_status_t *status);
```

**Status Structure:**

```c
typedef struct {
    bool is_active;              // Device is active
    uint32_t sample_rate;        // Current sample rate
    uint32_t buffer_size;        // Current buffer size
    uint64_t frames_processed;   // Total frames processed
    uint32_t xruns;              // Buffer over/underrun count
    float cpu_load;              // CPU usage percentage
    uint32_t latency_us;         // Current latency in microseconds
} vcard_status_t;
```

#### Set Status Callback

```c
/**
 * Registers callback for status changes
 *
 * @param device_id Device ID
 * @param callback Callback function
 * @param user_data User data passed to callback
 * @return 0 on success, error code on failure
 */
typedef void (*vcard_status_callback_t)(int device_id, 
                                        const vcard_status_t *status,
                                        void *user_data);

int vcard_set_status_callback(int device_id, 
                               vcard_status_callback_t callback,
                               void *user_data);
```

## MIDI API

### MIDI Device Management

#### Open MIDI Port

```c
/**
 * Opens a MIDI port for reading or writing
 *
 * @param device_id Audio device ID
 * @param port_index MIDI port index
 * @param direction MIDI_INPUT or MIDI_OUTPUT
 * @param midi_handle Output parameter for MIDI handle
 * @return 0 on success, error code on failure
 */
int vcard_midi_open(int device_id, 
                    int port_index,
                    vcard_midi_direction_t direction,
                    vcard_midi_handle_t *midi_handle);
```

#### Close MIDI Port

```c
/**
 * Closes a MIDI port
 *
 * @param midi_handle MIDI handle
 * @return 0 on success, error code on failure
 */
int vcard_midi_close(vcard_midi_handle_t midi_handle);
```

### MIDI I/O

#### Send MIDI Message

```c
/**
 * Sends a MIDI message
 *
 * @param midi_handle MIDI handle
 * @param message MIDI message data
 * @param length Message length in bytes
 * @return 0 on success, error code on failure
 */
int vcard_midi_send(vcard_midi_handle_t midi_handle,
                    const uint8_t *message,
                    size_t length);
```

#### Receive MIDI Message

```c
/**
 * Receives a MIDI message (non-blocking)
 *
 * @param midi_handle MIDI handle
 * @param message Buffer for MIDI message
 * @param max_length Maximum message length
 * @param length Output parameter for actual message length
 * @return 0 on success, error code on failure
 */
int vcard_midi_receive(vcard_midi_handle_t midi_handle,
                       uint8_t *message,
                       size_t max_length,
                       size_t *length);
```

#### Set MIDI Callback

```c
/**
 * Registers callback for MIDI messages
 *
 * @param midi_handle MIDI handle
 * @param callback Callback function
 * @param user_data User data passed to callback
 * @return 0 on success, error code on failure
 */
typedef void (*vcard_midi_callback_t)(vcard_midi_handle_t handle,
                                      const uint8_t *message,
                                      size_t length,
                                      void *user_data);

int vcard_midi_set_callback(vcard_midi_handle_t midi_handle,
                             vcard_midi_callback_t callback,
                             void *user_data);
```

## Platform-Specific Notes

### Linux

Use standard ALSA API for audio I/O:
```c
snd_pcm_open(&pcm, "hw:VirtualCard", SND_PCM_STREAM_PLAYBACK, 0);
```

Control API available through ALSA control interface or custom ioctl.

### Windows

Use standard WASAPI for audio:
```c
IMMDeviceEnumerator *pEnumerator;
CoCreateInstance(&CLSID_MMDeviceEnumerator, ...);
```

Control API available through COM interface:
```c
IVirtualSoundCard *pVCard;
CoCreateInstance(&CLSID_VirtualSoundCard, ...);
```

### macOS

Use standard CoreAudio API:
```objc
AudioObjectGetPropertyData(deviceID, &propertyAddress, ...);
```

Control API available through IOKit or custom XPC service.

## Error Codes

```c
#define VCARD_SUCCESS           0
#define VCARD_ERROR_INVALID     -1
#define VCARD_ERROR_NO_MEMORY   -2
#define VCARD_ERROR_NOT_FOUND   -3
#define VCARD_ERROR_IN_USE      -4
#define VCARD_ERROR_NO_DEVICE   -5
#define VCARD_ERROR_PERMISSION  -6
#define VCARD_ERROR_TIMEOUT     -7
#define VCARD_ERROR_IO          -8
```

## Example Applications

### Simple Playback Example

```c
#include <vcard/vcard.h>

int main() {
    // Create device
    vcard_config_t config = {
        .name = "My Virtual Card",
        .channels_in = 0,
        .channels_out = 2,
        .sample_rate = 48000,
        .buffer_size = 512,
        .bit_depth = 24
    };
    
    int device_id;
    if (vcard_create_device(&config, &device_id) != 0) {
        fprintf(stderr, "Failed to create device\n");
        return 1;
    }
    
    // Use standard OS audio API to play audio to this device
    // ...
    
    // Cleanup
    vcard_destroy_device(device_id);
    return 0;
}
```

### MIDI Example

```c
#include <vcard/vcard.h>

void midi_callback(vcard_midi_handle_t handle,
                   const uint8_t *message,
                   size_t length,
                   void *user_data) {
    printf("Received MIDI message: ");
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", message[i]);
    }
    printf("\n");
}

int main() {
    // Assume device already created with MIDI ports
    int device_id = 0;
    
    // Open MIDI input port
    vcard_midi_handle_t midi_in;
    vcard_midi_open(device_id, 0, MIDI_INPUT, &midi_in);
    
    // Set callback
    vcard_midi_set_callback(midi_in, midi_callback, NULL);
    
    // Wait for MIDI messages
    sleep(10);
    
    // Cleanup
    vcard_midi_close(midi_in);
    return 0;
}
```

## Thread Safety

- All API functions are thread-safe
- Callbacks may be invoked from different threads
- User must ensure thread-safety in callback implementations

## Performance Tips

1. Use appropriate buffer sizes for your latency requirements
2. Avoid blocking operations in callbacks
3. Reuse device instances when possible
4. Monitor CPU usage and adjust buffer sizes if needed
5. Use direct hardware access when available (ALSA hw:, WASAPI exclusive mode)

## Additional Resources

- [Linux ALSA Programming](http://www.alsa-project.org/alsa-doc/alsa-lib/)
- [Windows WASAPI](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [macOS CoreAudio](https://developer.apple.com/documentation/coreaudio)
