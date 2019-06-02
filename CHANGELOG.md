# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- New (actually old, but don't tell anyone ;-)) custom cursors

### Changed
- The audio playback is now done using QtMultimedia instead of libao

### Fixed
- Font size rendering when running inside X11
- Minimum height of the main window is now computed instead of hardcoded -> this
  resolves some alignment issues across different platforms
- The playback controls tool bar can no longer be accidentally removed

## [0.1.0] - 2019-04-14
### Added
- The very basics: playing modules, handling playlists, a volume slider, ...
- Support for media keys in macOS
- An UI that does not look boring in comparison to your other applications.
  (needs more gradients though)

[Unreleased]: https://github.com/nilsding/foxbox/compare/0.1.0...HEAD
[0.1.0]: https://github.com/nilsding/foxbox/releases/tag/0.1.0
