return {
    -- Barycenter module
    {
        Name = "JupiterBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 2.0E+9,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "JUPITER BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        }
    },
    -- RenderableGlobe module
    {
        Name = "Jupiter",
        Parent = "JupiterBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 4.0E+8,
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_JUPITER",
                DestinationFrame = "GALACTIC"
            },
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = { 71492000, 71492000, 66854000 },
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Jupiter Texture",
                        FilePath = "textures/jupiter.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Tag = { "planet_solarSystem", "planet_giants" },
    },
    -- Trail module
    {   
        Name = "JupiterTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "JUPITER BARYCENTER",
                Observer = "SUN"
            },
            Color = { 0.8, 0.7, 0.7 },
            Period = 4330.595,
            Resolution = 1000
        },
        Tag = { "planetTrail_solarSystem", "planetTrail_giants" }
    }
}
