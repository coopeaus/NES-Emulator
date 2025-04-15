## Python Bindings

The purpose of this directory to compile the emulator C++ core to a Python module. It does not effect the main build.

### Pre-requisites

Package installation is not handled by the main build system. To compile, you'll need to install the `pybind11` package. I recommend installing it with `pip`, either globally or in a virtual environment.

`pip install pybind11`

See the [pybind11 docs](https://pybind11.readthedocs.io/en/stable/installing.html#) for more details.

### Build instructions

- Run `scripts/build.sh` from this directory

### Adding new methods

- Add a wrapper method to `emu.cpp` and expose it to the `PYBIND11_MODULE`
- Rebuild the module
- Add the method name to `method_names` list in `emu.py`
- Add function definition to `emu.pyi`, for linting and IDE hints

### Usage

- Add `import emu` to the top of any Python script, and you can use any of the exposed methods.
- Execute as you would any other Python script `python3 emu.py` (or in a Python shell: `python3 -i emu.py`)
- See `emu.py` and `test.py` for examples.
