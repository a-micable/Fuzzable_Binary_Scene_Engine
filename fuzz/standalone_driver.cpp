#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size);

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    std::ifstream in(argv[i], std::ios::binary);
    if (!in) {
      return 1;
    }
    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
    LLVMFuzzerTestOneInput(bytes.data(), bytes.size());
  }
  return 0;
}
