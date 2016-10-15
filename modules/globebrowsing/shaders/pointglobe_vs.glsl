/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014                                                                    *
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

#version __CONTEXT__

#include "PowerScaling/powerScaling_vs.hglsl"

layout(location = 0) in vec3 in_position;

uniform int windowWidth;
uniform float globeRadius;
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;
uniform mat4 directionToSunViewSpace;

out vec4 vs_positionClipSpace;
out vec4 vs_positionCameraSpace;

void main() {
    vec4 positionCameraSpace = modelViewTransform * vec4(in_position, 1);
    vs_positionClipSpace = projectionTransform * positionCameraSpace;

    vs_positionClipSpace = z_normalization(vs_positionClipSpace);    

    vec4 pointSizeCameraSpace = modelViewTransform * vec4(0,0, in_position.z, 1);
    pointSizeCameraSpace.x = globeRadius;
    pointSizeCameraSpace.y = globeRadius;
    vec4 pointSizeClipSpace = projectionTransform * pointSizeCameraSpace;
    vec4 pointSizeNDC = pointSizeClipSpace / pointSizeClipSpace.w;

    gl_PointSize = pointSizeNDC.x * 2 * windowWidth;
    gl_Position = vs_positionClipSpace;
}