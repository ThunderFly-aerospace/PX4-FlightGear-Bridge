

find_package(ignition-math4 REQUIRED)

add_executable(flightgear_bridge 
               flightgear_bridge.cpp
			   px4_communicator.cpp
			   fg_communicator.cpp
			   vehicle_state.cpp
			   geo_mag_declination.cpp
			   )

target_include_directories(flightgear_bridge 
                BEFORE
                PUBLIC ${PX4_SOURCE_DIR}/mavlink/include)

target_compile_options(flightgear_bridge PUBLIC -fexceptions -Wno-cast-align -Wno-address-of-packed-member)

target_link_libraries(flightgear_bridge ignition-math4::ignition-math4)

