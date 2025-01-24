#pragma once

#include <cstdint>

namespace cista
{

template<typename T>
struct basic_external_ptr
{
    basic_external_ptr() : el_(0) {}
    basic_external_ptr(T* el) : el_(reinterpret_cast<uintptr_t>(el)) {}

    basic_external_ptr(const basic_external_ptr& other) = default;
    basic_external_ptr(basic_external_ptr&& other) noexcept = default;

    basic_external_ptr& operator=(const basic_external_ptr<T>& other) = default;
    basic_external_ptr& operator=(basic_external_ptr&& other) noexcept = default;

    explicit operator bool() const noexcept { return el_ != 0; }

    friend bool operator==(const basic_external_ptr& a, const basic_external_ptr& b) noexcept { return a.el_ == b.el_; }
    friend bool operator==(const basic_external_ptr& a, std::nullptr_t) noexcept { return a.el_ == 0; }
    friend bool operator==(std::nullptr_t, const basic_external_ptr& a) noexcept { return a.el_ == 0; }
    friend bool operator!=(const basic_external_ptr& a, const basic_external_ptr& b) noexcept { return !(a == b); }
    friend bool operator!=(const basic_external_ptr& a, std::nullptr_t) noexcept { return a.el_ != 0; }
    friend bool operator!=(std::nullptr_t, const basic_external_ptr& a) noexcept { return a.el_ != 0; }

    T* get() noexcept { return reinterpret_cast<T*>(el_); }
    const T* get() const noexcept { return reinterpret_cast<const T*>(el_); }
    T* operator->() noexcept { return reinterpret_cast<T*>(el_); }
    T& operator*() noexcept { return *reinterpret_cast<T*>(el_); }
    T const& operator*() const noexcept { return *reinterpret_cast<const T*>(el_); }
    T const* operator->() const noexcept { return reinterpret_cast<const T*>(el_); }

    uintptr_t el_;
};

namespace raw
{

template<typename T>
using external_ptr = cista::basic_external_ptr<T>;

}  // namespace raw

namespace offset
{

template<typename T>
using external_ptr = cista::basic_external_ptr<T>;

}  // namespace offset

}  // namespace cista
