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

#ifndef _MODULE_DCC_PACKET_H_
#define _MODULE_DCC_PACKET_H_

#include "module_dcc_types.h"

int module_dcc_packet_create( struct librailcan_module* module , uint16_t address , enum dcc_packet_type type , struct dcc_packet** packet );
int module_dcc_packet_clone( struct librailcan_module* module , struct dcc_packet* packet_src , struct dcc_packet** packet );
void module_dcc_packet_delete( struct librailcan_module* module , struct dcc_packet* packet );

int module_dcc_packet_list_add( struct librailcan_module* module , struct dcc_packet* packet );
void module_dcc_packet_list_remove( struct librailcan_module* module , struct dcc_packet* packet );
int module_dcc_packet_list_get( struct librailcan_module* module , uint16_t address , enum dcc_packet_type type , struct dcc_packet** packet );

void module_dcc_packet_queue_move_front( struct librailcan_module* module , struct dcc_packet* packet );
void module_dcc_packet_queue_remove( struct librailcan_module* module , struct dcc_packet* packet );

void module_dcc_packet_change_speed_steps( struct librailcan_module* module , struct dcc_packet* packet , enum dcc_speed_steps speed_steps );

void module_dcc_packet_set_speed( struct librailcan_module* module , struct dcc_packet* packet , enum dcc_speed_steps speed_steps , int8_t speed );

void module_dcc_packet_set_direction( struct librailcan_module* module , struct dcc_packet* packet , enum dcc_direction direction );

enum dcc_packet_type module_dcc_get_type_by_function_index( uint8_t index );
void module_dcc_packet_set_function( struct librailcan_module* module , struct dcc_packet* packet , uint8_t index , bool enabled );

void module_dcc_packet_update_ttl_and_flags( struct dcc_packet* packet );

int module_dcc_write_cv( struct librailcan_module* module , struct dcc_packet* packet , uint16_t cv , uint8_t value );
int module_dcc_write_cv_bit( struct librailcan_module* module , struct dcc_packet* packet , uint16_t cv , uint8_t bit , librailcan_bool value );

#endif
