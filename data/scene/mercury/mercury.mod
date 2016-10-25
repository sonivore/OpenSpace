return {
    -- Mercury barycenter module
    {
        Name = "MercuryBarycenter",
        Parent = "SolarSystemBarycenter",
        -- Scene Radius in KM:
        SceneRadius = 1.0E+5,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "MERCURY BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SUN",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
        },
    },
    -- Mercury module
    {   
        Name = "Mercury",
        Parent = "MercuryBarycenter",
        -- Scene Radius in KM:
        SceneRadius = 5.0E+4,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_MERCURY",
            Body = "MERCURY",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 2.4397, 6 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/mercury.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "MERCURY",
                --Reference = "ECLIPJ2000",
                Observer = "MERCURY BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_MERCURY",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        }
    },
    -- MercuryTrail module
    {   
        Name = "MercuryTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "MERCURY",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = {0.6, 0.5, 0.5 },
            TropicalOrbitPeriod = 87.968 ,
            EarthOrbitRatio = 0.241,
            DayLength = 4222.6,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        }
    }
}
