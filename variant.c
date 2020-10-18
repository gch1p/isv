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
#include "variant.h"

variant_t variant_double(double d)
{
    variant_t v;
    v.type = VARIANT_TYPE_DOUBLE;
    v.d = d;
    return v;
}

variant_t variant_long(long l)
{
    variant_t v;
    v.type = VARIANT_TYPE_LONG;
    v.l = l;
    return v;
}

variant_t variant_bool(bool b)
{
    variant_t v;
    v.type = VARIANT_TYPE_BOOL;
    v.b = b;
    return v;
}

variant_t variant_flag(bool b)
{
    variant_t v;
    v.type = VARIANT_TYPE_FLAG;
    v.b = b;
    return v;
}

/* this just stores a pointer to a string, it doesn't copy the string itself */
variant_t variant_string(const char *s)
{
    variant_t v;
    v.type = VARIANT_TYPE_STRING;
    v.s = s;
    return v;
}

inline bool variant_is_string(variant_t v)
{
    return v.type == VARIANT_TYPE_STRING;
}

inline bool variant_is_long(variant_t v)
{
    return v.type == VARIANT_TYPE_LONG;
}

inline bool variant_is_bool(variant_t v)
{
    return v.type == VARIANT_TYPE_BOOL;
}

inline bool variant_is_flag(variant_t v)
{
    return v.type == VARIANT_TYPE_FLAG;
}

inline bool variant_is_double(variant_t v)
{
    return v.type == VARIANT_TYPE_DOUBLE;
}