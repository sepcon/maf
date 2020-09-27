#pragma once

#include <variant>

#include "CSError.h"

namespace maf {
namespace messaging {

template <class Output>
class ResponseT {
  struct NoValue {};

 public:
  using OutputPtr = std::shared_ptr<Output>;
  using ErrorPtr = std::shared_ptr<CSError>;

  ResponseT(std::shared_ptr<Output> response) noexcept
      : response_(std::move(response)) {}

  ResponseT(std::shared_ptr<CSError> error) noexcept
      : response_(std::move(error)) {}

  ResponseT() noexcept : response_{NoValue{}} {}
  ResponseT(const ResponseT &) = default;
  ResponseT(ResponseT &&) = default;
  ResponseT &operator=(ResponseT &&) = default;
  ResponseT &operator=(const ResponseT &) = default;

  ~ResponseT() {}  // Avoid VC-C2644 warning

  OutputPtr getOutput() const noexcept {
    return isOutput() ? std::get<OutputPtr>(response_) : nullptr;
  }

  ErrorPtr getError() const noexcept {
    return isError() ? std::get<ErrorPtr>(response_) : nullptr;
  }

  bool isError() const noexcept { return has<ErrorPtr>(); }
  bool isOutput() const noexcept { return has<OutputPtr>(); }
  bool hasValue() const noexcept { return !has<NoValue>(); }

 private:
  // clang-format off
  template<class T>
  bool has() const noexcept { return std::holds_alternative<T>(response_); }

  std::variant<
    std::shared_ptr<Output>,
    std::shared_ptr<CSError>,
    NoValue
  > response_;

  // clang-format on
};

}  // namespace messaging
}  // namespace maf
