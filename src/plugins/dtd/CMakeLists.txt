SET(    bong_dtd_SRC
        dtdfiletype.c
    )

SET(    bong_dtd_HEADERS
        dtdfiletype.h
   )

include_directories(
    ${GOBJECT_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    )

link_directories (${CMAKE_BINARY_DIR}/src)

add_library(bong-dtd SHARED ${bong_dtd_SRC})
target_link_libraries(bong-dtd ${GOBJECT_LIBRARIES} ${GLIB2_LIBRARIES} bong)

set_target_properties(bong-dtd PROPERTIES VERSION "0.0.0" SOVERSION "0" LINK_INTERFACE_LIBRARIES "" DEFINE_SYMBOL BUILDING_BONG)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

install( TARGETS bong-dtd DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )

install( FILES
         ${bong_dtd_HEADERS}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bong COMPONENT devel
         )

set(INCLUDE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include/bong)
get_target_property(LIB_OUT_NAME bong-dtd LOCATION)
get_filename_component(LIB_OUT_NAME ${LIB_OUT_NAME} NAME)
set(LIBPATH ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})

configure_file(
   BongDtdConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BongDtdConfig.cmake @ONLY
)

install( FILES
         ${CMAKE_CURRENT_BINARY_DIR}/BongDtdConfig.cmake
     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/cmake/BongDtd
)

add_subdirectory(qt)