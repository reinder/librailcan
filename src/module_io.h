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

#ifndef _MODULE_IO_H_
#define _MODULE_IO_H_

#include "module.h"

int module_io_init( struct librailcan_module* module , const railcan_message_info_t* info );
void module_io_free( struct librailcan_module* module );
int module_io_open( struct librailcan_module* module );
void module_io_close( struct librailcan_module* module );
void module_io_received( struct librailcan_module* module , uint32_t id , int8_t dlc , const void* data );

#endif
