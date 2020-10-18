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

#ifndef ISV_VARIANT_H
#define ISV_VARIANT_H

#include <stdbool.h>

typedef enum {
    VARIANT_TYPE_STRING,
    VARIANT_TYPE_LONG,
    VARIANT_TYPE_DOUBLE,
    VARIANT_TYPE_BOOL,
    VARIANT_TYPE_FLAG,
} variant_type_t;

typedef struct {
    variant_type_t type;
    double d;
    long l;
    bool b;
    const char *s;
} variant_t;

variant_t variant_double(double d);
variant_t variant_long(long l);
variant_t variant_bool(bool b);
variant_t variant_flag(bool b);
variant_t variant_string(const char *s);

void variant_to_string(variant_t v, char *buf, size_t bufsize);

bool variant_is_string(variant_t v);
bool variant_is_long(variant_t v);
bool variant_is_bool(variant_t v);
bool variant_is_flag(variant_t v);
bool variant_is_double(variant_t v);

#endif //ISV_VARIANT_H
