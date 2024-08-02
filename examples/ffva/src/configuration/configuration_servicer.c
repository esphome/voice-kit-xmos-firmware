#define DEBUG_UNIT CONFIGURATION_SERVICER
#ifndef DEBUG_PRINT_ENABLE_CONFIGURATION_SERVICER
#define DEBUG_PRINT_ENABLE_CONFIGURATION_SERVICER 1
#endif
#include "debug_print.h"

#include <stdio.h>
#include <string.h>
#include <platform.h>
#include <xassert.h>

#include "platform/platform_conf.h"
#include "servicer.h"
#include "configuration_servicer.h"
#include "configuration_common.h"

static uint8_t vnr_value = 0;
static int8_t headphone_volume = 0;
static int8_t lineout_volume = 0;

void configuration_servicer_init(servicer_t *servicer)
{
    // Servicer resource info
    static control_resource_info_t cfg_res_info[NUM_RESOURCES_CONFIGURATION_SERVICER];

    memset(servicer, 0, sizeof(servicer_t));
    servicer->id = CONFIGURATION_SERVICER_RESID;
    servicer->start_io = 0;
    servicer->num_resources = NUM_RESOURCES_CONFIGURATION_SERVICER;

    servicer->res_info = &cfg_res_info[0];
    // Servicer resource
    servicer->res_info[0].resource = CONFIGURATION_SERVICER_RESID;
    servicer->res_info[0].command_map.num_commands = NUM_CONFIGURATION_SERVICER_RESID_CMDS;
    servicer->res_info[0].command_map.commands = configuration_servicer_resid_cmd_map;
}

void configuration_servicer(void *args) {
    device_control_servicer_t servicer_ctx;

    servicer_t *servicer = (servicer_t*)args;
    xassert(servicer != NULL);

    control_resid_t *resources = (control_resid_t*)pvPortMalloc(servicer->num_resources * sizeof(control_resid_t));
    for(int i=0; i<servicer->num_resources; i++)
    {
        resources[i] = servicer->res_info[i].resource;
    }

    control_ret_t dc_ret;
    // rtos_printf("Calling device_control_servicer_register(), servicer ID %d, on tile %d, core %d.\n", servicer->id, THIS_XCORE_TILE, rtos_core_id_get());

    dc_ret = device_control_servicer_register(&servicer_ctx,
                                            device_control_ctxs,
                                            1,
                                            resources, servicer->num_resources);
    // rtos_printf("Out of device_control_servicer_register(), servicer ID %d, on tile %d. servicer_ctx address = 0x%x\n", servicer->id, THIS_XCORE_TILE, &servicer_ctx);

    vPortFree(resources);

    for(;;){
        device_control_servicer_cmd_recv(&servicer_ctx, configuration_servicer_read_cmd, configuration_servicer_write_cmd, servicer, RTOS_OSAL_WAIT_FOREVER);
    }
}

control_ret_t configuration_servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;
    uint8_t cmd_id = CONTROL_CMD_CLEAR_READ(cmd);
    int32_t value_i32 = 0;
    float value_f = 0.0;

    memset(payload, 0, payload_len);

    // rtos_printf("configuration_servicer_read_cmd, cmd_id: %d.\n", cmd_id);

    // For read commands, payload[0] is reserved from status.
    switch (cmd_id)
    {
        case CONFIGURATION_SERVICER_RESID_VNR_VALUE:
        {
            // rtos_printf("CONFIGURATION_SERVICER_RESID_VNR\n");
            value_i32 = vnr_value;
            payload[0] = 0;
            memcpy(payload + 1, &value_i32, sizeof(value_i32));
        }
        break;

        case CONFIGURATION_SERVICER_RESID_LINEOUT_VOLUME:
        {
            payload[0] = 0;
            payload[1] = 0x11;
        }
        break;

        case CONFIGURATION_SERVICER_RESID_HEADPHONE_VOLUME:
        {
            payload[0] = 0;
            payload[1] = 0x22;
        }
        break;

        default:
        {
            // rtos_printf("CONFIGURATION_SERVICER UNHANDLED COMMAND!!!\n");
            ret = CONTROL_BAD_COMMAND;
            payload[0] = ret;
        }
        break;
    }

    return ret;
}

control_ret_t configuration_servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len)
{
    control_ret_t ret = CONTROL_SUCCESS;

    uint8_t cmd_id = CONTROL_CMD_CLEAR_READ(cmd);
    // rtos_printf("configuration_servicer_write_cmd cmd_id %d.\n", cmd_id);

    switch (cmd_id)
    {
        case CONFIGURATION_SERVICER_RESID_LINEOUT_VOLUME:
        {
            
        }
        break;

        case CONFIGURATION_SERVICER_RESID_HEADPHONE_VOLUME:
        {

        }
        break;
        
        default:
        {
            // rtos_printf("CONFIGURATION_SERVICER UNHANDLED COMMAND!!!\n");
            ret = CONTROL_BAD_COMMAND;
        }
        break;
    }

    return ret;
}

void configuration_set_vnr_value(int value)
{
    if (value > 100) value = 100;
    if (value < 0) value = 0;
    vnr_value = value;
}

void configuration_set_headphone_volume(int8_t value)
{

}

void configuration_get_headphone_volume(int8_t *value)
{
    *value = headphone_volume;
}

void configuration_set_lineout_volume(int8_t value)
{

}

void configuration_get_lineout_volume(int8_t *value)
{
    *value = lineout_volume;
}

void configuration_read(uint8_t zone)
{
    // configuration_t c;

    // configuration_read_from_flash(zone, )
}