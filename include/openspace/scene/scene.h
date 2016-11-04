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
 
#ifndef __SCENE_H__
#define __SCENE_H__

// std includes
#include <vector>
#include <map>
#include <set>
#include <mutex>

#include <openspace/documentation/documentation.h>

#include <openspace/util/camera.h>
#include <openspace/util/updatestructures.h>
#include <openspace/scripting/scriptengine.h>
#include <openspace/scene/scenegraph.h>

#include <ghoul/opengl/programobject.h>
#include <ghoul/misc/dictionary.h>

namespace openspace {

class SceneGraphNode;

// Notifications:
// SceneGraphFinishedLoading
class Scene {
public:
    // constructors & destructor
    Scene();
    ~Scene();

    /**
     * Initalizes the Scene
     */
    bool initialize();

    /*
     * Clean up everything
     */
    bool deinitialize();

    /*
     * Load the scenegraph from the provided folder
     */
    void scheduleLoadSceneFile(const std::string& sceneDescriptionFilePath);

    /**
     * Clear Scene graph
     */
    void clear();

    /*
     * Update all SceneGraphNodes relative positions
     */
    void update(const UpdateData& data);

    /*
     * Evaluate if the SceneGraphNodes are visible to the provided camera
     */
    void evaluate(Camera* camera);

    /*
     * Render visible SceneGraphNodes using the provided camera
     */
    void render(const RenderData& data, RendererTasks& tasks);

    /*
     * Returns the root SceneGraphNode
     */
    SceneGraphNode& root() const;

    /**
     * Return the scenegraph node with the specified name or <code>nullptr</code> if that
     * name does not exist
     */
    SceneGraphNode* sceneGraphNode(const std::string& name) const;

    /**
     * Return all scene graph nodes
     */
    std::vector<SceneGraphNode*> allSceneGraphNodes() const;

    /**
     * Returns the Lua library that contains all Lua functions available to change the
     * scene graph. The functions contained are
     * - openspace::luascriptfunctions::property_setValue
     * - openspace::luascriptfunctions::property_getValue
     * \return The Lua library that contains all Lua functions available to change the
     * scene graph
     */
    static scripting::LuaLibrary luaLibrary();

    /**
     * Return the documentaiton of the scene
     */
    static documentation::Documentation Documentation();

private:
    bool loadSceneInternal(const std::string& sceneDescriptionFilePath);
    void writeKeyboardDocumentation(const std::string& sceneFilename);
    void writePropertyDocumentation(const std::string& sceneFilename);

    std::string _sceneGraphToLoad;
    std::unique_ptr<SceneGraphNode> _rootNode;
};

} // namespace openspace

#endif // __SCENE_H__
