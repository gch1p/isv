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
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"

#define HEXDUMP_COLS 8

/* based on: https://gist.github.com/richinseattle/c527a3acb6f152796a580401057c78b4 */
void hexdump(void *mem, unsigned int len)
{
    unsigned int i, j;

    for (i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
        /* Print offset */
        if (i % HEXDUMP_COLS == 0)
            printf("0x%06x: ", i);

        if (i < len)
            /* Print hex data */
            printf("%02x ", 0xFF & ((char *) mem)[i]);
        else
            /* End of block, just aligning for ASCII dump */
            printf("   ");

        /* Print ASCII dump */
        if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if (j >= len)
                    /* end of block, not really printing */
                    putchar(' ');
                else if (isprint(((char *) mem)[j]))
                    /* printable char */
                    putchar(0xFF & ((char *) mem)[j]);
                else
                    /* other char */
                    putchar('.');
            }

            putchar('\n');
        }
    }
}

/* dst buffer's length must be at least len+1 */
void substr_copy(char *dst, const char *src, int len)
{
    strncpy(dst, src, len);
    dst[len] = '\0';
}

bool isnumeric(const char *s)
{
    size_t len = strlen(s);
    for (size_t i = 0; i < len; i++) {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

bool isdatevalid(const int y, const int m, const int d)
{
    /* primitive out of range checks */
    if (y < 2000 || y > 2099)
        return false;

    if (d < 1 || d > 31)
        return false;

    if (m < 1 || m > 12)
        return false;

    /* some more clever date validity checks */
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d == 31)
        return false;

    /* and finally a february check */
    /* i always wondered, when do people born at feb 29 celebrate their bday? */
    return m != 2 || ((y % 4 != 0 && d <= 28) || (y % 4 == 0 && d <= 29));
}

bool instrarray(const char *needle, const char **list, size_t list_size, int *index)
{
    bool found = false;
    for (size_t i = 0; i < list_size; i++) {
        /* LOG("%s: comparing %s == %s\n", __func__, list[i], needle); */
        if (!strcmp(list[i], needle)) {
            found = true;
            if (index != NULL)
                *index = (int)i;
            break;
        }
    }
    return found;
}
