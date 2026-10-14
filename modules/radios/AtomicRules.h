#pragma once

#include <telemetry/Telemetry.h>

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

class Radio;

template <bool...>
struct bool_pack {};

template <bool... Bs>
using all_true = std::is_same<bool_pack<true, Bs...>, bool_pack<Bs..., true>>;

class AtomicRule {
   public:
    // Returns the value of the AtomicRule
    bool get() const;

    // Constructs an AtomicRule from an arbitrary function that evaluates to a
    // boolean
    AtomicRule(std::function<bool()> function);

    // Constructs an AtomicRule from any number of Flight Stages for which it is
    // true
    template <typename... FlightStages>
    AtomicRule(Telemetry* telemetry, FlightStages... values);

    // Constructs an AtomicRule from an arbitrary boolean value to check
    explicit AtomicRule(bool* variable);

    // Constructs an AtomicRule from a specified interval of the packet counter
    AtomicRule(Radio* radio, uint8_t interval);


    // Logical combinations of AtomicRules

    AtomicRule andWith(const AtomicRule& other) const;

    AtomicRule orWith(const AtomicRule& other) const;

    AtomicRule negate() const;

    AtomicRule operator!() const;

   private:
    std::function<bool()> function;  // Internal Rule Function
};