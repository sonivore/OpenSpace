/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2017                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __OPENSPACE_CORE___KEYBINDINGMANAGER___H__
#define __OPENSPACE_CORE___KEYBINDINGMANAGER___H__

#include <openspace/engine/openspaceengine.h>
#include <openspace/util/mouse.h>
#include <openspace/util/keys.h>
#include <openspace/scripting/lualibrary.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/logging/logmanager.h>
#include <openspace/interaction/keyboardmouseeventconsumer.h>

namespace openspace {
namespace interaction {

class KeyBindingManager : public KeyboardMouseEventConsumer {
public:
    KeyBindingManager() = default;
    ~KeyBindingManager() = default;

    void resetKeyBindings();
    void bindKeyLocal(
        Key key,
        KeyModifier modifier,
        std::string luaCommand,
        std::string documentation = ""
    );
    void bindKey(
        Key key,
        KeyModifier modifier,
        std::string luaCommand,
        std::string documentation = ""
    );

    /**
    * Returns the Lua library that contains all Lua functions available to affect the
    * interaction. The functions contained are
    * - openspace::luascriptfunctions::setOrigin
    * \return The Lua library that contains all Lua functions available to affect the
    * interaction
    */
    static scripting::LuaLibrary luaLibrary();

    // Callback functions 
    bool handleKeyboard(Key key, KeyModifier modifier, KeyAction action) override;
    void writeDocumentation(const std::string& type, const std::string& file);

private:
    using Synchronized = ghoul::Boolean;

    struct KeyInformation {
        std::string command;
        Synchronized synchronization;
        std::string documentation;
    };

    std::multimap<KeyWithModifier, KeyInformation> _keyLua;
};

} // namespace interaction
} // namespace openspace

#endif // __OPENSPACE_CORE___KEYBINDINGMANAGER___H__
