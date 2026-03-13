#pragma once

#include <rime/common.h>
#include <rime/component.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/ticket.h>
#include <rime/translator.h>

#include <map>
#include <memory>
#include <utility>
#include "types/environment.h"

namespace rime {

// Primary template declaration
template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class ComponentWrapper;

// Base class for all ComponentWrapper specializations
template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class ComponentWrapperBase : public T_BASE {
public:
  std::shared_ptr<T_ACTUAL> actual() { return actual_; }
  void setActual(const std::shared_ptr<T_ACTUAL> actual) { actual_ = actual; }

  [[nodiscard]] Environment* environment() const { return environment_.get(); }

  ComponentWrapperBase(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase& operator=(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase(ComponentWrapperBase&&) = delete;
  ComponentWrapperBase& operator=(ComponentWrapperBase&&) = delete;

protected:
  explicit ComponentWrapperBase(const rime::Ticket& ticket);
  virtual ~ComponentWrapperBase();

private:
  std::unique_ptr<Environment> environment_;
  std::shared_ptr<T_ACTUAL> actual_{nullptr};
};

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class QuickJSComponent : public T_BASE::Component {
  using KeyType = std::pair<std::string, std::string>;

public:
  // NOLINTNEXTLINE(readability-identifier-naming)
  ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>* Create(const rime::Ticket& ticket);

private:
  std::map<KeyType, std::shared_ptr<T_ACTUAL>> components_;
};

}  // namespace rime
