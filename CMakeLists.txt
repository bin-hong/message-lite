
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )
if(${CMAKE_VERSION} VERSION_GREATER "2.8.10")
        cmake_policy( SET CMP0022 OLD )
endif()


project(messagelite)

#show complies information 
set(CMAKE_VERBOSE_MAKEFILE ON)

#set C-compilers options
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message("The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_C_FLAGS	 "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}  ${EXTRA_LDFLAGS} -g -Werror -O2 -pthread")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} ${EXTRA_LDFLAGS}  -g -Werror -O2 -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose build type: Debug, Release, RelWithDebInfo, MinSizeRel."
        FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#find dependency package
include( FindPkgConfig )
pkg_check_modules( GLIB REQUIRED
    glib-2.0
)

find_package(Threads REQUIRED)
#output inner var
#message(STATUS)
#message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
#message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
#message(STATUS)

# set include .h file path
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

#set so patch


#set src file var
aux_source_directory(${CMAKE_SOURCE_DIR}/src message_server_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/src_mc message_client_SRC)

#define execute process name
set(message_server Message_Server  )
set(message_client Message_Client  )

#set sub fot test
add_subdirectory(test)

#create execute process
add_executable(${message_server} ${message_server_SRC})
add_executable(${message_client} ${message_client_SRC})

#target_link_libraries(
	#${EXTRA_LDFLAGS}
#	pthread
#)
#target_link_libraries(messagelite ${CMAKE_THREAD_LIBS_INIT})

