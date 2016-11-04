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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <mutex>

// open space includes
#include <openspace/rendering/renderengine.h>

#include <openspace/util/syncdata.h>

// glm includes
#include <ghoul/glm.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

namespace openspace {
    class SyncBuffer;

    /**
    This class still needs some more love. Suggested improvements:
    - Accessors should return constant references to double precision class members.
    - The class might need some more reasonable accessors depending on use cases.
    (up vector world space?)
    - Make clear which function returns a combined view matrix (things that are
    dependent on the separate sgct nodes).
    */
    class Camera {
        /**
        Used to explicitly show which variables within the Camera class that are used
        for caching.
        */
        template<typename T>
        struct Cached
        {
            Cached() { isDirty = true; }
            T datum;
            bool isDirty;
        };

        // Static constants
        static const glm::dvec3 _VIEW_DIRECTION_CAMERA_SPACE;
        static const glm::dvec3 _LOOKUP_VECTOR_CAMERA_SPACE;
    public:
        Camera(SceneGraphNode& parent);
        Camera(const Camera& o);
        ~Camera();

        // Mutators
        void setPosition(const glm::dvec3& pos);
        void setRotation(const glm::dquat& rotation);
        void setParent(SceneGraphNode& parent);

        // Relative mutators
        void rotate(const glm::dquat& rotation);

        // Accessors
        // Remove Vec3 from the name when psc is gone
        const glm::dvec3& position() const;
        const glm::dvec3& unsynchedPosition() const;
        const glm::dvec3& lookUpVectorCameraSpace() const;
        const SceneGraphNode& parent() const;
        const glm::dquat& rotationQuaternion() const;

        /**
         * Return a matrix transforming an object from node's
         * coordinate system to the camera coordinate system
         */
        const glm::mat4& viewMatrix(const SceneGraphNode& node) const;
        /**
         * Return a matrix transforming an object from the camera
         * coordinate system to the projection coordinate system
         */
        const glm::mat4& projectionMatrix() const;
        /**
         * Return the combination of the viewMatrix and projectionMatrix
         */
        const glm::mat4& viewProjectionMatrix(const SceneGraphNode& node) const;

        /**
         * Return a matrix transforming an object from the camera rig coordinate
         * system to the camera coordinate system
         */
        const glm::mat4& cameraMatrix() const;

        /**
         * Return a matrix transforming an object from node's
         * coordinate system to the camera rig's coordinate system
         */
        const glm::mat4& cameraRigMatrix(const SceneGraphNode& node);

        /**
         * Return a matrix transforming an object from the camera rig coordinate
         * system to the projection coordinate system.
         * Combination of camera matrix and projection matrix.
         */
        const glm::mat4& cameraProjectionMatrix() const;

        void invalidateCache();

        void serialize(std::ostream& os) const;
        void deserialize(std::istream& is);
        
        /**
        Handles SGCT's internal matrices. Also caches a calculated viewProjection
        matrix. This is the data that is different for different cameras within
        SGCT.
        */
        class SgctInternal {
            friend class Camera;
        public:
            void setViewMatrix(glm::mat4 viewMatrix);
            void setProjectionMatrix(glm::mat4 projectionMatrix);

            const glm::mat4& viewMatrix() const;
            const glm::mat4& projectionMatrix() const;
            const glm::mat4& viewProjectionMatrix() const;
        private:
            SgctInternal();
            SgctInternal(const SgctInternal& o)
                : _viewMatrix(o._viewMatrix)
                , _projectionMatrix(o._projectionMatrix)
                , _cachedViewProjectionMatrix(o._cachedViewProjectionMatrix)
            {}

            // State
            glm::mat4 _viewMatrix;
            glm::mat4 _projectionMatrix;

            // Cache
            mutable Cached<glm::mat4> _cachedViewProjectionMatrix;
        } sgctInternal;

        std::vector<Syncable*> getSyncables();
    private:
        SyncData<glm::dvec3> _position;
        SyncData<glm::dquat> _rotation;

        SceneGraphNode* _parent;

        // Cached data
        //mutable Cached<glm::dvec3> _cachedViewDirection;
        //mutable Cached<glm::dvec3> _cachedLookupVector;
    };
} // namespace openspace

#endif // __CAMERA_H__
