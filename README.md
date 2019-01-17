# mandelbrot-gl
Just another OpenGL mandelbrot set explorer. Renders the Mandelbrot set in OpenGL.

Current features implemented in pixel shader:
* single precision (up to 10^-6 detail)
* emulated double precision (up to 10^-14 detail, but slower)
* true double precision (up to 10^-14 detail, a bit better than emulated)

Future features may include
* nanogui UI 
* minimap rendering
* benchmark mode
* emulated quadruple precision (with 4 floats or 2 doubles)
