#!/usr/bin/env python3
"""
Pipeline Orchestration for Charlotte Map Data Acquisition

Coordinates all data acquisition and transformation steps into a unified workflow.

Usage:
    from pipeline import CharlotteMapPipeline
    pipeline = CharlotteMapPipeline()
    pipeline.run_full_pipeline()

    # Or run specific steps:
    pipeline.run_steps(['download_tiles', 'extract_highway'])
"""

import json
import time
import sys
from pathlib import Path
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Any, Callable
from enum import Enum

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from .coordinates import latlon_to_tile, latlon_to_unreal, charlotte_bounds


class StepStatus(Enum):
    """Status of a pipeline step."""
    PENDING = "pending"
    RUNNING = "running"
    SUCCESS = "success"
    FAILED = "failed"
    SKIPPED = "skipped"


@dataclass
class StepResult:
    """Result of a pipeline step execution."""
    name: str
    status: StepStatus
    duration_seconds: float = 0.0
    message: str = ""
    error: Optional[str] = None
    stats: Dict[str, Any] = field(default_factory=dict)


@dataclass
class PipelineStep:
    """Definition of a pipeline step."""
    name: str
    display_name: str
    description: str
    requires: List[str] = field(default_factory=list)
    critical: bool = True  # If True, pipeline stops on failure


class CharlotteMapPipeline:
    """
    Main orchestration class for Charlotte map data acquisition.

    Pipeline Steps (in order):
        1. download_tiles    - Download map tiles from tile server
        2. download_dem      - Query USGS 3DEP for DEM data
        3. extract_highway   - Extract I-485 geometry from OSM
        4. generate_heightmap - Convert DEM to Unreal heightmap
        5. transform_vectors - Transform vectors to Unreal coordinates

    Each step can be run independently or as part of the full pipeline.
    """

    # Define all pipeline steps
    STEPS = [
        PipelineStep(
            name="download_tiles",
            display_name="Download Map Tiles",
            description="Download map tiles from OpenStreetMap for I-485 corridor",
            requires=[],
            critical=False  # Tiles are optional for core pipeline
        ),
        PipelineStep(
            name="download_dem",
            display_name="Download DEM Data",
            description="Query USGS 3DEP API for available elevation data",
            requires=[],
            critical=False  # DEM may require manual download
        ),
        PipelineStep(
            name="extract_highway",
            display_name="Extract Highway Geometry",
            description="Extract I-485 highway geometry from OpenStreetMap",
            requires=[],
            critical=True  # Highway boundary is essential
        ),
        PipelineStep(
            name="generate_heightmap",
            display_name="Generate Heightmap",
            description="Convert DEM GeoTIFF to Unreal Engine heightmap",
            requires=["download_dem"],
            critical=False  # Requires DEM file first
        ),
        PipelineStep(
            name="transform_vectors",
            display_name="Transform Vectors",
            description="Transform GeoJSON vectors to Unreal coordinate space",
            requires=["extract_highway"],
            critical=True  # Essential for Unreal integration
        ),
    ]

    def __init__(
        self,
        config_path: Optional[Path] = None,
        base_dir: Optional[Path] = None,
        verbose: bool = True
    ):
        """
        Initialize the pipeline.

        Args:
            config_path: Path to config directory (default: package config/)
            base_dir: Base directory for data (default: package data/)
            verbose: Print progress messages (default: True)
        """
        self.base_dir = Path(base_dir) if base_dir else Path(__file__).parent
        self.config_path = Path(config_path) if config_path else self.base_dir / "config"
        self.verbose = verbose

        # Data directories
        self.data_dir = self.base_dir / "data"
        self.raw_dir = self.data_dir / "raw"
        self.processed_dir = self.data_dir / "processed"

        # Results tracking
        self.results: Dict[str, StepResult] = {}
        self.start_time: Optional[float] = None

        # Load configuration
        self._load_config()

        # Setup directories
        self._setup_directories()

    def _load_config(self) -> None:
        """Load configuration files."""
        # Load bounds
        bounds_path = self.config_path / "charlotte_bounds.json"
        if bounds_path.exists():
            with open(bounds_path) as f:
                self.bounds = json.load(f)
        else:
            # Use defaults from coordinates module
            self.bounds = charlotte_bounds

        # Load zoom levels
        zoom_path = self.config_path / "zoom_levels.json"
        if zoom_path.exists():
            with open(zoom_path) as f:
                self.zoom_config = json.load(f)
        else:
            # Default zoom levels
            self.zoom_config = {
                "development": [15, 16],
                "production": [14, 15, 16, 17]
            }

    def _setup_directories(self) -> None:
        """Create output directory structure."""
        directories = [
            # Raw data directories
            self.raw_dir / "tiles",
            self.raw_dir / "dem",
            self.raw_dir / "osm",

            # Processed data directories
            self.processed_dir / "heightmaps",
            self.processed_dir / "vectors",
            self.processed_dir / "vectors" / "unreal",
        ]

        for directory in directories:
            directory.mkdir(parents=True, exist_ok=True)

        if self.verbose:
            print(f"Pipeline directories ready: {self.data_dir}")

    @property
    def steps(self) -> Dict[str, PipelineStep]:
        """Get steps as a dictionary."""
        return {step.name: step for step in self.STEPS}

    def _log(self, message: str) -> None:
        """Print message if verbose mode is enabled."""
        if self.verbose:
            print(message)

    def _run_step(
        self,
        step_name: str,
        step_func: Callable[[], Dict[str, Any]]
    ) -> StepResult:
        """Execute a single step and track results."""
        step = self.steps.get(step_name)
        if not step:
            return StepResult(
                name=step_name,
                status=StepStatus.FAILED,
                message=f"Unknown step: {step_name}"
            )

        self._log(f"\n{'='*60}")
        self._log(f"STEP: {step.display_name}")
        self._log(f"{'='*60}")
        self._log(f"  {step.description}")

        start = time.time()

        try:
            stats = step_func()
            duration = time.time() - start

            result = StepResult(
                name=step_name,
                status=StepStatus.SUCCESS,
                duration_seconds=duration,
                message="Completed successfully",
                stats=stats
            )

            self._log(f"  Status: SUCCESS ({duration:.1f}s)")

        except ImportError as e:
            duration = time.time() - start
            result = StepResult(
                name=step_name,
                status=StepStatus.SKIPPED,
                duration_seconds=duration,
                message=f"Missing dependency: {e}",
                error=str(e)
            )
            self._log(f"  Status: SKIPPED (missing dependency: {e})")

        except Exception as e:
            duration = time.time() - start
            result = StepResult(
                name=step_name,
                status=StepStatus.FAILED,
                duration_seconds=duration,
                message=f"Failed: {e}",
                error=str(e)
            )
            self._log(f"  Status: FAILED ({e})")

        self.results[step_name] = result
        return result

    def step_download_tiles(
        self,
        zoom_levels: Optional[List[int]] = None
    ) -> Dict[str, Any]:
        """
        Download map tiles from OpenStreetMap.

        Args:
            zoom_levels: Zoom levels to download (default: from config)

        Returns:
            Stats dict with 'success', 'failed', 'skipped' counts
        """
        from scripts.download_tiles import TileDownloader, load_config

        if zoom_levels is None:
            zoom_levels = self.zoom_config.get("development", [15, 16])

        self._log(f"  Zoom levels: {zoom_levels}")
        self._log(f"  Output: {self.raw_dir / 'tiles'}")

        # Create downloader
        downloader = TileDownloader(
            base_url="https://tile.openstreetmap.org",
            rate_limit_delay=0.25,
            max_workers=4,
            timeout=30
        )

        # Download tiles
        stats = downloader.download_all(
            bounds={
                'north': self.bounds['north'],
                'south': self.bounds['south'],
                'east': self.bounds['east'],
                'west': self.bounds['west']
            },
            zoom_levels=zoom_levels,
            output_dir=self.raw_dir / "tiles",
            progress_interval=50
        )

        return stats

    def step_download_dem(self) -> Dict[str, Any]:
        """
        Query USGS 3DEP for DEM data.

        Returns:
            Stats dict with product info
        """
        from scripts.download_dem import DEamacquisition

        self._log(f"  Querying USGS 3DEP API...")
        self._log(f"  Output: {self.raw_dir / 'dem'}")

        # Create acquirer
        acquirer = DEamacquisition(timeout=30)

        # Query for available products
        bbox = {
            'west': self.bounds['west'],
            'south': self.bounds['south'],
            'east': self.bounds['east'],
            'north': self.bounds['north']
        }

        datasets = [
            "3D Elevation Program (3DEP) 1 meter",
            "National Elevation Dataset (NED) 1/3 arc-second"
        ]

        all_products = {"items": [], "total": 0}

        for dataset in datasets:
            products = acquirer.query_usgs_3dep(bbox, datasets=[dataset])
            if "items" in products:
                all_products["items"].extend(products["items"])
                all_products["total"] += len(products["items"])
            time.sleep(1)  # Rate limiting

        # Save product list
        products_path = self.raw_dir / "dem" / "usgs_products.json"
        acquirer.save_product_list(all_products, products_path)

        # Extract download URLs
        download_urls = acquirer.get_dem_download_urls(all_products)

        return {
            "products_found": all_products["total"],
            "download_urls": len(download_urls),
            "products_file": str(products_path)
        }

    def step_extract_highway(self) -> Dict[str, Any]:
        """
        Extract I-485 highway geometry from OpenStreetMap.

        Returns:
            Stats dict with geometry info
        """
        from scripts.extract_highway import HighwayExtractor

        self._log(f"  Querying OpenStreetMap for I-485...")
        self._log(f"  Output: {self.raw_dir / 'osm'}")

        # Create extractor
        extractor = HighwayExtractor(rate_limit_delay=1.0)

        # Extract highway
        i485_gdf = extractor.extract_i485_highway(
            place="Charlotte, North Carolina, USA"
        )

        # Export raw data
        raw_output_path = self.raw_dir / "osm" / "i485_raw.geojson"
        extractor.export_to_geojson(i485_gdf, raw_output_path)

        # Process geometry with buffer
        centerline_gdf, boundary_gdf = extractor.process_highway_geometry(
            i485_gdf,
            buffer_width_meters=100
        )

        # Export processed centerline
        centerline_path = self.processed_dir / "vectors" / "i485_centerline.geojson"
        extractor.export_to_geojson(centerline_gdf, centerline_path)

        # Export boundary
        if boundary_gdf is not None:
            boundary_path = self.processed_dir / "vectors" / "i485_boundary.geojson"
            extractor.export_to_geojson(boundary_gdf, boundary_path)

        # Calculate stats
        total_length_km = centerline_gdf.geometry.length.sum() / 1000
        total_length_miles = total_length_km * 0.621371

        return {
            "segments": len(i485_gdf),
            "length_km": round(total_length_km, 2),
            "length_miles": round(total_length_miles, 2),
            "centerline_file": str(centerline_path),
            "boundary_file": str(boundary_path) if boundary_gdf is not None else None
        }

    def step_generate_heightmap(
        self,
        target_size: Optional[int] = None
    ) -> Dict[str, Any]:
        """
        Convert DEM to Unreal Engine heightmap.

        Args:
            target_size: Target resolution (power-of-2 + 1)

        Returns:
            Stats dict with heightmap info
        """
        from scripts.generate_heightmap import (
            dem_to_unreal_heightmap,
            find_dem_files,
            get_dem_info,
            RASTERIO_AVAILABLE,
            PIL_AVAILABLE
        )

        if not RASTERIO_AVAILABLE or not PIL_AVAILABLE:
            raise ImportError(
                "rasterio and Pillow are required for heightmap generation. "
                "Install with: pip install rasterio Pillow"
            )

        # Find DEM files
        dem_files = find_dem_files(self.raw_dir / "dem")

        if not dem_files:
            self._log("  No DEM files found. Run download_dem step first.")
            return {"heightmaps_generated": 0, "message": "No DEM files found"}

        self._log(f"  Found {len(dem_files)} DEM file(s)")

        heightmaps = []
        for dem_path in dem_files:
            self._log(f"  Processing: {dem_path.name}")

            try:
                output_path, metadata = dem_to_unreal_heightmap(
                    dem_path,
                    output_path=self.processed_dir / "heightmaps" / f"{dem_path.stem}.png",
                    target_size=target_size
                )
                heightmaps.append({
                    "source": str(dem_path.name),
                    "output": str(output_path.name),
                    "resolution": metadata["resolution"]
                })
            except Exception as e:
                self._log(f"    Error: {e}")

        return {
            "heightmaps_generated": len(heightmaps),
            "heightmaps": heightmaps
        }

    def step_transform_vectors(
        self,
        default_elevation: float = 0.0
    ) -> Dict[str, Any]:
        """
        Transform GeoJSON vectors to Unreal coordinate space.

        Args:
            default_elevation: Default elevation for 2D coordinates

        Returns:
            Stats dict with transformation info
        """
        from scripts.transform_data import (
            transform_geojson_to_unreal,
            validate_origin_transform
        )

        # Validate origin
        if not validate_origin_transform():
            raise ValueError("Origin transform validation failed!")

        # Find GeoJSON files to transform
        vectors_dir = self.processed_dir / "vectors"
        geojson_files = list(vectors_dir.glob("*.geojson"))

        # Exclude already-transformed files
        geojson_files = [
            f for f in geojson_files
            if "_unreal" not in f.stem
        ]

        if not geojson_files:
            self._log("  No GeoJSON files found to transform")
            return {"files_transformed": 0}

        self._log(f"  Found {len(geojson_files)} file(s) to transform")

        transformed = []
        for geojson_path in geojson_files:
            self._log(f"  Transforming: {geojson_path.name}")

            try:
                output_path, stats = transform_geojson_to_unreal(
                    geojson_path,
                    default_elevation=default_elevation
                )
                transformed.append({
                    "source": geojson_path.name,
                    "output": output_path.name,
                    "features": stats["features_transformed"]
                })
            except Exception as e:
                self._log(f"    Error: {e}")

        return {
            "files_transformed": len(transformed),
            "transformed": transformed
        }

    def run_full_pipeline(
        self,
        zoom_levels: Optional[List[int]] = None,
        heightmap_size: Optional[int] = None
    ) -> Dict[str, StepResult]:
        """
        Run the complete pipeline.

        Args:
            zoom_levels: Zoom levels for tile download
            heightmap_size: Target size for heightmaps

        Returns:
            Dict of step name to StepResult
        """
        self._log("\n" + "=" * 60)
        self._log("CHARLOTTE MAP DATA ACQUISITION PIPELINE")
        self._log("=" * 60)
        self._log(f"Bounds: {self.bounds['name']}")
        self._log(f"Output: {self.data_dir}")

        self.start_time = time.time()
        self.results = {}

        # Define step execution order
        step_sequence = [
            ("download_tiles", lambda: self.step_download_tiles(zoom_levels)),
            ("download_dem", lambda: self.step_download_dem()),
            ("extract_highway", lambda: self.step_extract_highway()),
            ("generate_heightmap", lambda: self.step_generate_heightmap(heightmap_size)),
            ("transform_vectors", lambda: self.step_transform_vectors()),
        ]

        # Execute steps in order
        for step_name, step_func in step_sequence:
            result = self._run_step(step_name, step_func)

            # Check if critical step failed
            step = self.steps[step_name]
            if step.critical and result.status == StepStatus.FAILED:
                self._log(f"\n  CRITICAL STEP FAILED: {step_name}")
                self._log("  Pipeline stopping.")
                break

        # Print summary
        self._print_summary()

        return self.results

    def run_steps(
        self,
        step_names: List[str],
        zoom_levels: Optional[List[int]] = None,
        heightmap_size: Optional[int] = None
    ) -> Dict[str, StepResult]:
        """
        Run specific steps only.

        Args:
            step_names: List of step names to run
            zoom_levels: Zoom levels for tile download
            heightmap_size: Target size for heightmaps

        Returns:
            Dict of step name to StepResult
        """
        self._log("\n" + "=" * 60)
        self._log("CHARLOTTE MAP DATA ACQUISITION PIPELINE")
        self._log(f"Running steps: {', '.join(step_names)}")
        self._log("=" * 60)

        self.start_time = time.time()
        self.results = {}

        # Step function mapping
        step_funcs = {
            "download_tiles": lambda: self.step_download_tiles(zoom_levels),
            "download_dem": lambda: self.step_download_dem(),
            "extract_highway": lambda: self.step_extract_highway(),
            "generate_heightmap": lambda: self.step_generate_heightmap(heightmap_size),
            "transform_vectors": lambda: self.step_transform_vectors(),
        }

        for step_name in step_names:
            if step_name not in step_funcs:
                self._log(f"WARNING: Unknown step '{step_name}', skipping")
                continue

            step_func = step_funcs[step_name]
            result = self._run_step(step_name, step_func)

            # Check if critical step failed
            step = self.steps[step_name]
            if step.critical and result.status == StepStatus.FAILED:
                self._log(f"\n  CRITICAL STEP FAILED: {step_name}")
                break

        self._print_summary()
        return self.results

    def _print_summary(self) -> None:
        """Print pipeline execution summary."""
        total_duration = time.time() - self.start_time if self.start_time else 0

        self._log("\n" + "=" * 60)
        self._log("PIPELINE SUMMARY")
        self._log("=" * 60)

        success_count = 0
        failed_count = 0
        skipped_count = 0

        for step in self.STEPS:
            result = self.results.get(step.name)
            if result:
                status_icon = {
                    StepStatus.SUCCESS: "✓",
                    StepStatus.FAILED: "✗",
                    StepStatus.SKIPPED: "○",
                    StepStatus.RUNNING: "⋯",
                    StepStatus.PENDING: "·"
                }.get(result.status, "?")

                self._log(
                    f"  {status_icon} {step.display_name}: "
                    f"{result.status.value} ({result.duration_seconds:.1f}s)"
                )

                if result.status == StepStatus.SUCCESS:
                    success_count += 1
                elif result.status == StepStatus.FAILED:
                    failed_count += 1
                else:
                    skipped_count += 1
            else:
                self._log(f"  · {step.display_name}: not run")

        self._log(f"\nTotal: {success_count} success, {failed_count} failed, {skipped_count} skipped")
        self._log(f"Duration: {total_duration:.1f} seconds")
        self._log(f"Output: {self.data_dir}")


def main():
    """CLI entry point for pipeline."""
    import argparse

    parser = argparse.ArgumentParser(
        description='Charlotte Map Data Acquisition Pipeline'
    )
    parser.add_argument(
        '--steps',
        type=str,
        help='Comma-separated list of steps to run (default: all)'
    )
    parser.add_argument(
        '--list',
        action='store_true',
        help='List available steps and exit'
    )
    parser.add_argument(
        '--config',
        type=str,
        help='Path to config directory (default: config/)'
    )
    parser.add_argument(
        '--zoom',
        type=str,
        default='15,16',
        help='Comma-separated zoom levels (default: 15,16)'
    )
    parser.add_argument(
        '--heightmap-size',
        type=int,
        choices=[513, 1009, 2017, 4033, 8065],
        help='Target heightmap resolution (power-of-2 + 1)'
    )

    args = parser.parse_args()

    # List steps
    if args.list:
        print("Available steps:")
        for step in CharlotteMapPipeline.STEPS:
            print(f"  {step.name:20} - {step.description}")
            if step.requires:
                print(f"                       Requires: {', '.join(step.requires)}")
        return

    # Create pipeline
    config_path = Path(args.config) if args.config else None
    pipeline = CharlotteMapPipeline(config_path=config_path)

    # Parse zoom levels
    zoom_levels = [int(z) for z in args.zoom.split(',')]

    # Run steps
    if args.steps:
        steps = [s.strip() for s in args.steps.split(',')]
        pipeline.run_steps(
            steps,
            zoom_levels=zoom_levels,
            heightmap_size=args.heightmap_size
        )
    else:
        pipeline.run_full_pipeline(
            zoom_levels=zoom_levels,
            heightmap_size=args.heightmap_size
        )


if __name__ == '__main__':
    main()
