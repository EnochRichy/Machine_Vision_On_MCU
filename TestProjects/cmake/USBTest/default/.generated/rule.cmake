# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/I41645/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(USBTest_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=32CZ8110CA90208"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG=1")
endfunction()
function(USBTest_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90"
        "-mprocessor=32CZ8110CA90208"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--gdwarf-2,--defsym=__DEBUG=1")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "XPRJ_default=default")
endfunction()
function(USBTest_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=32CZ8110CA90208"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/mcc"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/PIC32CZ8110CA90208_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/mcc"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/PIC32CZ8110CA90208_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.2.0/CMSIS/Core/Include")
endfunction()
function(USBTest_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=32CZ8110CA90208"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/mcc"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/PIC32CZ8110CA90208_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/mcc"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/packs/PIC32CZ8110CA90208_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../My_MCC_Config/src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.2.0/CMSIS/Core/Include")
endfunction()
function(USBTest_default_dependentObject_rule target)
    set(options
        "-mprocessor=32CZ8110CA90208"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(USBTest_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=32CZ8110CA90208"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${USBTest_default_LINKER_SCRIPT},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=_min_heap_size=512,--gc-sections,-Map=mem.map,--report-mem,-DVECTOR_REGION=boot_rom,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32CZ-CA90_DFP/1.7.168/CA90")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
function(USBTest_default_bin2hex_rule target)
    add_custom_target(
        USBTest_default_Bin2Hex ALL
        COMMAND ${MP_BIN2HEX} ${USBTest_default_image_name}
        WORKING_DIRECTORY ${USBTest_default_output_dir}
        BYPRODUCTS "${USBTest_default_output_dir}/${USBTest_default_image_base_name}.hex"
        COMMENT "Convert build file to .hex")
    add_dependencies(USBTest_default_Bin2Hex ${target})
endfunction()
