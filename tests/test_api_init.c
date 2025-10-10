/**
 * Test for API Initialization
 */

#include "vcard.h"
#include <stdio.h>

int main(void)
{
    int major, minor, patch;
    int result;
    int passed = 1;
    
    printf("Testing Virtual Sound Card API initialization...\n");
    
    // Test version retrieval
    vcard_get_version(&major, &minor, &patch);
    printf("  Library version: %d.%d.%d\n", major, minor, patch);
    
    if (major != VCARD_VERSION_MAJOR || 
        minor != VCARD_VERSION_MINOR || 
        patch != VCARD_VERSION_PATCH) {
        printf("  FAIL: Version mismatch\n");
        passed = 0;
    } else {
        printf("  PASS: Version matches\n");
    }
    
    // Test initialization
    result = vcard_init();
    if (result != VCARD_SUCCESS) {
        printf("  FAIL: Initialization failed with error %d\n", result);
        passed = 0;
    } else {
        printf("  PASS: Initialization successful\n");
    }
    
    // Test double initialization (should succeed)
    result = vcard_init();
    if (result != VCARD_SUCCESS) {
        printf("  FAIL: Double initialization failed\n");
        passed = 0;
    } else {
        printf("  PASS: Double initialization handled\n");
    }
    
    // Test cleanup
    vcard_cleanup();
    printf("  PASS: Cleanup called\n");
    
    // Test device listing (should return 0 devices in stub implementation)
    vcard_device_info_t devices[VCARD_MAX_DEVICES];
    int count = -1;
    result = vcard_list_devices(devices, VCARD_MAX_DEVICES, &count);
    if (result != VCARD_SUCCESS) {
        printf("  FAIL: Device listing failed\n");
        passed = 0;
    } else if (count != 0) {
        printf("  FAIL: Expected 0 devices, got %d\n", count);
        passed = 0;
    } else {
        printf("  PASS: Device listing returns 0 devices (stub implementation)\n");
    }
    
    printf("\n");
    if (passed) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}
