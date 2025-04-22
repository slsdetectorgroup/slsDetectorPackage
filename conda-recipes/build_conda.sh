
SCRIPTDIR=$(cd "$(dirname "$0")" && pwd)
VERSION=$(cat $SCRIPTDIR/../VERSION)
export VERSION

echo "building conda package version: $VERSION"

conda build .
