import os
import sys
from pathlib import Path
from lxml import etree

def createSymlinks(XMLpath, rootDir):
    parser = etree.XMLParser(remove_blank_text=False)
    try:
        tree = etree.parse(XMLpath, parser)
    except:
        print("Error parsing file")
        return

    root = tree.getroot()
    typedir = root.get("typedir")

    sizedirs = [source.get("sizedir") for source in root.findall("source")]

    for alias in root.findall("alias"):
        name = alias.get("name")
        target = alias.get("target")
        aliasDir = alias.get("typedir")

        if not aliasDir:
            aliasDir = typedir

        for sizedir in sizedirs:
            symlinkDir = os.path.join(rootDir, sizedir, aliasDir) # absolute path of where the symlink will be made
            if not os.path.isdir(symlinkDir):
                continue # if target dir doesn't exist, ignore it

            symlinkPath = os.path.join(symlinkDir, "%s.svg" % (name))
            if os.path.lexists(symlinkPath):
                os.remove(symlinkPath)
            
            targetPath = os.path.join(rootDir, sizedir, typedir, "%s.svg" % (target))
            relPath = os.path.relpath(targetPath, symlinkDir) # the actual path used in the symlink

            try:
                os.symlink(relPath, symlinkPath)
            except:
                print("Error making path!")
                continue

            print("Successfuly created symlink at: %s" % (symlinkPath))


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("usage python symlink.py iconthemeDir rootDir")
        sys.exit(1)

    iconthemeDir = Path(sys.argv[1])
    rootDir = sys.argv[2]

    if iconthemeDir.is_dir():
        XMLs = iconthemeDir.rglob("*.icontheme")
        for xml in XMLs:
            createSymlinks(xml, rootDir)
    else:
        print("Error! invalid icontheme path")
    

    
