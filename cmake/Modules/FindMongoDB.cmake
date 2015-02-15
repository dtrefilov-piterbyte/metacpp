# Find the MongoDB includes and client library
# This module defines
#  MongoDB_INCLUDE_DIR, where to find mongo/client/dbclient.h
#  MongoDB_LIBRARIES, the libraries needed to use MongoDB.
#  MongoDB_FOUND, If false, do not try to use MongoDB.

SET(MongoDB_PossibleIncludePaths
    "/usr/include/"
    "/usr/local/include/"
    "/usr/include/mongo/"
    "/usr/local/include/mongo/"
    "/opt/mongo/include/"
    "$ENV{ProgramFiles}/Mongo/*/include"
    "$ENV{SystemDrive}/Mongo/*/include"
  )

FIND_PATH(MongoDB_INCLUDE_DIR "mongo/client/dbclient.h"
    ${MongoDB_PossibleIncludePaths})

IF(WIN32)
    FIND_LIBRARY(MongoDB_LIBRARIES NAMES mongoclient
        PATHS
        "$ENV{ProgramFiles}/Mongo/*/lib"
        "$ENV{SystemDrive}/Mongo/*/lib"
    )
ELSE(WIN32)
    FIND_LIBRARY(MongoDB_LIBRARIES NAMES mongoclient
        PATHS
        "/usr/lib"
        "/usr/lib64"
        "/usr/lib/mongo"
        "/usr/lib64/mongo"
        "/usr/local/lib"
        "/usr/local/lib64"
        "/usr/local/lib/mongo"
        "/usr/local/lib64/mongo"
        "/opt/mongo/lib"
        "/opt/mongo/lib64"
    )
ENDIF(WIN32)

IF(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)
    SET(MongoDB_FOUND TRUE)
    MESSAGE(STATUS "Found MongoDB: ${MongoDB_INCLUDE_DIR}, ${MongoDB_LIBRARIES}")
    MESSAGE(STATUS "MongoDB using new interface: ${MongoDB_EXPOSE_MACROS}")
ELSE(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)
    SET(MongoDB_FOUND FALSE)
    IF(MongoDB_FIND_REQUIRED)
        message(FATAL_ERROR "MongoDB not found.")
    ELSE(MongoDB_FIND_REQUIRED)
        message(STATUS "MongoDB not found.")
    ENDIF(MongoDB_FIND_REQUIRED)
ENDIF(MongoDB_INCLUDE_DIR AND MongoDB_LIBRARIES)

MARK_AS_ADVANCED(MongoDB_INCLUDE_DIR MongoDB_LIBRARIES)
