# TODO! Add support for making the pkg? 
# Which tests should we have? 

default: ext

ext: ## [DEFAULT] build c extension in place
	rm -rf build/ pyctbgui/_decoder.cpython*
	python setup.py build_ext --inplace

clean: ## Remove the build folder and the shared library
	rm -rf build/ pyctbgui/_decoder.cpython*

test: ## Run unit tests using pytest
	python -m pytest -v tests/unit

test_gui: ## Run E2E tests using pytest
	python -m pytest -v tests/gui

setup_gui_test: ## Setup the environment for the E2E tests
	ctbDetectorServer_virtual > /tmp/simulator.log 2>&1 &
	slsReceiver > /tmp/slsReceiver.log 2>&1 &
	sleep 3
	sls_detector_put config tests/gui/data/simulator.config

killall: ## Kill all the processes started by setup_gui_test
	killall slsReceiver ctbDetectorServer_virtual


lint: ## run ruff linter to check formatting errors
	@ruff check tests pyctbgui *.py &&  echo "Ruff checks passed ✅"

format: ## format code inplace using style in pyproject.toml
	yapf --style pyproject.toml -m -r -i tests pyctbgui *.py

check_format: ## Check if source is formatted properly
	yapf --style pyproject.toml -r -d tests pyctbgui *.py

help: # from compiler explorer
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
