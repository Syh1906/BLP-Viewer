# 独立 BLP1 JPEG 编码组件

`blp_encoder` 为 Windows x64 的 C ABI 动态库。输入连续 RGBA 像素，返回单级 BLP1 JPEG 内存字节，不要求查看器、图形界面、文件路径或 Shell 扩展。接口见 `blp_encoder.h`。版本为 `blp-encoder 1.0.0`，ABI 为 1。

调用顺序：查询 ABI/版本，初始化空 `BlpEncoderBuffer`，调用 `blp_encoder_encode_rgba`，读取成功输出，最后调用同一 DLL 的 `blp_encoder_free`。失败时读取调用者自己的 UTF-8 诊断缓冲；没有进程级 last-error。输出内存不能用宿主的 free/delete 释放。

宽高须为 1 至 65500，RGBA 字节数必须完全匹配且不超过 uint32 可表达范围；quality 为 1 至 100，非法值拒绝。非二次幂原尺寸保留；单级接口不生成 mip、不重采样。调用者可先生成自己的 mip，再逐级编码与封装。JPEG 的 RGB 和 alpha 都有损，quality 100 不保证无损，必须按最终回读像素判断质量。

## 构建与检查

需要已有 CMake、MSVC v143 和 Windows SDK。仓库内提供固定 x64 TurboJPEG 静态库，不下载或安装组件。以下命令只构建独立编码库及其 C 调用验证程序，不构建或注册查看器/缩略图扩展。

```powershell
cmake -S encoder -B <构建目录> -G "Visual Studio 17 2022" -A x64
cmake --build <构建目录> --config Release
ctest --test-dir <构建目录> -C Release --output-on-failure
python encoder/package.py --cmake <cmake.exe> --build <构建目录> --out <新的包目录>
```

`package.py` 会重新构建并运行同一验证程序，核对构建前后来源文件未变，再安装至全新目录并记录来源、逐文件哈希及 ABI。已有输出目录会拒绝。

包内 `bin/blp_encoder.dll` 是运行组件；`include`、`lib` 用于原生开发，普通 ctypes 调用只需 DLL 和许可。该库使用 MSVC 动态运行库，使用者必须满足其运行条件；不要把本机加载成功视为全部 Windows 版本已验证。

## 实现与依赖

编码复用 `src/blp/blp_codec.cpp`，以四通道 JPEG 保存 B/G/R/A，4:4:4 采样，不进行 CMYK 色彩转换。透明输入声明 alphaBits=8，完全不透明输入声明为 0。原 C++ 的 `encode_jpeg_blp1` 仍支持多级生成；其尺寸过滤与此单级 C 接口相互独立。

随仓库静态库标识为 libjpeg-turbo 3.1.0（build 20260426）；文件 SHA256 为 `8fc6951409bebbe04de2f9a2152d0d5250eef5bb8df5e29d63409ef4bbddde15`。这是固定二进制的身份，不声称重建了该静态库或核实其全部编译参数。

本组件遵循仓库 MIT 许可，保留 stb 与 libjpeg-turbo 的原始声明。二进制包必须包含 `licenses`。This software is based in part on the work of the Independent JPEG Group.

libjpeg-turbo 3.1.0 的许可原文来自 [LICENSE.md](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/3.1.0/LICENSE.md) 与 [README.ijg](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/3.1.0/README.ijg)，分别保存在 `third_party/turbojpeg` 中。
