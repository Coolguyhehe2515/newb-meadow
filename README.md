# Newb Meadow

**Newb Meadow** is a realism shader for Minecraft Bedrock, forked from [Newb X](https://github.com/devendrn/newb-x-mcbe). Basically I wanted more natural looking skies and clouds instead of the usual vanilla-style look, plus a warmer cocoa-ish color grade overall. It's heavier than the base shader since realism isn't free performance-wise, so keep that in mind on weaker devices. Runs on Minecraft Bedrock 1.26+, Android/iOS only for now.

<br>

![Screenshots](docs/screenshots.jpg "Newb Meadow")

## Downloads

This shader hasn't been approved by the Newb community admins yet, so downloads aren't available at this time.

## Installation

> [!NOTE]
> Shaders are not officially supported on Minecraft Bedrock. The following are unofficial ways to load shaders. There are multiple ways to get it working. Start with the recommended method. If that doesn't work try the other method.

### Android

| **Using MB Loader App** |
|:-|
| 1. Install [MB Loader App](https://play.google.com/store/apps/details?id=io.github.bambosan.mbloader) |
| 2. Launch Minecraft from MB Loader. |
| 3. Import the resource pack and activate it in global resources. |


### iOS
Using shaders on iOS is not very straightforward and not recommended for beginners.

| **Using Minecraft with Hynis** |
|:-|
| 1. Download Minecraft with Hynis IPA file from our [Discord server](https://discord.com/invite/newb-community-844591537430069279). |
| 2. Sideload it with your preferred sideloading tool. |
| 3. Import the resource pack and activate it in global resources. |

<br>

## Building

### Install dependencies
- [Git](https://git-scm.com/)
- [Python](https://www.python.org/) 3.11 or higher required
- Python packages:
  - [lazurite](https://veka0.github.io/lazurite/#installation) (Must be `v0.8.4`. Newer or older version may not be supported)
  - [rich](https://rich.readthedocs.io/en/stable/introduction.html#installation) (Must be `v14.x.x`)

### Get source code
```
git clone https://github.com/KrispyBRN/newb-meadow/ -c core.symlinks=true
cd newb-meadow
```

### Install dependencies from requirements.txt
*Skip if you already have installed those versions.*
```
python -m pip install -r requirements.txt
```

### Setup build environment
```
./build.sh setup
```
This will download shaderc binary and material data required to build shader.

<br>

### Compile specific shader materials
```
./build.sh mats
```
Compiled material.bin files will be inside `build/<platform>/`

**Command usage:**
```
usage: build mats [-h] [-p {android,windows,merged,ios}] [-m M [M ...]] [-s S]

options:
  -h, --help            show this help message and exit
  -p {android,windows,merged,ios}
                        build profile
  -m M [M ...]          build materials (eg: Sky)
  -s S                  subpack config to use (eg: NO_WAVE)
```

### Compile and build full shader pack
```
./build.sh pack
```

The final mcpack will be inside `build/`.

**Command usage:**
```
usage: build pack [-h] [-p {android,windows,merged,ios}] [--no-zip] [--no-label] [-v V]

options:
  -h, --help            show this help message and exit
  -p {android,windows,merged,ios}
                        build profile
  --no-zip              don't make archive
  --no-label            don't label materials
  -v V                  version number eg: 17
```

> [!TIP]
> If you want to customize pack name, author, version and other details, you can do so in `src/newb/pack_config.toml`.

<br>

## Development

Clangd can be used to get code completion and error checks for source files inside include/newb. Fake bgfx header and clangd config are provided for the same.
- **Neovim**: Install clangd LSP.
- **VSCode**: Install [vscode-clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) extension.

## License

**Source Code:** The "Newb Meadow" source code, forked from "Newb Shader" (MIT License), is licensed under the MIT License. You are free to modify, distribute, and create derivative works based on the source code.

**Compiled Resource Packs (`.mcpack` files):** The compiled resource packs distributed by the "Newb Meadow" project are copyrighted works with restrictions. See the `COPYRIGHT.txt` file within the resource pack for more information.
