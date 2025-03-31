# HX-PDF-App

## 项目结构说明

```sh
.
|- cpp-backend # C++后端
|- qt-client   # Qt 客户端
```

## 环境搭建
### 一、Win系统配置 (Qt客户端)
#### 1.1 使用到的第三方库

- **[QT](https://www.qt.io/zh-cn/)** `6.8`: 显然这是一个QT项目, 理论上只需要是`6.0+`的版本即可.

- **[MuPdf](https://www.mupdf.com/)** `1.25.5`: 一个基于C语言编写的高性能PDF解析库 (支持多种格式, 如`.epub`等).

> [!TIP]
> 项目并没有提供`MuPdf`的静态库, 因为他们太大了! DeBug + Release 有 1G 大. 请自行编译
>
> 如果是Win系统, 需要把他们放到`./qt-client/lib/mupdf/lib/Debug`与`./qt-client/lib/mupdf/lib/Release`中
>
> ps: 需要三个静态库, 分别是: `libmupdf.lib`, `libresources.lib`, `libthirdparty.lib`.
>
> 如果是Linux系统, 需要手动修改CMake, 因为我没有适配...

#### 1.2 vcpkg + vscode + cmake 需要的配置 (目前项目没有使用到vcpkg, 可以忽略此项)

1. 安装`vcpkg`, 以后可能会用到.

2. 添加`.vscode/settings.json`写入:

```json
// .vscode/settings.json
{
  "cmake.configureSettings": {
    // 你的 vcpkg 的对应路径
    "CMAKE_TOOLCHAIN_FILE": "D:/MyApp/vcpkg/scripts/buildsystems/vcpkg.cmake",
    "VCPKG_TARGET_TRIPLET": "x64-windows"
  }
}
```

3. 下载4个qt插件 (vscode)

4. 修改根cmake:

```cmake
if(WIN32)
    # Qt编译器路径设置
    set(CMAKE_PREFIX_PATH "D:/MyApp/Qt/6.8.2/msvc2022_64")

    # vcpkg下载的包的路径
    set(LIB_ROOT "D:/MyApp/vcpkg/installed/x64-windows")
endif()
```

5. 尝试编译, 如果不行, 请喷qt、vcpkg、msvc, 以及疯狂喷win系统, 然后屁颠屁颠去使用linux!

### 二、Linux配置 (C++后端)

项目基于 [HXLibs](https://github.com/HengXin666/HXLibs) 作为后端框架, 其需要运行在 Linux 下.