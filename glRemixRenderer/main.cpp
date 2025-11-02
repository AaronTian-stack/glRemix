#include <iostream>

#include <ipc_protocol.h>

#include <thread>
#include <chrono>
#include <vector>
#include <conio.h>

// temporary test function, can remove
// the important snippets are the ability to check for the IPC file mapping capacity
// in general, `glRemix::IPCProtocol` wraps the actual file mapping object and view logic from IPC
// since that is getting into userland, feel free to change or add function logic there
void _readFrame()
{
    glRemix::IPCProtocol _ipcManager;

    while (!_ipcManager.InitReader())
    {
        std::cout << "IPC Manager could not init a reader instance. Have you launched an "
                     "executable that utilizes glRemix?" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(60));

        if (_kbhit())
        {
            std::cout << "IPC initialization attempts stopped by user." << std::endl;
            return;
        }
    }

    std::cout << "IPC reader instance successfully initialized" << std::endl;

    const uint32_t cap = _ipcManager.GetCapacity();  // ask shared mem for capacity
    std::vector<uint8_t> buffer(cap);                // decoupled local buffer here

    std::cout << "Polling IPC file mapping object for data..." << std::endl;

    // poll ipc here
    uint32_t bytesRead = 0;
    while (!_ipcManager.TryConsumeFrame(buffer.data(),
                                        static_cast<uint32_t>(buffer.size()),
                                        &bytesRead))
    {
        std::cout << "No frame data available." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(60));  // rest before next poll
        if (_kbhit())
        {
            std::cout << "Frame polling stopped by user." << std::endl;
            return;
        }
    }

    std::cout << "Received " << bytesRead << " bytes" << std::endl;

    // loop through data from frame
    size_t offset = 0;
    while (offset + sizeof(glRemix::GLCommand) <= bytesRead)
    {
        const auto* header = reinterpret_cast<const glRemix::GLCommand*>(buffer.data() + offset);
        offset += sizeof(glRemix::GLCommand);

        std::cout << "  Command: " << static_cast<int>(header->type)
                  << " (size: " << header->dataSize << ")" << std::endl;

        switch (header->type)
        {
            case glRemix::GLCommandType::GL_VERTEX3F: {
                const auto* v = reinterpret_cast<const glRemix::GLVertex3fCommand*>(buffer.data()
                                                                                    + offset);
                std::cout << "    glVertex3f(" << v->x << ", " << v->y << ", " << v->z << ")"
                          << std::endl;
                break;
            }  // only have write logic in the shim for `glVertex3f` right now
            default: std::cout << "    (Unhandled command)" << std::endl; break;
        }

        offset += header->dataSize;
    }

    std::cout << "Frame parsed successfully." << std::endl;

    return;  // return after one successful frame read for now
}

int main()
{
    _readFrame();

    std::cout << "Press Ctrl + C to exit" << std::endl; // hold here to read output
    _getch();

    return 0;
}
