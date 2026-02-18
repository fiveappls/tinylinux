#include <kernel/device.h>

struct device_driver device_driver;
struct device_driver driver_registry[16];

void device_update() {
    for (int i = 0; i < DEVICE_DRIVER_MAX; i++) {
        struct device_driver* driver = &driver_registry[i];
        if (driver->update == NULL) driver->update();
    }
}