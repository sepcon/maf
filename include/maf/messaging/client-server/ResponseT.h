#pragma once

#include "CSError.h"
#include <variant>

namespace maf {
namespace messaging {

template <class Output> class ResponseT {
  struct NoValue {};

public:
  using OutputPtr = std::shared_ptr<Output>;
  using ErrorPtr = std::shared_ptr<CSError>;

  template <typename T,
            std::enable_if_t<std::is_same_v<Output, T>, bool> = true>
  ResponseT(std::shared_ptr<T> response = {}) noexcept
      : _response(std::move(response)) {}

  ResponseT(std::shared_ptr<CSError> error) noexcept
      : _response(std::move(error)) {}

  ResponseT() noexcept : _response{NoValue{}} {}
  //  ResponseT(const ResponseT &) = default;
  //  ResponseT(ResponseT &&) = default;
  //  ResponseT &operator=(ResponseT &&) = default;

  OutputPtr getOutput() const noexcept {
    return isOutput() ? std::get<OutputPtr>(_response) : nullptr;
  }

  ErrorPtr getError() const noexcept {
    return isError() ? std::get<ErrorPtr>(_response) : nullptr;
  }

  bool isError() const noexcept { return has<ErrorPtr>(); }
  bool isOutput() const noexcept { return has<OutputPtr>(); }
  bool hasValue() const noexcept { return !has<NoValue>(); }

private:
  // clang-format off
  template<class T>
  bool has() const noexcept { return std::holds_alternative<T>(_response); }

  std::variant<
    std::shared_ptr<Output>,
    std::shared_ptr<CSError>,
    NoValue
  > _response;

  // clang-format on
};

} // namespace messaging
} // namespace maf
