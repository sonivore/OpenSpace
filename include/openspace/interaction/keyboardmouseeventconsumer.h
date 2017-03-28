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

#ifndef __OPENSPACE_CORE___KEYBOARDMOUSEKEYBOARDMOUSEEVENTCONSUMER___H__
#define __OPENSPACE_CORE___KEYBOARDMOUSEKEYBOARDMOUSEEVENTCONSUMER___H__

#include <openspace/util/mouse.h>
#include <openspace/util/keys.h>

namespace openspace {
namespace interaction {

class KeyboardMouseEventConsumer {
public:
    /**
     * Handle keyboard event
     */
    virtual bool handleKeyboard(Key key, KeyModifier mod, KeyAction action) { return false; };

    /**
     * Handle character event
     */
    virtual bool handleCharacter(unsigned int codepoint, KeyModifier modifier) { return false; };

    /**
     * Handle mouse button event
     */
    virtual bool handleMouseButton(MouseButton button, MouseAction action) { return false; };

    /**
     * Handle mouse position 
     */
    virtual bool handleMousePosition(double x, double y) { return false;  }

    /**
     * Handle mouse scroll
     */
    virtual bool handleMouseScroll(double pos) { return false; }
};

}
}

#endif // __OPENSPACE_CORE___KEYBOARDMOUSEKEYBOARDMOUSEEVENTCONSUMER___H__