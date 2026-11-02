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

The runnable outputs are placed in `build`:

- `build/UniversalScrollerPreferences` — preferences application
- `build/UniversalScroller` — input-server filter

Launch Preferences directly after building with:

```sh
make run
```

Install the resulting filter and preferences application for the current user:

```sh
make install
```

This copies them under `/boot/home/config/non-packaged`, then restarts the input
server. Launch the preferences application from
`/boot/home/config/non-packaged/apps/UniversalScroller`.

The existing settings file is retained when removing the application:

```sh
make uninstall
```

The filter supports mouse-movement scrolling, shifted horizontal wheel input,
normal and fast wheel factors, remapped clicks, key commands, clipboard
commands, and executable commands. Hold the Option key to bypass augmentation.

### Command values

- `PRIMARY_MOUSE_BUTTON`, `SECONDARY_MOUSE_BUTTON`, or
  `TERTIARY_MOUSE_BUTTON` remaps a click. Add `_DOUBLE` for a double click or a
  positive numeric suffix for repeated clicks.
- `SYSMSG_CUT`, `SYSMSG_COPY`, and `SYSMSG_PASTE` send clipboard commands.
- Key commands use
  `KEY_[SHIFT_][OPTION_][CONTROL_]key_raw-character_byte-count_[bytes_]text`.
- Other non-empty values are launched as executable commands.

Malformed values using a reserved `KEY_`, `SYSMSG_`, or mouse-button prefix are
rejected by Preferences and are never executed.

## Original project

The original project was written by Christian Aistleitner and contributors.
Its source and GPLv2 licence are retained in `UniversalScroller-Original`.

