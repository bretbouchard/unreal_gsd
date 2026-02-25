#!/usr/bin/env python3
"""
Map tile downloader for Charlotte I-485 corridor.

Downloads map tiles from OpenStreetMap or other tile providers with proper
rate limiting to avoid server blocks.

Usage:
    python -m tools.charlotte_map_acquisition.scripts.download_tiles
"""

import json
import math
import os
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import List, Tuple, Dict, Optional
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError


@dataclass
class TileCoord:
    """Tile coordinate with zoom level."""
    z: int
    x: int
    y: int

    def to_path(self) -> str:
        """Convert to file path: z/x/y.png"""
        return f"{self.z}/{self.x}/{self.y}.png"


class TileDownloader:
    """
    Downloads map tiles with rate limiting and parallel execution.

    Based on research patterns:
    - 250ms delay between requests to avoid rate limiting
    - Max 4 concurrent workers
    - Proper User-Agent header required by OSM
    - Resume capability (skip existing tiles)
    """

    def __init__(
        self,
        base_url: str = "https://tile.openstreetmap.org",
        rate_limit_delay: float = 0.25,
        max_workers: int = 4,
        timeout: int = 30
    ):
        """
        Initialize tile downloader.

        Args:
            base_url: Tile server URL (default: OSM)
            rate_limit_delay: Seconds between requests (default: 0.25)
            max_workers: Max concurrent downloads (default: 4)
            timeout: Request timeout in seconds (default: 30)
        """
        self.base_url = base_url.rstrip("/")
        self.rate_limit_delay = rate_limit_delay
        self.max_workers = max_workers
        self.timeout = timeout
        self.headers = {
            "User-Agent": "CharlotteMapAcquisition/0.1.0 (Educational/Research Project)"
        }

    @staticmethod
    def latlon_to_tile(lat: float, lon: float, zoom: int) -> Tuple[int, int]:
        """
        Convert latitude/longitude to tile coordinates.

        Uses standard Web Mercator projection (EPSG:3857).

        Args:
            lat: Latitude in degrees
            lon: Longitude in degrees
            zoom: Zoom level (0-19)

        Returns:
            Tuple of (x, y) tile coordinates
        """
        n = 2 ** zoom
        x = int((lon + 180) / 360 * n)
        lat_rad = math.radians(lat)
        y = int((1 - math.asinh(math.tan(lat_rad)) / math.pi) / 2 * n)
        return (x, y)

    def get_tiles_for_bounds(
        self,
        bounds: Dict[str, float],
        zoom: int
    ) -> List[TileCoord]:
        """
        Get all tile coordinates for a bounding box at a zoom level.

        Args:
            bounds: Dict with 'north', 'south', 'east', 'west' keys
            zoom: Zoom level

        Returns:
            List of TileCoord objects covering the bounds
        """
        # Get corner tiles
        nw_tile = self.latlon_to_tile(bounds['north'], bounds['west'], zoom)
        se_tile = self.latlon_to_tile(bounds['south'], bounds['east'], zoom)

        tiles = []
        for x in range(nw_tile[0], se_tile[0] + 1):
            for y in range(nw_tile[1], se_tile[1] + 1):
                tiles.append(TileCoord(z=zoom, x=x, y=y))

        return tiles

    def download_tile(
        self,
        tile: TileCoord,
        output_dir: Path,
        retry_count: int = 3
    ) -> bool:
        """
        Download a single tile with rate limiting and error handling.

        Args:
            tile: TileCoord to download
            output_dir: Base output directory
            retry_count: Number of retries on failure (default: 3)

        Returns:
            True if successful, False otherwise
        """
        # Create tile path
        tile_path = output_dir / tile.to_path()

        # Skip if already exists (resume capability)
        if tile_path.exists():
            return True

        # Create directory structure
        tile_path.parent.mkdir(parents=True, exist_ok=True)

        # Build URL
        url = f"{self.base_url}/{tile.z}/{tile.x}/{tile.y}.png"

        # Download with retries
        for attempt in range(retry_count):
            try:
                # Rate limiting
                time.sleep(self.rate_limit_delay)

                # Make request
                request = Request(url, headers=self.headers)
                with urlopen(request, timeout=self.timeout) as response:
                    data = response.read()

                # Save tile
                tile_path.write_bytes(data)
                return True

            except HTTPError as e:
                if e.code in (403, 429):
                    # Rate limited - wait longer
                    wait_time = (attempt + 1) * 5
                    print(f"Rate limited, waiting {wait_time}s...")
                    time.sleep(wait_time)
                elif e.code == 404:
                    # Tile doesn't exist
                    print(f"Tile not found: {tile.to_path()}")
                    return False
                else:
                    print(f"HTTP error {e.code} for {tile.to_path()}")

            except URLError as e:
                print(f"Network error for {tile.to_path()}: {e.reason}")
                time.sleep(2)

            except Exception as e:
                print(f"Unexpected error for {tile.to_path()}: {e}")

        print(f"Failed to download {tile.to_path()} after {retry_count} attempts")
        return False

    def download_all(
        self,
        bounds: Dict[str, float],
        zoom_levels: List[int],
        output_dir: Path,
        progress_interval: int = 50
    ) -> Dict[str, int]:
        """
        Download all tiles for bounds at specified zoom levels.

        Args:
            bounds: Dict with 'north', 'south', 'east', 'west' keys
            zoom_levels: List of zoom levels to download
            output_dir: Base output directory
            progress_interval: Print progress every N tiles (default: 50)

        Returns:
            Dict with 'success', 'failed', 'skipped' counts
        """
        stats = {'success': 0, 'failed': 0, 'skipped': 0}

        for zoom in zoom_levels:
            print(f"\n=== Zoom level {zoom} ===")

            # Get all tiles for this zoom level
            tiles = self.get_tiles_for_bounds(bounds, zoom)
            total = len(tiles)
            print(f"Total tiles: {total}")

            # Download tiles in parallel
            completed = 0
            with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
                # Submit all tasks
                future_to_tile = {
                    executor.submit(self.download_tile, tile, output_dir): tile
                    for tile in tiles
                }

                # Process results
                for future in as_completed(future_to_tile):
                    tile = future_to_tile[future]
                    try:
                        success = future.result()
                        if success:
                            # Check if it was skipped (already existed)
                            tile_path = output_dir / tile.to_path()
                            if tile_path.stat().st_size > 0:
                                stats['success' if tile_path.exists() else 'skipped'] += 1
                        else:
                            stats['failed'] += 1
                    except Exception as e:
                        print(f"Error processing {tile.to_path()}: {e}")
                        stats['failed'] += 1

                    completed += 1
                    if completed % progress_interval == 0:
                        print(f"Progress: {completed}/{total} tiles ({100*completed//total}%)")

            print(f"Zoom {zoom} complete: {completed}/{total} tiles")

        return stats


def load_config(config_dir: Path) -> Tuple[Dict, Dict]:
    """
    Load configuration files.

    Args:
        config_dir: Path to config directory

    Returns:
        Tuple of (bounds, zoom_levels) dicts
    """
    bounds_path = config_dir / "charlotte_bounds.json"
    zoom_path = config_dir / "zoom_levels.json"

    with open(bounds_path) as f:
        bounds = json.load(f)

    with open(zoom_path) as f:
        zoom_config = json.load(f)

    return bounds, zoom_config


def main():
    """Main entry point for tile downloading."""
    # Get script directory
    script_dir = Path(__file__).parent
    config_dir = script_dir.parent / "config"
    output_dir = script_dir.parent / "data" / "raw" / "tiles"

    print("Charlotte Map Tile Downloader")
    print("=" * 50)

    # Load configuration
    bounds, zoom_config = load_config(config_dir)
    print(f"\nBounds: {bounds['name']}")
    print(f"  North: {bounds['north']}, South: {bounds['south']}")
    print(f"  East: {bounds['east']}, West: {bounds['west']}")

    # Get development zoom levels
    zoom_levels = zoom_config['development']
    print(f"\nZoom levels: {zoom_levels}")

    # Estimate tile counts
    for zoom in zoom_levels:
        est = zoom_config['estimated_tiles'].get(str(zoom), 'unknown')
        print(f"  Zoom {zoom}: ~{est} tiles")

    # Create downloader
    downloader = TileDownloader(
        base_url="https://tile.openstreetmap.org",
        rate_limit_delay=0.25,
        max_workers=4,
        timeout=30
    )

    # Download tiles
    print(f"\nStarting download to: {output_dir}")
    start_time = time.time()

    stats = downloader.download_all(
        bounds={
            'north': bounds['north'],
            'south': bounds['south'],
            'east': bounds['east'],
            'west': bounds['west']
        },
        zoom_levels=zoom_levels,
        output_dir=output_dir,
        progress_interval=50
    )

    elapsed = time.time() - start_time

    # Print summary
    print("\n" + "=" * 50)
    print("Download Summary:")
    print(f"  Success: {stats['success']}")
    print(f"  Failed: {stats['failed']}")
    print(f"  Skipped: {stats['skipped']}")
    print(f"  Elapsed: {elapsed:.1f} seconds")
    print(f"  Output: {output_dir}")


if __name__ == "__main__":
    main()
