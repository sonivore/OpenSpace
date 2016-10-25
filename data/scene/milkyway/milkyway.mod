return {
    {
        Name = "MilkyWay",
        Parent = "Root",
        -- KM
        SceneRadius = 20.0E+19,
        Ephemeris = {
            Type = "Static"
        },
        Renderable = {
            Type = "RenderableSphere",
            Size = { 10, 22 },
            Segments = 40,
            Texture = "textures/DarkUniverse_mellinger_8k.jpg",
            Orientation = "Inside/Outside"
        }
    }
}
