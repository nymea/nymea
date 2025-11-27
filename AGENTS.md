# Repository Guidelines

## Project Structure & Module Organization
- `server/`: nymead entry points, service glue, and startup helpers.  
- `libnymea/`, `libnymea-core/`: reusable device APIs, JSON-RPC helpers, shared Qt utilities.  
- `plugins/`: protocol modules, each with scoped sources, resources, and metadata (`*.json`).  
- `tests/`: `auto/` (QtTest suites), `libnymea-tests/` (fixtures), `tools/` (harness utilities).  
- `doc/`, `icons/`, `translations/`, `data/`: docs, assets, localization, sample configs; `debian*/`: packaging. Keep new code beside the layer it extends to avoid cross-module coupling.

## Build, Test, and Development Commands
```bash
qmake nymea.pro && make -j"$(nproc)"            # full build
make install INSTALL_ROOT=$PWD/stage            # staged install
./nymead -c data/nymead.conf                    # run locally
qmake tests/tests.pro && make check             # run QtTest suites
```
Select the required Qt version before calling qmake (Qt 5 or Qt 6). Component-only work can be built from its subdir (e.g., `cd server && qmake server.pro`). Enable verbose logs with `QT_LOGGING_RULES="*.debug=true"`.

## Coding Style & Naming Conventions
- Qt style: 4-space indent, braces on their own line, `CamelCase` classes, `lowerCamelCase` functions/members.
- Prefer Qt containers/signals over STL; wrap log output in `NYMEA_LOGGING_CATEGORY`.  
- C++/Qt: follow Qt conventions (camelCase, `m_` member prefixes, grouped/alphabetised includes, Qt containers, Qt logging macros).
- Keep plugin IDs, translation filenames, and resource prefixes aligned with their directory names.  
- When user-visible text changes, refresh translations via `tools/update-translations.sh` (runs `lupdate`/`lrelease`).

## Testing Guidelines
- Tests sit in `tests/auto/<topic>/test*.cpp` and use `QTEST_MAIN`.  
- `qmake tests/tests.pro` generates makefiles; `make check` executes every target (use `make -C tests/<suite>` for a slice).  
- Mirror production directories when creating suites, mock I/O with helpers in `tests/tools/`, and cover both success paths and primary failure modes before sending a PR.

## Commit & Pull Request Guidelines
- Follow history conventions: imperative subjects under ~72 chars (“Add backup configuration…”).  
- Bodies should mention motivation and key testing steps; tag related issues.  
- PRs require scope summary, validation proof (`Tests: qmake tests && make check`), and screenshots or API traces whenever behavior changes.  
- Call out configuration or packaging impacts (e.g., added plugin JSON, changed defaults) so reviewers can replicate.

## Security & Configuration Tips
- Grant network discovery rights after manual installs: `sudo setcap cap_net_admin,cap_net_raw=eip /path/to/nymead`.  
- Store secrets in local overrides such as `/etc/nymea/*.conf` instead of `data/`.  
- Review logging before submission to ensure credentials and tokens remain redacted, and describe any new capability or permission requirements inside the PR.
