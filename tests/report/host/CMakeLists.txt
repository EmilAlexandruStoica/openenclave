# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

include(add_dcap_client_target)

oeedl_file(../tests.edl host gen)
add_executable(report_host host.cpp tcbinfo.cpp ../common/tests.cpp ${gen})

if(HAS_QUOTE_PROVIDER)
    target_compile_definitions(report_host PRIVATE OE_LINK_SGX_DCAP_QL)
endif()

add_custom_command(TARGET report_host
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../data ${CMAKE_CURRENT_BINARY_DIR}/../data
)

target_include_directories(report_host PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../common)
target_link_libraries(report_host oehostapp)

# On Windows, explicitly add the nuget dependencies for the DCAP client to the target executable
if (WIN32)
    add_dcap_client_target(report_host_dcap_client)
    add_dependencies(report_host report_host_dcap_client)
endif ()