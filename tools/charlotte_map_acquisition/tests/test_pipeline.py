#!/usr/bin/env python3
"""
Integration tests for Charlotte Map Data Acquisition pipeline.

Tests verify:
- Pipeline initialization and configuration
- Directory structure creation
- Step availability and ordering
- Step dependencies
"""

import pytest
import os
import sys
import tempfile
import shutil
from pathlib import Path

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from tools.charlotte_map_acquisition.pipeline import (
    CharlotteMapPipeline,
    StepStatus,
    PipelineStep
)


class TestPipelineInitialization:
    """Tests for pipeline initialization."""

    def test_pipeline_creates_with_defaults(self):
        """Pipeline should initialize with default configuration."""
        pipeline = CharlotteMapPipeline(verbose=False)

        assert pipeline.bounds is not None
        assert 'north' in pipeline.bounds
        assert 'south' in pipeline.bounds
        assert 'east' in pipeline.bounds
        assert 'west' in pipeline.bounds

    def test_pipeline_loads_bounds(self):
        """Pipeline should load Charlotte bounds from config."""
        pipeline = CharlotteMapPipeline(verbose=False)

        assert pipeline.bounds['north'] == 35.37
        assert pipeline.bounds['south'] == 35.08
        assert pipeline.bounds['east'] == -80.65
        assert pipeline.bounds['west'] == -81.00

    def test_pipeline_loads_zoom_config(self):
        """Pipeline should load zoom level configuration."""
        pipeline = CharlotteMapPipeline(verbose=False)

        assert pipeline.zoom_config is not None
        assert 'development' in pipeline.zoom_config

    def test_pipeline_has_data_directory(self):
        """Pipeline should have data directory configured."""
        pipeline = CharlotteMapPipeline(verbose=False)

        assert pipeline.data_dir is not None
        assert pipeline.raw_dir is not None
        assert pipeline.processed_dir is not None


class TestPipelineSteps:
    """Tests for pipeline step configuration."""

    def test_pipeline_has_all_steps(self):
        """Pipeline should have all 5 required steps."""
        pipeline = CharlotteMapPipeline(verbose=False)

        expected_steps = [
            'download_tiles',
            'download_dem',
            'extract_highway',
            'generate_heightmap',
            'transform_vectors'
        ]

        for step_name in expected_steps:
            assert step_name in pipeline.steps, f"Missing step: {step_name}"

    def test_pipeline_has_step_methods(self):
        """Pipeline should have step_X methods for each step."""
        pipeline = CharlotteMapPipeline(verbose=False)

        expected_methods = [
            'step_download_tiles',
            'step_download_dem',
            'step_extract_highway',
            'step_generate_heightmap',
            'step_transform_vectors'
        ]

        for method_name in expected_methods:
            assert hasattr(pipeline, method_name), f"Missing method: {method_name}"
            assert callable(getattr(pipeline, method_name)), f"Not callable: {method_name}"

    def test_steps_are_pipeline_step_objects(self):
        """Each step should be a PipelineStep object."""
        pipeline = CharlotteMapPipeline(verbose=False)

        for step_name, step in pipeline.steps.items():
            assert isinstance(step, PipelineStep), f"{step_name} is not a PipelineStep"
            assert step.name == step_name
            assert step.display_name is not None
            assert step.description is not None


class TestStepDependencies:
    """Tests for step dependency configuration."""

    def test_dem_before_heightmap(self):
        """DEM download should be required for heightmap generation."""
        pipeline = CharlotteMapPipeline(verbose=False)

        heightmap_step = pipeline.steps['generate_heightmap']
        assert 'download_dem' in heightmap_step.requires, \
            "Heightmap generation should require DEM download"

    def test_highway_before_transform(self):
        """Highway extraction should be required for vector transform."""
        pipeline = CharlotteMapPipeline(verbose=False)

        transform_step = pipeline.steps['transform_vectors']
        assert 'extract_highway' in transform_step.requires, \
            "Vector transform should require highway extraction"

    def test_tiles_no_dependencies(self):
        """Tile download should have no dependencies."""
        pipeline = CharlotteMapPipeline(verbose=False)

        tiles_step = pipeline.steps['download_tiles']
        assert len(tiles_step.requires) == 0, \
            "Tile download should have no dependencies"

    def test_dem_no_dependencies(self):
        """DEM download should have no dependencies."""
        pipeline = CharlotteMapPipeline(verbose=False)

        dem_step = pipeline.steps['download_dem']
        assert len(dem_step.requires) == 0, \
            "DEM download should have no dependencies"


class TestCriticalSteps:
    """Tests for critical step designation."""

    def test_highway_is_critical(self):
        """Highway extraction should be marked as critical."""
        pipeline = CharlotteMapPipeline(verbose=False)

        highway_step = pipeline.steps['extract_highway']
        assert highway_step.critical == True, \
            "Highway extraction should be critical"

    def test_transform_is_critical(self):
        """Vector transform should be marked as critical."""
        pipeline = CharlotteMapPipeline(verbose=False)

        transform_step = pipeline.steps['transform_vectors']
        assert transform_step.critical == True, \
            "Vector transform should be critical"

    def test_tiles_not_critical(self):
        """Tile download should not be critical."""
        pipeline = CharlotteMapPipeline(verbose=False)

        tiles_step = pipeline.steps['download_tiles']
        assert tiles_step.critical == False, \
            "Tile download should not be critical"

    def test_dem_not_critical(self):
        """DEM download should not be critical."""
        pipeline = CharlotteMapPipeline(verbose=False)

        dem_step = pipeline.steps['download_dem']
        assert dem_step.critical == False, \
            "DEM download should not be critical"


class TestStepOrder:
    """Tests for step execution order."""

    def test_step_order_constant(self):
        """STEPS constant should define execution order."""
        step_names = [step.name for step in CharlotteMapPipeline.STEPS]

        # First two should be tiles and dem (order may vary as both have no deps)
        first_two = set(step_names[:2])
        assert first_two == {'download_tiles', 'download_dem'}

        # Highway should be third (no deps, critical)
        assert step_names[2] == 'extract_highway'

        # Heightmap should be fourth (requires dem)
        assert step_names[3] == 'generate_heightmap'

        # Transform should be last (requires highway)
        assert step_names[4] == 'transform_vectors'


class TestDirectoryCreation:
    """Tests for directory structure creation."""

    def test_pipeline_creates_directories_on_init(self):
        """Pipeline should create data directories on initialization."""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_dir = Path(tmpdir)
            pipeline = CharlotteMapPipeline(base_dir=base_dir, verbose=False)

            expected_dirs = [
                pipeline.data_dir,
                pipeline.raw_dir / 'tiles',
                pipeline.raw_dir / 'dem',
                pipeline.raw_dir / 'osm',
                pipeline.processed_dir / 'heightmaps',
                pipeline.processed_dir / 'vectors',
                pipeline.processed_dir / 'vectors' / 'unreal',
            ]

            for dir_path in expected_dirs:
                assert dir_path.exists(), f"Directory not created: {dir_path}"


class TestStepStatus:
    """Tests for step status enum."""

    def test_step_status_values(self):
        """StepStatus should have all required values."""
        assert StepStatus.PENDING.value == "pending"
        assert StepStatus.RUNNING.value == "running"
        assert StepStatus.SUCCESS.value == "success"
        assert StepStatus.FAILED.value == "failed"
        assert StepStatus.SKIPPED.value == "skipped"


class TestPipelineResults:
    """Tests for pipeline result tracking."""

    def test_pipeline_tracks_results(self):
        """Pipeline should track step results."""
        pipeline = CharlotteMapPipeline(verbose=False)

        # Initially no results
        assert len(pipeline.results) == 0

    def test_pipeline_has_start_time_none(self):
        """Pipeline start_time should be None before execution."""
        pipeline = CharlotteMapPipeline(verbose=False)

        assert pipeline.start_time is None


class TestPipelineListSteps:
    """Tests for step listing functionality."""

    def test_steps_property_returns_dict(self):
        """steps property should return a dictionary."""
        pipeline = CharlotteMapPipeline(verbose=False)

        steps = pipeline.steps
        assert isinstance(steps, dict)
        assert len(steps) == 5

    def test_steps_dict_has_correct_keys(self):
        """steps dict should have step names as keys."""
        pipeline = CharlotteMapPipeline(verbose=False)

        steps = pipeline.steps
        expected_keys = {
            'download_tiles',
            'download_dem',
            'extract_highway',
            'generate_heightmap',
            'transform_vectors'
        }

        assert set(steps.keys()) == expected_keys


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
