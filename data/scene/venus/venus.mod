return {
    -- Barycenter module
    {
        Name = "VenusBarycenter",
        Parent = "SolarSystemBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 1.0E+8,
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "VENUS BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        }
    },
    -- RenderableGlobe module
    {   
        Name = "Venus",
        Parent = "VenusBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 5.0E+7,
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_VENUS",
                DestinationFrame = "GALACTIC"
            },
            Translation = {
                Type = "SpiceTranslation",
                Target = "VENUS",
                Observer = "VENUS BARYCENTER",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = { 6051900, 6051900, 6051800 },
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Venus Texture",
                        FilePath = "textures/venus.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Tag = { "planet_solarSystem", "planet_terrestrial" },
    },
    -- Trail module
    {   
        Name = "VenusTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "VENUS BARYCENTER",
                Observer = "SUN"
            },
            Color = { 1.0, 0.5, 0.2 },
            Period = 224.695,
            Resolution = 1000
        },
        Tag = { "planetTrail_solarSystem", "planetTrail_terrestrial" }
    }
}
