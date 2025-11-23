set(DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_yAJDuesN "c:/Program Files/Microchip/xc32/v4.60/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_yAJDuesN ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX/default.elf)
set(DEPENDENT_TARGET_DIRML_OV7670_GFX_default_yAJDuesN ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ML_OV7670_GFX)
set(DEPENDENT_BYPRODUCTSML_OV7670_GFX_default_yAJDuesN ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_yAJDuesN}/${sourceFileNameML_OV7670_GFX_default_yAJDuesN}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_yAJDuesN}/${sourceFileNameML_OV7670_GFX_default_yAJDuesN}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXML_OV7670_GFX_default_yAJDuesN} --image ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_yAJDuesN} --image-generated-c ${sourceFileNameML_OV7670_GFX_default_yAJDuesN}.c --image-generated-h ${sourceFileNameML_OV7670_GFX_default_yAJDuesN}.h --image-copy-mode ${modeML_OV7670_GFX_default_yAJDuesN} --image-offset ${addressML_OV7670_GFX_default_yAJDuesN} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_yAJDuesN}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFML_OV7670_GFX_default_yAJDuesN})
add_custom_target(
    dependent_produced_source_artifactML_OV7670_GFX_default_yAJDuesN 
    DEPENDS ${DEPENDENT_TARGET_DIRML_OV7670_GFX_default_yAJDuesN}/${sourceFileNameML_OV7670_GFX_default_yAJDuesN}.c
    )
