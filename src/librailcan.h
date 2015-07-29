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

#include <stddef.h>
#ifdef STDINT_H
  #include STDINT_H
#else
  #include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIBRAILCAN_STATUS_SUCCESS         0
#define LIBRAILCAN_STATUS_TIMEOUT         1
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
 * \param[in] timeout timeout in milliseconds, or negative if you want to wait forever
 * \return \ref librailcan_status "Status code".
 */
int librailcan_bus_process( struct librailcan_bus* bus , int timeout );

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

typedef void(*librailcan_dcc_get_packet_callback)( struct librailcan_module* module , const void** data , uint8_t* length );

struct librailcan_dcc_stats
{
  size_t total_packets_send;
  size_t reset_packets_send;
  size_t user_packets_send;
  size_t queue_packets_send;
  size_t idle_packets_send;
  size_t queue_packet_count;
  size_t list_packet_count;
};

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

int librailcan_dcc_get_stats( struct librailcan_module* module , struct librailcan_dcc_stats* stats , size_t stats_size );

/**
 * \defgroup module_dcc_locomotive_decoders Locomotive decoders
 * \{
 *   \brief Functions to control locomotive decoders.
 *
 *   \section module_dcc_locomotive_decoders_addresses Addresses
 *   Locomotive decoders can be addressed using short or long addresses.
 *   To indicate the kind of address used, the address must be OR-ed with the #LIBRAILCAN_DCC_ADDRESS_SHORT or #LIBRAILCAN_DCC_ADDRESS_LONG flag.
 *   The short address range is \c 1 ... \c 127, the long address range is \c 0 ... \c 10239.
 */

#define LIBRAILCAN_DCC_DIRECTION_FORWARD  1 //!< Forward operation. \see librailcan_dcc_set_direction
#define LIBRAILCAN_DCC_DIRECTIOM_REVERSE  0 //!< Reverse operation. \see librailcan_dcc_set_direction

#define LIBRAILCAN_DCC_ADDRESS_SHORT  0x0000 //!< 7 bit address
#define LIBRAILCAN_DCC_ADDRESS_LONG   0x8000 //!< 14 bit address

#define LIBRAILCAN_DCC_SPEED_128  0x80 //!< Flag to select 128 speed steps. \see librailcan_dcc_set_speed
#define LIBRAILCAN_DCC_SPEED_28   0x40 //!< Flag to select 28 speed steps. \see librailcan_dcc_set_speed
#define LIBRAILCAN_DCC_SPEED_14   0x20 //!< Flag to select 14 speed steps. \see librailcan_dcc_set_speed

#define LIBRAILCAN_DCC_FUNCTION_DISABLED  0 //!< Disable function. \see librailcan_dcc_set_function
#define LIBRAILCAN_DCC_FUNCTION_ENABLED   1 //!< Enable function. \see librailcan_dcc_set_function

/**
 * \brief Emergency stop locomotive.
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_locomotive_decoders_addresses "short or long address"
 * \return \ref librailcan_status "Status code".
 * \par Example
 * Emergency stop locomotive with long address 2414:
 * \code{.c}
 * r = librailcan_dcc_emergency_stop( module , LIBRAILCAN_DCC_ADDRESS_LONG | 2414 );
 * \endcode
 * \see librailcan_dcc_set_speed
 */
int librailcan_dcc_emergency_stop( struct librailcan_module* module , uint16_t address );

/**
 * \brief Set locomotive speed.
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_locomotive_decoders_addresses "short or long address"
 * \param[in] value decoder speed step OR-ed with speed step selection flag
 * \return \ref librailcan_status "Status code".
 * \par Example
 * Select speed step 5 of 28 for locomotive with short address 3:
 * \code{.c}
 * r = librailcan_dcc_set_speed( module , LIBRAILCAN_DCC_ADDRESS_SHORT | 3 , LIBRAILCAN_DCC_SPEED_28 | 5 );
 * \endcode
 * \see librailcan_dcc_emergency_stop
 * \see LIBRAILCAN_DCC_SPEED_128
 * \see LIBRAILCAN_DCC_SPEED_28
 * \see LIBRAILCAN_DCC_SPEED_14
 */
int librailcan_dcc_set_speed( struct librailcan_module* module , uint16_t address , uint8_t value );

/**
 * \brief Set locomotive direction.
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_locomotive_decoders_addresses "short or long address"
 * \param[in] value #LIBRAILCAN_DCC_DIRECTION_FORWARD or #LIBRAILCAN_DCC_DIRECTIOM_REVERSE
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_direction( struct librailcan_module* module , uint16_t address , uint8_t value );

/**
 * \brief Enable or disable locomotive function.
 *
 * \param[in] module a module handle
 * \param[in] address a \ref module_dcc_locomotive_decoders_addresses "short or long address"
 * \param[in] index function index: \c 0 ... \c 28
 * \param[in] value #LIBRAILCAN_DCC_FUNCTION_ENABLED or #LIBRAILCAN_DCC_FUNCTION_DISABLED
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_function( struct librailcan_module* module , uint16_t address , uint8_t index , uint8_t value );

/**
 * \}
 * \defgroup module_dcc_accessory_decoders Accessory decoders
 * \{
 *   \brief Functions to control accessory decoders.
 */

#define LIBRAILCAN_DCC_BASIC_ACCESSORY_OUTPUT_OFF  0 //!< Enable output. \see librailcan_dcc_set_basic_accessory_output
#define LIBRAILCAN_DCC_BASIC_ACCESSORY_OUTPUT_ON   1 //!< Disable output. \see librailcan_dcc_set_basic_accessory_output

#define LIBRAILCAN_DCC_EXTENDED_ACCESSORY_STATE_MIN   0 //!< \see librailcan_dcc_set_extended_accessory_state
#define LIBRAILCAN_DCC_EXTENDED_ACCESSORY_STATE_MAX   31 //!< \see librailcan_dcc_set_extended_accessory_state

/**
 * \brief Enable or disable basic accessory output.
 *
 * \param[in] module a module handle
 * \param[in] address decoder address: \c 0 ... \c 511
 * \param[in] index output index: \c 0 ... \c 7
 * \param[in] value #LIBRAILCAN_DCC_BASIC_ACCESSORY_OUTPUT_OFF or #LIBRAILCAN_DCC_BASIC_ACCESSORY_OUTPUT_ON
 * \return \ref librailcan_status "Status code".
 */
int librailcan_dcc_set_basic_accessory_output( struct librailcan_module* module , uint16_t address , uint8_t index , librailcan_bool value );

/**
 * \brief Set extended accessory state.
 *
 * \param[in] module a module handle
 * \param[in] address decoder address: \c 0 ... \c 2047
 * \param[in] value state: \c 0 ... \c 31
 * \return \ref librailcan_status "Status code".
 * \see LIBRAILCAN_DCC_EXTENDED_ACCESSORY_STATE_MIN
 * \see LIBRAILCAN_DCC_EXTENDED_ACCESSORY_STATE_MAX
 */
int librailcan_dcc_set_extended_accessory_state( struct librailcan_module* module , uint16_t address , uint8_t value );

/**
 * \}
 * \}
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif
