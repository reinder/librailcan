lib_LTLIBRARIES = librailcan.la

include_HEADERS = librailcan.h

librailcan_la_SOURCES = \
	bus.h \
	bus.c \
	librailcan_internal.h \
	library.c \
	module.h \
	module.c \
	module_dcc.h \
	module_dcc.c \
	module_dcc_locomotive.c \
	module_dcc_basic_accessory.c \
	module_dcc_extended_accessory.c \
	module_dcc_packet.h \
	module_dcc_packet.c \
	module_dcc_types.h \
	module_io.h \
	module_io.c \
	socketcan.h \
	socketcan.c \
	version.c

librailcan_CFLAGS = \
	-fPIC

librailcan_la_LDFLAGS = \
	-no-undefined
