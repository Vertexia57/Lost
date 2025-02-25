# Cheat Sheet
This contains a list of every external function given to the user by the Lost engine.

Every function listed is in the `lost` namespace, and must be prefixed with `lost::` to be accessed.

---
# Contents
- Engine
	- [[#Base Functions]]
	- [[#Window Functions]]
	- [[#Lost State Functions]]
	- [[Lost Cheat Sheet#ImGUI|ImGUI]]
- Rendering
	- [[#Renderer Functions]] (*Contains functions which tell the renderer what to do*)
	- [[#Shader Functions]]
	- [[#Texture And Material Functions]]
	- [[#Mesh Functions]]
	- [[#Camera Functions]]
	- [[#Rendering Functions]] (*Contains functions which draw to screen*)
	- [[#Text Functions]]
- Input
	- [[Lost Cheat Sheet#Keyboard Input|Keyboard Input]]
	- [[Lost Cheat Sheet#Mouse Input|Mouse Input]]
	- [[Lost Cheat Sheet#Controller/Gamepad Input|Controller/Gamepad Input]] TODO
- [[#Data Types]]
# Important Reads
- [[Data Handling within Lost]]
- [[Rendering]]
- [[Shaders]]

---
# Base Functions
More detailed descriptions for these functions can be found [[here]].

These functions don't really fit into any category listed below.

```cpp
// Base functions, used to setup, close and manage the engine
void init(); // Initializes the lost engine
void exit(); // Closes the lost engine, unloading any data created

// Gets the time it took for the LAST frame to finish processing
double getDeltaTime();
int    getFrameRate();
```
---
# Window Functions
More detailed descriptions for these functions can be found [[here]].

`Window`  is defined as `WindowContext*` and so if necessary you can access the values inside of `WindowContext` by dereferencing it.

Any functions which have `Window context = nullptr` will automatically run on the first window created.
This is to remove bloat from any programs that don't use more than one window, which is most of them.

A lot of these functions do not need to ever be touched by the user, that is why they are split here.
`Window functions` are common, `Extra Window Functions` are rare.

```cpp
// Window Functions
Window   createWindow(int width, int height, const char* title = "Application") // Creates a window
bool     windowOpen(); // Returns true if any window is currently open, and false if all are closed, this function also handles window closure, and is necessary for close events to work
void     closeWindow(Window context = nullptr); // Sets the window given to be closed
void     closeAllWindows(); // Closes all active windows
void     setWindowTitle(const char* title, Window context = nullptr); // Sets the title of the window given
void     setWindowPosition(int x, int y, Window context = nullptr); // Sets the position of the window given in display space
void     setWindowSize(int w, int h, Window context = nullptr); // Sets the size of the window given in display space
void     setWindowAttrib(uint32_t attribute, bool state, Window context = nullptr); // Sets the attribute given on the window given, using the enum WindowAttrib
void     makeWindowFullscreen(bool isWindowed = true, Window context = nullptr, int whichMonitor = -1); // Makes the window given fullscreen
//        ^ By default sets the first window created to a borderless fullscreen mode
//          "isWindowed" tells GLFW if it should use a borderless fullscreen or a real fullscreen
//          "whichMonitor" tells GLFW which monitor to put the screen on, by default using the primary monitor

// Extra Window Functions (Mainly used internally within the engine but are exposed since they can be useful)
Window   getWindow(unsigned int id = 0) // By default returns the first window created, but you can get a specific window using the ID value
Window   getCurrentWindow(); // Returns the current window active
uint32_t getCurrentWindowID(); // Returns the index of the currently active window
const std::vector<Window>& getWindows(); // Returns the array of all windows created
Window   getInvisibleContext(); // Returns the invisible context which stores graphics data
void     pushWindow(); // Pushes the current window context to the window stack
void     popWindow();  // Pops the top of the window stack off and sets the current window context to that window
void     setWindowCloseCallback(Window context, void(*callback)()); // Takes in a window and a void function reference and binds it to the window context
//        ^ Would advise using setWindowCloseCallback(window, &lost::closeAllWindows); when using multiple windows
```
---
# Renderer Functions
More detailed descriptions how the renderer works can be found [[Rendering|here]].

The renderer handles rendering meshes, automatically batching identical meshes and rendering them instanced depending on the current rendering mode.
The renderer handles all OpenGL graphics data excluding textures and shaders.

```cpp
// Renderer Functions
void setRenderMode(unsigned int renderMode); // Sets the render mode of the renderer
void renderInstanceQueue(); // Renders the instance queue, clearing it out

// Frame Functions
void beginFrame(Window context = nullptr); // Starts the frame, rendering to the context given, by default uses the first window created
void endFrame(); // Swaps the frame buffer to the screen, displaying it to the user

// Output Buffers. These are outputs in normal shaders and are inputs in post processing shaders
void addOutputBuffer(const char* bufferName, unsigned int format = LOST_FORMAT_RGBA, float* fillColor = nullptr); // By default fills with black, regardless of format
```
---
# Rendering Functions
More detailed descriptions for these functions can be found [[here]].

```cpp
void renderTexture(Texture texture, Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, -1.0f, -1.0f });
//    ^ Renders the texture to the screen using the default shader
//      "bounds" is the area of the screen it renders to and "texBounds" is the area on the texture it will use in pixels.
//      by default rendering the whole texture

void renderTexture(Texture texture, float x, float y, float w = -1, float h = -1);
//    ^ Renders the texture to the screen in the area given, the -1 just shows that it uses the texture's width as the width instead of a set width

// Renders the mesh given to the screen
void renderMesh(Mesh mesh, std::vector<Material> materials, glm::mat4x4& transform);
void renderMesh(Mesh mesh, std::vector<Material> materials, Vec3 pos, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Vec3 scale = { 1.0f, 1.0f, 1.0f });

void renderQuad3D(Material mat, Vec3 position, Vec2 size, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f }); 
//    ^ Renders a quad in 3D space, taking in a 3D position, size and rotation, texBounds is the UV bounds from 0.0f - 1.0f

void renderQuad(Material mat, Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f });
//    ^ Renders a quad to the screen not caring for perspective or view, texBounds is the UV bounds from 0.0f - 1.0f

void setCullMode(unsigned int cullMode)
//    ^ Sets the cull mode of the renderer, using LOST_CULL_FRONT, LOST_CULL_BACK, LOST_CULL_NONE, LOST_CULL_FRONT_AND_BACK and LOST_CULL_AUTO
//      LOST_CULL_AUTO is the default, when this mode is set it uses the cull mode of the material being used (Which by default is LOST_CULL_BACK)
```

---
# Camera Functions
More detailed descriptions for these functions can be found [[here]].

Each window context has a camera attached to it, this camera controls how each mesh rendered gets projected onto the screen
Each camera has their own transform and FOV information, meaning that they are not shared across windows. (This only really matters if you have more than one, which is very rare)

By default camera's are set at (0,0,0) face (0,1,0) and use a 3D projection

```cpp
void pushMatrix(); // Pushes the current camera matrix to a stack which can be popped off later
void popMatrix(); // Pops the matrix off the top of the stack and uses that as the new camera matrix

// Perspective Functions
void cameraUseScreenSpace(); // Sets the current camera's projection to screenspace, making rendering 2D
void cameraUseProject(); // Sets the current camera's projection to projected, making rendering 3D
void setCameraFOV(float degrees); // Sets the vertical FOV of the camera, in degrees

// Transform functions
void setCameraPosition(const glm::vec3& position); // Sets the position of the camera
void setCameraRotation(const glm::vec3& rotation); // Sets the rotation of the camera, using euler rotation
void setCameraScale(const glm::vec3& scale); // Sets the scale of the camera (A bigger camera shrinks the world)
void cameraLookAt(const glm::vec3& location); // Sets the rotation of the camera to look at a certain point, taking the current position into account
void cameraLookAtRelative(const glm::vec3& location); // Sets the rotation of the camera to look at a certain point relative to the current position of the camera

void setCameraTransform(const glm::vec3& translate, const glm::vec3& rotation, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f });
//    ^ Sets every value of the camera at once, is much more efficient than running all 3 seperately
```
---
# Shader Functions
More detailed descriptions for these functions can be found [[here]].

**For information about data loading, please read [[Data Handling within Lost|here]] as Lost does quite a bit of stuff in the background that may be useful to know**.

```cpp
// Shader Load Functions
Shader loadShader(const char* vertexLoc, const char* fragmentLoc, const char* id); 
//                             ^ When either is nullptr, loads the default module for that pipeline
Shader makeShader(const char* vertexCode, const char* fragmentCode, const char* id);  
//                             ^ When either is nullptr, loads the default module for that pipeline
Shader getShader(const char* id);
void   unloadShader(const char* id);
void   unloadShader(Shader& shader);
void   forceUnloadShader(const char* id);
void   forceUnloadShader(Shader& shader);

void   useShader(Shader shader);
void   clearShader(); // Uses the base shader
void   setUniform(const char* uniformName, void* data);

// Post Processing Functions
PostProcessShader loadPostProcessShader(const char* vertexLoc, const char* fragmentLoc, const char* id, RenderPass* passInputs = nullptr); 
//                 ^ Creates a post processing shader, "passInputs" are the extra passes that are fed into the shader's texture slots  
//                   When either is nullptr, loads the default module for that pipeline Eg. makePostProcessShader(nullptr, "blur.fs", "blur");
PostProcessShader makePostProcessShader(const char* vertexCode, const char* fragmentCode, const char* id, RenderPass* passInputs = nullptr); // Same as above
PostProcessShader getPostProcessShader(const char* id);
void              unloadPostProcessShader(PostProcesShader& ppShader);
void              forceUnloadPostProcessShader(PostProcesShader& ppShader);

void              enablePostProcessShader(PostProcessShader ppShader); // Enables a post processing shader in the renderer
void              disablePostProcessShader(PostProcessShader ppShader); // Disables a post processing shader in the renderer
void              bindCustomPostProcessFunction(PostProcessShader ppShader, void(*callback)(PostProcessShader ppShader));
//                 ^ Takes a custom callback and uses that function when the post process shader is being ran, this allows for custom texture managment
```
---
# Texture And Material Functions
More detailed descriptions for these functions can be found [[here]].

**For information about data loading, please read [[Data Handling within Lost|here]] as Lost does quite a bit of stuff in the background that may be useful to know**.

```cpp
// Image Functions (CPU/RAM side images. Slow but modifiable) Images cannot be rendered to the screen and need to be made into textures to work
// NOT CURRENTLY IMPLEMENTED
Image   loadImage(const char* fileLocation, const char* id = nullptr);
Image   getImage(const char* id);
void    unloadImage(Image image);
void    forceUnloadImage(Image image);

// Texture Functions (GPU/VRAM side images. Fast but constant)
Texture loadTexture(const char* fileLocation, const char* id = nullptr);
Texture makeTexture(Image image, const char* id = nullptr);
Texture getTexture(const char* id);
void    unloadTexture(const char* id);
void    unloadTexture(Texture texture);
void    forceUnloadTexture(const char* id);
void    forceUnloadTexture(Texture texture);

// Material Functions
Material makeMaterial(std::vector<Texture> textures, const char* id, Shader shader = nullptr, unsigned int renderQueue = LOST_SHADER_OPAQUE); 
//        ^ Takes the list of textures as the input into the shaders texture slots, if shader is not set, uses the default shader. 
//          This shader changes based on which renderer is being used
Material getMateral(const char* id);
void     destroyMaterial(const char* id);
void     destroyMaterial(Material material);
void     forceDestroyMaterial(const char* id);
void     forceDestroyMaterial(Material material);
```
---
# Mesh Functions
More detailed descriptions for these functions can be found [[here]].

*Lost can currently only load .objs at the moment*

**For information about data loading, please read [[Data Handling within Lost|here]] as Lost does quite a bit of stuff in the background that may be useful to know**.

```cpp
// Mesh Load Functions
Mesh loadMesh(const char* objLoc, const char* id = nullptr);
Mesh makeMesh(MeshData& meshData, const char* id);
Mesh getMesh(const char* id);
void unloadMesh(const char* id);
void unloadMesh(Mesh& obj);
void forceUnloadMesh(const char* id);
void forceUnloadMesh(Mesh& obj);

// [-------------------------]
//   Immediate Mesh Creation
// [-------------------------]

void beginMesh(unsigned int renderMode = LOST_MESH_TRIANGLES, bool screenspace = false);
//    ^ Starts the creation of a mesh which will be rendered once endMesh() is ran

void addVertex(Vec3 position, Color vertexColor = { 1.0f, 1.0f, 1.0f, 1.0f }, Vec2 textureCoord = { 0.0f, 0.0f }, Vec3 vertexNormal = { 0.0f, 0.0f, 1.0f }); 
void addVertex(Vertex vertex); 
//    ^ Adds a vertex to the mesh being created, must be ran after beginMesh()

void endMesh(Material material);
void endMesh(std::vector<Material>& materials);
//    ^ Finishes the creation of a mesh and renders it, using the materials provided

void setMeshTransform(glm::mat4x4& transform); // Sets the *WORLD* transform of the mesh being rendered, must be ran after beginMesh()
void setMeshTransform(Vec3 position, Vec3 scale = { 1.0f, 1.0f, 1.0f }, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, bool screenspace = false);
//    ^ Sets the *WORLD* transform of the mesh being rendered, must be ran after beginMesh()
//      "screenspace" when true will use a screenspace projection

```

---
# Text Functions
More detailed descriptions for these functions can be found [[here]].

Portions of this software are copyright ©2024 The FreeType Project (www.freetype.org).  All rights reserved.

**For information about data loading, please read [[Data Handling within Lost|here]] as Lost does quite a bit of stuff in the background that may be useful to know**.

```cpp

// Font Load Functions
Font loadFont(const char* fontLoc, float fontHeight, const char* id = nullptr);
//    ^ Loads the font at the location given, "fontHeight" dictates the size the font will take in the texture and the size
//      renderText() will use when "scale" is 1. This is essentially the "quality" of the font
Font getFont(const char* id);
void unloadFont(const char* id);
void unloadFont(Font& font);
void forceUnloadFont(const char* id);
void forceUnloadFont(Font& font);

// Text Rendering Functions
Vec2  textBounds(const char* text, Font font, float scale); // Returns the width and height of what the text given would take up when rendered
float textWidth (const char* text, Font font, float scale); // Returns the width of what the text given would take up when rendered
float textHeight(const char* text, Font font, float scale); // Returns the height of what the text given would take up when rendered

void renderText(const char* text, Font font, Vec2 position, float scale, int hAlign = 0, int vAlign = 0);
//    ^ Uses screenspace, hAlign and vAlign change the alignment of the text, horizontally and vertically
void renderTextPro3D(const char* text, Font font, Vec3 position, Vec3 rotation, Vec3 scale, int hAlign = 0, int vAlign = 0); 
//    ^ Renders text to the scene, using 3D space
```
---
# ImGUI
More detailed descriptions for these functions can be found [[here]].

Lost includes ImGUI built in by default, including it in the render pipeline automatically.
For a great manual for ImGUI, read [here](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html).

Note: **ImGUI does not work across multiple window contexts, and only binds to the first one created**
```cpp
// ImGUI Functions

void setupImGui();
//    ^ Initializes ImGUI within the lost engine, binding to the first window created
//      Must be ran AFTER creating a window. Can only be ran once.

// Extra functions
void imGuiDisplayProgramInfo(); 
//    ^ Displays a debug window which describes the current state of the engine,
//      including loaded assets, render passes, FPS and window contexts
```

---
# Lost State Functions
More detailed descriptions for these functions can be found [[here]].

The Lost State is a globally accessible singleton which stores info about the current state of the engine.
This information can be modified to change the way the engine works.

```cpp
// Lost State Functions
void setErrorMode(unsigned int mask, bool state); // Sets the error mode given to the state.
void setStateData(unsigned int which, const void* data) // Sets the data at the location given in "which" with the StateData enum.
void setDefaultMode() // Reverts the settings of the lost engine to it's original settings
const LostState& getLostState(); // Returns the current state of the lost engine, use this if you need to read values which are stored in the lost state
```
---
# Log Functions
More detailed descriptions for these functions can be found [[here]].

The log functions here are functions which help with debugging and are what is used internally within the Lost engine.
```cpp
// Log Levels (in order)
enum {
	LOST_LOG_NONE,    // ( White  ) Text to console, no context shown, or note
	LOST_LOG_SUCCESS, // ( Green  ) Text to console, no context shown
	LOST_LOG_INFO,    // ( Blue   ) Text to console, no context shown
	LOST_LOG_WARNING, // ( Yellow ) Text block to console, context shown if given
	LOST_LOG_ERROR,   // ( Orange ) Error popup appears showing text and pausing execution, text block to console, context shown if given
	LOST_LOG_FATAL    // ( Red    ) Kills program, error popup appears showing text, text block to console, context shown if given
}

// Log Functions
// [ Debug Mode Only! ]
void debugLog(const char* text, unsigned int level); // Logs the text
void debugLogIf(bool condition, const char* text, unsigned int level); // Logs the text if the condition is met
// [ Debug + Release Mode. ]
void log(const char* text, unsigned int level); // Logs the text, even in release mode
void setLogContext(std::string context); // Add context for the log function, helps with error messages, can be cleared with clearLogContext()
void clearLogContext(); // Clears the log context, does nothing unless setLogContext() has been ran

void startProcessTimeLog(); // Starts a timer
void endProcessTimeLog(const char* title); // Outputs that timer in nanoseconds to the console
```
---
# Keyboard Input
More detailed descriptions for these functions can be found [[here]].
```cpp
// Keyboard Input Functions
bool getKeyDown(int keyCode); // Returns if a specific key is down, updates once per frame
bool getKeyDown(char key); // Returns if a specific key is down, updates once per frame

// On down, is good for inputs which you only want on the rising edge of the key press, these only return true for one frame
bool getKeyTapped(int keyCode); // Returns true when a specific key starts being pressed down, updates once per frame
bool getKeyTapped(char key); // Returns true when a specific key starts being pressed down (Works with unicode characters <126 / <FF, updates once per frame

// On up, is good for inputs which you only want on the falling edge of the key press, these only return true for one frame
bool getKeyReleased(int keyCode); // Returns true when a specific key has been released, updates once per frame
bool getKeyReleased(char key); // Returns true when a specific key has been released, updates once per frame

```
---
# Mouse Input
More detailed descriptions for these functions can be found [[here]].
```cpp
// Mouse Input Functions

Vec2 getMousePosition(); // Returns the current position of the mouse relative to the focussed window
float getMouseX(); // Returns the current x position of the mouse relative to the focussed window
float getMouseY(); // Returns the current y position of the mouse relative to the focussed window

void setMousePosition(float x, float y, Window window = nullptr);
//    ^ Sets the current position of the mouse relative to the window given, by default using the first window created

bool getMouseDown(int mouseButton); // Returns true when the selected mouse button is down
bool getMouseTapped(int mouseButton); // Returns true when the selected mouse button has started being pressed, only true for one frame
bool getMouseReleased(int mouseButton); // Returns true when the selected mouse button has been released, only true for one frame

int getMouseScroll(bool horizontal = false); // Returns the amount the wheel has been scrolled in that frame
//                       ^ On some mice they can scroll horizontally, this is also a thing when using gestures on a mousepad
```
---
# Controller/Gamepad Input
More detailed descriptions for these functions can be found [[here]].
```cpp
// Gamepad Input Functions
```
---
# Data Types

```cpp
// All "types" below are just typedefs to a pointer, which is why you can dereference the values in them but cannot access them normally
typedef WindowContext*   Window;
typedef _Texture*        Texture;
typedef _Shader*         Shader;
typedef _PPShader*       PPShader;
typedef _Material*       Material;
typedef void*            Mesh;
```

# TODO