
add_executable(flightgear_bridge 
               flightgear_bridge.c)

target_include_directories(flightgear_bridge 
                BEFORE
                PUBLIC ${PX4_SOURCE_DIR}/mavlink/include)
