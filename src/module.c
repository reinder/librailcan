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
#include "module_dcc.h"
#include "module_io.h"
#include "bus.h"
#include "log.h"

int module_init( struct librailcan_module* module , const railcan_message_info_t* info )
{
  module->type = info->node_type;
  module->free = module_free;
  module->open = module_open;
  module->close = module_close;
  module->received = module_received;

  switch( module->type )
  {
    case LIBRAILCAN_MODULETYPE_IO:
      module_io_init( module , info );
      break;

    case LIBRAILCAN_MODULETYPE_DCC:
      module_dcc_init( module , info );
      break;

    default:
      LOG_WARNING( "unknown module type 0x%02x" , module->type );
      return LIBRAILCAN_STATUS_UNSUCCESSFUL;
  }

  return LIBRAILCAN_STATUS_SUCCESS;
}

int module_open( struct librailcan_module* module )
{
  module->is_open = true;

  return LIBRAILCAN_STATUS_SUCCESS;
}

void module_close( struct librailcan_module* module )
{
  module->is_open = false;
}

void module_free( struct librailcan_module* module )
{
  //bus_module_remove( module );
  free( module );
}

void module_received( struct librailcan_module* module , uint32_t id , int8_t dlc , const void* data )
{
}

int librailcan_module_open( struct librailcan_bus* bus , uint8_t address , struct librailcan_module** module )
{
  if( !bus || address < RAILCAN_SID_ADDRESS_FIRST || address > RAILCAN_SID_ADDRESS_LAST || !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  for( int i = 0 ; i < bus->module_count ; i++ )
    if( bus->modules[ i ]->address == address && !bus->modules[ i ]->is_open )
    {
      int r = bus->modules[ i ]->open( bus->modules[ i ] );
      if( r == LIBRAILCAN_STATUS_SUCCESS )
        *module = bus->modules[ i ];
      else
        bus->modules[ i ]->close( bus->modules[ i ] );
      return r;
    }

  return LIBRAILCAN_STATUS_UNSUCCESSFUL;
}

int librailcan_module_close( struct librailcan_module* module )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  module->close( module );

  return LIBRAILCAN_STATUS_SUCCESS;

}

int librailcan_module_get_bus( struct librailcan_module* module , struct librailcan_bus** bus )
{
  if( !module || !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  *bus = module->bus;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_module_get_type( struct librailcan_module* module , uint8_t* type )
{
  if( !module || !type )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  *type = module->type;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_module_get_user_data( struct librailcan_module* module , void** data )
{
  if( !module || !data )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  *data = module->user_data;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_module_set_user_data( struct librailcan_module* module , void* data )
{
  if( !module )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  module->user_data = data;

  return LIBRAILCAN_STATUS_SUCCESS;
}
