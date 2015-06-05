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

#include "socketcan.h"
#include <stdlib.h>
#include <string.h>
#include "librailcan.h"
#include "../shared/railcan-proto/railcan_proto.h"
#include "bus.h"
#include "module.h"
#include "log.h"

int socketcan_send( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data )
{
#ifdef HAVE_LINUX_CAN_H
  LOG_DEBUG( "send: message=%u, address=%u, dlc=%d\n" , RAILCAN_SID_TO_MESSAGE( id ) , RAILCAN_SID_TO_ADDRESS( id ) , dlc );

  struct can_queue_item* item = calloc( 1 , sizeof( *item ) );
  if( !item )
    return LIBRAILCAN_STATUS_NO_MEMORY;

  item->frame.can_id = id & CAN_SFF_MASK;
  if( dlc == LIBRAILCAN_DLC_RTR )
    item->frame.can_id |= CAN_RTR_FLAG;
  else
  {
    item->frame.can_dlc = dlc;
    if( dlc > 0 )
      memcpy( item->frame.data , data , dlc );
  }

  if( !bus->socketcan.send_queue.front )
    bus->socketcan.send_queue.front = item;
  else
    bus->socketcan.send_queue.rear->next = item;

  bus->socketcan.send_queue.rear = item;

  return LIBRAILCAN_STATUS_SUCCESS;
#else
  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
#endif
}
