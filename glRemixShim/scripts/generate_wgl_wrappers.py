"""Generate WGL wrapper definitions from a function list."""

import argparse
from pathlib import Path


# WGL function signatures from Windows opengl32.dll, obtained using dumpbin /exports
WGL_FUNCTIONS = [
    ("int", "wglChoosePixelFormat", "HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd", "hdc, ppfd", "0"),
    ("BOOL", "wglCopyContext", "HGLRC src, HGLRC dst, UINT mask", "src, dst, mask", "FALSE"),
    ("HGLRC", "wglCreateContext", "HDC dc", "dc", "nullptr"),
    ("HGLRC", "wglCreateLayerContext", "HDC dc, int layer", "dc, layer", "nullptr"),
    ("BOOL", "wglDeleteContext", "HGLRC context", "context", "FALSE"),
    ("BOOL", "wglDescribeLayerPlane", "HDC dc, int pixelFormat, int layer, UINT bytes, LPLAYERPLANEDESCRIPTOR descriptor", "dc, pixelFormat, layer, bytes, descriptor", "FALSE"),
    ("int", "wglDescribePixelFormat", "HDC hdc, int pixelFormat, UINT bytes, LPPIXELFORMATDESCRIPTOR descriptor", "hdc, pixelFormat, bytes, descriptor", "0"),
    ("HGLRC", "wglGetCurrentContext", "", "", "nullptr"),
    ("HDC", "wglGetCurrentDC", "", "", "nullptr"),
    ("PROC", "wglGetDefaultProcAddress", "LPCSTR name", "name", "nullptr"),
    ("int", "wglGetLayerPaletteEntries", "HDC dc, int layer, int start, int entries, COLORREF *colors", "dc, layer, start, entries, colors", "0"),
    ("int", "wglGetPixelFormat", "HDC hdc", "hdc", "0"),
    ("BOOL", "wglMakeCurrent", "HDC dc, HGLRC context", "dc, context", "FALSE"),
    ("BOOL", "wglRealizeLayerPalette", "HDC dc, int layer, BOOL realize", "dc, layer, realize", "FALSE"),
    ("int", "wglSetLayerPaletteEntries", "HDC dc, int layer, int start, int entries, const COLORREF *colors", "dc, layer, start, entries, colors", "0"),
    ("BOOL", "wglSetPixelFormat", "HDC hdc, int pixelFormat, const PIXELFORMATDESCRIPTOR *descriptor", "hdc, pixelFormat, descriptor", "FALSE"),
    ("BOOL", "wglShareLists", "HGLRC a, HGLRC b", "a, b", "FALSE"),
    ("BOOL", "wglSwapBuffers", "HDC dc", "dc", "FALSE"),
    ("BOOL", "wglSwapLayerBuffers", "HDC dc, UINT planes", "dc, planes", "FALSE"),
    ("DWORD", "wglSwapMultipleBuffers", "UINT count, const WGLSWAP *swaps", "count, swaps", "0"),
    ("BOOL", "wglUseFontBitmapsA", "HDC dc, DWORD first, DWORD count, DWORD listBase", "dc, first, count, listBase", "FALSE"),
    ("BOOL", "wglUseFontBitmapsW", "HDC dc, DWORD first, DWORD count, DWORD listBase", "dc, first, count, listBase", "FALSE"),
    ("BOOL", "wglUseFontOutlinesA", "HDC dc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT metrics", "dc, first, count, listBase, deviation, extrusion, format, metrics", "FALSE"),
    ("BOOL", "wglUseFontOutlinesW", "HDC dc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT metrics", "dc, first, count, listBase, deviation, extrusion, format, metrics", "FALSE"),
    ("BOOL", "wglSwapIntervalEXT", "int interval", "interval", "0"),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-o", "--output", type=Path, required=True, help="Path to write the generated wrappers .inl file")
    return parser.parse_args()


def generate_wrappers(output_path: Path) -> None:
    wrapper_lines = [
        "// Auto-generated. Do not edit manually.",
    ] + [
        f"GLREMIX_WGL_RETURN_WRAPPER({ret_type}, {name}, {f'({params})' if params else '()'}, {f'({args})' if args else '()'}, {default_val});"
        for ret_type, name, params, args, default_val in WGL_FUNCTIONS
        if name != "wglGetProcAddress"
    ]
    
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(wrapper_lines) + "\n", encoding="utf-8")
    print(f"Generated {output_path}")


if __name__ == "__main__":
    args = parse_args()
    generate_wrappers(args.output)
