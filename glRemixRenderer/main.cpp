#include "rt_app.h"

int main()
{
	glRemix::glRemixRenderer renderer;
	// TODO: Make sure to disable the debug layer on release, but we may want it in release builds for testing
	renderer.run_with_hwnd(true);
	return 0;
}
