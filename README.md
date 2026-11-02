# UniversalScroller

UniversalScroller provides enhanced mouse input for Haiku, including scrolling
by moving the mouse, inter-click actions, and configurable button commands.

This directory contains the maintained C++17 rewrite. The historical source is
kept in `UniversalScroller-Original` as a behavioural and licensing reference;
new code should not depend on it at build time.

## Current status

The rewrite is in progress. The command parser and configuration storage have
been migrated and include host-runnable unit tests. Existing version-1 settings
files remain compatible. The Haiku input filter and preferences application
still need to be migrated.

## Development

Run the portable tests on any system with a C++17 compiler:

```sh
make test
```

The Haiku application build will be added as the platform components are
migrated.

## Original project

The original project was written by Christian Aistleitner and contributors.
Its source and GPLv2 licence are retained in `UniversalScroller-Original`.

