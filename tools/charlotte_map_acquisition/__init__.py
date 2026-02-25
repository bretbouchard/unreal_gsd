"""
Charlotte Map Data Acquisition Package

This package provides tools for acquiring map tile and terrain data
for the Charlotte I-485 corridor, transforming coordinates for use
in Unreal Engine.

Coordinate System Flow:
    WGS84 (lat/lon) -> UTM Zone 17N (meters) -> Unreal (centimeters)

Usage:
    from charlotte_map_acquisition import coordinates
    from charlotte_map_acquisition.config import charlotte_bounds
"""

__version__ = "0.1.0"
__author__ = "GSD Development Team"

# Import key modules for convenience
from . import coordinates
from . import config
