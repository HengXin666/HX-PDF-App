# HX-PDF-App

## 项目结构说明

```sh
.
|- cpp-backend # C++后端
|- qt-client   # Qt 客户端
```

## 环境搭建
### 一、Win系统配置 (Qt客户端)

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