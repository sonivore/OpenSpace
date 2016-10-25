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

#include <openspace/util/distancetoobject.h>
#include <cassert>
#include <openspace/util/powerscaledcoordinate.h>

#include <openspace/engine/openspaceengine.h>
#include <openspace/interaction/interactionhandler.h>
#include <openspace/util/spicemanager.h>
#include <openspace/util/syncbuffer.h>

#include <ghoul/filesystem/filesystem.h>

namespace {
    const std::string _loggerCat = "Distance to object";
}


namespace openspace {

    DistanceToObject* DistanceToObject::_distInstance = nullptr;
    DistanceToObject::DistanceToObject() {}
    int sceneNr = 1;

    bool DistanceToObject::initialize() {
        assert(_distInstance == nullptr);
        _distInstance = new DistanceToObject();
        assert(sceneNr == 1);
        return true;
    }

    void DistanceToObject::deinitialize() {
        assert(_distInstance);
        delete _distInstance;
        _distInstance = nullptr;
    }



    DistanceToObject& DistanceToObject::ref() {
        assert(_distInstance);
        return *_distInstance;
    }

    double DistanceToObject::distanceCalc(const psc & positionPSC, const psc & targetPosPSC) const
    {
        glm::dvec3 targetPos(targetPosPSC.dvec3());
        glm::dvec3 position(positionPSC.dvec3());

        // JCC: Changed the distance calculations from meters to killometers.
        glm::dvec3 diffVec = (position / 1000.0) - (targetPos / 1000.0);    
        
        return glm::length(diffVec);
    }

    double DistanceToObject::distanceCalc(const glm::dvec3 & position, const glm::dvec3 & targetPos) const
    {
        // JCC: Changed the distance calculations from meters to killometers.
        glm::dvec3 diffVec = (position / 1000.0) - (targetPos / 1000.0);

        return glm::length(diffVec);
    }


}