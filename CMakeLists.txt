cmake_minimum_required(VERSION 3.16)
project(SimulationTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Use Qt 5.6.3 (available in D:\Qt\Qt5.6.3)
set(QT_VERSION "5" CACHE STRING "Qt version: 5 or 6")
set(Qt5_DIR "D:/Qt/Qt5.6.3/msvc2015_64/lib/cmake/Qt5" CACHE PATH "Qt5 CMake config path")

# Set Qt5_DIR for find_package
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};D:/Qt/Qt5.6.3/msvc2015_64/lib/cmake")

# Find Qt5 (5.6+)
find_package(Qt5 5.6 REQUIRED COMPONENTS Core Widgets OpenGL)
set(QT_LIBS Qt5::Widgets Qt5::OpenGL)
message(STATUS "Qt5 found, version: ${Qt5_VERSION}")

# OpenCASCADE 7.5+ (OCCT) installation path
set(OCC_ROOT "D:/OpenCASCADE-7.7.0" CACHE PATH "OpenCASCADE/OCCT root (7.5+ required)")
# Auto-detect lib dir: try win64/vc14/lib then lib
set(_OCC_LIB_CANDIDATES "${OCC_ROOT}/win64/vc14/lib" "${OCC_ROOT}/lib")
set(OCC_LIB_DIR "${OCC_ROOT}/lib" CACHE PATH "OCCT library directory")
foreach(_dir ${_OCC_LIB_CANDIDATES})
  if(EXISTS "${_dir}/TKernel.lib")
    set(OCC_LIB_DIR "${_dir}" CACHE PATH "OCCT library directory" FORCE)
    message(STATUS "Found OCCT libs in: ${_dir}")
    break()
  endif()
endforeach()
# OCCT bin dir (DLLs): try win64/vc14/bin then bin
set(OCC_BIN_DIR "${OCC_ROOT}/win64/vc14/bin")
if(NOT EXISTS "${OCC_BIN_DIR}")
  set(OCC_BIN_DIR "${OCC_ROOT}/bin")
endif()
# Extra DLLs for TBB
set(OCC_EXTRA_DLL_DIR "" CACHE PATH "Extra DLL dir (e.g. for tbb12.dll)")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OCC_ROOT}/inc
)

# Find OCCT libraries (STEP optional: 有则启用打开 STEP，无则仅 3D 视图/仿真)
# OCCT 7.9.x 将 TKSTEP/TKSTEPAttr/TKSTEPBase/TKSTEP209 合并为 TKDESTEP
# 同时支持旧版(TKSTEP)和新版(TKDESTEP)
set(_OCC_LIBS_CORE
  TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKG3d TKG2d TKTopAlgo TKPrim
  TKXSBase
  TKV3d TKService TKOpenGl
)
set(_OCC_LIBS_FOUND)
set(OCC_HAS_STEP FALSE)
foreach(_lib ${_OCC_LIBS_CORE})
  find_library(_OCC_${_lib} NAMES ${_lib} PATHS ${OCC_LIB_DIR} NO_DEFAULT_PATH)
  if(_OCC_${_lib})
    list(APPEND _OCC_LIBS_FOUND ${_OCC_${_lib}})
  else()
    message(WARNING "OCCT library not found: ${_lib}.lib in ${OCC_LIB_DIR}")
  endif()
endforeach()

# Try OCCT 7.9+ STEP lib: TKDESTEP (新 DE 框架)
find_library(_OCC_TKDESTEP NAMES TKDESTEP PATHS ${OCC_LIB_DIR} NO_DEFAULT_PATH)
find_library(_OCC_TKDE     NAMES TKDE     PATHS ${OCC_LIB_DIR} NO_DEFAULT_PATH)
if(_OCC_TKDESTEP)
  list(APPEND _OCC_LIBS_FOUND ${_OCC_TKDESTEP})
  if(_OCC_TKDE)
    list(APPEND _OCC_LIBS_FOUND ${_OCC_TKDE})
  endif()
  set(OCC_HAS_STEP TRUE)
  message(STATUS "OCCT STEP support: enabled (TKDESTEP / OCCT 7.9+)")
else()
  # Fallback: try old STEP libs (OCCT < 7.8)
  foreach(_lib TKSTEP TKSTEPAttr TKSTEPBase TKSTEP209)
    find_library(_OCC_${_lib} NAMES ${_lib} PATHS ${OCC_LIB_DIR} NO_DEFAULT_PATH)
    if(_OCC_${_lib})
      list(APPEND _OCC_LIBS_FOUND ${_OCC_${_lib}})
      if(_lib STREQUAL "TKSTEP")
        set(OCC_HAS_STEP TRUE)
      endif()
    endif()
  endforeach()
  if(OCC_HAS_STEP)
    message(STATUS "OCCT STEP support: enabled (TKSTEP / legacy)")
  else()
    message(STATUS "OCCT STEP support: disabled; 打开 STEP 将提示需完整 OCCT")
  endif()
endif()

# Output build information
message(STATUS "OpenCASCADE root: ${OCC_ROOT}")
message(STATUS "Include dir: ${OCC_ROOT}/inc")
message(STATUS "Lib dir: ${OCC_LIB_DIR}")
message(STATUS "OCCT libraries found: ${_OCC_LIBS_FOUND}")

# Source files
set(SOURCES
    src/main.cpp
    src/OccViewWidget.cpp
    src/SimulatorMainWindow.cpp
    src/SimulationEngine.cpp
    src/STEPReader.cpp
    src/SharedMemorySender.cpp
)

# Header files
set(HEADERS
    include/OccViewWidget.h
    include/SimulatorMainWindow.h
    include/SimulationEngine.h
    include/STEPReader.h
    include/SharedMemorySender.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Ensure target sees include dirs (required for some generators)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OCC_ROOT}/inc
)
if(NOT OCC_HAS_STEP)
  target_compile_definitions(${PROJECT_NAME} PRIVATE OCC_NO_STEP)
endif()

# Set UTF-8 encoding for MSVC compiler to fix Chinese character display
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${QT_LIBS}
    ${_OCC_LIBS_FOUND}
)

# Windows: copy OCCT DLLs to exe dir (try bin and lib; some packages put DLLs in lib)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
    set(_OCC_OUT "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    if(EXISTS "${OCC_BIN_DIR}")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory "${OCC_BIN_DIR}" "${_OCC_OUT}"
          COMMENT "Copying OCCT DLLs from bin")
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${OCC_LIB_DIR}" "${_OCC_OUT}"
        COMMENT "Copying OCCT DLLs from lib")
    if(OCC_EXTRA_DLL_DIR AND EXISTS "${OCC_EXTRA_DLL_DIR}")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory "${OCC_EXTRA_DLL_DIR}" "${_OCC_OUT}"
          COMMENT "Copying extra DLLs (e.g. TBB)")
    endif()
endif()
if(NOT OCC_EXTRA_DLL_DIR)
  message(STATUS "若运行时报缺少 tbb12.dll，请设置 -DOCC_EXTRA_DLL_DIR=包含tbb12.dll的目录 后重新配置并编译")
endif()

# -----------------------------------------------------------------------
# Auto-copy Qt5 DLLs required for runtime
# -----------------------------------------------------------------------
if(WIN32)
  # Qt5 DLLs
  set(_QT_BIN "D:/Qt/Qt5.6.3/msvc2015_64/bin")
  foreach(_qtdll Qt5Core Qt5Gui Qt5Widgets Qt5OpenGL)
    if(EXISTS "${_QT_BIN}/${_qtdll}.dll")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_QT_BIN}/${_qtdll}.dll"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying Qt5 DLL: ${_qtdll}.dll")
    endif()
  endforeach()
  # Qt5 platform plugin
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "D:/Qt/Qt5.6.3/msvc2015_64/plugins/platforms/qwindows.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/qwindows.dll"
    COMMENT "Copying Qt5 platform plugin")
endif()

message(STATUS "Note: If you encounter missing DLL errors at runtime, you may need to manually copy required DLLs to the executable directory")

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)