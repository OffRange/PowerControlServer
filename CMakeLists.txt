cmake_minimum_required(VERSION 3.19)
project("Power Control Server" VERSION 0.2.0 DESCRIPTION "Power Control Server - A tool that enables remote shutdown")
set (CMAKE_CXX_STANDARD 20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

 if (MSVC)
     add_definitions(-DUNICODE -D_UNICODE)
 endif()


# Output directory for generated files
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Common library
set(PCS_SERVER_SOURCES
    src/processor.cpp
    src/udp_server.cpp
)
add_library(server_lib ${PCS_SERVER_SOURCES})
target_include_directories(server_lib 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

configure_file(src/definitions.h.in ${GENERATED_DIR}/definitions.h @ONLY)

if(WIN32)
    set(PCS_WINDOWS_SOURCES
        src/win/service/service_report.cpp
        src/win/service/service_config.cpp
        src/win/service/service_globals.cpp
        src/win/service/service_controller.cpp
        src/win/firewall/firewall_register.cpp
    )
    add_library(windows_lib ${PCS_WINDOWS_SOURCES})
    target_include_directories(windows_lib 
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/win
    )
    
    set(SERVICE_FILE_NAME service)
    set(SERVICE_DLL_NAME service_msg)

    # Compile the .mc file using mc.exe and include the generated headers
    find_program(MC_COMPILER mc.exe)
    if(NOT MC_COMPILER)
        message(FATAL_ERROR "mc.exe not found in PATH. Make sure the Windows SDK is installed and mc.exe is in PATH.")
    endif()
    
    find_program(RC_COMPILER rc.exe)
    if(NOT RC_COMPILER)
        message(FATAL_ERROR "rc.exe not found in PATH. Make sure the Windows SDK is installed and rc.exe is in PATH.")
    endif()
    
    find_program(LINK_EXE link.exe)
    if(NOT LINK_EXE)
        message(FATAL_ERROR "link.exe not found in PATH. Make sure the Windows SDK is installed and link.exe is in PATH.")
    endif()
    
    # Path to the .mc file
    set(MC_FILE ${CMAKE_SOURCE_DIR}/${SERVICE_DLL_NAME}.mc)
    
    # MC
    execute_process(
        COMMAND ${MC_COMPILER} -U ${CMAKE_SOURCE_DIR}/src/win/${SERVICE_DLL_NAME}.mc
        WORKING_DIRECTORY ${GENERATED_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE o
    )
    
    if(result)
        message(FATAL_ERROR "Message compiler failed with result: ${result}")
    else()
        message(STATUS "Message compiler executed successfully.")
    endif()
    
    # LINK
    execute_process(
        COMMAND ${RC_COMPILER} -r ${GENERATED_DIR}/${SERVICE_DLL_NAME}.rc
        WORKING_DIRECTORY ${GENERATED_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE o
    )
    
    if(result)
        message(FATAL_ERROR "Resource compiler failed with result: ${result}")
    else()
        message(STATUS "Resource compiler executed successfully.")
    endif()
    
    # LINK
    execute_process(
        COMMAND ${LINK_EXE} -dll -noentry -out:${GENERATED_DIR}/${SERVICE_DLL_NAME}.dll ${GENERATED_DIR}/${SERVICE_DLL_NAME}.res
        WORKING_DIRECTORY ${GENERATED_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE o
    )
    
    if(result)
        message(FATAL_ERROR "LINK failed with result: ${result}")
    else()
        message(STATUS "LINK executed successfully.")
    endif()


    add_executable(${SERVICE_FILE_NAME} src/win/service/service.cpp ${GENERATED_DIR}/${SERVICE_DLL_NAME}.rc ${CMAKE_SOURCE_DIR}/res/powercontrol.rc)
    target_link_libraries(${SERVICE_FILE_NAME} PRIVATE wsock32 ws2_32 advapi32 server_lib windows_lib)
    
    install(FILES ${GENERATED_DIR}/${SERVICE_DLL_NAME}.dll DESTINATION .)
    install(TARGETS ${SERVICE_FILE_NAME} DESTINATION .)
    
    # Include the generated files directory
    include_directories(${GENERATED_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/src)
else(WIN32)
    set(SERVER_FILE_NAME power-control-server)
    
    add_executable(${SERVER_FILE_NAME} src/unix/main.cpp)
    target_link_libraries(${SERVER_FILE_NAME} PRIVATE server_lib)


    include_directories(${CMAKE_SOURCE_DIR}/src)

    install(TARGETS ${SERVER_FILE_NAME} RUNTIME DESTINATION bin)

    if(NOT APPLE)
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/src/unix/power-control-server.service.in"
            "${GENERATED_DIR}/power-control-server.service"
            @ONLY
        )
        
        include_directories(${GENERATED_DIR})

        install(FILES "${GENERATED_DIR}/power-control-server.service" DESTINATION /etc/systemd/system)
        install(CODE "execute_process(COMMAND systemctl enable power-control-server.service)")
        install(CODE "execute_process(COMMAND systemctl start power-control-server.service)")
    else()
        # Apple
        # TODO Apple
    endif()
endif(WIN32)


# CPACK CONFIGURATION

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "OffRange")
set(CPACK_PACKAGE_CONTACT "offrange.developer@gmail.com")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_LICENSE "MIT")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Set the installation directory for Windows
if(WIN32)
    # For Windows, require CMake 3.20 or higher
    if(${CMAKE_VERSION} VERSION_LESS "3.20")
        message(FATAL_ERROR "CMake 3.20 or higher is required on Windows.")
    endif()

    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
    set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})

    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/res/powercontrol.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/res/powercontrol.ico")

    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_BRANDING_TEXT "OffRange - Power Control Server")
    set(CPACK_NSIS_DISPLAY_NAME "Power Control Server")

    set(CPACK_NSIS_INSTALLED_ICON_NAME "${SERVICE_FILE_NAME}.exe")
    
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        ExecWait '\\\"$INSTDIR\\\\service.exe\\\" install'
    ")

    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        ExecWait '\\\"$INSTDIR\\\\service.exe\\\" uninstall'
    ")

    include(CPack)
endif()



# Include additional files (optional)
# install(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION .)
# install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION .)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
include(InstallRequiredSystemLibraries)