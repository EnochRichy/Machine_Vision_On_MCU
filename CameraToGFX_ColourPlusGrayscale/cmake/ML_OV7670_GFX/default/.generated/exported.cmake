set(DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_5a0pw6Ck "c:/Program Files/Microchip/xc32/v4.60/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_5a0pw6Ck ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX/default.elf)
set(DEPENDENT_TARGET_DIRML_OV7670_GFX_default_5a0pw6Ck ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX)
set(DEPENDENT_BYPRODUCTSML_OV7670_GFX_default_5a0pw6Ck ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_5a0pw6Ck}/${sourceFileNameML_OV7670_GFX_default_5a0pw6Ck}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_5a0pw6Ck}/${sourceFileNameML_OV7670_GFX_default_5a0pw6Ck}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_5a0pw6Ck} --image ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_5a0pw6Ck} --image-generated-c ${sourceFileNameML_OV7670_GFX_default_5a0pw6Ck}.c --image-generated-h ${sourceFileNameML_OV7670_GFX_default_5a0pw6Ck}.h --image-copy-mode ${modeML_OV7670_GFX_default_5a0pw6Ck} --image-offset ${addressML_OV7670_GFX_default_5a0pw6Ck} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_5a0pw6Ck}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_5a0pw6Ck})
add_custom_target(
    dependent_produced_source_artifactML_OV7670_GFX_default_5a0pw6Ck 
    DEPENDS ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_5a0pw6Ck}/${sourceFileNameML_OV7670_GFX_default_5a0pw6Ck}.c
    )
