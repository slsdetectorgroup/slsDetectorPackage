"""
Render the index HTML from a Jinja2 template with version data.
Can also add new versions to the YAML data file.

Usage:
  # Just render from existing data
  python render_main_index.py --template main_index.html.j2 --output main_index.html --data versions.yaml
  
  # Add a new version and render
  python render_main_index.py --data versions.yaml --add-version 10.1.0 --type Minor --date "15.10.2025" --template main_index.html.j2 --output main_index.html
"""
import argparse
from pathlib import Path
from jinja2 import Template
import yaml
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

def load_versions(data_path: Path):
    """Load version data from YAML file."""
    with open(data_path, 'r') as f:
        data = yaml.safe_load(f)
    return data['versions']


def save_versions(data_path: Path, versions):
    """Save version data to YAML file."""
    data = {
        'versions': versions
    }
    with open(data_path, 'w') as f:
        yaml.dump(data, f, default_flow_style=False, sort_keys=False)
    print(f"✓ Saved version data to {data_path}")

def extract_release_type(version: str) -> str: 
    """Extract release type from version string."""
    parts = version.split('.')
    if len(parts) != 3:
        return "Unknown"
    major, minor, patch = map(int, parts)
    if minor == 0 and patch == 0:
        return "Major"
    elif patch == 0:
        return "Minor"
    else:
        return "Bug Fix"


def add_version(data_path: Path, version: str, release_type: str, date: str, has_docs: bool = True):
    """Add a new version to the YAML data file."""
    versions = load_versions(data_path)
    
    # Add to table (check if not already present)
    new_entry = {
            'version': version,
            'type': release_type,
            'date': date,
            'has_docs': has_docs
    }
    versions.insert(0, new_entry)
    
    print(f"✓ Added version {new_entry} to version data")
   
    save_versions(data_path, versions)


def render_template(template_path: Path, output_path: Path, data_path: Path):
    """Render the Jinja2 template with version data."""
    versions = load_versions(data_path)
    
    with open(template_path, 'r') as f:
        template = Template(f.read())
    
    html = template.render(
        versions=versions
    )
    
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"✓ Rendered {output_path}")


def main():
    parser = argparse.ArgumentParser(description='Manage versions and render main index HTML')
    parser.add_argument('--data', type=Path, default=Path(SCRIPT_DIR + "/versions.yaml"), 
                        help='Path to versions YAML data file')
    
    # Options for adding a new version
    parser.add_argument('--version', type=str,
                        help='new version (e.g., 10.1.0)')
    parser.add_argument('--date', type=str,
                        help='Release date (e.g., 15.10.2025)')
    
    # Options for rendering
    parser.add_argument('--template', type=Path, default=Path(SCRIPT_DIR + "/index.html.j2"),
                        help='Path to Jinja2 template file')
    parser.add_argument('--output', type=Path, default=Path(SCRIPT_DIR + "/index.html"),
                        help='Path to output HTML file')
    
    args = parser.parse_args()
    
    # Add new version if requested
    if args.version:
        if not args.date:
            parser.error("--version requires --date")
        release_type = extract_release_type(args.version)
        add_version(args.data, args.version, release_type, args.date, has_docs=True)
    
    # Render template if requested
    if args.template and args.output:
        render_template(args.template, args.output, args.data)
    elif args.template or args.output:
        parser.error("--template and --output must be used together")


if __name__ == '__main__':
    main()
