#include <kernel/device.h>
#include <kernel/kpanic.h>
#include <kernel/uart.h>
#include <kernel/tty.h>
#include <stddef.h>

#define TTY_BUFFER_SIZE 256
#define MAX_TTY_COUNT 4

const struct device_ops tty_ops = {
    .update = &tty_update
};

struct tty_device tty_table[MAX_TTY_COUNT];
struct tty_device* current_tty = NULL;

void tty_init(void)
{
    struct device_driver* tty_driver = &driver_registry[0];
    tty_driver->name = "tty";
    tty_driver->create =  &tty_create;
    tty_driver->destroy = &tty_destroy;
    tty_driver->lookup =  &tty_lookup;
    tty_driver->update =  &tty_global_update;

    static struct uart_hardware_interface uart0 = {
        .data = (volatile uint8_t*)0x10000000,
        .status = (volatile uint8_t*)0x10000005
    };

    uint8_t minor;
    current_tty = (struct tty_device*) tty_create(&minor, &uart0);
    if (!current_tty) kpanic("failed to initialize tty");
}

struct device* tty_create(int8_t* minor, const void* args)
{
    for (int8_t i = 0; i < MAX_TTY_COUNT; i++) {
        struct tty_device* tty = &tty_table[i];
        if (tty->base.ops == NULL) {
            tty->base.ops = (struct device_ops*) &tty_ops;
            tty->tty = (struct tty_hardware_interface*) args;
            tty->base.count = 0;
            tty->base.head = 0;
            tty->base.tail = 0;
            *minor = i;
            return &tty->base;
        }
    }
    *minor = -1;
    return NULL;
}

void tty_destroy(uint8_t minor)
{
    struct tty_device* tty = &tty_table[minor];
    tty->base.ops = NULL;
    tty->tty = NULL;
}

struct device* tty_lookup(uint8_t minor)
{
    struct tty_device* tty = &tty_table[minor & MAX_TTY_COUNT-1];
    if (tty->base.ops == NULL) {
        return NULL;
    }
    return &tty->base;
}

void tty_global_update(void) 
{
    for (int i = 0; i < MAX_TTY_COUNT; i++) {
        struct tty_device* tty = &tty_table[i];
        if (tty->base.ops != NULL) {
            tty_update(&tty->base);
        }
    }
}

static inline uint8_t tty_write(
        struct tty_device* tty,
        struct device_request* current_req,
        volatile struct tty_hardware_interface* tty_interface
        )
{
    uint32_t i = tty->current_bytes_copied;
    char c = ((char*) current_req->buffer)[i];

    tty_interface->character = c;
    tty_interface->write = 1;
    tty_interface->cursor_location++;

    tty->current_bytes_copied = ++i;

    if (i == current_req->count) {
        return 1;
    }
    return 0;
}

static inline uint8_t tty_read(struct tty_device* tty, struct device_request* current_req, volatile struct tty_hardware_interface* tty_interface) {
        
    if (tty_interface->input_ready) {
        char c = tty_interface->input_character;
        uint32_t i = tty->current_bytes_copied;

        if (c == '\r' || c == '\n') {
            tty_interface->cursor_location = (tty_interface->cursor_location + 32) & 0b11100000;
            return 1;
        }
        else if (c == 127 || c == 8) {
        	if (!(i >= 1)) return 0;
            ((char*) current_req->buffer)[--i] = 0;
            tty->current_bytes_copied = i;
            tty_interface->character = ' ';
            tty_interface->cursor_location--;
            tty_interface->write = 1;
            return 0;
        }        
        //TODO: add backspace functionality with "\b \b" response to \b, and add raw mode and echo flag

        tty_interface->character = c;
        tty_interface->write = 1;
        tty_interface->cursor_location++;
        
        ((char*) current_req->buffer)[i++] = c;
        tty->current_bytes_copied = i;
        
        if (i == current_req->count) {
            return 1;
        }
    }

    return 0;
}

void tty_putc(struct tty_device* tty, char c)
{
    struct uart_hardware_interface* uart = (struct uart_hardware_interface*) tty->tty;
    uart_putc(uart, c);

    //uint8_t head = tty->base.head;
    //tty->base.buffer[head] = c;
    //tty->base.head = (head + 1) % TTY_BUFFER_SIZE;

    //tty->tty->cursor_location++;

    //volatile struct tty_hardware_interface* iface = tty->tty;
    //iface->character = c;
    //iface->write = 1;
    //iface->cursor_location++;
}

void tty_update(struct device* dev)
{
    struct tty_device* tty = (struct tty_device*) dev;
    volatile struct tty_hardware_interface* tty_interface = tty->tty;
    struct device_request* current_req = tty->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL) return;

        tty->current_req = current_req;
        tty->current_bytes_copied = 0;
    }
    
    uint8_t exit;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = tty_write(tty, current_req, tty_interface);
    } else {
        exit = tty_read(tty, current_req, tty_interface);
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = tty->current_bytes_copied;
        tty->current_req = NULL;
   }
}