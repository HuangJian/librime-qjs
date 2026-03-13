#pragma once

#include <exception>
#include <string>
#include <utility>

enum class JsErrorType : std::uint8_t {
  SYNTAX,
  RANGE,
  REFERENCE,
  TYPE,
  EVAL,
  GENERIC,
  INTERNAL,
  UNKNOWN,
};

class JsException final : public std::exception {
private:
  std::string message_;
  JsErrorType type_;

public:
  // Constructor that takes an error message
  explicit JsException(const JsErrorType type, std::string message)
      : message_(std::move(message)), type_(type) {}

  // Override what() method from std::exception
  [[nodiscard]] const char* what() const noexcept override { return message_.c_str(); }
  [[nodiscard]] JsErrorType getType() const noexcept { return type_; }
};
