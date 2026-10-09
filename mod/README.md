<p align="center">
   <img src="https://YOUR-SABERRANK-API-DOMAIN/SnoreSaber-iOS-Default-1024x1024@1x.png" title="SnoreSaber" alt="SnoreSaber icon" width="96" />
</p>

<h1 align="center">PC Mod</h1>

The [BSIPA](https://github.com/nike4613/BeatSaber-IPA-Reloaded) plugin for SnoreSaber on PC 

## Multi-version builds

The mod supports these base targets and compatible Beat Saber versions:

- `1.29.0` → 1.29.0–1.29.1
- `1.37.1` → 1.37.1–1.37.2
- `1.38.0` → 1.38.0–1.39.1
- `1.40.0` → 1.40.0–1.40.8
- `1.42.0` → 1.42.0–1.44.1

On Windows with BSManager, `build.ps1 -TargetVersion All` automatically searches `C:\Users\<you>\BSManager\BSInstances` for compatible installations. You can also build one target, for example `build.ps1 -TargetVersion 1.40.0`.

The `1.40.0` target is the compatibility target for Beat Saber 1.40.8, so your 1.40.8 installation should be used for that build.

## Local Build Settings

If you want to be able to upload scores from a dev build of SnoreSaber (without being rude about it) you're going to need a dev token. Feel free to contact one of our admins for one. You can find their social contact information [here](https://YOUR-SABERRANK-API-DOMAIN/team) of if emails more your thing, here ya go: developers@saberrank.local

For local MSBuild secrets/settings, copy `Directory.Build.local.props.example` to `Directory.Build.local.props`

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for code standards and pull request expectations
