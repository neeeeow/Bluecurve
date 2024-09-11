#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Change the current working directory to the script's directory
cd "$SCRIPT_DIR" || { echo "Failed to change directory to script's location. Exiting."; exit 1; }

read -p "Install Bluecurve? (y/N): " user_input

# Default to 'N' if the input is empty
user_input=${user_input:-N}

if ! [[ "$user_input" =~ ^[Yy]$ ]]; then
	echo "Exiting."
    exit 1
fi

ARCH=$(uname -m)

# Copy the GTK 2 engine to the appropriate directory
if [ "$ARCH" = "x86_64" ]; then
    echo "Installing GTK 2 engine..."
	mkdir -p ~/.gtk-2.0/engines
	cp engine/x86_64/libbluecurve.so ~/.gtk-2.0/engines
elif [ "$ARCH" = "i686" ] || [ "$ARCH" = "i386" ]; then
    echo "Installing GTK 2 engine..."
	mkdir -p ~/.gtk-2.0/engines
	cp engine/i686/libbluecurve.so ~/.gtk-2.0/engines
else
	# If user is running something other than x86_64 or i686, they have to manually compile the theme
    echo "Unsupported architecture: $ARCH . You will have to manually compile the GTK 2 engine and copy it to ~/.gtk-2.0/engines"
	echo "See https://github.com/neeeeow/Bluecurve for more details"
    exit 1
fi

# Copy icon set
echo "Installing icon set..."
mkdir -p ~/.icons
cp -r icons/* ~/.icons

# Copy themes
echo "Installing themes..."
mkdir -p ~/.themes
cp -r themes/* ~/.themes

read -p "Install the Luxi font family? (only do this if your distribution does not include them) (y/N): " user_input

# Default to 'N' if the input is empty
user_input=${user_input:-N}

if [[ "$user_input" =~ ^[Yy]$ ]]; then
    echo "Installing the Luxi font family..."
    mkdir -p ~/.local/share/fonts
	cp fonts/*.ttf ~/.local/share/fonts
fi

read -p "Install the Red Hat Graphical Boot plymouth port? (root required) (y/N): " user_input

# Default to 'N' if the input is empty
user_input=${user_input:-N}

if [[ "$user_input" =~ ^[Yy]$ ]]; then
    echo "Installing plymouth themes..."
    sudo cp -r plymouth/* /usr/share/plymouth/themes
	echo "To enable the plymouth themes, use the command plymouth-set-default-theme."
	echo "For more information, see the README file at https://github.com/neeeeow/Bluecurve"
fi

echo " "
echo "Bluecurve has been installed on your system!"

