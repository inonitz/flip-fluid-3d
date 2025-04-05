#ifndef __AWC2_INPUT_INTERFACE_HEADER__
#define __AWC2_INPUT_INTERFACE_HEADER__
#include "C/awc2_api.h"
#include "input_types.hpp"


/*
    [NOTE]:
    The isXXX(...) functions aren't meant to be used inside user callbacks - 
    The Input State of the relevant Window/Context is updated before the user-callback,
    but still re-binding contexts on every user-callback is very inefficient/expensive.

    So (ex.) when you call Input::isKeyPressed(...) inside a user-callback,
    the Context state of the library isn't updated (you have to call setCurrentContext(...) ),
    and so the lastest context that was bound will be used, instead of the id supplied in the arguments 
    to your callback.
    Basically, Don't 
*/
namespace AWC2::Input {
    AWC2_API bool isKeyPressed (keyCode key);
    AWC2_API bool isKeyReleased(keyCode key);
    AWC2_API bool isKeyRepeated(keyCode key);
    AWC2_API bool isMouseButtonPressed (mouseButton but);
    AWC2_API bool isMouseButtonReleased(mouseButton but);
    AWC2_API bool isMouseMoving();
    AWC2_API bool isMouseScrollMoving();
    AWC2_API cursorPosition getMouseScrollOffset();
    AWC2_API cursorPosition getMousePosition();
    AWC2_API cursorPosition getMousePositionDelta();
    AWC2_API void           setCursorMode(cursorMode mode);
} // namespace AWC2::Input


#endif