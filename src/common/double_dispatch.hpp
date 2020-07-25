
//
// Created by henrik on 17.11.19.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <type_traits>
#include <typeinfo>
#include <utility>


template<typename... UsrArgs>
struct DoubleDispatcher
{
private:
    struct TypeIdPredicate
    {
        template<typename Concrete, typename Base>
        bool check(const Base& base)
        {
            return (typeid(base) == typeid(Concrete));
        }
    };

    struct DynamicCastPredicate
    {
        template<typename Concrete, typename Base>
        bool check(const Base& base)
        {
            return (dynamic_cast<const Concrete*>(&base) != nullptr);
        }
    };

    template<typename Base, typename Concrete>
    struct AddQualifiers;

    template<typename Base, typename Concrete>
    struct AddQualifiers<Base*, Concrete>
    {
        using type = Concrete*;
    };
    template<typename Base, typename Concrete>
    struct AddQualifiers<Base&, Concrete>
    {
        using type = Concrete&;
    };
    template<typename Base, typename Concrete>
    struct AddQualifiers<const Base*, Concrete>
    {
        using type = const Concrete*;
    };

    template<typename Base, typename Concrete>
    struct AddQualifiers<const Base&, Concrete>
    {
        using type = const Concrete&;
    };

    template<typename T>
    static T& deref(T* v)
    {
        return *v;
    }

    template<typename T>
    static T& deref(T& v)
    {
        return v;
    }

public:
    /**
     * Wrapper of custom_dispatch with Predicate = TypeIdPredicate
     * Calls Handler::on_dispatch(Type) for every type whose runtime type is Type
     * Rejects any type that is not explicitly specified in ConcreteTypes, even if it is a derivative of one
     * If you also want to receive types which inherit from a specified type, use relaxed_dispatch
     * See custom_dispatch for documentation of parameters
     */
    template<typename Handler, typename BaseType, typename... ConcreteTypes>
    static bool strict_dispatch(Handler handler, BaseType param, UsrArgs... args)
    {
        return double_dispatch<Handler, TypeIdPredicate, BaseType, ConcreteTypes...>(
            handler, param, TypeIdPredicate(), std::forward<UsrArgs>(args)...);
    }

    /**
     * Wrapper of custom_dispatch with Predicate = DynamicCastPredicate
     * Calls Handler::on_dispatch(Type) for every type whose runtime type is Type or a derivative of Type
     * See custom_dispatch for documentation of parameters
     */
    template<typename Handler, typename BaseType, typename... ConcreteTypes>
    static bool relaxed_dispatch(Handler handler, BaseType param, UsrArgs... args)
    {
        return double_dispatch<Handler, DynamicCastPredicate, BaseType, ConcreteTypes...>(
            handler, param, DynamicCastPredicate(), std::forward<UsrArgs>(args)...);
    }

    /**
     * Calls Handler::on_dispatch(Type) with the correct concrete type of param
     * @tparam Handler Type to call the concrete methods on
     * @tparam Predicate Functor with bool check<ConcreteType>(BaseType) method to determine if BaseType can be casted
     * to ConcreteType
     * @tparam BaseType Base of all dispatched types. This type should be fully qualified (e.g. const, pointer /
     * reference)
     * @tparam ConcreteTypes List of all types a concrete function is provided for. These should not be qualified
     * @param handler Object to call concrete methods on
     * @param param Base object to dispatch
     * @param pred Functor of type Predicate
     * @return True if a matching concrete function for the runtime type of param was found
     */
    template<typename Handler, typename Predicate, typename BaseType, typename... ConcreteTypes>
    static bool custom_dispatch(Handler handler, BaseType param, Predicate pred, UsrArgs... args)
    {
        return double_dispatch<Handler, Predicate, BaseType, ConcreteTypes...>(
            handler, param, pred, std::forward<UsrArgs>(args)...);
    }

private:
    template<typename Handler, typename Predicate, typename BaseType, typename DerivedType, typename... DerivedTypes>
    static bool double_dispatch(Handler handler, BaseType param, Predicate pred, UsrArgs... args)
    {
        if(pred.template check<DerivedType>(deref(param)))
        {
            handler.on_dispatch(static_cast<typename AddQualifiers<BaseType, DerivedType>::type>(param),
                                std::forward<UsrArgs>(args)...);
            return true;
        }
        return double_dispatch<Handler, Predicate, BaseType, DerivedTypes...>(
            handler, param, pred, std::forward<UsrArgs>(args)...);
    }

    template<typename Handler, typename Predicate, typename BaseType>
    static bool double_dispatch(Handler, BaseType, Predicate, UsrArgs...)
    {
        return false;
    }
};
