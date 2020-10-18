/**
 * Copyright (C) 2020  Evgeny Zinoviev
 * This file is part of isv <https://github.com/gch1p/isv>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ISV_P18_H
#define ISV_P18_H

#include <stdbool.h>
#include <string.h>
#include "util.h"

#define P18_QUERY_CMDS_ENUM_OFFSET 1000
#define P18_SET_CMDS_ENUM_OFFSET   1100

#define MK_P18_UNPACK_FN_NAME(msg_type)    p18_unpack_ ## msg_type ## _msg
#define MK_P18_PARSE_CB_FN_NAME(msg_type)  p18_parse_cb_ ## msg_type ## _msg
#define MK_P18_MSG_T(msg_type)             p18_ ## msg_type ## _msg_t

#define P18_MSG_T(msg_type)             MK_P18_MSG_T(msg_type)
#define P18_UNPACK_FN_NAME(msg_type)    MK_P18_UNPACK_FN_NAME(msg_type)
#define P18_PARSE_CB_FN_NAME(msg_type)  MK_P18_PARSE_CB_FN_NAME(msg_type)

#define P18_UNPACK_FN(msg_type) \
     P18_MSG_T(msg_type) P18_UNPACK_FN_NAME(msg_type)(const char *data)

#define P18_PARSE_CB_FN(msg_type) \
     static void P18_PARSE_CB_FN_NAME(msg_type)(const char *value, \
                                                size_t value_len, \
                                                unsigned int index, \
                                                void *message_ptr)

#define P18_PARSE_LIST_AND_RETURN(msg_type, exp_items_count) \
    P18_MSG_T(msg_type) m = {0}; \
    p18_parse_list(data, &m, (exp_items_count), P18_PARSE_CB_FN_NAME(msg_type)); \
    return m;

#define P18_EXPECT_LISTITEM_LENGTH(len)     \
    p18_expect_listitem_length((const char *)__func__, index, (len), value_len)

typedef void (*p18_parse_cb_t)(const char *, size_t, unsigned int, void *);

/* ------------------------------------------ */
/* Commands list */

enum p18_query_command {
    P18_QUERY_PROTOCOL_ID = P18_QUERY_CMDS_ENUM_OFFSET,
    P18_QUERY_CURRENT_TIME,
    P18_QUERY_TOTAL_GENERATED,
    P18_QUERY_YEAR_GENERATED,
    P18_QUERY_MONTH_GENERATED,
    P18_QUERY_DAY_GENERATED,
    P18_QUERY_SERIES_NUMBER,
    P18_QUERY_CPU_VERSION,
    P18_QUERY_RATED_INFORMATION,
    P18_QUERY_GENERAL_STATUS,
    P18_QUERY_WORKING_MODE,
    P18_QUERY_FAULTS_WARNINGS,
    P18_QUERY_FLAGS_STATUSES,
    P18_QUERY_DEFAULTS,
    P18_QUERY_MAX_CHARGING_CURRENT_SELECTABLE_VALUES,
    P18_QUERY_MAX_AC_CHARGING_CURRENT_SELECTABLE_VALUES,
    P18_QUERY_PARALLEL_RATED_INFORMATION,
    P18_QUERY_PARALLEL_GENERAL_STATUS,
    P18_QUERY_AC_CHARGE_TIME_BUCKET,
    P18_QUERY_AC_SUPPLY_LOAD_TIME_BUCKET,
};

enum p18_set_command {
    P18_SET_LOADS = P18_SET_CMDS_ENUM_OFFSET,
    P18_SET_FLAG,
    P18_SET_DEFAULTS,
    P18_SET_BAT_MAX_CHARGE_CURRENT,
    P18_SET_BAT_MAX_AC_CHARGE_CURRENT,
    P18_SET_AC_OUTPUT_FREQ,
    P18_SET_BAT_MAX_CHARGE_VOLTAGE,
    P18_SET_AC_OUTPUT_RATED_VOLTAGE,
    P18_SET_OUTPUT_SOURCE_PRIORITY,
    P18_SET_BAT_CHARGING_THRESHOLDS_WHEN_UTILITY_AVAIL, /* Battery re-charging and re-discharing voltage when utility is available */
    P18_SET_CHARGING_SOURCE_PRIORITY,
    P18_SET_SOLAR_POWER_PRIORITY,
    P18_SET_AC_INPUT_VOLTAGE_RANGE,
    P18_SET_BAT_TYPE,
    P18_SET_OUTPUT_MODEL,
    P18_SET_BAT_CUTOFF_VOLTAGE,
    P18_SET_SOLAR_CONFIG,
    P18_SET_CLEAR_GENERATED,
    P18_SET_DATE_TIME,
    P18_SET_AC_CHARGE_TIME_BUCKET,
    P18_SET_AC_SUPPLY_LOAD_TIME_BUCKET,
};

typedef enum {
    P18_BT_AGM     = 0,
    P18_BT_FLOODED = 1,
    P18_BT_USER    = 2,
} p18_battery_type_t;

typedef enum {
    P18_IVR_APPLIANCE = 0,
    P18_IVR_UPS       = 1,
} p18_input_voltage_range_t;

typedef enum {
    P18_OSP_SOLAR_UTILITY_BATTERY = 0,
    P18_OSP_SOLAR_BATTERY_UTILITY = 1,
} p18_output_source_priority_t;

typedef enum {
    P18_CSP_SOLAR_FIRST       = 0,
    P18_CSP_SOLAR_AND_UTILITY = 1,
    P18_CSP_SOLAR_ONLY        = 2,
} p18_charger_source_priority_t;

typedef enum {
    P18_MT_OFF_GRID_TIE = 0,
    P18_MT_GRID_TIE     = 1,
} p18_machine_type_t;

typedef enum {
    P18_TRANSFORMERLESS = 0,
    P18_TRANSFORMER     = 1,
} p18_topology_t;

typedef enum {
    P18_OMS_SINGLE_MODULE           = 0,
    P18_OMS_PARALLEL_OUTPUT         = 1,
    P18_OMS_PHASE1_OF_3PHASE_OUTPUT = 2,
    P18_OMS_PHASE2_OF_3PHASE_OUTPUT = 3,
    P18_OMS_PHASE3_OF_3PHASE_OUTPUT = 4,
} p18_output_model_setting_t;

typedef enum {
    P18_SPP_BATTERY_LOAD_UTILITY = 0,
    P18_SPP_LOAD_BATTERY_UTILITY = 1,
} p18_solar_power_priority_t;

typedef enum {
    P18_MPPT_CS_ABNORMAL    = 0,
    P18_MPPT_CS_NOT_CHARGED = 1,
    P18_MPPT_CS_CHARGED     = 2,
} p18_mppt_charger_status_t;

typedef enum {
    P18_BPD_DONOTHING = 0,
    P18_BPD_CHARGE    = 1,
    P18_BPD_DISCHARGE = 2,
} p18_battery_power_direction_t;

typedef enum {
    P18_DAPD_DONOTHING = 0,
    P18_DAPD_AC_DC     = 1,
    P18_DAPD_DC_AC     = 2,
} p18_dc_ac_power_direction_t;

typedef enum {
    P18_LPD_DONOTHING = 0,
    P18_LPD_INPUT     = 1,
    P18_LPD_OUTPUT    = 2,
} p18_line_power_direction_t;

typedef enum {
    P18_WM_POWER_ON_MODE = 0,
    P18_WM_STANDBY_MODE  = 1,
    P18_WM_BYPASS_MODE   = 2,
    P18_WM_BATTERY_MODE  = 3,
    P18_WM_FAULT_MODE    = 4,
    P18_WM_HYBRID_MODE   = 5,
} p18_working_mode_t;

typedef enum {
    P18_PCS_NON_EXISTENT = 0,
    P18_PCS_EXISTENT     = 1,
} p18_parallel_id_connection_status_t;


/* ------------------------------------------ */
/* Message structs */

typedef struct {
    unsigned int id;
} p18_protocol_id_msg_t;

typedef struct {
    unsigned int year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
} p18_current_time_msg_t;

typedef struct {
    unsigned long kwh;
} p18_total_generated_msg_t;

typedef struct {
    unsigned long kwh;
} p18_year_generated_msg_t;

typedef struct {
    unsigned long kwh;
} p18_month_generated_msg_t;

typedef struct {
    unsigned long kwh;
} p18_day_generated_msg_t;

typedef struct {
    short length;
    char id[32];
} p18_series_number_msg_t;

typedef struct {
    char main_cpu_version[6];
    char slave1_cpu_version[6];
    char slave2_cpu_version[6];
} p18_cpu_version_msg_t;

typedef struct {
    unsigned int ac_input_rating_voltage;         /* unit: 0.1V */
    unsigned int ac_input_rating_current;         /* unit: 0.1A */
    unsigned int ac_output_rating_voltage;        /* unit: 0.1A */
    unsigned int ac_output_rating_freq;           /* unit: 0.1Hz */
    unsigned int ac_output_rating_current;        /* unit: 0.1A */
    unsigned int ac_output_rating_apparent_power; /* unit: VA */
    unsigned int ac_output_rating_active_power;   /* unit: W */
    unsigned int battery_rating_voltage;          /* unit: 0.1V */
    unsigned int battery_recharge_voltage;        /* unit: 0.1V */
    unsigned int battery_redischarge_voltage;     /* unit: 0.1V */
    unsigned int battery_under_voltage;           /* unit: 0.1V */
    unsigned int battery_bulk_voltage;            /* unit: 0.1V */
    unsigned int battery_float_voltage;           /* unit: 0.1V */
    p18_battery_type_t battery_type;
    unsigned int max_ac_charging_current;         /* unit: A */
    unsigned int max_charging_current;            /* unit: A */
    p18_input_voltage_range_t input_voltage_range;
    p18_output_source_priority_t output_source_priority;
    p18_charger_source_priority_t charger_source_priority;
    unsigned int parallel_max_num;
    p18_machine_type_t machine_type;
    p18_topology_t topology;
    p18_output_model_setting_t output_model_setting;
    p18_solar_power_priority_t solar_power_priority;
    char mppt[4];
} p18_rated_information_msg_t;

typedef struct {
    unsigned int grid_voltage;              /* unit: 0.1V */
    unsigned int grid_freq;                 /* unit: 0.1Hz */
    unsigned int ac_output_voltage;         /* unit: 0.1V */
    unsigned int ac_output_freq;            /* unit: 0.1Hz */
    unsigned int ac_output_apparent_power;  /* unit: VA */
    unsigned int ac_output_active_power;    /* unit: W */
    unsigned int output_load_percent;       /* unit: % */
    unsigned int battery_voltage;           /* unit: 0.1V */
    unsigned int battery_voltage_scc;       /* unit: 0.1V */
    unsigned int battery_voltage_scc2;      /* unit: 0.1V */
    unsigned int battery_discharge_current; /* unit: A */
    unsigned int battery_charging_current;  /* unit: A */
    unsigned int battery_capacity;          /* unit: % */
    unsigned int inverter_heat_sink_temp;   /* unit: C */
    unsigned int mppt1_charger_temp;        /* unit: C */
    unsigned int mppt2_charger_temp;        /* unit: C */
    unsigned int pv1_input_power;           /* unit: W */
    unsigned int pv2_input_power;           /* unit: W */
    unsigned int pv1_input_voltage;         /* unit: 0.1V */
    unsigned int pv2_input_voltage;         /* unit: 0.1V */
    bool settings_values_changed;           /* inverter returns:
                                               0: nothing changed
                                               1: something changed */
    p18_mppt_charger_status_t mppt1_charger_status;
    p18_mppt_charger_status_t mppt2_charger_status;
    bool load_connected;                    /* inverter returns:
                                               0: disconnected
                                               1: connected */
    p18_battery_power_direction_t battery_power_direction;
    p18_dc_ac_power_direction_t dc_ac_power_direction;
    p18_line_power_direction_t line_power_direction;
    unsigned int local_parallel_id;         /* 0 .. (parallel number - 1) */
} p18_general_status_msg_t;

typedef struct {
    p18_working_mode_t mode;
} p18_working_mode_msg_t;

typedef struct {
    unsigned int fault_code;
    bool line_fail;
    bool output_circuit_short;
    bool inverter_over_temperature;
    bool fan_lock;
    bool battery_voltage_high;
    bool battery_low;
    bool battery_under;
    bool over_load;
    bool eeprom_fail;
    bool power_limit;
    bool pv1_voltage_high;
    bool pv2_voltage_high;
    bool mppt1_overload_warning;
    bool mppt2_overload_warning;
    bool battery_too_low_to_charge_for_scc1;
    bool battery_too_low_to_charge_for_scc2;
} p18_faults_warnings_msg_t;

typedef struct {
    bool buzzer;
    bool overload_bypass;
    bool lcd_escape_to_default_page_after_1min_timeout;
    bool overload_restart;
    bool over_temp_restart;
    bool backlight_on;
    bool alarm_on_primary_source_interrupt;
    bool fault_code_record;
    char reserved;
} p18_flags_statuses_msg_t;

typedef struct {
    unsigned int ac_output_voltage;      /* unit: 0.1V */
    unsigned int ac_output_freq;
    p18_input_voltage_range_t ac_input_voltage_range;
    unsigned int battery_under_voltage;
    unsigned int charging_float_voltage;
    unsigned int charging_bulk_voltage;
    unsigned int battery_recharge_voltage;
    unsigned int battery_redischarge_voltage;
    unsigned int max_charging_current;
    unsigned int max_ac_charging_current;
    p18_battery_type_t battery_type;
    p18_output_source_priority_t output_source_priority;
    p18_charger_source_priority_t charger_source_priority;
    p18_solar_power_priority_t solar_power_priority;
    p18_machine_type_t machine_type;
    p18_output_model_setting_t output_model_setting;
    bool flag_buzzer;
    bool flag_overload_restart;
    bool flag_over_temp_restart;
    bool flag_backlight_on;
    bool flag_alarm_on_primary_source_interrupt;
    bool flag_fault_code_record;
    bool flag_overload_bypass;
    bool flag_lcd_escape_to_default_page_after_1min_timeout;
} p18_defaults_msg_t;

typedef struct {
    size_t len;
    int amps[32];
} p18_max_charging_current_selectable_values_msg_t;

typedef struct {
    size_t len;
    int amps[32];
} p18_max_ac_charging_current_selectable_values_msg_t;

typedef struct {
    p18_parallel_id_connection_status_t parallel_id_connection_status;
    int serial_number_valid_length;
    char serial_number[32];
    p18_charger_source_priority_t charger_source_priority;
    unsigned int max_ac_charging_current;         /* unit: A */
    unsigned int max_charging_current;            /* unit: A */
    p18_output_model_setting_t output_model_setting;
} p18_parallel_rated_information_msg_t;

typedef struct {
    p18_parallel_id_connection_status_t parallel_id_connection_status;
    p18_working_mode_t work_mode;
    unsigned int fault_code;
    unsigned int grid_voltage;                   /* unit: 0.1V */
    unsigned int grid_freq;                      /* unit: 0.1Hz */
    unsigned int ac_output_voltage;              /* unit: 0.1V */
    unsigned int ac_output_freq;                 /* unit: 0.1Hz */
    unsigned int ac_output_apparent_power;       /* unit: VA */
    unsigned int ac_output_active_power;         /* unit: W */
    unsigned int total_ac_output_apparent_power; /* unit: VA */
    unsigned int total_ac_output_active_power;   /* unit: W */
    unsigned int output_load_percent;            /* unit: % */
    unsigned int total_output_load_percent;      /* unit: % */
    unsigned int battery_voltage;                /* unit: 0.1V */
    unsigned int battery_discharge_current;      /* unit: A */
    unsigned int battery_charging_current;       /* unit: A */
    unsigned int total_battery_charging_current; /* unit: A */
    unsigned int battery_capacity;               /* unit: % */
    unsigned int pv1_input_power;                /* unit: W */
    unsigned int pv2_input_power;                /* unit: W */
    unsigned int pv1_input_voltage;              /* unit: 0.1V */
    unsigned int pv2_input_voltage;              /* unit: 0.1V */
    p18_mppt_charger_status_t mppt1_charger_status;
    p18_mppt_charger_status_t mppt2_charger_status;
    bool load_connected;                         /* inverter returns:
                                                    0: disconnected
                                                    1: connected */
    p18_battery_power_direction_t battery_power_direction;
    p18_dc_ac_power_direction_t dc_ac_power_direction;
    p18_line_power_direction_t line_power_direction;
    unsigned int max_temp;                       /* unit: C */
} p18_parallel_general_status_msg_t;

typedef struct {
    unsigned short start_h;
    unsigned short start_m;
    unsigned short end_h;
    unsigned short end_m;
} p18_ac_charge_time_bucket_msg_t;

typedef struct {
    unsigned short start_h;
    unsigned short start_m;
    unsigned short end_h;
    unsigned short end_m;
} p18_ac_supply_load_time_bucket_msg_t;


/* ------------------------------------------ */
/* Some constants and allowed values */

typedef struct {
    unsigned int id;
    char message[64];
} p18_fault_code_list_item_t;

typedef struct {
    char *key;
    char *p18_key;
    char *title;
} p18_flag_printable_list_item_t;
extern const p18_flag_printable_list_item_t p18_flags_printable_list[9];

extern const int p18_ac_output_rated_voltages[5];

extern const char *p18_battery_util_recharging_voltages_12v_unit[8];
extern const char *p18_battery_util_recharging_voltages_24v_unit[8];
extern const char *p18_battery_util_recharging_voltages_48v_unit[8];

extern const char *p18_battery_util_redischarging_voltages_12v_unit[12];
extern const char *p18_battery_util_redischarging_voltages_24v_unit[12];
extern const char *p18_battery_util_redischarging_voltages_48v_unit[12];


/* ------------------------------------------ */
/* Common methods */

bool p18_build_command(int command, const char **args, size_t args_size, char *buf);
bool p18_validate_query_response(const char *buf, size_t size, size_t *data_size);
bool p18_set_result(const char *buf, size_t size);

/* ------------------------------------------ */
/* Command-specific methods */

P18_UNPACK_FN(protocol_id);
P18_UNPACK_FN(current_time);
P18_UNPACK_FN(total_generated);
P18_UNPACK_FN(year_generated);
P18_UNPACK_FN(month_generated);
P18_UNPACK_FN(day_generated);
P18_UNPACK_FN(series_number);
P18_UNPACK_FN(cpu_version);
P18_UNPACK_FN(rated_information);
P18_UNPACK_FN(general_status);
P18_UNPACK_FN(working_mode);
P18_UNPACK_FN(faults_warnings);
P18_UNPACK_FN(flags_statuses);
P18_UNPACK_FN(defaults);
P18_UNPACK_FN(max_charging_current_selectable_values);
P18_UNPACK_FN(max_ac_charging_current_selectable_values);
P18_UNPACK_FN(parallel_rated_information);
P18_UNPACK_FN(parallel_general_status);
P18_UNPACK_FN(ac_charge_time_bucket);
P18_UNPACK_FN(ac_supply_load_time_bucket);


/* ------------------------------------------ */
/* Label getters */

const char *p18_battery_type_label(p18_battery_type_t type);
const char *p18_input_voltage_range_label(p18_input_voltage_range_t range);
const char *p18_output_source_priority_label(p18_output_source_priority_t priority);
const char *p18_charge_source_priority_label(p18_charger_source_priority_t priority);
const char *p18_machine_type_label(p18_machine_type_t type);
const char *p18_topology_label(p18_topology_t topology);
const char *p18_output_model_setting_label(p18_output_model_setting_t setting);
const char *p18_solar_power_priority_label(p18_solar_power_priority_t priority);
const char *p18_mppt_charger_status_label(p18_mppt_charger_status_t status);
const char *p18_battery_power_direction_label(p18_battery_power_direction_t direction);
const char *p18_dc_ac_power_direction_label(p18_dc_ac_power_direction_t direction);
const char *p18_line_power_direction_label(p18_line_power_direction_t direction);
const char *p18_working_mode_label(p18_working_mode_t mode);
const char *p18_fault_code_label(unsigned int code);
const char *p18_parallel_connection_status_label(p18_parallel_id_connection_status_t status);

#endif //ISV_P18_H