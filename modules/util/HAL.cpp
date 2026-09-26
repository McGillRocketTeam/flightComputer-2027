#include <radios/STM32HAL.h>
#include <util/HAL.h>

static InterruptBinding s_extiCallbacks[NUM_INTERRUPTS];

// Overrides the default _weak interrupt callback. Allows for interrupts to
// trigger custom callbacks
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == 0) {
        return;
    } else {
        // Convert from one-hot bitmask to interrupt channel number
        int interruptLine = __builtin_ctz(GPIO_Pin);

        fireCallback(interruptLine);
    }
};

// Save Callback to table
void setCallback(uint32_t interruptNum, void (*cb)()) {
    // Functions must be registered with a context, so any function that does
    // not need one becomes the context MRT_HAL_noContext takes in the context,
    // and executes it.
    setCallback(interruptNum, &MRT_HAL_functionAsContext,
                reinterpret_cast<void*>(cb));
}

void setCallback(uint32_t interruptNum, void (*cbContextual)(void*),
                 void* context) {
    if (interruptNum > 0 && interruptNum < NUM_INTERRUPTS) {
        s_extiCallbacks[interruptNum].callback = cbContextual;
        s_extiCallbacks[interruptNum].context = context;
    }
}

void MRT_HAL_functionAsContext(void* ctx) {
    // Smuggles the function in through it's own context.
    auto fn = reinterpret_cast<void (*)()>(ctx);
    fn();
}

// Find and fire callback
void fireCallback(uint32_t interruptNum) {
    if (interruptNum <= 0 || interruptNum >= NUM_INTERRUPTS) {
        return;
    }
    InterruptBinding binding = s_extiCallbacks[interruptNum];
    binding.callback(binding.context);
}
