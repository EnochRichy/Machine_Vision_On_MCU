# cmake files support debug production
include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(USBTest_default_library_list )

# Handle files with suffix s, for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_assemble)
add_library(USBTest_default_default_XC32_assemble OBJECT ${USBTest_default_default_XC32_FILE_TYPE_assemble})
    USBTest_default_default_XC32_assemble_rule(USBTest_default_default_XC32_assemble)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(USBTest_default_default_XC32_assembleWithPreprocess OBJECT ${USBTest_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    USBTest_default_default_XC32_assembleWithPreprocess_rule(USBTest_default_default_XC32_assembleWithPreprocess)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_compile)
add_library(USBTest_default_default_XC32_compile OBJECT ${USBTest_default_default_XC32_FILE_TYPE_compile})
    USBTest_default_default_XC32_compile_rule(USBTest_default_default_XC32_compile)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(USBTest_default_default_XC32_compile_cpp OBJECT ${USBTest_default_default_XC32_FILE_TYPE_compile_cpp})
    USBTest_default_default_XC32_compile_cpp_rule(USBTest_default_default_XC32_compile_cpp)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_dependentObject)
add_library(USBTest_default_default_XC32_dependentObject OBJECT ${USBTest_default_default_XC32_FILE_TYPE_dependentObject})
    USBTest_default_default_XC32_dependentObject_rule(USBTest_default_default_XC32_dependentObject)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC32
if(USBTest_default_default_XC32_FILE_TYPE_bin2hex)
add_library(USBTest_default_default_XC32_bin2hex OBJECT ${USBTest_default_default_XC32_FILE_TYPE_bin2hex})
    USBTest_default_default_XC32_bin2hex_rule(USBTest_default_default_XC32_bin2hex)
    list(APPEND USBTest_default_library_list "$<TARGET_OBJECTS:USBTest_default_default_XC32_bin2hex>")

endif()


# Main target for this project
add_executable(USBTest_default_image_bRE6iwhd ${USBTest_default_library_list})

if(NOT CMAKE_HOST_WIN32)
    set_target_properties(USBTest_default_image_bRE6iwhd PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${USBTest_default_output_dir})
endif()
set_target_properties(USBTest_default_image_bRE6iwhd PROPERTIES OUTPUT_NAME "default")
set_target_properties(USBTest_default_image_bRE6iwhd PROPERTIES SUFFIX ".elf")

target_link_libraries(USBTest_default_image_bRE6iwhd PRIVATE ${USBTest_default_default_XC32_FILE_TYPE_link})


# Add the link options from the rule file.
USBTest_default_link_rule(USBTest_default_image_bRE6iwhd)

# Call bin2hex function from the rule file
USBTest_default_bin2hex_rule(USBTest_default_image_bRE6iwhd)

if(CMAKE_HOST_WIN32)
    add_custom_command(
        TARGET USBTest_default_image_bRE6iwhd
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${USBTest_default_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:USBTest_default_image_bRE6iwhd> ${USBTest_default_output_dir}/${USBTest_default_original_image_name}
        BYPRODUCTS ${USBTest_default_output_dir}/${USBTest_default_original_image_name}
        COMMENT "Copying elf to out location")
    set_property(
        TARGET USBTest_default_image_bRE6iwhd
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        ${USBTest_default_output_dir}/${USBTest_default_original_image_name})
endif()

