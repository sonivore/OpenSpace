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

#ifndef __SETSCENE_H__
#define __SETSCENE_H__

#include <openspace/util/powerscaledcoordinate.h>
#include <openspace/rendering/renderengine.h> 
#include <openspace/util/distancetoobject.h>
#include <openspace/scene/scenegraphnode.h>
#include <glm/gtc/type_ptr.hpp>
#include <openspace/scene/scene.h>

#include <iostream>

namespace openspace {
    
    //std::string setScene(Scene* scene, Camera* camera, std::string _nameOfScene);

    ///**
    //* Calculates the world position of target from the common node between camera's parent and target.
    //*/
    //const glm::dvec3 vectorPosition(const std::string & cameraParent, const SceneGraphNode* target, const Scene* scene);

    //const glm::mat4 setNewViewMatrix(const std::string & cameraParent, SceneGraphNode* target, Scene*);

    //glm::vec3 pathCollector(const std::vector<SceneGraphNode*> & path, const std::string & commonParent, const bool inverse);

    //void setRelativeOrigin(Camera* camera, Scene* scene);

    //void newCameraOrigin(std::vector<SceneGraphNode*> commonParentPath, std::string commonParent, Camera* camera, Scene* scene);

    //glm::vec3 Vec3Subtract(glm::vec3 vec1, glm::vec3 vec2);

    //std::string commonParent(std::vector<SceneGraphNode*> t1, std::vector<SceneGraphNode*> t2);

    //SceneGraphNode* findCommonParentNode(const std::string & firstPath, const std::string & secondPath, const Scene* scene);

    //std::vector<SceneGraphNode*> pathTo(SceneGraphNode* node);
}
#endif