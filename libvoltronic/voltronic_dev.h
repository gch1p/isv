/**
 * Copyright (C) 2019  Johan van der Vyver
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

#ifndef __VOLTRONIC__DEV__H__
#define __VOLTRONIC__DEV__H__

#include <stddef.h>

/**
 * Opaque pointer to a voltronic device
 */
typedef void* voltronic_dev_t;

/**
 * Read bytes from a voltronic device
 *
 * dev -> Opaque device pointer
 * buffer -> Buffer where read bytes are written to
 * buffer_size -> Maximum number of bytes to read
 * timeout_milliseconds -> Number of milliseconds to wait for a single byte before giving up
 *
 * Returns the number of bytes read
 * Returns 0 on timeout
 * Returns -1 on error
 *
 * Function sets errno (POSIX)/LastError (Windows) to approriate error on failure
 */
int voltronic_dev_read(
    const voltronic_dev_t dev,
    char* buffer,
    const size_t buffer_size,
    const unsigned int timeout_milliseconds);

/**
 * Write bytes to a voltronic device
 *
 * dev -> Opaque device pointer
 * buffer -> Buffer of bytes to write to device
 * buffer_size -> Number of bytes to write
 * timeout_milliseconds -> Number of milliseconds to wait before giving up
 *
 * Returns the number of bytes written
 * Returns 0 on timeout
 * Returns -1 on error
 *
 * Function sets errno (POSIX)/LastError (Windows) to approriate error on failure
 */
int voltronic_dev_write(
    const voltronic_dev_t dev,
    const char* buffer,
    const size_t buffer_size,
    const unsigned int timeout_milliseconds);

/**
 * Options for voltronic_dev_execute
 */
#define VOLTRONIC_EXECUTE_DEFAULT_OPTIONS    (0)
#define DISABLE_WRITE_VOLTRONIC_CRC                (1 << 0)
#define DISABLE_PARSE_VOLTRONIC_CRC                (1 << 1)
#define DISABLE_VERIFY_VOLTRONIC_CRC             (1 << 2)

/**
 * Write a command to the device and wait for a response from the device
 *
 * dev -> Opaque device pointer
 * send_buffer -> Buffer of bytes to write to device
 * send_buffer_length -> Number of bytes to write
 * receive_buffer -> Buffer where read bytes are written to
 * receive_buffer_length -> Maximum number of bytes to read
 * timeout_milliseconds -> Number of milliseconds to wait before giving up
 * options -> See options above
 *
 * Returns 1 on success and 0 on failure
 *
 * Function sets errno (POSIX)/LastError (Windows) to approriate error on failure
 */
int voltronic_dev_execute(
    const voltronic_dev_t dev,
    const unsigned int options,
    const char *send_buffer,
    size_t send_buffer_length,
    char *receive_buffer,
    size_t receive_buffer_length,
    size_t *received,
    const unsigned int timeout_milliseconds);

/**
 * Close the connection to the device
 *
 * dev -> Opaque device pointer
 */
int voltronic_dev_close(
    voltronic_dev_t dev);

#endif
