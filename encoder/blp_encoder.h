#pragma once
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(BLP_ENCODER_BUILD)
#    define BLP_ENCODER_API __declspec(dllexport)
#  else
#    define BLP_ENCODER_API __declspec(dllimport)
#  endif
#else
#  define BLP_ENCODER_API
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* ABI 1：连续 RGBA 输入，输出为单级 JPEG BLP1；不读写文件，不缩放。
 * rgba_size 必须恰好是 width * height * 4；宽高为 1..65500，quality 为 1..100。
 * 参数不钳制。output 在调用前必须为 {NULL, 0}，失败保持为空；非空时拒绝并保留原缓冲。
 * error 可为 NULL；否则 error_capacity 必须大于 0，诊断为截断后带 NUL 的 UTF-8。
 * 返回 0 成功、1 参数错误、2 编码错误、3 内存分配失败。异常不越过 C 边界。
 * 成功后只能由本 DLL 的 blp_encoder_free 释放。输入在函数返回后不再保留。
 * 每次调用独立持有编码器和诊断缓冲，可由多个线程同时调用。
 */
typedef struct BlpEncoderBuffer {
    uint8_t* data;
    size_t size;
} BlpEncoderBuffer;

BLP_ENCODER_API uint32_t blp_encoder_abi_version(void);
BLP_ENCODER_API const char* blp_encoder_version(void);
BLP_ENCODER_API int blp_encoder_encode_rgba(
    const uint8_t* rgba, size_t rgba_size, uint32_t width, uint32_t height,
    int quality, BlpEncoderBuffer* output, char* error, size_t error_capacity);
/* NULL 和已清空的缓冲可释放；有效缓冲必须由同一 DLL 返回且未经修改。 */
BLP_ENCODER_API void blp_encoder_free(BlpEncoderBuffer* output);

#ifdef __cplusplus
}
#endif
