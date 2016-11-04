/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2016                                                               *
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

#include <openspace/scene/scene.h>

#include <openspace/openspace.h>
#include <openspace/engine/configurationmanager.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/engine/wrapper/windowwrapper.h>
#include <openspace/interaction/interactionhandler.h>
#include <openspace/query/query.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/scripting/scriptengine.h>
#include <openspace/scripting/script_helper.h>
#include <openspace/util/time.h>
#include <openspace/util/distancetoobject.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/exception.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/onscopeexit.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/texture.h>

#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <chrono>

#ifdef OPENSPACE_MODULE_ONSCREENGUI_ENABLED
#include <modules/onscreengui/include/gui.h>
#endif

#include "scene_doc.inl"
#include "scene_lua.inl"

namespace {
    const std::string _loggerCat = "Scene";
    const std::string _moduleExtension = ".mod";
    const std::string _commonModuleToken = "${COMMON_MODULE}";
    const std::string _mostProbableSceneName = "SolarSystemBarycenter";
    const std::string _parentOfAllNodes = "Root";

    const std::string KeyCamera = "Camera";
    const std::string KeyFocusObject = "Focus";
    const std::string KeyPositionObject = "Position";
    const std::string KeyViewOffset = "Offset";

    const std::string MainTemplateFilename = "${OPENSPACE_DATA}/web/properties/main.hbs";
    const std::string PropertyOwnerTemplateFilename = "${OPENSPACE_DATA}/web/properties/propertyowner.hbs";
    const std::string PropertyTemplateFilename = "${OPENSPACE_DATA}/web/properties/property.hbs";
    const std::string HandlebarsFilename = "${OPENSPACE_DATA}/web/common/handlebars-v4.0.5.js";
    const std::string JsFilename = "${OPENSPACE_DATA}/web/properties/script.js";
    const std::string BootstrapFilename = "${OPENSPACE_DATA}/web/common/bootstrap.min.css";
    const std::string CssFilename = "${OPENSPACE_DATA}/web/common/style.css";
}

namespace openspace {

Scene::Scene() {}

Scene::~Scene() {
    deinitialize();
}

bool Scene::initialize() {
    LDEBUG("Initializing SceneGraph");   
    return true;
}

bool Scene::deinitialize() {
    clear();
    return true;
}
void Scene::writeKeyboardDocumentation(const std::string& sceneFilename) {
    // After loading the scene, the keyboard bindings have been set
    const std::string KeyboardShortcutsType =
        ConfigurationManager::KeyKeyboardShortcuts + "." +
        ConfigurationManager::PartType;

    const std::string KeyboardShortcutsFile =
        ConfigurationManager::KeyKeyboardShortcuts + "." +
        ConfigurationManager::PartFile;


    std::string type;
    std::string file;
    bool hasType = OsEng.configurationManager().getValue(KeyboardShortcutsType, type);
    bool hasFile = OsEng.configurationManager().getValue(KeyboardShortcutsFile, file);

    if (hasType && hasFile) {
        OsEng.interactionHandler().writeKeyboardDocumentation(type, file, sceneFilename);
    }
}


void Scene::update(const UpdateData& data) {
    if (!_sceneGraphToLoad.empty()) {
        clear();
        try {          
            loadSceneInternal(_sceneGraphToLoad);

            // Reset the InteractionManager to Orbital/default mode
            // TODO: Decide if it belongs in the scene and/or how it gets reloaded
            OsEng.interactionHandler().setInteractionMode("Orbital");

            writeKeybindingsDocumentation();

           
            LINFO("Loaded " << _sceneGraphToLoad);
            _sceneGraphToLoad = "";
        }
        catch (const ghoul::RuntimeError& e) {
            LERROR(e.what());
            _sceneGraphToLoad = "";
            return;
        }
    }

    try {
        _rootNode->update(data);
    } catch (const ghoul::RuntimeError& e) {
        LERRORC(e.component, e.what());
    }

}

void Scene::render(const RenderData& data, RendererTasks& tasks) {
    _rootNode->render(data, tasks);
    
}

void Scene::scheduleLoadSceneFile(const std::string& sceneDescriptionFilePath) {
    _sceneGraphToLoad = sceneDescriptionFilePath;
}

void Scene::clear() {
    LINFO("Clearing current scene graph");
    _rootNode->clearChildren();
}

bool Scene::loadSceneInternal(const std::string& sceneDescriptionFilePath) {
    ghoul::Dictionary dictionary;
    
    OsEng.windowWrapper().setSynchronization(false);
    OnExit(
        [](){ OsEng.windowWrapper().setSynchronization(true); }
    );
    
    lua_State* state = ghoul::lua::createNewLuaState();
    OnExit(
           // Delete the Lua state at the end of the scope, no matter what
           [state](){ ghoul::lua::destroyLuaState(state); }
           );
    
    OsEng.scriptEngine().initializeLuaState(state);

    ghoul::lua::loadDictionaryFromFile(
        sceneDescriptionFilePath,
        dictionary,
        state
    );

    // Perform testing against the documentation/specification
    openspace::documentation::testSpecificationAndThrow(
        Scene::Documentation(),
        dictionary,
        "Scene"
    );


    _graph.loadFromFile(sceneDescriptionFilePath);

    // Initialize all nodes
    for (SceneGraphNode* node : _graph.nodes()) {
        try {
            bool success = node->initialize();
            if (success)
                LDEBUG(node->name() << " initialized successfully!");
            else
                LWARNING(node->name() << " not initialized.");
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(_loggerCat + "(" + e.component + ")", e.what());
        }
    }

    for (auto it = _graph.nodes().rbegin(); it != _graph.nodes().rend(); ++it)
        (*it)->calculateBoundingSphere();

    // Read the camera dictionary and set the camera state
    ghoul::Dictionary cameraDictionary;
    if (dictionary.getValue(KeyCamera, cameraDictionary)) {
        OsEng.interactionHandler().setCameraStateFromDictionary(cameraDictionary);
    }





    writePropertyDocumentation(sceneDescriptionFilePath);
    writeKeyboardDocumentation(sceneDescriptionFilePath);



    OsEng.runPostInitializationScripts(sceneDescriptionFilePath);

    OsEng.enableBarrier();

    return true;
}

SceneGraphNode& Scene::root() const {
    return _graph.rootNode();
}
    
SceneGraphNode* Scene::sceneGraphNode(const std::string& name) const {
    return _graph.sceneGraphNode(name);
}

std::vector<SceneGraphNode*> Scene::allSceneGraphNodes() const {
    return _graph.nodes();
}

SceneGraph& Scene::sceneGraph() {
    return _graph;
}

const std::string & Scene::sceneName() const {
    return _sceneName;
}

void Scene::setSceneName(const std::string & sceneName) {
    _sceneName = sceneName;
}

void Scene::updateSceneName(const Camera* camera) {
    _sceneName = currentSceneName(camera, _sceneName);
}

/*
std::string Scene::currentSceneName(const Camera* camera, std::string _nameOfScene) const {
    if (camera == nullptr || _nameOfScene.empty()) {
        LERROR("Camera object not allocated or empty name scene passed to the method.");
        return _mostProbableSceneName; // This choice is controversal. Better to avoid a crash.
    }
    const SceneGraphNode* node = sceneGraphNode(_nameOfScene);

    if (node == nullptr) {
        std::stringstream ss;
        ss << "There is no scenegraph node with name: " << _nameOfScene;
        LERROR(ss.str());
        return _mostProbableSceneName;
    }

    //Starts in the last scene we kow we were in, checks if we are still inside, if not check parent, continue until we are inside a scene
    double _distance = DistanceToObject::ref().distanceCalc(camera->positionVec3(), 
        node->dynamicWorldPosition().dvec3());

    // Traverses the scenetree to find a scene we are within. 
    while (_distance > node->sceneRadius()) {
        if (node->parent() != nullptr) {
            node = node->parent();
            _distance = DistanceToObject::ref().distanceCalc(
                camera->positionVec3(),
                node->dynamicWorldPosition().dvec3());
        } else {
            break;
        }
    }

    _nameOfScene = node->name();
    std::vector<SceneGraphNode*> childrenScene(node->children());
    size_t nrOfChildren = childrenScene.size();

    //Check if we are inside a child scene of the current scene. 
    bool outsideAllChildScenes = false;

    while (!childrenScene.empty() && !outsideAllChildScenes) {
        for (size_t i = 0; i < nrOfChildren; ++i) {
            SceneGraphNode * childNode = childrenScene.at(i);
            double _childDistance      = DistanceToObject::ref().distanceCalc(camera->positionVec3(),
                childNode->dynamicWorldPosition().dvec3());            
            
            // Set the new scene that we are inside the scene radius.
            if (_childDistance < childNode->sceneRadius()) {
                node          = childNode;
                childrenScene = node->children();
                _nameOfScene  = node->name();
                nrOfChildren  = childrenScene.size();
                break;
            }

            if (nrOfChildren - 1 == i) {
                outsideAllChildScenes = true;
            }
        }
    }

    return _nameOfScene;
}
const glm::dvec3 Scene::currentDisplacementPosition(const std::string & cameraParent,
    const SceneGraphNode* target) const {
    if (target != nullptr) {

        std::vector<SceneGraphNode*> cameraPath;
        std::vector<SceneGraphNode*> targetPath;

        SceneGraphNode* cameraParentNode = sceneGraphNode(cameraParent);
        SceneGraphNode* commonParentNode;
        std::vector<SceneGraphNode*> commonParentPath;

        //Find common parent for camera and object
        std::string commonParentName(cameraParent);  // initiates to camera parent in case 
                                                     // other path is not found
        cameraPath = pathTo(cameraParentNode);
        targetPath = pathTo(sceneGraphNode(target->name()));

        commonParentNode = findCommonParentNode(cameraParent, target->name());
        commonParentPath = pathTo(commonParentNode);

        //Find the path from the camera to the common parent

        glm::dvec3 collectorCamera(pathCollector(cameraPath, commonParentNode->name(), true));
        glm::dvec3 collectorTarget(pathCollector(targetPath, commonParentNode->name(), false));

        return collectorTarget + collectorCamera;
    }
    else {
        LERROR("Target scenegraph node is null.");
        return glm::dvec3(0.0, 0.0, 0.0);
    }
}*/


void Scene::writePropertyDocumentation(const std::string& sceneFilename) {

    // If a PropertyDocumentationFile was specified, generate it now
    const std::string KeyPropertyDocumentationType =
        ConfigurationManager::KeyPropertyDocumentation + '.' +
        ConfigurationManager::PartType;

    const std::string KeyPropertyDocumentationFile =
        ConfigurationManager::KeyPropertyDocumentation + '.' +
        ConfigurationManager::PartFile;

    std::string propertyDocumentationType;
    if (!OsEng.configurationManager().getValue(KeyPropertyDocumentationType, propertyDocumentationType)) {
        return;
    }
    std::string propertyDocumentationFile;
    if (!OsEng.configurationManager().getValue(KeyPropertyDocumentationFile, propertyDocumentationFile)) {
        return;
    }

    propertyDocumentationFile = absPath(propertyDocumentationFile);
    
    LDEBUG("Writing documentation for properties");
    if (propertyDocumentationType == "text") {
        std::ofstream file;
        file.exceptions(~std::ofstream::goodbit);
        file.open(propertyDocumentationFile);

        using properties::Property;
        _rootNode->traverse([&file](auto& node) {
            std::vector<Property*> properties = node->propertiesRecursive();
            if (!properties.empty()) {
                file << node->name() << std::endl;

                for (Property* p : properties) {
                    file << p->fullyQualifiedIdentifier() << ":   " <<
                        p->guiName() << std::endl;
                }

                file << std::endl;
            }
        });
    }
    else if (propertyDocumentationType == "html") {
        std::ofstream file;
        file.exceptions(~std::ofstream::goodbit);
        file.open(propertyDocumentationFile);


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

        std::ifstream propertyOwnerTemplateInput(absPath(PropertyOwnerTemplateFilename));
        std::string propertyOwnerTemplateContent{ std::istreambuf_iterator<char>{propertyOwnerTemplateInput},
            std::istreambuf_iterator<char>{} };

        std::ifstream propertyTemplateInput(absPath(PropertyTemplateFilename));
        std::string propertyTemplateContent{ std::istreambuf_iterator<char>{propertyTemplateInput},
            std::istreambuf_iterator<char>{} };

        // Create JSON
        std::function<std::string(properties::PropertyOwner*)> createJson =
            [&createJson](properties::PropertyOwner* owner) -> std::string
        {
            std::stringstream json;
            json << "{";
            json << "\"name\": \"" << owner->name() << "\",";

            json << "\"properties\": [";
            auto properties = owner->properties();
            for (properties::Property* p : properties) {
                json << "{";
                json << "\"id\": \"" << p->identifier() << "\",";
                json << "\"type\": \"" << p->className() << "\",";
                json << "\"fullyQualifiedId\": \"" << p->fullyQualifiedIdentifier() << "\",";
                json << "\"guiName\": \"" << p->guiName() << "\"";
                json << "}";
                if (p != properties.back()) {
                    json << ",";
                }
            }
            json << "],";

            json << "\"propertyOwners\": [";
            auto propertyOwners = owner->propertySubOwners();
            for (properties::PropertyOwner* o : propertyOwners) {
                json << createJson(o);
                if (o != propertyOwners.back()) {
                    json << ",";
                }
            }
            json << "]";
            json << "}";

            return json.str();
        };


        std::stringstream json;
        json << "[";
        std::vector<SceneGraphNode*> nodes = _rootNode->allNodes();
        for (SceneGraphNode* node : nodes) {
            json << createJson(node);
            if (node != nodes.back()) {
                json << ",";
            }
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
            << "\t\t<script id=\"propertyOwnerTemplate\" type=\"text/x-handlebars-template\">\n"
            << propertyOwnerTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t\t<script id=\"propertyTemplate\" type=\"text/x-handlebars-template\">\n"
            << propertyTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t<script>\n"
            << "var propertyOwners = JSON.parse('" << jsonString << "');\n"
            << "var version = [" << OPENSPACE_VERSION_MAJOR << ", " << OPENSPACE_VERSION_MINOR << ", " << OPENSPACE_VERSION_PATCH << "];\n"
            << "var sceneFilename = '" << sceneFilename << "';\n"
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

        file << html.str();
    } else {
        LERROR("Undefined type '" << type << "' for Property documentation");
    }
}

scripting::LuaLibrary Scene::luaLibrary() {
    return {
        "",
        {
            {
                "setPropertyValue",
                &luascriptfunctions::property_setValue,
                "string, *",
                "Sets all properties identified by the URI (with potential wildcards) in "
                "the first argument. The second argument can be any type, but it has to "
                "match the type that the property (or properties) expect."
            },
            {
                "setPropertyValueRegex",
                &luascriptfunctions::property_setValueRegex,
                "string, *",
                "Sets all properties that pass the regular expression in the first "
                "argument. The second argument can be any type, but it has to match the "
                "type of the properties that matched the regular expression. The regular "
                "expression has to be of the ECMAScript grammar."
            },
            {
                "setPropertyValueSingle",
                &luascriptfunctions::property_setValueSingle,
                "string, *",
                "Sets a property identified by the URI in "
                "the first argument. The second argument can be any type, but it has to "
                "match the type that the property expects.",
            },
            {
                "getPropertyValue",
                &luascriptfunctions::property_getValue,
                "string",
                "Returns the value the property, identified by "
                "the provided URI."
            },
            {
                "loadScene",
                &luascriptfunctions::loadScene,
                "string",
                "Loads the scene found at the file passed as an "
                "argument. If a scene is already loaded, it is unloaded first"
            },
            {
                "addSceneGraphNode",
                &luascriptfunctions::addSceneGraphNode,
                "table",
                "Loads the SceneGraphNode described in the table and adds it to the "
                "SceneGraph"
            },
            {
                "removeSceneGraphNode",
                &luascriptfunctions::removeSceneGraphNode,
                "string",
                "Removes the SceneGraphNode identified by name"
            }
        }
    };
}

}  // namespace openspace
