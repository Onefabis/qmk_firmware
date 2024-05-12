#pragma once

#include "quantum.h"
#include "pointing_device.h"
typedef union {
    uint32_t raw;
    struct {
        uint8_t dpi_config;
    };
} keyboard_config_t;

extern keyboard_config_t keyboard_config;

enum userspace_custom_keycodes {                                // ಠ_ಠ
    NEW_SAFE_RANGE                            // use "NEWPLACEHOLDER for keymap specific codes
};


enum tractyl_keycodes {
    KC_ACCEL = NEW_SAFE_RANGE,
};

enum ploopy_keycodes {
    DPI_CONFIG = SAFE_RANGE,
    KEYMAP_SAFE_RANGE,
};

typedef struct {
    uint16_t device_cpi;
} kb_config_data_t;

__attribute__((aligned(16))) typedef struct {
    int8_t x;
    int8_t y;
} kb_mouse_report_t;

extern kb_mouse_report_t sync_mouse_report;

void process_mouse(void);
//void process_mouse_user(report_mouse_t* mouse_report, int8_t x, int8_t y);
void trackball_set_cpi(uint16_t cpi);


