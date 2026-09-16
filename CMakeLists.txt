cmake_minimum_required(VERSION 3.21)
project(vk_devcheck LANGUAGES CXX)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# 通配符匹配
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
add_executable(vk_devcheck ${APP_SOURCES})

target_compile_features(vk_devcheck PRIVATE cxx_std_20)
set_target_properties(vk_devcheck PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(vk_devcheck PRIVATE Vulkan::Vulkan glfw glm::glm tinyobjloader::tinyobjloader imgui::imgui)
if(MSVC)
  target_compile_options(vk_devcheck PRIVATE /utf-8 /W4)
else()
  target_compile_options(vk_devcheck PRIVATE -Wall -Wextra)
endif()

# 编译 GLSL，产物集中放在项目根目录的 Shaders，供后续渲染代码使用。
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)
set(SHADER_OUTPUTS)
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
foreach(src IN LISTS SHADER_SOURCES)
  get_filename_component(shader "${src}" NAME)
  set(dst "${SHADER_OUTPUT_DIR}/${shader}.spv")
  add_custom_command(OUTPUT "${dst}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
    COMMAND Vulkan::glslc "${src}" -o "${dst}"
    DEPENDS "${src}"
    VERBATIM)
  list(APPEND SHADER_OUTPUTS "${dst}")
endforeach()
add_custom_target(shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(vk_devcheck shaders)

option(LVE_BUILD_TESTS "Build interactive Vulkan/ImGui smoke tests (requires a GPU and desktop)" OFF)
if(LVE_BUILD_TESTS)
  enable_testing()
  add_executable(camera_capture_test tests/camera_capture_test.cpp)
  target_compile_features(camera_capture_test PRIVATE cxx_std_20)
  target_include_directories(camera_capture_test PRIVATE src)
  add_test(NAME camera_capture COMMAND camera_capture_test)
  add_executable(imgui_smoke tests/imgui_smoke.cpp
    src/lve_Window.cpp src/lve_device.cpp src/lve_renderer.cpp
    src/lve_swap_chain.cpp src/lve_imgui_layer.cpp)
  target_compile_features(imgui_smoke PRIVATE cxx_std_20)
  target_include_directories(imgui_smoke PRIVATE src)
  target_link_libraries(imgui_smoke PRIVATE Vulkan::Vulkan glfw imgui::imgui)
  if(MSVC)
    target_compile_options(imgui_smoke PRIVATE /utf-8 /W4)
  endif()
  add_test(NAME imgui_smoke COMMAND imgui_smoke)
  set_tests_properties(imgui_smoke PROPERTIES TIMEOUT 30
    FAIL_REGULAR_EXPRESSION "VUID-|Validation Error|ImGui Vulkan result:")
endif()
