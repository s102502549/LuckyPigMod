set(EXECUTABLE_OUTPUT_PATH      "${CMAKE_CURRENT_SOURCE_DIR}/src/main/assets/${ANDROID_ABI}")
add_executable(write src/main/cpp/write.cpp)
add_executable(read src/main/cpp/read.cpp)