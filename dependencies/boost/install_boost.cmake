if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    message(FATAL_ERROR "CMAKE_INSTALL_PREFIX is not defined")
endif()

if(NOT DEFINED BOOST_VERSION)
    message(FATAL_ERROR "BOOST_VERSION is not defined")
endif()

file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/cmake/Boost-${BOOST_VERSION}/BoostConfig.cmake" "
set(Boost_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(Boost_FOUND TRUE)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Boost REQUIRED_VARS Boost_INCLUDE_DIR)
")

file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/cmake/Boost-${BOOST_VERSION}/BoostConfigVersion.cmake" "
set(PACKAGE_VERSION ${BOOST_VERSION})
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_VERSION VERSION_EQUAL PACKAGE_FIND_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
")
