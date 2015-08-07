/**
 * This file is part of the librailcan library.
 *
 * Copyright (C) 2015 Reinder Feenstra <reinderfeenstra@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef _MODULE_DCC_TYPES_H_
#define _MODULE_DCC_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include "librailcan.h"
  #include <string.h> // for: size_t

enum dcc_packet_type
{
  dcc_idle ,
  dcc_speed_and_direction ,
  dcc_f0_f4 ,
  dcc_f5_f8 ,
  dcc_f9_f12 ,
  dcc_f13_f20 ,
  dcc_f21_f28 ,
  dcc_locomotive_disposable ,
  dcc_basic_accessory ,
  dcc_basic_accessory_disposable  ,
  dcc_extended_accessory ,
  dcc_extended_accessory_disposable
};

enum dcc_speed_steps
{
  dcc_14 = 14 ,
  dcc_28 = 28 ,
  dcc_128 = 126
};

enum dcc_direction
{
  dcc_reverse ,
  dcc_forward
};

#define DCC_PACKET_TTL_INFINITE   (-1)
#define DCC_PACKET_TTL_REMOVE     10
#define DCC_PACKET_TTL_F13_F28    5
#define DCC_PACKET_TTL_DISPOSABLE 1
#define DCC_PACKET_TTL_ACCESSORY  2

struct dcc_packet
{
  uint16_t address;
  enum dcc_packet_type type;
  enum dcc_speed_steps speed_steps;
  uint8_t data[8];
  uint8_t data_length;
  int8_t ttl; //!< Number of times to send before removing from the queue or \c DCC_PACKET_TTL_INFINITE.
  bool remove; //!< Remove packet from the list when \c ttl reaches zero.
  struct dcc_packet* previous;
  struct dcc_packet* next;
};

struct module_dcc
{
  bool enabled;
  struct
  {
    struct dcc_packet** items;
    size_t length;
    size_t count;
  } packet_list;
  struct dcc_packet* packet_priority_queue;
  struct dcc_packet* packet_queue;
  librailcan_dcc_get_packet_callback get_packet_callback;
  struct librailcan_dcc_stats stats;
};

#endif
