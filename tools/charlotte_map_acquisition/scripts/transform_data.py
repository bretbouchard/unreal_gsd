#!/usr/bin/env python3
"""
Coordinate Transform Script for Vector Data

Transforms GeoJSON vector data from WGS84 to Unreal Engine coordinate space
using the shared coordinate pipeline.

Coordinate System Flow:
    WGS84 (lat/lon) -> UTM Zone 17N (meters) -> Unreal (centimeters)

Unreal Axis Conventions:
    - X-axis: North direction (UTM Northing)
    - Y-axis: East direction (UTM Easting)
    - Z-axis: Up direction (Elevation)

Usage:
    python transform_data.py [--input FILE] [--output FILE]

    Or as module:
    from transform_data import transform_geojson_to_unreal

Output:
    Transformed vectors saved to ../data/processed/vectors/unreal/
"""

import os
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any, Union, Callable

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from coordinates import (
    latlon_to_unreal,
    unreal_to_latlon,
    get_origin_utm,
    validate_coordinates,
    charlotte_bounds
)

# Default paths
SCRIPT_DIR = Path(__file__).parent.resolve()
DATA_DIR = SCRIPT_DIR.parent / "data"
CONFIG_DIR = SCRIPT_DIR.parent / "config"
PROCESSED_VECTORS_DIR = DATA_DIR / "processed" / "vectors"
UNREAL_VECTORS_DIR = PROCESSED_VECTORS_DIR / "unreal"


# Type aliases
Coordinate = Tuple[float, float, float]  # (x, y, z) or (lon, lat, elev)
CoordTransformer = Callable[[float, float, float], Coordinate]


def transform_point(
    lon: float,
    lat: float,
    elevation: float = 0.0
) -> Coordinate:
    """Transform a single WGS84 coordinate to Unreal space.

    Args:
        lon: Longitude in decimal degrees (WGS84)
        lat: Latitude in decimal degrees (WGS84)
        elevation: Elevation in meters (default: 0)

    Returns:
        Tuple of (unreal_x, unreal_y, unreal_z) in centimeters
    """
    return latlon_to_unreal(lat, lon, elevation)


def transform_linestring(
    coords: List[List[float]],
    elevation: float = 0.0
) -> List[Coordinate]:
    """Transform a LineString's coordinates to Unreal space.

    Args:
        coords: List of [lon, lat] or [lon, lat, elev] coordinates
        elevation: Default elevation if not provided in coords

    Returns:
        List of (unreal_x, unreal_y, unreal_z) coordinates
    """
    transformed = []
    for coord in coords:
        lon = coord[0]
        lat = coord[1]
        elev = coord[2] if len(coord) > 2 else elevation
        transformed.append(transform_point(lon, lat, elev))
    return transformed


def transform_polygon(
    rings: List[List[List[float]]],
    elevation: float = 0.0
) -> List[List[Coordinate]]:
    """Transform a Polygon's rings to Unreal space.

    A polygon consists of:
        - 1 exterior ring (required)
        - 0+ interior rings (holes)

    Args:
        rings: List of rings, each ring is a list of [lon, lat] coords
        elevation: Default elevation if not provided in coords

    Returns:
        List of transformed rings, each ring is a list of coordinates
    """
    return [
        transform_linestring(ring, elevation)
        for ring in rings
    ]


def transform_geometry(
    geometry: Dict[str, Any],
    default_elevation: float = 0.0
) -> Dict[str, Any]:
    """Transform a GeoJSON geometry to Unreal coordinates.

    Supports:
        - Point
        - LineString
        - Polygon
        - MultiPoint
        - MultiLineString
        - MultiPolygon
        - GeometryCollection

    Args:
        geometry: GeoJSON geometry object
        default_elevation: Default elevation for 2D coordinates

    Returns:
        New geometry dict with Unreal coordinates
    """
    geom_type = geometry.get("type")
    coords = geometry.get("coordinates", [])

    if geom_type == "Point":
        # Point: [lon, lat] or [lon, lat, elev]
        elev = coords[2] if len(coords) > 2 else default_elevation
        new_coords = list(transform_point(coords[0], coords[1], elev))

    elif geom_type == "LineString":
        # LineString: [[lon, lat], ...]
        new_coords = [list(c) for c in transform_linestring(coords, default_elevation)]

    elif geom_type == "Polygon":
        # Polygon: [[[lon, lat], ...], ...] (rings)
        new_coords = [
            [list(c) for c in ring]
            for ring in transform_polygon(coords, default_elevation)
        ]

    elif geom_type == "MultiPoint":
        # MultiPoint: [[lon, lat], ...]
        new_coords = [
            list(transform_point(c[0], c[1], c[2] if len(c) > 2 else default_elevation))
            for c in coords
        ]

    elif geom_type == "MultiLineString":
        # MultiLineString: [[[lon, lat], ...], ...]
        new_coords = [
            [list(c) for c in transform_linestring(line, default_elevation)]
            for line in coords
        ]

    elif geom_type == "MultiPolygon":
        # MultiPolygon: [[[[lon, lat], ...], ...], ...]
        new_coords = [
            [
                [list(c) for c in ring]
                for ring in transform_polygon(polygon, default_elevation)
            ]
            for polygon in coords
        ]

    elif geom_type == "GeometryCollection":
        # GeometryCollection: {"geometries": [...]}
        new_coords = None  # Handled separately below
        new_geometries = [
            transform_geometry(g, default_elevation)
            for g in geometry.get("geometries", [])
        ]
        return {
            "type": "GeometryCollection",
            "geometries": new_geometries
        }

    else:
        raise ValueError(f"Unsupported geometry type: {geom_type}")

    return {
        "type": geom_type,
        "coordinates": new_coords
    }


def transform_feature(
    feature: Dict[str, Any],
    default_elevation: float = 0.0
) -> Dict[str, Any]:
    """Transform a GeoJSON feature to Unreal coordinates.

    Preserves all properties and adds transformation metadata.

    Args:
        feature: GeoJSON feature object
        default_elevation: Default elevation for 2D coordinates

    Returns:
        New feature dict with transformed geometry and metadata
    """
    geometry = feature.get("geometry")
    properties = feature.get("properties", {})

    # Transform geometry
    new_geometry = transform_geometry(geometry, default_elevation)

    # Add original CRS info to properties
    new_properties = properties.copy()
    new_properties["_original_crs"] = "EPSG:4326"
    new_properties["_transformed_crs"] = "Unreal"
    new_properties["_coordinate_convention"] = "X=North, Y=East, Z=Up (cm)"

    return {
        "type": "Feature",
        "geometry": new_geometry,
        "properties": new_properties
    }


def transform_geojson_to_unreal(
    input_path: Path,
    output_path: Optional[Path] = None,
    default_elevation: float = 0.0
) -> Tuple[Path, Dict[str, Any]]:
    """Transform a GeoJSON file to Unreal coordinate space.

    Args:
        input_path: Path to input GeoJSON file
        output_path: Output path. If None, uses default location.
        default_elevation: Default elevation for 2D coordinates

    Returns:
        Tuple of (output_path, stats_dict)

    Raises:
        FileNotFoundError: If input file doesn't exist
        ValueError: If file is not valid GeoJSON
    """
    input_path = Path(input_path)

    # Load GeoJSON
    with open(input_path, 'r') as f:
        geojson = json.load(f)

    # Validate GeoJSON structure
    if "type" not in geojson:
        raise ValueError("Invalid GeoJSON: missing 'type' field")

    # Transform based on GeoJSON type
    if geojson["type"] == "FeatureCollection":
        features = geojson.get("features", [])
        new_features = [
            transform_feature(f, default_elevation)
            for f in features
        ]
        new_geojson = {
            "type": "FeatureCollection",
            "features": new_features
        }

    elif geojson["type"] == "Feature":
        new_geojson = transform_feature(geojson, default_elevation)

    elif geojson["type"] in ("Point", "LineString", "Polygon",
                              "MultiPoint", "MultiLineString", "MultiPolygon",
                              "GeometryCollection"):
        new_geojson = transform_geometry(geojson, default_elevation)

    else:
        raise ValueError(f"Unsupported GeoJSON type: {geojson['type']}")

    # Add metadata
    new_geojson["_metadata"] = {
        "source_file": input_path.name,
        "transform": "WGS84 -> UTM17N -> Unreal",
        "origin_wgs84": {
            "lat": charlotte_bounds["center_lat"],
            "lon": charlotte_bounds["center_lon"]
        },
        "origin_unreal": [0, 0, 0],
        "default_elevation_m": default_elevation
    }

    # Determine output path
    if output_path is None:
        UNREAL_VECTORS_DIR.mkdir(parents=True, exist_ok=True)
        stem = input_path.stem
        output_path = UNREAL_VECTORS_DIR / f"{stem}_unreal.geojson"

    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Save transformed GeoJSON
    with open(output_path, 'w') as f:
        json.dump(new_geojson, f, indent=2)

    # Calculate stats
    stats = {
        "input_file": str(input_path),
        "output_file": str(output_path),
        "features_transformed": (
            len(new_features) if geojson["type"] == "FeatureCollection" else 1
        )
    }

    return output_path, stats


def validate_origin_transform() -> bool:
    """Verify that the origin transforms to (0, 0, 0).

    Returns:
        True if origin is correct, False otherwise
    """
    origin_lat = charlotte_bounds["center_lat"]
    origin_lon = charlotte_bounds["center_lon"]

    unreal_x, unreal_y, unreal_z = latlon_to_unreal(origin_lat, origin_lon, 0)

    # Allow small floating point tolerance
    tolerance = 1.0  # 1 cm tolerance
    is_valid = (
        abs(unreal_x) < tolerance and
        abs(unreal_y) < tolerance and
        abs(unreal_z) < tolerance
    )

    if not is_valid:
        print(f"WARNING: Origin transform failed!")
        print(f"  Expected: (0, 0, 0)")
        print(f"  Got: ({unreal_x}, {unreal_y}, {unreal_z})")

    return is_valid


def get_coordinate_range(
    geojson_path: Path
) -> Dict[str, Tuple[float, float]]:
    """Get the range of coordinates in a transformed GeoJSON.

    Args:
        geojson_path: Path to transformed GeoJSON file

    Returns:
        Dict with min/max for each axis: {"x": (min, max), "y": ..., "z": ...}
    """
    with open(geojson_path, 'r') as f:
        geojson = json.load(f)

    min_x = min_y = min_z = float('inf')
    max_x = max_y = max_z = float('-inf')

    def process_coords(coords, geom_type):
        nonlocal min_x, min_y, min_z, max_x, max_y, max_z

        if geom_type == "Point":
            x, y, z = coords[0], coords[1], coords[2] if len(coords) > 2 else 0
            min_x = min(min_x, x); max_x = max(max_x, x)
            min_y = min(min_y, y); max_y = max(max_y, y)
            min_z = min(min_z, z); max_z = max(max_z, z)

        elif geom_type in ("LineString", "MultiPoint"):
            for c in coords:
                x, y, z = c[0], c[1], c[2] if len(c) > 2 else 0
                min_x = min(min_x, x); max_x = max(max_x, x)
                min_y = min(min_y, y); max_y = max(max_y, y)
                min_z = min(min_z, z); max_z = max(max_z, z)

        elif geom_type in ("Polygon", "MultiLineString"):
            for ring in coords:
                for c in ring:
                    x, y, z = c[0], c[1], c[2] if len(c) > 2 else 0
                    min_x = min(min_x, x); max_x = max(max_x, x)
                    min_y = min(min_y, y); max_y = max(max_y, y)
                    min_z = min(min_z, z); max_z = max(max_z, z)

        elif geom_type == "MultiPolygon":
            for polygon in coords:
                for ring in polygon:
                    for c in ring:
                        x, y, z = c[0], c[1], c[2] if len(c) > 2 else 0
                        min_x = min(min_x, x); max_x = max(max_x, x)
                        min_y = min(min_y, y); max_y = max(max_y, y)
                        min_z = min(min_z, z); max_z = max(max_z, z)

    def process_geometry(geometry):
        geom_type = geometry.get("type")
        coords = geometry.get("coordinates")

        if geom_type == "GeometryCollection":
            for g in geometry.get("geometries", []):
                process_geometry(g)
        else:
            process_coords(coords, geom_type)

    if geojson["type"] == "FeatureCollection":
        for feature in geojson.get("features", []):
            process_geometry(feature.get("geometry", {}))
    elif geojson["type"] == "Feature":
        process_geometry(geojson.get("geometry", {}))
    else:
        process_geometry(geojson)

    return {
        "x": (min_x, max_x),
        "y": (min_y, max_y),
        "z": (min_z, max_z)
    }


def main():
    """Main entry point for command-line usage."""
    parser = argparse.ArgumentParser(
        description="Transform GeoJSON vector data to Unreal coordinate space"
    )
    parser.add_argument(
        "--input", "-i",
        type=Path,
        help="Input GeoJSON file (default: i485_boundary.geojson)"
    )
    parser.add_argument(
        "--output", "-o",
        type=Path,
        help="Output GeoJSON file"
    )
    parser.add_argument(
        "--elevation", "-e",
        type=float,
        default=0.0,
        help="Default elevation in meters (default: 0)"
    )
    parser.add_argument(
        "--validate",
        action="store_true",
        help="Validate origin transform only"
    )

    args = parser.parse_args()

    # Validate origin transform
    print("Validating origin transform...")
    if validate_origin_transform():
        print("  Origin (center of I-485) transforms to (0, 0, 0) - OK")
    else:
        print("  ERROR: Origin transform validation failed!")
        sys.exit(1)

    if args.validate:
        print("\nValidation complete.")
        return

    # Determine input file
    if args.input:
        input_path = args.input
    else:
        # Default to I-485 boundary
        input_path = PROCESSED_VECTORS_DIR / "i485_boundary.geojson"

    if not input_path.exists():
        print(f"ERROR: Input file not found: {input_path}")
        print("\nTo extract highway boundary, run:")
        print("  python extract_highway.py")
        sys.exit(1)

    print(f"\nTransforming: {input_path.name}")

    try:
        # Transform GeoJSON
        output_path, stats = transform_geojson_to_unreal(
            input_path,
            output_path=args.output,
            default_elevation=args.elevation
        )

        print(f"  Output: {output_path}")
        print(f"  Features: {stats['features_transformed']}")

        # Get coordinate range
        coord_range = get_coordinate_range(output_path)
        print(f"\nCoordinate range (cm):")
        print(f"  X (North): {coord_range['x'][0]:.1f} to {coord_range['x'][1]:.1f}")
        print(f"  Y (East):  {coord_range['y'][0]:.1f} to {coord_range['y'][1]:.1f}")
        print(f"  Z (Up):    {coord_range['z'][0]:.1f} to {coord_range['z'][1]:.1f}")

        # Check for outliers (coordinates > 100km from origin)
        max_dist = 100 * 100 * 1000  # 100 km in cm
        for axis, (min_v, max_v) in coord_range.items():
            if abs(min_v) > max_dist or abs(max_v) > max_dist:
                print(f"  WARNING: {axis.upper()}-axis values exceed 100km from origin!")

        print("\nDone!")

    except Exception as e:
        print(f"  ERROR: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
