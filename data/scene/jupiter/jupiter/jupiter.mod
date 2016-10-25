return {
    -- Jupiter barycenter module
    {
        Name = "JupiterBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SceneRadius unit is KM                
        SceneRadius = 2.0E+6,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "JUPITER BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            },
        },
    },
    -- Jupiter module
    {   
        Name = "Jupiter",
        Parent = "JupiterBarycenter",
        -- SceneRadius unit is KM
        SceneRadius = 4.0E+5,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_JUPITER",
            Body = "JUPITER BARYCENTER",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 6.9911, 7 },
                Segments = 200
            },
            Textures = {
                Type = "simple",
                Color = "textures/jupiter.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "StaticTranslation",
                Position = {0, 0, 0}, -- jupiter is at its barycenter
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_JUPITER",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        }
    },
    -- JupiterTrail module
    {   
        Name = "JupiterTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "JUPITER BARYCENTER",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = { 0.8, 0.7, 0.7 },
            TropicalOrbitPeriod = 4330.595 ,
            EarthOrbitRatio = 11.857,
            DayLength = 9.9259,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        }
    }
}
