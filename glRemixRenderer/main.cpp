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
void _readGLFromIPC()
{
    glRemix::IPCProtocol ipc;

    while (!ipc.InitReader())
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

    const uint32_t bufCapacity = ipc.GetCapacity();  // ask shared mem for capacity
    std::vector<uint8_t> ipcBuf(bufCapacity);                // decoupled local buffer here

    // NOTE:
    // IPC can technically already handle continuous reading.
    // Though subsequent calls will have no commands as only glVertex3f writes to IPC right now.
    // This will be fixed ASAP.
    // However, even then, I do plan to flush out synchronization logic more though, so single-frame read might overall be better for the time being.
    int i = 0;
    while (i <= 0)  // set to 0 for single-frame capture, set to true for continuous
    {
        // poll ipc here
        uint32_t bytesRead = 0;
        while (!ipc.TryConsumeFrame(ipcBuf.data(),
                                            static_cast<uint32_t>(ipcBuf.size()),
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

        const auto* frameHeader = reinterpret_cast<const glRemix::GLFrameUnifs*>(ipcBuf.data());
        std::cout << "Frame " << frameHeader->frameIndex << " (" << frameHeader->payloadSize
                  << " bytes payload)\n";

        if (frameHeader->payloadSize == 0)
        {
            std::cout << "Frame " << frameHeader->frameIndex << ": no new commands.\n";
            i++;
            continue;
        }

        // loop through data from frame
        // can comment out the logs here too
        size_t offset = sizeof(glRemix::GLFrameUnifs);
        while (offset + sizeof(glRemix::GLCommandUnifs) <= bytesRead)
        {
            const auto* header = reinterpret_cast<const glRemix::GLCommandUnifs*>(ipcBuf.data()
                                                                                  + offset);
            offset += sizeof(glRemix::GLCommandUnifs);

             std::cout << "  Command: " << static_cast<int>(header->type)
                       << " (size: " << header->dataSize << ")" << std::endl;

            switch (header->type)
            {
                case glRemix::GLCommandType::GLCMD_VERTEX3F: {
                    const auto* v = reinterpret_cast<const glRemix::GLVertex3fCommand*>(
                        ipcBuf.data() + offset);
                     std::cout << "    glVertex3f(" << v->x << ", " << v->y << ", " << v->z << ")"
                               << std::endl;
                    break;
                }  // only have write logic in the shim for `glVertex3f` right now
                default: std::cout << "    (Unhandled command)" << std::endl; break;
            }

            offset += header->dataSize;
        }

        std::cout << "Frame " << frameHeader->frameIndex << " parsed successfully." << std::endl;

        i++;
    }
}

int main()
{
    _readGLFromIPC();

    std::cout << "Press Ctrl + C to exit" << std::endl; // hold here to read output
    _getch();

    return 0;
}
