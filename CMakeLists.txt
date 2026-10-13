cmake_minimum_required(VERSION 3.16)
project(cinzel CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)        # Caso não seja especificado, o padrão é Debug
endif()

# ---------- Núcleo (algoritmos de processamento de imagens) ----------
file(GLOB CORE_SRC src/core/*.cpp)
add_library(cinzel_core STATIC ${CORE_SRC})
target_include_directories(cinzel_core PUBLIC src/core external/stb)
target_compile_options(cinzel_core PRIVATE -Wall -Wextra)

# ---------- Testes ----------
add_executable(cinzel_testes src/testes/testes.cpp)
target_link_libraries(cinzel_testes PRIVATE cinzel_core)

# ---------- Interface (ImGui baixado automaticamente) ----------
include(FetchContent)
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9)
FetchContent_MakeAvailable(imgui)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(cinzel src/gui/main.cpp)
target_link_libraries(cinzel PRIVATE cinzel_core imgui)
