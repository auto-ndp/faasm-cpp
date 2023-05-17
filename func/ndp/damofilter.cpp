/*
 * Faaslet that reads as input an object (damon.data file format)
 * and a hotness score threshold and filters the object to keep
 * only entries that are above this hotness threshold.
 */

#include "faasm/faasm.h"
#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <sstream>
#include <string>
#include <vector>

std::string objKey;
int threshold = 0;

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
  std::stringstream filtDataStream;
  std::string line;
  std::vector<std::string> values;

  while (std::getline(objDataStream, line)) {
    std::istringstream iss(line);
    std::string value;
    while (iss >> value) {
      values.push_back(value);
    }

    int accesses = std::stoi(values.at(9));
    if (accesses >= threshold)
      filtDataStream << line << std::endl;

    values.clear();
  }

  const std::string tmp = filtDataStream.str();
  const char* filteredData = tmp.c_str();
  std::string filtObjKey = objKey + "_filter" + std::to_string(threshold);

  int32_t result = __faasmndp_put(filtObjKey.data(), filtObjKey.size(),
    reinterpret_cast<const uint8_t*>(filteredData.data()),
    filteredData.size());

  if (result != 0) {
    const string_view output{
      "Error creating/updating the object with the given key"
    };
    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
      output.size());
    return 1;
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

  const std::string output{ "OK" };
  faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
    output.size());

  return 0;
}