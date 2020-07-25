
//
// Created by henrik on 16.05.19.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>


template<typename TBase, typename TKey, typename TPtr = std::unique_ptr<TBase>, typename... TArgs>
struct Factory
{
    using FactoryType = TBase* (*)(TArgs&&... args);

    friend TBase;

protected:
    Factory() = default;

    struct Shared
    {
        template<typename, auto, bool>
        friend struct RegisterConst;
        template<typename, bool>
        friend struct RegisterDyn;
        friend Factory;

    private:
        static std::unordered_map<TKey, FactoryType>& get_factories()
        {
            static std::unordered_map<TKey, FactoryType> s_factories;
            return s_factories;
        }
    };

    static const std::unordered_map<TKey, FactoryType>& get_factories() { return Shared::get_factories(); }

protected:
    using Identifier = TKey;

    template<typename T, T>
    struct DummyUser
    {
    };

public:
    template<typename, auto, bool>
    friend struct RegisterConst;
    template<typename, bool>
    friend struct RegisterDyn;

    [[nodiscard]] static TPtr make(const TKey& key, TArgs&&... args)
    {
        static_assert(std::is_base_of_v<Factory<TBase, TKey, TPtr, TArgs...>, TBase>,
                      "Trying to instantiate derived class of non-registry");
        return TPtr{Shared::get_factories().at(key)(std::forward<TArgs>(args)...)};
    }

    [[nodiscard]] static TPtr make(std::string_view key, TArgs&&... args)
    {
        return make(std::string(key), std::forward<TArgs>(args)...);
    }

    struct Empty
    {
    };

    template<typename TRegistrar, auto KEY, bool INHERIT_FROM_BASE = true>
    struct RegisterConst : std::conditional_t<INHERIT_FROM_BASE, TBase, Empty>
    {
        friend TRegistrar;

    private:
        using Base = std::conditional_t<INHERIT_FROM_BASE, TBase, Empty>;
        using Base::Base;

        struct Private
        {
            friend RegisterConst;

        private:
            static bool register_class()
            {
                Shared::get_factories()[TKey{KEY}] = [](TArgs&&... args) -> TBase* {
                    return new TRegistrar(std::forward<TArgs>(args)...);
                };
                return true;
            }
        };

        [[maybe_unused]] inline static bool s_registered{Private::register_class()};
        using ValueUser = DummyUser<const bool*, &s_registered>;
    };

    template<typename TRegistrar, bool INHERIT_FROM_BASE = true>
    struct RegisterDyn : std::conditional_t<INHERIT_FROM_BASE, TBase, Empty>
    {
        friend TRegistrar;

    private:
        using Base = std::conditional_t<INHERIT_FROM_BASE, TBase, Empty>;
        using Base::Base;

        struct Private
        {
            friend RegisterDyn;

        private:
            static bool register_class()
            {
                Shared::get_factories()[TKey{TRegistrar::get_key()}] = [](TArgs&&... args) -> TBase* {
                    return new TRegistrar(std::forward<TArgs>(args)...);
                };
                return true;
            }
        };

        [[maybe_unused]] inline static bool s_registered{Private::register_class()};
        using ValueUser = DummyUser<const bool*, &s_registered>;
    };
};
