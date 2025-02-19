---

---
# Rendering Engine

There are 2 rendering engines that Lost has inbuilt, `Renderer2D` and `Renderer3D`
These two renderers do much different things, and are specially optimised for each task

---
## 2D Renderer
`Renderer2D` is set automatically with `lost::init()` and is the default rendering engine Lost uses.
The 2D renderer does very little batching unlike the 3D renderer, the 2D renderer renders every batch whenever there is any difference between two meshes
The differences it checks are:
 - Mesh
 - Material
Any difference within these causes the renderer to render the entire batch to the GPU


---
## 3D Renderer
`Renderer3D` has to be set manually withing `lost::init()` by using `lost::init(LOST_RENDER_3D)`
The 3D renderer batches as much as it can, batching everything until `lost::endFrame()` or until `lost::renderInstanceQueue()` is ran
When the instance queue is rendered the renderer does these things in order:
 - Sorts the render list, sorted by, in order:
	 - Shader Sort Queue (Opaque, Transparent, etc.) 
	 - Shader
	 - Mesh
	 - Material
	 - *Material indices (Done in background)*
 - Loops through all queued meshes
	 - Runs a draw call for every unique mesh and material, including their uniforms
  - *After here, this only runs on `lost::endFrame()`*
  - Gets all texture passes from render and puts them into the active post processing shader `TODO: overhaul this entire thing`
  - Renders to screen
---
# Post Processing Shaders

---
# Materials

Lost uses **Materials**, unlike most c++ engines.
Materials have a set **shader**, **texture list** and **uniforms**.

Here's a quick example
```cpp

```
---
