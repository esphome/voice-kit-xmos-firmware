// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

// This file intentionally only includes pure generic C constructs to allow compilation and testing by an x86 processor.

#include <stdint.h>
#include <stdbool.h>
#include <xmath/xmath.h>

typedef struct {
    uint32_t total_data_samples;
    uint32_t total_ticks;
}usb_rate_calc_info_t;

usb_rate_calc_info_t determine_USB_audio_rate(uint32_t timestamp,
                                    uint32_t data_length,
                                    uint32_t direction);
void reset_state();
void sof_toggle();
