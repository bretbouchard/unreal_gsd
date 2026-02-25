"""
Coordinate Transformation Module

Transforms coordinates between WGS84, UTM Zone 17N, and Unreal Engine
coordinate systems for the Charlotte I-485 corridor.

Coordinate System Flow:
    WGS84 (lat/lon) -> UTM Zone 17N (meters) -> Unreal (centimeters)

Unreal Axis Conventions:
    - X-axis: North direction (UTM Northing)
    - Y-axis: East direction (UTM Easting)
    - Z-axis: Up direction (Elevation)

Why UTM Zone 17N:
    Charlotte, NC is located at approximately 80.8W longitude, which falls
    within UTM Zone 17N (covering 84W to 78W). This zone provides minimal
    distortion for the Charlotte metropolitan area.

Usage:
    from charlotte_map_acquisition import coordinates

    # Transform lat/lon to Unreal coordinates
    unreal_x, unreal_y, unreal_z = coordinates.latlon_to_unreal(35.227, -80.843, 200)

    # Get tile coordinates for map tiles
    tile_x, tile_y = coordinates.latlon_to_tile(35.227, -80.843, 17)

    # Validate coordinates are within Charlotte bounds
    is_valid = coordinates.validate_coordinates(35.227, -80.843)
"""

import math
from typing import Tuple, Optional
from pathlib import Path

try:
    from pyproj import CRS, Transformer
    PYPROJ_AVAILABLE = True
except ImportError:
    PYPROJ_AVAILABLE = False
    CRS = None
    Transformer = None

from .config import charlotte_bounds


# Coordinate Reference Systems
WGS84_CRS = CRS.from_epsg(4326) if PYPROJ_AVAILABLE else None  # GPS coordinates
UTM17N_CRS = CRS.from_epsg(32617) if PYPROJ_AVAILABLE else None  # UTM Zone 17N

# Create transformer (always_xy=True means input/output is (lon, lat) order)
_transformer: Optional[Transformer] = None


def _get_transformer() -> Transformer:
    """Get or create the WGS84 to UTM17N transformer.

    Returns:
        Transformer instance for coordinate conversion
    """
    global _transformer
    if _transformer is None:
        if not PYPROJ_AVAILABLE:
            raise ImportError(
                "pyproj is required for coordinate transformations. "
                "Install with: pip install pyproj>=3.6.0"
            )
        _transformer = Transformer.from_crs(
            WGS84_CRS,
            UTM17N_CRS,
            always_xy=True  # Input/output in (lon, lat) order
        )
    return _transformer


def get_origin_utm() -> Tuple[float, float]:
    """Get the UTM coordinates of the origin point (I-485 center).

    The origin is set to the center of the I-485 corridor, which becomes
    (0, 0, 0) in Unreal Engine coordinates.

    Returns:
        Tuple of (easting, northing) in UTM meters
    """
    center_lon = charlotte_bounds['center_lon']
    center_lat = charlotte_bounds['center_lat']

    transformer = _get_transformer()
    easting, northing = transformer.transform(center_lon, center_lat)

    return easting, northing


def latlon_to_utm(lat: float, lon: float) -> Tuple[float, float]:
    """Convert WGS84 lat/lon to UTM Zone 17N coordinates.

    Args:
        lat: Latitude in decimal degrees (WGS84)
        lon: Longitude in decimal degrees (WGS84)

    Returns:
        Tuple of (easting, northing) in meters
    """
    transformer = _get_transformer()
    easting, northing = transformer.transform(lon, lat)
    return easting, northing


def utm_to_latlon(easting: float, northing: float) -> Tuple[float, float]:
    """Convert UTM Zone 17N coordinates to WGS84 lat/lon.

    Args:
        easting: UTM easting in meters
        northing: UTM northing in meters

    Returns:
        Tuple of (latitude, longitude) in decimal degrees
    """
    transformer = _get_transformer()
    lon, lat = transformer.transform(easting, northing, direction='INVERSE')
    return lat, lon


def latlon_to_unreal(
    lat: float,
    lon: float,
    elevation_meters: float = 0.0
) -> Tuple[float, float, float]:
    """Convert WGS84 lat/lon/elevation to Unreal Engine coordinates.

    This is the primary coordinate transformation function used by all
    pipeline scripts. It applies the full transformation pipeline:
        1. WGS84 (lat/lon) -> UTM Zone 17N (meters)
        2. Apply origin offset (center of I-485 becomes 0,0)
        3. Convert meters to Unreal centimeters (* 100)

    Unreal Axis Mapping:
        - X = North direction (UTM Northing offset)
        - Y = East direction (UTM Easting offset)
        - Z = Up direction (Elevation)

    Args:
        lat: Latitude in decimal degrees (WGS84)
        lon: Longitude in decimal degrees (WGS84)
        elevation_meters: Elevation/altitude in meters (default: 0)

    Returns:
        Tuple of (unreal_x, unreal_y, unreal_z) in centimeters
        - unreal_x: North-South position (X-axis in Unreal)
        - unreal_y: East-West position (Y-axis in Unreal)
        - unreal_z: Vertical position (Z-axis in Unreal)
    """
    # Get UTM coordinates
    easting, northing = latlon_to_utm(lat, lon)

    # Get origin offset
    origin_easting, origin_northing = get_origin_utm()

    # Calculate offset from origin in meters
    offset_east = easting - origin_easting  # East-West offset
    offset_north = northing - origin_northing  # North-South offset

    # Convert to Unreal centimeters
    # X = North, Y = East, Z = Up
    unreal_x = offset_north * 100  # North -> X
    unreal_y = offset_east * 100   # East -> Y
    unreal_z = elevation_meters * 100  # Up -> Z

    return unreal_x, unreal_y, unreal_z


def unreal_to_latlon(
    unreal_x: float,
    unreal_y: float,
    unreal_z: float = 0.0
) -> Tuple[float, float, float]:
    """Convert Unreal Engine coordinates to WGS84 lat/lon/elevation.

    Inverse transformation of latlon_to_unreal.

    Args:
        unreal_x: North-South position in centimeters (X-axis)
        unreal_y: East-West position in centimeters (Y-axis)
        unreal_z: Vertical position in centimeters (Z-axis)

    Returns:
        Tuple of (latitude, longitude, elevation_meters)
    """
    # Get origin offset
    origin_easting, origin_northing = get_origin_utm()

    # Convert from centimeters to meters and apply offset
    easting = (unreal_y / 100) + origin_easting  # Y -> East
    northing = (unreal_x / 100) + origin_northing  # X -> North
    elevation_meters = unreal_z / 100  # Z -> Up

    # Convert to lat/lon
    lat, lon = utm_to_latlon(easting, northing)

    return lat, lon, elevation_meters


def latlon_to_tile(lat: float, lon: float, zoom: int) -> Tuple[int, int]:
    """Convert WGS84 lat/lon to XYZ tile coordinates.

    Uses the standard Web Mercator tile coordinate system used by
    OpenStreetMap, Google Maps, Mapbox, and other tile providers.

    Args:
        lat: Latitude in decimal degrees (-85.0511 to 85.0511)
        lon: Longitude in decimal degrees (-180 to 180)
        zoom: Zoom level (0-22, typically 15-18 for city data)

    Returns:
        Tuple of (tile_x, tile_y) tile coordinates

    Raises:
        ValueError: If coordinates are out of valid range
    """
    # Validate latitude range for Web Mercator
    if lat < -85.0511 or lat > 85.0511:
        raise ValueError(
            f"Latitude {lat} out of Web Mercator range (-85.0511 to 85.0511)"
        )

    # Convert to radians
    lat_rad = math.radians(lat)

    # Calculate tile coordinates
    n = 2 ** zoom
    tile_x = int((lon + 180.0) / 360.0 * n)
    tile_y = int(
        (1.0 - math.asinh(math.tan(lat_rad)) / math.pi) / 2.0 * n
    )

    # Clamp to valid range
    tile_x = max(0, min(n - 1, tile_x))
    tile_y = max(0, min(n - 1, tile_y))

    return tile_x, tile_y


def tile_to_latlon(
    tile_x: int,
    tile_y: int,
    zoom: int
) -> Tuple[float, float]:
    """Convert XYZ tile coordinates to WGS84 lat/lon (center of tile).

    Args:
        tile_x: Tile X coordinate
        tile_y: Tile Y coordinate
        zoom: Zoom level

    Returns:
        Tuple of (latitude, longitude) at tile center
    """
    n = 2 ** zoom

    # Get center of tile
    lon = (tile_x + 0.5) / n * 360.0 - 180.0
    lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * (tile_y + 0.5) / n)))
    lat = math.degrees(lat_rad)

    return lat, lon


def get_tile_bounds(
    bounds: Optional[dict] = None,
    zoom: int = 17
) -> Tuple[int, int, int, int]:
    """Get tile coordinate range for a bounding box.

    Args:
        bounds: Bounding box dict with north, south, east, west keys.
                Defaults to Charlotte I-485 bounds.
        zoom: Zoom level

    Returns:
        Tuple of (min_x, min_y, max_x, max_y) tile coordinates
    """
    if bounds is None:
        bounds = charlotte_bounds

    # Get corner tile coordinates
    # Note: North is higher latitude, but lower tile Y
    min_x, max_y = latlon_to_tile(bounds['south'], bounds['west'], zoom)
    max_x, min_y = latlon_to_tile(bounds['north'], bounds['east'], zoom)

    return min_x, min_y, max_x, max_y


def count_tiles_in_bounds(
    bounds: Optional[dict] = None,
    zoom: int = 17
) -> int:
    """Count the number of tiles needed to cover a bounding box.

    Args:
        bounds: Bounding box dict with north, south, east, west keys.
                Defaults to Charlotte I-485 bounds.
        zoom: Zoom level

    Returns:
        Number of tiles needed
    """
    min_x, min_y, max_x, max_y = get_tile_bounds(bounds, zoom)

    width = max_x - min_x + 1
    height = max_y - min_y + 1

    return width * height


def validate_coordinates(lat: float, lon: float) -> bool:
    """Check if coordinates are within Charlotte I-485 bounds.

    Args:
        lat: Latitude in decimal degrees
        lon: Longitude in decimal degrees

    Returns:
        True if coordinates are within bounds, False otherwise
    """
    return (
        charlotte_bounds['south'] <= lat <= charlotte_bounds['north'] and
        charlotte_bounds['west'] <= lon <= charlotte_bounds['east']
    )


def get_bounds_info() -> dict:
    """Get detailed information about the Charlotte bounds.

    Returns:
        Dictionary with bounds info including UTM coordinates and tile counts
    """
    origin_easting, origin_northing = get_origin_utm()

    # Get corner UTM coordinates
    sw_easting, sw_northing = latlon_to_utm(
        charlotte_bounds['south'],
        charlotte_bounds['west']
    )
    ne_easting, ne_northing = latlon_to_utm(
        charlotte_bounds['north'],
        charlotte_bounds['east']
    )

    # Calculate dimensions
    width_m = ne_easting - sw_easting
    height_m = ne_northing - sw_northing

    return {
        'name': charlotte_bounds['name'],
        'center': {
            'lat': charlotte_bounds['center_lat'],
            'lon': charlotte_bounds['center_lon'],
            'utm_easting': origin_easting,
            'utm_northing': origin_northing
        },
        'corners': {
            'sw': {'lat': charlotte_bounds['south'], 'lon': charlotte_bounds['west']},
            'ne': {'lat': charlotte_bounds['north'], 'lon': charlotte_bounds['east']}
        },
        'dimensions': {
            'width_m': width_m,
            'height_m': height_m,
            'area_km2': (width_m * height_m) / 1_000_000
        },
        'utm_zone': charlotte_bounds['utm_zone'],
        'utm_epsg': charlotte_bounds['utm_epsg']
    }


# Module-level constants for quick access
ORIGIN_UTM = None  # Lazy-loaded via get_origin_utm()

__all__ = [
    'latlon_to_unreal',
    'unreal_to_latlon',
    'latlon_to_utm',
    'utm_to_latlon',
    'latlon_to_tile',
    'tile_to_latlon',
    'get_tile_bounds',
    'count_tiles_in_bounds',
    'validate_coordinates',
    'get_origin_utm',
    'get_bounds_info',
]
