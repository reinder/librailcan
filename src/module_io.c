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

#include "module_io.h"
#include <stdlib.h>
#include "bus.h"
#include "utils.h"
#include "log.h"

struct module_io
{
  unsigned int digital_input_count;
  librailcan_tristate* digital_inputs;
  librailcan_digital_io_changed_callback digital_input_changed_callback;
  unsigned int digital_output_count;
  librailcan_tristate* digital_outputs;
  librailcan_digital_io_changed_callback digital_output_changed_callback;
};

int module_io_init( struct librailcan_module* module , const railcan_message_info_t* info )
{
  struct module_io* io = calloc( 1 , sizeof( *io ) );
  if( !io )
    goto error;

  io->digital_input_count = info->io.digital_input_count;
  io->digital_output_count = info->io.digital_output_count;

  module->private_data = io;
  module->free = module_io_free;
  module->open = module_io_open;
  module->close = module_io_close;
  module->received = module_io_received;

  return LIBRAILCAN_STATUS_SUCCESS;

error:
  free( io );

  return LIBRAILCAN_STATUS_NO_MEMORY;
}

void module_io_free( struct librailcan_module* module )
{
  free( module->private_data );

  module_free( module );
}

int module_io_open( struct librailcan_module* module )
{
  int r = module_open( module );
  if( r != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  struct module_io* io = module->private_data;

  io->digital_inputs = calloc( io->digital_input_count , sizeof( librailcan_tristate ) );
  if( !io->digital_inputs )
    goto error;

  io->digital_outputs = calloc( io->digital_output_count , sizeof( librailcan_tristate ) );
  if( !io->digital_outputs )
    goto error;

  if( module->bus->send( module->bus , RAILCAN_SID( RAILCAN_SID_MESSAGE_INPUTS , module->address ) , LIBRAILCAN_DLC_RTR , NULL ) != LIBRAILCAN_STATUS_SUCCESS )
    LOG_WARNING( "failed sending input rtr" );

  if( module->bus->send( module->bus , RAILCAN_SID( RAILCAN_SID_MESSAGE_OUTPUTS , module->address ) , LIBRAILCAN_DLC_RTR , NULL ) != LIBRAILCAN_STATUS_SUCCESS )
    LOG_WARNING( "failed sending output rtr" );

  return LIBRAILCAN_STATUS_SUCCESS;

error:
  free( io->digital_inputs );
  free( io->digital_outputs );

  io->digital_inputs = NULL;
  io->digital_outputs = NULL;

  return LIBRAILCAN_STATUS_NO_MEMORY;
}

void module_io_close( struct librailcan_module* module )
{
  struct module_io* io = module->private_data;

  free( io->digital_inputs );
  free( io->digital_outputs );

  io->digital_inputs = NULL;
  io->digital_outputs = NULL;

  module_close( module );
}

void module_io_received( struct librailcan_module* module , uint32_t id , int8_t dlc , const void* data )
{
  struct module_io* io = module->private_data;

  switch( RAILCAN_SID_TO_MESSAGE( id ) )
  {
    case RAILCAN_SID_MESSAGE_INPUTS:
    {
      if( dlc == LIBRAILCAN_DLC_RTR )
        break;

      const uint_fast8_t len = min( io->digital_input_count , dlc * 8 );
      for( uint_fast8_t i = 0 ; i < len ; i++ )
      {
        librailcan_tristate value = ( ((uint8_t*)data)[ i / 8 ] & ( 1 << ( 7 - ( i % 8 ) ) ) ) ? LIBRAILCAN_TRISTATE_TRUE : LIBRAILCAN_TRISTATE_FALSE;
        if( value != io->digital_inputs[ i ] )
        {
          io->digital_inputs[ i ] = value;
          if( io->digital_input_changed_callback )
            io->digital_input_changed_callback( module , i , value );
        }
      }
      break;
    }
    case RAILCAN_SID_MESSAGE_OUTPUTS:
    {
      if( dlc == LIBRAILCAN_DLC_RTR )
        break;

      const uint_fast8_t len = min( io->digital_output_count , dlc * 8 );
      for( uint_fast8_t i = 0 ; i < len ; i++ )
      {
        librailcan_tristate value = ( ((uint8_t*)data)[ i / 8 ] & ( 1 << ( 7 - ( i % 8 ) ) ) ) ? LIBRAILCAN_TRISTATE_TRUE : LIBRAILCAN_TRISTATE_FALSE;
        if( value != io->digital_outputs[ i ] )
        {
          io->digital_outputs[ i ] = value;
          if( io->digital_output_changed_callback )
            io->digital_output_changed_callback( module , i , value );
        }
      }
      break;
    }
    default:
      module_received( module , id , dlc , data );
      break;
  }
}

int librailcan_io_get_digital_input_count( struct librailcan_module* module , unsigned int* count )
{
  if( !module || !count )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  *count = ((struct module_io*)module->private_data)->digital_input_count;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_io_read_digital_input( struct librailcan_module* module , unsigned int index , librailcan_tristate* value )
{
  if( !module || !value )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct module_io* io = module->private_data;

  if( index >= io->digital_input_count )
    return LIBRAILCAN_STATUS_INVALID_INDEX;

  *value = io->digital_inputs[ index ];

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_io_set_digital_input_changed_callback( struct librailcan_module* module , librailcan_digital_io_changed_callback callback )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  ((struct module_io*)module->private_data)->digital_input_changed_callback = callback;

  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
}

int librailcan_io_get_digital_output_count( struct librailcan_module* module , unsigned int* count )
{
  if( !module || !count )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  *count = ((struct module_io*)module->private_data)->digital_output_count;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_io_read_digital_output( struct librailcan_module* module , unsigned int index , librailcan_tristate* value )
{
  if( !module || !value )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct module_io* io = module->private_data;

  if( index >= io->digital_output_count )
    return LIBRAILCAN_STATUS_INVALID_INDEX;

  *value = io->digital_outputs[ index ];

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_io_write_digital_output( struct librailcan_module* module , unsigned int index , librailcan_tristate value )
{
  if( !module || ( value != LIBRAILCAN_TRISTATE_FALSE && value != LIBRAILCAN_TRISTATE_TRUE ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;
  else if( !module->is_active )
    return LIBRAILCAN_STATUS_NOT_ACTIVE;

  struct module_io* io = module->private_data;

  if( index >= io->digital_output_count )
    return LIBRAILCAN_STATUS_INVALID_INDEX;

  if( value != io->digital_outputs[ index ] )
  {
    uint8_t data[8] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };

    for( uint_fast8_t i = 0 ; i < io->digital_output_count ; i++ )
      if( ( i != index && io->digital_outputs[ i ] == LIBRAILCAN_TRISTATE_TRUE ) ||
          ( i == index && value == LIBRAILCAN_TRISTATE_TRUE ) )
        data[ i >> 3 ] |= 1 << ( 7 - ( i & 0x7 ) );

    int r = module->bus->send( module->bus , RAILCAN_SID( RAILCAN_SID_MESSAGE_OUTPUTS , module->address ) , ( io->digital_output_count + 7 ) / 8 , data );

    if( r == LIBRAILCAN_STATUS_SUCCESS )
      io->digital_outputs[ index ] = value;
    else
      LOG_ERROR( "can_send failed: %d" , r );

    return r;
  }

  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
}

int librailcan_io_set_digital_output_changed_callback( struct librailcan_module* module , librailcan_digital_io_changed_callback callback )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_IO )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  ((struct module_io*)module->private_data)->digital_output_changed_callback = callback;

  return LIBRAILCAN_STATUS_SUCCESS;
}
