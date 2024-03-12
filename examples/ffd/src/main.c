// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"
#include "event_groups.h"

/* Library headers */
#include "rtos_printf.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "audio_pipeline.h"
#include "intent_engine/intent_engine.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
#include "gpio_ctrl/leds.h"
#include "intent_handler/intent_handler.h"

#ifndef MEM_ANALYSIS_ENABLED
#define MEM_ANALYSIS_ENABLED 0
#endif

void audio_pipeline_input(void *input_app_data,
                          int32_t **input_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) input_app_data;

    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     input_audio_frames,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              input_audio_frames,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    rtos_mic_array_rx(mic_array_ctx,
                      input_audio_frames,
                      frame_count,
                      portMAX_DELAY);
}

int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
#if ON_TILE(AUDIO_PIPELINE_TILE_NO) && appconfINTENT_ENABLED
    intent_engine_sample_push((int32_t *)output_audio_frames, frame_count);
#endif // ON_TILE(AUDIO_PIPELINE_TILE_NO) && appconfINTENT_ENABLED

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

#if MEM_ANALYSIS_ENABLED
static void mem_analysis(void)
{
    for (;;) {
        rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
#endif

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(0)
    led_task_create(appconfLED_TASK_PRIORITY, NULL);
#endif

#if ON_TILE(1)
    gpio_gpi_init(gpio_ctx_t0);
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
    // Setup flash low-level mode
    //   NOTE: must call rtos_qspi_flash_fast_read_shutdown_ll to use non low-level mode calls
    rtos_qspi_flash_fast_read_setup_ll(qspi_flash_ctx);
#endif

#if appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO)
    QueueHandle_t q_intent = xQueueCreate(appconfINTENT_QUEUE_LEN, sizeof(int32_t));
    intent_handler_create(appconfINTENT_MODEL_RUNNER_TASK_PRIORITY, q_intent);
    intent_engine_create(appconfINTENT_MODEL_RUNNER_TASK_PRIORITY, q_intent);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
#if appconfINTENT_ENABLED
    // Wait until the intent engine is initialized before starting the
    // audio pipeline.
    intent_engine_ready_sync();
#endif
    audio_pipeline_init(NULL, NULL);
#endif

#if MEM_ANALYSIS_ENABLED
    mem_analysis();
#else
    vTaskSuspend(NULL);
    while(1){;} /* Trap */
#endif
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
