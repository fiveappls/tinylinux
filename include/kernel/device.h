#pragma once

#include <stdint.h>
#include <stddef.h>

#define DEVICE_DRIVER_MAX 16
#define DEVICE_MAX_REQ DEVICE_DRIVER_MAX
#define DEVICE_MAX_REQ_MSK DEVICE_MAX_REQ-1

#define TTY_BUFFER_SIZE 256

struct device {
    struct device_ops* ops;
    struct device_request *request_queue[DEVICE_MAX_REQ];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    char buffer[TTY_BUFFER_SIZE];
};

struct device_request {
    void* buffer;
    uint16_t count;
    uint8_t index;
    uint8_t operation;
    uint32_t offset : 24;
    uint8_t state : 8;
};

struct device_ops {
    void (*update)(struct device*);
    void (*ioctl)(struct device*, int, void*);
};

extern struct device_driver {
    const char* name;
    struct device* (*create)(int8_t* minor, const void* args);
    void (*destroy)(uint8_t minor);
    struct device* (*lookup)(uint8_t minor);
    void (*update)();
} device_driver;

enum {
    DEVICE_OP_RD = 0,
    DEVICE_OP_WR = 1,
    DEVICE_STATE_FINISHED = 255,
    DEVICE_STATE_PENDING  = 0
};

extern struct device_driver driver_registry[DEVICE_DRIVER_MAX];

void device_update();

static inline struct device_request* device_queue_pop(struct device* dev)
{
    if (dev->count == 0) return NULL;
    
    struct device_request* req = dev->request_queue[dev->head];
    dev->head = (dev->head + 1) & DEVICE_MAX_REQ_MSK;
    dev->count--;
    
    return req;
}