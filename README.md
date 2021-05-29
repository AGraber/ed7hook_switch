# ed7hook_switch
Japanese and English localization patches for the Nintendo Switch ports of the Crossbell games by CLE:
- 英雄伝説 零の軌跡：改 (Zero no Kiseki: Kai | "Trails from Zero") [01005E5013862000]
- 英雄伝説 碧の軌跡：改 (Ao no Kiseki: Kai | "Trails to Azure") [010085B01388A000] (WIP)

This project is based on [Skyline](https://github.com/skyline-dev/skyline), an enviroment
for linking, runtime hooking and code patching originally for SSBU.

#### Note:
This repository is meant for development of the patch. If you just want to play
Trails from Zero on English on your Switch, wait until this patch is properly
packaged with translated assets and released.

## Building
- Install libwebp dependency with devkitpro: `pacman -S switch-libwebp`
- Build using `make`

## Installing
Place the resulting `subsdk9` file into the exefs of the game. This can be
easily done through atmosphere's LayeredFS feature.

## Features
- Adds support for PNG and WebP for textures.
- Enables Japanese mode on Switch consoles with language set to Japanese (title screen missing).
- If not playing on any of the officially supported languages or Japanese, the patch will
attempt to English assets from the `data_en` folder instead of `data`.
- Various patches for proper variable width font support across the game.
- Patches translation table (originally meant for Korean/Translation) with English translations,
as well as other functions that pass around unlocalized strings.

# Thanks to
- zakaria for providing aditional patches, executable text translation, and testing the patch
- [TommyB](https://github.com/TommyB123) for helping me test the patch
- [The Geofront](https://github.com/GeofrontTeam) for their awesome translations of both games
- The Skyline Project
- [SciresM](https://github.com/SciresM) for helping me figure out an issue with Azure's npdm
file about permissions for accessing another game's saves

# Skyline Original Credits:
This project is derived from OdysseyReversed and Starlight
- [3096](https://github.com/3096)
- [khang06](https://github.com/khang06)
- [OatmealDome](https://github.com/OatmealDome)
- [Random0666](https://github.com/random0666)
- [shadowninja108](https://github.com/shadowninja108)
- [shibbo](https://github.com/shibbo) - Repo derived from their work on OdysseyReversed
- [Thog](https://github.com/Thog) - Expertise in how rtld is implemented
- [jakibaki ](https://github.com/jakibaki) - Advice with numerous things, including runtime hooking

# Credits
- devkitA64
- libnx - switch build rules
