/* 用纯 C 消费导出接口；无需查看器、文件输入或 GUI。 */
#include "blp_encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(value) do { if (!(value)) { fprintf(stderr, "失败：%s（行%d）\n", #value, __LINE__); return 1; } } while (0)
static uint32_t u32(const uint8_t* data) { uint32_t value; memcpy(&value, data, 4); return value; }

int main(void) {
    uint8_t rgba[16 * 8 * 4];
    uint8_t original[sizeof(rgba)];
    BlpEncoderBuffer output = {NULL, 0};
    char error[128];
    size_t pixel;
    int iteration;
    CHECK(blp_encoder_abi_version() == 1);
    CHECK(strcmp(blp_encoder_version(), "blp-encoder 1.0.0") == 0);
    for (pixel = 0; pixel < 16 * 8; ++pixel) {
        rgba[pixel * 4] = 80; rgba[pixel * 4 + 1] = 120;
        rgba[pixel * 4 + 2] = 160; rgba[pixel * 4 + 3] = 128;
    }
    memcpy(original, rgba, sizeof(rgba));
    CHECK(blp_encoder_encode_rgba(NULL, sizeof(rgba), 16, 8, 100, &output, error, sizeof(error)) == 1);
    CHECK(error[0] && !output.data && !output.size);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba) - 1, 16, 8, 100, &output, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 0, 8, 100, &output, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), UINT32_MAX, UINT32_MAX, 100, &output, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 0, &output, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 101, &output, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 100, NULL, error, sizeof(error)) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 100, &output, error, 0) == 1);
    CHECK(blp_encoder_encode_rgba(rgba, 0, 16, 8, 100, &output, error, 1) == 1 && error[0] == 0);
    for (iteration = 0; iteration < 512; ++iteration) {
        CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 100, &output, error, sizeof(error)) == 0);
        CHECK(output.data && output.size > 160 && !error[0]);
        CHECK(!memcmp(output.data, "BLP1", 4) && u32(output.data + 4) == 0);
        CHECK(u32(output.data + 8) == 8 && u32(output.data + 12) == 16 && u32(output.data + 16) == 8);
        CHECK(u32(output.data + 24) == 0 && u32(output.data + 28) >= 160 && !u32(output.data + 32));
        CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 100, &output, error, sizeof(error)) == 1);
        CHECK(output.data && output.size > 160); /* 拒绝覆盖尚未释放的缓冲。 */
        blp_encoder_free(&output);
        CHECK(!output.data && !output.size);
        blp_encoder_free(&output);
    }
    CHECK(!memcmp(original, rgba, sizeof(rgba)));
    CHECK(blp_encoder_encode_rgba(rgba, 3 * 5 * 4, 3, 5, 90, &output, NULL, 0) == 0);
    CHECK(u32(output.data + 12) == 3 && u32(output.data + 16) == 5);
    blp_encoder_free(&output);
    for (pixel = 0; pixel < 16 * 8; ++pixel) rgba[pixel * 4 + 3] = 255;
    CHECK(blp_encoder_encode_rgba(rgba, sizeof(rgba), 16, 8, 100, &output, error, sizeof(error)) == 0);
    CHECK(u32(output.data + 8) == 0);
    blp_encoder_free(&output);
    blp_encoder_free(NULL);
    puts("C ABI 参数、长度、输入保护、尺寸和512次分配释放检查通过。");
    return 0;
}
