#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <array>

#ifndef SHA256_H
#define SHA256_H

#include <array>


template <typename InIter>
void output_hex(InIter first, InIter last, std::ostream& os) {
    os.setf(std::ios::hex, std::ios::basefield);
    while (first != last) {
        os.width(2);
        os.fill('0');
        os << static_cast<unsigned int>(*first);
        ++first;
    }
    os.setf(std::ios::dec, std::ios::basefield);
}

template <typename InIter>
void bytes_to_hex_string(InIter first, InIter last, std::string& hex_str) {
    std::ostringstream oss;
    output_hex(first, last, oss);
    hex_str.assign(oss.str());
}

template <typename InContainer>
void bytes_to_hex_string(const InContainer& bytes, std::string& hex_str) {
    bytes_to_hex_string(bytes.begin(), bytes.end(), hex_str);
}

template <typename InIter>
std::string bytes_to_hex_string(InIter first, InIter last) {
    std::string hex_str;
    bytes_to_hex_string(first, last, hex_str);
    return hex_str;
}

template <typename InContainer>
std::string bytes_to_hex_string(const InContainer& bytes) {
    std::string hex_str;
    bytes_to_hex_string(bytes, hex_str);
    return hex_str;
}

namespace sha256 {

using HashType = std::array<uint8_t, 32>;
HashType compute(const uint8_t* Data, const uint64_t Len);

} // sha256

#endif

#ifndef INTMEM_H
#define INTMEM_H

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace intmem {

// bswap functions. Uses GCC/clang/MSVC intrinsics.
#ifdef _MSC_VER
#include <stdlib.h>
static uint8_t  bswap(uint8_t V) { return V; }
static uint16_t bswap(unsigned short V) { return _byteswap_ushort(V); }
static_assert(sizeof(uint32_t) == sizeof(unsigned long), "unsigned long isn't 32-bit wide!");
static uint32_t bswap(uint32_t V) { return _byteswap_ulong(V); }
static uint64_t bswap(uint64_t V) { return _byteswap_uint64(V); }
#else
static uint8_t  bswap(uint8_t V) { return V; }
static uint16_t bswap(uint16_t V) { return __builtin_bswap16(V); }
static uint32_t bswap(uint32_t V) { return __builtin_bswap32(V); }
static uint64_t bswap(uint64_t V) { return __builtin_bswap64(V); }
#endif

static int8_t  bswap(int8_t V) { return V; }
static int16_t bswap(int16_t V) { return bswap((uint16_t)V); }
static int32_t bswap(int32_t V) { return bswap((uint32_t)V); }
static int64_t bswap(int64_t V) { return bswap((uint64_t)V); }

template <class T>
static T loadu(const void* Ptr)
{
  static_assert(std::is_integral<T>::value, "T must be an integer!");
  T Ret;
  memcpy(&Ret, Ptr, sizeof(T));
  return Ret;
}

template <class T>
static void storeu(void* Ptr, T const V)
{
  static_assert(std::is_integral<T>::value, "T must be an integer!");
  memcpy(Ptr, &V, sizeof(V));
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
template <class T>
static T bswap_le(T const v)
{
  return v;
}

template <class T>
static T bswap_be(T const v)
{
  return bswap(v);
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
template <class T>
static T bswap_le(T const v)
{
  return bswap(v);
}

template <class T>
static T bswap_be(T const v)
{
  return v;
}
#else
#error Unsupported endianess!
#endif

template <class T>
static T loadu_le(const void* Ptr)
{
  return bswap_le(loadu<T>(Ptr));
}

template <class T>
static T loadu_be(const void* Ptr)
{
  return bswap_be(loadu<T>(Ptr));
}

template <class T>
static void storeu_le(void* Ptr, T const V)
{
  storeu(Ptr, bswap_le(V));
}

template <class T>
static void storeu_be(void* Ptr, T const V)
{
  storeu(Ptr, bswap_be(V));
}

template <class T>
static T load_le(const T* Ptr)
{
  return bswap_le(*Ptr);
}

template <class T>
static T load_be(const T* Ptr)
{
  return bswap_be(*Ptr);
}

template <class T>
static void store_le(T* Ptr, T const V)
{
  *Ptr = bswap_le(V);
}

template <class T>
static void store_be(T* Ptr, T const V)
{
  *Ptr = bswap_be(V);
}

} // intmem 

#endif

#if defined(_MSC_VER)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

static const uint32_t SHA256_K[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
  0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
  0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
  0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
  0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
  0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

using StateType = std::array<uint32_t, 8>;
using BlockType = std::array<uint32_t, 16>;
using WType     = std::array<uint32_t, 64>;

static FORCEINLINE uint32_t rotr(uint32_t const v, int off)
{
  return (v >> off) | (v << (32-off));
}

static FORCEINLINE void transform(StateType& S, uint8_t const* Data)
{
  WType W = {0};
#pragma unroll
  for (size_t i = 0; i < 16; ++i) {
    W[i] = intmem::loadu_be<uint32_t>(&Data[i*sizeof(uint32_t)]);
  }

  for (size_t i = 16; i < 64; ++i) {
      const uint32_t s0 = rotr(W[i-15], 7) ^ rotr(W[i-15], 18) ^ (W[i-15] >> 3);
      const uint32_t s1 = rotr(W[i-2], 17) ^ rotr(W[i-2], 19)  ^ (W[i-2] >> 10);
      W[i] = (W[i-16] + s0 + W[i-7] + s1);
  }

  StateType InS = S;
  for (size_t i = 0; i < 64; ++i) {
      uint32_t s0 = rotr(InS[0], 2) ^ rotr(InS[0], 13) ^ rotr(InS[0], 22);
      uint32_t maj = (InS[0] & InS[1]) ^ (InS[0] & InS[2]) ^ (InS[1] & InS[2]);
      uint32_t t2 = s0 + maj;
      uint32_t s1 = rotr(InS[4], 6) ^ rotr(InS[4], 11) ^ rotr(InS[4], 25);
      uint32_t ch = (InS[4] & InS[5]) ^ ((~InS[4]) & InS[6]);
      uint32_t t1 = InS[7] + s1 + ch + SHA256_K[i] + W[i];

      InS[7] = InS[6];
      InS[6] = InS[5];
      InS[5] = InS[4];
      InS[4] = (InS[3] + t1);
      InS[3] = InS[2];
      InS[2] = InS[1];
      InS[1] = InS[0];
      InS[0] = (t1 + t2);
  }

  for (size_t i = 0; i < std::tuple_size<StateType>(); ++i) {
    S[i] += InS[i];
  }
}

sha256::HashType sha256::compute(const uint8_t* Data, const uint64_t Len)
{
  StateType State = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  const uint64_t BlockCount = Len/sizeof(BlockType);
  for (uint64_t i = 0; i < BlockCount; ++i) {
    transform(State, &Data[i*sizeof(BlockType)]);
  }

  const uint64_t Rem = Len-BlockCount*sizeof(BlockType);

  uint8_t LastBlock[sizeof(BlockType)];
  memset(&LastBlock, 0, sizeof(LastBlock));
  memcpy(&LastBlock[0], &Data[BlockCount*sizeof(BlockType)], Rem);
  LastBlock[Rem] = 0x80;
  if (Rem >= 56) {
    transform(State, LastBlock);
    memset(&LastBlock, 0, sizeof(LastBlock));
  }
  intmem::storeu_be(&LastBlock[56], Len << 3);
  transform(State, LastBlock);

  HashType Ret;
  static_assert(sizeof(HashType) == sizeof(StateType), "bad definition of HashType");
  for (size_t i = 0; i < 8; ++i) {
    intmem::storeu_be(&Ret[i*sizeof(uint32_t)], State[i]);
  }
  return Ret;
}