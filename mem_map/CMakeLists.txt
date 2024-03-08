project (mem_map CXX)

add_executable(mem_map
  mem_map.cpp
)

target_include_directories (mem_map PRIVATE "${HDF5_SRC_INCLUDE_DIRS};${HDF5_SRC_BINARY_DIR}")

if (NOT BUILD_SHARED_LIBS)
  target_link_libraries (mem_map PRIVATE ${HDF5_CPP_LIB_TARGET} ${HDF5_HL_LIB_TARGET} ${HDF5_LIB_TARGET})
else ()
  target_link_libraries (mem_map PRIVATE ${HDF5_CPP_LIBSH_TARGET} ${HDF5_HL_LIBSH_TARGET} ${HDF5_LIBSH_TARGET})
endif ()
