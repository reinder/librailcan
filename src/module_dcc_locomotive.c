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

#include "module.h"
#include <stdlib.h>
#include "module_dcc_packet.h"

#define MODULE_DCC_LOCOMOTIVE_FUNCTION_INDEX_MAX  28

static bool is_valid_address( uint16_t address )
{
  if( address & LIBRAILCAN_DCC_LOCOMOTIVE_ADDRESS_LONG )
    return ( address & ~LIBRAILCAN_DCC_LOCOMOTIVE_ADDRESS_LONG ) <= 10239;
  else // short
    return ( address >= 1 ) && ( address <= 127 );
}

int librailcan_dcc_locomotive_emergency_stop( struct librailcan_module* module , uint16_t address )
{
  if( !module || !is_valid_address( address ) )
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

int librailcan_dcc_locomotive_set_speed( struct librailcan_module* module , uint16_t address , uint8_t value )
{
  if( !module || !is_valid_address( address ) )
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

  if( value & LIBRAILCAN_DCC_LOCOMOTIVE_SPEED_128 )
  {
    speed_steps = dcc_128;
    speed = value & 0x7f;
  }
  else if( ( value & 0xc0 ) == LIBRAILCAN_DCC_LOCOMOTIVE_SPEED_28 )
  {
    speed_steps = dcc_28;
    speed = value & 0x1f;
  }
  else if( ( value & 0xe0 ) == LIBRAILCAN_DCC_LOCOMOTIVE_SPEED_14 )
  {
    speed_steps = dcc_14;
    speed = value & 0x0f;
  }
  else
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  if( speed > speed_steps )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  module_dcc_packet_set_speed( module , packet , speed_steps , speed );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_locomotive_set_direction( struct librailcan_module* module , uint16_t address , uint8_t value )
{
  if( !module || !is_valid_address( address ) || ( value != LIBRAILCAN_DCC_LOCOMOTIVE_DIRECTION_FORWARD && value != LIBRAILCAN_DCC_LOCOMOTIVE_DIRECTIOM_REVERSE ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_list_get( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  module_dcc_packet_set_direction( module , packet , value == LIBRAILCAN_DCC_LOCOMOTIVE_DIRECTION_FORWARD ? dcc_forward : dcc_reverse );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_locomotive_set_function( struct librailcan_module* module , uint16_t address , uint8_t index , uint8_t value )
{
  if( !module || !is_valid_address( address ) || index > MODULE_DCC_LOCOMOTIVE_FUNCTION_INDEX_MAX || ( value != LIBRAILCAN_DCC_LOCOMOTIVE_FUNCTION_DISABLED && value != LIBRAILCAN_DCC_LOCOMOTIVE_FUNCTION_ENABLED ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet = NULL;
  int r;

  enum dcc_packet_type type = module_dcc_get_type_by_function_index( index );

  if( index == 0 )
  {
    if( ( r = module_dcc_packet_list_get( module , address , dcc_speed_and_direction , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
      return r;

    if( packet && packet->speed_steps == dcc_14 )
      type = dcc_speed_and_direction; // F0 is in speed and direction instruction when using 14 speed steps.
    else
      packet = NULL;
  }

  if( !packet && ( r = module_dcc_packet_list_get( module , address , type , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , type , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  module_dcc_packet_set_function( module , packet , index , value == LIBRAILCAN_DCC_LOCOMOTIVE_FUNCTION_ENABLED );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_locomotive_write_cv( struct librailcan_module* module , uint16_t address , uint16_t cv , uint8_t value )
{
  if( !module || !is_valid_address( address ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_create( module , address , dcc_locomotive_disposable , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  uint8_t code;
  switch( cv )
  {
    case 23:
      code = 0x2;
      break;

    case 24:
      code = 0x3;
      break;

    default: // Use long form.
      if( ( r = module_dcc_write_cv( module , packet , cv , value ) ) != LIBRAILCAN_STATUS_SUCCESS )
        free( packet );

      return r;
  }

  // Use short form:
  packet->data[ packet->data_length++ ] = 0xf0 | code; // Configuration Variable Access Instruction - Short Form (1111CCCC) - CCCC = code
  packet->data[ packet->data_length++ ] = value;

  module_dcc_packet_queue_move_front( module , packet );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_dcc_locomotive_write_cv_bit( struct librailcan_module* module , uint16_t address , uint16_t cv , uint8_t bit , librailcan_bool value )
{
  if( !module || !is_valid_address( address ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_create( module , address , dcc_locomotive_disposable , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  if( ( r = module_dcc_write_cv_bit( module , packet , cv , bit , value ) ) != LIBRAILCAN_STATUS_SUCCESS )
  {
    free( packet );
    return r;
  }

  return LIBRAILCAN_STATUS_SUCCESS;
}
