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

#include <openspace/scene/scenegraphnode.h>
#include <openspace/documentation/documentation.h>

#include <openspace/query/query.h>
#include <openspace/util/spicemanager.h>
#include <openspace/util/time.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/logging/consolelog.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/opengl/shadermanager.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/shaderobject.h>

#include <modules/base/translation/statictranslation.h>
#include <modules/base/rotation/staticrotation.h>
#include <modules/base/scale/staticscale.h>

#include <openspace/engine/openspaceengine.h>
#include <openspace/util/factorymanager.h>
#include <openspace/util/setscene.h>

#include <cctype>
#include <chrono>
#include <memory>

#include "scenegraphnode_doc.inl"

namespace {
    const std::string _loggerCat = "SceneGraphNode";
    const std::string KeyRenderable = "Renderable";

    const std::string keyTransformTranslation = "Transform.Translation";
    const std::string keyTransformRotation = "Transform.Rotation";
    const std::string keyTransformScale = "Transform.Scale";
}

namespace openspace {

// Constants used outside of this file
const std::string SceneGraphNode::RootNodeName = "Root";
const std::string SceneGraphNode::KeyName = "Name";
const std::string SceneGraphNode::KeyParentName = "Parent";
const std::string SceneGraphNode::KeyAttachmentRadius = "SceneRadius";
const std::string SceneGraphNode::KeyDependencies = "Dependencies";

std::unique_ptr<SceneGraphNode> SceneGraphNode::createFromDictionary(const ghoul::Dictionary& dictionary){
    openspace::documentation::testSpecificationAndThrow(
        SceneGraphNode::Documentation(),
        dictionary,
        "SceneGraphNode"
    );

    std::unique_ptr<SceneGraphNode> result = std::make_unique<SceneGraphNode>();

    std::string name = dictionary.value<std::string>(KeyName);
    result->setName(name);

    double sceneRadius;
    dictionary.getValue(KeyAttachmentRadius, sceneRadius);
    result->setAttachmentRadius(sceneRadius);

    if (dictionary.hasValue<ghoul::Dictionary>(KeyRenderable)) {
        ghoul::Dictionary renderableDictionary;
        dictionary.getValue(KeyRenderable, renderableDictionary);

        renderableDictionary.setValue(KeyName, name);

        result->_renderable = Renderable::createFromDictionary(renderableDictionary);
        if (result->_renderable == nullptr) {
            LERROR("Failed to create renderable for SceneGraphNode '"
                   << result->name() << "'");
            return nullptr;
        }
        result->addPropertySubOwner(*result->_renderable);
        LDEBUG("Successfully created renderable for '" << result->name() << "'");
    }

    if (dictionary.hasKey(keyTransformTranslation)) {
        ghoul::Dictionary translationDictionary;
        dictionary.getValue(keyTransformTranslation, translationDictionary);
        result->_translation = 
            std::unique_ptr<Translation>(Translation::createFromDictionary(translationDictionary));
        if (result->_translation == nullptr) {
            LERROR("Failed to create ephemeris for SceneGraphNode '"
                << result->name() << "'");
            return nullptr;
        }
        result->addPropertySubOwner(result->_translation.get());
        LDEBUG("Successfully created ephemeris for '" << result->name() << "'");
    }

    if (dictionary.hasKey(keyTransformRotation)) {
        ghoul::Dictionary rotationDictionary;
        dictionary.getValue(keyTransformRotation, rotationDictionary);
        result->_rotation =
            std::unique_ptr<Rotation>(Rotation::createFromDictionary(rotationDictionary));
        if (result->_rotation == nullptr) {
            LERROR("Failed to create rotation for SceneGraphNode '"
                << result->name() << "'");
            return nullptr;
        }
        result->addPropertySubOwner(result->_rotation.get());
        LDEBUG("Successfully created rotation for '" << result->name() << "'");
    }

    if (dictionary.hasKey(keyTransformScale)) {
        ghoul::Dictionary scaleDictionary;
        dictionary.getValue(keyTransformScale, scaleDictionary);
        result->_scale =
            std::unique_ptr<Scale>(Scale::createFromDictionary(scaleDictionary));
        if (result->_scale == nullptr) {
            LERROR("Failed to create scale for SceneGraphNode '"
                << result->name() << "'");
            return nullptr;
        }
        result->addPropertySubOwner(result->_scale.get());
        LDEBUG("Successfully created scale for '" << result->name() << "'");
    }

    LDEBUG("Successfully created SceneGraphNode '"
                   << result->name() << "'");
    return result;
}

SceneGraphNode::SceneGraphNode()
    : _parent()
    , _translation(std::make_unique<StaticTranslation>())
    , _rotation(std::make_unique<StaticRotation>())
    , _scale(std::make_unique<StaticScale>())
    , _performanceRecord({0, 0, 0})
    , _renderable(nullptr)
    , _boundingSphereVisible(false)
    , _sceneRadius(0.0)
{
}

SceneGraphNode::~SceneGraphNode() {
    deinitialize();
}

bool SceneGraphNode::initialize() {
    if (_renderable) {
        _renderable->initialize();
    }

    if (_translation) {
        _translation->initialize();
    }

    if (_rotation) {
        _rotation->initialize();
    }
    if (_scale) {
        _scale->initialize();
    }

    return true;
}

bool SceneGraphNode::deinitialize() {
    LDEBUG("Deinitialize: " << name());

    if (_renderable) {
        _renderable->deinitialize();
        _renderable = nullptr;
    }
    _children.clear();

    // reset variables
    _boundingSphereVisible = false;
    _boundingSphere = 0;
    _sceneRadius = 0.0;

    return true;
}

void SceneGraphNode::update(const UpdateData& data) {
    if (_translation) {
        if (data.doPerformanceMeasurement) {
            glFinish();
            auto start = std::chrono::high_resolution_clock::now();

            _translation->update(data);

            glFinish();
            auto end = std::chrono::high_resolution_clock::now();
            _performanceRecord.updateTimeTranslation = (end - start).count();
        }
        else {
            _translation->update(data);
        }
    }

    if (_rotation) {
        if (data.doPerformanceMeasurement) {
            glFinish();
            auto start = std::chrono::high_resolution_clock::now();

            _rotation->update(data);

            glFinish();
            auto end = std::chrono::high_resolution_clock::now();
            _performanceRecord.updateTimeRotation = (end - start).count();
        }
        else {
            _rotation->update(data);
        }
    }

    if (_scale) {
        if (data.doPerformanceMeasurement) {
            glFinish();
            auto start = std::chrono::high_resolution_clock::now();

            _scale->update(data);

            glFinish();
            auto end = std::chrono::high_resolution_clock::now();
            _performanceRecord.updateTimeScaling = (end - start).count();
        }
        else {
            _scale->update(data);
        }
    }

    if (_renderable && _renderable->isReady()) {
        if (data.doPerformanceMeasurement) {
            glFinish();
            auto start = std::chrono::high_resolution_clock::now();
            _renderable->update(data);

            glFinish();
            auto end = std::chrono::high_resolution_clock::now();
            _performanceRecord.updateTimeRenderable = (end - start).count();
        }
        else
            _renderable->update(data);
    }
}

void SceneGraphNode::evaluate(const Camera& camera, const psc& parentPosition) {}

void SceneGraphNode::render(const RenderData& data, RendererTasks& tasks) {
    RenderData newData = {
        data.camera,
        *this,
        data.doPerformanceMeasurement,
        data.renderBinMask,
    };

    bool visible = _renderable != nullptr &&
        _renderable->isVisible() &&
        _renderable->isReady() &&
        _renderable->isEnabled() &&
        _renderable->matchesRenderBinMask(data.renderBinMask);

    if (visible) {
        if (data.doPerformanceMeasurement) {
            glFinish();
            auto start = std::chrono::high_resolution_clock::now();

            _renderable->render(newData, tasks);

            glFinish();
            auto end = std::chrono::high_resolution_clock::now();
            _performanceRecord.renderTime = (end - start).count();
        }
        else
            _renderable->render(newData, tasks);
    }
}

void SceneGraphNode::setParent(SceneGraphNode& parent) {
    _parent = &parent;
}

void SceneGraphNode::addChild(SceneGraphNode& child) {
    _children.push_back(&child);
}

void SceneGraphNode::setAttachmentRadius(double sceneRadius) {
    _attachmentRadius = std::move(sceneRadius);
}

glm::dvec3 SceneGraphNode::translation() const
{
    return _translation->position();
}

const glm::mat3& SceneGraphNode::rotation() const
{
    return _rotation->matrix();
}

const glm::mat3& SceneGraphNode::inverseRotation() const
{
    return _rotation->matrix();
}


double SceneGraphNode::scale() const
{
    return _scale->scaleValue();
}

int SceneGraphNode::depth() const {
    int d = 0;
    for (const SceneGraphNode* current = this; current != nullptr; current = &current->parent(), d++);
    return d;
}

TransformData SceneGraphNode::relativeTransform(const SceneGraphNode& observer) const {
    glm::vec3 thisTranslation = glm::vec3(0);
    glm::mat3 thisRotation = glm::mat3(1.0);
    float thisScale = 1.0;

    glm::vec3 observerTranslation = glm::vec3(0);
    glm::mat3 observerRotation = glm::mat3(1.0);
    float observerScale = 1.0;

    int thisDepth = depth();
    int observerDepth = observer.depth();

    const SceneGraphNode* thisAncestor = this;
    const SceneGraphNode* observerAncestor = &observer;

    if (thisDepth - observerDepth > 0) {
        // This node is deeper down in scene graph than the reference node
        for (int i = 0; i < thisDepth - observerDepth; i++) {
            thisTranslation = thisAncestor->rotation() * thisTranslation;
            thisTranslation *= thisAncestor->scale();
            thisTranslation += thisAncestor->translation();

            thisScale *= thisAncestor->scale();
            thisRotation *= thisAncestor->rotation();

            thisAncestor = &thisAncestor->parent();
        }
    } else {
        // The reference node is deeper down in scene graph than this node
        for (int i = 0; i < observerDepth - thisDepth; i++) {
            observerRotation *= observerAncestor->inverseRotation();
            observerScale /= observerAncestor->scale();

            observerTranslation -= observerAncestor->translation();
            observerTranslation /= observerAncestor->scale();
            observerTranslation = observerAncestor->inverseRotation() * observerTranslation;

            observerAncestor = &observerAncestor->parent();
        }
    }

    while (thisAncestor != observerAncestor) {
        thisTranslation = thisAncestor->rotation() * thisTranslation;
        thisTranslation *= thisAncestor->scale();
        thisTranslation += thisAncestor->translation();

        thisScale *= thisAncestor->scale();
        thisRotation *= thisAncestor->rotation();

        observerRotation *= observerAncestor->inverseRotation();
        observerScale /= observerAncestor->scale();

        observerTranslation -= observerAncestor->translation();
        observerTranslation /= observerAncestor->scale();
        observerTranslation = observerAncestor->inverseRotation() * observerTranslation;

        thisAncestor = &thisAncestor->parent();
        observerAncestor = &observerAncestor->parent();
    }

    TransformData td;
    td.translation = thisTranslation + observerTranslation;
    td.rotation = thisRotation * observerRotation;
    td.scale = thisScale * observerScale;
    return td;
}



SceneGraphNode& SceneGraphNode::parent() const {
    if (_parent == nullptr) {
        throw SceneGraph::SceneGraphError("The root node has no parent");
    }
    return *_parent;
}

const std::vector<SceneGraphNode*>& SceneGraphNode::children() const{
    return _children;
}

// bounding sphere
float SceneGraphNode::calculateBoundingSphere(){
    // set the bounding sphere to 0.0
    _boundingSphere = 0.0;
    /*
    This is not how to calculate a bounding sphere, better to leave it at 0 if not a
    renderable. --KB
    if (!_children.empty()) {  // node
        PowerScaledScalar maxChild;

        // loop though all children and find the one furthest away/with the largest
        // bounding sphere
        for (size_t i = 0; i < _children.size(); ++i) {
            // when positions is dynamic, change this part to fins the most distant
            // position
            //PowerScaledScalar child = _children.at(i)->position().length()
            //            + _children.at(i)->calculateBoundingSphere();
            PowerScaledScalar child = _children.at(i)->calculateBoundingSphere();
            if (child > maxChild) {
                maxChild = child;
            }
        }
        _boundingSphere += maxChild;
    } 
    */
    // if has a renderable, use that boundingsphere
    if (_renderable ) {
        float renderableBS = _renderable->getBoundingSphere();
        if(renderableBS > _boundingSphere)
            _boundingSphere = renderableBS;
    }
    //LINFO("Bounding Sphere of '" << name() << "': " << _boundingSphere);
    
    return _boundingSphere;
}

float SceneGraphNode::boundingSphere() const{
    return _boundingSphere;
}

void SceneGraphNode::setRenderable(std::unique_ptr<Renderable> renderable) {
    _renderable = std::move(renderable);
}

const Renderable& SceneGraphNode::renderable() const {
    return *_renderable;
}

Renderable& SceneGraphNode::renderable() {
    return *_renderable;
}

std::shared_ptr<SceneGraphNode> SceneGraphNode::find(const std::string& name) {
    if (this->name() == name)
        return std::shared_ptr(this);
    else
        for (SceneGraphNode* it : _children) {
            SceneGraphNode* tmp = it->find(name);
            if (tmp != nullptr)
                return tmp;
        }
    return nullptr;
}


const double& SceneGraphNode::sceneRadius() const {
    return _sceneRadius;
}

}  // namespace openspace
