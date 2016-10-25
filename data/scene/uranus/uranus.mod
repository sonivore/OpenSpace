return {
    -- Uranus barycenter module
    {
        Name = "UranusBarycenter",
        Parent = "SolarSystemBarycenter",
         -- SceneRadius unit is KM                
		SceneRadius = 1.0E+6,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "URANUS BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SUN",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
        },
    },

    -- Uranus module
    {   
        Name = "Uranus",
        Parent = "UranusBarycenter",
        -- SceneRadius unit is KM                
		SceneRadius = 3.0E+5,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_URANUS",
            Body = "URANUS",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 2.5559, 7 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/uranus.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "URANUS BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "URANUS BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_URANUS",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
    },
    -- UranusTrail module
    {   
        Name = "UranusTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "URANUS BARYCENTER",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = {0.60,0.95,1.00 },
            TropicalOrbitPeriod = 30588.740 ,
            EarthOrbitRatio = 83.749,
            DayLength = 17.24,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        },
    }
}
