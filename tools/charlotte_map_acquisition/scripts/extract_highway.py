#!/usr/bin/env python3
"""
Highway boundary extraction script for Charlotte I-485 corridor.

Extracts I-485 highway geometry from OpenStreetMap using OSMnx.

Usage:
    python -m tools.charlotte_map_acquisition.scripts.extract_highway
"""

import json
from pathlib import Path
from typing import Dict, Optional, Tuple

try:
    import osmnx as ox
    import geopandas as gpd
    from shapely.geometry import LineString, MultiLineString, Polygon
    from shapely.ops import unary_union
    OSMNX_AVAILABLE = True
except ImportError:
    OSMNX_AVAILABLE = False
    print("WARNING: OSMnx not installed. Install with: pip install osmnx")


class HighwayExtractor:
    """
    Extracts highway geometry from OpenStreetMap.

    Based on research:
    - I-485 is a 66.68-mile complete loop around Charlotte
    - Tagged in OSM as: highway=motorway, ref=I-485
    - Use OSMnx for easy OSM data extraction
    """

    def __init__(self, rate_limit_delay: float = 1.0):
        """
        Initialize highway extractor.

        Args:
            rate_limit_delay: Seconds between OSM API requests (default: 1.0)
        """
        if not OSMNX_AVAILABLE:
            raise ImportError("OSMnx is required but not installed")

        self.rate_limit_delay = rate_limit_delay

        # Configure OSMnx
        ox.settings.use_cache = True
        ox.settings.log_console = True

    def extract_i485_highway(
        self,
        place: str = "Charlotte, North Carolina, USA"
    ) -> gpd.GeoDataFrame:
        """
        Download I-485 highway geometry from OpenStreetMap.

        Args:
            place: Place name to search (default: Charlotte, NC)

        Returns:
            GeoDataFrame with I-485 highway segments
        """
        print(f"Extracting I-485 from OpenStreetMap...")
        print(f"  Place: {place}")

        # Custom filter for I-485
        # highway=motorway for interstate
        # ref~"485" to match I-485 reference
        custom_filter = '["highway"="motorway"]["ref"~"485"]'

        try:
            # Download highway features
            gdf = ox.features_from_place(
                place,
                tags={
                    "highway": "motorway",
                    "ref": True  # Has a reference number
                }
            )

            print(f"  Downloaded {len(gdf)} highway features")

            # Filter to only I-485 segments
            # Check if 'ref' column contains '485'
            if 'ref' in gdf.columns:
                i485_gdf = gdf[gdf['ref'].str.contains('485', case=False, na=False)]
                print(f"  Filtered to {len(i485_gdf)} I-485 segments")
            else:
                print("  WARNING: 'ref' column not found, using all motorways")
                i485_gdf = gdf

            return i485_gdf

        except Exception as e:
            print(f"Error extracting highway data: {e}")
            raise

    def get_i485_bounding_box(
        self,
        config_path: Optional[Path] = None
    ) -> Dict[str, float]:
        """
        Get I-485 bounding box from config or compute from geometry.

        Args:
            config_path: Path to bounds config file (optional)

        Returns:
            Dict with 'north', 'south', 'east', 'west' keys
        """
        if config_path and config_path.exists():
            with open(config_path) as f:
                config = json.load(f)
                return {
                    'north': config['north'],
                    'south': config['south'],
                    'east': config['east'],
                    'west': config['west']
                }

        # If no config, return approximate I-485 bounds
        return {
            'north': 35.37,
            'south': 35.08,
            'east': -80.65,
            'west': -81.00
        }

    def process_highway_geometry(
        self,
        gdf: gpd.GeoDataFrame,
        buffer_width_meters: Optional[float] = None
    ) -> Tuple[gpd.GeoDataFrame, gpd.GeoDataFrame]:
        """
        Process highway geometry: dissolve segments, optionally buffer.

        Args:
            gdf: GeoDataFrame with highway segments
            buffer_width_meters: Width to buffer road centerline (optional)

        Returns:
            Tuple of (centerline_gdf, boundary_gdf)
        """
        print("\nProcessing highway geometry...")

        # Dissolve all segments into single geometry
        print("  Dissolving segments...")
        dissolved = unary_union(gdf.geometry)

        # Ensure it's a LineString or MultiLineString
        if dissolved.geom_type == 'MultiLineString':
            print(f"  Result: MultiLineString with {len(dissolved.geoms)} parts")
        elif dissolved.geom_type == 'LineString':
            print("  Result: LineString")
        else:
            print(f"  WARNING: Unexpected geometry type: {dissolved.geom_type}")

        # Create centerline GeoDataFrame
        centerline_gdf = gpd.GeoDataFrame(
            geometry=[dissolved],
            crs=gdf.crs
        )
        centerline_gdf['name'] = 'I-485'
        centerline_gdf['type'] = 'centerline'

        # Calculate total length
        total_length_km = dissolved.length / 1000
        total_length_miles = total_length_km * 0.621371
        print(f"  Total length: {total_length_km:.2f} km ({total_length_miles:.2f} miles)")

        # Optionally create buffer polygon
        boundary_gdf = None
        if buffer_width_meters:
            print(f"  Creating buffer polygon (width: {buffer_width_meters}m)...")
            buffer_geom = dissolved.buffer(buffer_width_meters)

            boundary_gdf = gpd.GeoDataFrame(
                geometry=[buffer_geom],
                crs=gdf.crs
            )
            boundary_gdf['name'] = 'I-485'
            boundary_gdf['type'] = 'boundary'
            boundary_gdf['buffer_width_m'] = buffer_width_meters

        return centerline_gdf, boundary_gdf

    def export_to_geojson(
        self,
        gdf: gpd.GeoDataFrame,
        output_path: Path
    ) -> None:
        """
        Export GeoDataFrame to GeoJSON file.

        Args:
            gdf: GeoDataFrame to export
            output_path: Path to output file
        """
        # Create output directory
        output_path.parent.mkdir(parents=True, exist_ok=True)

        # Export to GeoJSON
        gdf.to_file(output_path, driver='GeoJSON')

        print(f"  Exported to: {output_path}")
        print(f"  Features: {len(gdf)}")


def print_extraction_statistics(gdf: gpd.GeoDataFrame) -> None:
    """Print statistics about extracted highway data."""
    print("\n" + "=" * 50)
    print("Extraction Statistics:")
    print("=" * 50)

    # Number of segments
    print(f"Total segments: {len(gdf)}")

    # Geometry types
    if 'geometry' in gdf.columns:
        geom_types = gdf.geometry.geom_type.value_counts()
        print("\nGeometry types:")
        for geom_type, count in geom_types.items():
            print(f"  {geom_type}: {count}")

    # Total length
    if 'geometry' in gdf.columns:
        total_length = gdf.geometry.length.sum()
        print(f"\nTotal length: {total_length/1000:.2f} km ({total_length*0.621371/1000:.2f} miles)")

    # OSM tags
    if 'name' in gdf.columns:
        unique_names = gdf['name'].dropna().unique()
        print(f"\nUnique names: {len(unique_names)}")
        for name in unique_names[:5]:
            print(f"  - {name}")

    if 'ref' in gdf.columns:
        unique_refs = gdf['ref'].dropna().unique()
        print(f"\nUnique refs: {len(unique_refs)}")
        for ref in unique_refs[:5]:
            print(f"  - {ref}")


def main():
    """Main entry point for highway extraction."""
    if not OSMNX_AVAILABLE:
        print("\nERROR: OSMnx is required for highway extraction")
        print("Install with: pip install osmnx")
        return

    # Get script directory
    script_dir = Path(__file__).parent
    config_dir = script_dir.parent / "config"
    raw_output_dir = script_dir.parent / "data" / "raw" / "osm"
    processed_output_dir = script_dir.parent / "data" / "processed" / "vectors"

    print("Charlotte Highway Boundary Extraction")
    print("=" * 50)

    # Create extractor
    extractor = HighwayExtractor(rate_limit_delay=1.0)

    # Extract I-485 highway
    try:
        i485_gdf = extractor.extract_i485_highway(
            place="Charlotte, North Carolina, USA"
        )

        if len(i485_gdf) == 0:
            print("\nWARNING: No I-485 segments found")
            print("This could indicate:")
            print("  - OSM data not available")
            print("  - Network connectivity issues")
            print("  - Tagging differences in OSM")
            return

    except Exception as e:
        print(f"\nERROR: Failed to extract highway data: {e}")
        return

    # Print extraction statistics
    print_extraction_statistics(i485_gdf)

    # Export raw OSM data
    print("\nExporting raw OSM data...")
    raw_output_path = raw_output_dir / "i485_raw.geojson"
    extractor.export_to_geojson(i485_gdf, raw_output_path)

    # Process geometry
    centerline_gdf, boundary_gdf = extractor.process_highway_geometry(
        i485_gdf,
        buffer_width_meters=100  # 100m buffer for highway corridor
    )

    # Export processed centerline
    print("\nExporting processed centerline...")
    centerline_path = processed_output_dir / "i485_centerline.geojson"
    extractor.export_to_geojson(centerline_gdf, centerline_path)

    # Export boundary polygon
    if boundary_gdf is not None:
        print("\nExporting boundary polygon...")
        boundary_path = processed_output_dir / "i485_boundary.geojson"
        extractor.export_to_geojson(boundary_gdf, boundary_path)

    # Print summary
    print("\n" + "=" * 50)
    print("Extraction Complete")
    print("=" * 50)
    print(f"Raw OSM data: {raw_output_path}")
    print(f"Centerline: {centerline_path}")
    if boundary_gdf is not None:
        print(f"Boundary: {boundary_path}")

    # Print statistics
    if 'geometry' in centerline_gdf.columns:
        total_length_km = centerline_gdf.geometry.length.sum() / 1000
        total_length_miles = total_length_km * 0.621371
        print(f"\nI-485 Total Length: {total_length_km:.2f} km ({total_length_miles:.2f} miles)")
        print(f"Expected: ~66.68 miles (107.3 km)")

        # Check if close to expected
        if abs(total_length_miles - 66.68) < 5:
            print("✓ Length matches expected value")
        else:
            print("⚠ Length differs from expected - verify extraction")


if __name__ == "__main__":
    main()
