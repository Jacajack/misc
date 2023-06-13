# Maintainer: Oh, I don't think so.
pkgname=winechad
pkgver=1.0
pkgrel=1
epoch=
pkgdesc='A set of utilities for jailing network interfaces'
arch=('any')
url='https://github.com/Jacajack/misc/tree/master/linux/winechad'
license=('MIT')
groups=()
depends=('python3' 'python-colorama' 'winetricks')
optdepends=('jailif: for remote LAN gaming' 'playonlinux4: for prefix management')
provides=()
conflicts=()
replaces=()
options=()
source=('winechad')
cksums=(SKIP)

prepare() {
	:
}

check() {
	:
}

package() {
	mkdir -p "$pkgdir/usr/bin"
	cp winechad "$pkgdir/usr/bin"
}
