# Shaders
Shaders are handled slightly differently within Lost to most other c++ engines that are out there. 
Within lost they are used much similarly to how they are in Unity or Unreal engine.

`TODO: Update images, remove renderqueue

---
# Contents
 - [[Shaders#Built-in shader code|Built-in shader code]] information about what the Lost engine does by default
 - [[Shaders#Creating a shader|Creating A Shader]]
---
### Built-in shader code
These are the shaders that Lost will use by default, Lost uses these when any shader input uses the "built-in" shader
```glsl
// Vertex Shader
#version 460 core

layout (location=0) in vec3 vertPos;
layout (location=1) in vec2 vertTexCoord;
layout (location=2) in vec4 vertColor;
layout (location=3) in vec3 vertNormal;
layout (location=4) in mat4 mvp;

out vec3 fragPos;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main() {
    gl_Position = vec4(vertPos, 1.0) * mvp;
    fragPos = vertPos;
    fragColor = vertColor;
    fragTexCoord = vertTexCoord;
    fragNormal = vertNormal;
}
```
```glsl
// Fragment Shader
#version 460 core

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

layout (location=0) out vec4 finalColor;

uniform sampler2D color;

void main() {
    finalColor = texture(color, fragTexCoord);
}
```
---
# Creating A Shader
Creating a shader within lost isn't as simple as usual as the renderer requires more information.
## Example
First off, lets just jump to it, we'll copy the code from [[Shaders#Built-in shader code|above]] into a `.vs` and `.fs` file respectively (file extension doesn't matter but it's good to keep it easy to read)
The only change we will make is changing a line in a fragment shader:
```glsl
// From:
finalColor = texture(color, fragTexCoord);
// To:
finalColor = texture(color, fragTexCoord) * vec4(1.0f, 0.0f, 0.0f, 1.0f);
```
This will just turn the entire image red cutting off the green and blue values, while keeping the alpha the same

To load this new shader into Lost we can use the `loadShader` function and plug it into a material, here's an example
```cpp
// Load the texture
lost::Texture texture = lost::loadTexture("data/texture.png");

// Load the shader,     lost::loadShader(     vertex     ,      fragment     ,   shaderID  )
lost::Shader shader   = lost::loadShader("data/shader.vs", "data/fragment.fs", "testShader");

// Create a material, using the texture and shader we loaded
lost::Material mat    = lost::makeMaterial({ texture }, "mat", shader, LOST_SHADER_OPAQUE);

// Unloading manually is not necessary, as the engine automatically unloads everything, but it wouldn't hurt
```

Upon running this code, we get these new messages in the logs
![[ShaderLoadInfo.png]]
Let's break this down into each part:
 - All the files are loaded within `lost::loadTexture` and `lost::loadShader`
 - Then, the shader gets created, logging a bunch of information about the shader
 - `VS`: The vertex shader location, if the input is `nullptr` it will use the [[Shaders#Built-in shader code|built-in]] shader
 - `FS`: The fragment shader location, if the input is `nullptr` it will use the [[Shaders#Built-in shader code|built-in]] shader
 - `Shader Queue Order`: The order that the shaders are sorted, for example: Transparent materials need to be rendered **after** opaque ones, A lower number is rendered first a higher one last
 - `Texture Input for materials`: The texture inputs that the shader has accessible, this is what the first input in `lost::makeMaterial` is for, each texture put into the material is put into the appropriate texture slot
 - `Uniform Inputs for materials`: This is the list of uniforms that were found within the shader, these can be set in materials
### What is `Shader Queue Order`?
When rendering with the [[Rendering#3D Renderer|3D Renderer]] the engine doesn't render anything until it's told to or the end of a frame as an optimisation.
This also allows for transparency to work properly, since it needs to be rendered **after** all opaque meshes and sorted by depth.
For more information why [this website](https://learnopengl.com/Advanced-OpenGL/Blending) has a great example for why.

This can also allow for extra effects, like "pseudo-postprocessing", place a mesh with inverted normals around the camera with a material around it that has a high shader queue and the box will always be rendered last, going over the top of everything
### What is `Texture Input for materials`?
In shader code, uniform that has the type `sampler2D` is a texture input, and when loading a shader Lost recognises these and initialises them for use.
When creating a material you need to put in a list of textures, these texture are what is put into these slots.
The slot id is the index that the texture goes into in the list and the name is just to tell you what texture should go there

For example:
``` 
- Texture Inputs for materials:
    - color, slot: 0
    - normal, slot: 1
```
```cpp
// Would require a material that would look like this:
lost::makeMaterial({ tex, texNormalMap }, "mat", shaderLight)
// "shaderLight" here is an example since the base shader doesn't support normal maps
```




`TODO: Consider adding custom indexed attributes, this really fucks with vertex buffers though and might need a refactor, but it does optimise having multiple materials with the same shader and different values, sadly textures cannot be attributes though and so this optimisation only works on a lot of things with the same texture`

For help on coding shaders if you've never worked with them before, I highly recommend [The Book of Shaders](https://thebookofshaders.com/) 

