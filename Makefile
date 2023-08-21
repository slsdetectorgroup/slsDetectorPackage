# TODO! Add support for making the pkg? 
# Which tests should we have? 

default: ext

ext: ## [DEFAULT] build c extension in place
	rm -rf build/ pyctbgui/_decoder.cpython*
	python setup.py build_ext --inplace

clean: ## Remove the build folder and the shared library
	rm -rf build/ pyctbgui/_decoder.cpython*

test: ## Run unit tests using pytest
	python -m pytest -v

check_format:
	@ruff check . &&  echo "format checks passed ✅"

format: ## format code inplace using style in pyproject.toml
	yapf --style pyproject.toml -m -r -i .

help: # from compiler explorer
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
