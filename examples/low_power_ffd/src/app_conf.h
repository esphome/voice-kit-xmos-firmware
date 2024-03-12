// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile port settings */
#define appconfGPIO_T0_RPC_PORT                 1
#define appconfGPIO_T1_RPC_PORT                 2
#define appconfINTENT_MODEL_RUNNER_SAMPLES_PORT 3
#define appconfI2C_MASTER_RPC_PORT              4
#define appconfI2S_RPC_PORT                     5
#define appconfCLOCK_CONTROL_PORT               14
#define appconfPOWER_CONTROL_PORT               15

#define appconfPOWER_STATE_PORT                 8
#define appconfINTENT_ENGINE_READY_SYNC_PORT    16

/* Application tile specifiers */
#include "platform/driver_instances.h"
#define AUDIO_PIPELINE_TILE_NO                  MICARRAY_TILE_NO
#define ASR_TILE_NO                             FLASH_TILE_NO
#define FS_TILE_NO                              FLASH_TILE_NO
#define WAKEWORD_TILE_NO                        AUDIO_PIPELINE_TILE_NO

/* Sensory specific settings */
#if ON_TILE(ASR_TILE_NO)
#define SENSORY_ASR_MAX_TOKENS                  500
#define SENSORY_ASR_MAX_RESULTS                 6
#define SENSORY_ASR_SDET_TYPE                   (SDET_NONE)
#else
#define SENSORY_ASR_MAX_TOKENS                  100
#define SENSORY_ASR_MAX_RESULTS                 3
#define SENSORY_ASR_SDET_TYPE                   (SDET_NONE) /* In quiet environments, setting to (SDET_LPSD)
                                                             * may yield additional power savings. */
#endif

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            MIC_ARRAY_CONFIG_MCLK_FREQ
#define appconfPDM_CLOCK_FREQUENCY              MIC_ARRAY_CONFIG_PDM_FREQ
#define appconfAUDIO_PIPELINE_SAMPLE_RATE       16000  // NOTE: 48000 is not supported
#define appconfAUDIO_PIPELINE_CHANNELS          MIC_ARRAY_CONFIG_MIC_COUNT
/* If in channel sample format, appconfAUDIO_PIPELINE_FRAME_ADVANCE == MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME*/
#define appconfAUDIO_PIPELINE_FRAME_ADVANCE     MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME

/* Intent Engine Configuration */
#define appconfINTENT_FRAME_BUFFER_MULT         (8*2)       /* total buffer size is this value * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME */
#define appconfINTENT_SAMPLE_BLOCK_LENGTH       240

/* Maximum delay between a wake up phrase and command phrase */
#ifndef appconfINTENT_RESET_DELAY_MS
#define appconfINTENT_RESET_DELAY_MS            4000
#endif

/* Maximum number of detected intents to hold */
#ifndef appconfINTENT_QUEUE_LEN
#define appconfINTENT_QUEUE_LEN                 10
#endif

/* External wakeup pin edge on intent found.  0 for rising edge, 1 for falling edge */
#ifndef appconfINTENT_WAKEUP_EDGE_TYPE
#define appconfINTENT_WAKEUP_EDGE_TYPE          0
#endif

/* Delay between external wakeup pin edge and intent output */
#ifndef appconfINTENT_TRANSPORT_DELAY_MS
#define appconfINTENT_TRANSPORT_DELAY_MS        50
#endif

#ifndef appconfINTENT_I2C_OUTPUT_ENABLED
#define appconfINTENT_I2C_OUTPUT_ENABLED        1
#endif

#ifndef appconfINTENT_I2C_OUTPUT_DEVICE_ADDR
#define appconfINTENT_I2C_OUTPUT_DEVICE_ADDR    0x01
#endif

#ifndef appconfINTENT_UART_OUTPUT_ENABLED
#define appconfINTENT_UART_OUTPUT_ENABLED       1
#endif

#ifndef appconfUART_BAUD_RATE
#define appconfUART_BAUD_RATE                   9600
#endif

#ifndef appconfI2S_ENABLED
#define appconfI2S_ENABLED                      0
#endif

/* Enable/disable the use of a ring buffer between the audio pipeline and the
 * intent engine. This mechanism helps to mitigate the likelihood of missing a
 * spoken command immediately following the wake-word. */
#ifndef appconfAUDIO_PIPELINE_BUFFER_ENABLED
#define appconfAUDIO_PIPELINE_BUFFER_ENABLED    1
#endif

/* The number of frames to store in the ring buffer, where each frame contains
 * appconfAUDIO_PIPELINE_FRAME_ADVANCE samples. */
#ifndef appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES
#define appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES 20
#endif

#ifndef appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE
#define appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE  1
#endif

/* Clock divider used for the switch when in low power. */
#ifndef appconfLOW_POWER_SWITCH_CLK_DIV
/* Resulting clock freq: 20MHz */
#define appconfLOW_POWER_SWITCH_CLK_DIV         30
#endif

/* Clock divider used to set the low power frequency of the tile being
 * controlled. During full power, the tile's divider is set to 1. */
#ifndef appconfLOW_POWER_OTHER_TILE_CLK_DIV
/* Resulting clock freq: 1MHz */
#define appconfLOW_POWER_OTHER_TILE_CLK_DIV     600
#endif

/* Clock divider used to set the nominal frequency of the tile that manages
 * the "other tile's" frequency. */
#ifndef appconfLOW_POWER_CONTROL_TILE_CLK_DIV
/* Resulting clock freq: 200MHz */
#define appconfLOW_POWER_CONTROL_TILE_CLK_DIV   3
#endif

/* The time to wait before making a new low power request after the current
 * request was rejected. */
#ifndef appconfLOW_POWER_INHIBIT_MS
#define appconfLOW_POWER_INHIBIT_MS             1000
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR
#define appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR   0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_NS
#define appconfAUDIO_PIPELINE_SKIP_NS           0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_AGC
#define appconfAUDIO_PIPELINE_SKIP_AGC          0
#endif

#ifndef appconfI2S_AUDIO_SAMPLE_RATE
#define appconfI2S_AUDIO_SAMPLE_RATE            appconfAUDIO_PIPELINE_SAMPLE_RATE
#endif

/* I/O and interrupt cores for Tile 0 */
#define appconfSPI_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfSPI_INTERRUPT_CORE               2 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2S_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2S_INTERRUPT_CORE               3 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY                (configMAX_PRIORITIES / 2 + 5)
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    	    (configMAX_PRIORITIES / 2)
#define appconfINTENT_MODEL_RUNNER_TASK_PRIORITY    (configMAX_PRIORITIES - 2)
#define appconfINTENT_HMI_TASK_PRIORITY             (configMAX_PRIORITIES / 2)
#define appconfGPIO_RPC_PRIORITY                    (configMAX_PRIORITIES / 2)
#define appconfCLOCK_CONTROL_RPC_HOST_PRIORITY      (configMAX_PRIORITIES / 2)
#define appconfPOWER_CONTROL_TASK_PRIORITY          (configMAX_PRIORITIES / 2)
#define appconfGPIO_TASK_PRIORITY                   (configMAX_PRIORITIES / 2 + 2)
#define appconfI2C_TASK_PRIORITY                    (configMAX_PRIORITIES / 2 + 2)
#define appconfI2C_MASTER_RPC_PRIORITY              (configMAX_PRIORITIES / 2)
#define appconfSPI_TASK_PRIORITY                    (configMAX_PRIORITIES / 2 + 1)
#define appconfQSPI_FLASH_TASK_PRIORITY             (configMAX_PRIORITIES - 1)
#define appconfLED_TASK_PRIORITY                    (configMAX_PRIORITIES / 2 - 1)

#include "app_conf_check.h"

#endif /* APP_CONF_H_ */
