#include "bse/catalog/rule_catalog.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace bse::catalog {
namespace {

constexpr std::size_t kProfileCount = 6250;

constexpr std::array<std::string_view, 20> kDomains = {
    "material", "texture", "animation", "skeleton", "scene", "camera",
    "light", "compression", "serializer", "parser", "resource", "diagnostics",
    "mesh", "export", "platform", "memory", "streaming", "math", "topology",
    "metadata"
};

constexpr std::array<std::string_view, 12> kConcerns = {
    "integrity", "quality", "compatibility", "stability", "ordering", "alignment",
    "coverage", "latency", "precision", "range", "references", "budget"
};

struct CatalogData {
    std::vector<std::string> text;
    std::vector<RuleProfile> profiles;

    CatalogData() {
        text.reserve(kProfileCount * 2);
        profiles.reserve(kProfileCount);
        for (std::uint32_t id = 1; id <= kProfileCount; ++id) {
            const auto domain = kDomains[(id - 1) % kDomains.size()];
            const auto concern = kConcerns[((id - 1) / kDomains.size()) % kConcerns.size()];
            const auto severity = static_cast<RuleSeverity>((id * 17u + id / 7u) % 4u);
            const auto minimumVersion = 1u + (id % 3u);
            const auto maximumVersion = minimumVersion + 1u + (id % 4u);
            const auto weight = 3u + ((id * 7u) % 97u);
            const auto threshold = 20u + ((id * 13u) % 3440u);

            auto number = std::to_string(id);
            number.insert(number.begin(), 4u - std::min<std::size_t>(4u, number.size()), '0');
            text.push_back("rule_" + number + "_" + std::string(domain) + "_" + std::string(concern));
            text.push_back("Checks " + std::string(concern) + " behavior for " +
                           std::string(domain) + " assets using the catalog policy profile.");
            const auto name = std::string_view{text[text.size() - 2]};
            const auto summary = std::string_view{text[text.size() - 1]};
            profiles.push_back({id, severity, domain, concern, name, summary,
                                minimumVersion, maximumVersion, weight, threshold,
                                (id % 12u) != 0u});
        }
    }
};

const CatalogData& catalogData() {
    static const CatalogData data;
    return data;
}

} // namespace

const std::vector<RuleProfile>& allRuleProfiles() {
    return catalogData().profiles;
}

std::vector<RuleProfile> profilesForDomain(std::string_view domain) {
    std::vector<RuleProfile> result;
    const auto& profiles = allRuleProfiles();
    std::copy_if(profiles.begin(), profiles.end(), std::back_inserter(result),
                 [domain](const RuleProfile& profile) { return profile.domain == domain; });
    return result;
}

std::vector<RuleProfile> profilesForConcern(std::string_view concern) {
    std::vector<RuleProfile> result;
    const auto& profiles = allRuleProfiles();
    std::copy_if(profiles.begin(), profiles.end(), std::back_inserter(result),
                 [concern](const RuleProfile& profile) { return profile.concern == concern; });
    return result;
}

std::vector<RuleProfile> profilesAtLeast(RuleSeverity severity) {
    std::vector<RuleProfile> result;
    const auto& profiles = allRuleProfiles();
    std::copy_if(profiles.begin(), profiles.end(), std::back_inserter(result),
                 [severity](const RuleProfile& profile) {
                     return static_cast<unsigned>(profile.severity) >=
                            static_cast<unsigned>(severity);
                 });
    return result;
}

const RuleProfile* findRuleProfile(std::uint32_t id) {
    const auto& profiles = allRuleProfiles();
    const auto found = std::lower_bound(profiles.begin(), profiles.end(), id,
                                        [](const RuleProfile& profile, std::uint32_t value) {
                                            return profile.id < value;
                                        });
    return found == profiles.end() || found->id != id ? nullptr : &*found;
}

CatalogSummary summarizeProfiles() {
    CatalogSummary summary;
    for (const auto& profile : allRuleProfiles()) {
        ++summary.totalRules;
        summary.totalWeight += profile.weight;
        switch (profile.severity) {
        case RuleSeverity::Info: ++summary.infoRules; break;
        case RuleSeverity::Warning: ++summary.warningRules; break;
        case RuleSeverity::Error: ++summary.errorRules; break;
        case RuleSeverity::Fatal: ++summary.fatalRules; break;
        }
    }
    return summary;
}

std::string severityName(RuleSeverity severity) {
    switch (severity) {
    case RuleSeverity::Info: return "info";
    case RuleSeverity::Warning: return "warning";
    case RuleSeverity::Error: return "error";
    case RuleSeverity::Fatal: return "fatal";
    }
    return "unknown";
}

} // namespace bse::catalog