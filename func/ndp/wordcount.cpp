#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using std::string_view;

// faasm currently doesn't support passing arguments to functions invoked,
// overcome that with global memory for the time being
struct NdpWorkerComm
{
    // input
    string_view objKey;
    // output
    std::unordered_map<std::string, uint32_t> wordCounts;
} workerComm;

int ndp_worker()
{
    // input params
    const auto& objKey = workerComm.objKey;
    // output params
    auto& wordCounts = workerComm.wordCounts;
    // worker code [extracted]
    uint32_t fetchedLength{};
    uint8_t* objData = __faasmndp_getMmap(
      objKey.data(), objKey.size(), 0, 1 * 1024 * 1024 * 1024, &fetchedLength);
    if (objData == nullptr) {
        printf("Error fetching object with key %.*s\n",
               static_cast<int>(objKey.size()),
               objKey.data());
        return 1;
    }
    string_view remainingData(reinterpret_cast<char*>(objData), fetchedLength);
    wordCounts = std::unordered_map<std::string, uint32_t>();
    for (;;) {
        while (!remainingData.empty() && remainingData[0] <= ' ') {
            remainingData = remainingData.substr(1);
        }
        if (remainingData.empty()) {
            break;
        }
        uint32_t nextSpace{ 1 };
        while (nextSpace < remainingData.size() &&
               remainingData[nextSpace] > ' ') {
            nextSpace++;
        }
        string_view word = remainingData.substr(0, nextSpace);
        remainingData = remainingData.substr(nextSpace);
        wordCounts[std::string(word)] += 1;
    }
    return 0;
}

int worker(const string_view objKey, std::vector<char>& outputBuf)
{
    // NDP call into storage
    workerComm = NdpWorkerComm();
    workerComm.objKey = objKey;
    int ndpRetCode =
      __faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), &ndp_worker);
    if (ndpRetCode != 0) {
        return ndpRetCode;
    }
    auto& wordCounts = workerComm.wordCounts;
    // sort and output counts
    std::vector<std::pair<std::string, uint32_t>> sortedCounts;
    sortedCounts.reserve(wordCounts.size());
    uint32_t outSz{ 0 };
    for (auto& pair : wordCounts) {
        outSz += pair.first.size() + 5;
        sortedCounts.push_back(
          std::make_pair(std::string(std::move(pair.first)), pair.second));
    }
    std::sort(sortedCounts.begin(), sortedCounts.end());
    char fmtNumber[16];
    outputBuf.reserve(outSz);
    for (const auto& pair : sortedCounts) {
        outputBuf.insert(outputBuf.end(), pair.first.begin(), pair.first.end());
        outputBuf.push_back('\t');
        int fmtLen = snprintf(fmtNumber,
                              sizeof(fmtNumber),
                              "%u\n",
                              static_cast<unsigned int>(pair.second));
        outputBuf.insert(outputBuf.end(), fmtNumber, fmtNumber + fmtLen);
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
    if (inputStr.size() < 1) {
        const string_view output{ "FAILED - no key. Usage: wordcount key1..." };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    std::vector<char> outputBuf;
    int result = worker(inputStr, outputBuf);
    if (result != 0) {
        return result;
    }

    faasmSetOutput(reinterpret_cast<const uint8_t*>(outputBuf.data()),
                   outputBuf.size());
    return 0;
}
