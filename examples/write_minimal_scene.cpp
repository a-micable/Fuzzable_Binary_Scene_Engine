#include "bse/serializer.hpp"
#include "bse/scene.hpp"

#include <fstream>
#include <iostream>

int main() {
  auto bytes = bse::SerializeScene(bse::MakeMinimalScene("example"));
  if (!bytes.ok()) {
    std::cerr << bytes.status().message() << "\n";
    return 1;
  }
  std::ofstream out("minimal.bsen", std::ios::binary);
  out.write(reinterpret_cast<const char*>(bytes.value().data()),
            static_cast<std::streamsize>(bytes.value().size()));
  return out.good() ? 0 : 1;
}
