quarke
======

**quarke** is a 3D renderer written in C++ using OpenGL 3.3. It uses a modular deferred shading approach to allow for various aesthetic effects in screen-space. It's a lot of Blinn-fun!

Some interesting features;

- Deferred / multipass shading
- Omni-directional dynamic point shadow mapping
- Blinn-phong per-fragment illumination
- Built-in direct-color TGA loader, delegates to tinyobjloader for OBJs

![Screenshot](/img/screenshot-2016-09-28.png)

*It might become a playable game, or it might not- right now, it's quite amusing just working on the rendering pipeline.*
