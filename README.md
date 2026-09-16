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
- Dear ImGui 英文调试面板：性能、物体变换、相机参数、背景色和 Demo Window

## 技术栈

| 组件 | 用途 |
| --- | --- |
| C++20 | 应用与渲染代码 |
| Vulkan | 图形 API |
| GLFW | 窗口与输入 |
| GLM | 向量、矩阵与相机计算 |
| CMake | 项目配置与构建 |
| vcpkg | C++ 依赖管理 |
| Dear ImGui 1.92.9b | GLFW/Vulkan backend 与调试界面 |
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
tinyobjloader
imgui[glfw-binding,vulkan-binding]
```

安装依赖示例：

```powershell
vcpkg install vulkan glfw3 glm tinyobjloader "imgui[glfw-binding,vulkan-binding]" --triplet x64-windows
```

沿用 vcpkg classic 模式。当前接入基线为上游 **ImGui 1.92.9b**，vcpkg 包版本显示为 `1.92.9`（port 使用带 `b` 后缀的源码）。本次验证的 vcpkg commit 为 `a1cae005c39be7b18ba319fced856b68d7276271`。使用旧版 port 时先更新到包含该版本的 vcpkg；此实现要求 `IMGUI_VERSION_NUM >= 19291`。

CMake 只链接 `imgui::imgui`，GLFW/Vulkan backend 已由包编译。使用 backend 内置 Descriptor Pool（64）及动态字体纹理管理，不需要单独编译 backend 或手动调用旧版字体上传函数。

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
| 操作 UI | 默认显示光标，拖动参数；Ctrl+单击数值可输入 |
| 进入相机控制 | 在面板外按住鼠标右键 |
| 前后移动 | 按住右键时 `W` / `S` |
| 左右移动 | 按住右键时 `A` / `D` |
| 上下移动 | 按住右键时 `E` / `Q` |
| 视角旋转 | 按住右键时移动鼠标，或使用方向键 |
| 返回 UI | 松开右键、按 `Escape` 或窗口失焦 |
| 退出程序 | 关闭窗口 |

在 UI 内开始的右键按压，即使拖到场景区域也不会接管相机；取消控制后需要重新按下右键。输入框获得键盘输入时相机不处理键盘移动。

`Debug` 面板支持按 ID 选择对象，编辑位置、旋转（角度）和正值缩放；相机支持位置、旋转、FOV（10°–120°）、速度、灵敏度及重置。Rendering 中可编辑清屏背景色及打开 Demo Window。修改当帧生效，仅在当前运行中保留，不生成 `imgui.ini`。

## UI 架构与验证

`LveImguiLayer` 管理 Context 和 backend；`DebugUI` 负责场景面板。UI 绘制位于场景之后、同一个 RenderPass 的 subpass 0 内。交换链重建通过 generation 通知 UI；图片数量变化时重建 Vulkan backend，保留 Context。呈现信号量按交换链图片索引分配；帧栅栏和获取图片信号量仍按两帧并行管理。

可选测试需要可用 GPU、桌面会话及 Debug Vulkan validation layer：

```powershell
cmake --preset windows -DLVE_BUILD_TESTS=ON
cmake --build --preset debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build --preset release
ctest --test-dir build -C Release --output-on-failure
```

- `camera_capture`：UI/场景输入所有权、拖出面板、松键、Escape、失焦、回调取消及重新按压。
- `imgui_smoke`：真实 Vulkan/UI 绘制、兼容 RenderPass、字体重建、六次窗口缩放及关闭。通过合成图片数量元数据覆盖 backend 重建分支，不宣称驱动实际改变了交换链图片数量。Vulkan VUID 校验错误会令测试失败。
- 人工交互复核：拖动变换、相机和背景参数，编辑输入框，按住右键移动/旋转，失焦和 Escape 释放，最小化恢复及关闭。测试记录见 [docs/validation.md](docs/validation.md)。

首版使用英文内置字体、单窗口覆盖面板，不包含 Docking、多窗口或离屏场景视口。

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
	├── lve_imgui_layer.*   # ImGui backend 与资源生命周期
	├── debug_ui.*          # 调试与参数编辑面板
	├── camera_capture.hpp # 相机输入捕获状态机
	├── lve_GameObject.hpp  # 游戏对象与变换
	└── simple_render_system.*
```
