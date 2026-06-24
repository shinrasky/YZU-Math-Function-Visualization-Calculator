# 数学函数可视化计算器（C + raylib）

本项目基于 C99 与 raylib 实现一个可交互的数学函数可视化计算器，覆盖平面函数绘制、基础交点计算和 2D/3D 视图切换，采用分层模块化结构，便于后续扩展隐函数、手绘拟合和空间曲面绘制。

## 1. 当前已实现功能

- 2D 坐标系网格与坐标轴绘制
- 视图平移与缩放（鼠标交互）
- 表达式解析与求值（递归下降）
- 显函数绘制：$y=f(x)$
- 极坐标函数绘制：$r=f(\theta)$
- 函数列表管理：添加、显隐、删除、清空
- 自动交点标注（对前两条可见显函数）
- 2D / 3D 模式切换（3D 参考场景 + 自由相机）

## 2. 目录结构

```text
Mathematic/
├── include/
│   ├── core/
│   │   ├── coord_sys.h
│   │   ├── expr_parser.h
│   │   ├── fitting.h
│   │   ├── function.h
│   │   └── numerical.h
│   ├── render/
│   │   ├── render2d.h
│   │   ├── render3d.h
│   │   └── view.h
│   ├── ui/
│   │   ├── input.h
│   │   └── widgets.h
│   └── utils/
│       ├── common.h
│       └── list.h
├── src/
│   ├── core/
│   ├── render/
│   ├── ui/
│   ├── utils/
│   └── main.c
├── assets/
├── CMakeLists.txt
├── plan.md
└── README.md
```

## 3. 依赖与环境

- Windows 10/11
- MinGW-w64（含 `gcc`、`mingw32-make`）
- CMake 3.15+
- raylib 4.5+（推荐预编译包）

> 默认约定 raylib 路径：`third_party/raylib`
>
> 也可在 CMake 配置时通过 `-DRAYLIB_ROOT=...` 指定。

## 4. 构建说明

### 4.1 准备 raylib

将 raylib 目录组织成如下结构（示例）：

```text
third_party/
└── raylib/
    ├── include/
    │   └── raylib.h
    └── lib/
        ├── libraylib.a
        └── (可能还有其他 .a)
```

### 4.2 命令行构建（MinGW Makefiles）

在项目根目录执行：

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

若 raylib 不在默认目录：

```bash
cmake .. -G "MinGW Makefiles" -DRAYLIB_ROOT="C:/your/path/to/raylib"
mingw32-make
```

构建成功后可执行文件默认为：

- `build/math_calculator.exe`

## 5. 运行说明

在 `build` 目录执行：

```bash
./math_calculator.exe
```

## 6. 操作说明

### 6.1 基础交互

- 左上角 `Switch 2D / Switch 3D`：切换 2D 与 3D 模式
- `Tab`：快捷切换 2D / 3D

### 6.2 2D 模式

- 左侧输入框输入表达式并 `Add` 或按 `Enter` 添加
- 输入示例：
  - `y=sin(x)`
  - `y=x^2-2*x+1`
  - `r=1+cos(theta)`
- 在绘图区：
  - 鼠标左键拖拽：平移（输入框未激活时）
  - 鼠标滚轮：缩放
- 函数列表每项支持：
  - `V/H`：显示/隐藏
  - `X`：删除
- `Clear`：清空全部函数

### 6.3 交点标注

程序会对“前两条可见显函数（非极坐标）”自动计算并标出交点。

例如添加：

- `y=x`
- `y=x^2`

将在图上标出 `(0,0)` 与 `(1,1)` 附近交点。

### 6.4 3D 模式

- 使用 raylib 自由相机（`CAMERA_FREE`）进行视角浏览
- 当前提供 3D 参考场景（网格 + 立方体），用于后续扩展空间曲线/曲面

## 7. 表达式语法

### 7.1 运算符

- `+ - * / ^`
- 支持括号：`( ... )`

### 7.2 函数

- `sin cos tan`
- `asin acos atan`
- `sqrt exp abs`
- `ln log`

### 7.3 常量与变量

- 常量：`pi`, `e`
- 变量：
  - 显函数常用 `x`
  - 极坐标常用 `theta`

## 8. 常见问题

### 8.1 找不到 raylib 头文件或库

请检查：

- `third_party/raylib/include/raylib.h` 是否存在
- `third_party/raylib/lib/libraylib.a` 是否存在
- CMake 配置时 `RAYLIB_ROOT` 是否正确

### 8.2 编译时报未定义引用（OpenGL/Win32）

本项目已在 `CMakeLists.txt` 中链接：

- `raylib winmm gdi32 opengl32`

若你使用的 raylib 包有额外依赖，请按其文档补充链接库。

## 9. 后续扩展建议

- 隐函数绘制（Marching Squares）
- 手绘轨迹采样 + 最小二乘拟合 UI
- 空间曲面采样与渲染（线框/填充）
- 表达式错误定位修复与更友好的提示
- 导出图片（PNG）
