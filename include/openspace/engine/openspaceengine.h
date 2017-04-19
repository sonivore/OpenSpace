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

#ifndef __OPENSPACE_CORE___OPENSPACEENGINE___H__
#define __OPENSPACE_CORE___OPENSPACEENGINE___H__

#include <openspace/util/keys.h>
#include <openspace/util/mouse.h>

#include <ghoul/glm.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ghoul {
    
class Dictionary;

namespace cmdparser { class CommandlineParser; }
namespace fontrendering { class FontManager; }

} // namespace ghoul

namespace openspace {

class ConfigurationManager;
class DownloadManager;
class GUI;
class LuaConsole;
class ModuleEngine;
class NetworkEngine;
class ParallelConnection;
class RenderEngine;
class SettingsEngine;
class SceneManager;
class VirtualPropertyManager;

class SyncEngine;
class TimeManager;
class WindowWrapper;

namespace interaction {
    class Navigator;
    class KeyboardMouseInteractionHandler;
    class KeyboardMouseState;
    class KeyBindingManager;
}
namespace gui { class GUI; }
namespace properties { class PropertyOwner; }
namespace scripting {
    struct LuaLibrary;
    class ScriptEngine;
    class ScriptScheduler;
} // namespace scripting

class OpenSpaceEngine {
public:
    static void create(int argc, char** argv,
        std::unique_ptr<WindowWrapper> windowWrapper,
        std::vector<std::string>& sgctArguments, bool& requestClose);
    static void destroy();
    static OpenSpaceEngine& ref();

    double runTime();
    void setRunTime(double t);

    // callbacks
    void initialize();
    void initializeGL();
    void deinitialize();
    void preSynchronization();
    void postSynchronizationPreDraw();
    void draw(const glm::mat4& sceneMatrix, const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix);
    void postDraw();
    void keyboardCallback(Key key, KeyModifier mod, KeyAction action);
    void charCallback(unsigned int codepoint, KeyModifier mod);
    void mouseButtonCallback(MouseButton button, MouseAction action);
    void mousePositionCallback(double x, double y);
    void mouseScrollWheelCallback(double pos);
    void externalControlCallback(const char* receivedChars, int size, int clientId);
    void encode();
    void decode();

    void scheduleLoadScene(std::string scenePath);

    void enableBarrier();
    void disableBarrier();
    
    void writeDocumentation();
    void toggleShutdownMode();
    
    void runPostInitializationScripts(const std::string& sceneDescription);

    // Guaranteed to return a valid pointer
    ConfigurationManager& configurationManager();
    LuaConsole& console();
    DownloadManager& downloadManager();
    ModuleEngine& moduleEngine();
    NetworkEngine& networkEngine();
    ParallelConnection& parallelConnection();
    RenderEngine& renderEngine();
    SettingsEngine& settingsEngine();
    TimeManager& timeManager();
    WindowWrapper& windowWrapper();
    ghoul::fontrendering::FontManager& fontManager();
    interaction::Navigator& navigator();
    interaction::KeyboardMouseInteractionHandler& keyboardMouseInteractionHandler();
    interaction::KeyBindingManager& keyBindingManager();
    interaction::KeyboardMouseState& keyboardMouseState();
    properties::PropertyOwner& globalPropertyOwner();
    scripting::ScriptEngine& scriptEngine();
    scripting::ScriptScheduler& scriptScheduler();
    VirtualPropertyManager& virtualPropertyManager();

    /**
     * Returns the Lua library that contains all Lua functions available to affect the
     * application.
     */
    static scripting::LuaLibrary luaLibrary();

private:
    OpenSpaceEngine(std::string programName,
        std::unique_ptr<WindowWrapper> windowWrapper);

    void loadScene(const std::string& scenePath);
    void gatherCommandlineArguments();
    void loadFonts();
    void runPreInitializationScripts(const std::string& sceneDescription);
    void configureLogging();
    
    // Components
    std::unique_ptr<ConfigurationManager> _configurationManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<DownloadManager> _downloadManager;
    std::unique_ptr<LuaConsole> _console;
    std::unique_ptr<ModuleEngine> _moduleEngine;
    std::unique_ptr<NetworkEngine> _networkEngine;
    std::unique_ptr<ParallelConnection> _parallelConnection;
    std::unique_ptr<RenderEngine> _renderEngine;
    std::unique_ptr<SettingsEngine> _settingsEngine;
    std::unique_ptr<SyncEngine> _syncEngine;
    std::unique_ptr<TimeManager> _timeManager;
    std::unique_ptr<WindowWrapper> _windowWrapper;
    std::unique_ptr<ghoul::cmdparser::CommandlineParser> _commandlineParser;
    std::unique_ptr<ghoul::fontrendering::FontManager> _fontManager;
    std::unique_ptr<interaction::Navigator> _navigator;
    std::unique_ptr<interaction::KeyboardMouseInteractionHandler> _keyboardMouseInteractionHandler;
    std::unique_ptr<interaction::KeyBindingManager> _keyBindingManager;
    std::unique_ptr<interaction::KeyboardMouseState> _keyboardMouseState;
    std::unique_ptr<scripting::ScriptEngine> _scriptEngine;
    std::unique_ptr<scripting::ScriptScheduler> _scriptScheduler;
    std::unique_ptr<VirtualPropertyManager> _virtualPropertyManager;

    // Others
    std::unique_ptr<properties::PropertyOwner> _globalPropertyNamespace;
    
    bool _scheduledSceneSwitch;
    std::string _scenePath;

    double _runTime;

    // Structure that is responsible for the delayed shutdown of the application
    struct {
        // Whether the application is currently in shutdown mode (i.e. counting down the
        // timer and closing it at '0'
        bool inShutdown;
        // Total amount of time the application will wait before actually shutting down
        float waitTime;
        // Current state of the countdown; if it reaches '0', the application will
        // close
        float timer;
    } _shutdown;

    // The first frame might take some more time in the update loop, so we need to know to
    // disable the synchronization; otherwise a hardware sync will kill us after 1 minute
    bool _isFirstRenderingFirstFrame;

    static OpenSpaceEngine* _engine;
};

#define OsEng (openspace::OpenSpaceEngine::ref())

} // namespace openspace

#endif // __OPENSPACE_CORE___OPENSPACEENGINE___H__
