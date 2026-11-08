# Real-Time Audio Rules

These rules bind every line of NXF code that can run on the audio
thread, and every API the audio thread calls. They are acceptance
criteria, not aspirations.

## In the audio callback (and anything it calls)

1. **No heap allocation or deallocation.** All buffers, nodes and
   queues are preallocated before the stream starts. This includes
   hidden allocation: no growing containers, no `std::function`
   copies that allocate, no string formatting.
2. **No blocking synchronization.** No mutex locks that can wait, no
   condition-variable waits, no joining threads, no I/O waits.
3. **No file I/O, no network, no logging I/O, no UI calls.**
4. **Bounded work only.** Loops are bounded by preallocated sizes
   known before the block starts.
5. **No exceptions across the callback boundary**, and no code that
   can throw into the device driver's call stack.

## Communication

- Cross-thread state uses atomics or lock-free/wait-free queues
  where the algorithm justifies it.
- **A type is never *claimed* lock-free without verification.**
  Where the standard provides it, check
  `std::atomic<T>::is_always_lock_free` (or the runtime
  `is_lock_free()`) for the exact type on the target platform, and
  record the result; a queue is "real-time-safe" only with its
  capacity, producer/consumer counts and memory ordering documented.

## Ownership and lifetime

- The audio thread renders a **snapshot**: graph/state objects it
  reads outlive the block by construction (published snapshots,
  deferred destruction on a non-audio thread after the stream
  confirms retirement).
- Nothing the audio thread reads may be freed by another thread
  while a block could be in flight. Teardown happens with the
  stream stopped, or through a retirement mechanism the engine
  documents and tests.

## Device lifecycle

- Device shutdown, loss and reconfiguration are handled off the
  audio thread: the callback reports state via atomics; UI/worker
  threads stop, rebuild and restart the stream.
- Underruns/overruns are counted and surfaced to the UI — never
  silently swallowed, never "fixed" by allocating in the callback.
