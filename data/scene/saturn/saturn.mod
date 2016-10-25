return {
    -- Saturn barycenter module
    {
        Name = "SaturnBarycenter",
        Parent = "SolarSystemBarycenter",
        -- Scene Radius in KM:
        SceneRadius = 1.0E+6,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "SATURN BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SUN",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
        },
    },

    -- Saturn module
    {   
        Name = "Saturn",
        Parent = "SaturnBarycenter",
        -- Scene Radius in KM:
        SceneRadius = 6.0E+5,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_SATURN",
            Body = "SATURN",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 6.0268, 7 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/saturn.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "SATURN BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SATURN BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_SATURN",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
    },
    -- SaturnTrail module
    {   
        Name = "SaturnTrail",
        --Parent = "SaturnBarycenter",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "SATURN BARYCENTER",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = {0.85,0.75,0.51 },
            TropicalOrbitPeriod = 10746.94 ,
            EarthOrbitRatio = 29.424,
            DayLength = 10.656,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        },
    }
}
