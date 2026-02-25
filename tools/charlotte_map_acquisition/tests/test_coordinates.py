#!/usr/bin/env python3
"""
Unit tests for coordinate transformation module.

Tests verify:
- Origin transformation (I-485 center -> Unreal 0,0,0)
- Axis conventions (X=North, Y=East, Z=Up)
- Distance calculations
- Tile coordinate generation
- Coordinate validation
"""

import pytest
import math
import sys
from pathlib import Path

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from tools.charlotte_map_acquisition.coordinates import (
    latlon_to_unreal,
    unreal_to_latlon,
    latlon_to_utm,
    utm_to_latlon,
    latlon_to_tile,
    tile_to_latlon,
    get_tile_bounds,
    count_tiles_in_bounds,
    validate_coordinates,
    get_origin_utm,
    get_bounds_info,
    charlotte_bounds
)


class TestOriginTransformation:
    """Tests for origin point transformation."""

    def test_origin_transforms_to_zero(self):
        """Center of I-485 bounds should transform to (0, 0, 0) in Unreal."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        x, y, z = latlon_to_unreal(origin_lat, origin_lon, 0)

        # Should be within 1 cm of origin
        assert abs(x) < 1, f"X should be ~0, got {x}"
        assert abs(y) < 1, f"Y should be ~0, got {y}"
        assert abs(z) < 1, f"Z should be ~0, got {z}"

    def test_origin_with_elevation(self):
        """Origin with elevation should have correct Z value."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        _, _, z = latlon_to_unreal(origin_lat, origin_lon, 100)  # 100m elevation

        # Z should be 100m in centimeters
        assert abs(z - 10000) < 1, f"Z should be ~10000cm, got {z}"


class TestAxisConventions:
    """Tests for Unreal axis conventions."""

    def test_north_offset_positive_x(self):
        """Moving north should increase X (Unreal forward)."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        x1, _, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        x2, _, _ = latlon_to_unreal(origin_lat + 0.1, origin_lon, 0)  # 0.1 degree north

        assert x2 > x1, "Moving north should increase X"

    def test_east_offset_positive_y(self):
        """Moving east should increase Y (Unreal right)."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        _, y1, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        _, y2, _ = latlon_to_unreal(origin_lat, origin_lon + 0.1, 0)  # 0.1 degree east

        assert y2 > y1, "Moving east should increase Y"

    def test_elevation_becomes_z(self):
        """Elevation should map directly to Z axis in centimeters."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        _, _, z1 = latlon_to_unreal(origin_lat, origin_lon, 0)
        _, _, z2 = latlon_to_unreal(origin_lat, origin_lon, 100)  # 100m up

        assert z2 == z1 + 10000, f"100m elevation should add 10000cm to Z, got {z2 - z1}"

    def test_south_offset_negative_x(self):
        """Moving south should decrease X."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        x1, _, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        x2, _, _ = latlon_to_unreal(origin_lat - 0.1, origin_lon, 0)  # 0.1 degree south

        assert x2 < x1, "Moving south should decrease X"

    def test_west_offset_negative_y(self):
        """Moving west should decrease Y."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        _, y1, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        _, y2, _ = latlon_to_unreal(origin_lat, origin_lon - 0.1, 0)  # 0.1 degree west

        assert y2 < y1, "Moving west should decrease Y"


class TestDistanceCalculations:
    """Tests for distance calculations via coordinate transforms."""

    def test_known_latitude_distance(self):
        """1 degree latitude should be approximately 111km."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        x1, _, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        x2, _, _ = latlon_to_unreal(origin_lat + 1.0, origin_lon, 0)  # 1 degree north

        distance_cm = x2 - x1
        distance_km = distance_cm / 100000

        # 1 degree latitude ≈ 111km at Charlotte's latitude
        assert 108 < distance_km < 114, f"Expected ~111km, got {distance_km:.1f}km"

    def test_known_longitude_distance(self):
        """1 degree longitude at Charlotte should be approximately 91km."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        _, y1, _ = latlon_to_unreal(origin_lat, origin_lon, 0)
        _, y2, _ = latlon_to_unreal(origin_lat, origin_lon + 1.0, 0)  # 1 degree east

        distance_cm = y2 - y1
        distance_km = distance_cm / 100000

        # 1 degree longitude at 35N ≈ 91km (cos(35) * 111)
        assert 88 < distance_km < 94, f"Expected ~91km, got {distance_km:.1f}km"


class TestInverseTransformation:
    """Tests for inverse coordinate transformations."""

    def test_roundtrip_origin(self):
        """Origin should round-trip correctly."""
        origin_lat = charlotte_bounds['center_lat']
        origin_lon = charlotte_bounds['center_lon']

        # Forward then inverse
        x, y, z = latlon_to_unreal(origin_lat, origin_lon, 100)
        lat, lon, elev = unreal_to_latlon(x, y, z)

        assert abs(lat - origin_lat) < 0.0001, f"Latitude mismatch: {lat} vs {origin_lat}"
        assert abs(lon - origin_lon) < 0.0001, f"Longitude mismatch: {lon} vs {origin_lon}"
        assert abs(elev - 100) < 0.01, f"Elevation mismatch: {elev} vs 100"

    def test_roundtrip_offset_point(self):
        """Offset point should round-trip correctly."""
        test_lat = 35.3
        test_lon = -80.7
        test_elev = 200

        # Forward then inverse
        x, y, z = latlon_to_unreal(test_lat, test_lon, test_elev)
        lat, lon, elev = unreal_to_latlon(x, y, z)

        assert abs(lat - test_lat) < 0.0001
        assert abs(lon - test_lon) < 0.0001
        assert abs(elev - test_elev) < 0.01


class TestTileCoordinates:
    """Tests for tile coordinate calculations."""

    def test_latlon_to_tile_zoom_15(self):
        """Verify tile calculation at zoom 15 for Charlotte center."""
        # Charlotte center
        x, y = latlon_to_tile(charlotte_bounds['center_lat'], charlotte_bounds['center_lon'], 15)

        # These are known values for Charlotte at zoom 15
        # Tile coordinates should be positive integers
        assert isinstance(x, int)
        assert isinstance(y, int)
        assert x > 0
        assert y > 0

        # Rough expected range for Charlotte at zoom 15 (calculated from lat/lon)
        # lon=-80.843 at zoom 15 -> x ~ 9025
        # lat=35.227 at zoom 15 -> y ~ 12954
        assert 9000 < x < 9100, f"X tile out of expected range: {x}"
        assert 12900 < y < 13000, f"Y tile out of expected range: {y}"

    def test_higher_zoom_more_tiles(self):
        """Higher zoom should produce larger tile numbers (roughly 2x)."""
        x15, y15 = latlon_to_tile(35.227, -80.843, 15)
        x16, y16 = latlon_to_tile(35.227, -80.843, 16)

        # Zoom 16 tiles are 2x zoom 15 (plus possible offset of 0 or 1)
        assert x16 in [x15 * 2, x15 * 2 + 1], f"Zoom 16 X should be ~2x zoom 15: {x15} vs {x16}"
        assert y16 in [y15 * 2, y15 * 2 + 1], f"Zoom 16 Y should be ~2x zoom 15: {y15} vs {y16}"

    def test_tile_bounds_returns_valid_range(self):
        """get_tile_bounds should return valid ranges for Charlotte."""
        x_min, y_min, x_max, y_max = get_tile_bounds(charlotte_bounds, zoom=15)

        assert x_min < x_max, f"X min should be less than max: {x_min} vs {x_max}"
        assert y_min < y_max, f"Y min should be less than max: {y_min} vs {y_max}"

        # Zoom 15 for Charlotte I-485 area (~32x32 km = 1024 sq km)
        # At zoom 15, each tile is ~4.8km, so we expect ~7x7 = 49 tiles per side
        # Total tiles should be around 1000-1500
        tile_count = (x_max - x_min + 1) * (y_max - y_min + 1)
        assert 800 < tile_count < 1500, f"Unexpected tile count: {tile_count}"

    def test_count_tiles_in_bounds(self):
        """Tile count should match get_tile_bounds calculation."""
        count = count_tiles_in_bounds(charlotte_bounds, zoom=15)
        x_min, y_min, x_max, y_max = get_tile_bounds(charlotte_bounds, zoom=15)

        expected = (x_max - x_min + 1) * (y_max - y_min + 1)
        assert count == expected, f"Count mismatch: {count} vs {expected}"


class TestCoordinateValidation:
    """Tests for coordinate validation."""

    def test_valid_center_coords(self):
        """Center coordinates should validate."""
        assert validate_coordinates(
            charlotte_bounds['center_lat'],
            charlotte_bounds['center_lon']
        ) == True

    def test_valid_corner_coords(self):
        """Corner coordinates should validate."""
        # All corners should be valid
        assert validate_coordinates(charlotte_bounds['north'], charlotte_bounds['east']) == True
        assert validate_coordinates(charlotte_bounds['north'], charlotte_bounds['west']) == True
        assert validate_coordinates(charlotte_bounds['south'], charlotte_bounds['east']) == True
        assert validate_coordinates(charlotte_bounds['south'], charlotte_bounds['west']) == True

    def test_invalid_north(self):
        """Coordinates too far north should fail."""
        assert validate_coordinates(36.0, -80.843) == False

    def test_invalid_south(self):
        """Coordinates too far south should fail."""
        assert validate_coordinates(34.0, -80.843) == False

    def test_invalid_east(self):
        """Coordinates too far east should fail."""
        assert validate_coordinates(35.227, -79.0) == False

    def test_invalid_west(self):
        """Coordinates too far west should fail."""
        assert validate_coordinates(35.227, -82.0) == False


class TestBoundsInfo:
    """Tests for bounds info utility function."""

    def test_get_bounds_info_returns_dict(self):
        """get_bounds_info should return a dictionary."""
        info = get_bounds_info()
        assert isinstance(info, dict)

    def test_bounds_info_has_required_keys(self):
        """Bounds info should have all required keys."""
        info = get_bounds_info()

        required_keys = ['name', 'center', 'corners', 'dimensions', 'utm_zone', 'utm_epsg']
        for key in required_keys:
            assert key in info, f"Missing key: {key}"

    def test_bounds_info_dimensions(self):
        """Bounds info should include area calculation."""
        info = get_bounds_info()

        assert 'width_m' in info['dimensions']
        assert 'height_m' in info['dimensions']
        assert 'area_km2' in info['dimensions']

        # Charlotte I-485 interior is roughly 1000 sq km
        # (0.29 lat x 0.35 lon ≈ 32km x 35km ≈ 1120 sq km at Charlotte latitude)
        area = info['dimensions']['area_km2']
        assert 800 < area < 1200, f"Unexpected area: {area} km2"


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
