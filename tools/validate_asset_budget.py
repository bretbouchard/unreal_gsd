#!/usr/bin/env python3
"""
Asset Budget Validation Script for CI/CD Integration

Validates asset sizes against configurable budgets defined in Config/AssetBudgets.json.
Exits with code 0 if all assets within budget, 1 if any asset exceeds budget.

Usage:
    python validate_asset_budget.py --config Config/AssetBudgets.json --content Content/

Arguments:
    --config: Path to asset budget configuration JSON (default: Config/AssetBudgets.json)
    --content: Path to Content directory (default: Content/)
    --verbose: Enable verbose output (default: False)
    --json: Output results as JSON (default: False)
"""

import os
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, List, Tuple

class AssetBudgetValidator:
    def __init__(self, config_path: str, content_dir: str, verbose: bool = False):
        self.config_path = config_path
        self.content_dir = content_dir
        self.verbose = verbose
        self.config = self.load_config()
        self.results = {
            'passed': True,
            'total_assets': 0,
            'errors': 0,
            'warnings': 0,
            'issues': []
        }

    def load_config(self) -> Dict:
        """Load asset budget configuration from JSON file"""
        try:
            with open(self.config_path, 'r') as f:
                config = json.load(f)
            if self.verbose:
                print(f"Loaded config from: {self.config_path}")
            return config
        except FileNotFoundError:
            print(f"ERROR: Config file not found: {self.config_path}", file=sys.stderr)
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"ERROR: Invalid JSON in config file: {e}", file=sys.stderr)
            sys.exit(1)

    def get_asset_type(self, asset_path: str) -> str:
        """Determine asset type from file path"""
        # Unreal asset types based on directory structure or metadata
        # This is a simplified approach - production would parse .uasset files
        path_lower = asset_path.lower()

        if '/geometry/' in path_lower or '/meshes/' in path_lower:
            return 'StaticMesh'
        elif '/textures/' in path_lower or '/materials/' in path_lower:
            return 'Texture2D'
        elif '/characters/' in path_lower or '/skeletal/' in path_lower:
            return 'SkeletalMesh'
        elif '/audio/' in path_lower or '/sounds/' in path_lower:
            return 'SoundWave'
        elif '/animations/' in path_lower:
            return 'AnimSequence'
        elif '/blueprints/' in path_lower:
            return 'Blueprint'
        else:
            return 'default'

    def get_asset_size_mb(self, asset_path: str) -> float:
        """Get asset size in megabytes"""
        try:
            size_bytes = os.path.getsize(asset_path)
            return size_bytes / (1024.0 * 1024.0)
        except OSError as e:
            if self.verbose:
                print(f"Warning: Could not get size for {asset_path}: {e}")
            return 0.0

    def get_all_assets(self) -> List[str]:
        """Find all .uasset files in Content directory"""
        assets = []
        content_path = Path(self.content_dir)

        if not content_path.exists():
            print(f"ERROR: Content directory not found: {self.content_dir}", file=sys.stderr)
            sys.exit(1)

        for root, dirs, files in os.walk(content_path):
            for file in files:
                if file.endswith('.uasset'):
                    asset_path = os.path.join(root, file)
                    assets.append(asset_path)

        return assets

    def validate_asset(self, asset_path: str) -> Tuple[bool, str]:
        """Validate single asset against budget"""
        asset_type = self.get_asset_type(asset_path)
        asset_size_mb = self.get_asset_size_mb(asset_path)

        # Get budget for this asset type
        budgets = self.config.get('asset_budgets', {})
        budget_config = budgets.get(asset_type, budgets.get('default', {'max_size_mb': 100.0}))
        max_size_mb = budget_config.get('max_size_mb', 100.0)
        severity = budget_config.get('severity', 'error')
        description = budget_config.get('description', '')

        self.results['total_assets'] += 1

        # Check against budget
        if asset_size_mb > max_size_mb:
            issue = {
                'asset_path': asset_path,
                'asset_type': asset_type,
                'size_mb': asset_size_mb,
                'budget_mb': max_size_mb,
                'severity': severity,
                'message': f"{asset_path}: {asset_size_mb:.2f}MB exceeds {max_size_mb:.2f}MB budget for {asset_type}"
            }
            self.results['issues'].append(issue)

            if severity == 'error':
                self.results['errors'] += 1
                self.results['passed'] = False
                return False, issue['message']
            else:
                self.results['warnings'] += 1
                return True, issue['message']

        # Check warn threshold
        global_settings = self.config.get('global_settings', {})
        warn_threshold = global_settings.get('warn_threshold_percent', 80.0) / 100.0
        enable_warnings = global_settings.get('enable_warnings', True)

        if enable_warnings and asset_size_mb > (max_size_mb * warn_threshold):
            usage_percent = (asset_size_mb / max_size_mb) * 100.0
            if self.verbose:
                print(f"WARNING: {asset_path} using {usage_percent:.1f}% of budget ({asset_size_mb:.2f}MB / {max_size_mb:.2f}MB)")

        if self.verbose:
            print(f"OK: {asset_path} ({asset_size_mb:.2f}MB, budget: {max_size_mb:.2f}MB)")

        return True, ""

    def validate_all_assets(self) -> bool:
        """Validate all assets against budgets"""
        print(f"Validating assets in: {self.content_dir}")
        print(f"Using config: {self.config_path}")

        assets = self.get_all_assets()
        print(f"Found {len(assets)} assets to validate")

        for asset_path in assets:
            passed, message = self.validate_asset(asset_path)
            if not passed:
                print(f"ERROR: {message}", file=sys.stderr)

        return self.results['passed']

    def print_summary(self):
        """Print validation summary"""
        print("\n" + "="*60)
        print("ASSET BUDGET VALIDATION SUMMARY")
        print("="*60)
        print(f"Total Assets Checked: {self.results['total_assets']}")
        print(f"Errors: {self.results['errors']}")
        print(f"Warnings: {self.results['warnings']}")
        print(f"Status: {'PASSED' if self.results['passed'] else 'FAILED'}")
        print("="*60)

    def output_json(self):
        """Output results as JSON for CI parsing"""
        output = json.dumps(self.results, indent=2)
        print(output)

def main():
    parser = argparse.ArgumentParser(description='Validate asset sizes against budgets')
    parser.add_argument('--config', default='Config/AssetBudgets.json',
                       help='Path to asset budget configuration JSON')
    parser.add_argument('--content', default='Content/',
                       help='Path to Content directory')
    parser.add_argument('--verbose', action='store_true',
                       help='Enable verbose output')
    parser.add_argument('--json', action='store_true',
                       help='Output results as JSON')

    args = parser.parse_args()

    # Create validator
    validator = AssetBudgetValidator(
        config_path=args.config,
        content_dir=args.content,
        verbose=args.verbose
    )

    # Run validation
    passed = validator.validate_all_assets()

    # Output results
    if args.json:
        validator.output_json()
    else:
        validator.print_summary()

    # Exit with appropriate code
    global_settings = validator.config.get('global_settings', {})
    fail_on_error = global_settings.get('fail_on_error', True)

    if not passed and fail_on_error:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == '__main__':
    main()
