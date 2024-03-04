#!/usr/bin/python3
import json
import argparse
import sys
from pathlib import Path

# Note: *NIX masterrace only for now
def os_escape(s):
	return Path(s.replace('/', '|'))

class SongMetadata:
	def __init__(self, filename, artist, album, title, number, year):
		self.filename = Path(filename)
		self.artist = artist
		self.album = album
		self.title = title
		self.number = int(number)
		self.year = int(year)
	
	def filename_in_library(self):
		return self.filename
	
	def filename_in_album(self, fallback = False):
		if not fallback:
			return os_escape(f"{self.number:02} - {self.title}{self.filename.suffix}")
		else:
			return os_escape(f"{self.number:02} - {self.title} ({self.filename.stem}){self.filename.suffix}")

	def album_dirname(self):
		return os_escape(f"[{self.year}] {self.album}")

	def artist_dirname(self):
		return os_escape(self.artist)

	def filename_in_playlist(self, number):
		return os_escape(f"{number:03} - {self.title}{self.filename.suffix}")
		
	def artist_album_song_path(self, fallback = False):
		return self.artist_dirname() / self.album_dirname() / self.filename_in_album(fallback)
		
	def artist_song_path(self, number):
		return self.artist_dirname() / self.filename_in_playlist(number)

class Playlist:
	def __init__(self, name, song_lib_filenames):
		self.name = name
		self.song_lib_filenames = song_lib_filenames
		
	def playlist_dirname(self, number = 0):
		if number == 0:
			return os_escape(self.name)
		else:
			return os_escape(f"{self.name} ({number})")
		
	def song_paths(self, metadata, number = 0):
		directory = self.playlist_dirname()
		paths = []
		cnt = 1
		for s in self.songs:
			paths.append(directory / s.filename_in_playlist(cnt))
			cnt += 1
		return paths
		

# Load JSON metadata from provided path
# JSON format example:
#	{
#		"whatever.ogg": {
#			"number": 3,
#			"title": "Peep Show",
#			"artist": "Miranda Sex Garden",
#			"album": "Fairytales of Slavery",
#			"date": 1994
#		},
#		...
#	}
def load_metadata(path):
	metadata = []
	with open(path, "r") as f:
		j = json.load(f)
		for filename, meta in j.items():
			metadata.append(SongMetadata(
				filename = filename,
				artist = meta["artist"],
				album = meta["album"],
				title = meta["title"],
				number = meta["number"],
				year = meta["date"],
			))
	return metadata

def load_playlists_json(path):
	counters = {}
	playlists = []
	with open(path, "r") as f:
		j = json.load(f)
		for p in j:
			num = counters.get(p["name"], 0)
			counters[p["name"]] = num + 1
			name = p["name"]
			if num != 0:
				name += f" ({num})"
			playlists.append(Playlist(name, p["songs"]))
			
	return playlists

def get_lib_song_path(tree_path, lib_path, filename, up):
	if tree_path.is_absolute():
		return lib_path / filename
	else:
		return Path("../" * up) / lib_path / filename

# Builds Artist / Album / Song tree in specified location
def build_album_artist_tree(songs, tree_path, lib_path):
	for song in songs:
		song_path = tree_path / song.artist_album_song_path()
		song_path.parent.mkdir(parents = True, exist_ok = True)
		try:
			song_path.symlink_to(get_lib_song_path(tree_path, lib_path, song.filename_in_library(), 2))
		except FileExistsError:
			pass

# Builds Artist / Song tree in specified location
def build_artist_tree(songs, tree_path, lib_path):
	songs = sorted(songs, key = lambda s: (s.artist, s.album, s.year, s.number))
	
	counters = {}
	for song in songs:
		number = counters.get(song.artist, 1)
		counters[song.artist] = number + 1
		song_path = tree_path / song.artist_song_path(number)
		song_path.parent.mkdir(parents = True, exist_ok = True)
		try:
			song_path.symlink_to(get_lib_song_path(tree_path, lib_path, song.filename_in_library(), 1))
		except FileExistsError:
			pass

def build_playlist_tree(metadata, tree_path, lib_path, playlists):
	metadata_map = {}
	for m in metadata:
		metadata_map[m.filename] = m
	
	for p in playlists:
		cnt = 1
		(tree_path / p.playlist_dirname()).mkdir(parents = True, exist_ok = True)
		for song_filename in p.song_lib_filenames:
			if Path(song_filename) in metadata_map:
				song = metadata_map[Path(song_filename)]
				song_path = tree_path / p.playlist_dirname() / song.filename_in_playlist(cnt)
				try:
					song_path.symlink_to(get_lib_song_path(tree_path, lib_path, song.filename_in_library(), 1))
				except FileExistsError:
					pass
			else:
				song_path = tree_path / p.playlist_dirname() / (f"{cnt} - " + song_filename)
				try:
					song_path.symlink_to(get_lib_song_path(tree_path, lib_path, Path(song_filename), 1))
				except FileExistsError:
					pass
			cnt += 1
	

def main():
	parser = argparse.ArgumentParser(description = "Filesystem utils for hierarchical music libraries")
	parser.add_argument("metadata_path", metavar = "METADATA", type = Path, help = "Music metadata JSON file")
	subparsers = parser.add_subparsers(title = "subcommands", dest = "subcommand")
	
	parser_album_artist_tree = subparsers.add_parser("artist-album-tree", help = "Build album/artist/song fs tree")
	parser_album_artist_tree.add_argument("lib_path", metavar = "LIB", type = Path, help = "Flat library path (absolute or relative to TREE)")
	parser_album_artist_tree.add_argument("tree_path", metavar = "TREE", type = Path, help = "Path to the tree")
	
	parser_artist_tree = subparsers.add_parser("artist-tree", help = "Build artist/song fs tree")
	parser_artist_tree.add_argument("lib_path", metavar = "LIB", type = Path, help = "Flat library path (absolute or relative to TREE)")
	parser_artist_tree.add_argument("tree_path", metavar = "TREE", type = Path, help = "Path to the tree")
	
	parser_playlist_tree = subparsers.add_parser("playlist-tree", help = "Build playlist fs tree")
	parser_playlist_tree.add_argument("lib_path", metavar = "LIB", type = Path, help = "Flat library path (absolute or relative to TREE)")
	parser_playlist_tree.add_argument("tree_path", metavar = "TREE", type = Path, help = "Path to the tree")
	parser_playlist_tree.add_argument("playlist_json", metavar = "PLAYLISTS", type = Path, help = "Playlists JSON file")
	
	args = parser.parse_args()
	songs = load_metadata(args.metadata_path)
	
	if args.subcommand == "artist-album-tree":
		build_album_artist_tree(songs, args.tree_path, args.lib_path)
	elif args.subcommand == "artist-tree":
		build_artist_tree(songs, args.tree_path, args.lib_path)
	elif args.subcommand == "playlist-tree":
		build_playlist_tree(songs, args.tree_path, args.lib_path, load_playlists_json(args.playlist_json))
	else:
		parser.print_help()

if __name__ == '__main__':
	main()


