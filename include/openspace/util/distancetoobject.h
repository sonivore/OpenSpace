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

#ifndef __DISTANCETOOBJ_H__
#define __DISTANCETOOBJ_H__

#include <openspace/util/powerscaledcoordinate.h>


#include <glm/gtc/type_ptr.hpp>

#include <iostream>
namespace openspace {

class DistanceToObject {
public:
    static bool initialize();
    static void deinitialize();
    static int sceneNrPub;

    /*
    * Returns the reference to the distance singleton object.
    * \return The reference to the distance singleton object
    */

    static DistanceToObject& ref();
    double distanceCalc(const psc & position, const psc & targetPos) const;
    double distanceCalc(const glm::dvec3 & position, const glm::dvec3 & targetPos) const;
private:
    /// Creates the distance object. Only used in the initialize() method
    DistanceToObject();
    double distToScene = 500000.0;
    DistanceToObject(const DistanceToObject& src) = delete;

    DistanceToObject& operator=(const DistanceToObject& rhs) = delete;
    double distance;
    static DistanceToObject* _distInstance; ///< The singleton instance
                                            //float distance;

                                            //float _distancefromcamera;
                                            //static DistanceToObject* _distancefromcamera; ///< The singleton instance

};
}
#endif // __DISTANCETOOBJ_H__