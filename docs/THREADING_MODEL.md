# NXF Threading Model

NXF applications have sharply different kinds of threads. Mixing
their rules is how audio software glitches, deadlocks and crashes.

| Thread | Owns | May do | Must never do |
|---|---|---|---|
| **Audio (device callback)** | rendering the current graph snapshot | read preallocated buffers, process DSP, publish level meters via atomics | allocate, lock (blocking), I/O, log, call UI, wait |
| **UI / message** | windows, components, project document edits, user intent | edit the model, publish new state to the audio side, start/stop workers | block on the audio thread, render audio |
| **Worker / background** | file I/O, plugin scanning, project load/save, analysis, export | heavy work, then hand results back to UI/audio via queues | touch UI objects directly, mutate live audio state |
| **Device / OS callbacks (non-audio)** | device-change notifications | translate OS events into NXF events for the UI/worker side | assume they are the audio thread |

## Communication rules

- **UI → audio:** publish immutable/preallocated state snapshots or
  send bounded messages through a real-time-safe queue. The audio
  thread picks changes up at block boundaries; it is never forced
  to wait for the UI.
- **Audio → UI:** atomics and lock-free queues only (levels, states,
  "take finished" notices). The UI polls or is woken; the audio
  thread never calls into UI code.
- **Worker → anywhere:** results are delivered as messages. Ownership
  of buffers transfers explicitly (see `docs/API_DESIGN.md`).
- Every public API documents which thread(s) it may be called from.
  "Thread-safe" without a named pattern is not a specification.

Device loss and reconfiguration are coordinated on the UI/worker
side: stop the stream, rebuild state, restart — never inside the
callback that reported the problem.
