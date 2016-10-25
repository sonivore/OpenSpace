return {
    -- Pluto barycenter module
    {
        Name = "PlutoBarycenter",
        Parent = "SolarSystemBarycenter",
        -- Scene Radius in KM:
        SceneRadius = 1.0E+6,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "PLUTO BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "SUN",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                },
            },
        },
    },
    -- Pluto module
    {   
        Name = "Pluto",
        Parent = "PlutoBarycenter",
         -- Scene Radius in KM:
        SceneRadius = 1.0E+4,
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_PLUTO",
            Body = "PLUTO",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 1.187 , 6 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/pluto_highres_180.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "PLUTO BARYCENTER",
                --Reference = "ECLIPJ2000",
                Observer = "PLUTO BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_PLUTO",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
    },
    {   
        Name = "Charon",
        Parent = "PlutoBarycenter",
         -- Scene Radius in KM:
        SceneRadius = 6.0E+3,                
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_CHARON",
            Body = "CHARON",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 6.035 , 5 },
                Segments = 200
            },
            Textures = {
                Type = "simple",
                Color = "textures/Charon-Text.png",
                --Color = "textures/charon-texture.jpg",
            },
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "CHARON",
                --Reference = "ECLIPJ2000",
                Observer = "PLUTO BARYCENTER",
                Kernels = {
                    "${OPENSPACE_DATA}/spice/plu055.bsp"
                }
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_CHARON",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
    },
    -- CharonTrail module
    {   
        Name = "CharonTrail",
        Parent = "PlutoBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "CHARON",
            Frame = "GALACTIC",
            --Frame = "ECLIPJ2000",
            Observer = "PLUTO BARYCENTER",
            RGB = {0.00,0.62,1.00},
            TropicalOrbitPeriod = 120 ,
            EarthOrbitRatio = 0.03,
            DayLength = 1,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        },
        GuiName = "/Solar/CharonTrail"
    },    
    -- PlutoTrail module
    {   
        Name = "PlutoTrail",
        Parent = "SolarSystemBarycenter",
        --Parent = "PlutoBarycenter",
        --Renderable = {
        --    Type = "RenderableTrail",
        --    Body = "PLUTO BARYCENTER",
        --    Frame = "GALACTIC",
        --    --Frame = "ECLIPJ2000",
        --    Observer = "SUN",
        --    RGB = {0.58, 0.61, 1.00},
        --    TropicalOrbitPeriod = 59799.9 ,
        --    EarthOrbitRatio = 163.73,
        --    DayLength = 16.11,
        --    Textures = {
        --        Type = "simple",
        --        Color = "${COMMON_MODULE}/textures/glare_blue.png",
        --        -- need to add different texture
        --    },  
        --},
        Renderable = {
            Type = "RenderableTrailNew",
            -- Spice
            Body = "PLUTO BARYCENTER",
            Frame = "GALACTIC",
            Observer = "SUN",
            -- Optional rendering properties
            LineColor = { 0.58, 0.61, 1.00 },
            PointColor = { 0.58, 0.61, 1.00 },
            LineFade = 0.0, -- [0,1]
            RenderPart = 0.13,
            LineWidth = 2,
            ShowTimeStamps = false,
            RenderFullTrail = false,
            -- Time interval
            TimeRange = {
                Start = "2016 SEP 8 23:05:00.50",
                End = "2023 SEP 24 12:00:00",
            },
            SampleDeltaTime = 3600, -- Seconds between each point
            SubSamples = 0, 
        },
        GuiName = "/Solar/PlutoTrail"
    },
}
