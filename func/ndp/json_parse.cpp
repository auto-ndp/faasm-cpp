#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <sstream>
#include <string_view>
#include <vector>

#include "modern_json/json.hpp"

using std::string_view;
using json = nlohmann::json;

string_view objKey;
json jObj;

int offloaded()
{
    uint32_t fetchedLength{};

    char* objData = (char*)__faasmndp_getMmap(
      objKey.data(), objKey.size(), 0, 1 * 1024 * 1024 * 1024, &fetchedLength);
    if (objData == nullptr) {
        const string_view output{
            "FAILED - no object found with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 0;
    }
    string_view objView(objData, fetchedLength);

    jObj = json::parse(objView.begin(), objView.end());
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
            "FAILED - no key/value pair. Usage: json_parse with input 'key'"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 0;
    }
    objKey = inputStr;

    __faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), offloaded);

    uint64_t numNodes = 0;
    std::string dump = jObj.dump();
    std::stringstream outputs;
    outputs << "Dump length is: " << (int)dump.length() << "\n";
    auto output = outputs.str();

    faasmSetOutput((uint8_t*)output.data(), output.length());
    return 0;
}
