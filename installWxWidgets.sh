#!/bin/bash
releaseFile=/etc/os-release

if [ $(id -u) != 0 ]
then
  echo "ERROR: This script must be run as root"
  exit 2
fi

if grep -qi "arch" $releaseFile
then
  echo "System is Arch based and uses pacman!"
  sudo pacman -Qi wxwidgets-common wxwidgets-gtk3 wxwidgets-qt5 &>/dev/null || sudo pacman -Sy --noconfirm wxwidgets-common
  sudo pacman -Qi wxwidgets-gtk3 wxwidgets-qt5 &>/dev/null || sudo pacman -Sy --noconfirm wxwidgets-gtk3
  sudo pacman -Qi wxwidgets-qt5 &>/dev/null || sudo pacman -Sy --noconfirm wxwidgets-qt5
fi

if grep -qi "ubuntu\|debian" $releaseFile
then
  echo "System is Ubuntu/Debian based and uses APT!"
  sudo apt update
  sudo apt install libwxgtk3.0-gtk3-0v5 libwxgtk3.0-gtk3-dev -y
fi 

if grep -qi "fedora" $releaseFile
then
  echo "System is RPM based and uses dnf!"
  sudo dnf update
  sudo dnf install wxGTK3 wxGTK3-devel -y
fi
