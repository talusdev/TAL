
Debian
====================
This directory contains files used to package tald/tal-qt
for Debian-based Linux systems. If you compile tald/tal-qt yourself, there are some useful files here.

## tal: URI support ##


tal-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install tal-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your talqt binary to `/usr/bin`
and the `../../share/pixmaps/tal128.png` to `/usr/share/pixmaps`

tal-qt.protocol (KDE)

