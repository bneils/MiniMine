# MINIMINE
A hobby project of mine to create a game in C for my TI-84 Plus CE.
My implementation of Minesweeper allows for the following:
- Flagging cells
- Opening 3x3's when flags permit
- Scrolling for large (16x30) games that go off screen (NOTE: A red line is used to denote the border)
- A pause screen to show remaining mines
- A menu to choose between the original three difficulties

Images can be found on [Cemetech](https://www.cemetech.net/downloads/files/2246/x2556).

# Building
Uses the [CE toolchain](https://github.com/CE_programming/toolchain/).
Depends on either the [latest nightly](https://github.com/CE_programming/toolchain/releases/tag/nightly), or any release after Sept. 2021 due to a compiler bug.
If you download the nightly build, make sure to download the correct clibs as well.
