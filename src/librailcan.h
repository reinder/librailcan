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

#ifndef _LIBRAILCAN_H_
#define _LIBRAILCAN_H_

#ifdef STDINT_H
  #include STDINT_H
#else
  #include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIBRAILCAN_STATUS_SUCCESS         0
#define LIBRAILCAN_STATUS_UNSUCCESSFUL   -1
#define LIBRAILCAN_STATUS_NO_MEMORY      -2
#define LIBRAILCAN_STATUS_NOT_SUPPORTED  -3
#define LIBRAILCAN_STATUS_INVALID_PARAM  -4
#define LIBRAILCAN_STATUS_INVALID_INDEX  -5
#define LIBRAILCAN_STATUS_NOT_ACTIVE     -6

#define LIBRAILCAN_DEBUGLEVEL_NONE     0
#define LIBRAILCAN_DEBUGLEVEL_ERROR    1
#define LIBRAILCAN_DEBUGLEVEL_WARNING  2
#define LIBRAILCAN_DEBUGLEVEL_INFO     3
#define LIBRAILCAN_DEBUGLEVEL_DEBUG    4

typedef uint8_t librailcan_bool;

#define LIBRAILCAN_BOOL_FALSE  0
#define LIBRAILCAN_BOOL_TRUE   1

/**
 * \brief ...
 *
 * \param[in] level ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_set_debug_level( int level );

/**
 * \defgroup version Version
 * \{
 */

struct librailcan_version
{
  const uint16_t major;
  const uint16_t minor;
  const uint16_t patch;
  const char* extra;
};

/**
 * \brief Get library version.
 *
 * \return Pointer to library version struct.
 */
const struct librailcan_version* librailcan_get_version( void );

/**
 * \brief Get protocol version.
 *
 * \return Pointer to protocol version struct.
 */
const struct librailcan_version* librailcan_get_protocol_version( void );

/**
 * \}
 * \defgroup bus Bus
 * \{
 */

#define LIBRAILCAN_DLC_RTR  -1

struct librailcan_bus;

typedef int(*librailcan_bus_send)( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data );
typedef int(*librailcan_bus_scan_callback)( struct librailcan_bus* bus , uint8_t address , uint8_t type );

int librailcan_bus_open_custom( librailcan_bus_send send , struct librailcan_bus** bus );

/**
 * \brief Open a RailCAN bus.
 *
 * \param[in] interface SocketCAN interface name, e.g. \c can0
 * \param[out] bus ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_open_socketcan( const char* interface , struct librailcan_bus** bus );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_close( struct librailcan_bus* bus );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \param[out] fd ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_get_fd( struct librailcan_bus* bus , int* fd );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \param[out] events ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_get_poll_events( struct librailcan_bus* bus , short* events );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \param[in] revents ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_process_poll( struct librailcan_bus* bus , short revents );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \param[in] id
 * \param[in] dlc data lenght \c 0..8 or #LIBRAILCAN_DLC_RTR.
 * \param[in] data
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_received( struct librailcan_bus* bus , uint32_t id , int8_t dlc , const void* data );

/**
 * \brief Scan bus for modules.
 *
 * \param[in] bus a bus handle
 * \param[in] start ...
 * \param[in] end ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_scan( struct librailcan_bus* bus , uint8_t start , uint8_t end );

/**
 * \brief ...
 *
 * \param[in] bus a bus handle
 * \param[in] callback ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_set_scan_callback( struct librailcan_bus* bus , librailcan_bus_scan_callback callback );

/**
 * \brief Get user supplied bus data.
 *
 * \param[in] bus a bus handle
 * \param[out] data ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_get_user_data( struct librailcan_bus* bus , void** data );

/**
 * \brief Set user bus data.
 *
 * \param[in] bus a bus handle
 * \param[in] data ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_set_user_data( struct librailcan_bus* bus , void* data );

/**
 * \}
 * \defgroup module Module
 * \{
 */

#define LIBRAILCAN_MODULETYPE_UNKNOWN  0x00
#define LIBRAILCAN_MODULETYPE_IO       0x01
#define LIBRAILCAN_MODULETYPE_DCC      0x02

struct librailcan_module;

/**
 * \brief Open a module.
 *
 * \param[in] bus a bus handle
 * \param[in] address module address
 * \param[out] module ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_open( struct librailcan_bus* bus , uint8_t address , struct librailcan_module** module );

/**
 * \brief Close a module.
 *
 * \param[in] module a module handle
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_close( struct librailcan_module* module );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] bus ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_get_bus( struct librailcan_module* module , struct librailcan_bus** bus );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[out] type ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_get_type( struct librailcan_module* module , uint8_t* type );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[out] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_get_active( struct librailcan_module* module , librailcan_bool* value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_set_active( struct librailcan_module* module , librailcan_bool value );

/**
 * \brief Get user supplied module data.
 *
 * \param[in] module a module handle
 * \param[out] data ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_get_user_data( struct librailcan_module* module , void** data );

/**
 * \brief Set user module data.
 *
 * \param[in] module a module handle
 * \param[in] data ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_module_set_user_data( struct librailcan_module* module , void* data );

/**
 * \defgroup module_io IO
 * \{
 */

#define LIBRAILCAN_TRISTATE_UNDEFINED  0
#define LIBRAILCAN_TRISTATE_FALSE      1
#define LIBRAILCAN_TRISTATE_TRUE       2

typedef uint8_t librailcan_tristate;

typedef void(*librailcan_digital_io_changed_callback)( struct librailcan_module* module , unsigned int index , librailcan_tristate value );

/**
 * \brief Get number of digital inputs.
 *
 * \param[in] module a module handle
 * \param[out] count number of digital inputs
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_get_digital_input_count( struct librailcan_module* module , unsigned int* count );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] index ...
 * \param[out] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_read_digital_input( struct librailcan_module* module , unsigned int index , librailcan_tristate* value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] callback pointer to callback function or \c NULL to disable.
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_set_digital_input_changed_callback( struct librailcan_module* module , librailcan_digital_io_changed_callback callback );

/**
 * \brief Get number of digital outputs.
 *
 * \param[in] module a module handle
 * \param[out] count number of digital inputs
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_get_digital_output_count( struct librailcan_module* module , unsigned int* count );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] index ...
 * \param[out] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_read_digital_output( struct librailcan_module* module , unsigned int index , librailcan_tristate* value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] index ...
 * \param[in] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_write_digital_output( struct librailcan_module* module , unsigned int index , librailcan_tristate value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] callback ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_io_set_digital_output_changed_callback( struct librailcan_module* module , librailcan_digital_io_changed_callback callback );

/**
 * \}
 * \defgroup module_dcc DCC
 * \{
 */

#define LIBRAILCAN_DCC_DIRECTION_FORWARD  1
#define LIBRAILCAN_DCC_DIRECTIOM_REVERSE  0

#define LIBRAILCAN_DCC_ADDRESS_SHORT  0x0000 //!< 7 bit address
#define LIBRAILCAN_DCC_ADDRESS_LONG   0x8000 //!< 14 bit address

#define LIBRAILCAN_DCC_SPEED_128  0x80
#define LIBRAILCAN_DCC_SPEED_28   0x40
#define LIBRAILCAN_DCC_SPEED_14   0x20

#define LIBRAILCAN_DCC_FUNCTION_DISABLED  0
#define LIBRAILCAN_DCC_FUNCTION_ENABLED   1

typedef void(*librailcan_dcc_get_packet_callback)( struct librailcan_module* module , const void** data , uint8_t* length );

int librailcan_dcc_get_enabled( struct librailcan_module* module , uint8_t* value );

int librailcan_dcc_set_enabled( struct librailcan_module* module , uint8_t value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] callback ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_get_packet_callback( struct librailcan_module* module , librailcan_dcc_get_packet_callback callback );

/**
 * \defgroup module_dcc_decoder Decoder
 * \{
 */

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] address ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_emergency_stop( struct librailcan_module* module , uint16_t address );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_address "DCC address"
 * \param[in] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_speed( struct librailcan_module* module , uint16_t address , uint8_t value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_address "DCC address"
 * \param[in] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_direction( struct librailcan_module* module , uint16_t address , uint8_t value );

/**
 * \brief ...
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_address "DCC address"
 * \param[in] index function index, \c 0 ... \c 28
 * \param[in] value ...
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_function( struct librailcan_module* module , uint16_t address , uint8_t index , uint8_t value );

/**
 * \}
 * \}
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif
