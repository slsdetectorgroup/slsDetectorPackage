# New Chip Test Board Gui using Python
Prototype for a new python based GUI for the Chip Test Board


## Getting started

```bash 
git clone https://github.com/slsdetectorgroup/pyctbgui.git
cd pyctbgui
make #compiles the c extension inplace
./CtbGui
```


## Display help for the Makefile

```
$ make help
check_format         Check if source is formatted properly
clean                Remove the build folder and the shared library
ext                  [DEFAULT] build c extension in place
format               format code inplace using style in pyproject.toml
lint                 run ruff linter to check formatting errors
test                 Run unit tests using pytest
```


## setup pre-commit hooks
```
pre-commit install
```
