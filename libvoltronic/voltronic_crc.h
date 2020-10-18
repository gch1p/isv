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

#ifndef __VOLTRONIC__CRC__H__
#define __VOLTRONIC__CRC__H__

  #include <stddef.h>

  /**
   * The underlying numeric type used to store the CRC
   */
  #if defined(_WIN32) || defined(WIN32)
    #include "windows.h"

    typedef unsigned __int16 voltronic_crc_t;
  #else
    #include <stdint.h>

    typedef uint16_t voltronic_crc_t;
  #endif

  /**
   * Write a voltronic_crc_t to a buffer
   *
   * crc - CRC to write
   * buffer - Buffer to write CRC to or NULL/0
   *
   * Returns the size of the CRC
   */
  int write_voltronic_crc(
    const voltronic_crc_t crc,
    char* buffer);

  /**
   * Read a voltronic_crc_t from a buffer
   *
   * buffer - Buffer to read CRC from;
   *          If the buffer is smaller than the size
   *          returned by write_voltronic_crc behavoir is undefined
   *
   * Returns the CRC read from the buffer
   */
  voltronic_crc_t read_voltronic_crc(const char* buffer);

  /**
   * Calculate the Voltronic CRC by reading a buffer and calculating the CRC from the bytes
   *
   * buffer - Buffer to read from
   * buffer_length - Number of bytes in the buffer
   *
   * Returns the CRC created from the bytes in the buffer
   */
  voltronic_crc_t calculate_voltronic_crc(
    const char* buffer,
    size_t buffer_length);

#endif
