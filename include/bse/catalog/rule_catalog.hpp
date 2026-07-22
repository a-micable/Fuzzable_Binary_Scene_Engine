#pragma once
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
