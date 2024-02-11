#include "faasm/faasm.h"

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <string_view>
#include <vector>
#include <fstream>
#include <sstream>

using std::string_view;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        const string_view output{
            "FAILED - no key/file pair. Usage: put_simple with input 'key file'"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 2;
    }

    const string_view objKey{ argv[1] };

    std::ifstream file(argv[2]);
    if (!file) {
        const string_view output{
            "FAILED - could not open file"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContents = buffer.str();
    const string_view objValue{ fileContents };

    int32_t result =
      __faasmndp_put(objKey.data(),
                     objKey.size(),
                     reinterpret_cast<const uint8_t*>(objValue.data()),
                     objValue.size());

    if (result != 0) {
        const string_view output{
            "Error creating/updating the object with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }

    const string_view output{ "OK" };
    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                   output.size());

    return 0;
}
