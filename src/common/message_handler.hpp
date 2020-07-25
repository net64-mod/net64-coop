//
// Created by henrik on 20.02.20.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "common/double_dispatch.hpp"
#include "common/message_interface.hpp"


namespace Impl
{
template<typename, typename, typename...>
struct IMessageHandler;

template<typename InheritFrom, typename Derived, typename MessageType, typename... UsrArgs>
struct Derive : IMessageHandler<InheritFrom, MessageType, UsrArgs...>
{
    using MessageIdPredicate = typename IMessageHandler<InheritFrom, MessageType, UsrArgs...>::MessageIdPredicate;

    void handle_message(const MessageType& msg, UsrArgs... args) final
    {
        static_cast<Derived*>(this)->on_message(msg, std::forward<UsrArgs>(args)...);
    }
};

template<typename InheritFrom, typename Derived, typename MessageType, typename... UsrArgs>
struct DeriveFilter
{
    using MessageIdPredicate = typename IMessageHandler<InheritFrom, MessageType, UsrArgs...>::MessageIdPredicate;

    template<typename... Messages>
    struct Filter : IMessageHandler<InheritFrom, MessageType, UsrArgs...>
    {
        struct Handler
        {
            Derived& obj_;

            template<typename T>
            void on_dispatch(const T& msg, UsrArgs... args)
            {
                obj_.on_message(msg, std::forward<UsrArgs>(args)...);
            }
        };

        void handle_message(const MessageType& msg, UsrArgs... args) final
        {
            DoubleDispatcher<UsrArgs...>::
                template custom_dispatch<Handler, MessageIdPredicate, const MessageType&, Messages...>(
                    Handler{*static_cast<Derived*>(this)}, msg, MessageIdPredicate(), std::forward<UsrArgs>(args)...);
        }
    };
};

/**
 * Interface for all message handlers
 * @tparam Identifier Type to identify message types
 * @tparam UsrArgs additional arguments passed
 */
template<typename InheritFrom, typename MessageType, typename... UsrArgs>
struct IMessageHandler : InheritFrom
{
    using Identifier = typename MessageType::id_t;

    struct MessageIdPredicate
    {
        template<typename Concrete, typename Base>
        bool check(const Base& base)
        {
            return (base.id() == Concrete::ID);
        }
    };

    virtual ~IMessageHandler() = default;

    /// React to message
    virtual void handle_message(const MessageType& msg, UsrArgs... args) = 0;


    template<typename Derived>
    struct Derive final
    {
        using ReceiveAll = Impl::Derive<InheritFrom, Derived, MessageType, UsrArgs...>;

        template<typename... Messages>
        using Receive =
            typename Impl::DeriveFilter<InheritFrom, Derived, MessageType, UsrArgs...>::template Filter<Messages...>;
    };
};

struct Empty
{
};

} // namespace Impl

template<typename MessageType, typename... UsrArgs>
struct IMessageHandler : Impl::IMessageHandler<Impl::Empty, MessageType, UsrArgs...>
{
    template<typename InheritFrom>
    using Base = Impl::IMessageHandler<InheritFrom, MessageType, UsrArgs...>;
};
