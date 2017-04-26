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

#include <openspace/interaction/keybindingmanager.h>
#include <openspace/openspace.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/util/time.h>

#include <ghoul/filesystem/filesystem.h>

#include <sstream>
#include <fstream>
#include <iterator>

namespace {
    const char* _loggerCat = "KeyBindingManager";

    const char* MainTemplateFilename = "${OPENSPACE_DATA}/web/keybindings/main.hbs";
    const char* KeybindingTemplateFilename = "${OPENSPACE_DATA}/web/keybindings/keybinding.hbs";
    const char* HandlebarsFilename = "${OPENSPACE_DATA}/web/common/handlebars-v4.0.5.js";
    const char* JsFilename = "${OPENSPACE_DATA}/web/keybindings/script.js";
    const char* BootstrapFilename = "${OPENSPACE_DATA}/web/common/bootstrap.min.css";
    const char* CssFilename = "${OPENSPACE_DATA}/web/common/style.css";
} // namespace

#include "keybindingmanager_lua.inl"

namespace openspace {
namespace interaction {

void KeyBindingManager::resetKeyBindings() {
    _keyLua.clear();
}

void KeyBindingManager::bindKeyLocal(Key key, KeyModifier modifier,
    std::string luaCommand, std::string documentation)
{
    _keyLua.insert({
        { key, modifier },
        { std::move(luaCommand), Synchronized::No, std::move(documentation) }
    });
}

void KeyBindingManager::bindKey(Key key, KeyModifier modifier,
    std::string luaCommand, std::string documentation)
{
    _keyLua.insert({
        { key, modifier },
        { std::move(luaCommand), Synchronized::Yes, std::move(documentation) }
    });
}

bool KeyBindingManager::handleKeyboard(Key key, KeyModifier modifier, KeyAction action) {
    bool consumed = false;
    if (action == KeyAction::Press || action == KeyAction::Repeat) {
        // iterate over key bindings
        auto ret = _keyLua.equal_range({ key, modifier });
        for (auto it = ret.first; it != ret.second; ++it) {
            auto remote = it->second.synchronization ?
                scripting::ScriptEngine::RemoteScripting::Yes :
                scripting::ScriptEngine::RemoteScripting::No;

            OsEng.scriptEngine().queueScript(it->second.command, remote);
            consumed = true;
        }
    }
    return consumed;
}


void KeyBindingManager::writeKeyboardDocumentation(const std::string& type,
    const std::string& file)
{
    if (type == "text") {
        std::ofstream f;
        f.exceptions(~std::ofstream::goodbit);
        f.open(absPath(file));

        for (const auto& p : _keyLua) {
            std::string remoteScriptingInfo;
            bool remoteScripting = p.second.synchronization;

            if (!remoteScripting) {
                remoteScriptingInfo = " (LOCAL)";
            }
            f << std::to_string(p.first) << ": "
                << p.second.command << remoteScriptingInfo << '\n'
                << p.second.documentation << '\n';
        }
    }
    else if (type == "html") {
        std::ofstream f;
        f.exceptions(~std::ofstream::goodbit);
        f.open(absPath(file));

        std::ifstream handlebarsInput(absPath(HandlebarsFilename));
        std::ifstream jsInput(absPath(JsFilename));

        std::string jsContent;
        std::back_insert_iterator<std::string> jsInserter(jsContent);

        std::copy(std::istreambuf_iterator<char>{handlebarsInput}, std::istreambuf_iterator<char>(), jsInserter);
        std::copy(std::istreambuf_iterator<char>{jsInput}, std::istreambuf_iterator<char>(), jsInserter);

        std::ifstream bootstrapInput(absPath(BootstrapFilename));
        std::ifstream cssInput(absPath(CssFilename));

        std::string cssContent;
        std::back_insert_iterator<std::string> cssInserter(cssContent);

        std::copy(std::istreambuf_iterator<char>{bootstrapInput}, std::istreambuf_iterator<char>(), cssInserter);
        std::copy(std::istreambuf_iterator<char>{cssInput}, std::istreambuf_iterator<char>(), cssInserter);

        std::ifstream mainTemplateInput(absPath(MainTemplateFilename));
        std::string mainTemplateContent{ std::istreambuf_iterator<char>{mainTemplateInput},
            std::istreambuf_iterator<char>{} };

        std::ifstream keybindingTemplateInput(absPath(KeybindingTemplateFilename));
        std::string keybindingTemplateContent{ std::istreambuf_iterator<char>{keybindingTemplateInput},
            std::istreambuf_iterator<char>{} };

        std::stringstream json;
        json << "[";
        bool first = true;
        for (const auto& p : _keyLua) {
            if (!first) {
                json << ",";
            }
            first = false;
            json << "{";
            json << "\"key\": \"" << std::to_string(p.first) << "\",";
            json << "\"script\": \"" << p.second.command << "\",";
            json << "\"remoteScripting\": " << (p.second.synchronization ? "true," : "false,");
            json << "\"documentation\": \"" << p.second.documentation << "\"";
            json << "}";
        }
        json << "]";

        std::string jsonString = "";
        for (const char& c : json.str()) {
            if (c == '\'') {
                jsonString += "\\'";
            }
            else {
                jsonString += c;
            }
        }


        std::stringstream html;
        html << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "\t<head>\n"
            << "\t\t<script id=\"mainTemplate\" type=\"text/x-handlebars-template\">\n"
            << mainTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t\t<script id=\"keybindingTemplate\" type=\"text/x-handlebars-template\">\n"
            << keybindingTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t<script>\n"
            << "var keybindings = JSON.parse('" << jsonString << "');\n"
            << "var version = [" << OPENSPACE_VERSION_MAJOR << ", " << OPENSPACE_VERSION_MINOR << ", " << OPENSPACE_VERSION_PATCH << "];\n"
            << "var generationTime = '" << Time::now().ISO8601() << "';\n"
            << jsContent << "\n"
            << "\t</script>\n"
            << "\t<style type=\"text/css\">\n"
            << cssContent << "\n"
            << "\t</style>\n"
            << "\t\t<title>Documentation</title>\n"
            << "\t</head>\n"
            << "\t<body>\n"
            << "\t<body>\n"
            << "</html>\n";

        f << html.str();
    }
    else {
        throw ghoul::RuntimeError(
            "Unsupported keyboard documentation type '" + type + "'",
            "InteractionHandler"
            );
    }
}

scripting::LuaLibrary KeyBindingManager::luaLibrary() {
    return{
        "",
        {
            {
                "clearKeys",
                &luascriptfunctions::clearKeys,
                "",
                "Clear all key bindings"
            },
            {
                "bindKey",
                &luascriptfunctions::bindKey,
                "string, string [,string]",
                "Binds a key by name to a lua string command to execute both locally "
                "and to broadcast to clients if this is the host of a parallel session. "
                "The first argument is the key, the second argument is the Lua command "
                "that is to be executed, and the optional third argument is a human "
                "readable description of the command for documentation purposes."
            },
            {
                "bindKeyLocal",
                &luascriptfunctions::bindKeyLocal,
                "string, string [,string]",
                "Binds a key by name to a lua string command to execute only locally. "
                "The first argument is the key, the second argument is the Lua command "
                "that is to be executed, and the optional third argument is a human "
                "readable description of the command for documentation purposes."
            }
        }
    };
}

} // namespace interaction
} // namespace openspace
