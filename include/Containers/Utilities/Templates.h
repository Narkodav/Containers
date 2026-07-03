#pragma once
#include <type_traits>

namespace Containers {

    // --- common CRTP base to provide helpers ---
    template<template<typename...> typename CRTP, typename Derived, typename... Args>
    class CRTPBase {
        using CRTPType = CRTP<Derived, Args...>;

    public:
        // --- Access the final derived object ---
        Derived& derived() {
            return static_cast<Derived&>(static_cast<CRTPType&>(*this));
        }
        const Derived& derived() const {
            return static_cast<const Derived&>(static_cast<const CRTPType&>(*this));
        }

        // --- Access the final derived object ptr ---
        Derived* derivedPtr() {
            return static_cast<Derived*>(static_cast<CRTPType*>(this));
        }
        const Derived* derivedPtr() const {
            return static_cast<const Derived*>(static_cast<const CRTPType*>(this));
        }

        //// --- Access the CRTP layer itself ---
        //CRTPType& crtp() { return static_cast<CRTPType&>(*this); }
        //const CRTPType& crtp() const { return static_cast<const CRTPType&>(*this); }

        // --- Apply a function to the derived object ---
        template<typename Func>
        decltype(auto) withDerived(Func&& f) {
            return std::forward<Func>(f)(derived());
        }

        template<typename Func>
        decltype(auto) withDerived(Func&& f) const {
            return std::forward<Func>(f)(derived());
        }

        // --- Conditional static dispatch to derived ---
        template<typename T, typename Func>
        decltype(auto) ifDerived(Func&& f) {
            if constexpr (std::is_base_of_v<CRTPType, T>) {
                return std::forward<Func>(f)(static_cast<T&>(derived()));
            }
            else {
                // no-op or static assert if desired
            }
        }

        // --- Cast to arbitrary interface (CRTP interface downcast) ---
        template<typename Interface>
        Interface& as() { return static_cast<Interface&>(derived()); }

        template<typename Interface>
        const Interface& as() const { return static_cast<const Interface&>(derived()); }
    };

} // namespace Containers