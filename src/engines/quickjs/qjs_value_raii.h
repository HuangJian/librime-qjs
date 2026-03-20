#pragma once

#include <quickjs.h>
#include <memory>

class QjsValueRAII {
public:
  QjsValueRAII() : ctx_(nullptr), val_(JS_UNDEFINED) {}
  explicit QjsValueRAII(JSContext* ctx, JSValue val, std::shared_ptr<void> engine = nullptr)
      : engine_(std::move(engine)), ctx_(ctx), val_(val) {}
  ~QjsValueRAII() {
    if (ctx_) {
      JS_FreeValue(ctx_, val_);
    }
  }

  // Delete copy constructor and assignment operator
  QjsValueRAII(const QjsValueRAII&) = delete;
  QjsValueRAII& operator=(const QjsValueRAII&) = delete;

  // Move constructor
  QjsValueRAII(QjsValueRAII&& other) noexcept
      : engine_(std::move(other.engine_)), ctx_(other.ctx_), val_(other.val_) {
    other.val_ = JS_UNDEFINED;
    other.ctx_ = nullptr;
  }

  // Move assignment
  QjsValueRAII& operator=(QjsValueRAII&& other) noexcept {
    if (this != &other) {
      if (ctx_) {
        JS_FreeValue(ctx_, val_);
      }
      engine_ = std::move(other.engine_);
      ctx_ = other.ctx_;
      val_ = other.val_;
      other.val_ = JS_UNDEFINED;
      other.ctx_ = nullptr;
    }
    return *this;
  }

  operator JSValue() const& { return val_; }
  operator JSValue() && { return release(); }

  bool operator!() const { return JS_IsException(val_) || JS_IsUndefined(val_) || JS_IsNull(val_); }

  JSValue release() {
    JSValue val = val_;
    val_ = JS_UNDEFINED;
    return val;
  }

  JSValue release() const {
    JSValue val = val_;
    const_cast<QjsValueRAII*>(this)->val_ = JS_UNDEFINED;
    return val;
  }

  [[nodiscard]] JSValue get() const { return val_; }

private:
  std::shared_ptr<void> engine_;
  JSContext* ctx_;
  JSValue val_;
};

class QjsCStringRAII {
public:
  explicit QjsCStringRAII(JSContext* ctx, const char* ptr, std::shared_ptr<void> engine = nullptr)
      : engine_(std::move(engine)), ctx_(ctx), ptr_(ptr) {}
  ~QjsCStringRAII() {
    if (ctx_) {
      JS_FreeCString(ctx_, ptr_);
    }
  }

  QjsCStringRAII(const QjsCStringRAII&) = delete;
  QjsCStringRAII& operator=(const QjsCStringRAII&) = delete;

  QjsCStringRAII(QjsCStringRAII&& other) noexcept
      : engine_(std::move(other.engine_)), ctx_(other.ctx_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.ctx_ = nullptr;
  }

  QjsCStringRAII& operator=(QjsCStringRAII&& other) noexcept {
    if (this != &other) {
      if (ctx_) {
        JS_FreeCString(ctx_, ptr_);
      }
      engine_ = std::move(other.engine_);
      ctx_ = other.ctx_;
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
      other.ctx_ = nullptr;
    }
    return *this;
  }

  operator const char*() const { return ptr_; }
  [[nodiscard]] const char* get() const { return ptr_; }

private:
  std::shared_ptr<void> engine_;
  JSContext* ctx_;
  const char* ptr_;
};
