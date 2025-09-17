cmake_minimum_required(VERSION 3.22)

project(esquire_isotp)
add_library(esquire_isotp INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)


target_include_directories(esquire_isotp INTERFACE
    ./
)

target_sources(esquire_isotp INTERFACE

    ./isotp_link_containers.c
    ./isotp_links.c
    ./isotp_service.c
    ./isotp_tasks.c
    ./isotp_user.c
    ./isotp.c
    ./rx_tx_tasks.c
)

if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


