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

// open space includes
#include <openspace/util/camera.h>
#include <openspace/util/syncbuffer.h>
#include <openspace/query/query.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/interaction/interactionhandler.h>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace openspace {

    //////////////////////////////////////////////////////////////////////////////////////
    //                                        CAMERA                                        //
    //////////////////////////////////////////////////////////////////////////////////////

    namespace {
        const std::string _loggerCat = "Camera";
    }

    const glm::dvec3 Camera::_VIEW_DIRECTION_CAMERA_SPACE = glm::dvec3(0, 0, -1);
    const glm::dvec3 Camera::_LOOKUP_VECTOR_CAMERA_SPACE = glm::dvec3(0, 1, 0);

    Camera::Camera(SceneGraphNode& parent)
        : _parent(&parent)
        , _position(glm::vec3(0.0))
        , _rotation(glm::quat(glm::vec3(0.0)))
    {}
    
    Camera::Camera(const Camera& o)
        : sgctInternal(o.sgctInternal)
        , _position(o._position)
        , _rotation(o._rotation)
    {}

    Camera::~Camera() { }

    // Mutators

    void Camera::setPosition(const glm::dvec3& pos) {
        _position = pos;
    }

    void Camera::setRotation(const glm::dquat& rotation) {
        _rotation = rotation;
    }

    void Camera::setParent(SceneGraphNode& parent) {
        _parent = &parent;
    }

    // Relative mutators
    void Camera::rotate(const glm::dquat& rotation) {
        _rotation = rotation * glm::dquat(_rotation);
    }

    // Accessors

    const SceneGraphNode& Camera::parent() const {
        return *_parent;
    }

    const glm::dvec3& Camera::position() const {
        return _position;
    }

    const glm::dvec3& Camera::unsynchedPosition() const {
        return _position;
    }

    const glm::dvec3& Camera::lookUpVectorCameraSpace() const {
        return _LOOKUP_VECTOR_CAMERA_SPACE;
    }

    const glm::dquat& Camera::rotationQuaternion() const {
        return _rotation;
    }

    const glm::mat4& Camera::viewMatrix(const SceneGraphNode & node) const {
        TransformData relativeTransform = node.relativeTransform(parent());

        glm::mat4 nodeToCameraParent = 
            glm::translate(glm::mat4(1.0), glm::vec3(relativeTransform.translation)) * // Translation
            glm::mat4(relativeTransform.rotation) *                                    // Rotation
            glm::scale(glm::mat4(1.0), glm::vec3(relativeTransform.scale));            // Scaling

 
        glm::mat4 cameraParentToCamera =
            glm::mat4_cast(glm::quat(glm::inverse(glm::dquat(_rotation)))) *
            glm::translate(glm::mat4(1.0), -1.0f * glm::vec3(glm::dvec3(_position)));

        return sgctInternal.viewMatrix() * cameraParentToCamera * nodeToCameraParent;
    }

    const glm::mat4& Camera::viewProjectionMatrix(const SceneGraphNode& node) const {
        return sgctInternal.projectionMatrix() * viewMatrix(node);
    }

    const glm::mat4& Camera::projectionMatrix() const {
        return sgctInternal.projectionMatrix();
    }

    const glm::mat4 & Camera::cameraMatrix() const {
        return sgctInternal.viewMatrix();
    }

    void Camera::invalidateCache() {
        //_cachedViewDirection.isDirty = true;
        //_cachedLookupVector.isDirty = true;
        //_cachedViewRotationMatrix.isDirty = true;
        //_cachedCombinedViewMatrix.isDirty = true;
    }

    
    void Camera::serialize(std::ostream& os) const {
        glm::dvec3 p = position();
        glm::dquat q = rotationQuaternion();
        os << p.x << " " << p.y << " " << p.z << std::endl;
        os << q.x << " " << q.y << " " << q.z << " " << q.w << std::endl;
    }

    void Camera::deserialize(std::istream& is) {
        glm::dvec3 p;
        glm::dquat q;
        is >> p.x >> p.y >> p.z;
        is >> q.x >> q.y >> q.z >> q.w;
        setPosition(p);
        setRotation(q);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //                                    SGCT INTERNAL                                 //
    //////////////////////////////////////////////////////////////////////////////////////

    Camera::SgctInternal::SgctInternal()
        : _viewMatrix()
        , _projectionMatrix()
    { }

    void Camera::SgctInternal::setViewMatrix(glm::mat4 viewMatrix) {
        _viewMatrix = std::move(viewMatrix);
        _cachedViewProjectionMatrix.isDirty = true;
    }

    void Camera::SgctInternal::setProjectionMatrix(glm::mat4 projectionMatrix) {
        _projectionMatrix = std::move(projectionMatrix);
        _cachedViewProjectionMatrix.isDirty = true;
    }

    const glm::mat4& Camera::SgctInternal::viewMatrix() const {
        return _viewMatrix;
    }

    const glm::mat4& Camera::SgctInternal::projectionMatrix() const {
        return _projectionMatrix;
    }

    const glm::mat4& Camera::SgctInternal::viewProjectionMatrix() const {
        if (_cachedViewProjectionMatrix.isDirty) {
            _cachedViewProjectionMatrix.datum = _projectionMatrix * _viewMatrix;
            _cachedViewProjectionMatrix.isDirty = false;
        }
        return _cachedViewProjectionMatrix.datum;
    }

    std::vector<Syncable*> Camera::getSyncables() {
        return{ &_position, &_rotation };
    }

} // namespace openspace
