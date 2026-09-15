# Vulkan Project

一个基于 Vulkan 的 C++ 实时渲染练习项目。项目从窗口、设备、交换链、渲染管线到模型绘制搭建了完整的 Vulkan 渲染流程。

## 项目特点

- 使用 Vulkan 初始化实例、调试回调、物理设备、逻辑设备和图形队列
- 使用 GLFW 创建窗口并承载 Vulkan surface
- 实现交换链、深度缓冲、命令缓冲和帧同步
- 使用 C++20 编写基础模型、相机、游戏对象和渲染系统
- 使用 push constants 传递对象变换矩阵与颜色
- 使用 GLSL 顶点/片元着色器，并在构建时自动编译为 SPIR-V
- Debug 构建默认启用 `VK_LAYER_KHRONOS_validation` 校验层

## 技术栈

| 组件 | 用途 |
| --- | --- |
| C++20 | 应用与渲染代码 |
| Vulkan | 图形 API |
| GLFW | 窗口与输入 |
| GLM | 向量、矩阵与相机计算 |
| CMake | 项目配置与构建 |
| vcpkg | C++ 依赖管理 |
| GLSL / SPIR-V | 着色器编写与编译 |

## 环境要求

- Windows x64
- Visual Studio 2022（包含 MSVC 和 C++ 桌面开发工具）
- CMake 3.21 或更高版本
- Vulkan SDK，并确保 `glslc` 可用
- vcpkg，并设置 `VCPKG_ROOT` 环境变量
- Vulkan-capable GPU 及对应驱动

项目通过 vcpkg 查找以下依赖：

```text
vulkan
glfw3
glm
```

安装依赖示例：

```powershell
vcpkg install vulkan glfw3 glm --triplet x64-windows
```

## 构建与运行

在项目根目录执行：

```powershell
cmake --preset windows
cmake --build --preset debug
```

构建 Release 版本：

```powershell
cmake --build --preset release
```

运行 Debug 程序：

```powershell
.\build\Debug\vk_devcheck.exe
```

构建过程会自动调用 Vulkan SDK 提供的 `glslc`，将 `shaders/` 下的 `.vert` 和 `.frag` 文件编译为同目录下的 `.spv` 文件。

特意保留了vscode中的配置脚本以供参考。

## 操作方式

| 操作 | 按键 |
| --- | --- |
| 前后移动 | `W` / `S` |
| 左右移动 | `A` / `D` |
| 上下移动 | `E` / `Q` |
| 视角旋转 | 鼠标移动 |
| 备用视角控制 | 方向键 |
| 退出程序 | 关闭窗口 |

## 目录结构

```text
.
├── CMakeLists.txt          # CMake 构建配置与着色器编译规则
├── CMakePresets.json       # Windows x64 配置、Debug/Release 预设
├── shaders/
│   ├── simple_shader.vert  # 顶点着色器
│   ├── simple_shader.frag  # 片元着色器
│   └── *.spv               # 构建生成的 SPIR-V 文件
└── src/
	├── main.cpp
	├── lve_FirstApp.*      # 应用入口与示例场景
	├── lve_Window.*        # GLFW 窗口
	├── lve_device.*        # Vulkan 设备与资源辅助
	├── lve_swap_chain.*    # 交换链与帧同步
	├── lve_renderer.*      # 帧与 render pass 管理
	├── lve_Pipeline.*      # 图形管线
	├── lve_model.*         # 顶点缓冲与模型绘制
	├── lve_camera.*        # 透视相机
	├── lve_GameObject.hpp  # 游戏对象与变换
	└── simple_render_system.*
```