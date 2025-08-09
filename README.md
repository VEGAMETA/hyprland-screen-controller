# Hyprland Screen Contoller

A utility to manipulate screen colors in Hyprland using custom shader.

## Description

This tool applies real-time color transformations via shaders in Hyprland compositor. It serves as a custom solution for color management.

## Installation

```bash
   git clone https://github.com/VEGAMETA/hyprland-screen-controller.git
   cd hyprland-screen-controller
```

```bash
make install
```

## Configuration

Config and shader files are at `~/.config/hypr/`

### Set shader within hypland config file

```json
decoration {
    screen_shader = ~/.config/hypr/screen_controller_shader.frag
    ...
}
```

You can change configuration file at
`~/.config/hypr/screen_controller_shader.conf`

To indorse changes just call

```bash
hssc
```

## Usage

To show avalible arguments call

```bash
hssc help
```

Example:

```bash
hssc --hue 0.007 --gamma 1.15
```
