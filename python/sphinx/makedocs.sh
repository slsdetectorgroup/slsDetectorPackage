make clean
make html
rm -rf ../docs/
mv  _build/html/ ../docs/
touch ../docs/.nojekyll
rm -rf _build
