return {
    -- Barycenter module
    {
        Name = "NeptuneBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 1.0E+9,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "NEPTUNE BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        }
    },
    -- RenderableGlobe module
    {   
        Name = "Neptune",
        Parent = "NeptuneBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 3.0E+8,
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_NEPTUNE",
                DestinationFrame = "GALACTIC"
            }
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = { 24764000, 24764000, 24314000 },
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Texture",
                        FilePath = "textures/neptune.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Tag = { "planet_solarSystem", "planet_giants" },

    },
    -- Trail module
    {   
        Name = "NeptuneTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "NEPTUNE BARYCENTER",
                Observer = "SUN"
            },
            Color = {0.2, 0.5, 1.0 },
            Period  = 60200,
            Resolution = 1000
        },
        Tag = { "planetTrail_solarSystem", "planetTrail_giants" }
    }
}
