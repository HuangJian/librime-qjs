#pragma once

#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/ticket.h>

#include <glog/logging.h>
#include <map>
#include <memory>
#include <typeinfo>
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

  [[nodiscard]] Environment& environment() const { return *environment_; }

  ComponentWrapperBase(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase& operator=(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase(ComponentWrapperBase&&) = delete;
  ComponentWrapperBase& operator=(ComponentWrapperBase&&) = delete;

protected:
  explicit ComponentWrapperBase(const Ticket& ticket)
      : T_BASE(ticket),
        environment_(std::make_unique<Environment>(*ticket.engine, ticket.name_space)) {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name()
               << " ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapperBase() {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name() << " ComponentWrapper destroyed";
  }

private:
  std::unique_ptr<Environment> environment_;
  std::shared_ptr<T_ACTUAL> actual_{nullptr};
};

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class QuickJSComponent : public T_BASE::Component {
  using KeyType = std::pair<std::string, std::string>;

public:
  // NOLINTNEXTLINE(readability-identifier-naming)
  ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>* Create(const Ticket& ticket) override {
    // The same plugin could have difference configurations for different schemas, and then behave differently.
    // So we need to create a new component for each schema.
    const std::string schemaId = ticket.engine->schema()->schema_id();
    KeyType key = std::make_pair(schemaId, ticket.name_space);

    auto component = new ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>(ticket);
    std::shared_ptr<T_ACTUAL> actual = nullptr;
    if (components_.count(key)) {
      actual = components_[key];
    } else {
      LOG(INFO) << "[qjs] creating component '" << ticket.name_space << "' for schema " << schemaId;
      actual = std::make_shared<T_ACTUAL>(ticket, component->environment());
      components_[key] = actual;
    }

    component->setActual(actual);
    return component;
  }

private:
  std::map<KeyType, std::shared_ptr<T_ACTUAL>> components_;
};

}  // namespace rime
