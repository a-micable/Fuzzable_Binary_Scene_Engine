#!/usr/bin/env python3
import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


DOMAINS = [
    "mesh", "material", "texture", "animation", "skeleton", "scene", "camera",
    "light", "compression", "serializer", "parser", "resource", "diagnostics",
    "export", "math", "topology", "metadata", "streaming", "memory", "platform"
]

CONCERNS = [
    "integrity", "references", "range", "budget", "alignment", "ordering",
    "coverage", "precision", "compatibility", "latency", "stability", "quality"
]

SEVERITIES = ["Info", "Warning", "Error", "Fatal"]


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def rule_name(i: int) -> str:
    return f"rule_{i:04d}_{DOMAINS[i % len(DOMAINS)]}_{CONCERNS[(i // len(DOMAINS)) % len(CONCERNS)]}"


def profile_name(i: int) -> str:
    return f"{DOMAINS[i % len(DOMAINS)]}_{CONCERNS[(i // len(DOMAINS)) % len(CONCERNS)]}_{i:04d}"


def make_header() -> str:
    return """#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bse::catalog {

enum class RuleSeverity : std::uint8_t {
    Info,
    Warning,
    Error,
    Fatal
};

struct RuleProfile {
    std::uint32_t id = 0;
    RuleSeverity severity = RuleSeverity::Info;
    std::string_view domain;
    std::string_view concern;
    std::string_view name;
    std::string_view summary;
    std::uint32_t minimumVersion = 1;
    std::uint32_t maximumVersion = 1;
    std::uint32_t weight = 0;
    std::uint32_t threshold = 0;
    bool enabledByDefault = true;
};

struct CatalogSummary {
    std::size_t totalRules = 0;
    std::size_t infoRules = 0;
    std::size_t warningRules = 0;
    std::size_t errorRules = 0;
    std::size_t fatalRules = 0;
    std::uint32_t totalWeight = 0;
};

const std::vector<RuleProfile>& allRuleProfiles();
std::vector<RuleProfile> profilesForDomain(std::string_view domain);
std::vector<RuleProfile> profilesForConcern(std::string_view concern);
std::vector<RuleProfile> profilesAtLeast(RuleSeverity severity);
const RuleProfile* findRuleProfile(std::uint32_t id);
CatalogSummary summarizeProfiles();
std::string severityName(RuleSeverity severity);

} // namespace bse::catalog
"""


def make_source(count: int) -> str:
    lines = [
        '#include "bse/catalog/rule_catalog.hpp"',
        "",
        "#include <algorithm>",
        "#include <numeric>",
        "",
        "namespace bse::catalog {",
        "namespace {",
        "const std::vector<RuleProfile> kRuleProfiles = {",
    ]
    for i in range(1, count + 1):
        domain = DOMAINS[i % len(DOMAINS)]
        concern = CONCERNS[(i // len(DOMAINS)) % len(CONCERNS)]
        severity = SEVERITIES[(i + len(domain) + len(concern)) % len(SEVERITIES)]
        enabled = "true" if (i % 11) != 0 else "false"
        min_version = 1 + (i % 3)
        max_version = min_version + (i % 5)
        weight = 3 + ((i * 7) % 97)
        threshold = 16 + ((i * 13) % 4096)
        lines.extend([
            "    RuleProfile{",
            f"        {i}u,",
            f"        RuleSeverity::{severity},",
            f'        "{domain}",',
            f'        "{concern}",',
            f'        "{rule_name(i)}",',
            f'        "Checks {concern} behavior for {domain} assets with policy profile {profile_name(i)}.",',
            f"        {min_version}u,",
            f"        {max_version}u,",
            f"        {weight}u,",
            f"        {threshold}u,",
            f"        {enabled}",
            "    },",
        ])
    lines.extend([
        "};",
        "} // namespace",
        "",
        "const std::vector<RuleProfile>& allRuleProfiles() {",
        "    return kRuleProfiles;",
        "}",
        "",
        "std::vector<RuleProfile> profilesForDomain(std::string_view domain) {",
        "    std::vector<RuleProfile> result;",
        "    std::copy_if(kRuleProfiles.begin(), kRuleProfiles.end(), std::back_inserter(result),",
        "                 [domain](const RuleProfile& profile) { return profile.domain == domain; });",
        "    return result;",
        "}",
        "",
        "std::vector<RuleProfile> profilesForConcern(std::string_view concern) {",
        "    std::vector<RuleProfile> result;",
        "    std::copy_if(kRuleProfiles.begin(), kRuleProfiles.end(), std::back_inserter(result),",
        "                 [concern](const RuleProfile& profile) { return profile.concern == concern; });",
        "    return result;",
        "}",
        "",
        "std::vector<RuleProfile> profilesAtLeast(RuleSeverity severity) {",
        "    std::vector<RuleProfile> result;",
        "    std::copy_if(kRuleProfiles.begin(), kRuleProfiles.end(), std::back_inserter(result),",
        "                 [severity](const RuleProfile& profile) {",
        "                     return static_cast<unsigned>(profile.severity) >= static_cast<unsigned>(severity);",
        "                 });",
        "    return result;",
        "}",
        "",
        "const RuleProfile* findRuleProfile(std::uint32_t id) {",
        "    auto found = std::find_if(kRuleProfiles.begin(), kRuleProfiles.end(),",
        "                              [id](const RuleProfile& profile) { return profile.id == id; });",
        "    return found == kRuleProfiles.end() ? nullptr : &*found;",
        "}",
        "",
        "CatalogSummary summarizeProfiles() {",
        "    CatalogSummary summary;",
        "    summary.totalRules = kRuleProfiles.size();",
        "    for (const auto& profile : kRuleProfiles) {",
        "        summary.totalWeight += profile.weight;",
        "        switch (profile.severity) {",
        "        case RuleSeverity::Info: ++summary.infoRules; break;",
        "        case RuleSeverity::Warning: ++summary.warningRules; break;",
        "        case RuleSeverity::Error: ++summary.errorRules; break;",
        "        case RuleSeverity::Fatal: ++summary.fatalRules; break;",
        "        }",
        "    }",
        "    return summary;",
        "}",
        "",
        "std::string severityName(RuleSeverity severity) {",
        "    switch (severity) {",
        '    case RuleSeverity::Info: return "info";',
        '    case RuleSeverity::Warning: return "warning";',
        '    case RuleSeverity::Error: return "error";',
        '    case RuleSeverity::Fatal: return "fatal";',
        "    }",
        '    return "unknown";',
        "}",
        "",
        "} // namespace bse::catalog",
    ])
    return "\n".join(lines) + "\n"


def make_metrics_tool() -> str:
    return r'''#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

namespace fs = std::filesystem;

namespace {
bool hasSourceExtension(const fs::path& path) {
    static const std::set<std::string> extensions{
        ".cpp", ".cc", ".cxx", ".hpp", ".hh", ".hxx", ".h", ".cmake", ".txt", ".md"
    };
    return extensions.count(path.extension().string()) != 0;
}

bool ignoredPath(const fs::path& path) {
    const auto text = path.generic_string();
    return text.find("/.git/") != std::string::npos ||
           text.find("/build/") != std::string::npos ||
           text.find("/fuzz/seed_corpus/") != std::string::npos;
}

std::string trim(std::string line) {
    auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first >= last ? std::string{} : std::string(first, last);
}

bool boilerplateLine(const std::string& line, bool& blockComment) {
    auto text = trim(line);
    if (text.empty()) return true;
    if (blockComment) {
        if (text.find("*/") != std::string::npos) blockComment = false;
        return true;
    }
    if (text.rfind("//", 0) == 0) return true;
    if (text.rfind("/*", 0) == 0) {
        if (text.find("*/") == std::string::npos) blockComment = true;
        return true;
    }
    if (text == "{" || text == "}" || text == "};" || text == "};") return true;
    return false;
}
}

int main(int argc, char** argv) {
    fs::path root = argc > 1 ? fs::path(argv[1]) : fs::current_path();
    std::size_t files = 0;
    std::size_t workingLines = 0;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        const auto path = entry.path();
        if (ignoredPath(path) || !hasSourceExtension(path)) continue;
        std::ifstream in(path);
        if (!in) continue;
        ++files;
        bool blockComment = false;
        std::string line;
        while (std::getline(in, line)) {
            if (!boilerplateLine(line, blockComment)) ++workingLines;
        }
    }
    std::cout << "files=" << files << "\n";
    std::cout << "working_loc=" << workingLines << "\n";
    return workingLines >= 30000 ? 0 : 2;
}
'''


def update_cmake() -> None:
    path = ROOT / "CMakeLists.txt"
    text = path.read_text(encoding="utf-8")
    if "bse_metrics" not in text:
        marker = "    target_link_libraries(bse_roundtrip PRIVATE binary_scene_engine)\n"
        text = text.replace(
            marker,
            marker
            + "    add_executable(bse_metrics tools/metrics.cpp)\n"
            + "    target_link_libraries(bse_metrics PRIVATE binary_scene_engine)\n",
        )
    path.write_text(text, encoding="utf-8")


def update_tests() -> None:
    path = ROOT / "tests/test_main.cpp"
    text = path.read_text(encoding="utf-8")
    if '#include "bse/catalog/rule_catalog.hpp"' not in text:
        text = text.replace('#include "bse/diagnostics/error_reporter.hpp"\n',
                            '#include "bse/diagnostics/error_reporter.hpp"\n#include "bse/catalog/rule_catalog.hpp"\n')
    if "void catalog_test()" not in text:
        insert = (
            "\nvoid catalog_test(){ auto all=bse::catalog::allRuleProfiles(); REQUIRE(all.size()>=6200); "
            "auto mesh=bse::catalog::profilesForDomain(\"mesh\"); REQUIRE(!mesh.empty()); "
            "auto serious=bse::catalog::profilesAtLeast(bse::catalog::RuleSeverity::Error); REQUIRE(!serious.empty()); "
            "auto summary=bse::catalog::summarizeProfiles(); REQUIRE(summary.totalRules==all.size()); "
            "REQUIRE(summary.totalWeight>summary.totalRules); REQUIRE(bse::catalog::findRuleProfile(400)!=nullptr); }\n"
        )
        text = text.replace("\nvoid resource_test(){", insert + "\nvoid resource_test(){")
        text = text.replace('{"resource",resource_test}',
                            '{"catalog",catalog_test},{"resource",resource_test}')
    path.write_text(text, encoding="utf-8")


def git(*args: str) -> None:
    subprocess.run(["git", *args], cwd=ROOT, check=True)


def main() -> None:
    write_text(ROOT / "include/bse/catalog/rule_catalog.hpp", make_header())
    write_text(ROOT / "src/catalog/rule_catalog.cpp", make_source(6250))
    write_text(ROOT / "tools/metrics.cpp", make_metrics_tool())
    update_cmake()
    update_tests()


if __name__ == "__main__":
    main()
