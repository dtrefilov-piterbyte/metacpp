
# scans directories for C/C++ source and header files
MACRO(ADD_SOURCE_DIRECTORY SOURCES HEADERS SCAN_DIRS)
    FOREACH(DIR ${SCAN_DIRS})

        file(GLOB SRC_TEMP
            "${DIR}/*.cpp" "${DIR}/*.c" "${DIR}/*.cc"
        )
        file(GLOB HDR_TEMP
            "${DIR}/*.h" "${DIR}/*.hh" "${DIR}/*.hpp"
        )
        LIST(APPEND ${SOURCES} ${SRC_TEMP})
        LIST(APPEND ${HEADERS} ${HDR_TEMP})
    ENDFOREACH(DIR)
ENDMACRO(ADD_SOURCE_DIRECTORY)
