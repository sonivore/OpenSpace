/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2017                                                               *
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

#include <modules/solarbrowsing/rendering/renderablesolarimageryprojection.h>
#include <modules/solarbrowsing/rendering/renderablesolarimagery.h>
//#include <modules/solarbrowsing/rendering/renderablesolarvideo.h>
#include <modules/space/rendering/planetgeometry.h>
#include <openspace/util/time.h>
#include <openspace/scene/scenegraphnode.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/texture.h>
#include <ghoul/opengl/textureunit.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/engine/openspaceengine.h>
#include <modules/fitsfilereader/include/fitsfilereader.h>
#include <modules/solarbrowsing/rendering/spacecraftcameraplane.h>

#include <memory>
#include <fstream>
#include <limits>

using namespace ghoul::opengl;

namespace {
    static const std::string _loggerCat = "RendearbleSpacecraftCameraSphere";
    const char* keyRadius = "Radius";
}

namespace openspace {
RenderableSolarImageryProjection::RenderableSolarImageryProjection(
      const ghoul::Dictionary& dictionary)
    : Renderable(dictionary)
    , _shader(nullptr)
    , _sphere(nullptr)
{
    if (!dictionary.getValue("Name", _nodeName)) {
        throw ghoul::RuntimeError("Nodename has to be specified");
    }
}

bool RenderableSolarImageryProjection::initialize() {
    const std::vector<SceneGraphNode*>& allNodes
          = OsEng.renderEngine().scene()->allSceneGraphNodes();
    for (auto node : allNodes) {
        if (dynamic_cast<RenderableSolarImagery*>(node->renderable())) {
           // SceneGraphNode* tmp2 = const_cast<SceneGraphNode*>(node);
            auto thisNode = OsEng.renderEngine().scene()->sceneGraphNode(_nodeName);
            thisNode->addDependency(*node);
        }

    }

    _solarImageryDependencies
         = OsEng.renderEngine().scene()->sceneGraphNode(_nodeName)->dependencies();

    const std::string path = "/home/noven/workspace/OpenSpace/data/hmimap1.fits";
    FitsFileReader fts(false);
    std::shared_ptr<ImageData<float>> imageData = fts.readImage<float>(path);
    float* data;
    if (imageData) {
        const std::valarray<float>& imageContents = imageData->contents;
        data = new float[imageContents.size()];
        std::memmove(data, &imageContents[0], imageContents.size() * sizeof(float));
    }

    //const glm::size3_t imageSize(sizeX, sizeY, 1);
    _magnetogramTexture = std::make_unique<Texture>(
        data,
        glm::size3_t(3600, 1440, 1),
        ghoul::opengl::Texture::Red,
        GL_R32F,
        GL_FLOAT,
        Texture::FilterMode::Linear,
        Texture::WrappingMode::ClampToEdge
    );
    _magnetogramTexture->uploadTexture();

    if (!_shader) {
        RenderEngine& renderEngine = OsEng.renderEngine();
        _shader = renderEngine.buildRenderProgram("SpacecraftImageSphereProgram",
            "${MODULE_SOLARBROWSING}/shaders/spacecraftimagesphere_vs.glsl",
            "${MODULE_SOLARBROWSING}/shaders/spacecraftimagesphere_fs.glsl"
            );
        if (!_shader) {
            return false;
        }
    }

    //PowerScaledScalar planetSize(glm::vec2(696701000.f, 0.f));
    PowerScaledScalar planetSize(glm::vec2(6.96701f, 8.f));
    _sphere = std::make_unique<PowerScaledSphere>(PowerScaledSphere(planetSize, 100));
    _sphere->initialize();

    return isReady();
}

bool RenderableSolarImageryProjection::deinitialize() {
    RenderEngine& renderEngine = OsEng.renderEngine();
    if (_shader) {
        renderEngine.removeRenderProgram(_shader);
        _shader = nullptr;
    }
    return true;
}

bool RenderableSolarImageryProjection::isReady() const {
    return _shader && _sphere && _magnetogramTexture;
}

void RenderableSolarImageryProjection::update(const UpdateData& data) {
    if (_shader->isDirty()) {
        _shader->rebuildFromFile();
    }
}

void RenderableSolarImageryProjection::render(const RenderData& data) {
    glm::dmat4 modelTransform =
        glm::translate(glm::dmat4(1.0), data.modelTransform.translation) * // Translation
        glm::dmat4(data.modelTransform.rotation) *  // Spice rotation
        glm::dmat4(glm::scale(glm::dmat4(1.0), glm::dvec3(data.modelTransform.scale)));
    glm::dmat4 modelViewTransform = data.camera.combinedViewMatrix() * modelTransform;

    _shader->activate();
    // _shader->setUniform(
    //     "modelTransform",
    //     modelTransform
    // );

    // _shader->setUniform(
    //     "viewTransform",
    //     data.camera.combinedViewMatrix()
    // );

    //  _shader->setUniform(
    //     "projectionTransform",
    //      data.camera.projectionMatrix() 
    // );

    _shader->setUniform(
        "modelViewProjectionTransform",
        data.camera.projectionMatrix() * glm::mat4(modelViewTransform)
    );

    const int numPlanes = _solarImageryDependencies.size();
    const int MAX_SPACECRAFT_OBSERVATORY = 5;
    int solarImageryCount = 0;

    ghoul::opengl::TextureUnit txUnits[MAX_SPACECRAFT_OBSERVATORY];
    ghoul::opengl::TextureUnit tfUnits[MAX_SPACECRAFT_OBSERVATORY];

    for (int i = 0; i < numPlanes; ++i) {
        auto* solarImagery = static_cast<RenderableSolarImagery*>(
              _solarImageryDependencies[i]->renderable());

        bool isCoronaGraph = solarImagery->_isCoronaGraph;

        const SpacecraftCameraPlane& plane = solarImagery->cameraPlane();
        const glm::dvec3 planePos = plane.worldPosition();
        const glm::dmat4 planeRot = plane.worldRotation();

        // Camera looking away, fake corona graph same bool same logic, TODO: change name
        // if (!solarImagery->_shouldRenderPlane) {
        //     isCoronaGraph = true;
        // }

        _shader->setUniform("isCoronaGraph[" + std::to_string(i) + "]", isCoronaGraph);

        //_shader->setUniform("magicPlaneFactor[" + std::to_string(i) + "]", solarImagery->_magicPlaneFactor);

        _shader->setUniform("sunToSpacecraftReferenceFrame[" + std::to_string(i) + "]",
                        planeRot * glm::dmat4(data.modelTransform.rotation));
        _shader->setUniform("planePositionSpacecraft[" + std::to_string(i) + "]",
                            glm::dvec3(planeRot * glm::dvec4(planePos, 1.0)));

        //_shader->setUniform("imageSize[" + std::to_string(i) + "]" , solarImagery->_imageSize);
        //_shader->setUniform("sharpenValue[" + std::to_string(i) + "]", solarImagery->_sharpenValue);
        _shader->setUniform("gammaValue[" + std::to_string(i) + "]", solarImagery->_gammaValue);
        _shader->setUniform("contrastValue[" + std::to_string(i) + "]", solarImagery->_contrastValue);

        // Offset and scale
        _shader->setUniform("scale[" + std::to_string(i) + "]", solarImagery->_currentScale);
        _shader->setUniform("centerPixel[" + std::to_string(i) + "]", solarImagery->_currentCenterPixel);

        // Imagery texture
        txUnits[i].activate();
        solarImagery->getImageryTexture()->bind();
        _shader->setUniform("imageryTexture[" + std::to_string(i) + "]", txUnits[i]);
        tfUnits[i].activate();

        auto lut = solarImagery->getTransferFunction();
        if (lut) {
            lut->bind();
            _shader->setUniform("hasLut[" + std::to_string(i) + "]", true);
        } else {
            _shader->setUniform("hasLut[" + std::to_string(i) + "]", false);
        }
        // Must bind all sampler2D, otherwise undefined behaviour
        _shader->setUniform("lut[" + std::to_string(i) + "]", tfUnits[i]);
        solarImageryCount++;
    }

    // Set the rest of the texture units for well defined behaviour
    for (int i = solarImageryCount; i < MAX_SPACECRAFT_OBSERVATORY; ++i) {
        txUnits[i].activate();
        _shader->setUniform("imageryTexture[" + std::to_string(i) + "]", txUnits[i]);
        tfUnits[i].activate();
        _shader->setUniform("lut[" + std::to_string(i) + "]", tfUnits[i]);
    }

    ghoul::opengl::TextureUnit imageUnit;
    imageUnit.activate();
    _magnetogramTexture->bind();
    _shader->setUniform("magnetogram", imageUnit);

    _shader->setUniform("numSpacecraftCameraPlanes", numPlanes);

    _sphere->render();;
    _shader->deactivate();
}

}
