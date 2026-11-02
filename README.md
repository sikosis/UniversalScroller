# UniversalScroller

UniversalScroller provides enhanced mouse input for Haiku, including scrolling
by moving the mouse, inter-click actions, and configurable button commands.

This directory contains the maintained C++17 rewrite. The historical source is
kept in `UniversalScroller-Original` as a behavioural and licensing reference;
new code should not depend on it at build time.

## Current status

The command parser, configuration storage, Haiku input filter, and preferences
application have been migrated to C++17. Existing version-1 settings files
remain compatible.

## Development

Run the portable tests on any system with a C++17 compiler:

```sh
make test
```

Build the input-server filter and preferences application with:

```sh
make
```

The filter supports mouse-movement scrolling, shifted horizontal wheel input,
normal and fast wheel factors, remapped clicks, key commands, clipboard
commands, and executable commands. Hold the Option key to bypass augmentation.

## Original project

The original project was written by Christian Aistleitner and contributors.
Its source and GPLv2 licence are retained in `UniversalScroller-Original`.

