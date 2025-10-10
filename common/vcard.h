/**
 * Virtual Sound Card - Common Header
 * 
 * Cross-platform virtual audio device driver
 * Supports Linux, Windows, and macOS
 */

#ifndef VCARD_H
#define VCARD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version Information */
#define VCARD_VERSION_MAJOR 0
#define VCARD_VERSION_MINOR 1
#define VCARD_VERSION_PATCH 0

/* Constants */
#define VCARD_MAX_CHANNELS      32
#define VCARD_MAX_DEVICE_NAME   64
#define VCARD_MAX_DEVICES       16
#define VCARD_MAX_MIDI_PORTS    16
#define VCARD_MAX_ROUTES        128

/* Error Codes */
#define VCARD_SUCCESS           0
#define VCARD_ERROR_INVALID     -1
#define VCARD_ERROR_NO_MEMORY   -2
#define VCARD_ERROR_NOT_FOUND   -3
#define VCARD_ERROR_IN_USE      -4
#define VCARD_ERROR_NO_DEVICE   -5
#define VCARD_ERROR_PERMISSION  -6
#define VCARD_ERROR_TIMEOUT     -7
#define VCARD_ERROR_IO          -8

/* Sample Rates (Hz) */
typedef enum {
    VCARD_RATE_44100  = 44100,
    VCARD_RATE_48000  = 48000,
    VCARD_RATE_88200  = 88200,
    VCARD_RATE_96000  = 96000,
    VCARD_RATE_176400 = 176400,
    VCARD_RATE_192000 = 192000
} vcard_sample_rate_t;

/* Bit Depths */
typedef enum {
    VCARD_BIT_16 = 16,
    VCARD_BIT_24 = 24,
    VCARD_BIT_32 = 32
} vcard_bit_depth_t;

/* Device Configuration */
typedef struct {
    char name[VCARD_MAX_DEVICE_NAME];   /* Device name */
    uint32_t channels_in;                /* Input channels (1-32) */
    uint32_t channels_out;               /* Output channels (1-32) */
    uint32_t sample_rate;                /* Sample rate in Hz */
    uint32_t buffer_size;                /* Buffer size in frames */
    uint32_t bit_depth;                  /* Bit depth (16, 24, 32) */
    uint32_t midi_ports_in;              /* MIDI input ports (0-16) */
    uint32_t midi_ports_out;             /* MIDI output ports (0-16) */
} vcard_config_t;

/* Device Information */
typedef struct {
    int device_id;                       /* Device ID */
    char name[VCARD_MAX_DEVICE_NAME];   /* Device name */
    bool is_active;                      /* Device is active */
    uint32_t channels_in;                /* Input channels */
    uint32_t channels_out;               /* Output channels */
    uint32_t sample_rate;                /* Sample rate */
} vcard_device_info_t;

/* Device Status */
typedef struct {
    bool is_active;                      /* Device is active */
    uint32_t sample_rate;                /* Current sample rate */
    uint32_t buffer_size;                /* Current buffer size */
    uint64_t frames_processed;           /* Total frames processed */
    uint32_t xruns;                      /* Buffer over/underrun count */
    float cpu_load;                      /* CPU usage percentage */
    uint32_t latency_us;                 /* Current latency in microseconds */
} vcard_status_t;

/* Audio Routing */
typedef struct {
    int num_routes;                      /* Number of routes */
    struct {
        uint32_t source_channel;         /* Source channel index */
        uint32_t dest_channel;           /* Destination channel index */
        float gain;                       /* Gain multiplier (0.0-1.0) */
    } routes[VCARD_MAX_ROUTES];
} vcard_routing_t;

/* MIDI Direction */
typedef enum {
    VCARD_MIDI_INPUT  = 0,
    VCARD_MIDI_OUTPUT = 1
} vcard_midi_direction_t;

/* Opaque Handle Types */
typedef void* vcard_midi_handle_t;

/* Callback Types */
typedef void (*vcard_status_callback_t)(int device_id, 
                                        const vcard_status_t *status,
                                        void *user_data);

typedef void (*vcard_midi_callback_t)(vcard_midi_handle_t handle,
                                      const uint8_t *message,
                                      size_t length,
                                      void *user_data);

/* API Functions */

/**
 * Initialize the Virtual Sound Card library
 * @return 0 on success, error code on failure
 */
int vcard_init(void);

/**
 * Cleanup the Virtual Sound Card library
 */
void vcard_cleanup(void);

/**
 * Get library version
 * @param major Major version number
 * @param minor Minor version number
 * @param patch Patch version number
 */
void vcard_get_version(int *major, int *minor, int *patch);

/* Device Management */

/**
 * Create a new virtual audio device
 * @param config Device configuration
 * @param device_id Output parameter for created device ID
 * @return 0 on success, error code on failure
 */
int vcard_create_device(const vcard_config_t *config, int *device_id);

/**
 * Destroy a virtual audio device
 * @param device_id Device ID to destroy
 * @return 0 on success, error code on failure
 */
int vcard_destroy_device(int device_id);

/**
 * List all virtual audio devices
 * @param devices Array to store device information
 * @param max_devices Maximum number of devices to return
 * @param count Output parameter for actual number of devices
 * @return 0 on success, error code on failure
 */
int vcard_list_devices(vcard_device_info_t *devices, 
                       int max_devices, 
                       int *count);

/* Device Configuration */

/**
 * Get current device configuration
 * @param device_id Device ID
 * @param config Output parameter for configuration
 * @return 0 on success, error code on failure
 */
int vcard_get_config(int device_id, vcard_config_t *config);

/**
 * Update device configuration
 * @param device_id Device ID
 * @param config New configuration
 * @return 0 on success, error code on failure
 */
int vcard_set_config(int device_id, const vcard_config_t *config);

/* Audio Routing */

/**
 * Configure audio routing between channels
 * @param device_id Device ID
 * @param routing Routing configuration
 * @return 0 on success, error code on failure
 */
int vcard_set_routing(int device_id, const vcard_routing_t *routing);

/**
 * Get current audio routing configuration
 * @param device_id Device ID
 * @param routing Output parameter for routing configuration
 * @return 0 on success, error code on failure
 */
int vcard_get_routing(int device_id, vcard_routing_t *routing);

/* Status and Monitoring */

/**
 * Get current device status
 * @param device_id Device ID
 * @param status Output parameter for status
 * @return 0 on success, error code on failure
 */
int vcard_get_status(int device_id, vcard_status_t *status);

/**
 * Register callback for status changes
 * @param device_id Device ID
 * @param callback Callback function
 * @param user_data User data passed to callback
 * @return 0 on success, error code on failure
 */
int vcard_set_status_callback(int device_id, 
                               vcard_status_callback_t callback,
                               void *user_data);

/* MIDI API */

/**
 * Open a MIDI port
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

/**
 * Close a MIDI port
 * @param midi_handle MIDI handle
 * @return 0 on success, error code on failure
 */
int vcard_midi_close(vcard_midi_handle_t midi_handle);

/**
 * Send a MIDI message
 * @param midi_handle MIDI handle
 * @param message MIDI message data
 * @param length Message length in bytes
 * @return 0 on success, error code on failure
 */
int vcard_midi_send(vcard_midi_handle_t midi_handle,
                    const uint8_t *message,
                    size_t length);

/**
 * Receive a MIDI message (non-blocking)
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

/**
 * Register callback for MIDI messages
 * @param midi_handle MIDI handle
 * @param callback Callback function
 * @param user_data User data passed to callback
 * @return 0 on success, error code on failure
 */
int vcard_midi_set_callback(vcard_midi_handle_t midi_handle,
                             vcard_midi_callback_t callback,
                             void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* VCARD_H */
