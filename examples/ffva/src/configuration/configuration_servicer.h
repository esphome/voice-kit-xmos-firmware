#pragma once

#include "servicer.h"

#define CONFIGURATION_SERVICER_RESID                    (241)
#define NUM_RESOURCES_CONFIGURATION_SERVICER            (1) // Configuration servicer

#define CONFIGURATION_SERVICER_RESID_VNR_VALUE          0x00
#define CONFIGURATION_SERVICER_RESID_LINEOUT_VOLUME     0x10
#define CONFIGURATION_SERVICER_RESID_HEADPHONE_VOLUME   0x11

#define NUM_CONFIGURATION_SERVICER_RESID_CMDS           3

static control_cmd_info_t configuration_servicer_resid_cmd_map[] =
{
    { CONFIGURATION_SERVICER_RESID_VNR_VALUE, 1, sizeof(uint8_t), CMD_READ_ONLY },
    { CONFIGURATION_SERVICER_RESID_LINEOUT_VOLUME, 1, sizeof(uint8_t), CMD_READ_WRITE },
    { CONFIGURATION_SERVICER_RESID_HEADPHONE_VOLUME, 1, sizeof(uint8_t), CMD_READ_WRITE },
};

typedef struct {
    uint8_t hdr;
    uint8_t resid;
    uint8_t reg;
    uint8_t crc8;
    uint8_t data[4];
} configuration_data_t;

static configuration_data_t configuration_data_map[] = 
{
    { 0xA5, CONFIGURATION_SERVICER_RESID, CONFIGURATION_SERVICER_RESID_LINEOUT_VOLUME, 0, 0, 0, 0, 0},
    { 0xA5, CONFIGURATION_SERVICER_RESID, CONFIGURATION_SERVICER_RESID_HEADPHONE_VOLUME, 0, 0, 0, 0, 0},
};

void configuration_servicer(void *args);

void configuration_servicer_init(servicer_t *servicer);

control_ret_t configuration_servicer_read_cmd(control_resource_info_t *res_info, control_cmd_t cmd, uint8_t *payload, size_t payload_len);

control_ret_t configuration_servicer_write_cmd(control_resource_info_t *res_info, control_cmd_t cmd, const uint8_t *payload, size_t payload_len);

void configuration_set_vnr_value(int value);
void configuration_set_headphone_volume(int8_t value);
void configuration_get_headphone_volume(int8_t *value);
void configuration_set_lineout_volume(int8_t value);
void configuration_get_lineout_volume(int8_t *value);
