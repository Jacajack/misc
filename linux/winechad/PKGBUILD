# Maintainer: Oh, I don't think so.
pkgname=winechad
pkgver=2.1
pkgrel=1
epoch=
pkgdesc='A utility for WINE prefix management'
arch=('any')
url='https://github.com/Jacajack/misc/tree/master/linux/winechad'
license=('MIT')
groups=()
depends=(
	python3
	python-rich
	firejail

	wine
	wine-gecko
	winetricks

	desktop-file-utils
	fontconfig
	freetype2
	gcc-libs
	gettext
	lib32-fontconfig
	lib32-freetype2
	lib32-gcc-libs
	lib32-gettext
	lib32-libpcap
	lib32-libunwind
	lib32-libxcursor
	lib32-libxi
	lib32-libxkbcommon
	lib32-libxrandr
	libpcap
	libunwind
	libxcursor
	libxi
	libxkbcommon
	libxrandr
	lib32-openal

	gnutls
	lib32-gnutls

	gst-plugins-bad
	gst-plugins-base
	gst-plugins-base-libs
	gst-plugins-good
	gst-plugins-ugly
	lib32-gst-plugins-base
	lib32-gst-plugins-base-libs 
	lib32-gst-plugins-good 

	alsa-lib
	alsa-plugins
	lib32-alsa-lib 
	lib32-alsa-plugins
	lib32-libpulse
	lib32-libcanberra-pulse
	lib32-mpg123 # l3codeca.acm.so

	sdl2
	v4l-utils
	lib32-libxcomposite
	lib32-libxinerama
	lib32-sdl2
	lib32-v4l-utils
	libgphoto2 
	libpulse
	libxcomposite
	libxinerama
	unixodbc

	# pcsclite (optional)
	# lib32-opencl-icd-loader (lib32-ocl-icd) (optional)
	# samba (optional)
	# sane
	# lib32-libcups (optional)
	# lib32-pcsclite (optional)
	# opencl-icd-loader
	
)
optdepends=(
	
)
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
