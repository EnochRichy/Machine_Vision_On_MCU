# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "D:\\MV_PIC32CZ_Git\\TestProjects\\GreyScale\\out\\ML_OV7670_GFX"
  )
endif()
