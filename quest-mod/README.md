# SnoreSaber Quest

SnoreSaber implementation for Quest.

Currently should be almost 1:1 to the PC version of SnoreSaber, though some features are missing / different which can be seen [here](https://github.com/SnoreSaber/SnoreSaber-Quest/issues?q=is%3Aissue+is%3Aopen+label%3Afeature-parity)

If you are a user looking to install SnoreSaber on Quest, download the latest official `.qmod` from GitHub Releases.

## Building

Requires [QPM](https://github.com/QuestPackageManager/QPM.CLI), CMake, Ninja, and an Android NDK (write its path into `ndkpath.txt` in the repo root).

```sh
qpm restore   # fetch dependencies into extern/
qpm s build   # configure + build into build/
```

To package a `.qmod`: `qpm qmod build`, then `pwsh ./scripts/createqmod.ps1 SnoreSaber`. Note that locally built qmods will not authenticate with SnoreSaber unless official or development upload-trust metadata is provided.

### Build all five supported targets in one run (Windows)

Install QPM, CMake, Ninja, PowerShell 7, and the Android NDK first. Set `ndkpath.txt` to your installed NDK path. Then double-click `Build-All-Quest-Versions.bat`, or run `pwsh -ExecutionPolicy Bypass -File ./scripts/build-all.ps1`. The Windows launcher supports Windows PowerShell 5.1 as well as PowerShell 7, and temporarily rewrites QPM workspace script commands that otherwise hard-code `pwsh`. The script sequentially restores the target-specific `bs-cordl` dependency, clean-builds and packages Quest versions `1.29.0`, `1.37.0`, `1.38.0`, `1.40.8`, and `1.42.0`, and writes the `.qmod` files plus `build-summary.json` into `dist/`.

If a target fails, the script stops and keeps packages that were already built. It restores your original `qpm.json` and `mod.template.json` afterward. Local builds do not authenticate with SnoreSaber upload trust unless official or development trust metadata is provided. Successful compilation does not replace runtime testing on each Quest game version.

## Credits

-  [RedBrumbler](https://github.com/RedBrumbler), [Optimus](OptimusChen) - UI Implementation
-  [Qwasyx](https://github.com/Qwasyx) - Maintainer
-  [zoller27osu](https://github.com/zoller27osu), [Sc2ad](https://github.com/Sc2ad) and [jakibaki](https://github.com/jakibaki) - [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook)
-  [raftario](https://github.com/raftario)
-  [Lauriethefish](https://github.com/Lauriethefish) and [danrouse](https://github.com/danrouse) for [this template](https://github.com/Lauriethefish/quest-mod-template)


Windows fix note: QPM's shared workspace configuration (`qpm.shared.json`, under `config.workspace.scripts`) also hard-coded `pwsh`. This package replaces those local script commands with `powershell.exe`. Extract this ZIP into a new folder; do not reuse an older extracted folder.


QPM prerequisite: if the build says `qpm` is not recognized, install the QPM CLI from https://github.com/QuestPackageManager/QPM.CLI/releases and ensure `qpm.exe` is on PATH. The build script now checks for QPM before starting and searches a few common install folders.


JSON encoding note: use the QPM-Fix2 package if `qpm restore` reports `expected value at line 1 column 1`. Windows PowerShell 5.1 can write UTF-8 BOMs with `Set-Content -Encoding utf8`; the updated build script writes config JSON without a BOM. To repair an already affected `qpm.json`, remove the leading U+FEFF before running QPM again.
