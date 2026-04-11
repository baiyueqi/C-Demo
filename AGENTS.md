# AGENTS.md

## Environment
- Start work in `nix develop` at the repo root. `flake.nix` is the real source of truth for tooling: LLVM/Clang 16, `cmake`, Python/Jupyter, `gh`, Boost, `spdlog`, and `nlohmann_json` are provided there.

## Build And Verify
- Do not assume a repo-wide build exists. There is no root `CMakeLists.txt`, test runner, lint config, formatter config, or CI workflow in this repo.
- `README.md` shows generic `cmake -B build && cmake --build build` examples, but there is no checked-in CMake project to run at the root.
- Verify changes locally and narrowly, usually per file or per subdirectory.
- Verified build entrypoints:
  - `make -C OS/process_control` builds `main` from `main.cpp clinic.cpp ticket.cpp` with `clang++ -pthread -std=c++17`
  - `make -C OS/linux_cpp_lab` builds `fork_demo` and `thread_demo`; `make -C <dir> clean` is available in both OS subprojects
- For most other folders, compile the touched source directly, e.g. `clang++ -std=c++17 path/to/file.cpp -o /tmp/<name>`. Add `-pthread` when the file uses threads.
- Many programs are interactive console demos, so "verification" often means compile successfully and, when practical, run with minimal manual input.

## Repo Shape
- This repo is a collection of mostly independent learning/demo programs, not a single application.
- High-signal directories:
  - `OS/`: the only area with checked-in `Makefile`s
  - `foundation/`, `CyberspaceSecurity/`, `InSecurityMath/`, `src/`: mostly standalone source files and small demos compiled ad hoc
  - `notebook/`: notebooks; not part of a verified build flow

## Gotchas
- The repo contains tracked binaries alongside sources, for example `foundation/game`, `foundation/hello`, `CyberspaceSecurity/RSA`, and `InSecurityMath/main`. Do not assume extensionless paths are directories, and avoid editing binary files.
- `.gitignore` only covers generic compiler outputs plus a few specific paths like `/src/out/` and `/foundation/vector/`. New build artifacts in other directories may show up as untracked files.
