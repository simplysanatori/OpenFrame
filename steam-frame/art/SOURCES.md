# Library art sources

The OpenFrame Steam library entry uses community artwork from [SteamGridDB](https://www.steamgriddb.com)
for the game "Halo: Combat Evolved" (SteamGridDB game id 38383). The images are NOT committed here (they
are Halo artwork, see `README.md`); this file records exactly which assets were used so the set can be
fetched again. Each is the first asset the API lists for the game (SteamGridDB's own ranking; every asset
has 0 votes in the API), static, SFW, PNG.

| File | Kind | SGDB asset id | Author | Style | Size | URL |
|---|---|---|---|---|---|---|
| `grid_p.png` | portrait capsule | 100011 | UncookedMoa | alternate | 600 x 900 | https://cdn2.steamgriddb.com/grid/09a146c8d1cfdbdb54ceb60ede93cdab.png |
| `grid.png` | wide capsule | 34594 | ender-blade | alternate | 920 x 430 | https://cdn2.steamgriddb.com/grid/33880528245ea2845dc447bd36ebc547.png |
| `hero.png` | hero | 151267 | Gector(lint)Nathan | alternate | 3840 x 1240 | https://cdn2.steamgriddb.com/hero/1a6d2e708873df49ad05540c37e8d36a.png |
| `logo.png` | logo | 893 | Bcoder | official | 2459 x 1060 | https://cdn2.steamgriddb.com/logo/d56b9fc4b0f1be8871f5e1c40c0067e7.png |
| `icon.ico` | icon | 2766 | Onyx8011 | custom | 256 x 256 | https://cdn2.steamgriddb.com/icon/045cf83ab0722e782cf72d14e44adf98.png |

The icon is a PNG on SteamGridDB; it was converted with `ffmpeg -i icon.png -vf scale=256:256 icon.ico`.

To install them on another machine: download the URLs above, name them as in the first column, and put
them in `steam-frame/art/` (or the unpacked release's `art/`) before running `install.sh`, which copies
them into Steam's grid folder;.
