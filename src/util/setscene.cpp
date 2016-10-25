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

#include <openspace/util/setscene.h>
#include <openspace/scene/scene.h>

namespace {
    const std::string _loggerCat = "Set Scene";
}


namespace openspace {
    /*
    // finds the current parent for the camera and sets returns the parent as a string. 
    std::string setScene(Scene* scene, Camera* camera, std::string _nameOfScene) 
    {        
        //psc cameraPos = camera->position();
        //glm::vec3 cp = scene->sceneGraphNode(camera->getParent())->worldPosition().vec3() + camera->getDisplacementVector();
        //psc cameraPos = PowerScaledCoordinate::CreatePowerScaledCoordinate(cp.x, cp.y, cp.z);
        //sets the current node to the the node with the previous name. 
        SceneGraphNode* node = scene->sceneGraphNode(_nameOfScene);

        //Starts in the last scene we kow we were in, checks if we are still inside, if not check parent, continue until we are inside a scene
        double _distance = (DistanceToObject::ref().distanceCalc(camera->position(), node->worldPosition()));

        std::vector<SceneGraphNode*> childrenScene(node->children());
        size_t nrOfChildren = childrenScene.size();
        
        // Traverses the scenetree to find a scene we are within. 
        while (_distance > node->sceneRadius()) 
        {
            if (node->parent() != nullptr) 
            {
                node          = node->parent();
                _nameOfScene  = node->name();
                childrenScene = node->children();
                nrOfChildren  = childrenScene.size();
                _distance     = (DistanceToObject::ref().distanceCalc(camera->position(), node->worldPosition()));
                break;
            }            
            else if (node->parent() == nullptr) 
            {
                // We have reached the root. 
                //node = scene->allSceneGraphNodes()[0];
                _nameOfScene  = node->name();
                childrenScene = node->children();
                nrOfChildren  = childrenScene.size();
                break;
            }
        }

        //Check if we are inside a child scene of the current scene. 
        bool outsideAllChildScenes = false;

        while (!childrenScene.empty() && !outsideAllChildScenes) 
        {
            for (size_t i = 0; i < nrOfChildren; ++i) 
            {
                double _childDistance = DistanceToObject::ref().distanceCalc(camera->position(),
                    childrenScene.at(i)->dynamicWorldPosition(*camera, childrenScene.at(i), scene));
                double childSceneRadius = childrenScene.at(i)->sceneRadius();
                
                // Set the new scene that we are inside the scene radius.
                if (_childDistance < childSceneRadius ) 
                {
                    node          = childrenScene.at(i);
                    childrenScene = node->children();
                    _nameOfScene  = node->name();
                    nrOfChildren  = childrenScene.size();
                    break;
                }

                if (nrOfChildren - 1 == i) 
                {
                    outsideAllChildScenes = true;
                }
            }
        }

        return _nameOfScene;
    }

    const glm::dvec3 vectorPosition(const std::string & cameraParent, const SceneGraphNode* target, const Scene* scene)
    {
        if (target != nullptr) {
            std::string targetName(target->name());

            std::vector<SceneGraphNode*> cameraPath;
            std::vector<SceneGraphNode*> targetPath;

            SceneGraphNode* cameraParentNode = scene->sceneGraphNode(cameraParent);
            SceneGraphNode* commonParentNode;
            std::vector<SceneGraphNode*> commonParentPath;

            //Find common parent for camera and object
            std::string commonParentName(cameraParent);  // initiates to camera parent in case 
                                                         // other path is not found
            cameraPath = pathTo(cameraParentNode);
            targetPath = pathTo(scene->sceneGraphNode(targetName));
            //targetPath = pathTo(target);

            commonParentNode = findCommonParentNode(cameraParent, targetName, scene);
            commonParentName = commonParentNode->name();
            commonParentPath = pathTo(commonParentNode);

            //Find the path from the camera to the common parent

            glm::dvec3 collectorCamera(pathCollector(cameraPath, commonParentName, true));
            glm::dvec3 collectorTarget(pathCollector(targetPath, commonParentName, false));

            return collectorTarget + collectorCamera;
        }
        else
            return glm::dvec3(0.0, 0.0, 0.0);

    }

    const glm::mat4 setNewViewMatrix(const std::string & cameraParent, SceneGraphNode* target, Scene* scene)
    {
        //collect the positions for all parents in, get psc.vec3.
        // add them all up then use glm::translate to make it a mat 4. 

        //sceneGraphNode(camera->getParent())
        //Scene scene = Scene::Scene();
        psc pscViewMatrix;

        // JCC:
        // Why is collector initialized with (1,1,1)? Shouldn't it be (0,0,0)?
        glm::vec3 collectorCamera(1.f, 1.f, 1.f);
        glm::vec3 collectorTarget(1.f, 1.f, 1.f);

        glm::mat4 newViewMatrix;
        //glm::mat4 viewMatrix = camera->viewMatrix();;
        std::string targetName(target->name());
        
        std::vector<SceneGraphNode*> cameraPath;
        std::vector<SceneGraphNode*> targetPath;

        SceneGraphNode* cameraParentNode = scene->sceneGraphNode(cameraParent);
        SceneGraphNode* commonParentNode;
        std::vector<SceneGraphNode*> commonParentPath;
        //	std::vector<SceneGraphNode*> allnodes = scene->allSceneGraphNodes();

        //Scene scene = RenderEngine::scene()

        //Find common parent for camera and object
        std::string commonParentName(cameraParent);  // initiates to camera parent in case 
                                                     // other path is not found
        cameraPath = pathTo(cameraParentNode);
        targetPath = pathTo(scene->sceneGraphNode(targetName));

        commonParentNode = findCommonParentNode(cameraParent, targetName, scene);
        commonParentName = commonParentNode->name();
        commonParentPath = pathTo(commonParentNode);

        //Find the path from the camera to the common parent

        collectorCamera = pathCollector(cameraPath, commonParentName, true);
        collectorTarget = pathCollector(targetPath, commonParentName, false);
        glm::vec3 translationCollector;

        translationCollector = collectorTarget + collectorCamera;
        newViewMatrix = glm::translate(newViewMatrix, translationCollector);


        // JCC: This is not the view matrix. It must be built again with
        // the new translation.

        return glm::transpose(newViewMatrix);
    }

    bool hasChild(std::string name, SceneGraphNode* node) 
    {
        std::vector<SceneGraphNode*> children = node->children();

        for (int i = 0; i < children.size(); i++) {
            if (children[i]->name() == name)
                return true;
        }
        return false;
    }

    void setRelativeOrigin(Camera* camera, Scene* scene) 
    {
        std::string target = camera->getParent();

        //For clarity
        SceneGraphNode* commonParentNode = scene->sceneGraphNode(target);
        std::vector<SceneGraphNode*> commonParentPath = pathTo(commonParentNode);

        newCameraOrigin(commonParentPath, target, camera, scene);

        //return newCameraOrigin(commonParentPath, commonParentName, camera, scene);
    }

    //ToDo remove commonParent string, its in commonParentPath[0]->name

    void newCameraOrigin(std::vector<SceneGraphNode*> commonParentPath, std::string commonParent, Camera* camera, Scene* scene) 
    {      
        glm::vec3 displacementVector = camera->position().vec3();
        if (commonParentPath.size()>1) { // <1 if in root system. 
            glm::vec3 tempDvector;
            for (int i = commonParentPath.size() - 1; i >= 1; i--) {
                tempDvector = commonParentPath[i - 1]->worldPosition().vec3() - commonParentPath[i]->worldPosition().vec3();

                //displacementVector = displacementVector - tempDvector;

                displacementVector = displacementVector - tempDvector;
            }
        }

        //Move the camera to the position of the common parent (The new origin)
        //Then add the distance from the displacementVector to get the camera into correct position
        glm::vec3 origin = commonParentPath[0]->position().vec3();

        glm::vec3 newOrigin;
        camera->setDisplacementVector(displacementVector);

        
        //newOrigin[0] = (origin[0] + displacementVector[0]);
        //newOrigin[1] = (origin[1] + displacementVector[1]);
        //newOrigin[2] = (origin[2] + displacementVector[2]);
        
        newOrigin = origin + displacementVector;
        camera->setPosition(newOrigin);
        //return(newOrigin);
    }

    //not needed(?)
    glm::vec3 Vec3Subtract(glm::vec3 vec1, glm::vec3 vec2) 
    {
        for (int i = 0; i<3; i++) {
            vec1[i] = vec1[i] - vec2[i];
        }

        return vec1;
    }


    glm::vec3 pathCollector(const std::vector<SceneGraphNode*> & path, const std::string & commonParentName, const bool inverse)
    {
        SceneGraphNode* firstElement = path.front();
        glm::vec3 collector(path.back()->position().vec3());

        //
        //for (std::vector<int>::iterator it = myvector.begin(); *it != commonParent; ++it)
        //std::cout << ' ' << *it.;
        //

        int depth = 0;
        // adds all elements to the collector, continues untill commomParent is found.
        while (firstElement->name() != commonParentName)
        {

            //std::cout << "Node name: " << firstElement << "  Common parent: " << commonParent << "\n";


            //collector = collector+firstElement->position().vec3();
            // if the line above does not work

            //for (int j = 0; j <= 2; j++) {

            //    if (inverse)
            //        collector[j] = collector[j] - firstElement->position().vec3()[j];
            //    else
            //        collector[j] = collector[j] + firstElement->position().vec3()[j];

            //}

            if (inverse)
                collector = collector - firstElement->position().vec3();
            else
                collector = collector + firstElement->position().vec3();

            //std::cout << "Node Parent name: " << firstElement->parent()->name() << "\n";
            //firstElement = path[depth];
            //depth++;

            // JCC: Previous version wrong?
            firstElement = path[++depth];
        }

        return collector;
    }

    std::string commonParent(std::vector<SceneGraphNode*> t1, std::vector<SceneGraphNode*> t2) //Camera* camera, std::string nameOftarget, Scene* scene){
    {
        std::string commonParentReturn = "SolarSystem";
        
        //std::vector<SceneGraphNode*> t1 = pathTo(target, scene);
        //std::vector<SceneGraphNode*> t2 = pathTo(nameOftarget, scene);
        

        //traverse the list backwards, position before first difference = common parent.
        //int t1t = t1.size();
        //int t2t = t2.size();
        //traverses the lists 
        int iterator = 0;
        int min = std::min(t1.size(), t2.size());
        while (iterator < min && t1.back()->name() == t2.back()->name()) {
            commonParentReturn = t1.back()->name();
            t1.pop_back();
            t2.pop_back();
            iterator++;
        }
        
        //int counter = std::min(t1.size(), t2.size());
        //for (int i = 0; i < counter-1; i++){

        //if (t1.back()->name() == t1.back()->name()){
        //commonParentReturn = t1.back()->name();


        //}


        //}
        
        //while (t1.back() != NULL && t2.back() != NULL && t1.back() == t2.back()) // && !t1.empty && !t2.empty
        //{

        //commonParentReturn = t1.back()->name().c_str();
        //;
        //t2.pop_back();

        //}
        
        //std::cout << "commonParentReturn" << commonParentReturn << "\n";
        return commonParentReturn;
    }

    //help function for findCommonParent
    std::vector<SceneGraphNode*> pathTo(SceneGraphNode* node)//std::string nameOfScene, Scene* scene)
    {
        std::vector<SceneGraphNode*> path;
        //	path.push_back(node->parent());
        //std::vector<SceneGraphNode*> childrenScene = node->children();
        std::string name = node->name();
        while (node->parent() != nullptr) {
        //while (name != "Root") {
            path.push_back(node);
            node = node->parent();
            name = node->name();
        }
        path.push_back(node);
        return path;
    }

    SceneGraphNode* findCommonParentNode(const std::string & firstPath, const std::string & secondPath, const Scene* scene) 
    {
        //SceneGraphNode* node = ;
        std::vector<SceneGraphNode*> firstPathNode  = pathTo( scene->sceneGraphNode(firstPath) );
        std::vector<SceneGraphNode*> secondPathNode = pathTo( scene->sceneGraphNode(secondPath) );
        std::string strCommonParent = commonParent(firstPathNode, secondPathNode);

        return scene->sceneGraphNode(strCommonParent);
    }
    */
}