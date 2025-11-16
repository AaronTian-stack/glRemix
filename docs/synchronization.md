# Synchronization Dev Notes

IPC Milestone 2 Deliverables:

1. Event-based IPC synchronization
2. Double-buffered IPC interchange
    1. IMPORTANT: Never skip host app frame
3. Refactor to remove extra buffer used in shim via FrameRecorder

## Event Objects in Windows

`CreateEventW`: creates the initial event object. Initializes in a non-signaled state.
`WaitForSingleObject` & `WaitForMultipleObjects`: Blocks the calling thread until one or multiple events is signaled.
