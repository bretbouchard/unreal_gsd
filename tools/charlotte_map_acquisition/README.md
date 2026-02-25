# Charlotte Map Data Acquisition

Python package for acquiring map tile and terrain data for the Charlotte I-485 corridor, with coordinate transformations for Unreal Engine integration.

## Overview

This package provides the foundational infrastructure for automated map data acquisition:

- **Configuration Management**: Charlotte I-485 bounding box and zoom level configurations
- **Coordinate Transformations**: WGS84 -> UTM Zone 17N -> Unreal centimeters
- **Tile Utilities**: Lat/lon to XYZ tile coordinate conversions

### Coordinate System Flow

```
WGS84 (lat/lon) -> UTM Zone 17N (meters) -> Unreal Engine (centimeters)
    EPSG:4326         EPSG:32617                Origin at I-485 center
```

### Unreal Axis Conventions

- **X-axis**: North direction
- **Y-axis**: East direction
- **Z-axis**: Up direction

## Installation

```bash
cd tools/charlotte_map_acquisition
pip install -r requirements.txt
```

### Dependencies

| Package | Purpose |
|---------|---------|
| gdal | GIS data processing |
| pyproj | Coordinate transformations |
| osmnx | OpenStreetMap data |
| geopandas | Geospatial dataframes |
| shapely | Geometry operations |
| rasterio | Raster data handling |
| requests | HTTP requests for tile downloads |
| Pillow | Image processing |
| numpy | Numerical operations |

## Quick Start

```python
from charlotte_map_acquisition import coordinates
from charlotte_map_acquisition.config import charlotte_bounds, zoom_levels

# Get Charlotte I-485 bounds
print(f"Area: {charlotte_bounds['name']}")
print(f"Bounds: N={charlotte_bounds['north']}, S={charlotte_bounds['south']}")

# Transform coordinates for Unreal
lat, lon = 35.227, -80.843  # Charlotte center
unreal_x, unreal_y, unreal_z = coordinates.latlon_to_unreal(lat, lon, 0)
print(f"Unreal coords: X={unreal_x}, Y={unreal_y}, Z={unreal_z}")

# Get tile coordinates for a zoom level
zoom = 17
tile_x, tile_y = coordinates.latlon_to_tile(lat, lon, zoom)
print(f"Tile: {tile_x}, {tile_y} at zoom {zoom}")
```

## Configuration

### Charlotte Bounds (`config/charlotte_bounds.json`)

| Property | Value | Description |
|----------|-------|-------------|
| north | 35.37 | Northern latitude |
| south | 35.08 | Southern latitude |
| east | -80.65 | Eastern longitude |
| west | -81.00 | Western longitude |
| center_lat | 35.227 | Center latitude (origin) |
| center_lon | -80.843 | Center longitude (origin) |
| utm_zone | 17N | UTM zone for Charlotte |
| utm_epsg | 32617 | EPSG code for UTM zone |

### Zoom Levels (`config/zoom_levels.json`)

| Zoom | Tile Size (m) | Est. Tiles | Use Case |
|------|---------------|------------|----------|
| 15 | 4800 | 400 | Development |
| 16 | 2400 | 1600 | Development |
| 17 | 1200 | 6400 | Production |
| 18 | 600 | 25600 | High Quality |

## Project Structure

```
charlotte_map_acquisition/
    __init__.py           # Package initialization
    coordinates.py        # Coordinate transformation module
    config/
        __init__.py       # Config module
        charlotte_bounds.json
        zoom_levels.json
    requirements.txt
    README.md
```

## Documentation

For detailed research and implementation notes, see the research document in the planning phase directory.

## License

Internal project - Unreal GSD Infrastructure
