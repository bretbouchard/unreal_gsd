#!/usr/bin/env python3
"""
Charlotte Map Data Acquisition Pipeline

Usage:
    python -m tools.charlotte_map_acquisition              # Run full pipeline
    python -m tools.charlotte_map_acquisition --steps tiles,dem  # Run specific steps
    python -m tools.charlotte_map_acquisition --list       # List available steps
    python -m tools.charlotte_map_acquisition --help       # Show help
"""

import argparse
import sys
from pathlib import Path

# Ensure the package can be imported
sys.path.insert(0, str(Path(__file__).parent.parent))

from tools.charlotte_map_acquisition.pipeline import CharlotteMapPipeline, StepStatus


def main():
    """CLI entry point for the package."""
    parser = argparse.ArgumentParser(
        description='Charlotte Map Data Acquisition Pipeline - '
                    'Automated retrieval of Charlotte I-485 corridor map data for Unreal Engine'
    )
    parser.add_argument(
        '--steps',
        type=str,
        help='Comma-separated list of steps to run (default: all steps)'
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
        help='Comma-separated zoom levels for tile download (default: 15,16)'
    )
    parser.add_argument(
        '--heightmap-size',
        type=int,
        choices=[513, 1009, 2017, 4033, 8065],
        help='Target heightmap resolution (power-of-2 + 1)'
    )

    args = parser.parse_args()

    # List available steps
    if args.list:
        print("Charlotte Map Data Acquisition Pipeline")
        print("=" * 60)
        print("\nAvailable steps:")
        print("-" * 60)
        for step in CharlotteMapPipeline.STEPS:
            critical = " [CRITICAL]" if step.critical else ""
            print(f"  {step.name}")
            print(f"    {step.description}{critical}")
            if step.requires:
                print(f"    Requires: {', '.join(step.requires)}")
            print()
        print("Usage:")
        print("  python -m tools.charlotte_map_acquisition --steps tiles,dem")
        print("  python -m tools.charlotte_map_acquisition --steps extract_highway,transform_vectors")
        sys.exit(0)

    # Create pipeline
    config_path = Path(args.config) if args.config else None
    pipeline = CharlotteMapPipeline(config_path=config_path)

    # Parse zoom levels
    zoom_levels = [int(z.strip()) for z in args.zoom.split(',')]

    # Run pipeline
    if args.steps:
        steps = [s.strip() for s in args.steps.split(',')]
        results = pipeline.run_steps(
            steps,
            zoom_levels=zoom_levels,
            heightmap_size=args.heightmap_size
        )
    else:
        results = pipeline.run_full_pipeline(
            zoom_levels=zoom_levels,
            heightmap_size=args.heightmap_size
        )

    # Exit with error code if any critical step failed
    for step in CharlotteMapPipeline.STEPS:
        result = results.get(step.name)
        if result and step.critical and result.status == StepStatus.FAILED:
            sys.exit(1)

    sys.exit(0)


if __name__ == '__main__':
    main()
