return {
    -- Barycenter module
    {
        Name = "UranusBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 1.0E+9,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "URANUS BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        }
    },
    -- RenderableGlobe module
    {   
        Name = "Uranus",
        Parent = "UranusBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 3.0E+8,
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_URANUS",
                DestinationFrame = "GALACTIC"
            }
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = { 25559000, 25559000, 24973000 },
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Texture",
                        FilePath = "textures/uranus.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Tag = { "planet_solarSystem", "planet_giants" }
    },
    -- Trail module
    {   
        Name = "UranusTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "URANUS BARYCENTER",
                Observer = "SUN"
            },
            Color = {0.60, 0.95, 1.00 },
            Period = 30588.740,
            Resolution = 1000
        },
        Tag = { "planetTrail_solarSystem", "planetTrail_giants" }
    }
}
