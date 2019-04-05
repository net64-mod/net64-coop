//
// Created by henrik on 30.03.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "types.hpp"
#include "common/operator_proxy.hpp"
#include "core/memory/mem_ref.hpp"


namespace Core::Memory
{

/**
 * Base class for fancy pointer pointing to a value in n64 memory
 *
 * @tparam TType   Type of pointed value
 * @tparam TSIZE   Size of the pointed value in n64 memory
 * @tparam THandle Class for accessing memory
 */
template<typename TType, std::size_t TSIZE, typename THandle>
struct PtrBase
{
    using AddrType = typename THandle::addr_t;
    using SAddrType = typename THandle::saddr_t;
    using USizeType = typename THandle::usize_t;
    using SSizeType = typename THandle::ssize_t;
    static constexpr USizeType SIZE{TSIZE};
    using RawType = std::remove_cv_t<TType>;
    using QualifiedType = std::remove_volatile_t<TType>;
    using HandleType = THandle;

    /// Comparison SFINAE helper
    template<typename T>
    static constexpr bool convertible_v = std::is_convertible_v<QualifiedType*, T>;

    /// Set contained memory handle
    void set_hdl(const HandleType& hdl)
    {
        mem_hdl_ = hdl;
    }

    /// Set referenced address
    void set_offset(AddrType addr)
    {
        addr_ = addr;
    }

    /// Invalidate pointer (make it a NULL pointer)
    void invalidate()
    {
        addr_ = HandleType::INVALID_OFFSET;
    }

    /// Return constant reference to memory handle
    const HandleType& hdl() const
    {
        return mem_hdl_;
    }

    /// Get the referenced address
    AddrType offset() const
    {
        return addr_;
    }

    /// Check if pointer points to valid offset
    bool valid() const
    {
        return HandleType::valid_offset(addr_);
    }

    /// Equal to Ptr<T>::valid()
    operator bool() const
    {
        return valid();
    }

    /// Equal to !Ptr<T>::valid()
    bool operator!() const
    {
        return !valid();
    }

#define TEMPLATE_ template<template<typename, typename>typename TPointer, typename T,\
                  typename = std::enable_if_t<convertible_v<T*>>>

    /// Allow standard pointer conversions
    TEMPLATE_
    operator TPointer<T, HandleType>() const
    {
        return {mem_hdl_, addr_};
    }

    // Pointer comparison operators

    TEMPLATE_
    bool operator==(const TPointer<T, HandleType>& other) const
    {
        return (mem_hdl_ == other.hdl() && addr_ == other.offset());
    }

    TEMPLATE_
    bool operator!=(const TPointer<T, HandleType>& other) const
    {
        return !(*this == other);
    }

#undef TEMPLATE_

    // Pointer arithmetic operators

    PtrBase operator+(SSizeType n) const
    {
        return {mem_hdl_, addr_ + n};
    }

    PtrBase operator-(SSizeType n) const
    {
        return {mem_hdl_, addr_ - n};
    }

    PtrBase operator+=(SSizeType n)
    {
        addr_ += n;
        return *this;
    }

    PtrBase operator-=(SSizeType n)
    {
        addr_ -= n;
        return *this;
    }

    PtrBase& operator++()
    {
        addr_ += SIZE;
        return *this;
    }

    const PtrBase operator++(int)
    {
        auto old{*this};
        *this++;
        return old;
    }

    PtrBase& operator--()
    {
        addr_ -= SIZE;
        return *this;
    }

    const PtrBase operator--(int)
    {
        auto old{*this};
        *this--;
        return old;
    }

protected:
    explicit PtrBase(HandleType hdl)
    :mem_hdl_{hdl}
    {}

    PtrBase(HandleType hdl, AddrType addr)
    :mem_hdl_{hdl}, addr_{addr}
    {}

    mutable HandleType mem_hdl_;
    AddrType addr_{HandleType::INVALID_OFFSET};
};

/// Fancy n64 pointer for non pointer types (int, float, structs)
template<typename TType, typename THandle = Handle>
struct Ptr : PtrBase<TType, sizeof(TType), THandle>
{
    using Base = PtrBase<TType, sizeof(TType), THandle>;
    using HandleType = THandle;
    using AddrType = typename Base::AddrType;
    using USizeType = typename THandle::usize_t;
    using QualifiedType = typename Base::QualifiedType;

    explicit Ptr(HandleType hdl)
    :Base(hdl)
    {}

    Ptr(HandleType hdl, AddrType addr)
    :Base(hdl, addr)
    {}

    /// Return reference wrapper to address
    Ref<QualifiedType, HandleType> operator*() const
    {
        return {this->mem_hdl_, this->addr_};
    }

    /// Return reference wrapper to array field
    Ref<QualifiedType, HandleType> operator[](USizeType i) const
    {
        return {this->mem_hdl_, this->addr_ + Base::SIZE * i};
    }

    /// Call reference function
    Common::OperatorProxy<Ref<QualifiedType, HandleType>>
    operator->() const
    {
        return Ref<QualifiedType, HandleType>{this->mem_hdl_, this->addr_};
    }
};

/// Fancy pointer specialization for pointer types (int*, float**)
template<typename TType, typename THandle>
struct Ptr<N64Ptr<TType>, THandle> : PtrBase<TType, sizeof(typename THandle::addr_t), THandle>
{
    using Base = PtrBase<TType, sizeof(typename THandle::addr_t), THandle>;
    using HandleType = THandle;
    using AddrType = typename Base::AddrType;
    using USizeType = typename THandle::usize_t;
    using QualifiedType = typename Base::QualifiedType;

    explicit Ptr(HandleType hdl)
    :Base(hdl)
    {}

    Ptr(HandleType hdl, AddrType addr)
    :Base(hdl, addr)
    {}

    /// Return pointer to the address stored in the current address
    const Ptr<QualifiedType, HandleType> operator*() const
    {
        return {this->mem_hdl_, this->mem_hdl_.template read<AddrType>(this->addr_)};
    }

    /// Return pointer to the address stored in the current address + index
    const Ptr<QualifiedType, HandleType> operator[](USizeType i) const
    {
        return {this->mem_hdl_, this->mem_hdl_.template read<AddrType>(this->addr_ + Base::SIZE * i)};
    }

    /// Call nested pointer function
    Common::OperatorProxy<const Ptr<QualifiedType, HandleType>>
    operator->() const
    {
        return {**this};
    }
};

/// Specialization for constant pointers. Same as above
template<typename TType, typename THandle>
struct Ptr<N64Ptr<const TType>, THandle> : PtrBase<TType, sizeof(typename THandle::addr_t), THandle>
{
    using Base = PtrBase<TType, sizeof(typename THandle::addr_t), THandle>;
    using HandleType = THandle;
    using AddrType = typename Base::AddrType;
    using USizeType = typename Base::USizeType;
    using QualifiedType = typename Base::QualifiedType;

    explicit Ptr(HandleType hdl)
    :Base(hdl)
    {}

    Ptr(HandleType hdl, AddrType addr)
    :Base(hdl, addr)
    {}

    /// Return pointer to the address stored in the current address
    const Ptr<QualifiedType, HandleType> operator*() const
    {
        return {this->mem_hdl_, this->mem_hdl_.template read<AddrType>(this->addr_)};
    }

    /// Return pointer to the address stored in the current address + index
    const Ptr<QualifiedType, HandleType> operator[](USizeType i) const
    {
        return {this->mem_hdl_, this->mem_hdl_.template read<AddrType>(this->addr_ + Base::SIZE * i)};
    }

    /// Call nested pointer function
    Common::OperatorProxy<const Ptr<QualifiedType, HandleType>>
    operator->() const
    {
        return {**this};
    }
};

/// Pointer to const typedef
template<typename T, typename U>
using CPtr = Ptr<const T, U>;

} // Core::Memory
