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

#define MODULE_DCC_FUNCTION_INDEX_MAX  28

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

      if( !dcc->enabled ) // reset packet
      {
        static const uint8_t dcc_reset[] = { 0x00 , 0x00 };
        dcc_data = dcc_reset;
        length = sizeof( dcc_reset );
      }
      else if( dcc->get_packet_callback )
      {
        dcc->get_packet_callback( module , &dcc_data , &length );
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
      }
      else // idle packet
      {
        static const uint8_t dcc_idle[] = { 0xff , 0x00 };
        dcc_data = dcc_idle;
        length = sizeof( dcc_idle );
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

int librailcan_dcc_emergency_stop( struct librailcan_module* module , uint16_t address )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_list_get( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  module_dcc_packet_set_speed( module , packet , packet->speed_steps , -1 );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_set_speed( struct librailcan_module* module , uint16_t address , uint8_t value )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_list_get( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  int8_t speed;
  enum dcc_speed_steps speed_steps;


  if( value & LIBRAILCAN_DCC_SPEED_128 )
  {
    speed_steps = dcc_128;
    speed = value & 0x7f;
  }
  else if( ( value & 0xc0 ) == LIBRAILCAN_DCC_SPEED_28 )
  {
    speed_steps = dcc_28;
    speed = value & 0x1f;
  }
  else if( ( value & 0xe0 ) == LIBRAILCAN_DCC_SPEED_14 )
  {
    speed_steps = dcc_14;
    speed = value & 0x0f;
  }
  else
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  if( speed > speed_steps )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  module_dcc_packet_set_speed( module , packet , speed_steps , value );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_set_direction( struct librailcan_module* module , uint16_t address , uint8_t value )
{
  if( !module || ( value != LIBRAILCAN_DCC_DIRECTION_FORWARD && value != LIBRAILCAN_DCC_DIRECTIOM_REVERSE ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_list_get( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  module_dcc_packet_set_direction( module , packet , value == LIBRAILCAN_DCC_DIRECTION_FORWARD ? dcc_forward : dcc_reverse );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_set_function( struct librailcan_module* module , uint16_t address , uint8_t index , uint8_t value )
{
  if( !module || index > MODULE_DCC_FUNCTION_INDEX_MAX || ( value != LIBRAILCAN_DCC_FUNCTION_DISABLED && value != LIBRAILCAN_DCC_FUNCTION_ENABLED ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  enum dcc_packet_type type = module_dcc_get_type_by_function_index( index );

  if( ( r = module_dcc_packet_list_get( module , address , type , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , type , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  module_dcc_packet_set_function( module , packet , index , value == LIBRAILCAN_DCC_FUNCTION_ENABLED );

  return LIBRAILCAN_STATUS_SUCCESS;
}
