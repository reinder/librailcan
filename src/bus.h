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

#ifndef _BUS_H_
#define _BUS_H_

#include "librailcan.h"
#include <string.h>

enum bus_interface
{
  if_custom ,
  if_socketcan
};

struct librailcan_bus
{
  enum bus_interface interface;
  union
  {
    struct
    {

    } custom;
    struct
    {
      int fd;
      struct
      {
        struct can_queue_item* front;
        struct can_queue_item* rear;
      } send_queue;
    } socketcan;
  };
  librailcan_bus_send send;
  struct librailcan_module** modules;
  size_t modules_length;
  size_t module_count;
  librailcan_bus_scan_callback scan_callback;
  void* user_data;
};

int bus_open( enum bus_interface interface , struct librailcan_bus** bus );
int bus_add_module( struct librailcan_bus* bus , struct librailcan_module* module );
void bus_received( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data );

#endif
