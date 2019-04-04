//
// Created by henrik on 31.03.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "types.hpp"
#include "common/is_instantiation.hpp"
#include "core/memory/memory_handle.hpp"


namespace Core::Memory
{

/// Indicates pointer in n64 memory
template<typename T>
struct be_ptr_t
{
    using Type = T;
    n64_addr_t padding;
};
// Has to be 4 bytes large
static_assert(sizeof(be_ptr_t<void>) == sizeof(n64_addr_t));

// Forward declarations
template<typename, typename>
struct Ptr;

template<typename, typename>
struct AggregateRef;

template<typename, typename>
struct FundamentalRef;

/// Typedef to matching reference class
template<typename T, typename U = Handle>
using Ref = std::conditional_t<std::is_standard_layout_v<T>,
    std::conditional_t<std::is_fundamental_v<T>,
        FundamentalRef<T, U>,
    AggregateRef<T, U>>,
void>;

/// Typedef for const references
template<typename T, typename U = Handle>
using CRef = Ref<const T, U>;

/// Wrapper for aggregate values in n64 memory. Essentially acts like a reference
template<typename TType, typename THandle>
struct AggregateRef
{
    using AddrType = typename THandle::addr_t;
    using RawType = std::remove_cv_t<TType>;
    using QualifiedType = std::remove_volatile_t<TType>;
    using HandleType = THandle;
    static constexpr bool MUTABLE{!std::is_const_v<QualifiedType>};

    template<typename T>
    using Qualified = std::conditional_t<MUTABLE, T, std::add_const_t<T>>;

    /// SFINAE helpers for member accessors
    template<typename T>
    static constexpr bool enable_member_v = std::is_same_v<std::remove_cv_t<T>, RawType>;
    template<typename T, typename U>
    static constexpr bool enable_member_array_v = std::is_same_v<std::remove_cv_t<T>, RawType> && std::extent_v<U> == 0;

    AggregateRef(HandleType hdl, AddrType addr)
    :mem_hdl_{hdl}, addr_{addr}
    {}

    /// Return reference to struct field
    template<typename T, typename TMember,
             typename = std::enable_if_t<enable_member_v<T>>>
    const auto field(TMember (T::*const member)) const
    {
        return make_return_type<TMember>(mem_hdl_, addr_ + offset_u32(member));
    }

    /// Return pointer to struct field
    template<typename T, typename TMember,
             typename = std::enable_if_t<enable_member_v<T>>>
    const auto field_ptr(TMember (T::*const member)) const
    {
        return Ptr<Qualified<TMember>, HandleType>{mem_hdl_, addr_ + offset_u32(member)};
    }

    /// Return reference to struct field with array subscription
    template<typename T, typename TMember,
             typename = std::enable_if_t<enable_member_array_v<T, TMember>>>
    const auto field(TMember (T::*const member), AddrType index) const
    {
        // Bounds checking
        assert(index * sizeof(std::remove_extent_t<TMember>) < sizeof(TMember));

        return make_return_type<Qualified<std::remove_extent_t<TMember>>>(
            mem_hdl_, addr_ + offset_u32(member) + index * (AddrType)sizeof(std::remove_extent_t<TMember>)
        );
    }

    /// Return pointer to struct field with array subscription
    template<typename T, typename TMember,
             typename = std::enable_if_t<enable_member_array_v<T, TMember>>>
    const auto field_ptr(TMember (T::*const member), AddrType index) const
    {
        // Bounds checking
        assert(index * sizeof(std::remove_extent_t<TMember>) < sizeof(TMember));

        return AggregateRef<Qualified<std::remove_extent_t<TMember>>, HandleType>{
            mem_hdl_, addr_ + offset_u32(member) + index * (AddrType)sizeof(std::remove_extent_t<TMember>)
        };
    }

private:
    void write(const TType& val) const
    {
        mem_hdl_.template write<RawType>(addr_, val);
    }

    RawType read() const
    {
        return mem_hdl_.template read<RawType>(addr_);
    }

    /// Helper function to collapse references to pointers to just pointers
    template<typename T>
    const auto make_return_type(HandleType hdl, AddrType addr) const
    {
        if constexpr(Common::is_instantiation_of_v<be_ptr_t, T>)
        {

            return Ptr<typename T::Type, HandleType>{hdl, mem_hdl_.template read<AddrType>(addr)};
        }
        else
        {
            return Ref<Qualified<T>, HandleType>{hdl, addr};
        }
    }

    mutable HandleType mem_hdl_;
    AddrType addr_{};
};

/// Wrapper for fundamental values in n64 memory. Essentially acts like a reference
template<typename TType, typename THandle>
struct FundamentalRef
{
    using AddrType = typename THandle::addr_t;
    using RawType = std::remove_cv_t<TType>;
    using QualifiedType = std::remove_volatile_t<TType>;
    using HandleType = THandle;
    static constexpr bool MUTABLE{!std::is_const_v<QualifiedType>};

    FundamentalRef(HandleType hdl, AddrType addr)
    :mem_hdl_{hdl}, addr_{addr}
    {}

    /// Conversion operator to const reference
    operator FundamentalRef<const RawType, HandleType>() const
    {
        return {mem_hdl_, addr_};
    }

    /// Conversion operator to underlying type
    operator RawType() const
    {
        return read();
    }

#define MUTABLE_ONLY_ template<typename = std::enable_if_t<MUTABLE>>

    /// Assignment operator for underlying type
    MUTABLE_ONLY_
    RawType operator=(const RawType& other) const
    {
        write(other);
        return other;
    }

    // Arithmetic operators
    
    RawType operator+(const RawType& other) const
    {
        return read() + other;
    }

    RawType operator-(const RawType& other) const
    {
        return read() - other;
    }

    RawType operator*(const RawType& other) const
    {
        return read() * other;
    }

    RawType operator/(const RawType& other) const
    {
        return read() / other;
    }

    RawType operator%(const RawType& other) const
    {
        return read() % other;
    }

    RawType operator&(const RawType& other) const
    {
        return read() & other;
    }

    RawType operator|(const RawType& other) const
    {
        return read() | other;
    }

    RawType operator^(const RawType& other) const
    {
        return read() ^ other;
    }

    MUTABLE_ONLY_
    RawType operator+=(const RawType& other) const
    {
        RawType val{read() + other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator-=(const RawType& other) const
    {
        RawType val{read() - other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator*=(const RawType& other) const
    {
        RawType val{read() * other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator/=(const RawType& other) const
    {
        RawType val{read() / other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator%=(const RawType& other) const
    {
        RawType val{read() % other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator&=(const RawType& other) const
    {
        RawType val{read() & other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator|=(const RawType& other) const
    {
        RawType val{read() | other};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator^=(const RawType& other) const
    {
        RawType val{read() ^ other};
        write(val);
        return val;
    }
    
    RawType operator~() const
    {
        return ~read();
    }

    RawType operator<<(std::size_t pos) const
    {
        return (read() << pos);
    }

    RawType operator>>(std::size_t pos) const
    {
        return (read() >> pos);
    }

    
    RawType operator<<=(std::size_t pos) const
    {
        RawType val{read() << pos};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator>>=(std::size_t pos) const
    {
        RawType val{read() >> pos};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator++() const
    {
        RawType val{read() + (RawType)1};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator++(int) const
    {
        RawType old_val{read()},
                new_val{old_val + (RawType)1};
        write(new_val);
        return old_val;
    }

    MUTABLE_ONLY_
    RawType operator--() const
    {
        RawType val{read() - (RawType)1};
        write(val);
        return val;
    }

    MUTABLE_ONLY_
    RawType operator--(int) const
    {
        RawType old_val{read()},
                new_val{old_val - (RawType)1};
        write(new_val);
        return old_val;
    }

    /// Forward comparison operators
    bool operator==(const RawType& other) const
    {
        return read() == other;
    }

    bool operator!=(const RawType& other) const
    {
        return !(*this == other);
    }

    bool operator<(const RawType& other) const
    {
        return read() < other;
    }

    bool operator<=(const RawType& other) const
    {
        return !(*this > other);
    }

    bool operator>(const RawType& other) const
    {
        return read() > other;
    }

    bool operator>=(const RawType& other) const
    {
        return !(*this < other);
    }

#undef MUTABLE_ONLY_

private:
    void write(const TType& val) const
    {
        mem_hdl_.template write<RawType>(addr_, val);
    }

    RawType read() const
    {
        return mem_hdl_.template read<RawType>(addr_);
    }

    mutable HandleType mem_hdl_;
    AddrType addr_{};
};

} // Core::Memory
