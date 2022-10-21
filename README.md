# GreenBlue

_Work in progress :^)_

This project is meant to be a composition of independent libraries that provide easy to use functions for game development, such as creating windows, receiving input, creating opengl context and abstraction of graphics backend.

This project prefers functionality over abstractions, in case you don't need certain platforms and restrictions associated with them. 

Also this project doesn't do automatic platform and architecture detection in any greater extent than what is provided with compiler (e.g. `sizeof(void*)`).

## Code style
|Thing|Style|
|-|-|
|Macros|`GB_ALLCAPS`|
|Global constants|`GB_ALLCAPS`|
|Enum|`GB_eTitleCase`|
|Functions|`gb_snake_case`|
|Function argument|`lowerCase`|
|Function argument - output pointer|`lowerCase_out`|
|Function error id (const)|`gb_<function_name>_ALLCAPS`|
|Structs|`gb_sTitleCase`|
|Classes (structs that shouldn't be created directly)|`gb_cTitleCase`|
|Struct field|`lowerCase`|
|Local variables|`lowerCase`|

`gb` can be replaced with longer module name (e.g. `gbapp_win32`)

## Contents
The project itself consists of several independent libraries for different purposes and platforms.

### GBAPP-WIN32
Application layer for win32 platform. Provides you with window creation, window configuration functions. Also provides creation of wgl or dx contexts.
    
### GB2D-OPENGL
2D drawing library powered by OpenGL3.3.
    