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
    
    # Options for rendering
    parser.add_argument('--template', type=Path, default=Path(SCRIPT_DIR + "/index.html.j2"),
                        help='Path to Jinja2 template file')
    parser.add_argument('--output', type=Path, default=Path(SCRIPT_DIR + "/index.html"),
                        help='Path to output HTML file')
    
    args = parser.parse_args()
    
    # Render template if requested
    if args.template and args.output:
        render_template(args.template, args.output, args.data)
    elif args.template or args.output:
        parser.error("--template and --output must be used together")


if __name__ == '__main__':
    main()
