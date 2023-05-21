#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <string_view>
#include <vector>

using std::string_view;

std::string objKey;
std::string findStr;
std::string replStr;

int work() {
  uint32_t fetchedLength;
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

  char* charData = reinterpret_cast<char*>(objData);
  std::string objString(charData, fetchedLength);
  size_t startPos = 0;
  for (size_t foundPos = objString.find(findStr); 
    foundPos != std::string::npos;
    foundPos = objString.find(findStr, startPos)) {
    // found, now replace
    objString.replace(foundPos, findStr.length(), replStr);
    startPos = foundPos + replStr.length();
  }

  int32_t result = __faasmndp_put(objKey.data(), objKey.size(),
    reinterpret_cast<const uint8_t*>(objString.data()),
    objString.size());

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
  
  size_t keyEnd = inputStr.find(' ');
  size_t findStrStart = inputStr.find('\'', keyEnd) + 1;
  size_t findStrEnd = inputStr.find('\'', findStrStart);
  size_t replStrStart = inputStr.find('\'', findStrEnd + 1) + 1;
  size_t replStrEnd = inputStr.find('\'', replStrStart);

  if (keyEnd < 1 || findStrStart >= findStrEnd || replStrStart >= replStrEnd) {
    const std::string output{
      "FAILED! Input: \"obj_key 'find_str' 'repl_str'\""
    };
    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
      output.size());
    return 0;
  }

  objKey = inputStr.substr(0, keyEnd);
  findStr = inputStr.substr(findStrStart, findStrEnd);
  replStr = inputStr.substr(replStrStart, replStrEnd);

  if (__faasmndp_storageCallAndAwait(objKey.data(), objKey.size(), work) != 0)
    return 1;

  const std::string output{ "OK" };
  faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
    output.size());

  return 0;
}