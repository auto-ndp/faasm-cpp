#include "faasm/faasm.h"

#include <Eigen/Dense>

#include "ndpapi.h"

#include <stdio.h>
#include <string.h>

#include <regex>
#include <string_view>
#include <vector>

#include "util/kmeansrex.cpp"

using std::string_view;

string_view objKey;

uint32_t endswap32(uint32_t x)
{
    return ((x & 0xFFu) << 24u) | ((x & 0xFF00u) << 8u) |
           ((x & 0xFF0000u) >> 8u) | ((x & 0xFF000000u) >> 24u);
}

Eigen::MatrixXf pcaReducedData;

int work()
{
    uint32_t fetchedLength{};
    uint8_t* objData = __faasmndp_getMmap(
      objKey.data(), objKey.size(), 0, 1 * 1024 * 1024 * 1024, &fetchedLength);
    if (objData == nullptr || fetchedLength < 64) {
        const string_view output{
            "FAILED - no object found with the given key"
        };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 1;
    }
    const uint32_t magic = endswap32(*reinterpret_cast<uint32_t*>(objData));
    const uint32_t nImages =
      endswap32(*reinterpret_cast<uint32_t*>(objData + 4));
    const uint32_t nRows = endswap32(*reinterpret_cast<uint32_t*>(objData + 8));
    const uint32_t nCols =
      endswap32(*reinterpret_cast<uint32_t*>(objData + 12));
    const uint8_t* pixelDataStart = objData + 16;
    if (magic != 0x00000803) {
        const string_view output{ "FAILED - bad magic number" };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 2;
    }
    const uint32_t pixelsPerImage = nRows * nCols;
    if (pixelsPerImage * nImages > fetchedLength - 16) {
        const string_view output{ "FAILED - missing image data" };
        faasmSetOutput(reinterpret_cast<const uint8_t*>(output.data()),
                       output.size());
        return 3;
    }
    Eigen::MatrixXf imageData(nImages, pixelsPerImage);
    const float fmul = 1.0f / 255.0f;
    for (uint32_t image = 0; image < nImages; image++) {
        const uint8_t* imageStart = pixelDataStart + (nRows * nCols) * image;
        for (uint32_t pixel = 0; pixel < nRows * nCols; pixel++) {
            imageData(image, pixel) = float(imageStart[pixel]) * fmul;
        }
    }
    // normalize data
    {
        Eigen::VectorXf mins = imageData.colwise().minCoeff();
        Eigen::VectorXf maxs = imageData.colwise().maxCoeff();
        Eigen::VectorXf scaleCoeff = maxs - mins;
        for (auto& val : scaleCoeff) {
            val = (val == 0) ? 1.0f : 1.0f / val;
        }
        imageData.rowwise() -= mins.transpose();
        imageData.array().rowwise() *= scaleCoeff.transpose().array();
    }
    // center on means
    {
        Eigen::VectorXf means = imageData.colwise().mean();
        imageData.rowwise() -= means.transpose();
    }
    Eigen::MatrixXf covarianceMat = imageData.adjoint() * imageData;
    covarianceMat = covarianceMat / (imageData.rows() - 1);
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigensolver(covarianceMat);
    Eigen::VectorXf eigenvalues = eigensolver.eigenvalues();
    eigenvalues /= eigenvalues.sum();
    Eigen::MatrixXf eigenvectors = eigensolver.eigenvectors();
    Eigen::MatrixXf pcaTransform = eigenvectors.rightCols(32);
    pcaReducedData = imageData * pcaTransform;
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
            "FAILED - no key/value pair. Usage: grep with input 'key regex'"
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

    const int K = 10;
    const int n_features = pcaReducedData.cols();
    const int n_iters = 25;
    const int seed = 1234567;
    const int n_examples = pcaReducedData.rows();

    Eigen::ArrayXXd dblData = pcaReducedData.array().cast<double>();
    Eigen::ArrayXXd mu = Eigen::ArrayXXd::Zero(K, n_features);
    Eigen::ArrayXd z = Eigen::ArrayXd::Zero(n_examples);
    RunKMeans(dblData.data(),
              n_examples,
              n_features,
              K,
              n_iters,
              seed,
              "plusplus",
              mu.data(),
              z.data());

    std::string output;
    output.reserve(z.rows() * 4);
    for (auto& el : z) {
        output += std::to_string(int(el));
        output.push_back(' ');
    }
    output.push_back('\n');

    faasmSetOutput(reinterpret_cast<uint8_t*>(output.data()), output.size());
    return 0;
}
