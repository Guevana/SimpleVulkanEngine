cmake_minimum_required(VERSION 3.21)
project(vk_devcheck LANGUAGES CXX)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

# 通配符匹配
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
add_executable(vk_devcheck ${APP_SOURCES})

target_compile_features(vk_devcheck PRIVATE cxx_std_20)
set_target_properties(vk_devcheck PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(vk_devcheck PRIVATE Vulkan::Vulkan glfw glm::glm tinyobjloader::tinyobjloader)
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
