set(DEPENDENT_MP_BIN2HEXUSBTest_default_bRE6iwhd "c:/Program Files/Microchip/xc32/v4.60/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFUSBTest_default_bRE6iwhd ${CMAKE_CURRENT_LIST_DIR}/../../../../out/USBTest/default.elf)
set(DEPENDENT_TARGET_DIRUSBTest_default_bRE6iwhd ${CMAKE_CURRENT_LIST_DIR}/../../../../out/USBTest)
set(DEPENDENT_BYPRODUCTSUSBTest_default_bRE6iwhd ${DEPENDENT_TARGET_DIRUSBTest_default_bRE6iwhd}/${sourceFileNameUSBTest_default_bRE6iwhd}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRUSBTest_default_bRE6iwhd}/${sourceFileNameUSBTest_default_bRE6iwhd}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXUSBTest_default_bRE6iwhd} --image ${DEPENDENT_DEPENDENT_TARGET_ELFUSBTest_default_bRE6iwhd} --image-generated-c ${sourceFileNameUSBTest_default_bRE6iwhd}.c --image-generated-h ${sourceFileNameUSBTest_default_bRE6iwhd}.h --image-copy-mode ${modeUSBTest_default_bRE6iwhd} --image-offset ${addressUSBTest_default_bRE6iwhd} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRUSBTest_default_bRE6iwhd}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFUSBTest_default_bRE6iwhd})
add_custom_target(
    dependent_produced_source_artifactUSBTest_default_bRE6iwhd 
    DEPENDS ${DEPENDENT_TARGET_DIRUSBTest_default_bRE6iwhd}/${sourceFileNameUSBTest_default_bRE6iwhd}.c
    )
