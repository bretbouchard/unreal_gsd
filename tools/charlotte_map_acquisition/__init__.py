"""
Charlotte Map Data Acquisition Package

This package provides tools for acquiring map tile and terrain data
for the Charlotte I-485 corridor, transforming coordinates for use
in Unreal Engine.

Coordinate System Flow:
    WGS84 (lat/lon) -> UTM Zone 17N (meters) -> Unreal (centimeters)

Usage:
    # Run full pipeline
    python -m tools.charlotte_map_acquisition

    # Run specific steps
    python -m tools.charlotte_map_acquisition --steps tiles,dem

    # List available steps
    python -m tools.charlotte_map_acquisition --list

    # As a module
    from tools.charlotte_map_acquisition import CharlotteMapPipeline
    from tools.charlotte_map_acquisition import latlon_to_tile, latlon_to_unreal
"""

__version__ = "0.1.0"
__author__ = "GSD Development Team"

# Import key modules for convenience
from . import coordinates
from . import config

# Import main classes and functions for easy access
from .pipeline import CharlotteMapPipeline
from .coordinates import (
    latlon_to_tile,
    latlon_to_unreal,
    charlotte_bounds
)

__all__ = [
    # Main pipeline class
    'CharlotteMapPipeline',

    # Coordinate functions
    'latlon_to_tile',
    'latlon_to_unreal',
    'charlotte_bounds',

    # Modules
    'coordinates',
    'config',
]
