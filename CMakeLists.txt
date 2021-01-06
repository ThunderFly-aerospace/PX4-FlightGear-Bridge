cmake_minimum_required (VERSION 2.8.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Eigen3 REQUIRED)
find_package(MAVLink)

message(STATUS "${MAVLINK_INCLUDE_DIRS}")

add_executable(flightgear_bridge 
               src/flightgear_bridge.cpp
			   src/px4_communicator.cpp
			   src/fg_communicator.cpp
			   src/vehicle_state.cpp
			   src/geo_mag_declination.cpp
			   )

target_include_directories(flightgear_bridge 
                BEFORE
                PUBLIC ${MAVLINK_INCLUDE_DIRS})

target_compile_options(flightgear_bridge PUBLIC -g -fexceptions -Wno-cast-align -Wno-address-of-packed-member)
target_link_libraries(flightgear_bridge Eigen3::Eigen)

