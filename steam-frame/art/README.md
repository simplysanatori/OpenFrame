# Steam library art

No images are committed here (they are Halo artwork, see below); `SOURCES.md` lists the SteamGridDB assets the owner's library entry uses. `install.sh` copies these files, when they exist, into Steam's
grid folder under the shortcut's app id (`steam-frame/steam-cef.py` and `install.sh` do the naming):

| File here | Steam's name | Size |
|---|---|---|
| `grid.png` | `<id>.png` | wide capsule, 920 x 430 |
| `grid_p.png` | `<id>p.png` | portrait capsule, 600 x 900 |
| `hero.png` | `<id>_hero.png` | hero, 3840 x 1240 |
| `logo.png` | `<id>_logo.png` | logo, transparent PNG |
| `icon.ico` | `<id>_icon.ico` | library icon |

Without images the library entry has Steam's default look, and `install.sh` says so. `release.sh` packs
whatever images are in this folder, so keep the Halo images out of it when building a release.

Use art you made or may use. Do not use Halo artwork, Bungie's or Microsoft's logos or screenshots of
the game as the store art of a release without checking the terms. OpenH1's own library artwork
(`port/assets/steam/openh1-vr/`, by Erin Cena, see `AUTHORS.md`) is OpenH1's and carries OpenH1's name, so
it is not reused here.
