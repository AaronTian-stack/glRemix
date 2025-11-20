#include "rt_app.h"
#include <string>

int main(int argc, char* argv[])
{
    // Check for -d flag to enable debug layer
    bool enable_debug = (argc > 1 && std::string(argv[1]) == "-d");

    glRemix::glRemixRenderer renderer;
    renderer.run_with_hwnd(enable_debug);
    return 0;
}
