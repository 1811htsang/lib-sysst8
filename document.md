# LIBSYSST8 - Finite State Machine (FSM) Library

This library provides a simple and efficient implementation of a Finite State Machine (FSM) in C. It allows you to define states, transitions, and handle events in a structured manner.

`st8` is abbreviation of "state" in the context of this library.

Although the library is meant to be used for FSM context, it is provided a simple implementation of a message queue to facilitate the communication between different tasks and event in form of messages. It also provide a simple implementation to report error code in form of FCR (Fatal Code Return) to facilitate the debugging and error handling.

This whole library is a much lightweight and simple implementation of its parent library `μEDP` which is a more complete implementation of FSM and FCR as well as other features for embedded systems. The `lib-sysst8` library is designed to be easy to use and integrate into existing C projects, making it suitable for a wide range of applications instead of complex configuration and features of `μEDP` library.

## Working Principle

The FSM operates by maintaining a current state and processing incoming messages (events) to determine the next state based on defined transitions. Each state is represented by a function that handles specific messages and can trigger transitions to other states.

This can be visualized as follows:

```asciidoc
╔════════════════════════════════════╗
║             INIT──────1st st8      ║
║cur──────────┐│                     ║
║      ┌──────┼┼───┐                 ║
║      │nxt ┌─┼┼───┼──────────┐      ║
║      │    │ ││ ┌─┼───┐ ┌──┐ ┼───sta║
║      │    │┌┼┼─┼┐│   │ │  │ │      ║
║      │ ┌──┼┼NTRY┼┼──STAY  │ │      ║
║      │ │  │└─┼──┘│     └──┘ │      ║
║      │ │  └──┼───┼──────────┘      ║
║      │ │     ┼───┼─────act         ║
║      │ └────EXIT │                 ║
║      └───────────┘               fn║
╚════════════════════════════════════╝
```

The state function will then decide what to do with these signals, except for the `SYSST8_FSM_SIG_INIT` signal, which is only sent once at the beginning of the FSM's operation.

`INIT` is the initial state, and `1st st8` is the first message received by the FSM. It must be selected with the first state function. The FSM will then process messages and transition between states based on the defined logic.

Next then, with current APIs - `go_next()` and `go_back()`, it is always the two signals `SYSST8_FSM_SIG_NTRY` and `SYSST8_FSM_SIG_EXIT` that are sent to the state function. In `go_next()`, the `SYSST8_FSM_SIG_EXIT` signal is sent to the current state function, and then the `SYSST8_FSM_SIG_NTRY` signal is sent to the next state function. In `go_back()`, the `SYSST8_FSM_SIG_EXIT` signal is sent to the current state function, and then the `SYSST8_FSM_SIG_NTRY` signal is sent to the previous state function. Additionally, this signal couples must ensured the final signal of API is used with `enqueue` to ensure the dispatching of the signal is done in the main loop. If not, the whole FSM wont work. Despite for the top meaning of the API, it must always follow the **exit then entry** manner.

Then the stay signal `SYSST8_FSM_SIG_STAY` is sent to the current state function, which means the FSM will stay in the current state and not transition to any other state.

Between the `SYSST8_FSM_SIG_EXIT` and `SYSST8_FSM_SIG_NTRY` signals, the FSM will perform the action defined in the state function, which may include updating internal variables, performing calculations, or triggering other events.

## Implementation Guidelines

At the end of FSM initialization and before the FSM dispatching, the user must provide the transition using `go_next()`, `go_back()` or `stay()` to ensure the FSM is in a valid state before processing messages. The FSM will not process any messages if transitions are not defined, as a result, nothing will happen.

Then, dive into the function of the state function, the user must handle the signals `SYSST8_FSM_SIG_NTRY`, `SYSST8_FSM_SIG_EXIT`, and `SYSST8_FSM_SIG_STAY` to ensure the FSM operates correctly. As the implementation of the FSM is only based on the NTRY/EXIT/STAY signals, no other signals will be sent to the state function, and the user must handle these signals to ensure the FSM operates correctly.
