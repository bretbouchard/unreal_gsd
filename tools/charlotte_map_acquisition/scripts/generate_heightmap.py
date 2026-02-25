#!/usr/bin/env python3
"""
Heightmap Generator for Unreal Engine

Converts DEM (Digital Elevation Model) GeoTIFF files to Unreal Engine
compatible 16-bit grayscale PNG heightmaps.

Unreal Heightmap Requirements:
    - 16-bit grayscale PNG (mode='I;16' in Pillow)
    - White (65535) = highest point
    - Black (0) = lowest point
    - Resolution: power-of-2 + 1 (1009, 2017, 4033, etc.) for terrain

Usage:
    python generate_heightmap.py [--resample SIZE]

    Or as module:
    from generate_heightmap import dem_to_unreal_heightmap, get_dem_info

Output:
    Heightmaps saved to ../data/processed/heightmaps/
    Metadata JSON saved alongside each heightmap
"""

import os
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, Tuple, Optional, Any

import numpy as np

try:
    import rasterio
    from rasterio.warp import calculate_default_transform, reproject, Resampling
    RASTERIO_AVAILABLE = True
except ImportError:
    RASTERIO_AVAILABLE = False
    rasterio = None

try:
    from PIL import Image
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    Image = None

# Default paths relative to this script
SCRIPT_DIR = Path(__file__).parent.resolve()
DATA_DIR = SCRIPT_DIR.parent / "data"
RAW_DEM_DIR = DATA_DIR / "raw" / "dem"
PROCESSED_HEIGHTMAPS_DIR = DATA_DIR / "processed" / "heightmaps"


def get_dem_info(dem_path: Path) -> Dict[str, Any]:
    """Extract metadata from a DEM GeoTIFF file.

    Args:
        dem_path: Path to the DEM GeoTIFF file

    Returns:
        Dictionary with DEM metadata including:
        - width, height: Raster dimensions
        - bounds: (west, south, east, north) in CRS units
        - crs: Coordinate reference system
        - resolution: (pixel_width, pixel_height)
        - nodata: No-data value
        - dtype: Data type
        - min_elevation: Minimum elevation value (excluding nodata)
        - max_elevation: Maximum elevation value (excluding nodata)
    """
    if not RASTERIO_AVAILABLE:
        raise ImportError(
            "rasterio is required for DEM processing. "
            "Install with: pip install rasterio>=1.3.0"
        )

    with rasterio.open(dem_path) as src:
        # Read the first band
        data = src.read(1)

        # Handle nodata values
        if src.nodata is not None:
            valid_data = data[data != src.nodata]
        else:
            valid_data = data[~np.isnan(data)]

        bounds = src.bounds

        return {
            "width": src.width,
            "height": src.height,
            "bounds": {
                "west": bounds.left,
                "south": bounds.bottom,
                "east": bounds.right,
                "north": bounds.top
            },
            "crs": str(src.crs),
            "resolution": (src.res[0], src.res[1]),
            "nodata": src.nodata,
            "dtype": str(src.dtypes[0]),
            "min_elevation": float(valid_data.min()) if len(valid_data) > 0 else None,
            "max_elevation": float(valid_data.max()) if len(valid_data) > 0 else None,
            "mean_elevation": float(valid_data.mean()) if len(valid_data) > 0 else None,
            "valid_pixels": int(len(valid_data)),
            "total_pixels": int(data.size)
        }


def normalize_elevation(
    elevation_array: np.ndarray,
    nodata: Optional[float] = None,
    output_min: int = 0,
    output_max: int = 65535
) -> np.ndarray:
    """Normalize elevation values to 16-bit range (0-65535).

    Unreal Engine convention:
        - White (65535) = highest point
        - Black (0) = lowest point

    Args:
        elevation_array: 2D numpy array of elevation values
        nodata: No-data value to exclude from normalization
        output_min: Minimum output value (default: 0)
        output_max: Maximum output value (default: 65535 for 16-bit)

    Returns:
        Normalized uint16 numpy array with nodata values set to 0
    """
    # Create a mask for valid data
    if nodata is not None:
        valid_mask = elevation_array != nodata
    else:
        valid_mask = ~np.isnan(elevation_array)

    # Get valid data range
    valid_data = elevation_array[valid_mask]

    if len(valid_data) == 0:
        # No valid data, return zeros
        return np.zeros_like(elevation_array, dtype=np.uint16)

    elev_min = valid_data.min()
    elev_max = valid_data.max()

    # Normalize to 0-65535 range
    normalized = np.zeros_like(elevation_array, dtype=np.float64)

    if elev_max > elev_min:
        # Avoid division by zero
        normalized[valid_mask] = (
            (elevation_array[valid_mask] - elev_min) /
            (elev_max - elev_min) *
            (output_max - output_min) + output_min
        )
    else:
        # Flat terrain, set to middle value
        normalized[valid_mask] = (output_max + output_min) / 2

    # Convert to uint16
    return normalized.astype(np.uint16)


def calculate_target_resolution(
    width: int,
    height: int,
    target_size: Optional[int] = None
) -> Tuple[int, int]:
    """Calculate optimal power-of-2 + 1 resolution for heightmap.

    Unreal Engine terrain uses power-of-2 + 1 resolution:
        - 513x513 (512 quads)
        - 1009x1009 (1008 quads)
        - 2017x2017 (2016 quads)
        - 4033x4033 (4032 quads)

    Args:
        width: Original width
        height: Original height
        target_size: Desired target size (power-of-2 + 1). If None, finds best fit.

    Returns:
        Tuple of (new_width, new_height)
    """
    # Standard Unreal terrain sizes (power-of-2 + 1)
    standard_sizes = [513, 1009, 2017, 4033, 8065]

    if target_size is not None:
        # Use specified target size
        if target_size not in standard_sizes:
            print(f"Warning: {target_size} is not a standard Unreal terrain size")
        return (target_size, target_size)

    # Find the best fit based on the larger dimension
    max_dim = max(width, height)

    # Find the smallest standard size that's at least as big
    for size in standard_sizes:
        if size >= max_dim:
            return (size, size)

    # If larger than all standards, use the largest
    return (standard_sizes[-1], standard_sizes[-1])


def resample_dem(
    dem_path: Path,
    target_width: int,
    target_height: int,
    output_path: Optional[Path] = None
) -> np.ndarray:
    """Resample DEM to target resolution.

    Args:
        dem_path: Path to source DEM
        target_width: Target width in pixels
        target_height: Target height in pixels
        output_path: Optional path to save resampled GeoTIFF

    Returns:
        Resampled elevation array
    """
    if not RASTERIO_AVAILABLE:
        raise ImportError("rasterio is required for resampling")

    with rasterio.open(dem_path) as src:
        # Calculate new transform
        transform, width, height = calculate_default_transform(
            src.crs, src.crs,
            target_width, target_height,
            *src.bounds
        )

        # Prepare output array
        data = np.zeros((target_height, target_width), dtype=np.float32)

        # Reproject/resample
        reproject(
            source=rasterio.band(src, 1),
            destination=data,
            src_transform=src.transform,
            src_crs=src.crs,
            dst_transform=transform,
            dst_crs=src.crs,
            resampling=Resampling.bilinear
        )

        # Save resampled if output path specified
        if output_path:
            profile = src.profile.copy()
            profile.update(
                transform=transform,
                width=target_width,
                height=target_height,
                dtype='float32'
            )
            with rasterio.open(output_path, 'w', **profile) as dst:
                dst.write(data, 1)

        return data


def dem_to_unreal_heightmap(
    dem_path: Path,
    output_path: Optional[Path] = None,
    target_size: Optional[int] = None,
    unreal_scale: float = 100.0
) -> Tuple[Path, Dict[str, Any]]:
    """Convert DEM GeoTIFF to Unreal Engine compatible heightmap.

    Args:
        dem_path: Path to the DEM GeoTIFF file
        output_path: Output path for heightmap PNG. If None, uses default location.
        target_size: Target resolution (power-of-2 + 1). If None, auto-detects.
        unreal_scale: Scale factor for Unreal units (default: 100 for cm)

    Returns:
        Tuple of (output_path, metadata_dict)

    Raises:
        ImportError: If required dependencies are not installed
        ValueError: If DEM file cannot be read
    """
    if not RASTERIO_AVAILABLE:
        raise ImportError(
            "rasterio is required for DEM processing. "
            "Install with: pip install rasterio>=1.3.0"
        )
    if not PIL_AVAILABLE:
        raise ImportError(
            "Pillow is required for PNG export. "
            "Install with: pip install Pillow>=9.0.0"
        )

    dem_path = Path(dem_path)

    # Get DEM info
    dem_info = get_dem_info(dem_path)

    # Calculate target resolution
    if target_size is None:
        target_width, target_height = calculate_target_resolution(
            dem_info["width"],
            dem_info["height"]
        )
    else:
        target_width = target_height = target_size

    needs_resampling = (
        dem_info["width"] != target_width or
        dem_info["height"] != target_height
    )

    # Read or resample elevation data
    if needs_resampling:
        print(f"Resampling from {dem_info['width']}x{dem_info['height']} "
              f"to {target_width}x{target_height}")
        elevation = resample_dem(dem_path, target_width, target_height)
    else:
        with rasterio.open(dem_path) as src:
            elevation = src.read(1).astype(np.float32)

    # Normalize to 16-bit
    heightmap_data = normalize_elevation(
        elevation,
        nodata=dem_info["nodata"]
    )

    # Determine output path
    if output_path is None:
        # Create output directory if needed
        PROCESSED_HEIGHTMAPS_DIR.mkdir(parents=True, exist_ok=True)

        # Generate filename with resolution info
        stem = dem_path.stem
        output_path = PROCESSED_HEIGHTMAPS_DIR / f"{stem}_{target_width}x{target_height}.png"

    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Create Pillow Image with 16-bit mode
    # PIL's 'I;16' mode is 16-bit unsigned integer grayscale
    img = Image.fromarray(heightmap_data, mode='I;16')

    # Save as PNG
    img.save(output_path)

    # Prepare metadata
    metadata = {
        "source_file": str(dem_path.name),
        "heightmap_file": str(output_path.name),
        "resolution": {
            "width": target_width,
            "height": target_height
        },
        "source_resolution": {
            "width": dem_info["width"],
            "height": dem_info["height"]
        },
        "resampled": needs_resampling,
        "elevation": {
            "min_meters": dem_info["min_elevation"],
            "max_meters": dem_info["max_elevation"],
            "mean_meters": dem_info["mean_elevation"],
            "range_meters": (
                dem_info["max_elevation"] - dem_info["min_elevation"]
                if dem_info["min_elevation"] and dem_info["max_elevation"] else None
            )
        },
        "unreal": {
            "scale": unreal_scale,
            "min_value": 0,
            "max_value": 65535,
            "convention": "white=highest, black=lowest"
        },
        "crs": dem_info["crs"],
        "bounds": dem_info["bounds"]
    }

    # Save metadata JSON
    metadata_path = output_path.with_suffix('.json')
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)

    return output_path, metadata


def find_dem_files(search_dir: Optional[Path] = None) -> list[Path]:
    """Find DEM files in the specified directory.

    Args:
        search_dir: Directory to search. Defaults to raw DEM directory.

    Returns:
        List of paths to DEM files
    """
    if search_dir is None:
        search_dir = RAW_DEM_DIR

    search_dir = Path(search_dir)
    if not search_dir.exists():
        return []

    # Common DEM file extensions
    extensions = ['.tif', '.tiff', '.geotiff', '.dem']

    dem_files = []
    for ext in extensions:
        dem_files.extend(search_dir.glob(f"*{ext}"))
        dem_files.extend(search_dir.glob(f"*{ext.upper()}"))

    return sorted(dem_files)


def main():
    """Main entry point for command-line usage."""
    parser = argparse.ArgumentParser(
        description="Convert DEM GeoTIFF files to Unreal Engine heightmaps"
    )
    parser.add_argument(
        "--input", "-i",
        type=Path,
        help="Input DEM file (default: process all in raw/dem/)"
    )
    parser.add_argument(
        "--output", "-o",
        type=Path,
        help="Output heightmap PNG file"
    )
    parser.add_argument(
        "--resample",
        type=int,
        choices=[513, 1009, 2017, 4033, 8065],
        help="Target resolution (power-of-2 + 1)"
    )
    parser.add_argument(
        "--scale",
        type=float,
        default=100.0,
        help="Unreal scale factor (default: 100 for cm)"
    )
    parser.add_argument(
        "--info",
        action="store_true",
        help="Only print DEM info, don't generate heightmap"
    )

    args = parser.parse_args()

    # Check dependencies
    if not RASTERIO_AVAILABLE:
        print("ERROR: rasterio is required. Install with: pip install rasterio>=1.3.0")
        sys.exit(1)
    if not PIL_AVAILABLE:
        print("ERROR: Pillow is required. Install with: pip install Pillow>=9.0.0")
        sys.exit(1)

    # Get DEM files to process
    if args.input:
        dem_files = [args.input]
    else:
        dem_files = find_dem_files()
        if not dem_files:
            print(f"No DEM files found in {RAW_DEM_DIR}")
            print("\nTo download DEM data, run:")
            print("  python download_dem.py")
            sys.exit(0)

    # Process each DEM file
    for dem_path in dem_files:
        print(f"\nProcessing: {dem_path.name}")

        try:
            # Get and print DEM info
            info = get_dem_info(dem_path)
            print(f"  Resolution: {info['width']}x{info['height']}")
            print(f"  CRS: {info['crs']}")
            print(f"  Elevation range: {info['min_elevation']:.1f}m - {info['max_elevation']:.1f}m")

            if args.info:
                print(f"  Bounds: ({info['bounds']['west']:.2f}, {info['bounds']['south']:.2f}) "
                      f"to ({info['bounds']['east']:.2f}, {info['bounds']['north']:.2f})")
                continue

            # Generate heightmap
            output_path, metadata = dem_to_unreal_heightmap(
                dem_path,
                output_path=args.output,
                target_size=args.resample,
                unreal_scale=args.scale
            )

            print(f"  Output: {output_path}")
            print(f"  Size: {metadata['resolution']['width']}x{metadata['resolution']['height']}")
            print(f"  Resampled: {metadata['resampled']}")
            print(f"  Metadata: {output_path.with_suffix('.json')}")

        except Exception as e:
            print(f"  ERROR: {e}")
            sys.exit(1)

    print("\nDone!")


if __name__ == "__main__":
    main()
