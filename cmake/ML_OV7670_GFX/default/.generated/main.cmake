include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(ML_OV7670_GFX_default_library_list )

# Handle files with suffix s, for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_assemble)
add_library(ML_OV7670_GFX_default_default_XC32_assemble OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_assemble})
    ML_OV7670_GFX_default_default_XC32_assemble_rule(ML_OV7670_GFX_default_default_XC32_assemble)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(ML_OV7670_GFX_default_default_XC32_assembleWithPreprocess OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    ML_OV7670_GFX_default_default_XC32_assembleWithPreprocess_rule(ML_OV7670_GFX_default_default_XC32_assembleWithPreprocess)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_compile)
add_library(ML_OV7670_GFX_default_default_XC32_compile OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_compile})
    ML_OV7670_GFX_default_default_XC32_compile_rule(ML_OV7670_GFX_default_default_XC32_compile)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(ML_OV7670_GFX_default_default_XC32_compile_cpp OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_compile_cpp})
    ML_OV7670_GFX_default_default_XC32_compile_cpp_rule(ML_OV7670_GFX_default_default_XC32_compile_cpp)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_dependentObject)
add_library(ML_OV7670_GFX_default_default_XC32_dependentObject OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_dependentObject})
    ML_OV7670_GFX_default_default_XC32_dependentObject_rule(ML_OV7670_GFX_default_default_XC32_dependentObject)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC32
if(ML_OV7670_GFX_default_default_XC32_FILE_TYPE_bin2hex)
add_library(ML_OV7670_GFX_default_default_XC32_bin2hex OBJECT ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_bin2hex})
    ML_OV7670_GFX_default_default_XC32_bin2hex_rule(ML_OV7670_GFX_default_default_XC32_bin2hex)
    list(APPEND ML_OV7670_GFX_default_library_list "$<TARGET_OBJECTS:ML_OV7670_GFX_default_default_XC32_bin2hex>")

endif()


# Main target for this project
add_executable(ML_OV7670_GFX_default_image_SBoOX_9b ${ML_OV7670_GFX_default_library_list})

if(NOT CMAKE_HOST_WIN32)
    set_target_properties(ML_OV7670_GFX_default_image_SBoOX_9b PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ML_OV7670_GFX_default_output_dir}")
endif()
set_target_properties(ML_OV7670_GFX_default_image_SBoOX_9b PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf")
target_link_libraries(ML_OV7670_GFX_default_image_SBoOX_9b PRIVATE ${ML_OV7670_GFX_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
ML_OV7670_GFX_default_link_rule( ML_OV7670_GFX_default_image_SBoOX_9b)

# Call bin2hex function from the rule file
ML_OV7670_GFX_default_bin2hex_rule(ML_OV7670_GFX_default_image_SBoOX_9b)
if(CMAKE_HOST_WIN32)
    add_custom_command(
        TARGET ML_OV7670_GFX_default_image_SBoOX_9b
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ML_OV7670_GFX_default_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ML_OV7670_GFX_default_image_SBoOX_9b> ${ML_OV7670_GFX_default_output_dir}/${ML_OV7670_GFX_default_original_image_name}
        BYPRODUCTS ${ML_OV7670_GFX_default_output_dir}/${ML_OV7670_GFX_default_original_image_name}
        COMMENT "Copying elf to out location")
    set_property(
        TARGET ML_OV7670_GFX_default_image_SBoOX_9b
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        ${ML_OV7670_GFX_default_output_dir}/${ML_OV7670_GFX_default_original_image_name})
endif()

