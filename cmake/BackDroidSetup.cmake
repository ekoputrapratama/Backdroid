

function(stella_include_libraries name)
    foreach(arg IN LISTS ARGN)
      if(arg MATCHES "^Qt6")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT arg MATCHES "DriverPlugin$")
          message("Including library ${arg} to build directory")
          get_target_property(SHARED_LIB_PATH ${arg} IMPORTED_LOCATION_RELEASE)
        
          if(arg MATCHES "DriverPlugin$")
            file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib/sqldrivers)
          else()
            file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib)
          endif()
        
          get_filename_component(ORIG_SO_NAME ${SHARED_LIB_PATH} NAME)
          get_filename_component(SO_NAME ${SHARED_LIB_PATH} NAME_WE)
          file(RENAME 
            ${CMAKE_CURRENT_BINARY_DIR}/lib/${ORIG_SO_NAME} 
            ${CMAKE_CURRENT_BINARY_DIR}/lib/${SO_NAME}.so.5
          )
        endif()
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
          get_target_property(SHARED_LIB_PATH ${arg} IMPORTED_LOCATION_RELEASE)
          file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        endif()
      else()
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
          set(SHARED_LIB_PATH ${SYS_ROOT_PATH}/bin/${arg}.dll)
          file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        endif()
      endif()
    endforeach()
endfunction()


function(stella_include_plugins name)
  foreach(arg IN LISTS ARGN)
    set(SHARED_LIB_PATH ${QT6_ROOT_PATH}/plugins/${arg})
    file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/plugins)
  endforeach()
endfunction()

function(stella_include_qml_plugins name)
  foreach(arg IN LISTS ARGN)
    set(SHARED_LIB_PATH ${QT6_ROOT_PATH}/qml/${arg})
    file(COPY ${SHARED_LIB_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/qml)
  endforeach()
endfunction()

if(NOT CMAKE_CROSSCOMPILING)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(QT6_ROOT_PATH /usr/lib/qt6)
    set(SYS_ROOT_PATH /usr)
  endif()
endif()

function(include_files target)
  get_target_property(target_type ${target} TYPE)
  
  foreach(arg IN LISTS ARGN)
    set(srcFile ${CMAKE_CURRENT_LIST_DIR}/${arg})
    get_filename_component(srcName ${srcFile} NAME)
    
    if(${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "STATIC_LIBRARY")  
      set(destFile ${LIBRARY_OUTPUT_PATH}/${srcName})
    elseif(${target_type} STREQUAL "EXECUTABLE")
      set(destFile ${CMAKE_CURRENT_BINARY_DIR}/${srcName})
    endif()
    message("from ${srcFile} to ${destFile}")
    configure_file(${srcFile} ${destFile} COPYONLY)
  endforeach()
endfunction(include_files)
