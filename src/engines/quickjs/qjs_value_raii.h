#pragma once

#include <quickjs.h>

class QjsValueRAII {
public:
  QjsValueRAII() : ctx_(nullptr), val_(JS_UNDEFINED) {}
  explicit QjsValueRAII(JSContext* ctx, JSValue val) : ctx_(ctx), val_(val) {}
  ~QjsValueRAII() {
    if (ctx_) {
      JS_FreeValue(ctx_, val_);
    }
  }

  // Delete copy constructor and assignment operator
  QjsValueRAII(const QjsValueRAII&) = delete;
  QjsValueRAII& operator=(const QjsValueRAII&) = delete;

  // Move constructor
  QjsValueRAII(QjsValueRAII&& other) noexcept : ctx_(other.ctx_), val_(other.val_) {
    other.val_ = JS_UNDEFINED;
    other.ctx_ = nullptr;
  }

  // Move assignment
  QjsValueRAII& operator=(QjsValueRAII&& other) noexcept {
    if (this != &other) {
      if (ctx_) {
        JS_FreeValue(ctx_, val_);
      }
      ctx_ = other.ctx_;
      val_ = other.val_;
      other.val_ = JS_UNDEFINED;
      other.ctx_ = nullptr;
    }
    return *this;
  }

  operator JSValue() const { return val_; }

  bool operator!() const { return JS_IsException(val_) || JS_IsUndefined(val_) || JS_IsNull(val_); }

  JSValue release() {
    JSValue val = val_;
    val_ = JS_UNDEFINED;
    return val;
  }

  [[nodiscard]] JSValue get() const { return val_; }

private:
  JSContext* ctx_;
  JSValue val_;
};

class QjsCStringRAII {
public:
  explicit QjsCStringRAII(JSContext* ctx, const char* ptr) : ctx_(ctx), ptr_(ptr) {}
  ~QjsCStringRAII() {
    if (ctx_) {
      JS_FreeCString(ctx_, ptr_);
    }
  }

  QjsCStringRAII(const QjsCStringRAII&) = delete;
  QjsCStringRAII& operator=(const QjsCStringRAII&) = delete;

  QjsCStringRAII(QjsCStringRAII&& other) noexcept : ctx_(other.ctx_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.ctx_ = nullptr;
  }

  QjsCStringRAII& operator=(QjsCStringRAII&& other) noexcept {
    if (this != &other) {
      if (ctx_) {
        JS_FreeCString(ctx_, ptr_);
      }
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
  JSContext* ctx_;
  const char* ptr_;
};
