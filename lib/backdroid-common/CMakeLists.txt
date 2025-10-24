project(backdroid-common VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if((NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux") AND (NOT DEFINED libusb_USE_STATIC_LIBS))
    set(libusb_USE_STATIC_LIBS YES)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DOS_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/include
)

set(common_RESOURCES
  qml/common.qrc
)
set(common_SRCS
  src/common_wrapper.cpp
  src/adb_wrapper.cpp
  src/fastboot_wrapper.cpp
  src/sed_wrapper.cpp
  src/sqlite3.cpp
  src/android_device.cpp
  src/async_process.cpp
  src/devicewatcher.cpp
)

set(common_HDRS
  include/common_wrapper.h
  include/adb_wrapper.h
  include/fastboot_wrapper.h
  include/sed_wrapper.h
  include/sqlite3.h
  include/android_device.h
  include/async_process.h
  include/devicewatcher.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
  # set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})
endif()
find_package(Qt6 COMPONENTS Core DBus Sql Xml REQUIRED)
add_library(backdroid-common STATIC ${common_HDRS} ${common_SRCS}  ${common_RESOURCES})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(backdroid-common PRIVATE 
    udev
    usb-1.0
    Qt6::Core
    Qt6::DBus
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(backdroid-common PRIVATE 
    qdevicewatcher
    usb-1.0
    Qt6::Core
    Qt6::Sql
    Qt6::Xml
    Qt6::QSQLiteDriverPlugin
    Qt6::DBus
  )
endif()

get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # install(TARGETS ${PROJECT_NAME}
	# 	LIBRARY DESTINATION lib)
  install(FILES ${CMAKE_HOME_DIRECTORY}/bin/linux/sed
		DESTINATION bin/)
  install(FILES ${CMAKE_HOME_DIRECTORY}/bin/linux/adb
		DESTINATION bin/)
	install(FILES ${CMAKE_HOME_DIRECTORY}/bin/linux/fastboot
		DESTINATION bin/)
  # get_target_property(SHARED_LIB_PATH usb-1.0 IMPORTED_LOCATION_RELEASE)
  # get_filename_component(filename "${SHARED_LIB_PATH}" NAME)
  # get_filename_component(filename_we "${SHARED_LIB_PATH}" NAME_WE)
  # string(REGEX REPLACE "^(.*)\\.so\\.([0-9]+)[.]([0-9]+)[.]([0-9]+)$" "\\2" major ${filename})
  
  # file(COPY /usr/lib/${filename} DESTINATION ${CMAKE_HOME_DIRECTORY}/build/appdir/usr/lib/)
  # file(RENAME 
  #   ${CMAKE_HOME_DIRECTORY}/build/appdir/usr/lib/${filename} 
  #   ${CMAKE_HOME_DIRECTORY}/build/appdir/usr/lib/${filename_we}.so.${major}
  # )
  # install(CODE "execute_process(COMMAND ${CMAKE_HOME_DIRECTORY}/build.sh WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY})")
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/linux/adb DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/linux/fastboot DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/linux/sed DESTINATION ${OUTPUT_PATH})
  
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/windows/adb.exe DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/windows/fastboot.exe DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/windows/AdbWinApi.dll DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/windows/AdbWinUsbApi.dll DESTINATION ${OUTPUT_PATH})
  file(COPY ${CMAKE_HOME_DIRECTORY}/bin/windows/sed.exe DESTINATION ${OUTPUT_PATH})
endif()
