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

#ifndef ISV_UTIL_H
#define ISV_UTIL_H

#include <stdbool.h>
#include <stdio.h>

#include "util.h"

extern bool g_verbose;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define UNUSED(x)     (void)(x)
#define MIN(x, y)     ((x) < (y) ? (x) : (y))

#define LOG(f_, ...) \
    if (g_verbose) fprintf(stderr, (f_), ##__VA_ARGS__)

#define ERROR(f_, ...) \
    fprintf(stderr, (f_), ##__VA_ARGS__)

#define HEXDUMP(mem, len) \
    if (g_verbose) hexdump((mem), (len))

#define FOREACH_OFSIZE(item, array, len) \
    for (int loop_keep = 1, loop_count = 0; \
        loop_keep && loop_count != (int)(len); \
        loop_keep = !loop_keep, loop_count++) \
        for (item = (array) + loop_count; loop_keep; loop_keep = !loop_keep)

#define FOREACH(item, array) \
    FOREACH_OFSIZE(item, array, ARRAY_SIZE(array))

void hexdump(void *mem, unsigned int len);
void substr_copy(char *dst, const char *src, int len);
bool isnumeric(const char *s);
bool isdatevalid(int y, int m, int d);
bool instrarray(const char *needle, const char **list, size_t list_size, int *index);

#endif //ISV_UTIL_H
