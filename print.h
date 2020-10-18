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

#ifndef ISV_PRINT_H
#define ISV_PRINT_H

#include "p18.h"
#include "variant.h"

#define MAKE_PRINT_FN_NAME(msg_type)  print_ ## msg_type ## _msg
#define PRINT_FN_NAME(msg_type)       MAKE_PRINT_FN_NAME(msg_type)

#define PRINT_FN(msg_type) \
     void PRINT_FN_NAME(msg_type)(const P18_MSG_T(msg_type) *m, print_format_t format)

typedef enum {
    PRINT_UNIT_V = 1,
    PRINT_UNIT_A,
    PRINT_UNIT_WH,
    PRINT_UNIT_KWH,
    PRINT_UNIT_VA,
    PRINT_UNIT_HZ,
    PRINT_UNIT_PERCENTAGE,
    PRINT_UNIT_CELSIUS,
} print_unit_t;

typedef enum {
    PRINT_FORMAT_TABLE,
    PRINT_FORMAT_PARSABLE_TABLE,
    PRINT_FORMAT_JSON,
    PRINT_FORMAT_JSON_W_UNITS,
} print_format_t;

typedef struct {
    char *key;
    char *title;
    variant_t value;
    short precision;
    print_unit_t unit;
} print_item_t;

void print_json(print_item_t *items, size_t size, bool with_units);
void print_set_result(bool success, print_format_t format);
bool print_is_json_format(print_format_t f);

PRINT_FN(protocol_id);
PRINT_FN(current_time);
PRINT_FN(total_generated);
PRINT_FN(year_generated);
PRINT_FN(month_generated);
PRINT_FN(day_generated);
PRINT_FN(series_number);
PRINT_FN(cpu_version);
PRINT_FN(rated_information);
PRINT_FN(general_status);
PRINT_FN(working_mode);
PRINT_FN(faults_warnings);
PRINT_FN(flags_statuses);
PRINT_FN(defaults);
PRINT_FN(max_charging_current_selectable_values);
PRINT_FN(max_ac_charging_current_selectable_values);
PRINT_FN(parallel_rated_information);
PRINT_FN(parallel_general_status);
PRINT_FN(ac_charge_time_bucket);
PRINT_FN(ac_supply_load_time_bucket);

#endif //ISV_PRINT_H
