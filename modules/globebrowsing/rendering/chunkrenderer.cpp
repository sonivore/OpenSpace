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

#include <modules/globebrowsing/rendering/chunkrenderer.h>

#include <modules/globebrowsing/chunk/chunk.h>
#include <modules/globebrowsing/globes/renderableglobe.h>
#include <modules/globebrowsing/meshes/grid.h>
#include <modules/globebrowsing/rendering/layershadermanager.h>
#include <modules/globebrowsing/rendering/gpu/gpulayermanager.h>
#include <modules/globebrowsing/rendering/layer/layergroup.h>
#include <modules/globebrowsing/tile/rawtiledatareader/rawtiledatareader.h>
#include <openspace/util/updatestructures.h>
#include <openspace/util/spicemanager.h>

namespace {
    const double KM_TO_M = 1000.0;
}

namespace openspace::globebrowsing {

ChunkRenderer::ChunkRenderer(std::shared_ptr<Grid> grid,
                             std::shared_ptr<LayerManager> layerManager)
    : _grid(grid)
    , _layerManager(layerManager)
{
    _globalLayerShaderManager = std::make_shared<LayerShaderManager>(
            "GlobalChunkedLodPatch",
            "${MODULE_GLOBEBROWSING}/shaders/globalchunkedlodpatch_vs.glsl",
            "${MODULE_GLOBEBROWSING}/shaders/globalchunkedlodpatch_fs.glsl");

    _localLayerShaderManager = std::make_shared<LayerShaderManager>(
            "LocalChunkedLodPatch",
            "${MODULE_GLOBEBROWSING}/shaders/localchunkedlodpatch_vs.glsl",
            "${MODULE_GLOBEBROWSING}/shaders/localchunkedlodpatch_fs.glsl");

    _globalGpuLayerManager = std::make_shared<GPULayerManager>();
    _localGpuLayerManager = std::make_shared<GPULayerManager>();
}

void ChunkRenderer::renderChunk(const Chunk& chunk, const RenderData& data) {
    // A little arbitrary with 10 but it works
    if (chunk.tileIndex().level <
        chunk.owner().debugProperties().modelSpaceRenderingCutoffLevel) {
        renderChunkGlobally(chunk, data);
    }
    else {
        renderChunkLocally(chunk, data);
    }
}

void ChunkRenderer::setGrid(std::shared_ptr<Grid> grid) {
    _grid = grid;
}

std::shared_ptr<Grid> ChunkRenderer::grid() const {
    return _grid;
}

void ChunkRenderer::update() {
    // unused atm. Could be used for caching or precalculating
}

void ChunkRenderer::recompileShaders(const RenderableGlobe& globe) {
    LayerShaderManager::LayerShaderPreprocessingData preprocessingData =
        LayerShaderManager::LayerShaderPreprocessingData::get(globe);
    _globalLayerShaderManager->recompileShaderProgram(preprocessingData);
    _localLayerShaderManager->recompileShaderProgram(preprocessingData);
}

ChunkProgramEnv* ChunkRenderer::getActivatedProgramWithTileData(
                                 std::shared_ptr<LayerShaderManager> layeredShaderManager,
                                 std::shared_ptr<GPULayerManager> gpuLayerManager,
                                 const Chunk& chunk)
{
    const TileIndex& tileIndex = chunk.tileIndex();

    // Now the shader program can be accessed
    ChunkProgramEnv* programEnv = layeredShaderManager->programEnv();
    ghoul::opengl::ProgramObject* programObject = programEnv->program.get();
    ChunkProgramEnv::ChunkUniformCache& uniformCache = programEnv->uniformCache;

    if (layeredShaderManager->updatedSinceLastCall()) {
        gpuLayerManager->bind(programObject, *_layerManager);
        uniformCache.calculateEclipseShadows = programObject->uniformLocation("calculateEclipseShadows");
        uniformCache.cameraPosition = programObject->uniformLocation("cameraPosition");
        uniformCache.chunkLevel = programObject->uniformLocation("chunkLevel");
        uniformCache.deltaTheta0 = programObject->uniformLocation("deltaTheta0");
        uniformCache.deltaTheta1 = programObject->uniformLocation("deltaTheta1");
        uniformCache.deltaPhi0 = programObject->uniformLocation("deltaPhi0");
        uniformCache.deltaPhi1 = programObject->uniformLocation("deltaPhi1");
        uniformCache.distanceScaleFactor = programObject->uniformLocation("distanceScaleFactor");
        uniformCache.hardShadows = programObject->uniformLocation("hardShadows");
        uniformCache.heightScale = programObject->uniformLocation("heightScale");
        uniformCache.inverseViewTransform = programObject->uniformLocation("inverseViewTransform");
        uniformCache.invViewModelTransform = programObject->uniformLocation("invViewModelTransform");
        uniformCache.lightDirectionCameraSpace = programObject->uniformLocation("lightDirectionCameraSpace");
        uniformCache.lonLatScalingFactor = programObject->uniformLocation("lonLatScalingFactor");
        uniformCache.modelTransform = programObject->uniformLocation("modelTransform");
        uniformCache.modelViewProjectionTransform = programObject->uniformLocation("modelViewProjectionTransform");
        uniformCache.modelViewTransform = programObject->uniformLocation("modelViewTransform");
        uniformCache.minLatLon = programObject->uniformLocation("minLatLon");
        uniformCache.orenNayarRoughness = programObject->uniformLocation("orenNayarRoughness");
        uniformCache.p00 = programObject->uniformLocation("p00");
        uniformCache.p01 = programObject->uniformLocation("p01");
        uniformCache.p10 = programObject->uniformLocation("p10");
        uniformCache.p11 = programObject->uniformLocation("p11");
        uniformCache.patchNormalModelSpace = programObject->uniformLocation("patchNormalModelSpace");
        uniformCache.patchNormalCameraSpace = programObject->uniformLocation("patchNormalCameraSpace");
        uniformCache.projectionTransform = programObject->uniformLocation("projectionTransform");
        uniformCache.radiiSquared = programObject->uniformLocation("radiiSquared");
        uniformCache.skirtLength = programObject->uniformLocation("skirtLength");
        uniformCache.tileDelta = programObject->uniformLocation("tileDelta");
        uniformCache.vertexResolution = programObject->uniformLocation("vertexResolution");
        uniformCache.xSegments = programObject->uniformLocation("xSegments");
    }

    // Activate the shader program
    programObject->activate();

    gpuLayerManager->setValue(programObject, *_layerManager, tileIndex);

    // The length of the skirts is proportional to its size
    // TODO: Skirt length should probably be proportional to the size reffered to by
    // the chunk's most high resolution height map.
    programObject->setUniform(uniformCache.skirtLength,
        glm::min(static_cast<float>(chunk.surfacePatch().halfSize().lat * 1000000),
            8700.0f));
    programObject->setUniform(uniformCache.xSegments, _grid->xSegments());

    if (chunk.owner().debugProperties().showHeightResolution) {
        programObject->setUniform(uniformCache.vertexResolution,
            glm::vec2(_grid->xSegments(), _grid->ySegments()));
    }

    return programEnv;
}

void ChunkRenderer::calculateEclipseShadows(const Chunk& chunk,
                                            ghoul::opengl::ProgramObject* programObject,
                                            const RenderData& data)
{
    // Shadow calculations..
    if (chunk.owner().ellipsoid().hasEclipseShadows()) {
        std::vector<RenderableGlobe::ShadowRenderingStruct> shadowDataArray;
        std::vector<Ellipsoid::ShadowConfiguration> shadowConfArray =
            chunk.owner().ellipsoid().shadowConfigurationArray();
        shadowDataArray.reserve(shadowConfArray.size());
        double lt;
        for (const auto & shadowConf : shadowConfArray) {
            // TO REMEMBER: all distances and lengths in world coordinates are in
            // meters!!! We need to move this to view space...
            // Getting source and caster:
            glm::dvec3 sourcePos = SpiceManager::ref().targetPosition(
                shadowConf.source.first,
                "SUN",
                "GALACTIC",
                {},
                data.time.j2000Seconds(),
                lt
            );
            sourcePos *= KM_TO_M; // converting to meters
            glm::dvec3 casterPos = SpiceManager::ref().targetPosition(
                shadowConf.caster.first,
                "SUN",
                "GALACTIC",
                {},
                data.time.j2000Seconds(),
                lt
            );
            casterPos *= KM_TO_M; // converting to meters
            // psc caster_pos = PowerScaledCoordinate::CreatePowerScaledCoordinate(
            //     casterPos.x,
            //     casterPos.y,
            //     casterPos.z
            // );


            // First we determine if the caster is shadowing the current planet (all
            // calculations in World Coordinates):
            glm::dvec3 planetCasterVec = casterPos - data.position.dvec3();
            glm::dvec3 sourceCasterVec = casterPos - sourcePos;
            double sc_length = glm::length(sourceCasterVec);
            glm::dvec3 planetCaster_proj = (glm::dot(planetCasterVec, sourceCasterVec) /
                                            (sc_length*sc_length)) * sourceCasterVec;
            double d_test = glm::length(planetCasterVec - planetCaster_proj);
            double xp_test = shadowConf.caster.second * sc_length /
                             (shadowConf.source.second + shadowConf.caster.second);
            double rp_test = shadowConf.caster.second *
                             (glm::length(planetCaster_proj) + xp_test) / xp_test;

            glm::dvec3 sunPos = SpiceManager::ref().targetPosition(
                "SUN",
                "SUN",
                "GALACTIC",
                {},
                data.time.j2000Seconds(),
                lt
            );
            double casterDistSun = glm::length(casterPos - sunPos);
            double planetDistSun = glm::length(data.position.dvec3() - sunPos);

            RenderableGlobe::ShadowRenderingStruct shadowData;
            shadowData.isShadowing = false;

            // Eclipse shadows considers planets and moons as spheres
            if (((d_test - rp_test) < (chunk.owner().ellipsoid().radii().x * KM_TO_M)) &&
                (casterDistSun < planetDistSun)) {
                // The current caster is shadowing the current planet
                shadowData.isShadowing = true;
                shadowData.rs = shadowConf.source.second;
                shadowData.rc = shadowConf.caster.second;
                shadowData.sourceCasterVec = glm::normalize(sourceCasterVec);
                shadowData.xp = xp_test;
                shadowData.xu = shadowData.rc * sc_length /
                                (shadowData.rs - shadowData.rc);
                shadowData.casterPositionVec = casterPos;
            }
            shadowDataArray.push_back(shadowData);
        }

        const std::string uniformVarName("shadowDataArray[");
        unsigned int counter = 0;
        for (const auto & sd : shadowDataArray) {
            std::stringstream ss;
            ss << uniformVarName << counter << "].isShadowing";
            programObject->setUniform(ss.str(), sd.isShadowing);
            if (sd.isShadowing) {
                ss.str(std::string());
                ss << uniformVarName << counter << "].xp";
                programObject->setUniform(ss.str(), sd.xp);
                ss.str(std::string());
                ss << uniformVarName << counter << "].xu";
                programObject->setUniform(ss.str(), sd.xu);
                /*ss.str(std::string());
                ss << uniformVarName << counter << "].rs";
                programObject->setUniform(ss.str(), sd.rs);*/
                ss.str(std::string());
                ss << uniformVarName << counter << "].rc";
                programObject->setUniform(ss.str(), sd.rc);
                ss.str(std::string());
                ss << uniformVarName << counter << "].sourceCasterVec";
                programObject->setUniform(ss.str(), sd.sourceCasterVec);
                ss.str(std::string());
                ss << uniformVarName << counter << "].casterPositionVec";
                programObject->setUniform(ss.str(), sd.casterPositionVec);
            }
            counter++;
        }

        programObject->setUniform(
            "inverseViewTransform",
            glm::inverse(data.camera.combinedViewMatrix())
        );
        programObject->setUniform("modelTransform", chunk.owner().modelTransform());
        programObject->setUniform(
            "hardShadows",
            chunk.owner().generalProperties().eclipseHardShadows
        );
        programObject->setUniform("calculateEclipseShadows", true);
    }
}

void ChunkRenderer::setCommonUniforms(ChunkProgramEnv& programEnv,
                                      const Chunk& chunk, const RenderData& data)
{
    ghoul::opengl::ProgramObject& programObject = *programEnv.program.get();
    ChunkProgramEnv::ChunkUniformCache& uniformCache = programEnv.uniformCache;

    glm::dmat4 modelTransform = chunk.owner().modelTransform();
    glm::dmat4 viewTransform = data.camera.combinedViewMatrix();
    glm::dmat4 modelViewTransform = viewTransform * modelTransform;

    const bool nightLayersActive =
        !_layerManager->layerGroup(layergroupid::NightLayers).activeLayers().empty();
    const bool waterLayersActive =
        !_layerManager->layerGroup(layergroupid::WaterMasks).activeLayers().empty();

    if (nightLayersActive ||
        waterLayersActive ||
        chunk.owner().generalProperties().atmosphereEnabled ||
        chunk.owner().generalProperties().performShading)
    {
        glm::vec3 directionToSunWorldSpace =
            glm::normalize(-data.modelTransform.translation);
        glm::vec3 directionToSunCameraSpace =
            glm::vec3(viewTransform * glm::dvec4(directionToSunWorldSpace, 0));
        programObject.setUniform(
            uniformCache.lightDirectionCameraSpace, -directionToSunCameraSpace);
    }

    if (chunk.owner().generalProperties().performShading) {
        programObject.setUniform(
            uniformCache.orenNayarRoughness,
            chunk.owner().generalProperties().orenNayarRoughness);
    }

    if (chunk.owner().generalProperties().useAccurateNormals &&
        !_layerManager->layerGroup(layergroupid::HeightLayers).activeLayers().empty())
    {
        glm::dvec3 corner00 = chunk.owner().ellipsoid().cartesianSurfacePosition(
            chunk.surfacePatch().getCorner(Quad::SOUTH_WEST));
        glm::dvec3 corner10 = chunk.owner().ellipsoid().cartesianSurfacePosition(
            chunk.surfacePatch().getCorner(Quad::SOUTH_EAST));
        glm::dvec3 corner01 = chunk.owner().ellipsoid().cartesianSurfacePosition(
            chunk.surfacePatch().getCorner(Quad::NORTH_WEST));
        glm::dvec3 corner11 = chunk.owner().ellipsoid().cartesianSurfacePosition(
            chunk.surfacePatch().getCorner(Quad::NORTH_EAST));

        // This is an assumption that the height tile has a resolution of 64 * 64
        // If it does not it will still produce "correct" normals. If the resolution is
        // higher the shadows will be softer, if it is lower, pixels will be visible.
        // Since default is 64 this will most likely work fine.
        float tileDelta = 1.0f / 64.0f;
        glm::vec3 deltaTheta0 = glm::vec3(corner10 - corner00) * tileDelta;
        glm::vec3 deltaTheta1 = glm::vec3(corner11 - corner01) * tileDelta;
        glm::vec3 deltaPhi0 = glm::vec3(corner01 - corner00) * tileDelta;
        glm::vec3 deltaPhi1 = glm::vec3(corner11 - corner10) * tileDelta;

        // Transform to camera space
        glm::mat3 modelViewTransformMat3 = glm::mat3(modelViewTransform);
        deltaTheta0 = modelViewTransformMat3 * deltaTheta0;
        deltaTheta1 = modelViewTransformMat3 * deltaTheta1;
        deltaPhi0 = modelViewTransformMat3 * deltaPhi0;
        deltaPhi1 = modelViewTransformMat3 * deltaPhi1;

        // Upload uniforms
        programObject.setUniform(uniformCache.deltaTheta0, glm::length(deltaTheta0));
        programObject.setUniform(uniformCache.deltaTheta1, glm::length(deltaTheta1));
        programObject.setUniform(uniformCache.deltaPhi0, glm::length(deltaPhi0));
        programObject.setUniform(uniformCache.deltaPhi1, glm::length(deltaPhi1));
        programObject.setUniform(uniformCache.tileDelta, tileDelta);

        // This should not be needed once the light calculations for the atmosphere
        // is performed in view space..
        programObject.setUniform(uniformCache.invViewModelTransform,
                               glm::inverse(glm::mat4(data.camera.combinedViewMatrix()) *
                                            glm::mat4(chunk.owner().modelTransform())));
    }
}

void ChunkRenderer::renderChunkGlobally(const Chunk& chunk, const RenderData& data) {
    ChunkProgramEnv* programEnv = getActivatedProgramWithTileData(
        _globalLayerShaderManager,
        _globalGpuLayerManager,
        chunk);

    ghoul::opengl::ProgramObject* programObject = programEnv->program.get();
    ChunkProgramEnv::ChunkUniformCache& uniformCache = programEnv->uniformCache;

    if (programObject == nullptr) {
        return;
    }

    const Ellipsoid& ellipsoid = chunk.owner().ellipsoid();

    if (_layerManager->hasAnyBlendingLayersEnabled()) {
        // Calculations are done in the reference frame of the globe. Hence, the
        // camera position needs to be transformed with the inverse model matrix
        glm::dmat4 inverseModelTransform = chunk.owner().inverseModelTransform();
        glm::dvec3 cameraPosition = glm::dvec3(
            inverseModelTransform * glm::dvec4(data.camera.positionVec3(), 1));
        float distanceScaleFactor = static_cast<float>(
            chunk.owner().generalProperties().lodScaleFactor * ellipsoid.minimumRadius()
        );
        programObject->setUniform(uniformCache.cameraPosition, glm::vec3(cameraPosition));
        programObject->setUniform(uniformCache.distanceScaleFactor, distanceScaleFactor);
        programObject->setUniform(uniformCache.chunkLevel, chunk.tileIndex().level);
    }

    // Calculate other uniform variables needed for rendering
    Geodetic2 swCorner = chunk.surfacePatch().getCorner(Quad::SOUTH_WEST);
    auto patchSize = chunk.surfacePatch().size();

    glm::dmat4 modelTransform = chunk.owner().modelTransform();
    glm::dmat4 viewTransform = data.camera.combinedViewMatrix();
    glm::mat4 modelViewTransform = glm::mat4(viewTransform * modelTransform);
    glm::mat4 modelViewProjectionTransform = data.camera.sgctInternal.projectionMatrix() *
        modelViewTransform;

    // Upload the uniform variables
    programObject->setUniform(
        uniformCache.modelViewProjectionTransform, modelViewProjectionTransform);
    programObject->setUniform(uniformCache.minLatLon, glm::vec2(swCorner.toLonLatVec2()));
    programObject->setUniform(uniformCache.lonLatScalingFactor, glm::vec2(patchSize.toLonLatVec2()));
    // Ellipsoid Radius (Model Space)
    programObject->setUniform(uniformCache.radiiSquared, glm::vec3(ellipsoid.radiiSquared()));

    if (_layerManager->layerGroup(
            layergroupid::GroupID::NightLayers).activeLayers().size() > 0 ||
        _layerManager->layerGroup(
            layergroupid::GroupID::WaterMasks).activeLayers().size() > 0 ||
        chunk.owner().generalProperties().atmosphereEnabled ||
        chunk.owner().generalProperties().performShading)
    {
        programObject->setUniform(uniformCache.modelViewTransform, modelViewTransform);
    }

    if (chunk.owner().generalProperties().useAccurateNormals &&
        !_layerManager->layerGroup(layergroupid::HeightLayers).activeLayers().empty())
    {
        // Apply an extra scaling to the height if the object is scaled
        programObject->setUniform(
            uniformCache.heightScale, static_cast<float>(data.modelTransform.scale));
    }

    setCommonUniforms(*programEnv, chunk, data);

    if (chunk.owner().ellipsoid().hasEclipseShadows()) {
        calculateEclipseShadows(chunk, programObject, data);
    }

    // OpenGL rendering settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // render
    _grid->geometry().drawUsingActiveProgram();

    _globalGpuLayerManager->deactivate();

    // disable shader
    programObject->deactivate();

}

void ChunkRenderer::renderChunkLocally(const Chunk& chunk, const RenderData& data) {
    ChunkProgramEnv* programEnv = getActivatedProgramWithTileData(
        _localLayerShaderManager,
        _localGpuLayerManager,
        chunk);

    ghoul::opengl::ProgramObject* programObject = programEnv->program.get();
    ChunkProgramEnv::ChunkUniformCache& uniformCache = programEnv->uniformCache;

    if (programObject == nullptr) {
        return;
    }

    using namespace glm;

    const Ellipsoid& ellipsoid = chunk.owner().ellipsoid();


    if (_layerManager->hasAnyBlendingLayersEnabled()) {
        float distanceScaleFactor = static_cast<float>(
            chunk.owner().generalProperties().lodScaleFactor *
            chunk.owner().ellipsoid().minimumRadius()
        );

        programObject->setUniform(uniformCache.distanceScaleFactor, distanceScaleFactor);
        programObject->setUniform(uniformCache.chunkLevel, chunk.tileIndex().level);
    }

    // Calculate other uniform variables needed for rendering
    // Send the matrix inverse to the fragment for the global and local shader (JCC)
    dmat4 modelTransform = chunk.owner().modelTransform();
    dmat4 viewTransform = data.camera.combinedViewMatrix();
    dmat4 modelViewTransform = viewTransform * modelTransform;

    std::vector<int> cornerUniformLocations = {
        uniformCache.p01,
        uniformCache.p11,
        uniformCache.p00,
        uniformCache.p10
    };

    std::vector<glm::dvec3> cornersCameraSpace(4);
    std::vector<glm::dvec3> cornersModelSpace(4);
    for (int i = 0; i < 4; ++i) {
        Quad q = static_cast<Quad>(i);
        Geodetic2 corner = chunk.surfacePatch().getCorner(q);
        glm::dvec3 cornerModelSpace = ellipsoid.cartesianSurfacePosition(corner);
        cornersModelSpace[i] = cornerModelSpace;
        glm::dvec3 cornerCameraSpace =
            glm::dvec3(modelViewTransform * glm::dvec4(cornerModelSpace, 1));
        cornersCameraSpace[i] = cornerCameraSpace;
        programObject->setUniform(cornerUniformLocations[i], vec3(cornerCameraSpace));

    }

    // TODO: Patch normal can be calculated for all corners and then linearly
    // interpolated on the GPU to avoid cracks for high altitudes.
    vec3 patchNormalCameraSpace = normalize(
        cross(cornersCameraSpace[Quad::SOUTH_EAST] -
                cornersCameraSpace[Quad::SOUTH_WEST],
            cornersCameraSpace[Quad::NORTH_EAST] -
                cornersCameraSpace[Quad::SOUTH_WEST]));

    // In order to improve performance, lets use the normal in object space (model space)
    // for deferred rendering.
    vec3 patchNormalModelSpace = normalize(
        cross(cornersModelSpace[Quad::SOUTH_EAST] -
            cornersModelSpace[Quad::SOUTH_WEST],
            cornersModelSpace[Quad::NORTH_EAST] -
            cornersModelSpace[Quad::SOUTH_WEST]));

    programObject->setUniform(uniformCache.patchNormalModelSpace, patchNormalModelSpace);
    programObject->setUniform(uniformCache.patchNormalCameraSpace, patchNormalCameraSpace);
    programObject->setUniform(uniformCache.projectionTransform,
        data.camera.sgctInternal.projectionMatrix()
    );

    if (_layerManager->layerGroup(layergroupid::HeightLayers).activeLayers().size() > 0) {
        // Apply an extra scaling to the height if the object is scaled
        programObject->setUniform(
            uniformCache.heightScale, static_cast<float>(data.modelTransform.scale));
    }

    setCommonUniforms(*programEnv, chunk, data);

    if (chunk.owner().ellipsoid().hasEclipseShadows()) {
        calculateEclipseShadows(chunk, programObject, data);
    }

    // OpenGL rendering settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // render
    _grid->geometry().drawUsingActiveProgram();

    _localGpuLayerManager->deactivate();

    // disable shader
    programObject->deactivate();
}

} // namespace openspace:;globebrowsing
