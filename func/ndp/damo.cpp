#include "faasm/faasm.h"
#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <sstream>
#include <string>
#include <vector>

std::string objKey;
int threshold = 0;
std::vector<std::tuple<double, uint64_t>> accessedMemorySize;

struct entry {
  uint64_t start_addr;
  uint64_t end_addr;
  int accesses;
};

struct record {
  uint32_t timestamp; // microseconds
  int nr_regions;
  std::vector<struct entry> entries;
};

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
  std::vector<struct record> records;

  while (std::getline(objDataStream, line)) {
    std::istringstream iss(line);
    std::string value;
    while (iss >> value) {
      values.push_back(value);
    }

    uint32_t ts = (uint32_t)(std::stod(values.at(3)) * 1e6);

    std::string nregstoken = values.at(6);
    int nregs = std::stoi(nregstoken.substr(nregstoken.find("=") + 1));

    std::string addrstoken = values.at(7);
    uint64_t start_addr = std::stoull(
      addrstoken.substr(0, addrstoken.find("-")), nullptr, 16);
    uint64_t end_addr = std::stoull(
      addrstoken.substr(addrstoken.find("-") + 1), nullptr, 16);

    int accesses = std::stoi(values.at(9));

    if (records.empty() || records.back().timestamp != ts) {
      // add new record with one entry
      struct entry e;
      e.start_addr = start_addr;
      e.end_addr = end_addr;
      e.accesses = accesses;

      struct record rec;
      rec.timestamp = ts;
      rec.nr_regions = nregs;
      rec.entries.push_back(e);

      records.push_back(rec);
    } else {
      // add new entry to last record
      struct entry e;
      e.start_addr = start_addr;
      e.end_addr = end_addr;
      e.accesses = accesses;
      
      records.back().entries.push_back(e);
    }

    values.clear();
  }

  uint32_t start_timestamp = records.front().timestamp;
  for (const auto& rec : records) {
    double ts = (double)((rec.timestamp - start_timestamp) / 1e6);
    uint64_t size = 0;
    for (const auto& e : rec.entries)
      if (e.accesses >= threshold)
        size += e.end_addr - e.start_addr;
    
    accessedMemorySize.push_back(std::tuple<double, uint64_t>{ts, size});
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

  if (__faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), work) != 0)
    return 1;

  std::string output;
  output.reserve(accessedMemorySize.size() * 4);
  for (const auto& x : accessedMemorySize) {
    output += std::to_string(std::get<0>(x)) + ",";
    output += std::to_string(std::get<0>(x)) + "\n";
  }

  faasmSetOutput(reinterpret_cast<uint8_t*>(output.data()), output.size());
  return 0;
}