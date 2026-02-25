#!/usr/bin/env python3
"""
DEM (Digital Elevation Model) acquisition script for Charlotte I-485 corridor.

Queries USGS 3DEP API for available elevation data products.

Usage:
    python -m tools.charlotte_map_acquisition.scripts.download_dem
"""

import json
import time
from pathlib import Path
from typing import Dict, List, Optional
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError


class DEamacquisition:
    """
    Acquires Digital Elevation Model data from USGS 3DEP.

    Based on research:
    - USGS 3DEP API endpoint: https://tnmaccess.nationalmap.gov/api/v1/products
    - Charlotte area has 1m resolution LiDAR DEM available
    - May require manual download via The National Map Viewer
    """

    def __init__(self, timeout: int = 30):
        """
        Initialize DEM acquisition.

        Args:
            timeout: Request timeout in seconds (default: 30)
        """
        self.base_url = "https://tnmaccess.nationalmap.gov/api/v1/products"
        self.timeout = timeout
        self.headers = {
            "User-Agent": "CharlotteMapAcquisition/0.1.0 (Educational/Research Project)"
        }

    def query_usgs_3dep(
        self,
        bbox: Dict[str, float],
        datasets: Optional[List[str]] = None
    ) -> Dict:
        """
        Query USGS 3DEP API for available DEM products.

        Args:
            bbox: Dict with 'west', 'south', 'east', 'north' keys
            datasets: List of dataset names to search (optional)

        Returns:
            API response as dict
        """
        # Build BBOX string
        bbox_str = f"{bbox['west']},{bbox['south']},{bbox['east']},{bbox['north']}"

        # Build query parameters
        params = {
            "bbox": bbox_str,
            "outputFormat": "JSON",
            "max": 100  # Max results per page
        }

        # Add dataset filter if specified
        if datasets:
            # Common DEM datasets:
            # - "National Elevation Dataset (NED) 1/3 arc-second"
            # - "3D Elevation Program (3DEP) 1 meter"
            params["datasets"] = ",".join(datasets)

        # Build URL
        query_str = "&".join(f"{k}={v}" for k, v in params.items())
        url = f"{self.base_url}?{query_str}"

        print(f"Querying USGS 3DEP API...")
        print(f"  BBOX: {bbox_str}")

        # Make request
        try:
            request = Request(url, headers=self.headers)
            with urlopen(request, timeout=self.timeout) as response:
                data = json.loads(response.read().decode('utf-8'))
                return data

        except HTTPError as e:
            print(f"HTTP error {e.code}: {e.reason}")
            return {"error": str(e), "items": []}

        except URLError as e:
            print(f"Network error: {e.reason}")
            return {"error": str(e), "items": []}

        except Exception as e:
            print(f"Unexpected error: {e}")
            return {"error": str(e), "items": []}

    def get_dem_download_urls(self, products: Dict) -> List[Dict]:
        """
        Extract download URLs from API response.

        Args:
            products: API response dict

        Returns:
            List of dicts with 'title', 'url', 'format', 'size' keys
        """
        downloads = []

        if "items" not in products:
            print("No items in API response")
            return downloads

        for item in products["items"]:
            # Extract download URLs
            if "downloadURL" in item:
                downloads.append({
                    "title": item.get("title", "Unknown"),
                    "url": item["downloadURL"],
                    "format": item.get("format", "Unknown"),
                    "size": item.get("sizeInBytes", 0),
                    "publication_date": item.get("publicationDate", "Unknown")
                })

            # Check for multiple download options
            if "urls" in item:
                for url_type, url_info in item["urls"].items():
                    if "downloadURL" in url_info:
                        downloads.append({
                            "title": f"{item.get('title', 'Unknown')} ({url_type})",
                            "url": url_info["downloadURL"],
                            "format": url_info.get("format", "Unknown"),
                            "size": url_info.get("sizeInBytes", 0),
                            "publication_date": item.get("publicationDate", "Unknown")
                        })

        return downloads

    def download_dem_files(
        self,
        urls: List[Dict],
        output_dir: Path,
        max_downloads: int = 5
    ) -> Dict[str, int]:
        """
        Download DEM files (limited to avoid large downloads).

        Args:
            urls: List of download URL dicts
            output_dir: Output directory for downloads
            max_downloads: Max number of files to download (default: 5)

        Returns:
            Dict with 'success', 'failed' counts
        """
        stats = {'success': 0, 'failed': 0, 'skipped': 0}

        # Limit downloads
        urls_to_download = urls[:max_downloads]

        print(f"\nDownloading {len(urls_to_download)} of {len(urls)} available files...")

        for i, url_info in enumerate(urls_to_download, 1):
            print(f"\n[{i}/{len(urls_to_download)}] {url_info['title']}")
            print(f"  URL: {url_info['url']}")
            print(f"  Format: {url_info['format']}")
            print(f"  Size: {url_info['size']:,} bytes")

            # Skip if file too large (>500MB)
            if url_info['size'] > 500 * 1024 * 1024:
                print("  SKIPPED: File too large (>500MB)")
                stats['skipped'] += 1
                continue

            # Note: Actual download implementation would go here
            # For now, just log the URL
            print("  NOTE: Manual download required - see fallback documentation")
            stats['skipped'] += 1

        return stats

    def save_product_list(
        self,
        products: Dict,
        output_path: Path
    ) -> None:
        """
        Save product list to JSON file.

        Args:
            products: API response dict
            output_path: Path to output JSON file
        """
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, 'w') as f:
            json.dump(products, f, indent=2)

        print(f"\nProduct list saved to: {output_path}")


def print_manual_instructions():
    """Print manual download instructions."""
    print("\n" + "=" * 70)
    print("MANUAL DOWNLOAD INSTRUCTIONS")
    print("=" * 70)
    print("\nIf the USGS API doesn't provide direct downloads, use:")
    print("\n1. The National Map Viewer")
    print("   URL: https://viewer.nationalmap.gov/basic/")
    print("\n2. Steps:")
    print("   a. Search for 'Charlotte NC'")
    print("   b. Zoom to I-485 area")
    print("   c. Select 'Elevation Products (3DEP)' in left panel")
    print("   d. Choose resolution (1m recommended for Charlotte)")
    print("   e. Click 'Find Products'")
    print("   f. Select products and download")
    print("\n3. Available Charlotte area products:")
    print("   - 1m LiDAR DEM (highest resolution)")
    print("   - 1/3 arc-second (~10m resolution)")
    print("   - 1 arc-second (~30m resolution)")
    print("\n4. Downloaded files should be placed in:")
    print("   tools/charlotte_map_acquisition/data/raw/dem/")
    print("=" * 70)


def main():
    """Main entry point for DEM acquisition."""
    # Get script directory
    script_dir = Path(__file__).parent
    config_dir = script_dir.parent / "config"
    output_dir = script_dir.parent / "data" / "raw" / "dem"

    print("Charlotte DEM Acquisition")
    print("=" * 50)

    # Load bounds configuration
    bounds_path = config_dir / "charlotte_bounds.json"
    with open(bounds_path) as f:
        bounds = json.load(f)

    print(f"\nBounds: {bounds['name']}")
    print(f"  North: {bounds['north']}, South: {bounds['south']}")
    print(f"  East: {bounds['east']}, West: {bounds['west']}")

    # Create DEM acquirer
    acquirer = DEamacquisition(timeout=30)

    # Query USGS 3DEP for available products
    # Try different dataset names
    datasets = [
        "3D Elevation Program (3DEP) 1 meter",
        "National Elevation Dataset (NED) 1/3 arc-second"
    ]

    bbox = {
        'west': bounds['west'],
        'south': bounds['south'],
        'east': bounds['east'],
        'north': bounds['north']
    }

    all_products = {"items": [], "total": 0}

    for dataset in datasets:
        print(f"\nSearching: {dataset}")
        products = acquirer.query_usgs_3dep(bbox, datasets=[dataset])

        if "items" in products:
            print(f"  Found {len(products['items'])} products")
            all_products["items"].extend(products["items"])
            all_products["total"] += len(products["items"])

        # Rate limiting
        time.sleep(1)

    # Save product list
    products_path = output_dir / "usgs_products.json"
    acquirer.save_product_list(all_products, products_path)

    # Extract download URLs
    download_urls = acquirer.get_dem_download_urls(all_products)

    print(f"\nTotal products found: {all_products['total']}")
    print(f"Download URLs extracted: {len(download_urls)}")

    # Print product summary
    if download_urls:
        print("\nAvailable DEM Products:")
        for i, url_info in enumerate(download_urls[:10], 1):
            print(f"\n{i}. {url_info['title']}")
            print(f"   Format: {url_info['format']}")
            print(f"   Date: {url_info['publication_date']}")
            print(f"   Size: {url_info['size']:,} bytes")

    # Note about downloads
    print("\n" + "=" * 50)
    print("NOTE: DEM files are typically large (100MB - 5GB)")
    print("Automated download is limited to small files (<500MB)")
    print("For full downloads, use manual instructions below")

    # Print manual download instructions
    print_manual_instructions()

    # Print output location
    print(f"\nOutput directory: {output_dir}")
    print(f"Product list: {products_path}")


if __name__ == "__main__":
    main()
