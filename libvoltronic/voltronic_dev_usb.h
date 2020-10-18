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

#ifndef __VOLTRONIC__DEV__USB__H__
#define __VOLTRONIC__DEV__USB__H__

  #include "voltronic_dev.h"

  /**
   * Create an opaque pointer to a voltronic device connected over USB
   *
   * vendor_id - Device vendor id to search for. ie. 0x0665
   * product_id - Device product id to search for. ie. 0x5161
   *
   * Returns an opaque pointer to a voltronic device or 0 if an error occurred
   *
   * Function sets errno (POSIX)/LastError (Windows) to approriate error on failure 
   */
  voltronic_dev_t voltronic_usb_create(
    const unsigned int vendor_id,
    const unsigned int product_id);

#endif
