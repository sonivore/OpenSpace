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

#include <openspace/interaction/keyboardmouseinteractionhandler.h>
#include <openspace/interaction/keyboardmouseeventconsumer.h>

namespace {
    const char* _loggerCat = "KeyboardMouseInteractionHandler";
} // namespace

namespace openspace {
namespace interaction {

void KeyboardMouseInteractionHandler::addEventConsumer(KeyboardMouseEventConsumer* consumer, int priority) {
    _eventConsumers.emplace(std::make_pair(priority, consumer));
}

void KeyboardMouseInteractionHandler::setPriority(KeyboardMouseEventConsumer* consumer, int priority) {
    auto& it = std::find_if(_eventConsumers.begin(), _eventConsumers.end(), [consumer] (const auto& c) {
        return c.second == consumer;
    });
    if (it == _eventConsumers.end()) {
        throw ghoul::RuntimeError("Event consumer is not registered", "");
    } else {
        _eventConsumers.erase(it);
        _eventConsumers.emplace(std::make_pair(priority, consumer));
    }
}

void KeyboardMouseInteractionHandler::removeEventConsumer(KeyboardMouseEventConsumer* consumer) {
    auto& it = std::find_if(_eventConsumers.begin(), _eventConsumers.end(), [consumer](const auto& c) {
        return c.second == consumer;
    });
    if (it == _eventConsumers.end()) {
        throw ghoul::RuntimeError("Event consumer is not registered", "");
    } else {
        _eventConsumers.erase(it);
    }
}


bool KeyboardMouseInteractionHandler::handleKeyboard(Key key, KeyModifier mod, KeyAction action) {
    for (auto& consumer : _eventConsumers) {
        if (consumer.second->handleKeyboard(key, mod, action)) {
            return true;
        }
    }
    return false;
};

bool KeyboardMouseInteractionHandler::handleCharacter(unsigned int codepoint, KeyModifier modifier) {
    for (auto& consumer : _eventConsumers) {
        if (consumer.second->handleCharacter(codepoint, modifier)) {
            return true;
        }
    }
    return false;
};

bool KeyboardMouseInteractionHandler::handleMouseButton(MouseButton button, MouseAction action) {
    for (auto& consumer : _eventConsumers) {
        if (consumer.second->handleMouseButton(button, action)) {
            return true;
        }
    }
    return false;
};

bool KeyboardMouseInteractionHandler::handleMousePosition(double x, double y) {
    for (auto& consumer : _eventConsumers) {
        if (consumer.second->handleMousePosition(x, y)) {
            return true;
        }
    }
    return false;
};

bool KeyboardMouseInteractionHandler::handleMouseScroll(double x) {
    for (auto& consumer : _eventConsumers) {
        if (consumer.second->handleMouseScroll(x)) {
            return true;
        }
    }
    return false;
};

} // namespace interaction
} // namespace openspace
