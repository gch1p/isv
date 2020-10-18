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
 * GNU General Public    License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "print.h"
#include "util.h"

#define PRINT_AUTO(items) \
    if (print_is_table_format(format)) \
        print_table((items), ARRAY_SIZE(items), format == PRINT_FORMAT_PARSABLE_TABLE); \
    else \
        print_json((items), ARRAY_SIZE(items), format == PRINT_FORMAT_JSON_W_UNITS);

const short default_precision = 2;
const char *units[] = {
    " V",
    " A",
    " Wh",
    " kWh",
    " VA",
    " Hz",
    "%",
    " °C",
};
const char *unknown_unit = " ?";
const char *yes = "Yes";
const char *no = "No";
const char *true_s = "true";
const char *false_s = "false";
const char *enabled = "Enabled";
const char *disabled = "Disabled";

static const char* print_unit_label(print_unit_t unit)
{
    switch (unit) {
        case PRINT_UNIT_A:
        case PRINT_UNIT_V:
        case PRINT_UNIT_KWH:
        case PRINT_UNIT_WH:
        case PRINT_UNIT_VA:
        case PRINT_UNIT_HZ:
        case PRINT_UNIT_CELSIUS:
        case PRINT_UNIT_PERCENTAGE:
            return units[unit-1];

        default:
            return unknown_unit;
    }
}

static void print_table(print_item_t *items, size_t size, bool parsable)
{
    print_item_t item;
    char fmt[32], doublefmt[16];
    char k[64], v[64];

    if (!parsable) {
        size_t len, max_title_len = 0;
        for (size_t i = 0; i < size; i++) {
            item = items[i];
            len = strlen(item.title) + 1 /* for colon */;
            if (len > max_title_len)
                max_title_len = len;
        }
        sprintf(fmt, "%%-%zus %%s", max_title_len);
    } else {
        sprintf(fmt, "%%s %%s");
    }

    const char *unit;
    for (size_t i = 0; i < size; i++) {
        item = items[i];

        strcpy(k, parsable ? item.key : item.title);
        if (!parsable)
            strcat(k, ":");

        if (variant_is_double(item.value)) {
            sprintf(doublefmt, "%%2.%dlf",
                    item.precision ? item.precision : default_precision);
            snprintf(v, 32, doublefmt, item.value.d);
        } else if (variant_is_long(item.value))
            snprintf(v, 32, "%ld", item.value.l);
        else if (variant_is_string(item.value)) {
            char *pos = strchr(item.value.s, ' ');
            if (parsable && pos != NULL && pos != item.value.s)
                snprintf(v, 32, "\"%s\"", item.value.s);
            else
                snprintf(v, 32, "%s", item.value.s);
        } else if (variant_is_bool(item.value))
            snprintf(v, 32, "%s", item.value.b ? yes : no);
        else if (variant_is_flag(item.value))
            snprintf(v, 32, "%s", item.value.b ? enabled : disabled);

        printf(fmt, k, v);
        if (item.unit) {
            unit = print_unit_label(item.unit);
            if (parsable && *unit != ' ')
                putchar(' ');
            printf("%s", print_unit_label(item.unit));
        }

        putchar('\n');
    }
}

void print_json(print_item_t *items, size_t size, bool with_units)
{
    print_item_t item;
    putchar('{');
    for (size_t i = 0; i < size; i++) {
        item = items[i];
        printf("\"%s\":", item.key);

        if (item.unit && with_units)
            putchar('[');

        if (variant_is_string(item.value))
            printf("\"%s\"", item.value.s);
        else if (variant_is_double(item.value))
            printf("%2.2lf", item.value.d);
        else if (variant_is_long(item.value))
            printf("%ld", item.value.l);
        else if (variant_is_bool(item.value) || variant_is_flag(item.value))
            printf("%s", item.value.b ? true_s : false_s);

        if (item.unit && with_units) {
            const char *unit_label = print_unit_label(item.unit);
            if (unit_label[0] == ' ')
                unit_label++;
            printf(",\"%s\"]", unit_label);
        }

        if (i < size-1)
            putchar(',');
    }
    putchar('}');
    putchar('\n');
}

bool print_is_json_format(print_format_t f)
{
    return f == PRINT_FORMAT_JSON_W_UNITS || f == PRINT_FORMAT_JSON;
}

static bool print_is_table_format(print_format_t f)
{
    return f == PRINT_FORMAT_TABLE || f == PRINT_FORMAT_PARSABLE_TABLE;
}

void print_set_result(bool success, print_format_t format) {
    if (print_is_table_format(format))
        printf("%s\n", success ? "OK" : "Failure");
    else {
        print_item_t items[] = {
            {
                .key = (success ? "ok" : "error"),
                .value = (success ? variant_long(1) : variant_string("failure"))
            }
        };
        print_json(items, ARRAY_SIZE(items), false);
    }
}

static void print_table_list(const int *items, size_t size)
{
    for (size_t i = 0; i < size; i++)
        printf("%d\n", items[i]);
}

static void print_json_list(const int *items, size_t size)
{
    putchar('[');
    for (size_t i = 0; i < size; i++) {
        printf("%d", items[i]);
        if (i < size-1)
            putchar(',');
    }
    putchar(']');
    putchar('\n');
}


/* ------------------------------------------ */

PRINT_FN(protocol_id)
{
    print_item_t items[] = {
        {.key= "id", .title= "Protocol ID", .value= variant_long(m->id)}
    };
    PRINT_AUTO(items)
}

PRINT_FN(current_time)
{
    print_item_t items[] = {
        {.key= "year",   .title= "Year",   .value= variant_long(m->year)  },
        {.key= "month",  .title= "Month",  .value= variant_long(m->month) },
        {.key= "day",    .title= "Day",    .value= variant_long(m->day)   },
        {.key= "hour",   .title= "Hour",   .value= variant_long(m->hour)  },
        {.key= "minute", .title= "Minute", .value= variant_long(m->minute)},
        {.key= "second", .title= "Second", .value= variant_long(m->second)},
    };
    PRINT_AUTO(items)
}

PRINT_FN(total_generated)
{
    print_item_t items[] = {
        {
            .key = "kwh",
            .title = "kWh",
            .value = variant_long((long)m->kwh)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(year_generated)
{
    print_item_t items[] = {
        {
            .key = "kwh",
            .title = "kWh",
            .value = variant_long((long)m->kwh)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(month_generated)
{
    print_item_t items[] = {
        {
            .key = "kwh",
            .title = "kWh",
            .value = variant_long((long)m->kwh)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(day_generated)
{
    print_item_t items[] = {
        {
            .key = "wh",
            .title = "Wh",
            .value = variant_long((long)m->kwh)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(series_number)
{
    print_item_t items[] = {
        {.key= "sn", .title= "Series number", .value= variant_string(m->id)}
    };
    PRINT_AUTO(items)
}

PRINT_FN(cpu_version)
{
    print_item_t items[] = {
        {.key= "main_v",   .title= "Main CPU version",    .value= variant_string((m->main_cpu_version))},
        {.key= "slave1_v", .title= "Slave 1 CPU version", .value= variant_string(m->slave1_cpu_version)},
        {.key= "slave2_v", .title= "Slave 2 CPU version", .value= variant_string(m->slave2_cpu_version)},
    };
    PRINT_AUTO(items)
}

PRINT_FN(rated_information)
{
    print_item_t items[] = {
        {
            .key = "ac_input_rating_voltage",
            .title = "AC input rating voltage",
            .value = variant_double(m->ac_input_rating_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "ac_input_rating_current",
            .title = "AC input rating current",
            .value = variant_double(m->ac_input_rating_current/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "ac_output_rating_voltage",
            .title = "AC output rating voltage",
            .value = variant_double(m->ac_output_rating_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "ac_output_rating_freq",
            .title = "AC output rating frequency",
            .value = variant_double(m->ac_output_rating_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ
        },
        {
            .key = "ac_output_rating_current",
            .title =  "AC output rating current",
            .value = variant_double(m->ac_output_rating_current/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "ac_output_rating_apparent_power",
            .title = "AC output rating apparent power",
            .value = variant_long(m->ac_output_rating_apparent_power),
            .unit = PRINT_UNIT_VA,
        },
        {
            .key = "ac_output_rating_active_power",
            .title = "AC output rating active power",
            .value = variant_long(m->ac_output_rating_active_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "battery_rating_voltage",
            .title = "Battery rating voltage",
            .value = variant_double(m->battery_rating_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_recharge_voltage",
            .title = "Battery re-charge voltage",
            .value = variant_double(m->battery_recharge_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_redischarge_voltage",
            .title = "Battery re-discharge voltage",
            .value = variant_double(m->battery_redischarge_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_under_voltage",
            .title = "Battery under voltage",
            .value = variant_double(m->battery_under_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_bulk_voltage",
            .title = "Battery bulk voltage",
            .value = variant_double(m->battery_bulk_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_float_voltage",
            .title =  "Battery float voltage",
            .value = variant_double(m->battery_float_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_type",
            .title = "Battery type",
            .value = variant_string(p18_battery_type_label(m->battery_type))
        },
        {
            .key = "max_charging_current",
            .title = "Max charging current",
            .value = variant_long(m->max_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "max_ac_charging_current",
            .title = "Max AC charging current",
            .value = variant_long(m->max_ac_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "input_voltage_range",
            .title = "Input voltage range",
            .value = variant_string(p18_input_voltage_range_label(m->input_voltage_range))
        },
        {
            .key = "output_source_priority",
            .title = "Output source priority",
            .value = variant_string(p18_output_source_priority_label(m->output_source_priority))
        },
        {
            .key = "charger_source_priority",
            .title = "Charger source priority",
            .value = variant_string(p18_charge_source_priority_label(m->charger_source_priority))
        },
        {
            .key = "parallel_max_num",
            .title = "Parallel max num",
            .value = variant_long(m->parallel_max_num)
        },
        {
            .key = "machine_type",
            .title = "Machine type",
            .value = variant_string(p18_machine_type_label(m->machine_type))
        },
        {
            .key = "topology",
            .title = "Topology",
            .value = variant_string(p18_topology_label(m->topology))
        },
        {
            .key = "output_model_setting",
            .title = "Output model setting",
            .value = variant_string(p18_output_model_setting_label(m->output_model_setting))
        },
        {
            .key = "solar_power_priority",
            .title = "Solar power priority",
            .value = variant_string(p18_solar_power_priority_label(m->solar_power_priority))
        },
        {
            .key = "mppt",
            .title = "MPPT string",
            .value = variant_string(m->mppt)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(general_status)
{
    print_item_t items[] = {
        {
            .key = "grid_voltage",
            .title = "Grid voltage",
            .value = variant_double(m->grid_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "grid_freq",
            .title = "Grid frequency",
            .value = variant_double(m->grid_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ,
        },
        {
            .key = "ac_output_voltage",
            .title = "AC output voltage",
            .value = variant_double(m->ac_output_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "ac_output_freq",
            .title = "AC output frequency",
            .value = variant_double(m->ac_output_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ,
        },
        {
            .key = "ac_output_apparent_power",
            .title = "AC output apparent power",
            .value = variant_long(m->ac_output_apparent_power),
            .unit = PRINT_UNIT_VA,
        },
        {
            .key = "ac_output_active_power",
            .title = "AC output active power",
            .value = variant_long(m->ac_output_active_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "output_load_percent",
            .title = "Output load percent",
            .value = variant_long(m->output_load_percent),
            .unit = PRINT_UNIT_PERCENTAGE,
        },
        {
            .key = "battery_voltage",
            .title = "Battery voltage",
            .value = variant_double(m->battery_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_voltage_scc",
            .title = "Battery voltage from SCC",
            .value = variant_double(m->battery_voltage_scc/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_voltage_scc2",
            .title = "Battery voltage from SCC2",
            .value = variant_double(m->battery_voltage_scc2/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_discharge_current",
            .title = "Battery discharge current",
            .value = variant_long(m->battery_discharge_current),
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "battery_charging_current",
            .title = "Battery charging current",
            .value = variant_long(m->battery_charging_current),
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "battery_capacity",
            .title = "Battery capacity",
            .value = variant_long(m->battery_capacity),
            .unit = PRINT_UNIT_PERCENTAGE,
        },
        {
            .key = "inverter_heat_sink_temp",
            .title = "Inverter heat sink temperature",
            .value = variant_long(m->inverter_heat_sink_temp),
            .unit = PRINT_UNIT_CELSIUS,
        },
        {
            .key = "mppt1_charger_temp",
            .title = "MPPT1 charger temperature",
            .value = variant_long(m->mppt1_charger_temp),
            .unit = PRINT_UNIT_CELSIUS,
        },
        {
            .key = "mppt2_charger_temp",
            .title = "MPPT2 charger temperature",
            .value = variant_long(m->mppt2_charger_temp),
            .unit = PRINT_UNIT_CELSIUS,
        },
        {
            .key = "pv1_input_power",
            .title = "PV1 Input power",
            .value = variant_double(m->pv1_input_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "pv2_input_power",
            .title = "PV2 Input power",
            .value = variant_double(m->pv2_input_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "pv1_input_voltage",
            .title = "PV1 Input voltage",
            .value = variant_double(m->pv1_input_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "pv2_input_voltage",
            .title = "PV2 Input voltage",
            .value = variant_double(m->pv2_input_voltage/10.0),
                .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "settings_values_changed",
            .title = "Setting value configuration state",
            .value = variant_string(m->settings_values_changed ? "Nothing changed" : "Something changed"),
        },
        {
            .key = "mppt1_charger_status",
            .title = "MPPT1 charger status",
            .value = variant_string(p18_mppt_charger_status_label(m->mppt1_charger_status)),
        },
        {
            .key = "mppt2_charger_status",
            .title = "MPPT2 charger status",
            .value = variant_string(p18_mppt_charger_status_label(m->mppt2_charger_status)),
        },
        {
            .key = "load_connected",
            .title = "Load connection",
            .value = variant_string(m->load_connected ? "Connected" : "Disconnected"),
        },
        {
            .key = "battery_power_direction",
            .title = "Battery power direction",
            .value = variant_string(p18_battery_power_direction_label(m->battery_power_direction)),
        },
        {
            .key = "dc_ac_power_direction",
            .title = "DC/AC power direction",
            .value = variant_string(p18_dc_ac_power_direction_label(m->dc_ac_power_direction)),
        },
        {
            .key = "line_power_direction",
            .title = "Line power direction",
            .value = variant_string(p18_line_power_direction_label(m->line_power_direction)),
        },
        {
            .key = "local_parallel_id",
            .title = "Local parallel ID",
            .value = variant_long(m->local_parallel_id),
        }
    };
    PRINT_AUTO(items)
}

PRINT_FN(working_mode)
{
    print_item_t items[] = {
        {.key= "mode", .title= "Working mode", .value= variant_string(p18_working_mode_label(m->mode))}
    };
    PRINT_AUTO(items)
}

PRINT_FN(faults_warnings)
{
    print_item_t items[] = {
        {.key= "fault_code",                .title= "Fault code",               .value= variant_string(p18_fault_code_label(m->fault_code))},
        {.key= "line_fail",                 .title= "Line fail",                .value= variant_bool(m->line_fail)},
        {.key= "output_circuit_short",      .title= "Output circuit short",     .value= variant_bool(m->output_circuit_short)},
        {.key= "inverter_over_temperature", .title= "Inverter over temperature",.value= variant_bool(m->inverter_over_temperature)},
        {.key= "fan_lock",                  .title= "Fan lock",                 .value= variant_bool(m->fan_lock)},
        {.key= "battery_voltage_high",      .title= "Battery voltage high",     .value= variant_bool(m->battery_voltage_high)},
        {.key= "battery_low",               .title= "Battery low",              .value= variant_bool(m->battery_low)},
        {.key= "battery_under",             .title= "Battery under",            .value= variant_bool(m->battery_under)},
        {.key= "over_load",                 .title= "Over load",                .value= variant_bool(m->over_load)},
        {.key= "eeprom_fail",               .title= "EEPROM fail",              .value= variant_bool(m->eeprom_fail)},
        {.key= "power_limit",               .title= "Power limit",              .value= variant_bool(m->power_limit)},
        {.key= "pv1_voltage_high",          .title= "PV1 voltage high",         .value= variant_bool(m->pv1_voltage_high)},
        {.key= "pv2_voltage_high",          .title= "PV2 voltage high",         .value= variant_bool(m->pv2_voltage_high)},
        {.key= "mppt1_overload_warning",    .title= "MPPT1 overload warning",   .value= variant_bool(m->mppt1_overload_warning)},
        {.key= "mppt2_overload_warning",    .title= "MPPT2 overload warning",   .value= variant_bool(m->mppt2_overload_warning)},
        {.key= "battery_too_low_to_charge_for_scc1", .title= "Battery too low to charge for SCC1", .value= variant_bool(m->battery_too_low_to_charge_for_scc1)},
        {.key= "battery_too_low_to_charge_for_scc2", .title= "Battery too low to charge for SCC2", .value= variant_bool(m->battery_too_low_to_charge_for_scc2)},
    };
    PRINT_AUTO(items)
}

PRINT_FN(flags_statuses)
{
    print_item_t items[] = {
        {
            .key = "buzzer",
            .title = "Buzzer",
            .value = variant_flag(m->buzzer)
        },
        {
            .key = "overload_bypass",
            .title = "Overload bypass function",
            .value = variant_flag(m->overload_bypass)
        },
        {
            .key = "lcd_escape_to_default_page_after_1min_timeout",
            .title = "Escape to default page after 1min timeout",
            .value = variant_flag(m->lcd_escape_to_default_page_after_1min_timeout)
        },
        {
            .key = "overload_restart",
            .title = "Overload restart",
            .value = variant_flag(m->overload_restart)
        },
        {
            .key = "over_temp_restart",
            .title = "Over temperature restart",
            .value = variant_flag(m->over_temp_restart)
        },
        {
            .key = "backlight_on",
            .title = "Backlight on",
            .value = variant_flag(m->backlight_on)
        },
        {
            .key = "alarm_on_primary_source_interrupt",
            .title = "Alarm on when primary source interrupt",
            .value = variant_flag(m->alarm_on_primary_source_interrupt)
        },
        {
            .key = "fault_code_record",
            .title = "Fault code record",
            .value = variant_flag(m->fault_code_record)
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(defaults)
{
    print_item_t items[] = {
        {
            .key = "ac_output_voltage",
            .title = "AC output voltage",
            .value = variant_double(m->ac_output_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "ac_output_freq",
            .title = "AC output frequency",
            .value = variant_double(m->ac_output_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ,
        },
        {
            .key = "ac_input_voltage_range",
            .title = "AC input voltage range",
            .value = variant_string(p18_input_voltage_range_label(m->ac_input_voltage_range)),
        },
        {
            .key = "battery_under_voltage",
            .title = "Battery under voltage",
            .value = variant_double(m->battery_under_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_bulk_voltage",
            .title = "Charging bulk voltage",
            .value = variant_double(m->charging_bulk_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_float_voltage",
            .title =  "Charging float voltage",
            .value = variant_double(m->charging_float_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_recharge_voltage",
            .title = "Battery re-charge voltage",
            .value = variant_double(m->battery_recharge_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_redischarge_voltage",
            .title = "Battery re-discharge voltage",
            .value = variant_double(m->battery_redischarge_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "max_ac_charging_current",
            .title = "Max AC charging current",
            .value = variant_long(m->max_ac_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "max_charging_current",
            .title = "Max charging current",
            .value = variant_long(m->max_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "battery_type",
            .title = "Battery type",
            .value = variant_string(p18_battery_type_label(m->battery_type))
        },
        {
            .key = "output_source_priority",
            .title = "Output source priority",
            .value = variant_string(p18_output_source_priority_label(m->output_source_priority))
        },
        {
            .key = "charger_source_priority",
            .title = "Charger source priority",
            .value = variant_string(p18_charge_source_priority_label(m->charger_source_priority))
        },
        {
            .key = "solar_power_priority",
            .title = "Solar power priority",
            .value = variant_string(p18_solar_power_priority_label(m->solar_power_priority))
        },
        {
            .key = "machine_type",
            .title = "Machine type",
            .value = variant_string(p18_machine_type_label(m->machine_type))
        },
        {
            .key = "output_model_setting",
            .title = "Output model setting",
            .value = variant_string(p18_output_model_setting_label(m->output_model_setting))
        },
        {
            .key = "buzzer_flag",
            .title = "Buzzer flag",
            .value = variant_flag(m->flag_buzzer)
        },
        {
            .key = "overload_bypass_flag",
            .title = "Overload bypass function flag",
            .value = variant_flag(m->flag_overload_bypass)
        },
        {
            .key = "lcd_escape_to_default_page_after_1min_timeout_flag",
            .title = "Escape to default page after 1min timeout flag",
            .value = variant_flag(m->flag_lcd_escape_to_default_page_after_1min_timeout)
        },
        {
            .key = "overload_restart_flag",
            .title = "Overload restart flag",
            .value = variant_flag(m->flag_overload_restart)
        },
        {
            .key = "over_temp_restart_flag",
            .title = "Over temperature restart flag",
            .value = variant_flag(m->flag_over_temp_restart)
        },
        {
            .key = "backlight_on_flag",
            .title = "Backlight on flag",
            .value = variant_flag(m->flag_backlight_on)
        },
        {
            .key = "alarm_on_primary_source_interrupt_flag",
            .title = "Alarm on when primary source interrupt flag",
            .value = variant_flag(m->flag_alarm_on_primary_source_interrupt)
        },
        {
            .key = "fault_code_record_flag",
            .title = "Fault code record flag",
            .value = variant_flag(m->flag_fault_code_record)
        }
    };
    PRINT_AUTO(items)
}

PRINT_FN(max_charging_current_selectable_values)
{
    if (print_is_json_format(format))
        print_json_list(m->amps, m->len);
    else if (print_is_table_format(format))
        print_table_list(m->amps, m->len);
}

PRINT_FN(max_ac_charging_current_selectable_values)
{
    if (print_is_json_format(format))
        print_json_list(m->amps, m->len);
    else if (print_is_table_format(format))
        print_table_list(m->amps, m->len);
}

PRINT_FN(parallel_rated_information)
{
    print_item_t items[] = {
        {
            .key = "parallel_id_connection_status",
            .title = "Parallel ID connection status",
            .value = variant_string(p18_parallel_connection_status_label(m->parallel_id_connection_status)),
        },
        {
            .key = "serial_number",
            .title = "Serial number",
            .value = variant_string(m->serial_number),
        },
        {
            .key = "charger_source_priority",
            .title = "Charger source priority",
            .value = variant_string(p18_charge_source_priority_label(m->charger_source_priority))
        },
        {
            .key = "max_charging_current",
            .title = "Max charging current",
            .value = variant_long(m->max_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "max_ac_charging_current",
            .title = "Max AC charging current",
            .value = variant_long(m->max_ac_charging_current),
            .unit = PRINT_UNIT_A
        },
        {
            .key = "output_model_setting",
            .title = "Output model setting",
            .value = variant_string(p18_output_model_setting_label(m->output_model_setting))
        },
    };
    PRINT_AUTO(items)
}

PRINT_FN(parallel_general_status)
{
    print_item_t items[] = {
        {
            .key = "parallel_id_connection_status",
            .title = "Parallel ID connection status",
            .value = variant_string(p18_parallel_connection_status_label(m->parallel_id_connection_status)),
        },
        {
            .key = "mode",
            .title = "Working mode",
            .value = variant_string(p18_working_mode_label(m->work_mode))
        },
        {
            .key = "fault_code",
            .title = "Fault code",
            .value = variant_string(p18_fault_code_label(m->fault_code))
        },
        {
            .key = "grid_voltage",
            .title = "Grid voltage",
            .value = variant_double(m->grid_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "grid_freq",
            .title = "Grid frequency",
            .value = variant_double(m->grid_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ,
        },
        {
            .key = "ac_output_voltage",
            .title = "AC output voltage",
            .value = variant_double(m->ac_output_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "ac_output_freq",
            .title = "AC output frequency",
            .value = variant_double(m->ac_output_freq/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_HZ,
        },
        {
            .key = "ac_output_apparent_power",
            .title = "AC output apparent power",
            .value = variant_long(m->ac_output_apparent_power),
            .unit = PRINT_UNIT_VA,
        },
        {
            .key = "ac_output_active_power",
            .title = "AC output active power",
            .value = variant_long(m->ac_output_active_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "total_ac_output_apparent_power",
            .title = "Total AC output apparent power",
            .value = variant_long(m->total_ac_output_apparent_power),
            .unit = PRINT_UNIT_VA,
        },
        {
            .key = "total_ac_output_active_power",
            .title = "Total AC output active power",
            .value = variant_long(m->total_ac_output_active_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "output_load_percent",
            .title = "Output load percent",
            .value = variant_long(m->output_load_percent),
            .unit = PRINT_UNIT_PERCENTAGE,
        },
        {
            .key = "total_output_load_percent",
            .title = "Total output load percent",
            .value = variant_long(m->total_output_load_percent),
            .unit = PRINT_UNIT_PERCENTAGE,
        },
        {
            .key = "battery_voltage",
            .title = "Battery voltage",
            .value = variant_double(m->battery_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "battery_discharge_current",
            .title = "Battery discharge current",
            .value = variant_long(m->battery_discharge_current),
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "battery_charging_current",
            .title = "Battery charging current",
            .value = variant_long(m->battery_charging_current),
            .unit = PRINT_UNIT_A,
        },
        {
            .key = "pv1_input_power",
            .title = "PV1 Input power",
            .value = variant_double(m->pv1_input_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "pv2_input_power",
            .title = "PV2 Input power",
            .value = variant_double(m->pv2_input_power),
            .unit = PRINT_UNIT_WH,
        },
        {
            .key = "pv1_input_voltage",
            .title = "PV1 Input voltage",
            .value = variant_double(m->pv1_input_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "pv2_input_voltage",
            .title = "PV2 Input voltage",
            .value = variant_double(m->pv2_input_voltage/10.0),
            .precision = 1,
            .unit = PRINT_UNIT_V,
        },
        {
            .key = "mppt1_charger_status",
            .title = "MPPT1 charger status",
            .value = variant_string(p18_mppt_charger_status_label(m->mppt1_charger_status)),
        },
        {
            .key = "mppt2_charger_status",
            .title = "MPPT2 charger status",
            .value = variant_string(p18_mppt_charger_status_label(m->mppt2_charger_status)),
        },
        {
            .key = "load_connected",
            .title = "Load connection",
            .value = variant_string(m->load_connected ? "Connected" : "Disconnected"),
        },
        {
            .key = "battery_power_direction",
            .title = "Battery power direction",
            .value = variant_string(p18_battery_power_direction_label(m->battery_power_direction)),
        },
        {
            .key = "dc_ac_power_direction",
            .title = "DC/AC power direction",
            .value = variant_string(p18_dc_ac_power_direction_label(m->dc_ac_power_direction)),
        },
        {
            .key = "line_power_direction",
            .title = "Line power direction",
            .value = variant_string(p18_line_power_direction_label(m->line_power_direction)),
        },
        {
            .key = "max_temp",
            .title = "Max. temperature",
            .value = variant_long(m->max_temp),
        }
    };
    PRINT_AUTO(items)
}

PRINT_FN(ac_charge_time_bucket)
{
    static const int buf_size = 16;
    char start_time[buf_size], end_time[buf_size];
    snprintf(start_time, buf_size, "%02d:%02d", m->start_h, m->start_m);
    snprintf(end_time, buf_size, "%02d:%02d", m->end_h, m->end_m);
    print_item_t items[] = {
            {.key= "start_time", .title= "Start time", .value= variant_string(start_time)},
            {.key= "end_time",   .title= "End time",   .value= variant_string(end_time)},
    };
    PRINT_AUTO(items)
}

PRINT_FN(ac_supply_load_time_bucket)
{
    static const int buf_size = 16;
    char start_time[buf_size], end_time[buf_size];
    snprintf(start_time, buf_size, "%02d:%02d", m->start_h, m->start_m);
    snprintf(end_time, buf_size, "%02d:%02d", m->end_h, m->end_m);
    print_item_t items[] = {
            {.key= "start_time", .title= "Start time", .value= variant_string(start_time)},
            {.key= "end_time",   .title= "End time",   .value= variant_string(end_time)},
    };
    PRINT_AUTO(items)
}