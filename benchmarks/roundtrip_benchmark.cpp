#include "bse/parser.hpp"
#include "bse/serializer.hpp"
#include "bse/scene.hpp"

#include <chrono>
#include <iostream>

int main() {
  const auto scene = bse::MakeMinimalScene("benchmark");
  auto bytes = bse::SerializeScene(scene);
  if (!bytes.ok()) {
    std::cerr << bytes.status().message() << "\n";
    return 1;
  }

  constexpr int kIterations = 10000;
  const auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < kIterations; ++i) {
    auto parsed = bse::ParseScene(bytes.value());
    if (!parsed.ok()) {
      std::cerr << parsed.status().message() << "\n";
      return 1;
    }
  }
  const auto stop = std::chrono::steady_clock::now();
  const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
  std::cout << "roundtrips=" << kIterations << " micros=" << micros << "\n";
  return 0;
}
