#pragma once

#include "servicer.h"

#define CONFIGURATION_SERVICER_RESID   (241)
#define NUM_RESOURCES_CONFIGURATION_SERVICER     (1) // Configuration servicer

#define CONFIGURATION_SERVICER_RESID_VNR 0x80
#define NUM_CONFIGURATION_SERVICER_RESID_CMDS 1

static control_cmd_info_t configuration_servicer_resid_cmd_map[] =
{
    { CONFIGURATION_SERVICER_RESID_VNR, 1, sizeof(uint8_t), CMD_READ_ONLY },
};


void configuration_servicer(void *args);

void configuration_servicer_init(servicer_t *servicer);

control_ret_t configuration_servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len);

control_ret_t configuration_servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len);

void configuration_set_vnr_value(int value);