# Charlotte City Level

## Overview
Main gameplay level for Zombie Taxi, featuring Charlotte urban environment with World Partition streaming.

## Level Path
`/Game/Maps/Charlotte_City.umap`

## Setup Checklist

- [ ] Create level from Open World template
- [ ] Configure World Partition grid (256m cells, 768m range)
- [ ] Enable One File Per Actor
- [ ] Place GSDCityLevelActor with configuration
- [ ] Create HLOD layers for foliage, buildings, terrain
- [ ] Import city tiles from blender_gsd
- [ ] Generate HLODs
- [ ] Test streaming with player character

## Grid Settings

```
World Partition Setup > Runtime Grids:
  [0]:
    Grid Name: "MainGrid"
    Cell Size: 25600
    Loading Range: 76800
    Block on Slow Streaming: true
    Priority: 0
    Debug Color: (R=1, G=0, B=0, A=1)
```

## Content Organization

```
/Game/
├── Maps/
│   └── Charlotte_City.umap
├── CityTiles/
│   ├── Downtown/
│   ├── Midtown/
│   └── Suburbs/
├── HLOD/
│   ├── HLOD_Foliage.uasset
│   ├── HLOD_Buildings.uasset
│   └── HLOD_Terrain.uasset
└── __ExternalActors__/
    └── Maps/
        └── Charlotte_City/
```

## Next Steps

After level setup:
1. Import city tiles (Plan 04 interface)
2. Generate HLODs (Plan 02 workflow)
3. Test streaming performance
4. Configure streaming sources on vehicles

**Note:** The actual .umap file must be created in the Unreal Editor.
This documentation provides the configuration values and workflow.
