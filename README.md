# Vulkan 学习项目

在 VS Code 中打开本目录即可开发。当前程序只创建 Vulkan 实例并检测显卡，成功时输出 `Vulkan environment OK`；尚未创建窗口或绘制三角形。

## 在 VS Code 中使用

本机已安装 **C/C++** 和 **CMake Tools** 扩展。

1. 打开项目后，选择配置预设 **Windows x64**（或在命令面板执行 `CMake: Select Configure Preset`）。
2. 执行 `CMake: Select Build Preset`，选择 **debug**。
3. 执行 `CMake: Build` 编译。
4. 在 `src/main.cpp` 设置断点，执行 `CMake: Debug`；如果提示选择目标，选择 **vk_devcheck**。

代码补全、头文件路径和编译选项由 CMake Tools 自动提供。首次配置尚未完成时，settings.json 中基于环境变量的备用路径用于查找 GLFW 和 Vulkan 头文件。调试前会自动构建，调试工作目录为 `build`，方便以后使用相对路径 `shaders/triangle.vert.spv` 等。使用命令面板中的 CMake 调试入口即可，无需另外维护 tasks.json 或 launch.json。

如果头文件仍有红色波浪线，先执行 `CMake: Configure`，选择 **Windows x64**；之后执行 `C/C++: Reset IntelliSense Database`。修改过环境变量时，需要完全退出并重新打开 VS Code。

也可以在普通 PowerShell 终端中执行：

```powershell
cmake --preset windows
cmake --build --preset debug
./build/Debug/vk_devcheck.exe
```

发布构建使用 `cmake --build --preset release`，程序位于 `build/Release`。

## 文件用途

| 文件 | 用途 |
| --- | --- |
| `src/` | C++ 学习代码；保留现有头文件，方便后续扩展窗口类 |
| `shaders/` | 三角形 GLSL 源码；构建时编译，目前主程序尚未加载它们 |
| `CMakeLists.txt` | 定义程序、依赖库与着色器编译规则 |
| `CMakePresets.json` | 统一指定 Windows x64 工具链以及 Debug/Release 构建 |
| `.vscode/settings.json` | 让 VS Code 使用 CMake 配置代码补全和调试 |
| `.gitignore` | 防止未来使用 Git 时提交构建产物 |
| `build/` | 自动生成的工程文件、程序、DLL 和 SPIR-V；可删除后重新构建，已在 VS Code 文件树中隐藏 |

## 环境依赖

保留已安装的 Visual Studio 2022 C++ 编译工具、CMake、Vulkan SDK 和 vcpkg 中的 GLFW。无需打开 Visual Studio；CMake 使用其生成器自动找到 MSVC，无需手动运行开发者环境脚本，也不再依赖 Ninja。

当前机器环境变量为 `VULKAN_SDK=D:\VulkanSDK\1.4.357.0`、`VCPKG_ROOT=D:\vcpkg`，GLFW 已安装为 `x64-windows`。配置文件通过环境变量查找依赖，不再固定 SDK 或编译器版本路径。换电脑时需要安装同样的依赖并设置这两个环境变量，修改后重启 VS Code。着色器编译需要 SDK 中的 glslc，现在缺失时会在配置阶段明确报错。

清理时移除了重复的 `scripts/build.ps1` 和 `.vscode/c_cpp_properties.json`，以及把 SDK 的 Vulkan loader 复制到程序旁的自检逻辑；程序使用系统 Vulkan 运行时。所有生成文件集中在 `build/`，业务源码和着色器未改动。
