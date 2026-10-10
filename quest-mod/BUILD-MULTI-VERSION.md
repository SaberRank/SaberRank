# SnoreSaber Quest multi-version build matrix

The GitHub Actions workflow now creates a separate build job/artifact for each requested target:

- Beat Saber Quest 1.29.0 (bs-cordl 2900.0.0)
- Beat Saber Quest 1.37.0 (bs-cordl 3700.0.0)
- Beat Saber Quest 1.38.0 (bs-cordl 3800.0.0)
- Beat Saber Quest 1.40.8 (bs-cordl 4008.0.0)
- Beat Saber Quest 1.42.0 (bs-cordl 4200.0.0)

Run Actions → Build and release → Run workflow on `main`. The workflow uploads an independently named `.qmod` artifact for each target. Official build registration is off by default.

## Important compatibility caveat

The 1.37.0 and 1.40.8 package build identifiers are known (`1.37.0_9064817954` and `1.40.8_7379`). The package identifiers for 1.29.0, 1.38.0, and 1.42.0 need to be confirmed from the exact Quest APKs before those `.qmod` files can be treated as install-ready for those game builds. The workflow currently uses version-only identifiers for those three targets so it can expose the separate builds, but QuestPatcher/MBF may require the exact `<version>_<build>` identifier. Do not publish or distribute those three packages until their package identifiers and successful compile/install are verified.

Also, this matrix is a build setup, not proof of runtime parity. Each target needs a successful QPM dependency restore, native compile, install, launch, authentication, leaderboard, and score-upload test. Older targets may reveal API differences beyond the bs-cordl binding selection.
