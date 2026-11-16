# OpenFrame

Halo: Combat Evolved in VR on the Valve Steam Frame.
Based on [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo.

## Features

- 90 Hz with eye-tracked foveation
- Faster loads
- Recentre that works, HUD sized for the Frame
- Sprint, bullet time and third person on the stick
- Parkour, fists and wrist HUD from moistman42069's mod
- Online play with OpenH1 and OpenCE players

## How it works

The Xbox game's code, rebuilt, runs on the Frame itself in SteamVR. You supply the maps from your own disc.

## Requirements

- A Steam Frame with SteamVR (SteamOS Stable, Beta or Preview), and about 5 GB free
- The 24 `.map` files from your own Xbox disc, not PC or MCC maps ([how to get them](https://openh1.com/maps))
- A terminal on the Frame: SSH or Konsole
- Steam running while you install

## Install

1. Get the repository:

   ```sh
   git clone -b steam-frame https://github.com/simplysanatori/OpenFrame.git
   cd OpenFrame
   ```

2. Download `openframe-1.2.0-linux-arm64.tar.gz` and `SHA256SUMS` from [Releases](https://github.com/simplysanatori/OpenFrame/releases) to `~/Downloads`.

3. Check it and install with your maps:

   ```sh
   (cd ~/Downloads && sha256sum -c --ignore-missing SHA256SUMS)
   steam-frame/install.sh --maps /path/to/your/maps ~/Downloads/openframe-1.2.0-linux-arm64.tar.gz
   ```

Other installers (copy your maps into `~/.local/share/OpenFrame/maps` first):

- [Frametanium](https://github.com/Cogential/frametanium): add `simplysanatori/OpenFrame`.
- [FrameDrop](https://framedropvr.com): [Install with FrameDrop](https://framedropvr.com/install?manifest=https%3A%2F%2Fgithub.com%2Fsimplysanatori%2FOpenFrame%2Freleases%2Flatest%2Fdownload%2Fopenframe.framedrop.json).

## First run

1. Start SteamVR and put the headset on.
2. Launch **OpenFrame** from your Steam library.
3. When the loading picture fades, sit or stand as you will play.

## Controls

| Steam Frame | In the game |
|---|---|
| Right trigger | Fire |
| Left stick | Move. Click to crouch |
| Right stick: flick up, flick down, hold down | Sprint, bullet time, third person |
| **A**, **B**, **X**, **Y** | Jump, melee, action and reload, switch weapon |
| Right grip at shoulder or hip | Draw a weapon |
| Left grip, swing and let go | Throw a grenade |
| Both grips, held | Recentre |

The added features and left-handed play are in **Settings > CONTROLS > VR SETUP**. [All controls](docs/CONTROLS.md).

## Update

Download and check the new release, then in the `OpenFrame` folder:

```sh
git pull
steam-frame/install.sh ~/Downloads/openframe-<version>-linux-arm64.tar.gz
```

## Uninstall

Add `--purge` to also delete your maps, saves and settings.

```sh
~/Games/OpenFrame/app/tools/uninstall.sh
```

## Troubleshooting

- **Steam's debugging port is not reachable:** start Steam, try again.
- **Game asks for the disc image:** run `install.sh --maps` with your maps folder.
- **Plays flat in a window, or quits after a minute:** wake the headset and start SteamVR first.
- **HUD corners cut off:** raise **HUD INSET** in VR Setup > HUD... > HUD LAYOUT....

## Reporting a problem

```sh
~/Games/OpenFrame/app/tools/collect-logs.sh
```

It saves `openframe-logs-<date-time>.tar.gz` in your home folder, with personal details masked as far as it can. Check it, then attach it to an [issue](https://github.com/simplysanatori/OpenFrame/issues).

## More

- [User guide](docs/USER_GUIDE.md): settings, files, full troubleshooting
- [AGENTS.md](AGENTS.md): for developers and AI agents
- [Building from source](steam-frame/dev/DEV.md)
- [Changelog](docs/CHANGELOG.md)

## Support

Support OpenH1, OpenCE, the decompilation and moistman42069's mod first; they're linked under Credits.

For more Steam Frame ports, [buy me a coffee](https://ko-fi.com/sana748653).

## Credits

- [simplysanatori](https://github.com/simplysanatori): built OpenFrame
- [OpenH1](https://github.com/OpenH1/OpenH1): the game and its VR mode
- [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) by moistman42069, on astromaddie's [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR): the added VR features
- [OpenCE](https://github.com/OpenCommunityEdition/OpenCE), [punpckhdq/halo](https://github.com/punpckhdq/halo), [bnunu/halo-1](https://github.com/bnunu/halo-1): what OpenH1 builds on
- Bungie: Halo: Combat Evolved

Full lists: [CREDITS.md](CREDITS.md), [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md). OpenH1's README: [README.upstream.md](README.upstream.md).

Halo, Halo: Combat Evolved and their logos are trademarks of Microsoft. This is an unofficial fan project. It is not affiliated with or endorsed by Microsoft, Bungie, 343 Industries, Valve, OpenH1 or OpenCE.

## Licence

AGPL-3.0-or-later, as OpenH1: [LICENSE](LICENSE).

## How this was made

I started this project and used AI (Claude) to speed up development. I write, review and fix code by hand all the way through, keep the AI's work checked and reined in, and aim to keep the port as lightweight as possible. I test it on my own Steam Frame.
