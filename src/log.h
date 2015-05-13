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

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#define LOG( level , ... ) { if( debug_level >= level ) { fprintf( stderr , __VA_ARGS__ ); } }
#define LOG_ERROR( ... ) LOG( LIBRAILCAN_DEBUGLEVEL_ERROR , __VA_ARGS__ )
#define LOG_WARNING( ... ) LOG( LIBRAILCAN_DEBUGLEVEL_WARNING , __VA_ARGS__ )
#define LOG_INFO( ... ) LOG( LIBRAILCAN_DEBUGLEVEL_INFO , __VA_ARGS__ )
#define LOG_DEBUG( ... ) LOG( LIBRAILCAN_DEBUGLEVEL_DEBUG , __VA_ARGS__ )

extern int debug_level;

#endif