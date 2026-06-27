#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::diagnostics {
std::string summarizeErrors(const ErrorList& errors);
std::string describeErrorCode(ErrorCode code);
}
