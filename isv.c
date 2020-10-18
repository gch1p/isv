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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <assert.h>

#include "variant.h"
#include "p18.h"
#include "util.h"
#include "print.h"
#include "libvoltronic/voltronic_dev_usb.h"

#define COMMAND_BUF_LENGTH  128
#define RESPONSE_BUF_LENGTH 128

#define PRINT(msg_type) \
    { \
        P18_MSG_T(msg_type) m = P18_UNPACK_FN_NAME(msg_type)(buffer+5); \
        PRINT_FN_NAME(msg_type)(&m, g_format); \
    }

#define GET_ARGS(len) \
    get_args(argc, (const char **)argv, a, (len))

bool g_verbose = false;
print_format_t g_format = PRINT_FORMAT_TABLE;

static void usageintlist(const int *list, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%d", list[i]);
        if (i < size-1)
            printf(", ");
    }
}

static void usagestrlist(const char **list, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%s", list[i]);
        if (i < size-1)
            printf(", ");
    }
}

static void write_num(char *buf, int n)
{
    assert(n <= 9);
    *buf++ = n + '0';
    *buf++ = '\0';
}

static void usage(const char *progname)
{
    printf("Usage: %s OPTIONS\n", progname);
    printf("\n"
           "Options:\n"
           "    -h, --help:          print this help\n"
           "    -r <COMMAND>,\n"
           "    --raw <COMMAND>:     execute arbitrary command and print inverter's\n"
           "                         response. Command example: ^P005PI\n"
           "    -t <TIMEOUT>,\n"
           "    --timeout <TIMEOUT>: device read timeout, in milliseconds\n"
           "    -v, --verbose:       print debug information, like hexdumps of\n"
           "                         communication traffic with inverter\n"
           "    -p, --pretend:       do not actually execute command on inverter,\n"
           "                         but output some debug info\n"
           "    -f <FORMAT>,\n"
           "    --format <FORMAT>:   output format for --get and --set options, see below\n"
           "\n"
           "Options to get data from inverter:\n"
           "    --get-protocol-id\n"
           "    --get-date-time\n"
           "    --get-total-generated\n"
           "    --get-year-generated <YYYY>\n"
           "    --get-month-generated <YYYY> <MM>\n"
           "    --get-day-generated <YYYY> <MM> <DD>\n"
           "    --get-series-number\n"
           "    --get-cpu-version\n"
           "    --get-rated-information\n"
           "    --get-general-status\n"
           "    --get-working-mode\n"
           "    --get-faults-warnings\n"
           "    --get-flags\n"
           "    --get-defaults\n"
           "    --get-max-charging-current-selectable-values\n"
           "    --get-max-ac-charging-current-selectable-values\n"
           "    --get-parallel-rated-information <ID>\n"
           "        ID: parallel machine ID\n"
           "\n"
           "    --get-parallel-general-status <ID>\n"
           "        ID: parallel machine ID\n"
           "\n"
           "    --get-ac-charge-time-bucket\n"
           "    --get-ac-supply-load-time-bucket\n"
           "\n"
           "Options to set inverter's configuration:\n"
           "    --set-loads-supply 0|1\n"
           "    --set-flag <FLAG> 0|1\n"
           "    --set-defaults\n"
           "    --set-battery-max-charging-current <ID> <AMPS>\n"
           "      ID: parallel machine ID (use 0 for a single model)\n"
           "      AMPS: use --get-max-charging-current-selectable-values\n"
           "            to see a list of allowed current values\n"
           "\n"
           "    --set-battery-max-ac-charging-current <ID> <AMPS>\n"
           "      ID: parallel machine ID (use 0 for a single model)\n"
           "      AMPS: use --get-max-ac-charging-current-selectable-values\n"
           "            to see a list of allowed current values\n"
           "\n"
           "    --set-ac-output-freq 50|60\n"
           "    --set-battery-max-charging-voltage <CV> <FV>\n"
           "        CV: constant voltage (48.0~58.4)\n"
           "        FV: float voltage (48.0~58.4)\n"
           "\n"
           "    --set-ac-output-rated-voltage <V>\n"
           "        V: one of: "
           );
    usageintlist(p18_ac_output_rated_voltages,
                 ARRAY_SIZE(p18_ac_output_rated_voltages));
    printf("\n\n"
           "    --set-output-source-priority SUB|SBU\n"
           "        SUB for %s\n"
           "        SBU for %s\n",
           p18_output_source_priority_label(P18_OSP_SOLAR_UTILITY_BATTERY),
           p18_output_source_priority_label(P18_OSP_SOLAR_BATTERY_UTILITY));
    printf("\n"
           "    --set-battery-charging-thresholds <CV> <DV>\n"
           "        Sets battery re-charging and re-discharigng voltages when\n"
           "        utility is available.\n"
           "\n"
           "        CV: re-charging voltage\n"
           "            for 12V unit, one of: ");
    usagestrlist(p18_battery_util_recharging_voltages_12v_unit,
                 ARRAY_SIZE(p18_battery_util_recharging_voltages_12v_unit));
    printf("\n"
           "            for 24V unit, one of: ");
    usagestrlist(p18_battery_util_recharging_voltages_24v_unit,
                 ARRAY_SIZE(p18_battery_util_recharging_voltages_24v_unit));
    printf("\n"
           "            for 48V unit, one of: ");
    usagestrlist(p18_battery_util_recharging_voltages_48v_unit,
                 ARRAY_SIZE(p18_battery_util_recharging_voltages_48v_unit));
    printf("\n"
           "        DV: re-discharging voltage\n"
           "            for 12V unit, one of: ");
    usagestrlist(p18_battery_util_redischarging_voltages_12v_unit,
                 ARRAY_SIZE(p18_battery_util_redischarging_voltages_12v_unit));
    printf("\n"
           "            for 24V unit, one of: ");
    usagestrlist(p18_battery_util_redischarging_voltages_24v_unit,
                 ARRAY_SIZE(p18_battery_util_redischarging_voltages_24v_unit));
    printf("\n"
           "            for 48V unit, one of: ");
    usagestrlist(p18_battery_util_redischarging_voltages_48v_unit,
                 ARRAY_SIZE(p18_battery_util_redischarging_voltages_48v_unit));
    printf("\n\n"
           "    --set-charging-source-priority <ID> <PRIORITY>\n"
           "        ID: parallel machine ID (use 0 for a single model)\n"
           "        PRIORITY:\n"
           "            SF: %s,\n"
           "            SU: %s,\n"
           "            S: %s\n",
           p18_charge_source_priority_label(P18_CSP_SOLAR_FIRST),
           p18_charge_source_priority_label(P18_CSP_SOLAR_AND_UTILITY),
           p18_charge_source_priority_label(P18_CSP_SOLAR_ONLY));
    printf("\n"
           "    --set-solar-power-priority BLU|LBU\n"
           "        BLU: %s\n"
           "        LBU: %s\n",
           p18_solar_power_priority_label(P18_SPP_BATTERY_LOAD_UTILITY),
           p18_solar_power_priority_label(P18_SPP_LOAD_BATTERY_UTILITY));
    printf("\n"
           "    --set-ac-input-voltage-range APPLIANCE|UPS\n"
           "    --set-battery-type AGM|FLOODED|USER\n"
           "    --set-output-model <ID> <MODEL>\n"
           "        ID: parallel machine ID (use 0 for a single model)\n"
           "        MODEL:\n"
           "            SM: %s\n"
           "            P: %s\n"
           "            P1: %s\n"
           "            P2: %s\n"
           "            P3: %s\n",
           p18_output_model_setting_label(P18_OMS_SINGLE_MODULE),
           p18_output_model_setting_label(P18_OMS_PARALLEL_OUTPUT),
           p18_output_model_setting_label(P18_OMS_PHASE1_OF_3PHASE_OUTPUT),
           p18_output_model_setting_label(P18_OMS_PHASE2_OF_3PHASE_OUTPUT),
           p18_output_model_setting_label(P18_OMS_PHASE3_OF_3PHASE_OUTPUT));
    printf("\n"
           "    --set-battery-cutoff-voltage <V>\n"
           "        V: cut-off voltage (40.0~48.0)\n"
           "\n"
           "    --set-solar-configuration <ID>\n"
           "        ID: serial number\n"
           "\n"
           "    --clear-generated-data\n"
           "        Clears all data of generated energy.\n"
           "\n"
           "    --set-date-time <YYYY> <MM> <DD> <hh> <mm> <ss>\n"
           "        YYYY: year\n"
           "        MM: month\n"
           "        DD: day\n"
           "        hh: hours\n"
           "        mm: minutes\n"
           "        ss: seconds\n"
           "\n"
           "    --set-ac-charge-time-bucket <START> <END>\n"
           "        START: starting time, hh:mm format\n"
           "        END: ending time, hh:mm format\n"
           "\n"
           "    --set-ac-supply-load-time-bucket <START> <END>\n"
           "        START: starting time, hh:mm format\n"
           "        END: ending time, hh:mm format\n"
           "\n"
    );
    printf("Flags:\n");

    size_t len = ARRAY_SIZE(p18_flags_printable_list);
    for (size_t i = 0; i < len; i++) {
        p18_flag_printable_list_item_t item = p18_flags_printable_list[i];
        printf("    %s: %s\n", item.key, item.title);
    }

    printf("\n"
           "Formats:\n"
           "    table          human-readable table\n"
           "    parsable-table conveniently-parsable table\n"
           "    json           JSON object, like {\"ac_output_voltage\":230}\n"
           "    json-w-units   JSON object with units, like:\n"
           "                   {\"ac_output_voltage\":[230,\"V\"]}\n"
    );

    exit(1);
}

static void exit_with_error(int code, char *fmt, ...)
{
    static const size_t buf_size = 256;
    char buf[buf_size];
    va_list args;
    va_start(args, fmt);
    size_t len = vsnprintf(buf, buf_size, fmt, args);
    va_end(args);
    buf[MIN(len, buf_size-1)] = '\0';
    ERROR("error: %s\n", buf);
    if (g_format == PRINT_FORMAT_JSON || g_format == PRINT_FORMAT_JSON_W_UNITS) {
        print_item_t items[] = {
            {.key= "error", .value= variant_string(buf)}
        };
        print_json(items, 1, false);
    }
    exit(code);
}

static void get_args(int argc, const char **argv, const char **arguments_dst, size_t count)
{
    size_t i = 0;
    arguments_dst[i++] = optarg;
    for (; i < count; i++) {
        if (optind < argc && *argv[optind] != '-')
            arguments_dst[i] = argv[optind++];
        else
            exit_with_error(1, "option %s requires %zu arguments\n",
                            argv[optind-i-1], count);
    }
}

static void execute_raw(voltronic_dev_t dev, const char *command, int timeout)
{
    char buffer[RESPONSE_BUF_LENGTH];
    int result = voltronic_dev_execute(dev, 0, command, strlen(command),
                                   buffer, sizeof(buffer), NULL, timeout);
    if (result <= 0)
        exit_with_error(2, "failed to execute %s: %s\n", command, strerror(errno));
    printf("%s\n", buffer);
}

static void query(voltronic_dev_t dev,
          int command_key,
          int timeout,
          const char **args,
          size_t args_size,
          bool pretend)
{
    char buffer[RESPONSE_BUF_LENGTH];
    char command[COMMAND_BUF_LENGTH];

    if (!p18_build_command(command_key, args, args_size, command))
        exit_with_error(1, "invalid query command %d\n", command_key);

    if (pretend) {
        size_t command_len = strlen(command);
        LOG("would write %zu+3 %s:\n",
            command_len, (command_len > 1 ? "bytes" : "byte"));
        HEXDUMP(command, command_len);
        return;
    }

    size_t received;
    int result = voltronic_dev_execute(dev, 0, command, strlen(command),
                                           buffer, sizeof(buffer), &received,
                                           timeout);
    if (result <= 0)
        exit_with_error(2, "failed to execute %s: %s\n", command, strerror(errno));

    if (command_key < P18_SET_CMDS_ENUM_OFFSET) {
        size_t data_size;
        if (!p18_validate_query_response(buffer, received, &data_size))
            exit_with_error(2, "invalid response\n");

        if (command_key == P18_QUERY_PROTOCOL_ID)
            PRINT(protocol_id)
        else if (command_key == P18_QUERY_CURRENT_TIME)
            PRINT(current_time)
        else if (command_key == P18_QUERY_TOTAL_GENERATED)
            PRINT(total_generated)
        else if (command_key == P18_QUERY_YEAR_GENERATED)
            PRINT(year_generated)
        else if (command_key == P18_QUERY_MONTH_GENERATED)
            PRINT(month_generated)
        else if (command_key == P18_QUERY_DAY_GENERATED)
            PRINT(day_generated)
        else if (command_key == P18_QUERY_SERIES_NUMBER)
            PRINT(series_number)
        else if (command_key == P18_QUERY_CPU_VERSION)
            PRINT(cpu_version)
        else if (command_key == P18_QUERY_RATED_INFORMATION)
            PRINT(rated_information)
        else if (command_key == P18_QUERY_GENERAL_STATUS)
            PRINT(general_status)
        else if (command_key == P18_QUERY_WORKING_MODE)
            PRINT(working_mode)
        else if (command_key == P18_QUERY_FAULTS_WARNINGS)
            PRINT(faults_warnings)
        else if (command_key == P18_QUERY_FLAGS_STATUSES)
            PRINT(flags_statuses)
        else if (command_key == P18_QUERY_DEFAULTS)
            PRINT(defaults)
        else if (command_key == P18_QUERY_MAX_CHARGING_CURRENT_SELECTABLE_VALUES)
            PRINT(max_charging_current_selectable_values)
        else if (command_key == P18_QUERY_MAX_AC_CHARGING_CURRENT_SELECTABLE_VALUES)
            PRINT(max_ac_charging_current_selectable_values)
        else if (command_key == P18_QUERY_PARALLEL_RATED_INFORMATION)
            PRINT(parallel_rated_information)
        else if (command_key == P18_QUERY_PARALLEL_GENERAL_STATUS)
            PRINT(parallel_general_status)
        else if (command_key == P18_QUERY_AC_CHARGE_TIME_BUCKET)
            PRINT(ac_charge_time_bucket)
        else if (command_key == P18_QUERY_AC_SUPPLY_LOAD_TIME_BUCKET)
            PRINT(ac_supply_load_time_bucket)
    } else {
        bool success = p18_set_result(buffer, received);
        print_set_result(success, g_format);
        if (!success)
            exit(2);
    }
}

static void validate_date_args(const char *ys, const char *ms, const char *ds)
{
    static char *err_year = "invalid year";
    static char *err_month = "invalid month";
    static char *err_day = "invalid day";

    int y, m = 0, d = 0;

    /* validate year */
    if (!isnumeric(ys) || strlen(ys) != 4)
        exit_with_error(1, err_year);
    y = (int)strtoul(ys, NULL, 10);
    if (y < 2000 || y > 2099) {
        ERROR("%s\n", err_year);
        exit(1);
    }

    /* validate month */
    if (ms != NULL) {
        if (!isnumeric(ms) || strlen(ms) > 2)
            exit_with_error(1, err_month);
        m = (int)strtoul(ms, NULL, 10);
        if (m < 1 || m > 12)
            exit_with_error(1, err_month);
    }

    /* validate day */
    if (ds != NULL) {
        if (!isnumeric(ds) || strlen(ds) > 2)
            exit_with_error(1, err_day);
        d = (int) strtoul(ds, NULL, 10);
        if (d < 1 || d > 31)
            exit_with_error(1, err_day);
    }

    if (y != 0 && m != 0 && d != 0) {
        if (!isdatevalid(y, m, d))
            exit_with_error(1, "invalid date");
    }
}

static void validate_time_args(const char *hs, const char *ms, const char *ss)
{
    static char *err_hour = "invalid hour";
    static char *err_minute = "invalid minute";
    static char *err_second = "invalid second";

    unsigned int h, m, s;

    if (!isnumeric(hs) || strlen(hs) > 2)
        exit_with_error(1, err_hour);
    h = (unsigned int)strtoul(hs, NULL, 10);
    if (h > 23)
        exit_with_error(1, err_hour);

    if (!isnumeric(ms) || strlen(ms) > 2)
        exit_with_error(1, err_minute);
    m = (unsigned int)strtoul(ms, NULL, 10);
    if (m > 59)
        exit_with_error(1, err_minute);

    if (!isnumeric(ss) || strlen(ss) > 2)
        exit_with_error(1, err_second);
    s = (unsigned int)strtoul(ss, NULL, 10);
    if (s > 59)
        exit_with_error(1, err_second);
}

static bool get_float(const char *s, float *fptr)
{
    char *endptr;
    float f = strtof(s, &endptr);
    if (endptr == s)
        return false;
    if (fptr != NULL)
        *fptr = f;
    return true;
}

static bool get_uint(const char *s, unsigned int *iptr)
{
    char *endptr;
    unsigned int i = (unsigned int)strtoul(s, &endptr, 10);
    if (endptr == s)
        return false;
    if (iptr != NULL)
        *iptr = i;
    return true;
}

static bool is_valid_parallel_id(const char *s)
{
    return isnumeric(s) && strlen(s) == 1;
}

enum action {
    ACTION_HELP,
    ACTION_DUMP,
    ACTION_EXECUTE,
    ACTION_QUERY,
};

enum {
    OPT_HELP = 'h',
    OPT_DUMP = 'd',
    OPT_VERBOSE = 'v',
    OPT_RAW = 'r',
    OPT_PREDENT = 'p',
    OPT_TIMEOUT = 't',
    OPT_FORMAT = 'f',
};

int main(int argc, char *argv[])
{
    if (argv[1] == NULL)
        usage(argv[0]);

    enum action act = ACTION_HELP;
    int opt;
    int command_no = 0, timeout = 1000;
    bool pretend = false;
    const char *a[6] = {0}; /* p18 command arguments */
    static struct option long_options[] = {
        {"help",    no_argument,       0, OPT_HELP},
        {"dump",    no_argument,       0, OPT_DUMP},
        {"verbose", no_argument,       0, OPT_VERBOSE},
        {"raw",     required_argument, 0, OPT_RAW},
        {"pretend", required_argument, 0, OPT_PREDENT},
        {"timeout", required_argument, 0, OPT_TIMEOUT},
        {"format",  required_argument, 0, OPT_FORMAT},

        /* get queries */
        {"get-protocol-id",                               no_argument,       0, P18_QUERY_PROTOCOL_ID},
        {"get-date-time",                                 no_argument,       0, P18_QUERY_CURRENT_TIME},
        {"get-total-generated",                           no_argument,       0, P18_QUERY_TOTAL_GENERATED},
        {"get-year-generated",                            required_argument, 0, P18_QUERY_YEAR_GENERATED},
        {"get-month-generated",                           required_argument, 0, P18_QUERY_MONTH_GENERATED},
        {"get-day-generated",                             required_argument, 0, P18_QUERY_DAY_GENERATED},
        {"get-series-number",                             no_argument,       0, P18_QUERY_SERIES_NUMBER},
        {"get-cpu-version",                               no_argument,       0, P18_QUERY_CPU_VERSION},
        {"get-rated-information",                         no_argument,       0, P18_QUERY_RATED_INFORMATION},
        {"get-general-status",                            no_argument,       0, P18_QUERY_GENERAL_STATUS},
        {"get-working-mode",                              no_argument,       0, P18_QUERY_WORKING_MODE},
        {"get-faults-warnings",                           no_argument,       0, P18_QUERY_FAULTS_WARNINGS},
        {"get-flags",                                     no_argument,       0, P18_QUERY_FLAGS_STATUSES},
        {"get-defaults",                                  no_argument,       0, P18_QUERY_DEFAULTS},
        {"get-max-charging-current-selectable-values",    no_argument,       0, P18_QUERY_MAX_CHARGING_CURRENT_SELECTABLE_VALUES},
        {"get-max-ac-charging-current-selectable-values", no_argument,       0, P18_QUERY_MAX_AC_CHARGING_CURRENT_SELECTABLE_VALUES},
        {"get-parallel-rated-information",                required_argument, 0, P18_QUERY_PARALLEL_RATED_INFORMATION},
        {"get-parallel-general-status",                   required_argument, 0, P18_QUERY_PARALLEL_GENERAL_STATUS},
        {"get-ac-charge-time-bucket",                     no_argument,       0, P18_QUERY_AC_CHARGE_TIME_BUCKET},
        {"get-ac-supply-load-time-bucket",                no_argument,       0, P18_QUERY_AC_SUPPLY_LOAD_TIME_BUCKET},

        /* set queries */
        {"set-loads-supply",                    required_argument, 0, P18_SET_LOADS},
        {"set-flag",                            required_argument, 0, P18_SET_FLAG},
        {"set-defaults",                        no_argument,       0, P18_SET_DEFAULTS},
        {"set-battery-max-charging-current",    required_argument, 0, P18_SET_BAT_MAX_CHARGE_CURRENT},
        {"set-battery-max-ac-charging-current", required_argument, 0, P18_SET_BAT_MAX_AC_CHARGE_CURRENT},
        {"set-ac-output-freq",                  required_argument, 0, P18_SET_AC_OUTPUT_FREQ},
        {"set-battery-max-charging-voltage",    required_argument, 0, P18_SET_BAT_MAX_CHARGE_VOLTAGE},
        {"set-ac-output-rated-voltage",         required_argument, 0, P18_SET_AC_OUTPUT_RATED_VOLTAGE},
        {"set-output-source-priority",          required_argument, 0, P18_SET_OUTPUT_SOURCE_PRIORITY},
        {"set-battery-charging-thresholds",     required_argument, 0, P18_SET_BAT_CHARGING_THRESHOLDS_WHEN_UTILITY_AVAIL},
        {"set-charging-source-priority",        required_argument, 0, P18_SET_CHARGING_SOURCE_PRIORITY},
        {"set-solar-power-priority",            required_argument, 0, P18_SET_SOLAR_POWER_PRIORITY},
        {"set-ac-input-voltage-range",          required_argument, 0, P18_SET_AC_INPUT_VOLTAGE_RANGE},
        {"set-battery-type",                    required_argument, 0, P18_SET_BAT_TYPE},
        {"set-output-model",                    required_argument, 0, P18_SET_OUTPUT_MODEL},
        {"set-battery-cutoff-voltage",          required_argument, 0, P18_SET_BAT_CUTOFF_VOLTAGE},
        {"set-solar-configuration",             required_argument, 0, P18_SET_SOLAR_CONFIG},
        {"clear-generated-data",                no_argument      , 0, P18_SET_CLEAR_GENERATED},
        {"set-date-time",                       required_argument, 0, P18_SET_DATE_TIME},
        {"set-ac-charge-time-bucket",           required_argument, 0, P18_SET_AC_CHARGE_TIME_BUCKET},
        {"set-ac-supply-load-time-bucket",      required_argument, 0, P18_SET_AC_SUPPLY_LOAD_TIME_BUCKET},

        {0, 0, 0, 0}
    };

    bool getopt_err = false;
    while ((opt = getopt_long(argc, argv, "hdvr:pt:f:",
                              long_options, NULL)) != EOF) {
        if (opt == '?') {
            getopt_err = true;
            break;
        }

        if (opt == OPT_HELP)
            act = ACTION_HELP;

        else if (opt == OPT_DUMP)
            act = ACTION_DUMP;

        else if (opt == OPT_VERBOSE)
            g_verbose = true;

        else if (opt == OPT_PREDENT)
            pretend = true;

        else if (opt == OPT_FORMAT) {
            if (!strcmp(optarg, "json"))
                g_format = PRINT_FORMAT_JSON;
            else if (!strcmp(optarg, "json-w-units"))
                g_format = PRINT_FORMAT_JSON_W_UNITS;
            else if (!strcmp(optarg, "table"))
                g_format = PRINT_FORMAT_TABLE;
            else if (!strcmp(optarg, "parsable-table"))
                g_format = PRINT_FORMAT_PARSABLE_TABLE;
            else
                exit_with_error(1, "invalid format");
        }

        else if (opt == OPT_RAW) {
            if (strlen(optarg) > COMMAND_BUF_LENGTH - 1)
                exit_with_error(1, "command is too long");
            a[0] = optarg;
            act = ACTION_EXECUTE;
        }

        else if (opt == OPT_TIMEOUT) {
            timeout = atoi(optarg);
            if (timeout <= 0 || timeout > 60000)
                exit_with_error(1, "invalid timeout");
        }

        else if (opt >= P18_QUERY_CMDS_ENUM_OFFSET) {
            if (act == ACTION_QUERY)
                exit_with_error(1, "one query at a time, please");

            if (opt >= P18_QUERY_CMDS_ENUM_OFFSET) {
                act = ACTION_QUERY;
                command_no = opt;
            }

            switch (opt) {
                case P18_QUERY_YEAR_GENERATED:
                    GET_ARGS(1);
                    validate_date_args(a[0], NULL, NULL);
                    break;

                case P18_QUERY_MONTH_GENERATED:
                    GET_ARGS(2);
                    validate_date_args(a[0], a[1], NULL);
                    break;

                case P18_QUERY_DAY_GENERATED:
                    GET_ARGS(3);
                    validate_date_args(a[0], a[1], a[2]);
                    break;

                case P18_QUERY_PARALLEL_RATED_INFORMATION:
                case P18_QUERY_PARALLEL_GENERAL_STATUS:
                    GET_ARGS(1);
                    if (!isnumeric(a[0]) || strlen(a[0]) > 1)
                        exit_with_error(1, "invalid argument");
                    break;

                case P18_SET_LOADS:
                    GET_ARGS(1);
                    if (strcmp(a[0], "0") != 0 && strcmp(a[0], "1") != 0)
                        exit_with_error(1, "invalid argument, only 0 or 1 allowed");
                    break;

                case P18_SET_FLAG: {
                    GET_ARGS(2);
                    bool matchfound = false;
                    FOREACH (const p18_flag_printable_list_item_t *item,
                        p18_flags_printable_list) {
                        if (!strcmp(item->key, a[0])) {
                            a[0] = item->p18_key;
                            matchfound = true;
                            break;
                        }
                    }
                    if (!matchfound)
                        exit_with_error(1, "invalid flag");
                    if (strcmp(a[1], "0") != 0 &&
                        strcmp(a[1], "1") != 0)
                        exit_with_error(1, "invalid flag state, only 0 or 1 allowed");
                    break;
                }

                case P18_SET_BAT_MAX_CHARGE_CURRENT:
                case P18_SET_BAT_MAX_AC_CHARGE_CURRENT:
                    GET_ARGS(2);
                    if (!is_valid_parallel_id(a[0]))
                        exit_with_error(1, "invalid id");
                    if (!get_uint(a[1], NULL) || strlen(a[1]) > 3)
                        exit_with_error(1, "invalid argument");
                    break;

                case P18_SET_AC_OUTPUT_FREQ:
                    GET_ARGS(1);
                    if (strcmp(a[0], "50") != 0 && strcmp(a[0], "60") != 0)
                        exit_with_error(1, "invalid frequency, only 50 or 60 allowed");
                    break;

                case P18_SET_BAT_MAX_CHARGE_VOLTAGE:
                    GET_ARGS(2);
                    float cv, fv;
                    bool cvr = get_float(a[0], &cv);
                    bool fvr = get_float(a[1], &fv);
                    if (!cvr || cv < 48.0 || cv > 58.4)
                        exit_with_error(1, "invalid CV");
                    if (!fvr || fv < 48.0 || fv > 58.4)
                        exit_with_error(1, "invalid FV");
                    break;

                case P18_SET_AC_OUTPUT_RATED_VOLTAGE: {
                    GET_ARGS(1);
                    unsigned int v;
                    if (!get_uint(a[0], &v))
                        exit_with_error(1, "invalid argument");
                    bool matchfound = false;
                    FOREACH (const int *allowed_v, p18_ac_output_rated_voltages) {
                        if ((unsigned int)*allowed_v == v) {
                            matchfound = true;
                            break;
                        }
                    }
                    if (!matchfound)
                        exit_with_error(1, "invalid voltage");
                    break;

                case P18_SET_OUTPUT_SOURCE_PRIORITY:
                    GET_ARGS(1);
                    const char *allowed[] = {"SUB", "SBU"};
                    int index;
                    if (!instrarray(a[0], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid argument");
                    write_num((char *)a[0], index);
                    break;

                case P18_SET_BAT_CHARGING_THRESHOLDS_WHEN_UTILITY_AVAIL:
                    GET_ARGS(2);
                    if (   !instrarray(a[0], p18_battery_util_recharging_voltages_12v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_12v_unit), NULL)
                        && !instrarray(a[0], p18_battery_util_recharging_voltages_24v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_24v_unit), NULL)
                        && !instrarray(a[0], p18_battery_util_recharging_voltages_48v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_48v_unit), NULL))
                        exit_with_error(1, "invalid CV");
                    if (      !instrarray(a[1], p18_battery_util_redischarging_voltages_12v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_12v_unit), NULL)
                           && !instrarray(a[1], p18_battery_util_redischarging_voltages_24v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_24v_unit), NULL)
                           && !instrarray(a[1], p18_battery_util_redischarging_voltages_48v_unit, ARRAY_SIZE(p18_battery_util_recharging_voltages_48v_unit), NULL))
                        exit_with_error(1, "invalid DV");
                    break;
                }

                case P18_SET_CHARGING_SOURCE_PRIORITY: {
                    GET_ARGS(2);
                    if (!is_valid_parallel_id(a[0]))
                        exit_with_error(1, "invalid id");
                    const char *allowed[] = {"SF", "SU", "S"};
                    int index;
                    if (!instrarray(a[1], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid priority");
                    write_num((char *)a[1], index);
                    break;
                }

                case P18_SET_SOLAR_POWER_PRIORITY: {
                    GET_ARGS(1);
                    const char *allowed[] = {"BLU", "LBU"};
                    int index;
                    if (!instrarray(a[0], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid priority");
                    write_num((char *)a[0], index);
                    break;
                }

                case P18_SET_AC_INPUT_VOLTAGE_RANGE: {
                    GET_ARGS(1);
                    const char *allowed[] = {"APPLIANCE", "UPS"};
                    int index;
                    if (!instrarray(a[0], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid argument");
                    write_num((char *)a[0], index);
                    break;
                }

                case P18_SET_BAT_TYPE: {
                    GET_ARGS(1);
                    const char *allowed[] = {"AGM", "FLOODED", "USER"};
                    int index;
                    if (!instrarray(a[0], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid type");
                    write_num((char *)a[0], index);
                    break;
                }

                case P18_SET_OUTPUT_MODEL: {
                    GET_ARGS(2);
                    if (!is_valid_parallel_id(a[0]))
                        exit_with_error(1, "invalid id");
                    const char *allowed[] = {"SM", "P", "P1", "P2", "P3"};
                    int index;
                    if (!instrarray(a[1], allowed, ARRAY_SIZE(allowed), &index))
                        exit_with_error(1, "invalid model");
                    write_num((char *)a[1], index);
                    break;
                }

                case P18_SET_BAT_CUTOFF_VOLTAGE:
                    GET_ARGS(1);
                    float v;
                    bool vr = get_float(a[0], &v);
                    if (!vr || v < 40.0 || v > 48.0)
                        exit_with_error(1, "invalid voltage");
                    break;

                case P18_SET_SOLAR_CONFIG:
                    GET_ARGS(1);
                    if (!isnumeric(a[0]) || strlen(a[0]) > 20)
                        exit_with_error(1, "invalid argument");
                    break;

                case P18_SET_DATE_TIME:
                    GET_ARGS(6);
                    validate_date_args(a[0], a[1], a[2]);
                    validate_time_args(a[3], a[4], a[5]);
                    break;

                case P18_SET_AC_CHARGE_TIME_BUCKET:
                case P18_SET_AC_SUPPLY_LOAD_TIME_BUCKET:
                    GET_ARGS(2);
                    unsigned short start_h, start_m, end_h, end_m;
                    int results;

                    results = sscanf(a[0], "%hu:%hu", &start_h, &start_m);
                    if (results != 2 || start_h > 23 || start_m > 59)
                        exit_with_error(1, "invalid start time");

                    results = sscanf(a[1], "%hu:%hu", &end_h, &end_m);
                    if (results != 2 || end_h > 23 || end_m > 59)
                        exit_with_error(1, "invalid end time");

                    char *start_col = strchr(a[0], ':');
                    char *end_col = strchr(a[1], ':');
                    *start_col = '\0';
                    *end_col = '\0';

                    char *start_m_ptr = start_col+1;
                    char *end_h_ptr = (char *)a[1];
                    char *end_m_ptr = end_col+1;

                    a[1] = start_m_ptr;
                    a[2] = end_h_ptr;
                    a[3] = end_m_ptr;
                    break;
            }
        }
    }

    if (optind < argc)
        exit_with_error(1, "extra parameter found");

    if (getopt_err)
        exit(1);

    if (act == ACTION_HELP)
        usage(argv[0]);

    voltronic_dev_t dev = voltronic_usb_create(0x0665, 0x5161);

    if (!pretend && !dev)
        exit_with_error(1, "could not open USB device: %s", strerror(errno));

    switch (act) {
        case ACTION_EXECUTE:
            execute_raw(dev, a[0], timeout);
            break;

        case ACTION_QUERY:
            query(dev, command_no, timeout, a, sizeof(a), pretend);
            break;

        default:
            exit_with_error(1, "unexpected act %d", act);
    }

    if (dev)
        voltronic_dev_close(dev);

    return 0;
}