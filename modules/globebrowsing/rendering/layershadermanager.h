/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
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

#ifndef __OPENSPACE_MODULE_GLOBEBROWSING___LAYER_SHADER_MANAGER___H__
#define __OPENSPACE_MODULE_GLOBEBROWSING___LAYER_SHADER_MANAGER___H__

#include <modules/globebrowsing/rendering/layer/layermanager.h>
#include <modules/globebrowsing/rendering/layer/layer.h>

#include <array>
#include <string>

namespace ghoul::opengl { class ProgramObject; }

namespace openspace::globebrowsing {

class RenderableGlobe;

struct ChunkProgramEnv {
    struct ChunkUniformCache {
        int calculateEclipseShadows = -1;
        int cameraPosition = -1;
        int chunkLevel = -1;
        int deltaTheta0 = -1;
        int deltaTheta1 = -1;
        int deltaPhi0 = -1;
        int deltaPhi1 = -1;
        int distanceScaleFactor = -1;
        int hardShadows = -1;
        int heightScale = -1;
        int inverseViewTransform = -1;
        int invViewModelTransform = -1;
        int lightDirectionCameraSpace = -1;
        int lonLatScalingFactor = -1;
        int modelTransform = -1;
        int modelViewProjectionTransform = -1;
        int modelViewTransform = -1;
        int minLatLon = -1;
        int orenNayarRoughness = -1;
        int p00 = -1;
        int p01 = -1;
        int p10 = -1;
        int p11 = -1;
        int patchNormalModelSpace = -1;
        int patchNormalCameraSpace = -1;
        int projectionTransform = -1;
        int radiiSquared = -1;
        int skirtLength = -1;
        int tileDelta = -1;
        int vertexResolution = -1;
        int xSegments = -1;
    };

    std::unique_ptr<ghoul::opengl::ProgramObject> program;
    ChunkUniformCache uniformCache;
};


/**
 * This class has ownership of an updated shader program for rendering tiles.
 */
class LayerShaderManager {
public:
    /**
     * Data needed for shader preprocessing before compiling a layered texture shader
     * program.
     *
     * If a <code>LayerShaderPreprocessingData</code> is compared with another it can
     * be determined wheter or not a <code>LayerShaderManager</code> needs to
     * recompile its shader program. For each <code>TextureGroup</code> there is
     * information about how many layers it has and whether or not to blend the texture
     * levels.
     */
    struct LayerShaderPreprocessingData {
        /**
         * Settings per texture group that contains shader preprocessing information.
         */
        struct LayerGroupPreprocessingData {
            int lastLayerIdx;
            bool layerBlendingEnabled;
            std::vector<layergroupid::TypeID> layerType;
            std::vector<layergroupid::BlendModeID> blendMode;
            std::vector<layergroupid::AdjustmentTypeID> layerAdjustmentType;
            bool operator==(const LayerGroupPreprocessingData& other) const;
        };

        std::array<LayerGroupPreprocessingData, layergroupid::NUM_LAYER_GROUPS>
        layeredTextureInfo;
        std::vector<std::pair<std::string, std::string> > keyValuePairs;
        bool operator==(const LayerShaderPreprocessingData& other) const;

        static LayerShaderPreprocessingData get(const RenderableGlobe&);
    };

    LayerShaderManager(
        const std::string& shaderName,
        const std::string& vsPath,
        const std::string& fsPath);
    ~LayerShaderManager();

    /**
     * Returns a pointer to a <code>ChunkProgramEnv</code> for rendering tiles.
     */
    ChunkProgramEnv* programEnv();

    bool updatedSinceLastCall();

    void recompileShaderProgram(LayerShaderPreprocessingData preprocessingData);

private:
    ChunkProgramEnv _programEnv;
    LayerShaderPreprocessingData _preprocessingData;

    const std::string _shaderName;
    const std::string _vsPath;
    const std::string _fsPath;

    bool _updatedSinceLastCall;
};

} // namespace openspace::globebrowsing

#endif // __OPENSPACE_MODULE_GLOBEBROWSING___LAYER_SHADER_MANAGER___H__
