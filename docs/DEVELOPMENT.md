# Development layout

The repository root builds the maintained GTK application. Application and
shared C++ code live in `src/`, GResource inputs and CSS live in `data/`, and
tests live in `tests/`.

Configure, build, and test a fresh tree from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

`legacy/qt/` is a read-only historical reference. It is excluded from source
archives and is not built, tested, packaged, or used at runtime.
