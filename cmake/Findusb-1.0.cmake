
set(USB_ROOT_DIR
    "${USB_ROOT_DIR}"
	CACHE
	PATH
    "Directory to search for libusb")

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBUSB libusb-1.0)
endif()

find_library(USB_LIBRARY
	NAMES
    usb-1.0
    usb
	PATHS
	  ${PC_LIBUSB_LIBRARY_DIRS}
	  ${PC_LIBUSB_LIBDIR}
	HINTS
	  "${USB_ROOT_DIR}"
	PATH_SUFFIXES
	  lib
    lib64
	)

get_filename_component(_libdir "${USB_LIBRARY}" PATH)
get_filename_component(_librealpath "${USB_LIBRARY}" REALPATH)

find_path(USB_INCLUDE_DIR
	NAMES
	  libusb.h
	PATHS
    ${PC_LIBUSB_INCLUDE_DIRS}
    ${PC_LIBUSB_INCLUDEDIR}
	HINTS
    "${_libdir}"
    "${_libdir}/.."
    "${USB_ROOT_DIR}"
	PATH_SUFFIXES
	  include/libusb-1.0
	)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(usb-1.0
	DEFAULT_MSG
	USB_LIBRARY
	USB_INCLUDE_DIR
)

add_library(usb-1.0 SHARED IMPORTED)
set_target_properties(usb-1.0 
  PROPERTIES
    IMPORTED_LOCATION_RELEASE ${_librealpath}
    IMPORTED_LOCATION_DEBUG ${_librealpath}
)

if(USB_FOUND)
	list(APPEND USB_LIBRARIES ${USB_LIBRARY})
	list(APPEND USB_INCLUDE_DIRS ${USB_INCLUDE_DIR})
	mark_as_advanced(USB_ROOT_DIR)
endif()

mark_as_advanced(USB_INCLUDE_DIR
	USB_LIBRARY
)

