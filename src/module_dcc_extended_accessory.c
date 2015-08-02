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
#include "module_dcc_packet.h"

int librailcan_dcc_extended_accessory_set_state( struct librailcan_module* module , uint16_t address , uint8_t value )
{
  if( !module || address >= 1 << 11 || value > LIBRAILCAN_DCC_EXTENDED_ACCESSORY_STATE_MAX )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( module->type != LIBRAILCAN_MODULETYPE_DCC )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  struct dcc_packet* packet;
  int r;

  if( ( r = module_dcc_packet_list_get( module , address , dcc_extended_accessory , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;
  else if( !packet && ( r = module_dcc_packet_create( module , address , dcc_extended_accessory , &packet ) ) != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  packet->data[ 2 ] = value;

  module_dcc_packet_update_ttl_and_flags( packet );

  module_dcc_packet_queue_move_front( module , packet );

  return LIBRAILCAN_STATUS_SUCCESS;
}
