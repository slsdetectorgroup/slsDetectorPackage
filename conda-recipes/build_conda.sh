
SCRIPTDIR=$(cd "$(dirname "$0")" && pwd)
VERSION=$(cat $SCRIPTDIR/../VERSION)
export VERSION

if [ -z "$1" ]
then
    output_dir=$CONDA_PREFIX/conda-bld
else
    output_dir=$1
fi

if [ -z "$2" ]
then
    recipe=$SCRIPTDIR
else
    recipe=$SCRIPTDIR/$2
fi

echo "building conda package version: $VERSION using recipe $recipe and writing output to $output_dir"


conda build $recipe --output-folder $output_dir
