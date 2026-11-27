add_subdirectory(${HEROES_RECOMP_TOOLS_ROOT}/bc_diff)
# Retain XboxUtils for the runtime, without building the legacy CLI/analyzer.
add_subdirectory(${HEROES_RECOMP_TOOLS_ROOT}/XboxRecomp EXCLUDE_FROM_ALL)
add_subdirectory(${HEROES_RECOMP_TOOLS_ROOT}/XShaderRecomp)

add_subdirectory(${HEROES_RECOMP_TOOLS_ROOT}/file_to_c)
