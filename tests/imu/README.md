# IMU host tests

Uses the actual ST register driver with an in-memory bus, including embedded
register bank switching. No hardware or STM32 HAL is needed.

```sh
cmake -S tests/imu -B .pio/build/imu-host -G Ninja
cmake --build .pio/build/imu-host
ctest --test-dir .pio/build/imu-host --output-on-failure
```

Choose another generator if Ninja is unavailable. Tests cover configuration,
reset timeout, every initialization transaction failure and retry, missing
callbacks, identity mismatch, all ready-flag combinations, signed conversions,
cache preservation, error recovery, and sample validity/freshness.

The IMU constructor requires a populated `stmdev_ctx_t` and a millisecond delay
callback. Bus callbacks must return zero on success and enforce transaction
timeouts. Board bus adapters and task/interrupt scheduling are supplied by the
caller. Polling does not enable sensor interrupt routes. Call from an ISR only
if the callbacks support that context; otherwise notify a task. Serialize
sampling, initialization and getter access across execution contexts. Getter
pointers refer to internal caches, not snapshots. Inspect `getValidFields()`
before consuming data and `getUpdatedFields()` to identify fresh fields.

These are host tests; real board communication and sensor performance still
require hardware validation. Defaults match the 2026 ranges and rates: +/-16 g,
+/-2000 dps, 12.5 Hz. Converted units are mg, mdps and degrees Celsius.
