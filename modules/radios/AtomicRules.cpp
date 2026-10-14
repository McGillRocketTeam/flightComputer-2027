#include <radios/AtomicRules.h>

#ifdef STM32
#include <radios/Radios.h>
#endif

bool AtomicRule::get() const { return function ? function() : false; }

    // Constructs an AtomicRule from an arbitrary function that evaluates to a
    // boolean
    AtomicRule::AtomicRule(std::function<bool()> function) { this->function = function; }

    // Constructs an AtomicRule from any number of Flight Stages for which it is
    // true
    template <typename... FlightStages>
    AtomicRule::AtomicRule(Telemetry* telemetry, FlightStages... values) {
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
    AtomicRule::AtomicRule(bool* variable) {
        this->function = [variable]() { return (variable) && (*variable); };
    }

    #ifdef STM32
    // Constructs an AtomicRule from a specified interval of the packet counter
    AtomicRule::AtomicRule(Radio* radio, uint8_t interval){
        this->function = [radio, interval](){
            return !(radio->packet_counter % interval);
        };
    }
    #endif

    // Logical combinations of AtomicRules

    AtomicRule AtomicRule::andWith(const AtomicRule& other) const {
        return AtomicRule([lhs = this->function, rhs = other.function]() {
            return lhs() && rhs();
        });
    }

    AtomicRule AtomicRule::orWith(const AtomicRule& other) const {
        return AtomicRule([lhs = this->function, rhs = other.function]() {
            return lhs() || rhs();
        });
    }

    AtomicRule AtomicRule::negate() const {
        return AtomicRule([func = this->function]() { return !func(); });
    }

    AtomicRule AtomicRule::operator!() const{
        return this->negate();
    }