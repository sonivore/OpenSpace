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

#ifndef __OPENSPACE_MODULE_ONSCREENGUI___ONSCREENGUIMODULE___H__
#define __OPENSPACE_MODULE_ONSCREENGUI___ONSCREENGUIMODULE___H__

#include <openspace/util/openspacemodule.h>
#include <openspace/util/mouse.h>
#include <openspace/util/keys.h>

#include <modules/onscreengui/include/gui.h>

namespace openspace {
    
class OnScreenGUIModule : public OpenSpaceModule {
public:
    OnScreenGUIModule();

    void initialize() override;
    void deinitialize() override;
    void initializeGL() override;
    void deinitializeGL() override;
    void postDraw() override;
    bool handleKeyboard(Key key, KeyModifier mod, KeyAction action) override;
    bool handleCharacter(unsigned int codepoint, KeyModifier modifier) override;
    bool handleMouseButton(MouseButton button, MouseAction action) override;
    bool handleMouseScroll(double position) override;
    
    static gui::GUI gui;
};

} // namespace openspace

#endif // __OPENSPACE_MODULE_ONSCREENGUI___ONSCREENGUIMODULE___H__
