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

#include "module_dcc.h"
#include <stdlib.h>
#include "bus.h"
#include "module_dcc_packet.h"

int module_dcc_init( struct librailcan_module* module , const railcan_message_info_t* info )
{
  struct module_dcc* dcc = calloc( 1 , sizeof( *dcc ) );
  if( !dcc )
    return LIBRAILCAN_STATUS_NO_MEMORY;

  module->private_data = dcc;
  module->free = module_dcc_free;
  module->received = module_dcc_received;

  return LIBRAILCAN_STATUS_SUCCESS;
}

void module_dcc_free( struct librailcan_module* module )
{
  free( module->private_data );

  module_free( module );
}

void module_dcc_close( struct librailcan_module* module )
{
  struct module_dcc* dcc = module->private_data;

  while( dcc->packet_priority_queue )
  {
    struct dcc_packet* packet = dcc->packet_priority_queue;
    dcc->packet_priority_queue = packet->next;
    free( packet );
  }

  for( int i = 0 ; i < dcc->packet_list.count ; i++ )
    free( dcc->packet_list.items[i] );
  free( dcc->packet_list.items );

  memset( dcc , 0 , sizeof( *dcc ) ); // Reset everything.

  module_close( module );
}

void module_dcc_received( struct librailcan_module* module , uint32_t id , int8_t dlc , const void* data )
{
  switch( RAILCAN_SID_TO_MESSAGE( id ) )
  {
    case RAILCAN_SID_MESSAGE_DCC:
    {
      if( dlc != LIBRAILCAN_DLC_RTR || !module->is_active )
        break;

      struct module_dcc* dcc = module->private_data;

      const void* dcc_data = NULL;
      uint8_t length = 0;

      dcc->stats.total_packets_sent++;

      if( !dcc->enabled ) // reset packet
      {
        static const uint8_t dcc_reset[] = { 0x00 , 0x00 };
        dcc_data = dcc_reset;
        length = sizeof( dcc_reset );

        dcc->stats.reset_packets_sent++;
      }
      else if( dcc->get_packet_callback )
      {
        dcc->get_packet_callback( module , &dcc_data , &length );

        dcc->stats.user_packets_sent++;
      }
      else if( dcc->packet_priority_queue )
      {
        struct dcc_packet* packet = dcc->packet_priority_queue;

        dcc_data = packet->data;
        length = packet->data_length;

        if( --packet->ttl <= 0 )
        {
          dcc->packet_priority_queue = packet->next;
          dcc->stats.priority_queue_packet_count--;
          free( packet );
        }

        dcc->stats.priority_queue_packets_sent++;
      }
      else if( dcc->packet_queue )
      {
        struct dcc_packet* packet = dcc->packet_queue;

        dcc_data = packet->data;
        length = packet->data_length;

        dcc->packet_queue = packet->next;

        if( packet->ttl > 0 && --packet->ttl == 0 )
        {
          if( packet->remove )
            module_dcc_packet_delete( module , packet );
          else
            module_dcc_packet_queue_remove( module , packet );
        }

        dcc->stats.queue_packets_sent++;
      }
      else // idle packet
      {
        static const uint8_t dcc_idle[] = { 0xff , 0x00 };
        dcc_data = dcc_idle;
        length = sizeof( dcc_idle );

        dcc->stats.idle_packets_sent++;
      }

      if( length > 0 && length <= 8 )
        module->bus->send( module->bus , id , length , dcc_data );

      break;
    }
    default:
      module_received( module , id , dlc , data );
  }
}

int librailcan_dcc_get_enabled( struct librailcan_module* module , uint8_t* value )
{
  if( !module || !value )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  *value = ((struct module_dcc*)module->private_data)->enabled;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_set_enabled( struct librailcan_module* module , uint8_t value )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  ((struct module_dcc*)module->private_data)->enabled = value;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_set_get_packet_callback( struct librailcan_module* module , librailcan_dcc_get_packet_callback callback )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  ((struct module_dcc*)module->private_data)->get_packet_callback = callback;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_get_stats( struct librailcan_module* module , struct librailcan_dcc_stats* stats , size_t stats_size )
{
  if( !module || !stats || stats_size < sizeof( *stats ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  memcpy( stats , &((struct module_dcc*)module->private_data)->stats , sizeof( *stats ) );

  return LIBRAILCAN_STATUS_SUCCESS;
}
