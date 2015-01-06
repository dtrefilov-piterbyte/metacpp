# JSONCPP_FOUND - system has a sqlite3
# JSONCPP_INCLUDE_DIR - where to find header files
# JSONCPP_LIBRARIES - the libraries to link against

FIND_PATH(
    JSONCPP_INCLUDE_DIR
    json/json.h
    /usr/include
    /usr/include/jsoncpp
    /usr/local/include
    ${JSONCPP_PATH_INCLUDES}
)

FIND_LIBRARY(
    JSONCPP_LIBRARIES
    NAMES jsoncpp
    PATHS
        /usr/lib${LIB_SUFFIX}
        /usr/local/lib${LIB_SUFFIX}
        ${JSONCPP_PATH_LIB}
)

IF (JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIR)
    SET(JSONCPP_FOUND "YES")
ENDIF (JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIR)

IF (JSONCPP_FOUND)
    MESSAGE(STATUS "Found JsonCpp: ${JSONCPP_LIBRARIES}")
    MESSAGE(STATUS "   include: ${JSONCPP_INCLUDE_DIR}")
ELSE (JSONCPP_FOUND)
    MESSAGE(STATUS "JsonCpp not found.")
    MESSAGE(STATUS "JsonCpp: You can specify includes: -DJSONCPP_PATH_INCLUDES=/opt/include/jsoncpp")
    MESSAGE(STATUS "      currently found includes: ${JSONCPP_INCLUDE_DIR}")
    MESSAGE(STATUS "JsonCpp: You can specify libs: -DJSONCPP_PATH_LIB=/opt/jsoncpp/lib")
    MESSAGE(STATUS "      currently found libs: ${JSONCPP_LIBRARIES}")
    IF (JsonCpp_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find JsonCpp library")
    ENDIF (JsonCpp_FIND_REQUIRED)
ENDIF (JSONCPP_FOUND)
