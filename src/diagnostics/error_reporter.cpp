#include "bse/diagnostics/error_reporter.hpp"
#include <sstream>
namespace bse::diagnostics {
std::string describeErrorCode(ErrorCode c){ switch(c){case ErrorCode::None:return"none";case ErrorCode::IoError:return"io";case ErrorCode::TruncatedInput:return"truncated";case ErrorCode::InvalidMagic:return"invalid magic";case ErrorCode::UnsupportedVersion:return"unsupported version";case ErrorCode::InvalidSectionTable:return"section table";case ErrorCode::InvalidOffset:return"offset";case ErrorCode::InvalidObject:return"object";case ErrorCode::MissingReference:return"reference";case ErrorCode::CompressionError:return"compression";case ErrorCode::ValidationError:return"validation";case ErrorCode::InternalError:return"internal";} return"unknown"; }
std::string summarizeErrors(const ErrorList& e){ std::ostringstream o; o<<e.size()<<" error(s)"; for(auto& x:e.entries()) o<<"\n- "<<describeErrorCode(x.code)<<": "<<x.message; return o.str(); }
}
