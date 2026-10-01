#include "blp_encoder.h"
#include "blp/blp_codec.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <limits>
#include <new>

namespace {
void message(char* error, size_t capacity, const char* text) noexcept {
    if (!error || !capacity) return;
    size_t length = std::min(capacity - 1, std::strlen(text));
    while (length && (static_cast<unsigned char>(text[length]) & 0xC0) == 0x80) --length;
    std::memcpy(error, text, length);
    error[length] = '\0';
}
}

uint32_t blp_encoder_abi_version(void) { return 1; }
const char* blp_encoder_version(void) { return "blp-encoder 1.0.0"; }

int blp_encoder_encode_rgba(const uint8_t* rgba, size_t rgba_size,
                           uint32_t width, uint32_t height, int quality,
                           BlpEncoderBuffer* output, char* error, size_t error_capacity) {
    message(error, error_capacity, "");
    const uint64_t bytes = static_cast<uint64_t>(width) * height * 4;
    if (!output || output->data || output->size || !rgba || !width || !height ||
        width > 65500 || height > 65500 || bytes > std::numeric_limits<uint32_t>::max() ||
        rgba_size != bytes || quality < 1 || quality > 100 || (error && !error_capacity)) {
        message(error, error_capacity, "RGBA长度、宽高、质量或输出缓冲无效。");
        return 1;
    }
    try {
        std::vector<uint8_t> encoded;
        std::string detail;
        if (!blpcodec::encode_jpeg_blp1(rgba, width, height, quality, 1, encoded, &detail)) {
            message(error, error_capacity, detail.c_str());
            return 2;
        }
        auto* buffer = static_cast<uint8_t*>(std::malloc(encoded.size()));
        if (!buffer) throw std::bad_alloc();
        std::memcpy(buffer, encoded.data(), encoded.size());
        output->data = buffer;
        output->size = encoded.size();
        return 0;
    } catch (const std::bad_alloc&) {
        message(error, error_capacity, "编码内存分配失败。");
        return 3;
    } catch (const std::exception& exception) {
        message(error, error_capacity, exception.what());
        return 2;
    } catch (...) {
        message(error, error_capacity, "编码器发生未知错误。");
        return 2;
    }
}

void blp_encoder_free(BlpEncoderBuffer* output) {
    if (!output) return;
    std::free(output->data);
    output->data = nullptr;
    output->size = 0;
}
