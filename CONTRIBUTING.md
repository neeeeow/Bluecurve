## Contributing guidelines
Contributors are more than welcome. However to ensure cohesion throughout the project, contributors are asked to follow the contributing guidelines laid out below.

### Guidelines for submitting bug reports
- If you have found a bug, first ensure that you are using the most recent commit of the theme before [opening a new issue](https://github.com/neeeeow/Bluecurve/issues/new) on GitHub. Also ensure that the issue has not already been reported.
- Ensure that you report the issue with adqeuate detail and include any steps you have taken to try to mitigate the problem.
- If possible, include a screenshot. This makes it much easier to pinpoint the cause of the problem.
### Guidelines for contributing code
If you have found a bug and have been able to fix it yourself or wish to implement a new feature, great! Before opening a pull request, please ensure that your patch adheres to the guidelines below:
- Please use tabs, and only tabs, for indendation.
  - If you use spaces for indentation, your pull request will be rejected. **You have been warned!**
- Ensure that your patch keeps the theme's appearance consistent with Fedora Core 3.
  - Installing Fedora Core 3 in a virtual machine is recommended for this. Fedora Core 3 ISO files can be found [here](https://archives.fedoraproject.org/pub/archive/fedora/linux/core/3/i386/iso/).
- Changes to the appearance of widgets should go in the `gtk-style.css` file.
- Changes to MATE and Xfce elements should go in the `mate.css` and `xfce.css` files respectively. 
- Changes to the classic Red Hat 8 & 9 themes should go in the `classic-tweaks.css` file.
- Do not add any additional color definitions to `gtk.css`. Only add additional color definitions to `shade.css` if absolutely necessary.
  - Color definitions that are in `shade.css` generally should be shades of the colors defined in `gtk.css`.
    - The shade values should come from the Bluecurve [GTK 2 engine source code](https://download.fedoraproject.org/pub/fedora/linux/releases/37/Everything/source/tree/Packages/b/bluecurve-gtk-themes-1.0.0-29.fc37.src.rpm).
- Ensure that there are no profanities or otherwise inappropriate language in the comments.
- When submitting your pull request, ensure that you describe the issue that you've resolved or the feature that you've added. Include before/after screenshots if possible.
### Guidelines for package maintainers
- When packaging the theme, ensure that it is installed system-wide.
- The following files should be installed to the corresponding directories (although this may vary depending on your distribution).
  - The GTK 2 engine should be installed to `/usr/lib64/gtk-2.0/2.10.0/engines/` for x86_64 Linux and `/usr/lib/gtk-2.0/2.10.0/engines/` for i686 Linux.
  - The icons should be installed to `/usr/share/icons/`
  - The themes should be installed to `/usr/share/themes/`
  - The Luxi fonts should be installed to `/usr/local/share/fonts/`
  - The Plymouth themes should be installed to `/usr/share/plymouth/themes/`
- Ensure that you package the [releases](https://github.com/neeeeow/Bluecurve/releases) and not the latest commit. A new release usually comes every few commits.
- The version number of your package should be consistent with the version number of the releases [released on GitHub](https://github.com/neeeeow/Bluecurve/releases).
- If you have packaged the theme a distribution, please fork the repository and modify the README.md file with instructions on how to install the package. Then submit a pull request.
- **Note:** If your package goes unmaintained for a long time, the link to the package will be removed from the README.md file.
