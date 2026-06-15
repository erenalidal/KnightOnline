# macOS/non-Windows port: orijinal JpegFile.cpp Windows GDI'ya bağlıdır
# (ekran yakalama/DIB). Non-Windows'ta stub implementasyon kullanılır.
# See docs/MAC_PORT_ROADMAP.md.
if (WIN32)
  set(JPEGFILE_IMPL JpegFile.cpp)
else ()
  set(JPEGFILE_IMPL JpegFile_mac.cpp)
endif ()

add_library(JpegFile STATIC
  ${JPEGFILE_IMPL}
  JpegFile.h
)

set_target_properties(JpegFile PROPERTIES
  CXX_SCAN_FOR_MODULES OFF
)

# Enable warnings as errors
if(OPENKO_COMPILE_WARNINGS_AS_ERROR)
  set_property(TARGET JpegFile PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Setup dependencies
target_link_libraries(JpegFile PUBLIC libjpeg)

# Expose include path
target_include_directories(JpegFile PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../" # so we can access <JpegFile/...>
)

# macOS/non-Windows port: JpegFile.cpp uses <windows.h> (BITMAPINFO etc.).
# Provide via dxvk-native + compat shims. See docs/MAC_PORT_ROADMAP.md.
if (NOT WIN32)
  # Non-Windows'ta stub (JpegFile_mac.cpp) kullanılır — libjpeg YOK, bu yüzden
  # WarFare ile AYNI Win32 tiplerini (dxvk RECT dahil) kullanır ki link sembolleri
  # eşleşsin. (jpeg-compat lean şimi yalnızca gerçek JpegFile.cpp için gerekliydi.)
  set(_DXVK "${CMAKE_SOURCE_DIR}/deps/dxvk-native-src")
  target_include_directories(JpegFile PRIVATE
    "${CMAKE_SOURCE_DIR}/src/platform"
    "${CMAKE_SOURCE_DIR}/src/platform/win-compat"
    "${_DXVK}/include/native"
    "${_DXVK}/include/native/windows"
    "${_DXVK}/include/native/directx"
  )
endif ()

# Setup warning levels
target_compile_options(JpegFile PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
