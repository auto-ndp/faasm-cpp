#include "faasm/faasm.h"

#include "ndpapi.h"

#include "sha2.h"

#include <stdio.h>
#include <string.h>

#include <string_view>
#include <vector>

using std::string_view;

string_view objKey;
std::string outputHash;

int work()
{
    uint32_t fetchedLength{};
    uint8_t* objData = __faasmndp_getMmap(
      objKey.data(), objKey.size(), 0, 1 * 1024 * 1024 * 1024, &fetchedLength);
    if (objData == nullptr) {
        const string_view output{
            "FAILED - no object found with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    auto rawHash = sha256::compute(objData, fetchedLength);
    bytes_to_hex_string(rawHash, outputHash);
    return 0;
}

int main(int argc, char* argv[])
{
    long inputSz = faasmGetInputSize();
    std::vector<uint8_t> inputBuf(inputSz);
    faasmGetInput(inputBuf.data(), inputBuf.size());
    string_view inputStr(reinterpret_cast<char*>(inputBuf.data()),
                         inputBuf.size());
    if (inputStr.size() < 1) {
        const string_view output{
            "FAILED - invalid args. Usage: sha256sum with input 'key'"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 0;
    }
    objKey = inputStr;

    if (__faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), work) !=
        0) {
        return 1;
    }

    faasmSetOutput(reinterpret_cast<uint8_t*>(outputHash.data()),
                   outputHash.size());
    return 0;
}
