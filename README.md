# Bluecurve GTK 3/4
Red Hat Bluecurve theme ported over to GTK 3/4. Designed for the MATE and Xfce desktop environments.

### Screenshot (Red Hat 9 style)
![Red Hat 8/9 style](screenshots/desktop1.png)
### Screenshot (Fedora Core 1-3 style)
![Red Hat 8/9 style](screenshots/desktop2.png)
### Screenshots (Widget preview, click to enlarge)
[<img alt="Default color scheme" src="screenshots/Bluecurve_awf.png?raw=true" width="200" />](screenshots/Bluecurve_awf.png?raw=true)
[<img alt="BerriesAndCream color scheme" src="screenshots/BerriesAndCream_awf.png?raw=true" width="200" />](screenshots/BerriesAndCream_awf.png?raw=true)
[<img alt="GNOME 2 color scheme" src="screenshots/Gnome_awf.png?raw=true" width="200" />](screenshots/Gnome_awf.png?raw=true)
[<img alt="Grape color scheme" src="screenshots/Grape_awf.png?raw=true" width="200" />](screenshots/Grape_awf.png?raw=true)\
[<img alt="Lime color scheme" src="screenshots/Lime_awf.png?raw=true" width="200" />](screenshots/Lime_awf.png?raw=true)
[<img alt="Slate color scheme" src="screenshots/Slate_awf.png?raw=true" width="200" />](screenshots/Slate_awf.png?raw=true)
[<img alt="Strawberry color scheme" src="screenshots/Strawberry_awf.png?raw=true" width="200" />](screenshots/Strawberry_awf.png?raw=true)
[<img alt="Tangerine color scheme" src="screenshots/Tangerine_awf.png?raw=true" width="200" />](screenshots/Tangerine_awf.png?raw=true)

## Contents
- GTK 3/4 theme, forked from [Blueshell](https://github.com/Rakksor/Blueshell) and modified to make the theme better resemble the original GTK 2 theme and updated to support both GTK 3 and 4, as well as including all of the color schemes that were included with the Bluecurve theme in early versions of Fedora.
- GTK 2 engine and theme.
- Original Bluecurve Metacity themes updated to resolve issues arising from using old metacity themes on newer versions of MATE.
- Original Bluecurve XFWM4 theme.
- Bluecurve icon and cursor set.
- Port of Red Hat Graphical Boot splash screen to Plymouth (includes splash screen from Fedora Core 1-3, RHEL 4, and RHEL 5). Forked from [here.](https://www.gnome-look.org/p/1656023)
- Luxi font family (fonts used originally in Red Hat 8-9 and early versions of Fedora and RHEL).
- Wallpapers that shipped with Red Hat 8-9 and early versions of Fedora, some of which were updated to widescreen by myself.

## Installation
### 1. Download the theme

Either [download the latest release](https://github.com/neeeeow/Bluecurve/releases) or clone the git repository:
```bash
git clone https://github.com/neeeeow/Bluecurve.git
cd Bluecurve
```
> [!CAUTION]
> You must either download the latest release or clone the git repository. Simply downloading the repository as a .zip file breaks permissions!

### 2. Install GTK 2 engine (required)
If on x86_64, copy `engine/x86_64/libbluecurve.so` to `/usr/lib64/gtk-2.0/2.10.0/engines`
```bash
./install.sh
```
### Other distributions (manual installation)
#### 1. Download the theme
Either [download the latest release](https://github.com/neeeeow/Bluecurve/releases) or clone the git repository:
```bash
git clone https://github.com/neeeeow/Bluecurve.git
cd Bluecurve
```
#### 2. Compile GTK 2 engine
First ensure that you have the following installed on your system:
- Cmake
- gcc
- GTK 2 development libraries

Then navigate to the GTK 2 engine source code directory:
```bash
cd engine/src
```
Then create the `build` folder
```bash
mkdir build && cd build
```
Next run `cmake`
```bash
cmake ..
```
Finally, compile and install the GTK 2 engine (the engine is installed to `~/.gtk-2.0/engines`) and return to the repository's root directory:
```bash
make && make install && cd ../../../
```

### 3. Install icon and cursor set
Copy the contents of the `icons` folder to `/usr/share/icons`
```bash
cp -r icons/* ~/.icons
```
### 4. Install theme
Copy the contents of the `themes` folder to `/usr/share/themes`

```bash
cp -r themes/* ~/.themes
```

### (Optional) 5. Install Luxi fonts
> [!WARNING]
> Only do this step if your distribution doesn't include the Luxi font family.

First create the directory `~/.local/share/fonts` if it doesn't exist:
```bash
mkdir ~/.local/share/fonts
```
Next, copy the fonts to the directory:
```bash
cp fonts/*.ttf ~/.local/share/fonts
```

#### (Optional) 5. Install Plymouth themes
Included in the `plymouth` folder are Plymouth ports of the RHGB boot screen used in Fedora Core 1-3 and RHEL 4/5. Themes `rhgb-fc1`, `rhgb-rhel4` and `rhgb-centos4` require the font "Luxi Sans" to be installed (if not installed see step 5 above).

Copy the Plymouth themes to `/usr/share/plymouth/themes`
```bash
sudo cp -r plymouth/* /usr/share/plymouth/themes
```

## Hints
### Use Red Hat icon in MATE menu bar
Simply execute the following command:
```bash
gsettings set org.mate.panel.menubar icon-name 'redhat-icon-panel-menu'
```

### Enable Plymouth themes
The following Plymouth themes are included: 

| Folder name  | Theme |
| ------------- | ------------- |
| rhgb-fc1  | Fedora Core 1-3 Plymouth theme |
| rhgb-rhel4  | RHEL 4 Plymouth theme |
| rhgb-centos4  | CentOS 4 Plymouth theme |
| rhgb-rhel5  | RHEL 5 Plymouth theme |

If you installed the Plymouth themes, you need to enable the desired theme using the `plymouth-set-default-theme` command. Using `rhgb-fc1` as an example:
```bash
sudo plymouth-set-default-theme -R rhgb-fc1
```

> [!IMPORTANT]
> If `plymouth-set-default-theme` does not work, you might need to install the appropriate Plymouth script. On Fedora, install the package `plymouth-plugin-script`.

### Compile & install GTK 2 engine system-wide
If you wish to compile & install the GTK 2 engine system-wide, simply enable the `INSTALL_SYSTEM_WIDE` flag when compiling the engine.

**Instructions:**
```bash
cd engine/src
mkdir build && cd build
cmake -DINSTALL_SYSTEM_WIDE=ON ..
make
sudo make install
```

## Contributing
Contributors are more than welcome! If you would like to contribute, first read the [contributing guidelines](https://github.com/neeeeow/Bluecurve/blob/master/CONTRIBUTING.md).
