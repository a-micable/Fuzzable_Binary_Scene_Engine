#pragma once

#include <string>
#include <vector>

namespace bse {

enum class DiagnosticSeverity {
  kInfo,
  kWarning,
  kError
};

struct Diagnostic {
  DiagnosticSeverity severity = DiagnosticSeverity::kInfo;
  std::string code;
  std::string message;
};

class DiagnosticSink {
 public:
  void Info(std::string code, std::string message);
  void Warning(std::string code, std::string message);
  void Error(std::string code, std::string message);

  bool has_errors() const;
  const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

 private:
  std::vector<Diagnostic> diagnostics_;
};

}  // namespace bse
