return {
    -- Earth barycenter module
    {
        Name = "EarthBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SceneRadius unit is KM                
		SceneRadius = 4.0E+5, 
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "EARTH BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SUN",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
        },
    },
    -- Earth module
    {   
        Name = "Earth",
        Parent = "EarthBarycenter",
        -- SceneRadius unit is KM
		SceneRadius = 1.0E+5,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_EARTH",
            Body = "EARTH",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 6.371, 6 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/earth_bluemarble.jpg",
                Night = "textures/earth_night.jpg",
                Height = "textures/earth_bluemarble_height.jpg"
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "EARTH",
                --Reference = "ECLIPJ2000",
                Observer = "EARTH BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_EARTH",
                DestinationFrame = "GALACTIC",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
        GuiName = "/Solar/Planets/Earth"
    },
    -- EarthTrail module
    {   
        Name = "EarthTrail",
        --Parent = "EarthBarycenter",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "EARTH",
            --Reference = "ECLIPJ2000",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = { 0.5, 0.8, 1.0},
            TropicalOrbitPeriod = 365.242,
            EarthOrbitRatio = 1,
            DayLength = 24
        },
        GuiName = "/Solar/EarthTrail"
    },
    --[[
    {
        Name = "EarthMarker",
        Parent = "Earth",
        Renderable = {
            Type = "RenderablePlane",
            Size = {3.0, 11.0},
            Origin = "Center",
            Billboard = true,
            Texture = "textures/marker.png",
            BlendMode = "Additive"
        },
		Ephemeris = {
            Type = "Static",
            Position = {0, 0, 0, 5}
        }
    }
    ]]
}
