// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* App headers */
#include "app_conf.h"
#include "gpio_ctrl/leds.h"
#include "platform/driver_instances.h"


#if ON_TILE(0)

#define LED_BLINK_DELAY                 (500 / portTICK_PERIOD_MS)
#define LED_FLICKER_DELAY               (100 / portTICK_PERIOD_MS)
#define TASK_NOTIF_MASK_HBEAT_TIMER     0x00000010
#define TASK_NOTIF_MASK_WAITING         0x00000080
#define TASK_NOTIF_MASK_LISTEN          0x00000100
#define TASK_NOTIF_MASK_END_OF_EVAL      0x00001000

typedef enum led_state {
    LED_OFF,
    LED_ON,
    LED_TOGGLE
} led_state_t;

typedef enum led_color {
    LED_GREEN,
    LED_RED,
    LED_YELLOW, /* RED + GREEN */
} led_color_t;

#if XK_VOICE_L71
#define LED_GREEN_MASK      (1<<5)
#define LED_RED_MASK        (1<<4)
#define LED_YELLOW_MASK     (LED_GREEN_MASK | LED_RED_MASK)

#define gpo_setup()     {                                                   \
    gpo_port = rtos_gpio_port(PORT_GPO);                                    \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);                           \
}

#elif XCOREAI_EXPLORER
/* LED 0 is "green"
 * LED 1 is "red" */
#define LED_GREEN_MASK      (1<<0)
#define LED_RED_MASK        (1<<1)
#define LED_YELLOW_MASK     (LED_GREEN_MASK | LED_RED_MASK)

#define gpo_setup()     {                                      \
    gpo_port = rtos_gpio_port(PORT_LEDS);                      \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);              \
}
#endif

// Exclusively turn on the green LED. If the red LED is on, turn it off.
#define green_led_xon()  {                                                  \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    val = (val & ~LED_GREEN_MASK) | LED_RED_MASK;                           \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val);                         \
}
#define green_led_on()  {                                                   \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~LED_GREEN_MASK);      \
}
#define green_led_off()  {                                                  \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= LED_GREEN_MASK);       \
}
#define green_led_toggle() {                                                \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val ^= LED_GREEN_MASK);       \
}

// Exclusively toggle the green LED. If the red LED is on, turn it off.
#define green_led_xtoggle()  {                                              \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    val = (val ^ LED_GREEN_MASK) | LED_RED_MASK;                            \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val);                         \
}

// Exclusively turn on the red LED. If the green LED is on, turn it off.
#define red_led_xon()  {                                                    \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    val = (val & ~LED_RED_MASK) | LED_GREEN_MASK;                           \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val);                         \
}
#define red_led_on()  {                                                     \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~LED_RED_MASK);        \
}
#define red_led_off()  {                                                    \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= LED_RED_MASK);         \
}
#define red_led_toggle() {                                                  \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val ^= LED_RED_MASK);         \
}

// Exclusively toggle the red LED. If the green LED is on, turn it off.
#define red_led_xtoggle()  {                                                \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    val = (val ^ LED_RED_MASK) | LED_GREEN_MASK;                            \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val);                         \
}

#define yellow_led_on()  {                                                  \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~LED_YELLOW_MASK);     \
}
#define yellow_led_off()  {                                                 \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= LED_YELLOW_MASK);      \
}
#define yellow_led_toggle() {                                               \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);                \
    val = (val & LED_YELLOW_MASK) ? (val & ~LED_YELLOW_MASK) : (val | LED_YELLOW_MASK); \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val);                         \
}

static TaskHandle_t ctx_led_task = NULL;

static void hbeat_tmr_callback(TimerHandle_t pxTimer)
{
    xTaskNotify(ctx_led_task, TASK_NOTIF_MASK_HBEAT_TIMER, eSetBits);
}

static void led_task(void *args)
{
    const uint32_t bits_to_clear_on_entry = 0x00000000UL;
    const uint32_t bits_to_clear_on_exit = 0xFFFFFFFFUL;
    rtos_gpio_port_id_t gpo_port = 0;
    led_state_t state = LED_TOGGLE;
    led_color_t color = LED_GREEN;
    uint32_t notif_value;
    TimerHandle_t tmr_hbeat = xTimerCreate("tmr_hbeat",
                                pdMS_TO_TICKS(LED_BLINK_DELAY), pdTRUE, NULL,
                                hbeat_tmr_callback);

    gpo_setup();
    xTimerStart(tmr_hbeat, 0);

    while (1) {
        xTaskNotifyWait(bits_to_clear_on_entry, bits_to_clear_on_exit,
                        &notif_value, portMAX_DELAY);

        /*
         * Process the notification event data.
         */
        if (notif_value & TASK_NOTIF_MASK_WAITING) {
            // Normal operation (heartbeat).
            color = LED_GREEN;
            state = LED_TOGGLE;
            xTimerStart(tmr_hbeat, 0);
        } else if (notif_value & TASK_NOTIF_MASK_LISTEN) {
            // Wake word detected, listening for command.
            xTimerStop(tmr_hbeat, 0);
            color = LED_YELLOW;
            state = LED_ON;
        } else if (notif_value & TASK_NOTIF_MASK_END_OF_EVAL) {
            color = LED_RED;
            state = LED_TOGGLE;
            xTimerChangePeriod(tmr_hbeat, pdMS_TO_TICKS(LED_FLICKER_DELAY), 0);
            xTimerStart(tmr_hbeat, 0);
        }

        /*
         * Update the LED indication.
         */
        if (color == LED_GREEN) {
            switch (state) {
            case LED_ON:
                green_led_xon();
                break;
            case LED_TOGGLE:
                green_led_xtoggle();
                break;
            case LED_OFF:
            default:
                green_led_off();
                break;
            }
        } else if (color == LED_RED) {
            switch (state) {
            case LED_ON:
                red_led_xon();
                break;
            case LED_TOGGLE:
                red_led_xtoggle();
                break;
            case LED_OFF:
            default:
                red_led_off();
                break;
            }
        } else if (color == LED_YELLOW) {
            switch (state) {
            case LED_ON:
                yellow_led_on();
                break;
            case LED_TOGGLE:
                yellow_led_toggle();
                break;
            case LED_OFF:
            default:
                yellow_led_off();
                break;
            }
        }
    }
}

void led_task_create(unsigned priority, void *args)
{
    xTaskCreate((TaskFunction_t) led_task,
                "led_task",
                RTOS_THREAD_STACK_SIZE(led_task),
                args,
                priority,
                &ctx_led_task);
}

void led_indicate_waiting(void)
{
    xTaskNotify(ctx_led_task, TASK_NOTIF_MASK_WAITING, eSetBits);
}

void led_indicate_listening(void)
{
    xTaskNotify(ctx_led_task, TASK_NOTIF_MASK_LISTEN, eSetBits);
}

void led_indicate_end_of_eval(void)
{
    xTaskNotify(ctx_led_task, TASK_NOTIF_MASK_END_OF_EVAL, eSetBits);
}

#endif /* ON_TILE(0) */