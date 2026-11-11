"""
Render the index HTML from a Jinja2 template with release data.

The release data is a JSON list as produced by
  gh release list --json tagName,publishedAt

Usage:
  python render_main_index.py --data releases.json --template index.html.j2 --output index.html
"""
import argparse
import json
from pathlib import Path
from jinja2 import Template
import os
from datetime import date

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

MIN_VERSION = (5, 0, 0)       # releases older than this are not listed
MIN_DOCS_VERSION = (9, 0, 0)  # releases older than this have no docs


def parse_version(tag: str):
    """Return (major, minor, patch) for a tag like '10.0.1', or None for e.g. '7.0.0.rc1'."""
    parts = tag.split('.')
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        return None
    return tuple(int(p) for p in parts)


def release_type(version):
    """Derive the release type from the version number."""
    _, minor, patch = version
    if patch != 0:
        return "Bug Fix"
    if minor != 0:
        return "Minor"
    return "Major"


def read_release_json(data_path: Path):
    """Load release data from JSON file, newest version first."""
    with open(data_path, 'r') as f:
        releases = json.load(f)

    versions = []
    for release in releases:
        version = parse_version(release['tagName'])
        if version is None or version < MIN_VERSION:
            continue
        versions.append({
            'version': release['tagName'],
            'type': release_type(version),
            'date': release['publishedAt'].split('T')[0],
            'has_docs': version >= MIN_DOCS_VERSION,
            '_key': version,
        })

    versions.sort(key=lambda v: v['_key'], reverse=True)
    return versions

def render_template(template_path: Path, output_path: Path, versions : list):
    """Render the Jinja2 template with version data."""

    with open(template_path, 'r') as f:
        template = Template(f.read())

    html = template.render(
        versions=versions
    )

    with open(output_path, 'w') as f:
        f.write(html)

    print(f"✓ Rendered {output_path}")

def add_version(version: str, versions : list):
    """Add a version to the versions dict if it is not already present."""
    tuple_version = parse_version(version)
    if version not in versions:
        versions.insert(0,{
            'version': version,
            'type': release_type(tuple_version),
            'date': date.today().strftime("%Y-%m-%d"),
            'has_docs': tuple_version >= MIN_DOCS_VERSION,
            '_key': tuple_version,
        })
        
    versions.sort(key=lambda v: v['_key'], reverse=True)

def main():
    parser = argparse.ArgumentParser(description='Render main index HTML from release data')
    parser.add_argument('--json_releases', type=Path,
                        help='Path to releases JSON data file')
    
    parser.add_argument('--new_version', type=str,
                        help='New version to add to the releases data')

    # Options for rendering
    parser.add_argument('--template', type=Path, default=Path(SCRIPT_DIR + "/index.html.j2"),
                        help='Path to Jinja2 template file')
    parser.add_argument('--output', type=Path, default=Path(SCRIPT_DIR + "/index.html"),
                        help='Path to output HTML file')
    
    args = parser.parse_args()

    versions = read_release_json(args.json_releases)

    if args.new_version: 
        add_version(args.new_version, versions)

    # Render template if requested
    if args.template and args.output:
        render_template(args.template, args.output, versions)
    elif args.template or args.output:
        parser.error("--template and --output must be used together")


if __name__ == '__main__':
    main()
