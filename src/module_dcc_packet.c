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

#include "module_dcc_packet.h"
#include <stdlib.h>
#ifdef HAVE_ASSERT_H
#  include <assert.h>
#else
#  define assert( x )
#endif
#include "module.h"

#define DATA_INDEX( packet ) ( ( (packet)->data[0] & 0x80 ) ? 2 : 1 ) //!< get long / short address data index

int module_dcc_packet_create( struct librailcan_module* module , uint16_t address , enum dcc_packet_type type , struct dcc_packet** packet )
{
  // Create a new packet:
  *packet = calloc( 1 , sizeof( **packet ) );

  if( !(*packet) )
    return LIBRAILCAN_STATUS_NO_MEMORY;

  (*packet)->address = address;
  (*packet)->type = type;

  if( address & LIBRAILCAN_DCC_ADDRESS_LONG )
  {
    (*packet)->data[0] = 0x80 | ( ( address >> 8 ) & 0x3f );
    (*packet)->data[1] = address & 0xff;
  }
  else // short address
    (*packet)->data[0] = address & 0x7f;

  int n = DATA_INDEX( *packet );

  switch( type )
  {
    case dcc_speed_and_direction:
      (*packet)->speed_steps = dcc_14;
      (*packet)->data[ n++ ] = 0x61; // Speed and direction instruction (01DFSSSS), D=fwd, S=ESTOP, F=off
      break;

    case dcc_f0_f4:
      (*packet)->data[ n++ ] = 0x80; // Function group one instruction (100xxxxx)
      break;

    case dcc_f5_f8:
      (*packet)->data[ n++ ] = 0xb0; // Function group two instruction (101Sxxxx), (S = 1)
      break;

    case dcc_f9_f12:
      (*packet)->data[ n++ ] = 0xa0; // Function group two instruction (101Sxxxx), (S = 0)
      break;

    case dcc_f13_f20:
      (*packet)->data[ n++ ] = 0xde; // Feature expansion instruction (110CCCCC), F13-F20 function control (CCCCC = 11110)
      (*packet)->data[ n++ ] = 0x00;
      break;

    case dcc_f21_f28:
      (*packet)->data[ n++ ] = 0xdf; // Feature expansion instruction (110CCCCC), F21-F28 function control (CCCCC = 11111)
      (*packet)->data[ n++ ] = 0x00;
      break;
  }

  (*packet)->data_length = n;
  (*packet)->ttl = DCC_PACKET_TTL_INFINITE;

  int r = module_dcc_packet_list_add( module , *packet );
  if( r != LIBRAILCAN_STATUS_SUCCESS )
  {
    free( *packet );
    return r;
  }

  return LIBRAILCAN_STATUS_SUCCESS;
}

void module_dcc_packet_delete( struct librailcan_module* module , struct dcc_packet* packet )
{
  module_dcc_packet_queue_remove( module , packet );
  module_dcc_packet_list_remove( module , packet );
  free( packet );
}

int module_dcc_packet_list_add( struct librailcan_module* module , struct dcc_packet* packet )
{
  struct module_dcc* dcc = module->private_data;

  if( dcc->packet_list.length == dcc->packet_list.count )
  {
    if( dcc->packet_list.length == 0 )
      dcc->packet_list.length = 32;
    else
      dcc->packet_list.length *= 2;

    void* p = realloc( dcc->packet_list.items , dcc->packet_list.length * sizeof( *dcc->packet_list.items ) );

    if( !p )
      return LIBRAILCAN_STATUS_NO_MEMORY;

    dcc->packet_list.items = p;
  }

  dcc->packet_list.items[ dcc->packet_list.count ] = packet;
  dcc->packet_list.count++;

  return LIBRAILCAN_STATUS_SUCCESS;
}

void module_dcc_packet_list_remove( struct librailcan_module* module , struct dcc_packet* packet )
{
  struct module_dcc* dcc = module->private_data;

  for( size_t i = 0 ; i < dcc->packet_list.count ; i++ )
    if( dcc->packet_list.items[ i ] == packet )
    {
      dcc->packet_list.count--;
      if( i < dcc->packet_list.count )
        memmove( dcc->packet_list.items + i , dcc->packet_list.items + i + 1 , ( dcc->packet_list.count - i ) * sizeof( *(dcc->packet_list.items) ) );
      break;
    }
}

int module_dcc_packet_list_get( struct librailcan_module* module , uint16_t address , enum dcc_packet_type type , struct dcc_packet** packet )
{
  if( ( !( address & LIBRAILCAN_DCC_ADDRESS_LONG ) && ( address == 0 || address > 0x7f ) ) ||
      ( ( address & LIBRAILCAN_DCC_ADDRESS_LONG ) && address > 0x3fff ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  struct module_dcc* dcc = module->private_data;

  *packet = NULL;

  for( size_t i = 0 ; i < dcc->packet_list.count ; i++ )
    if( dcc->packet_list.items[ i ]->address == address &&
        dcc->packet_list.items[ i ]->type == type )
    {
      *packet = dcc->packet_list.items[ i ];
      break;
    }

  return LIBRAILCAN_STATUS_SUCCESS;
}

void module_dcc_packet_queue_move_front( struct librailcan_module* module , struct dcc_packet* packet )
{
  struct module_dcc* dcc = module->private_data;

  if( !dcc->packet_queue ) // Queue empty
  {
    dcc->packet_queue = packet;
    packet->previous = packet;
    packet->next = packet;
  }
  else if( dcc->packet_queue != packet )
  {
    if( packet->previous && packet->next )
    {
      // Extract packet from queue:
      packet->next->previous = packet->previous;
      packet->previous->next = packet->next;
    }

    // Add it at front:
    packet->previous = dcc->packet_queue->previous;
    dcc->packet_queue->previous->next = packet;
    packet->next = dcc->packet_queue;
    dcc->packet_queue->previous = packet;
    dcc->packet_queue = packet;
  }
}

void module_dcc_packet_queue_remove( struct librailcan_module* module , struct dcc_packet* packet )
{
  if( packet->previous && packet->next )
  {
    if( packet->next == packet && packet->previous == packet ) // Only one packet in queue
    {
      ((struct module_dcc*)module->private_data)->packet_queue = NULL;
    }
    else // Extract packet from queue:
    {
      packet->next->previous = packet->previous;
      packet->previous->next = packet->next;
    }

    // Clear:
    packet->next = NULL;
    packet->previous = NULL;
  }
}

void module_dcc_packet_set_speed( struct librailcan_module* module , struct dcc_packet* packet , enum dcc_speed_steps speed_steps , int8_t speed )
{
  int n = DATA_INDEX( packet );

  if( packet->speed_steps != speed_steps )
  {
    enum dcc_direction direction;

    if( packet->speed_steps == dcc_128 )
      direction = ( packet->data[n + 1] & 0x80 ) ? dcc_forward : dcc_reverse;
    else
      direction = ( packet->data[n] & 0x20 ) ? dcc_forward : dcc_reverse;

    packet->speed_steps = speed_steps;

    switch( packet->speed_steps )
    {
      case dcc_14:
      case dcc_28:
        packet->data[n] = 0x40; // Speed and direction instruction (01DFSSSS)
        packet->data_length = n + 1;
        break;

      case dcc_128:
        packet->data[n] = 0x9f; // Advanced operations instruction (100CCCCC), 128 Speed Step Control (CCCCC = 11111)
        packet->data[n + 1] = 0x00;
        packet->data_length = n + 2;
        break;
    }

    if( direction == dcc_forward )
    {
      if( packet->speed_steps == dcc_128 )
        packet->data[n + 1] |= 0x80;
      else
        packet->data[n] |= 0x20;
    }
  }

  switch( packet->speed_steps ) // Clear speed bits.
  {
    case dcc_14:
      packet->data[n] &= 0xf0;
      break;
    case dcc_28:
      packet->data[n] &= 0xe0;
      break;
    case dcc_128:
      n++;
      packet->data[n] &= 0x80;
      break;
  }

  if( speed == -1 ) // emergency stop
    packet->data[n] |= 0x01;
  else if( speed > 0 )
  {
    if( packet->speed_steps == dcc_28 )
    {
      speed += 3; // 0x04 => step 1
      packet->data[n] |= speed >> 1;
      if( speed & 0x01 )
        packet->data[n] |= 0x10;
    }
    else
      packet->data[n] |= speed + 1; // 0x02 => step 1
  }

  if( speed == -1 && ( packet->speed_steps != dcc_14 || !( packet->data[n] & 0x10 ) ) ) // emergency stop (and FL disabled)
  {
    packet->ttl = DCC_PACKET_TTL_REMOVE;
    packet->remove = true;
  }
  else
  {
    packet->ttl = DCC_PACKET_TTL_INFINITE;
    packet->remove = false;
  }

  module_dcc_packet_queue_move_front( module , packet );
}

void module_dcc_packet_set_direction( struct librailcan_module* module , struct dcc_packet* packet , enum dcc_direction direction )
{
  int n = DATA_INDEX( packet );
  uint8_t mask = 0;

  switch( packet->speed_steps )
  {
    case dcc_14:
    case dcc_28:
      mask = 0x20;
      break;
    case dcc_128:
      mask = 0x80;
      n++;
      break;
  }

  if( direction == dcc_forward )
    packet->data[n] |= mask;
  else // dcc_reverse
    packet->data[n] &= ~mask;

  module_dcc_packet_queue_move_front( module , packet );
}

enum dcc_packet_type module_dcc_get_type_by_function_index( uint8_t index )
{
  if( index <= 4 )
    return dcc_f0_f4;
  else if( index <= 8 )
    return dcc_f5_f8;
  else if( index <= 12 )
    return dcc_f9_f12;
  else if( index <= 20 )
    return dcc_f13_f20;
  else // index <= 28
    return dcc_f21_f28;
}

void module_dcc_packet_set_function( struct librailcan_module* module , struct dcc_packet* packet , uint8_t index , bool enabled )
{
  int n = DATA_INDEX( packet );
  uint8_t mask = 0;
  uint8_t mask_all = 0;

  switch( packet->type )
  {
    case dcc_speed_and_direction:
      assert( index == 0 && packet->speed_steps == dcc_14 );
      mask = 0x10;
      mask_all =0x1f;
      break;

    case dcc_f0_f4:
      assert( index <= 4 );
      if( index == 0 )
        mask = 0x10;
      else
        mask = 1 << ( index - 1 );
      mask_all = 0x1f;
      break;

    case dcc_f5_f8:
      assert( index >= 5 && index <= 8 );
      mask = 1 << ( index - 5 );
      mask_all = 0x0f;
      break;

    case dcc_f9_f12:
      assert( index >= 9 && index <= 12 );
      mask = 1 << ( index - 9 );
      mask_all = 0x0f;
      break;

    case dcc_f13_f20:
      assert( index >= 13 && index <= 20 );
      mask = 1 << ( index - 13 );
      mask_all = 0xff;
      n++;
      break;

    case dcc_f21_f28:
      assert( index >= 21 && index <= 28 );
      mask = 1 << ( index - 21 );
      mask_all = 0xff;
      n++;
      break;
  }

  if( enabled )
    packet->data[n] |= mask;
  else
    packet->data[n] &= ~mask;

  if( ( packet->data[n] & mask_all ) != ( ( packet->type == dcc_speed_and_direction ) ? 0x01 : 0x00 ) )
  {
    if( packet->type == dcc_f13_f20 || packet->type == dcc_f21_f28 )
      packet->ttl = DCC_PACKET_TTL_F13_F28;
    else
      packet->ttl = DCC_PACKET_TTL_INFINITE;
    packet->remove = false;
  }
  else
  {
    packet->ttl = DCC_PACKET_TTL_REMOVE;
    packet->remove = true;
  }

  module_dcc_packet_queue_move_front( module , packet );
}
