IF(HYPERSCAN_DIR)
  FIND_PATH(HYPERSCAN_INCLUDE_DIR NAMES hs/hs.h NO_DEFAULT_PATH PATHS ${HYPERSCAN_DIR}/include ${HYPERSCAN_DIR})
  FIND_LIBRARY(HYPERSCAN_LIBRARY NAMES hs NO_DEFAULT_PATH PATHS ${HYPERSCAN_DIR}/lib ${HYPERSCAN_DIR})
ELSE()
  FIND_PATH(HYPERSCAN_INCLUDE_DIR NAMES hs/hs.h PATHS /include /usr/include /usr/local/include /opt/local/include)
  FIND_LIBRARY(HYPERSCAN_LIBRARY NAMES hs)
ENDIF()

IF (HYPERSCAN_INCLUDE_DIR AND HYPERSCAN_LIBRARY)
  SET(HYPERSCAN_INCLUDE_DIRS "${HYPERSCAN_INCLUDE_DIR_unzh};${HYPERSCAN_INCLUDE_DIR_zh};${ZLIB_INCLUDE_DIRS}")
  SET(HYPERSCAN_LIBRARIES ${HYPERSCAN_LIBRARY})
  SET(HYPERSCAN_FOUND true)
ENDIF()
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(Hyperscan DEFAULT_MSG HYPERSCAN_LIBRARIES HYPERSCAN_INCLUDE_DIRS)
