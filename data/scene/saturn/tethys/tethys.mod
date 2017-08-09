return {
    {
        Name = "Tethys",
        Parent = "SaturnBarycenter",
        -- SphereOfInfluency unit is meters                
		SphereOfInfluency = 5.0E+6,
        Renderable = {
            Type = "RenderableGlobe",
            Radii = 531100,
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Tethys Texture",
                        FilePath = "textures/tethys.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "TETHYS",
                Observer = "SATURN BARYCENTER",
                Kernels = "${OPENSPACE_DATA}/spice/sat375.bsp"
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_ENCELADUS",
                DestinationFrame = "GALACTIC"
            }
        }
    },
    {
        Name = "TethysTrail",
        Parent = "SaturnBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "TETHYS",
                Observer = "SATURN BARYCENTER",
            },
            Color = { 0.5, 0.3, 0.3 },
            Period = 45 / 24,
            Resolution = 1000
        }
    }
}
