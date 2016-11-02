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
 
#ifndef __SCENEGRAPHNODE_H__
#define __SCENEGRAPHNODE_H__

// open space includes
#include <openspace/documentation/documentation.h>

#include <openspace/rendering/renderable.h>
#include <openspace/scene/translation.h>
#include <openspace/scene/rotation.h>
#include <openspace/scene/scale.h>
#include <openspace/properties/propertyowner.h>

#include <openspace/scene/scene.h>
#include <ghoul/misc/dictionary.h>
#include <openspace/util/updatestructures.h>

// std includes
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace openspace {

class SceneGraphNode : public properties::PropertyOwner {
public:
    struct PerformanceRecord {
        long long renderTime;  // time in ns
        long long updateTimeRenderable;  // time in ns
        long long updateTimeTranslation; // time in ns
        long long updateTimeRotation;  // time in ns
        long long updateTimeScaling;  // time in ns
    };

    static const std::string RootNodeName;
    static const std::string KeyName;
    static const std::string KeyParentName;
    static const std::string KeyDependencies;
    static const std::string KeyAttachmentRadius;
    
    SceneGraphNode();
    ~SceneGraphNode();

    static std::unique_ptr<SceneGraphNode> createFromDictionary(const ghoul::Dictionary& dictionary);

    bool initialize();
    bool deinitialize();

    void update(const UpdateData& data);
    void evaluate(const Camera& camera, const psc& parentPosition = psc());
    void render(const RenderData& data, RendererTasks& tasks);

    void addChild(SceneGraphNode& child);
    void setParent(SceneGraphNode& parent);

    glm::dvec3 translation() const;
    const glm::mat3& rotation() const;
    double scale() const;
    const glm::mat3& inverseRotation() const;
    int depth() const;

    TransformData relativeTransform(const SceneGraphNode& reference) const;

    SceneGraphNode& parent() const;
    const std::vector<SceneGraphNode*>& children() const;

    float calculateBoundingSphere();
    float boundingSphere() const;

    SceneGraphNode* find(const std::string& name);

    const PerformanceRecord& performanceRecord() const { return _performanceRecord; }

    const Renderable& renderable() const;
    Renderable& renderable();
    void setRenderable(std::unique_ptr<Renderable> renderable);

    void setAttachmentRadius(double attachmentRadius);
    double attachmentRadius() const;
   
    static documentation::Documentation Documentation();

private:
    std::vector<SceneGraphNode*> _children;
    SceneGraphNode* _parent;

    double _attachmentRadius;
    bool _boundingSphereVisible;
    float _boundingSphere;

    PerformanceRecord _performanceRecord;

    std::unique_ptr<Renderable> _renderable;
    std::unique_ptr<Translation> _translation;
    std::unique_ptr<Rotation> _rotation;
    std::unique_ptr<Scale> _scale;
};

} // namespace openspace

#endif // __SCENEGRAPHNODE_H__
