# Frame Import.

add_library(FrameFile
  OBJECT
  ${CMAKE_SOURCE_DIR}/include/frame/file/file_system.h
  ${CMAKE_SOURCE_DIR}/include/frame/file/image_stb.h
  file_system.cpp
  image.cpp
  image.h
  obj.cpp
  obj.h
  ply.cpp
  ply.h
)

target_include_directories(FrameFile
  PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(FrameFile
  PUBLIC
  fmt::fmt
  glm::glm
  happly::happly
  stb::stb
  protobuf::libprotobuf
  spdlog::spdlog
  tinyobjloader::tinyobjloader
)

set_property(TARGET FrameFile PROPERTY FOLDER "Frame")
