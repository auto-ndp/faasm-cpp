#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <string_view>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_ONLY_PNG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

using std::string_view;

constexpr const int SCALE = 4;

string_view objKey;
uint8_t* oldRgbaData;
int oldWidth{}, oldHeight{}, channelsInFile{};

int worker()
{
    uint32_t fetchedLength{};
    uint8_t* objData =
      __faasmndp_getMmap(reinterpret_cast<const uint8_t*>(objKey.data()),
                         objKey.size(),
                         1 * 1024 * 1024 * 1024,
                         &fetchedLength);

    if (objData == nullptr || fetchedLength < 4) {
        const string_view output{
            "FAILED - no object found with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }

    oldRgbaData = stbi_load_from_memory(objData,
                                        fetchedLength,
                                        &oldWidth,
                                        &oldHeight,
                                        &channelsInFile,
                                        STBI_rgb_alpha);

    if (oldRgbaData == nullptr || oldWidth == 0 || oldHeight == 0 ||
        channelsInFile == 0) {
        const string_view output{
            "FAILED - object found with the given key is not a valid image"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    long inputSz = faasmGetInputSize();
    std::vector<uint8_t> inputKey(inputSz);
    faasmGetInput(inputKey.data(), inputKey.size());
    if (inputKey.size() < 2) {
        const string_view output{ "FAILED - no key/value pair. Usage: "
                                  "thumbnailer with input 'key value'" };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    objKey =
      string_view(reinterpret_cast<char*>(inputKey.data()), inputKey.size());

    __faasmndp_storageCallAndAwait(worker);

    int newWidth{ oldWidth / SCALE };
    int newHeight{ oldHeight / SCALE };
    std::vector<uint8_t> newData(4 * newWidth * newHeight);

    for (int y = 0; y < newHeight; y++) {
        int oldYmax = (y + 1) * SCALE;
        for (int x = 0; x < newWidth; x++) {
            int oldXmax = (x + 1) * SCALE;
            for (int chan = 0; chan < 4; chan++) {
                int avg{ 0 };
                for (int oldY = y * SCALE; oldY < oldYmax; oldY++) {
                    for (int oldX = x * SCALE; oldX < oldXmax; oldX++) {
                        avg += oldRgbaData[(oldY * oldWidth + oldX) * 4 + chan];
                    }
                }
                avg /= SCALE * SCALE;
                newData[(y * newWidth + x) * 4 + chan] =
                  static_cast<uint8_t>(avg);
            }
        }
    }

    int outPngLen{ 0 };
    uint8_t* outPng = stbi_write_png_to_mem(
      newData.data(), 4 * newWidth, newWidth, newHeight, 4, &outPngLen);
    if (outPng == nullptr) {
        const string_view output{ "FAILED - couldn't encode output PNG" };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }

    faasmSetOutput(reinterpret_cast<const uint8_t*>(outPng), outPngLen);
    STBI_FREE(outPng);

    return 0;
}
