#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <string_view>
#include <vector>

using std::string_view;

string_view objKey;
string_view expression;
std::vector<size_t> matches;

int work()
{
    uint32_t fetchedLength{};
    uint8_t* objData =
      __faasmndp_getMmap(reinterpret_cast<const uint8_t*>(objKey.data()),
                         objKey.size(),
                         1 * 1024 * 1024 * 1024,
                         &fetchedLength);
    if (objData == nullptr) {
        const string_view output{
            "FAILED - no object found with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    char* charData = reinterpret_cast<char*>(objData);
    string_view objView(charData, fetchedLength);
    size_t startPos = 0;
    for (size_t foundPos = objView.find(expression);
         foundPos != string_view::npos;
         foundPos = objView.find(expression, startPos)) {
        matches.push_back(foundPos);
        startPos = foundPos + expression.size();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    long inputSz = faasmGetInputSize();
    std::vector<uint8_t> inputBuf(inputSz);
    faasmGetInput(inputBuf.data(), inputBuf.size());
    string_view inputStr(reinterpret_cast<char*>(inputBuf.data()),
                         inputBuf.size());
    size_t spacePos{ inputStr.find_first_of(' ') };
    if (inputStr.size() < 1 || spacePos == std::string_view::npos ||
        inputStr.size() == spacePos + 1) {
        const string_view output{
            "FAILED - no key/value pair. Usage: grep with input 'key regex'"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 0;
    }
    objKey = inputStr.substr(0, spacePos);
    expression = inputStr.substr(spacePos + 1);
    matches.reserve(64);

    if (__faasmndp_storageCallAndAwait(work) != 0) {
        return 1;
    }

    std::string output;
    output.reserve(matches.size() * 4);
    for (const auto& match : matches) {
        output += std::to_string(match);
        output.push_back(' ');
    }

    faasmSetOutput(reinterpret_cast<uint8_t*>(output.data()), output.size());
    return 0;
}
