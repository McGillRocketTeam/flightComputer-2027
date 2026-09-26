#pragma once

#include <telemetry/Telemetry.h>
#include <radios/Radios.h>

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

template <bool...>
struct bool_pack {};

template <bool... Bs>
using all_true = std::is_same<bool_pack<true, Bs...>, bool_pack<Bs..., true>>;

class AtomicRule {
   public:
    // Returns the value of the AtomicRule
    bool get() const { return function ? function() : false; }

    // Constructs an AtomicRule from an arbitrary function that evaluates to a
    // boolean
    AtomicRule(std::function<bool()> function) { this->function = function; }

    // Constructs an AtomicRule from any number of Flight Stages for which it is
    // true
    template <typename... FlightStages>
    AtomicRule(Telemetry* telemetry, FlightStages... values) {
        static_assert(all_true<std::is_same<flight_stage_t, FlightStages>::value...>::value,
                      "All arguments must be Telemetry::flight_stage");

        std::vector<flight_stage_t> stages = {values...};

        this->function = [telemetry, stages]() {
            return (telemetry) &&
                   (std::find(stages.begin(), stages.end(),
                              telemetry->rocket_telem.flight_stage) !=
                    stages.end());
        };
    }

    // Constructs an AtomicRule from an arbitrary boolean value to check
    explicit AtomicRule(bool* variable) {
        this->function = [variable]() { return (variable) && (*variable); };
    }

    // Constructs an AtomicRule from a specified interval of the packet counter
    AtomicRule(Radio* radio, uint8_t interval){
        this->function = [radio, interval](){
            return !(radio->packet_counter % interval);
        };
    }

    // Logical combinations of AtomicRules

    AtomicRule andWith(const AtomicRule& other) const {
        return AtomicRule([lhs = this->function, rhs = other.function]() {
            return lhs() && rhs();
        });
    }

    AtomicRule orWith(const AtomicRule& other) const {
        return AtomicRule([lhs = this->function, rhs = other.function]() {
            return lhs() || rhs();
        });
    }

    AtomicRule negate() const {
        return AtomicRule([func = this->function]() { return !func(); });
    }

    AtomicRule operator!() const{
        return this->negate();
    }

   private:
    std::function<bool()> function;  // Internal Rule Function
};