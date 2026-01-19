set(DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_SBoOX_9b "c:/Program Files/Microchip/xc32/v4.60/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_SBoOX_9b ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX/default.elf)
set(DEPENDENT_TARGET_DIRML_OV7670_GFX_default_SBoOX_9b ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX)
set(DEPENDENT_BYPRODUCTSML_OV7670_GFX_default_SBoOX_9b ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_SBoOX_9b}/${sourceFileNameML_OV7670_GFX_default_SBoOX_9b}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_SBoOX_9b}/${sourceFileNameML_OV7670_GFX_default_SBoOX_9b}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_SBoOX_9b} --image ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_SBoOX_9b} --image-generated-c ${sourceFileNameML_OV7670_GFX_default_SBoOX_9b}.c --image-generated-h ${sourceFileNameML_OV7670_GFX_default_SBoOX_9b}.h --image-copy-mode ${modeML_OV7670_GFX_default_SBoOX_9b} --image-offset ${addressML_OV7670_GFX_default_SBoOX_9b} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_SBoOX_9b}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_SBoOX_9b})
add_custom_target(
    dependent_produced_source_artifactML_OV7670_GFX_default_SBoOX_9b 
    DEPENDS ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_SBoOX_9b}/${sourceFileNameML_OV7670_GFX_default_SBoOX_9b}.c
    )
