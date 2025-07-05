# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ChronoVault_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ChronoVault_autogen.dir\\ParseCache.txt"
  "ChronoVault_autogen"
  )
endif()
