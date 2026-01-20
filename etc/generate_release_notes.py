from pathlib import Path
from datetime import datetime
from tempfile import template
import yaml
import argparse

ROOT_DIR = Path(__file__).resolve().parent.parent 

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
    
def get_previous_version(data_path: Path, version : str) -> str:
    """Get the previous version from the versions YAML file."""
    with open(data_path, 'r') as f:
        data = yaml.safe_load(f)

    prev_version = data['versions'][0]["version"]
    if prev_version == version: #already updated to new version 
        prev_version = data['versions'][1]["version"]

    return prev_version

def main(): 
    parser = argparse.ArgumentParser(description='Update release notes from templated md file')
    parser.add_argument('--version', type=str, help='Version to use in release notes, if not provided will read from VERSION file')
    args = parser.parse_args()

    if args.version:
        version = args.version
    else:
        version = Path(ROOT_DIR / "VERSION").read_text().strip()

    release_type = extract_release_type(version)

    prev_version = get_previous_version(ROOT_DIR / "docs/main_index/versions.yaml", version)

    # Read template
    template = Path(ROOT_DIR / "RELEASE.md.template").read_text()

    # Substitute version and date
    output = template.replace("{{VERSION}}", version)
    output = output.replace("{{DATE}}", datetime.now().strftime("%Y-%m-%d"))
    output = output.replace("{{RELEASE_TYPE}}", release_type)
    output = output.replace("{{PREVIOUS_VERSION}}", prev_version)

    # Write output
    Path(ROOT_DIR / "RELEASE.md").write_text(output)
    print(f"Generated RELEASE.md for version {version}")

if __name__ == "__main__": 
    main()
