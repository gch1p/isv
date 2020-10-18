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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "p18.h"
#include "util.h"

const char *p18_query_cmds[] = {
    "PI",      /* Protocol ID */
    "T",       /* Current time */
    "ET",      /* Total generated energy */
    "EY",      /* Year generated energy */
    "EM",      /* Month generated energy */
    "ED",      /* Day generated energy */
    "ID",      /* Series number */
    "VFW",     /* CPU version */
    "PIRI",    /* Rated information */
    "GS",      /* General status */
    "MOD",     /* Working mode */
    "FWS",     /* Fault and warning status */
    "FLAG",    /* Enable/Disable flags statuses */
    "DI",      /* Default values of changeable parameters */
    "MCHGCR",  /* Max charging current selectable (allowed) values */
    "MUCHGCR", /* Max AC charging current selectable (allowed) values */
    "PRI",     /* Parallel system rated information */
    "PGS",     /* Parallel system general status */
    "ACCT",    /* AC charge time bucket */
    "ACLT",    /* AC supply load time bucket */
};

const char *p18_set_cmds[] = {
    "LON",    /* Enable/disable machine supply power to the loads */
    "P",      /* Enable/disable flags */
    "PF",     /* Reset changeable params */
    "MCHGC",  /* Battery maximum charge current */
    "MUCHGC", /* Battery maximum AC charge current */
    /* The protocol documentation defines two commands, "F50" and "F60",
       but it's identical as if there were just one "F" command with an argument. */
    "F",      /* Set AC output frequency to be 50Hz or 60Hz */
    "MCHGV",  /* Battery maximum charge voltage */
    "V",      /* AC output rated voltage */
    "POP",    /* Output source priority */
    "BUCD",   /* Battery re-charged and re-discharged voltage when utility is available */
    "PCP",    /* Charging source priority */
    "PSP",    /* Solar power priority */
    "PGR",    /* AC input voltage range */
    "PBT",    /* Battery type */
    "POPM",   /* Output model */
    "PSDV",   /* Battery cut-off voltage */
    "ID",     /* Set solar configuration */
    "CLE",    /* Clear all data of generated energy */
    "DAT",    /* Set date and time */
    "ACCT",   /* AC charge time bucket */
    "ACLT",   /* AC supply load time bucket */
};

const p18_fault_code_list_item_t fault_codes[] = {
    {1, "Fan is locked"},
    {2, "Over temperature"},
    {3, "Battery voltage is too high"},
    {4, "Battery voltage is too low"},
    {5, "Output short circuited or Over temperature"},
    {6, "Output voltage is too high"},
    {7, "Over load time out"},
    {8, "Bus voltage is too high"},
    {9, "Bus soft start failed"},
    {11, "Main relay failed"},
    {51, "Over current inverter"},
    {52, "Bus soft start failed"},
    {53, "Inverter soft start failed"},
    {54, "Self-test failed"},
    {55, "Over DC voltage on output of inverter"},
    {56, "Battery connection is open"},
    {57, "Current sensor failed"},
    {58, "Output voltage is too low"},
    {60, "Inverter negative power"},
    {71, "Parallel version different"},
    {72, "Output circuit failed"},
    {80, "CAN communication failed"},
    {81, "Parallel host line lost"},
    {82, "Parallel synchronized signal lost"},
    {83, "Parallel battery voltage detect different"},
    {84, "Parallel Line voltage or frequency detect different"},
    {85, "Parallel Line input current unbalanced"},
    {86, "Parallel output setting different"},
};

const p18_flag_printable_list_item_t p18_flags_printable_list[9] = {
    {"BUZZ", "A", "Silence buzzer or open buzzer"},
    {"OLBP", "B", "Overload bypass function"},
    {"LCDE", "C", "LCD display escape to default page after 1min timeout"},
    {"OLRS", "D", "Overload restart"},
    {"OTRS", "E", "Overload temperature restart"},
    {"BLON", "F", "Backlight on"},
    {"ALRM", "G", "Alarm on primary source interrupt"},
    {"FTCR", "H", "Fault code record"},
    {"MTYP", "I", "Machine type (1=Grid-Tie, 0=Off-Grid-Tie)"},
};

const int p18_ac_output_rated_voltages[5] = {
    202,
    208,
    220,
    230,
    240,
};

const char *p18_battery_util_recharging_voltages_12v_unit[8] = {
    "11",
    "11.3",
    "11.5",
    "11.8",
    "12",
    "12.3",
    "12.5",
    "12.8",
};

const char *p18_battery_util_recharging_voltages_24v_unit[8] = {
    "22",
    "22.5",
    "23",
    "23.5",
    "24",
    "24.5",
    "25",
    "25.5",
};
const char *p18_battery_util_recharging_voltages_48v_unit[8] = {
    "44",
    "45",
    "46",
    "47",
    "48",
    "49",
    "50",
    "51",
};

const char *p18_battery_util_redischarging_voltages_12v_unit[12] = {
    "0",
    "12",
    "12.3",
    "12.5",
    "12.8",
    "13",
    "13.3",
    "13.5",
    "13.8",
    "14",
    "14.3",
    "14.5",
};
const char *p18_battery_util_redischarging_voltages_24v_unit[12] = {
    "0",
    "24",
    "24.5",
    "25",
    "25.5",
    "26",
    "26.5",
    "27",
    "27.5",
    "28",
    "28.5",
    "29"
};
const char *p18_battery_util_redischarging_voltages_48v_unit[12] = {
    "0",
    "48",
    "49",
    "50",
    "51",
    "52",
    "53",
    "54",
    "55",
    "56",
    "57",
    "58",
};

const char *p18_battery_types[] = {
    "AGM",
    "Flooded",
    "User",
};

const char *p18_voltage_ranges[] = {
    "Appliance",
    "UPS"
};

const char *p18_output_source_priorities[] = {
    "Solar-Utility-Battery",
    "Solar-Battery-Utility"
};

const char *p18_charger_source_priorities[] = {
    "Solar-First",
    "Solar-and-Utility",
    "Solar-Only"
};

const char *p18_machine_types[] = {
    "Off-Grid-Tie",
    "Grid-Tie"
};

const char *p18_topologies[] = {
    "Transformerless",
    "Transformer"
};

const char *p18_output_model_settings[] = {
    "Single module",
    "Parallel output",
    "Phase 1 of three phase output",
    "Phase 2 of three phase output",
    "Phase 3 of three phase",
};

const char *p18_solar_power_priorities[] = {
    "Battery-Load-Utility",
    "Load-Battery-Utility"
};

const char *p18_mppt_charger_statuses[] = {
    "Abnormal",
    "Not charging",
    "Charging"
};

const char *p18_battery_power_directions[] = {
    "Do nothing",
    "Charge",
    "Discharge"
};

const char *p18_dc_ac_power_directions[] = {
    "Do nothing",
    "AC/DC",
    "DC/AC"
};

const char *p18_line_power_directions[] = {
    "Do nothing",
    "Input",
    "Output",
};

const char *p18_working_modes[] = {
    "Power on mode",
    "Standby mode",
    "Bypass mode",
    "Battery mode",
    "Fault mode",
    "Hybrid mode",
};

const char *p18_parallel_connection_statuses[] = {
    "Non-existent",
    "Existent"
};

const char *p18_unknown = "Unknown";

/* ------------------------------------------ */

static char *p18_append_arguments(int command, char *buf, const char **a)
{
    switch (command) {
        case P18_QUERY_YEAR_GENERATED:
            buf += sprintf(buf, "%s", a[0]);
            break;

        case P18_QUERY_MONTH_GENERATED:
            buf += sprintf(buf, "%s%02d", a[0], atoi(a[1]));
            break;

        case P18_QUERY_DAY_GENERATED:
            buf += sprintf(buf, "%s%02d%02d",
                           a[0], atoi(a[1]), atoi(a[2]));
            break;

        case P18_QUERY_PARALLEL_RATED_INFORMATION:
        case P18_QUERY_PARALLEL_GENERAL_STATUS:
            buf += sprintf(buf, "%d", atoi(a[0]));
            break;

        case P18_SET_LOADS:
            buf += sprintf(buf, "%s", a[0]);
            break;

        case P18_SET_FLAG:
            buf += sprintf(buf, "%c%s",
                           *a[1] == '1' ? 'E' : 'D',
                           a[0]);
            break;

        case P18_SET_BAT_MAX_CHARGE_CURRENT:
        case P18_SET_BAT_MAX_AC_CHARGE_CURRENT:
            buf += sprintf(buf, "%c,%03d", *a[0], atoi(a[1]));
            break;

        case P18_SET_AC_OUTPUT_FREQ:
            buf += sprintf(buf, "%02d", atoi(a[0]));
            break;

        case P18_SET_BAT_MAX_CHARGE_VOLTAGE: {
            double cv = atof(a[0]);
            double fv = atof(a[1]);
            buf += sprintf(buf, "%03d,%03d", (int)round(cv*10), (int)round(fv*10));
            break;
        }

        case P18_SET_AC_OUTPUT_RATED_VOLTAGE: {
            int v = atoi(a[0]);
            buf += sprintf(buf, "%04d", v*10);
            break;
        }

        case P18_SET_OUTPUT_SOURCE_PRIORITY:
        case P18_SET_SOLAR_POWER_PRIORITY:
        case P18_SET_AC_INPUT_VOLTAGE_RANGE:
        case P18_SET_BAT_TYPE:
            buf += sprintf(buf, "%c", *a[0]);
            break;

        case P18_SET_BAT_CHARGING_THRESHOLDS_WHEN_UTILITY_AVAIL: {
            double cv = atof(a[0]);
            double dv = atof(a[1]);
            buf += sprintf(buf, "%03d,%03d", (int)round(cv*10), (int)round(dv*10));
            break;
        }

        case P18_SET_CHARGING_SOURCE_PRIORITY:
        case P18_SET_OUTPUT_MODEL:
            buf += sprintf(buf, "%c,%c", *a[0], *a[1]);
            break;

        case P18_SET_BAT_CUTOFF_VOLTAGE: {
            double v = atof(a[0]);
            buf += sprintf(buf, "%03d", (int)round(v*10));
            break;
        }

        case P18_SET_SOLAR_CONFIG: {
            int len = (int)strlen(a[0]);
            buf += sprintf(buf, "%02d", len);
            buf += sprintf(buf, "%s", a[0]);
            if (len < 20) {
                for (int i = 0; i < 20-len; i++)
                    buf += sprintf(buf, "%c", '0');
            }
            break;
        }

        case P18_SET_DATE_TIME: {
            int Y = atoi(a[0]) - 2000;
            int M = atoi(a[1]);
            int D = atoi(a[2]);
            int h = atoi(a[3]);
            int m = atoi(a[4]);
            int s = atoi(a[5]);
            buf += sprintf(buf, "%02d%02d%02d%02d%02d%02d", Y, M, D, h, m, s);
            break;
        }

        case P18_SET_AC_CHARGE_TIME_BUCKET:
        case P18_SET_AC_SUPPLY_LOAD_TIME_BUCKET:
            buf += sprintf(buf, "%02d%02d,%02d%02d",
                           atoi(a[0]), atoi(a[1]), atoi(a[2]), atoi(a[3]));
            break;
    }

    return buf;
}

bool p18_build_command(int command, const char **args, size_t args_size, char *buf)
{
    UNUSED(args_size);

    bool set = command >= P18_SET_CMDS_ENUM_OFFSET;
    int offset;
    const char **list;
    size_t list_size;
    if (!set) {
        list = p18_query_cmds;
        list_size = ARRAY_SIZE(p18_query_cmds);
        offset = P18_QUERY_CMDS_ENUM_OFFSET;
    } else {
        list = p18_set_cmds;
        list_size = ARRAY_SIZE(p18_set_cmds);
        offset = P18_SET_CMDS_ENUM_OFFSET;
    }

    int cmd_index = command - offset;
    if (cmd_index < 0 || cmd_index >= (int)list_size) {
        LOG("%s: invalid cmd_index %d\n", __func__, cmd_index);
        return false;
    }

    (*buf++) = '^';
    (*buf++) = set ? 'S' : 'P';

    char *len_p = buf;
    buf += sprintf(buf, "000");
    buf += sprintf(buf, "%s", list[cmd_index]);

    buf = p18_append_arguments(command, buf, args);

    char len_buf[4];
    sprintf(len_buf, "%03zu", (size_t)(buf-len_p));
    memcpy(len_p, len_buf, 3);

    return true;
}

bool p18_validate_query_response(const char *buf, size_t size, size_t *data_size)
{
    if (buf[0] != '^' || buf[1] != 'D')
        return false;

    char lenbuf[4];
    memcpy(lenbuf, &buf[2], 3);
    lenbuf[3] = '\0';

    unsigned int len = atoi(lenbuf);
    if (size < len+4)
        return false;

    if (data_size)
        *data_size = len-3;
    return true;
}

bool p18_set_result(const char *buf, size_t size)
{
    if (size < 2) {
        ERROR("%s: response is too small\n", __func__);
        return false;
    }
    if (buf[0] != '^') {
        ERROR("%s: invalid character %c, ^ expected\n", __func__, *buf);
        return false;
    }
    return buf[1] == '1';
}

static void p18_parse_list(
        const char *data,
        void *message_ptr,
        int expected_items_count, /* -1 means no fixed limit */
        p18_parse_cb_t callback)
{
    int data_size = (int)strlen(data);
    static const int buf_size = 64;
    char buf[buf_size];
    char *c = (char *)data;

    for (int i = 0, char_index = 0, item_index = 0;
         i < data_size;
         i++) {

        if (*c != ',' && i < data_size-1) {
            if (char_index < buf_size-1)
                buf[char_index++] = *c;
            c++;
            continue;
        }

        /* last character */
        if (*c != ',' && char_index < buf_size-1)
            buf[char_index++] = *c;

        if (expected_items_count != -1 && item_index >= expected_items_count)
            ERROR("warning: item %d is not expected\n", item_index);
        else {
            buf[char_index] = '\0';
            callback((const char *)buf, char_index, item_index, message_ptr);
        }

        char_index = 0;
        c++;
        item_index++;
    }
}

static void p18_expect_listitem_length(const char *f,
                                       int index,
                                       size_t expected_len,
                                       size_t actual_len) {
    if (actual_len != expected_len) {
        LOG("%s: length of item %d is %zu != %zu\n",
            f, index, actual_len, expected_len);
    }
}

/* ------------------------------------------ */
/* Command-specific methods */

P18_UNPACK_FN(protocol_id)
{
    char s[4];
    strncpy(s, data, 2);
    s[2] = '\0';

    p18_protocol_id_msg_t m;
    m.id = atoi(s);

    return m;
}

P18_UNPACK_FN(current_time)
{
    char s[4];
    int n;
    p18_current_time_msg_t m;

    substr_copy(s, data, 4);
    m.year = atoi(s);

    for (int i = 0; i < 5; i++) {
        substr_copy(s, data + 4 + (i * 2), 2);
        n = atoi(s);
        switch (i) {
            case 0: m.month = n; break;
            case 1: m.day = n; break;
            case 2: m.hour = n; break;
            case 3: m.minute = n; break;
            case 4: m.second = n; break;
            default:
                ERROR("%s: warning: unexpected code path, i=%d\n",
                      __func__, i);
                break;
        }
    }

    return m;
}

static long parse_generated(const char *buf)
{
    char s[16];
    strncpy(s, buf, 8);
    s[8] = '\0';
    return atol(s);
}

P18_UNPACK_FN(total_generated)
{
    p18_total_generated_msg_t m = {
        .kwh = parse_generated(data)
    };
    return m;
}

P18_UNPACK_FN(year_generated)
{
    p18_year_generated_msg_t m = {
        .kwh = parse_generated(data)
    };
    return m;
}

P18_UNPACK_FN(month_generated)
{
    p18_month_generated_msg_t m = {
        .kwh = parse_generated(data)
    };
    return m;
}

P18_UNPACK_FN(day_generated)
{
    p18_day_generated_msg_t m = {
        .kwh = parse_generated(data)
    };
    return m;
}

P18_UNPACK_FN(series_number)
{
    p18_series_number_msg_t m;

    /* read length, reuse the already available id datafer */
    substr_copy(m.id, data, 2);
    m.length = atoi(m.id);

    /* read id of given length */
    substr_copy(m.id, data + 2, m.length);

    return m;
}

P18_PARSE_CB_FN(cpu_version)
{
    //LOG("%s: value=%s, value_len=%d\n", __func__, value, value_len);

    p18_cpu_version_msg_t *m = (p18_cpu_version_msg_t *)message_ptr;
    P18_EXPECT_LISTITEM_LENGTH(5);

    switch (index) {
        case 0:
            substr_copy(m->main_cpu_version, value, 5);
            break;

        case 1:
            substr_copy(m->slave1_cpu_version, value, 5);
            break;

        case 2:
            substr_copy(m->slave2_cpu_version, value, 5);
            break;
    }
}
P18_UNPACK_FN(cpu_version)
{
    P18_PARSE_LIST_AND_RETURN(cpu_version, 3);
}

P18_PARSE_CB_FN(rated_information)
{
    p18_rated_information_msg_t *m = (p18_rated_information_msg_t *)message_ptr;
    int num = atoi(value);
    switch (index) {
        case 0: /* AAAA */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_input_rating_voltage = num;
            break;
        case 1: /* BBB */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_input_rating_current = num;
            break;
        case 2: /* CCCC */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_rating_voltage = num;
            break;
        case 3: /* DDD */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_output_rating_freq = num;
            break;
        case 4: /* EEE */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_output_rating_current = num;
            break;
        case 5: /* FFFF */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_rating_apparent_power = num;
            break;
        case 6: /* GGGG */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_rating_active_power = num;
            break;
        case 7: /* HHH */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_rating_voltage = num;
            break;
        case 8: /* III */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_recharge_voltage = num;
            break;
        case 9: /* JJJ */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_redischarge_voltage = num;
            break;
        case 10: /* KKK */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_under_voltage = num;
            break;
        case 11: /* LLL */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_bulk_voltage = num;
            break;
        case 12: /* MMM */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_float_voltage = num;
            break;
        case 13: /* N */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->battery_type = num;
            break;
        case 14: /* OO */
            P18_EXPECT_LISTITEM_LENGTH(2);
            m->max_ac_charging_current = num;
            break;
        case 15: /* PPP */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->max_charging_current = num;
            break;
        case 16: /* Q */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->input_voltage_range = num;
            break;
        case 17: /* R */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->output_source_priority = num;
            break;
        case 18: /* S */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->charger_source_priority = num;
            break;
        case 19: /* T */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->parallel_max_num = num;
            break;
        case 20: /* U */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->machine_type = num;
            break;
        case 21: /* V */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->topology = num;
            break;
        case 22: /* W */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->output_model_setting = num;
            break;
        case 23: /* Z */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->solar_power_priority = num;
            break;
        case 24: /* a */
            P18_EXPECT_LISTITEM_LENGTH(1);
            substr_copy(m->mppt, value, 1);
            break;
    }
}
P18_UNPACK_FN(rated_information)
{
    P18_PARSE_LIST_AND_RETURN(rated_information, 25)
}

P18_PARSE_CB_FN(general_status)
{
    p18_general_status_msg_t *m = (p18_general_status_msg_t *)message_ptr;
    int num = atoi(value);
    switch (index) {
        case 0: /* AAAA */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->grid_voltage = num;
            break;
        case 1: /* BBB */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->grid_freq = num;
            break;
        case 2: /* CCCC */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_voltage = num;
            break;
        case 3: /* DDD */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_output_freq = num;
            break;
        case 4: /* EEEE */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_apparent_power = num;
            break;
        case 5: /* FFFF */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_active_power = num;
            break;
        case 6: /* GGG */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->output_load_percent = num;
            break;
        case 7: /* HHH */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_voltage = num;
            break;
        case 8: /* III */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_voltage_scc = num;
            break;
        case 9: /* JJJ */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_voltage_scc2 = num;
            break;
        case 10: /* KKK */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_discharge_current = num;
            break;
        case 11: /* LLL */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_charging_current = num;
            break;
        case 12: /* MMM */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_capacity = num;
            break;
        case 13: /* NNN */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->inverter_heat_sink_temp = num;
            break;
        case 14: /* OOO */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->mppt1_charger_temp = num;
            break;
        case 15: /* PPP */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->mppt2_charger_temp = num;
            break;
        case 16: /* QQQQ */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv1_input_power = num;
            break;
        case 17: /* RRRR */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv2_input_power = num;
            break;
        case 18: /* SSSS */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv1_input_voltage = num;
            break;
        case 19: /* TTTT */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv2_input_voltage = num;
            break;
        case 20: /* U */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->settings_values_changed = num == 1;
            break;
        case 21: /* V */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->mppt1_charger_status = num;
            break;
        case 22: /* W */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->mppt2_charger_status = num;
            break;
        case 23: /* X */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->load_connected = num == 1;
            break;
        case 24: /* Y */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->battery_power_direction = num;
            break;
        case 25: /* Z */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->dc_ac_power_direction = num;
            break;
        case 26: /* a */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->line_power_direction = num;
            break;
        case 27: /* b */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->local_parallel_id = num;
            break;
    }
}
P18_UNPACK_FN(general_status)
{
    P18_PARSE_LIST_AND_RETURN(general_status, 28)
}

P18_UNPACK_FN(working_mode)
{
    char s[4];
    strncpy(s, data, 2);
    s[2] = '\0';

    p18_working_mode_msg_t m;
    m.mode = atoi(s);

    return m;
}

P18_PARSE_CB_FN(faults_warnings)
{
    p18_faults_warnings_msg_t *m = (p18_faults_warnings_msg_t *)message_ptr;
    int num = atoi(value);
    P18_EXPECT_LISTITEM_LENGTH(index == 0 ? 2 : 1);
    switch (index) {
        case 0:
            m->fault_code = num;
            break;
        case 1:
            m->line_fail = num > 0;
            break;
        case 2:
            m->output_circuit_short = num > 0;
            break;
        case 3:
            m->inverter_over_temperature = num > 0;
            break;
        case 4:
            m->fan_lock = num > 0;
            break;
        case 5:
            m->battery_voltage_high = num > 0;
            break;
        case 6:
            m->battery_low = num > 0;
            break;
        case 7:
            m->battery_under = num > 0;
            break;
        case 8:
            m->over_load = num > 0;
            break;
        case 9:
            m->eeprom_fail = num > 0;
            break;
        case 10:
            m->power_limit = num > 0;
            break;
        case 11:
            m->pv1_voltage_high = num > 0;
            break;
        case 12:
            m->pv2_voltage_high = num > 0;
            break;
        case 13:
            m->mppt1_overload_warning = num > 0;
            break;
        case 14:
            m->mppt2_overload_warning = num > 0;
            break;
        case 15:
            m->battery_too_low_to_charge_for_scc1 = num > 0;
            break;
        case 16:
            m->battery_too_low_to_charge_for_scc2 = num > 0;
            break;
    }
}
P18_UNPACK_FN(faults_warnings)
{
    P18_PARSE_LIST_AND_RETURN(faults_warnings, 17)
}

P18_PARSE_CB_FN(flags_statuses)
{
    p18_flags_statuses_msg_t *m = (p18_flags_statuses_msg_t *)message_ptr;
    int num = atoi(value);
    P18_EXPECT_LISTITEM_LENGTH(1);
    switch (index) {
        case 0:
            m->buzzer = num > 0;
            break;
        case 1:
            m->overload_bypass = num > 0;
            break;
        case 2:
            m->lcd_escape_to_default_page_after_1min_timeout = num > 0;
            break;
        case 3:
            m->overload_restart = num > 0;
            break;
        case 4:
            m->over_temp_restart = num > 0;
            break;
        case 5:
            m->backlight_on = num > 0;
            break;
        case 6:
            m->alarm_on_primary_source_interrupt = num > 0;
            break;
        case 7:
            m->fault_code_record = num > 0;
            break;
    }
}
P18_UNPACK_FN(flags_statuses)
{
    P18_PARSE_LIST_AND_RETURN(flags_statuses, 9)
}

P18_PARSE_CB_FN(defaults)
{
    p18_defaults_msg_t *m = (p18_defaults_msg_t *)message_ptr;
    int num = atoi(value);
    switch (index) {
        case 0: /* AAAA */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_voltage = num;
            break;
        case 1: /* BBB */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_output_freq = num;
            break;
        case 2: /* C */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->ac_input_voltage_range = num;
            break;
        case 3: /* DDD */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_under_voltage = num;
            break;
        case 4: /* EEE */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->charging_float_voltage = num;
            break;
        case 5: /* FFF */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->charging_bulk_voltage = num;
            break;
        case 6: /* GGG */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_recharge_voltage = num;
            break;
        case 7: /* HHH */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_redischarge_voltage = num;
            break;
        case 8: /* III */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->max_charging_current = num;
            break;
        case 9: /* JJ */
            P18_EXPECT_LISTITEM_LENGTH(2);
            m->max_ac_charging_current = num;
            break;
        case 10: /* K */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->battery_type = num;
            break;
        case 11: /* L */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->output_source_priority = num;
            break;
        case 12: /* M */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->charger_source_priority = num;
            break;
        case 13: /* N */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->solar_power_priority = num;
            break;
        case 14: /* O */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->machine_type = num;
            break;
        case 15: /* P */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->output_model_setting = num;
            break;
        case 16: /* S */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_buzzer = num > 0;
            break;
        case 17: /* T */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_overload_restart = num > 0;
            break;
        case 18: /* U */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_over_temp_restart = num > 0;
            break;
        case 19: /* V */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_backlight_on = num > 0;
            break;
        case 20: /* W */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_alarm_on_primary_source_interrupt = num > 0;
            break;
        case 21: /* X */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_fault_code_record = num > 0;
            break;
        case 22: /* Y */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_overload_bypass = num > 0;
            break;
        case 23: /* Z */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->flag_lcd_escape_to_default_page_after_1min_timeout = num > 0;
            break;
    }
}
P18_UNPACK_FN(defaults)
{
    P18_PARSE_LIST_AND_RETURN(defaults, 24)
}

P18_PARSE_CB_FN(max_charging_current_selectable_values)
{
    UNUSED(value_len);
    p18_max_charging_current_selectable_values_msg_t *m =
        (p18_max_charging_current_selectable_values_msg_t *)message_ptr;
    if (m->len >= ARRAY_SIZE(m->amps))
        ERROR("warning: %dth item, ignoring as we accepting up to %zu values\n",
              index, ARRAY_SIZE(m->amps));
    else
        m->amps[m->len++] = atoi(value);
}
P18_UNPACK_FN(max_charging_current_selectable_values)
{
    P18_PARSE_LIST_AND_RETURN(max_charging_current_selectable_values, -1)
}

P18_PARSE_CB_FN(max_ac_charging_current_selectable_values)
{
    UNUSED(value_len);
    p18_max_ac_charging_current_selectable_values_msg_t *m =
            (p18_max_ac_charging_current_selectable_values_msg_t *)message_ptr;
    if (m->len >= ARRAY_SIZE(m->amps))
        ERROR("warning: %dth item, ignoring as we accepting up to %zu values\n",
              index, ARRAY_SIZE(m->amps));
    else
        m->amps[m->len++] = atoi(value);
}
P18_UNPACK_FN(max_ac_charging_current_selectable_values)
{
    P18_PARSE_LIST_AND_RETURN(max_ac_charging_current_selectable_values, -1)
}

P18_PARSE_CB_FN(parallel_rated_information)
{
    p18_parallel_rated_information_msg_t *m = (p18_parallel_rated_information_msg_t *)message_ptr;
    switch (index) {
        case 0: /* A */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->parallel_id_connection_status = atoi(value);
            break;
        case 1: /* BB */
            P18_EXPECT_LISTITEM_LENGTH(2);
            m->serial_number_valid_length = atoi(value);
            break;
        case 2: /* CCCCCCCCCCCCCCCCCCCC */
            P18_EXPECT_LISTITEM_LENGTH(20);
            substr_copy(m->serial_number, value, MIN(m->serial_number_valid_length, (int)ARRAY_SIZE(m->serial_number)-1));
            break;
        case 3: /* D */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->charger_source_priority = atoi(value);
            break;
        case 4: /* EEE */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->max_charging_current = atoi(value);
            break;
        case 5: /* FF */
            P18_EXPECT_LISTITEM_LENGTH(2);
            m->max_ac_charging_current = atoi(value);
            break;
        case 6: /* G */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->output_model_setting = atoi(value);
            break;
    }
}
P18_UNPACK_FN(parallel_rated_information)
{
    P18_PARSE_LIST_AND_RETURN(parallel_rated_information, 7)
}

P18_PARSE_CB_FN(parallel_general_status)
{
    p18_parallel_general_status_msg_t *m = (p18_parallel_general_status_msg_t *)message_ptr;
    int num = atoi(value);
    switch (index) {
        case 0: /* A */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->parallel_id_connection_status = num;
            break;
        case 1: /* B */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->work_mode = num;
            break;
        case 2: /* CC */
            P18_EXPECT_LISTITEM_LENGTH(2);
            m->fault_code = num;
            break;
        case 3: /* DDDD */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->grid_voltage = num;
            break;
        case 4: /* EEE */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->grid_freq = num;
            break;
        case 5: /* FFFF */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_voltage = num;
            break;
        case 6: /* GGG */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->ac_output_freq = num;
            break;
        case 7: /* HHHH */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_apparent_power = num;
            break;
        case 8: /* IIII */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->ac_output_active_power = num;
            break;
        case 9: /* JJJJJ */
            P18_EXPECT_LISTITEM_LENGTH(5);
            m->total_ac_output_apparent_power = num;
            break;
        case 10: /* KKKKK */
            P18_EXPECT_LISTITEM_LENGTH(5);
            m->total_ac_output_active_power = num;
            break;
        case 11: /* LLL */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->output_load_percent = num;
            break;
        case 12: /* MMM */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->total_output_load_percent = num;
            break;
        case 13: /* NNN */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_voltage = num;
            break;
        case 14: /* OOO */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_discharge_current = num;
            break;
        case 15: /* PPP */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_charging_current = num;
            break;
        case 16: /* QQQ */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->total_battery_charging_current = num;
            break;
        case 17: /* MMM. It's not my mistake, it's as per the doc. */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->battery_capacity = num;
            break;
        case 18: /* RRRR */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv1_input_power = num;
            break;
        case 19: /* SSSS */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv2_input_power = num;
            break;
        case 20: /* TTTT */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv1_input_voltage = num == 1;
            break;
        case 21: /* UUUU */
            P18_EXPECT_LISTITEM_LENGTH(4);
            m->pv2_input_voltage = num;
            break;
        case 22: /* V */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->mppt1_charger_status = num;
            break;
        case 23: /* W */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->mppt2_charger_status = num;
            break;
        case 24: /* X */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->load_connected = num == 1;
            break;
        case 25: /* Y */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->battery_power_direction = num;
            break;
        case 26: /* Z */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->dc_ac_power_direction = num;
            break;
        case 27: /* a */
            P18_EXPECT_LISTITEM_LENGTH(1);
            m->line_power_direction = num;
            break;
        case 28: /* bbb. It's marked in red in the doc, idk what it means. */
                 /* My guess is that this elem is not always present. */
            P18_EXPECT_LISTITEM_LENGTH(3);
            m->max_temp = num;
            break;
    }
}
P18_UNPACK_FN(parallel_general_status)
{
    P18_PARSE_LIST_AND_RETURN(parallel_general_status, 29)
}

P18_PARSE_CB_FN(ac_charge_time_bucket)
{
    p18_ac_charge_time_bucket_msg_t *m = (p18_ac_charge_time_bucket_msg_t *)message_ptr;
    char buf[4];
    switch (index) {
        case 0: /* AAAA */
            P18_EXPECT_LISTITEM_LENGTH(4);

            substr_copy(buf, value, 2);
            m->start_h = atoi(buf);

            substr_copy(buf, value+2, 2);
            m->start_m = atoi(buf);

            break;

        case 1: /* BBBB */
            P18_EXPECT_LISTITEM_LENGTH(4);

            substr_copy(buf, value, 2);
            m->end_h = atoi(buf);

            substr_copy(buf, value+2, 2);
            m->end_m = atoi(buf);

            break;
    }
}
P18_UNPACK_FN(ac_charge_time_bucket)
{
    P18_PARSE_LIST_AND_RETURN(ac_charge_time_bucket, 2)
}

P18_PARSE_CB_FN(ac_supply_load_time_bucket)
{
    p18_ac_supply_load_time_bucket_msg_t *m = (p18_ac_supply_load_time_bucket_msg_t *)message_ptr;
    char buf[4];
    switch (index) {
        case 0: /* AAAA */
            P18_EXPECT_LISTITEM_LENGTH(4);

            substr_copy(buf, value, 2);
            m->start_h = atoi(buf);

            substr_copy(buf, value+2, 2);
            m->start_m = atoi(buf);

            break;

        case 1: /* BBBB */
            P18_EXPECT_LISTITEM_LENGTH(4);

            substr_copy(buf, value, 2);
            m->end_h = atoi(buf);

            substr_copy(buf, value+2, 2);
            m->end_m = atoi(buf);

            break;
    }
}
P18_UNPACK_FN(ac_supply_load_time_bucket)
{
    P18_PARSE_LIST_AND_RETURN(ac_supply_load_time_bucket, 2)
}

/* ------------------------------------------ */
/* Label getters */

const char *p18_battery_type_label(p18_battery_type_t type)
{
    switch (type) {
        case P18_BT_AGM:
        case P18_BT_USER:
        case P18_BT_FLOODED:
            return p18_battery_types[type];

        default:
            return p18_unknown;
    }
}

const char *p18_input_voltage_range_label(p18_input_voltage_range_t range)
{
    switch (range) {
        case P18_IVR_APPLIANCE:
        case P18_IVR_UPS:
            return p18_voltage_ranges[range];

        default:
            return p18_unknown;
    }
}

const char *p18_output_source_priority_label(p18_output_source_priority_t priority)
{
    switch (priority) {
        case P18_OSP_SOLAR_BATTERY_UTILITY:
        case P18_OSP_SOLAR_UTILITY_BATTERY:
            return p18_output_source_priorities[priority];

        default:
            return p18_unknown;
    }
}

const char *p18_charge_source_priority_label(p18_charger_source_priority_t priority)
{
    switch (priority) {
        case P18_CSP_SOLAR_FIRST:
        case P18_CSP_SOLAR_AND_UTILITY:
        case P18_CSP_SOLAR_ONLY:
            return p18_charger_source_priorities[priority];

        default:
            return p18_unknown;
    }
}

const char *p18_machine_type_label(p18_machine_type_t type)
{
    switch (type) {
        case P18_MT_GRID_TIE:
        case P18_MT_OFF_GRID_TIE:
            return p18_machine_types[type];

        default:
            return p18_unknown;
    }
}

const char *p18_topology_label(p18_topology_t topology)
{
    switch (topology) {
        case P18_TRANSFORMER:
        case P18_TRANSFORMERLESS:
            return p18_topologies[topology];

        default:
            return p18_unknown;
    }
}

const char *p18_output_model_setting_label(p18_output_model_setting_t setting)
{
    switch (setting) {
        case P18_OMS_PARALLEL_OUTPUT:
        case P18_OMS_PHASE1_OF_3PHASE_OUTPUT:
        case P18_OMS_PHASE2_OF_3PHASE_OUTPUT:
        case P18_OMS_PHASE3_OF_3PHASE_OUTPUT:
        case P18_OMS_SINGLE_MODULE:
            return p18_output_model_settings[setting];

        default:
            return p18_unknown;
    }
}

const char *p18_solar_power_priority_label(p18_solar_power_priority_t priority)
{
    switch (priority) {
        case P18_SPP_BATTERY_LOAD_UTILITY:
        case P18_SPP_LOAD_BATTERY_UTILITY:
            return p18_solar_power_priorities[priority];

        default:
            return p18_unknown;
    }
}

const char *p18_mppt_charger_status_label(p18_mppt_charger_status_t status)
{
    switch (status) {
        case P18_MPPT_CS_ABNORMAL:
        case P18_MPPT_CS_CHARGED:
        case P18_MPPT_CS_NOT_CHARGED:
            return p18_mppt_charger_statuses[status];

        default:
            return p18_unknown;
    }
}

const char *p18_battery_power_direction_label(p18_battery_power_direction_t direction)
{
    switch (direction) {
        case P18_BPD_CHARGE:
        case P18_BPD_DISCHARGE:
        case P18_BPD_DONOTHING:
            return p18_battery_power_directions[direction];

        default:
            return p18_unknown;
    }
}

const char *p18_dc_ac_power_direction_label(p18_dc_ac_power_direction_t direction)
{
    switch (direction) {
        case P18_DAPD_AC_DC:
        case P18_DAPD_DC_AC:
        case P18_DAPD_DONOTHING:
            return p18_dc_ac_power_directions[direction];

        default:
            return p18_unknown;
    }
}

const char *p18_line_power_direction_label(p18_line_power_direction_t direction)
{
    switch (direction) {
        case P18_LPD_DONOTHING:
        case P18_LPD_INPUT:
        case P18_LPD_OUTPUT:
            return p18_line_power_directions[direction];

        default:
            return p18_unknown;
    }
}

const char *p18_working_mode_label(p18_working_mode_t mode)
{
    switch (mode) {
        case P18_WM_BATTERY_MODE:
        case P18_WM_BYPASS_MODE:
        case P18_WM_FAULT_MODE:
        case P18_WM_HYBRID_MODE:
        case P18_WM_POWER_ON_MODE:
        case P18_WM_STANDBY_MODE:
            return p18_working_modes[mode];

        default:
            return p18_unknown;
    }
}

const char *p18_fault_code_label(unsigned int code)
{
    int len = (int)ARRAY_SIZE(fault_codes);
    for (int i = 0; i < len; i++) {
        const p18_fault_code_list_item_t *fc = &fault_codes[i];
        if (fc->id == code)
            return fc->message;
    }
    return p18_unknown;
}

const char *p18_parallel_connection_status_label(p18_parallel_id_connection_status_t status)
{
    switch (status) {
        case P18_PCS_NON_EXISTENT:
        case P18_PCS_EXISTENT:
            return p18_parallel_connection_statuses[status];

        default:
            return p18_unknown;
    }
}