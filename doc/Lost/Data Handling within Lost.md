# Data Handling
*Want the TL;DR? Read the [[#In Summary|summary]]*.

Lost uses a **global data system** this allows for data to be loaded in one context and accessed in another without carrying the data around or loading it again.
This means that sometimes functions don't do exactly what you might expect them to do.
## *These examples cover textures, but they apply to all data types*

For Example:
```cpp
#include "lost.h"

int main()
{
	lost::init(); // Initialize the Lost engine
	lost::createWindow(500, 500);

	// We'll do something a little dumb here but it's an example of what would happen if you loaded the same thing more than once
	// Like if you had an object which loaded and unloaded the image it used on creation and destruction
	Texture texA = lost::loadTexture("Image.png");
	Texture texB = lost::loadTexture("Image.png"); // Does not load the image again, just gets the one it's already loaded

	while (lost::windowOpen()) // Loops until the window should close
	{
		lost::beginFrame();
		
		// These functions do the exact same thing, since texA and texB are the same
		lost::renderTexture(texA, 50, 50); 
		lost::renderTexture(texB, 200, 50);
		
		lost::endFrame();
	}

	// Each load requires a matching unload, even if it's for the same image
	lost::unloadTexture(texA);
	lost::unloadTexture(texB); // We don't have to use texB here as texA is the exact same reference
	// If you know you won't be using the image later, you can use lost::forceUnloadTexture(tex)

	lost::exit(); // Exit the Lost engine

	return 0;
}
```

This code is an example of how Lost's loading and unloading works.
Lost keeps track of all the data it's loaded and the file locations for that data, checking before loading data to see if it's already loaded it, and just returning the data it already has to save on processing time.

The example here is an abstracted example of what would happen if you put a `loadTexture` and `unloadTexture` function in a class's constructor and destructor.

`lost::unloadTexture()` does not immediately unload the texture it's given, instead it checks to see how many times `lost::loadTexture()` has been ran on it, and only unloads it when the amount of times match.

## Why this is bad
In a lot of cases, objects are created and destroyed quite often, in the case were the object that uses an image is destroyed and created quite often and it's the only instance of the object: Lost will load and unload the texture every single time the object is created and destroyed. 

## What's the solution?
Lost has a solution for this, the definition for the `loadTexture` function has an `id` input, this input is on every single make or load function within Lost.
What this `id` does is it essentially replaces the reference for the load function, and allows you to use the `getTexture` function, here's an example:
```cpp
#include "lost.h"
int main()
{
	lost::init(); // Initialize the Lost engine
	lost::createWindow(500, 500);

	lost::loadTexture("Image.png", "testImage"); // Loads the texture into Lost with the ID "testImage"

	while (lost::openWindow()) // Loops until the window should close
	{
		lost::beginFrame();

		lost::renderTexture(lost::getTexture("testImage"), 50, 50); // Renders the texture with the ID "testImage"
		
		// NOTE! This is ALSO bad... this is just an example
		//
		// lost::getTexture(id) uses a binary tree, which means every use of the function adds quite a bit of
		// overhead depending on how many textures you have stored. O(log n) Specifically 
		//
		// It's best to cache the value you get from the function in a spot that isn't ran every frame;
		// like a constructor, or before the main loop
		
		lost::endFrame();
	}

	lost::unloadTexture("testImage"); // Unloads the texture with the ID "testImage"

	lost::exit(); // Exit the Lost engine

	return 0;
}
```
Doing it this way also saves a lot of the pain of moving files around and having to rewrite all the references to them since you only have to change one in the first load function.

## A little extra
`lost::loadTexture()` when not given an ID will just use the file location given as the ID.
So you would be able to do:
```cpp
lost::loadTexture("Image.png"); // No ID was set, uses the file path instead
lost::getTexture("Image.png"); // Functions identically to how it would with a normal ID
```

# In Summary

Lost stores all data with IDs, you can use these IDs to get data and use it across your program.
Don't like this? Disable it with `lost::setStateData(LOST_STATE_USE_DATA_IDS, (void*)true/false).

*`[data]` here can just be replaced with the data type you need, eg. `Shader`, `Texture`, `Image`*
- You should use data IDs wherever possible, using `lost::get[data](id)` when you need to use that data.
- Any `lost::get[data](id)` functions need to be ran after loading the data, otherwise they don't do anything.
- All `lost::get[data](id)` functions are $log(n) * m$; Where $n$ is the amount of data loaded of that type, and $m$ is the length of the IDs stored in the data handler.
  This isn't great, so like the note says in the example, it's best to use it in places that aren't ran very often, like the main loop.