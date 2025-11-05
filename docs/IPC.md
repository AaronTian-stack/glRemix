# IPC Dev Notes

## File mapping

To associate a file on disk with the virtual address space of your process(es). The `file mapping object` is what manages the association, `file view` (virtual mem) makes accessible. The object is a OS kernel obj that manages the physical mem (file on disk).

![fm diagram](./assets/fmap.png)
> [src: Microsoft](https://learn.microsoft.com/en-us/windows/win32/memory/file-mapping)

The file on disk can be anything, but using system paging file is faster (keeps everything in memory). Physical file only necessary for persistent data.

File mapping object doesn't need to load up the entire file. Similarly, file view can consist of only part of file mapping obj.

File mapping object to process is many-to-many, view to obj is many-to-one. Just be sure to unmap old view when you are done / want to make space for another view.

Multiple processes can use the same object to create their own views, and their data will remain identical and coherent. System will handle unloading and loading, swap-in, swap-out to save RAM (virtual memory paging).

## Shared Memory

The biggest worry here is about synchronization. Processes should use a semaphone to prevent data corruption (for sure need this).

Will follow [this code example](https://learn.microsoft.com/en-us/windows/win32/memory/creating-a-view-within-a-file) to get up & and running

## Resources
- https://learn.microsoft.com/en-us/windows/win32/memory/file-mapping
- https://learn.microsoft.com/en-us/windows/win32/winprog64/interprocess-communication
- https://learn.microsoft.com/en-us/windows/win32/dxtecharts/sixty-four-bit-programming-for-game-developers#compatibility-of-32-bit-applications-on-64-bit-platforms
- https://learn.microsoft.com/en-us/windows/win32/ipc/interprocess-communications#using-a-file-mapping-for-ipc
