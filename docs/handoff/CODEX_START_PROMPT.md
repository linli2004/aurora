# Codex Start Prompt
## 首次本地交接会话

将以下内容复制给 Codex：

---

You are taking over local development of Aurora Music Framework.

Do not edit files yet.

First:

1. Read `AGENTS.md`.
2. Read every file under `docs/handoff/`.
3. Read `docs/engineering/SOURCE_OF_TRUTH.md`.
4. Read `docs/engineering/ARCHITECTURE.md`.
5. Read the current `README.md` and `CHANGELOG.md`.
6. Inspect the current Git status, branch, commit, untracked files and recent history.
7. Inspect the build presets and test registration.
8. Report the actual current version from code and Git. Do not infer it from chat history.
9. Run no destructive commands.
10. Do not install dependencies.

Then produce a handoff audit with:

- repository map;
- implemented capabilities;
- current build and test commands;
- current architecture;
- mismatches between code and documentation;
- uncommitted or suspicious files;
- known transition implementation and its control flow;
- current risks;
- recommended first task;
- exact files that first task would change.

The first task is `AUR-013 — Freeze Local Handoff Baseline`.

For AUR-013:

- do not change product behavior;
- do not refactor;
- update only documentation or repository metadata after reporting the plan;
- configure and build the dev preset;
- run all tests;
- build and launch Release when possible;
- record the environment;
- propose a Git tag;
- report all commands and results.

Stop and ask before changing code if the repository state does not match the handoff documents.
