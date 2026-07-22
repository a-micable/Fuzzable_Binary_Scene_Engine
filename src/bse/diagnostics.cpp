#include "bse/diagnostics.hpp"

#include <algorithm>
#include <utility>

namespace bse {

void DiagnosticSink::Info(std::string code, std::string message) {
  diagnostics_.push_back({DiagnosticSeverity::kInfo, std::move(code), std::move(message)});
}

void DiagnosticSink::Warning(std::string code, std::string message) {
  diagnostics_.push_back({DiagnosticSeverity::kWarning, std::move(code), std::move(message)});
}

void DiagnosticSink::Error(std::string code, std::string message) {
  diagnostics_.push_back({DiagnosticSeverity::kError, std::move(code), std::move(message)});
}

bool DiagnosticSink::has_errors() const {
  return std::any_of(diagnostics_.begin(), diagnostics_.end(), [](const Diagnostic& diagnostic) {
    return diagnostic.severity == DiagnosticSeverity::kError;
  });
}

}  // namespace bse
