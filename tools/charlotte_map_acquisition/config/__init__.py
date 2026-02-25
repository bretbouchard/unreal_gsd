"""
Configuration module for Charlotte Map Data Acquisition.

Provides access to:
    - charlotte_bounds: I-485 corridor bounding box
    - zoom_levels: Tile zoom configurations
"""

import json
from pathlib import Path

# Configuration directory
CONFIG_DIR = Path(__file__).parent


def load_config(filename: str) -> dict:
    """Load a JSON configuration file.

    Args:
        filename: Name of the config file (e.g., 'charlotte_bounds.json')

    Returns:
        Parsed JSON configuration as dictionary
    """
    config_path = CONFIG_DIR / filename
    with open(config_path, 'r') as f:
        return json.load(f)


# Pre-loaded configurations
charlotte_bounds = load_config('charlotte_bounds.json')
zoom_levels = load_config('zoom_levels.json')

__all__ = ['charlotte_bounds', 'zoom_levels', 'load_config']
