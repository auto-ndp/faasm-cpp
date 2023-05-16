/*
 * Faaslet that reads as input an object (damon.data file format)
 * and a hotness score threshold and computes the maximum size
 * of the hot memory used by the target application.
 */

#include "faasm/faasm.h"
#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <sstream>
#include <string>
#include <vector>

struct record {
  uint64_t timestamp; // microseconds
  uint64_t memsize;
};

std::string objKey;
int threshold = 0;
std::vector<struct record> records;
uint64_t maxmemsize = 0;

int work() {
  uint32_t fetchedLength{};
  uint8_t* objData = __faasmndp_getMmap(objKey.data(), objKey.size(),
    0, 1 * 1024 * 1024 * 1024, &fetchedLength);

  if (objData == nullptr) {
    const std::string output{
      "FAILED - no object found with the given key"
    };
    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
      output.size());
    return 1;
  }

  std::string objDataString(objData, objData + fetchedLength);
  std::istringstream objDataStream(objDataString);
  std::string line;
  std::vector<std::string> values;

  while (std::getline(objDataStream, line)) {
    std::istringstream iss(line);
    std::string value;
    while (iss >> value) {
      values.push_back(value);
    }

    uint64_t ts = (uint64_t)(std::stod(values.at(3)) * 1e6);

    std::string addrstoken = values.at(7);
    uint64_t start_addr = std::stoull(
      addrstoken.substr(0, addrstoken.find("-")), nullptr, 16);
    uint64_t end_addr = std::stoull(
      addrstoken.substr(addrstoken.find("-") + 1), nullptr, 16);

    int accesses = std::stoi(values.at(9));

    if (records.empty() || records.back().timestamp != ts) {
      if (!records.empty()) {
        if (maxmemsize < records.back().memsize)
          maxmemsize = records.back().memsize;
        records.clear();
      }

      struct record rec;
      rec.timestamp = ts;
      if (accesses >= threshold)
        rec.memsize = end_addr - start_addr;
      else
        rec.memsize = 0;

      records.push_back(rec);
    } else {
      if (accesses >= threshold)
        records.back().memsize += end_addr - start_addr;
    }

    values.clear();
  }

  if (!records.empty()) {
    if (maxmemsize < records.back().memsize)
      maxmemsize = records.back().memsize;
    records.clear();
  }

  return 0;
}

int main(int argc, char* argv[]) {

  long inputSz = faasmGetInputSize();
  std::vector<uint8_t> inputBuf(inputSz);
  faasmGetInput(inputBuf.data(), inputBuf.size());
  std::string inputStr(reinterpret_cast<char*>(inputBuf.data()),
                       inputBuf.size());
  size_t spacePos{ inputStr.find(" ") };
  if (inputStr.size() < 1 || spacePos == std::string::npos ||
    inputStr.size() == spacePos + 1) {
    const std::string output{
      "FAILED - no key/value pair. Input '<object> <threshold>'"
    };
    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
      output.size());
    return 0;
  }

  objKey = inputStr.substr(0, spacePos);
  threshold = std::stoi(inputStr.substr(spacePos + 1));
  
  records.reserve(1);
  if (__faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), work) != 0)
    return 1;

  std::string output(std::to_string(maxmemsize));

  faasmSetOutput(reinterpret_cast<uint8_t*>(output.data()), output.size());
  return 0;
}