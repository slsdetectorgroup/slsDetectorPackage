# New Chip Test Board Gui using Python
Prototype for a new python based GUI for the Chip Test Board


## Getting started

To install all dependencies build the package with python package ``build``. This requires that you have ``build`` installed. 
We recommend using a virtual conda environment. 

```bash 
git clone https://github.com/slsdetectorgroup/pyctbgui.git
cd pyctbgui
python -m build 
pip install dist/pyctbgui-0.0.0-cp313-cp313-linux_x86_64.whl 
./CtbGui
```

Note that the above package installs ``slsdet`` and ``aare``. If you want to work on your local branch add the locally build python modules first to your ``PYTHONPATH``. 

## Format Code 

To check python code is formatted correctly according to style guides in pyproject.toml run (requires ``yapf`` to be installed.): 

```bash 
yapf --style pyproject.toml -r -d tests pyctbgui *.py
```

To format inplace using style in pyproject.toml run: 

```bash 
yapf --style pyproject.toml -m -r -i tests pyctbgui *.py
```

## Run Tests: 

The following command runs the unit tests (Requires ``pytest``to be installed): 

```bash
	python -m pytest -v tests/unit
```

The following command runs end to end tests: 

```bash
	python -m pytest -v tests/gui
```

## setup pre-commit hooks

```
pre-commit install
```
