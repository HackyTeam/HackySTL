# Agent guidelines for HackySTL

This file tells AI coding agents how to work in this repository. Its first
section restates the project's AI use policy from [README.md](README.md#ai-use-policy);
the rest are working rules that follow from it.

## AI use policy

AI use in contributions is allowed as long as all four of these hold:

1. **Simple and understandable.** The code must be understandable by every
   library contributor. Prefer plain code that matches the surrounding style
   over clever or dense code. If a change needs a long explanation to follow,
   simplify it first.
2. **Reviewed and verified by a non-agent.** A human must review and verify
   the output. An agent must not describe its own work as verified, correct or
   finished. Report exactly what was built and run, what passed, and what still
   needs a person to check.
3. **Disclosed.** Say that AI was used. Put the disclosure in commit messages
   and pull request descriptions, for example with a `Co-Authored-By` trailer
   that names the model, and mention it in replies when it is relevant.
4. **Explained thoroughly.** Explain what changed, why it changed, and any
   trade-offs or limits. Put this in the commit or pull request description and
   in the reply. Do not leave a change that nobody can explain.

## Working rules

- Read the code you are changing and the tests that cover it before editing.
  Reuse existing helpers instead of adding parallel ones.
- Follow [STYLE_GUIDES.md](STYLE_GUIDES.md): Allman braces, 4-space indent,
  snake_case names, PascalCase file names.
- The library does not use exceptions (`-fno-exceptions`). Report failures
  through `hsd::result`, `hsd::option` and `hsd::option_err`.
- Every bug fix needs a regression test under `Tests/` that CTest runs. Add the
  test before or with the fix, and confirm it fails without the fix.
- Examples in `Examples/` print their results and use `.cases` files. They must
  not use `assert`, `<cassert>` or `std::print`; `cmake/CheckExamples.py`
  enforces this.
- Keep the module docs in step with the code: `Tests/README.md` for new test
  files, `Examples/README.md` for new examples, and `Docs/LibraryReference.md`
  for new headers.
- Do not commit, push, amend or rewrite history unless the user asks.
- The working tree can contain edits by other people. Do not revert or
  overwrite changes you did not make. If a file changed unexpectedly, say so.

## Verifying a change

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Build with Clang as well when the change touches templates or the standard
library interface:

```sh
cmake -S . -B build-clang -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
```

Report the commands you ran and their results. A passing run is evidence for a
human reviewer, not a replacement for their review.
