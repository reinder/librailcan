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

#include "bus.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef HAVE_LINUX_CAN_H
#  include <linux/can.h>
#endif
#ifdef HAVE_LINUX_CAN_RAW_H
#  include <linux/can/raw.h>
#endif
#ifdef HAVE_POLL_H
#  include <poll.h>
#endif
#include "module.h"
#include "socketcan.h"
#include "railcan_proto.h"
#include "log.h"

int librailcan_bus_open_custom( librailcan_bus_send send , struct librailcan_bus** bus )
{
  if( !send || !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  int r = bus_open( if_custom , bus );
  if( r != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  (*bus)->send = send;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_open_socketcan( const char* device , struct librailcan_bus** bus )
{
#ifdef HAVE_LINUX_CAN_H
  if( !device || !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  // create socket
  int fd = socket( PF_CAN , SOCK_RAW , CAN_RAW );
  if( fd == -1 )
    goto error_no_socket;

  // locate interface
  struct ifreq ifr;
  strncpy( ifr.ifr_name , device , IFNAMSIZ );
  if( ioctl( fd , SIOCGIFINDEX , &ifr ) == -1 )
    goto error;

  // select interface and bind
  struct sockaddr_can addr;
  addr.can_family  = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  if( bind( fd , (struct sockaddr*)&addr , sizeof( addr ) ) == -1 )
    goto error;

  // select non-blocking mode
  fcntl( fd , F_SETFL , fcntl( fd , F_GETFL , 0 ) | O_NONBLOCK );

#ifdef HAVE_LINUX_CAN_RAW_H
  // setup filter for std frames only
  struct can_filter filter[1];
  filter[0].can_id   = 0;
  filter[0].can_mask = CAN_EFF_FLAG;
  setsockopt( fd , SOL_CAN_RAW , CAN_RAW_FILTER , &filter , sizeof( filter ) );
#endif

  int r = bus_open( if_socketcan , bus );
  if( r != LIBRAILCAN_STATUS_SUCCESS )
    return r;

  (*bus)->socketcan.fd = fd;
  (*bus)->send = socketcan_send;

  return LIBRAILCAN_STATUS_SUCCESS;

error:
  close( fd );

error_no_socket:
  return LIBRAILCAN_STATUS_UNSUCCESSFUL;
#else
  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
#endif
}

int librailcan_bus_close( struct librailcan_bus* bus )
{
  if( !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  if( bus->interface == if_socketcan )
    close( bus->socketcan.fd );

  for( int i = 0 ; i < bus->module_count ; i++ )
    free( bus->modules[ i ] );

  free( bus->modules );

  free( bus );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_get_fd( struct librailcan_bus* bus , int* fd )
{
  if( !bus || !fd )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( bus->interface != if_socketcan )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  *fd = bus->socketcan.fd;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_get_poll_events( struct librailcan_bus* bus , short* events )
{
#if defined( HAVE_POLL_H ) && defined( HAVE_LINUX_CAN_H )
  if( !bus || !events )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( bus->interface != if_socketcan )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  *events = POLLIN;
  if( bus->socketcan.send_queue.front )
    *events |= POLLOUT;
#else
  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
#endif
}

int librailcan_bus_process_poll( struct librailcan_bus* bus , short revents )
{
#if defined( HAVE_POLL_H ) && defined( HAVE_LINUX_CAN_H )
  if( !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( bus->interface != if_socketcan )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  if( revents & POLLOUT )
    while( bus->socketcan.send_queue.front )
    {
      struct can_queue_item* item = bus->socketcan.send_queue.front;

      ssize_t r = write( bus->socketcan.fd , &item->frame , sizeof( item->frame ) );

      if( r == sizeof( item->frame ) )
      {
        bus->socketcan.send_queue.front = item->next;
        free( item );
      }
      else if( r == 0 )
        break;
      else
      {
        LOG_ERROR( "write: r = %zd [%m]\n" , r );
        return LIBRAILCAN_STATUS_UNSUCCESSFUL;
      }
    }

  if( revents & POLLIN )
    while( 1 )
    {
      struct can_frame frame;

      ssize_t r = read( bus->socketcan.fd , &frame , sizeof( frame ) );

      if( r == sizeof( frame ) )
      {
#ifndef HAVE_LINUX_CAN_RAW_H
        if( frame.can_id & CAN_EFF_FLAG ) // ignore extended frames
          continue;
#endif
        bus_received( bus , frame.can_id , ( frame.can_id & CAN_RTR_FLAG ) ? LIBRAILCAN_DLC_RTR : frame.can_dlc , frame.data );
      }
      else if( r == 0 || ( r == -1 && errno == EAGAIN ) )
        break;
      else
      {
        LOG_ERROR( "read: r = %zd [%m]\n" , r );
        return LIBRAILCAN_STATUS_UNSUCCESSFUL;
      }
    }

  return LIBRAILCAN_STATUS_SUCCESS;
#else
  return LIBRAILCAN_STATUS_NOT_SUPPORTED;
#endif
}

int librailcan_bus_received( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data )
{
  if( !bus || id > 0x7ff || dlc < LIBRAILCAN_DLC_RTR || dlc > 8 || ( dlc > 0 && !data ) )
    return LIBRAILCAN_STATUS_INVALID_PARAM;
  else if( bus->interface != if_custom )
    return LIBRAILCAN_STATUS_NOT_SUPPORTED;

  bus_received( bus , id , dlc , data );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_scan( struct librailcan_bus* bus , uint8_t start , uint8_t end )
{
  if( !bus ||
      start < RAILCAN_SID_ADDRESS_FIRST || start > end ||
      end < start || end > RAILCAN_SID_ADDRESS_LAST )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  for( unsigned short address = start ; address <= end ; address++ )
    bus->send( bus , RAILCAN_SID( RAILCAN_SID_MESSAGE_INFO , address ) , LIBRAILCAN_DLC_RTR , NULL );

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_set_scan_callback( struct librailcan_bus* bus , librailcan_bus_scan_callback callback )
{
  if( !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  bus->scan_callback = callback;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_get_user_data( struct librailcan_bus* bus , void** data )
{
  if( !bus || !data )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  *data = bus->user_data;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int librailcan_bus_set_user_data( struct librailcan_bus* bus , void* data )
{
  if( !bus )
    return LIBRAILCAN_STATUS_INVALID_PARAM;

  bus->user_data = data;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int bus_open( enum bus_interface interface , struct librailcan_bus** bus )
{
  *bus = calloc( 1 , sizeof( **bus ) );

  if( !*bus )
    return LIBRAILCAN_STATUS_NO_MEMORY;

  (*bus)->interface = interface;

  return LIBRAILCAN_STATUS_SUCCESS;
}

int bus_add_module( struct librailcan_bus* bus , struct librailcan_module* module )
{
  if( !bus->modules )
  {
    bus->modules_length = 4;
    bus->modules = malloc( bus->modules_length * sizeof( *bus->modules ) );
    if( !bus->modules )
    {
      bus->modules_length = 0;
      return LIBRAILCAN_STATUS_NO_MEMORY;
    }
  }
  else if( bus->module_count == bus->modules_length )
  {
    void* new = realloc( bus->modules , 2 * bus->modules_length * sizeof( *bus->modules ) );
    if( !new )
      return LIBRAILCAN_STATUS_NO_MEMORY;
    bus->modules = new;
    bus->modules_length *= 2;
  }

  bus->modules[ bus->module_count ] = module;
  bus->module_count++;

  return LIBRAILCAN_STATUS_SUCCESS;
}

void bus_received( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data )
{
  const uint8_t address = RAILCAN_SID_TO_ADDRESS( id );

  LOG_DEBUG( "received: message=%u, address=%u, dlc=%d\n" , RAILCAN_SID_TO_MESSAGE( id ) , address , dlc );

  if( address == RAILCAN_SID_ADDRESS_BROADCAST ) // broadcast message
  {
    for( int i = 0 ; i < bus->module_count ; i++ )
      bus->modules[ i ]->received( bus->modules[ i ] , id , dlc , data );
  }
  else if( address >= RAILCAN_SID_ADDRESS_FIRST &&
           address <= RAILCAN_SID_ADDRESS_LAST )
  {
    for( int i = 0 ; i < bus->module_count ; i++ )
      if( bus->modules[ i ]->address == address )
      {
        bus->modules[ i ]->received( bus->modules[ i ] , id , dlc , data );
        return;
      }

    // unknown address, create a new module
    if( RAILCAN_SID_TO_MESSAGE( id ) == RAILCAN_SID_MESSAGE_INFO )
    {
      const railcan_message_info_t* info = (const railcan_message_info_t*) data;
      struct librailcan_module* module = calloc( 1 , sizeof( *module ) );

      if( module )
      {
        module->address = address;
        module->bus = bus;

        if( module_init( module , info ) == LIBRAILCAN_STATUS_SUCCESS )
        {
          LOG_INFO( "new module: address=%u, type=%u\n" , module->address , module->type );
          bus_add_module( bus , module );
          if( bus->scan_callback )
            bus->scan_callback( bus , module->address , module->type );
        }
        else
          free( module );
      }
    }
  }
}
