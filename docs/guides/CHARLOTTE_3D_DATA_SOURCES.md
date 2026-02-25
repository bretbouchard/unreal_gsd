# Charlotte 3D Data Sources - Full Handoff

> Research findings and recommendations for acquiring photorealistic 3D city data for Zombie Taxi / Charlotte gameplay level.

**Source:** Research session from `blender_gsd` project
**Date:** 2025-02-25
**Status:** Ready for implementation

---

## Executive Summary

The goal: **Own the Charlotte 3D city stack** for modification (zombification) and Unreal Engine integration.

### Key Finding

**Google Photorealistic 3D Tiles** (the super-realistic 3D from the Bilawal Sidhu video) cannot be owned/exported - it's streaming-only with restrictive ToS.

**Recommended path:** Use OpenStreetMap data (already started in `blender_gsd`) + optional USGS LiDAR for terrain accuracy.

---

## Decision Matrix

| Factor | BlenderGIS + OSM | USGS LiDAR | MapsModelsImporter | Google 3D Tiles API |
|--------|------------------|------------|--------------------|---------------------|
| **Legal Ownership** | FULL | FULL | GRAY (ToS violation) | NO (streaming only) |
| **Can Modify/Edit** | Yes | Yes | Yes | No |
| **Commercial Use** | Yes (OSM attribution) | Yes (public domain) | Risky | No |
| **Offline Use** | Yes | Yes | Yes | No |
| **Cost** | FREE | FREE | FREE | Paid API |
| **Texture Quality** | None (white models) | None | EXCELLENT | EXCELLENT |
| **Geometry Accuracy** | Medium | EXCELLENT | EXCELLENT | EXCELLENT |
| **Charlotte Coverage** | Yes | Yes | Yes | Yes |
| **Unreal Import** | Easy (FBX/glTF) | Medium | Easy | Complex (Cesium plugin) |
| **Future-Proof** | Yes (local files) | Yes (local files) | Maybe (Google could block) | No (API can change) |

### Recommended Approach

**Primary:** BlenderGIS + OSM (you already own this data)
**Enhancement:** USGS LiDAR for terrain accuracy (optional)
**Avoid:** Google 3D Tiles API (no ownership)
**Caution:** MapsModelsImporter (legal risk for commercial)

---

## What We Already Have (blender_gsd)

The `blender_gsd` project already has a working Charlotte pipeline:

### Existing Assets

| Asset | Location | Status |
|-------|----------|--------|
| GeoJSON Building Data | `blender_gsd/lib/charlotte_digital_twin/data/` | 1,069+ Uptown buildings |
| OSM Parser | `blender_gsd/lib/charlotte_digital_twin/data_acquisition/` | Working |
| Building Generator | `blender_gsd/lib/charlotte_digital_twin/buildings/geojson_importer.py` | Working |
| LED Facade System | `blender_gsd/lib/charlotte_digital_twin/buildings/led_facade.py` | Working |
| PBR Materials | `blender_gsd/lib/charlotte_digital_twin/buildings/building_materials.py` | Working |
| Road Network (I-277) | `blender_gsd/projects/charlotte/` | Working |
| Coordinate System | WGS84 -> Local meters | Working |

### What's Missing for Unreal

| Gap | Priority | Effort |
|-----|----------|--------|
| FBX/glTF export function | HIGH | 2-4 hours |
| Zombification pass (damage geometry) | HIGH | 4-8 hours |
| LOD generation (high/med/low) | MEDIUM | 2-4 hours |
| Texture packs for zombie aesthetic | MEDIUM | 4-8 hours |
| USGS LiDAR terrain (optional) | LOW | 8-16 hours |

---

## Data Source Details

### 1. OpenStreetMap (RECOMMENDED - Already Using)

**What it provides:**
- Building footprints (polygons)
- Building heights (partial coverage, ~24%)
- Floor counts (partial coverage, ~11%)
- Road network (complete)
- POI data (names, types)

**Charlotte Coverage:**
- 1,069 Uptown buildings (curated)
- 4,002 named buildings
- 387,329 total buildings in Charlotte metro

**License:** ODbL (Open Database License)
- Free to use with attribution
- Can modify and distribute
- Commercial use allowed

**How to get more:**
```bash
# Overpass API query for Charlotte buildings
[out:json][timeout:60];
(
  way["building"](35.20,-80.87,35.25,-80.80);
  relation["building"](35.20,-80.87,35.25,-80.80);
);
out body;
>;
out skel qt;
```

### 2. USGS 3DEP LiDAR (OPTIONAL - Terrain Accuracy)

**What it provides:**
- Real-world terrain elevation (1m resolution)
- Building footprints with accurate heights
- Vegetation, ground detail
- Point cloud data (LAZ format)

**Charlotte Coverage:** Yes - North Carolina has full 3DEP coverage

**How to access:**
1. [The National Map Viewer](https://www.usgs.gov/tools/national-map-viewer)
2. OpenTopography.org (free for academic, paid for commercial)
3. AWS Public Dataset (EPT format)

**Workflow:**
```
USGS 3DEP (LAZ point cloud)
    ↓
CloudCompare / PDAL (convert to mesh)
    ↓
Blender (cleanup, integrate)
    ↓
Unreal Engine
```

**License:** Public domain - free for any use

### 3. Google Photorealistic 3D Tiles (NOT RECOMMENDED)

**Why not:**
- Streaming only - cannot download/export
- ToS prohibits extracting geometry
- 3-hour session limit per connection
- API costs scale with usage
- Cannot redistribute modified versions

**If you just need visualization (not modification):**
- Cesium for Unreal plugin (free)
- Google Maps API key with billing
- URL: `https://tile.googleapis.com/v1/3dtiles/root.json?key=YOUR_KEY`

### 4. MapsModelsImporter (GRAY AREA)

**What it is:**
- Extracts Google Maps 3D geometry from browser
- Uses RenderDoc to capture GPU data
- Imports into Blender as mesh

**Pros:**
- Photorealistic geometry AND textures
- Full mesh you can edit
- Works for Charlotte

**Cons:**
- Violates Google ToS
- Risky for commercial projects
- Google could block at any time

**GitHub:** https://github.com/eliemichel/MapsModelsImporter

---

## Recommended Pipeline for Unreal

### Phase 1: Export from blender_gsd (Week 1)

```python
# Add to blender_gsd/lib/charlotte_digital_twin/buildings/exporter.py

import bpy
import os

def export_to_unreal(
    output_dir: str,
    format: str = "FBX",
    collection_name: str = "OSM_Buildings",
    apply_modifiers: bool = True,
):
    """Export Charlotte buildings to Unreal-compatible format."""

    collection = bpy.data.collections.get(collection_name)
    if not collection:
        raise ValueError(f"Collection '{collection_name}' not found")

    # Select all objects in collection
    bpy.ops.object.select_all(action='DESELECT')
    for obj in collection.objects:
        obj.select_set(True)

    output_path = os.path.join(output_dir, f"charlotte_buildings.{format.lower()}")

    if format == "FBX":
        bpy.ops.export_scene.fbx(
            filepath=output_path,
            use_selection=True,
            apply_unit_scale=True,
            axis_forward='-Z',  # Unreal forward
            axis_up='Z',        # Unreal up
            object_types={'MESH'},
            use_mesh_modifiers=apply_modifiers,
        )
    elif format == "GLTF":
        bpy.ops.export_scene.gltf(
            filepath=output_path,
            use_selection=True,
            export_format='GLTF_SEPARATE',
        )

    return output_path
```

### Phase 2: Zombification Pass (Week 1-2)

```python
# Add to blender_gsd/projects/charlotte/scripts/zombify_charlotte.py

import bpy
import random
from mathutils import Vector

def zombify_buildings(
    damage_probability: float = 0.3,
    max_damage: float = 0.4,
):
    """Add zombie apocalypse damage to buildings."""

    buildings = bpy.data.collections.get("OSM_Buildings")
    if not buildings:
        return

    for obj in buildings.objects:
        if random.random() < damage_probability:
            damage_type = random.choice([
                "broken_windows",
                "crumbled_corner",
                "fire_damage",
                "collapsed_roof",
            ])
            apply_damage(obj, damage_type, max_damage)

def apply_damage(building, damage_type: str, intensity: float):
    """Apply specific damage type to a building."""

    if damage_type == "broken_windows":
        # Darken faces to simulate broken glass
        _apply_broken_windows(building, intensity)
    elif damage_type == "crumbled_corner":
        # Proportional edit to deform geometry
        _apply_crumbled_corner(building, intensity)
    elif damage_type == "fire_damage":
        # Add black/soot material patches
        _apply_fire_damage(building, intensity)
    elif damage_type == "collapsed_roof":
        # Remove top portion of building
        _apply_collapsed_roof(building, intensity)
```

### Phase 3: LOD Generation (Week 2)

```python
# Add to blender_gsd/lib/charlotte_digital_twin/buildings/lod_generator.py

def generate_lods(building_obj, lod_levels=[1.0, 0.5, 0.25, 0.1]):
    """Generate LOD variants for Unreal."""

    lods = []

    for i, ratio in enumerate(lod_levels):
        lod = building_obj.copy()
        lod.data = building_obj.data.copy()
        lod.name = f"{building_obj.name}_LOD{i}"

        # Add decimation modifier
        mod = lod.modifiers.new(name="Decimate", type='DECIMATE')
        mod.ratio = ratio

        bpy.context.collection.objects.link(lod)
        lods.append(lod)

    return lods
```

### Phase 4: Unreal Import (Week 2-3)

1. Import FBX to `/Game/CityTiles/Downtown/`
2. Configure LODs in Static Mesh Editor
3. Generate HLODs for World Partition
4. Set up streaming distances

---

## File Locations (blender_gsd)

```
blender_gsd/
├── lib/charlotte_digital_twin/
│   ├── buildings/
│   │   ├── geojson_importer.py      # Main generator
│   │   ├── led_facade.py            # LED system
│   │   ├── building_materials.py    # PBR materials
│   │   ├── exporter.py              # TODO: Add for Unreal
│   │   └── lod_generator.py         # TODO: Add for Unreal
│   ├── data/
│   │   ├── charlotte_uptown_buildings.geojson  # 1,069 buildings
│   │   ├── charlotte_buildings_named.geojson   # 4,002 buildings
│   │   └── charlotte_buildings.geojson         # 387,329 buildings
│   └── data_acquisition/
│       └── osm_downloader.py        # OSM API client
├── projects/charlotte/
│   ├── scripts/
│   │   ├── charlotte_osm_scene.py   # Scene generator
│   │   └── zombify_charlotte.py     # TODO: Add damage pass
│   ├── scenes/
│   │   └── charlotte_osm_scene.blend
│   └── maps/
│       └── charlotte-merged.osm     # Raw OSM data
└── docs/
    └── CHARLOTTE_GEOJSON_WORKFLOW.md
```

---

## Unreal Integration Points

### Coordinate System Alignment

| System | Origin | Units | Axis |
|--------|--------|-------|------|
| OSM/GeoJSON | WGS84 (lat/lon) | Degrees | N/A |
| blender_gsd | 35.227°N, 80.843°W | Meters | Y-up, Z-forward |
| unreal_gsd | 35.227°N, 80.843°W | Centimeters | Z-up, X-forward |

**Existing conversion in `unreal_gsd/tools/charlotte_map_acquisition/coordinates.py`:**
```python
def latlon_to_unreal(lat, lon, altitude):
    """Convert WGS84 to Unreal centimeters."""
    # WGS84 -> UTM Zone 17N -> Unreal
    utm_x, utm_y = wgs84_to_utm(lat, lon)
    unreal_x = (utm_x - ORIGIN_UTM_X) * 100  # cm
    unreal_y = (utm_y - ORIGIN_UTM_Y) * 100  # cm
    unreal_z = altitude * 100  # cm
    return unreal_x, unreal_y, unreal_z
```

### World Partition Grid

From `unreal_gsd/Plugins/GSD_CityStreaming/Docs/CharlotteCityLevel.md`:
- Cell Size: 25,600 cm (256m)
- Loading Range: 76,800 cm (768m)

### Import Checklist

- [ ] Export FBX from blender_gsd with Z-up axis
- [ ] Import to `/Game/CityTiles/Downtown/`
- [ ] Configure LOD screen sizes
- [ ] Add to World Partition data layers
- [ ] Generate HLODs
- [ ] Test streaming with player character

---

## Next Steps for unreal_gsd Team

### Immediate (This Week)

1. **Review blender_gsd Charlotte pipeline**
   - Read: `blender_gsd/docs/CHARLOTTE_GEOJSON_WORKFLOW.md`
   - Test: Run `charlotte_osm_scene.py` in Blender

2. **Request export scripts**
   - FBX export with Unreal axis conventions
   - LOD generation for building meshes
   - Zombification pass for damage

### Short-term (Next 2 Weeks)

3. **Import pipeline**
   - Create `/Game/CityTiles/` structure
   - Test FBX import workflow
   - Verify coordinate alignment

4. **HLOD setup**
   - Configure HLOD layers for buildings
   - Set up World Partition streaming
   - Test performance in Charlotte_City level

### Medium-term (Next Month)

5. **Texture pipeline**
   - Create zombie texture packs
   - Material instances for damage states
   - Blend spaces for LOD transitions

6. **USGS LiDAR evaluation** (optional)
   - Download sample data from OpenTopography
   - Test terrain accuracy vs. flat ground
   - Decide if needed for gameplay

---

## Contacts & Resources

### Documentation

- [Google Photorealistic 3D Tiles](https://developers.google.com/maps/documentation/tile/3d-tiles)
- [Cesium for Unreal](https://cesium.com/learn/unreal/)
- [USGS 3DEP](https://www.usgs.gov/3d-elevation-program)
- [OpenStreetMap Wiki](https://wiki.openstreetmap.org/wiki/Buildings)

### Tools

- **BlenderGIS**: https://github.com/domlysz/BlenderGIS
- **MapsModelsImporter**: https://github.com/eliemichel/MapsModelsImporter
- **CloudCompare**: https://www.danielgm.net/cc/ (LiDAR processing)
- **OpenTopography**: https://opentopography.org/

### Reference Video

Bilawal Sidhu's "WorldView" demo (the viral Palantir-style dashboard):
- https://www.youtube.com/watch?v=rXvU7bPJ8n4
- Uses Google 3D Tiles + CesiumJS
- Shows what's possible with photorealistic 3D
- **Note:** Cannot own/export this data - just for inspiration

---

## Summary

| Decision | Recommendation |
|----------|----------------|
| **Primary data source** | OSM (already have in blender_gsd) |
| **Terrain enhancement** | USGS 3DEP (optional, public domain) |
| **Avoid** | Google 3D Tiles API (no ownership) |
| **Zombification** | Custom Blender pass on OSM geometry |
| **Export format** | FBX with Unreal axis conventions |
| **LOD strategy** | Generate in Blender, configure in Unreal |

**Bottom line:** We already own the Charlotte data via OSM. We just need to add export/zombification/LOD scripts and import to Unreal. No need for Google's API or gray-area extraction tools.
