import sys
from pathlib import Path
from lxml import etree

def fixGradients(path):
    parser = etree.XMLParser(remove_blank_text=False)
    try:
        tree = etree.parse(path, parser)
    except Exception as e:
        print("Error parsing file")
        return

    root = tree.getroot()

    # namespaces used by illustrators export
    namespaces = {
        "svg": "http://www.w3.org/2000/svg",
        "xlink": "http://www.w3.org/1999/xlink",
    }

    # search for all gradients in the SVG
    gradients = root.xpath(
        "//svg:linearGradient | //svg:radialGradient", namespaces=namespaces
    )

    # dictrionary 
    gradientDict = {g.get("id"): g for g in gradients if g.get("id")}

    needsOverwriting = False

    for grad in gradients:
        href = grad.get("href") or grad.get(
            "{http://www.w3.org/1999/xlink}href"
        )

        if href: # If there is an href, we need to find the parent and copy over the relevant attributes,
            # since QtSVG really hates hrefs
            parentId = href.lstrip("#")
            parentGrad = gradientDict.get(parentId)

            for attr, value in parentGrad.attrib.items():
                if (attr not in grad.attrib and not attr.endswith("id") and not attr.endswith("href")):
                    grad.set(attr, value) # Copy over any values from the parent not present in the child

            # Copy over gradient stops
            stops = parentGrad.xpath("./svg:stop", namespaces=namespaces)
            for stop in stops:
                grad.append(etree.fromstring(etree.tostring(stop)))

            # delete href references
            if "href" in grad.attrib:
                del grad.attrib["href"]
            if "{http://www.w3.org/1999/xlink}href" in grad.attrib:
                del grad.attrib["{http://www.w3.org/1999/xlink}href"]

            needsOverwriting = True

    if needsOverwriting:
        with open(path, "wb") as f:
            tree.write(f, pretty_print=False, xml_declaration=True, encoding="UTF-8")

        print("Successfully fixed and updated: %s" % (path))

                
if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python fix_gradients.py <path_to_svg_file>")
        sys.exit(1)
    path = Path(sys.argv[1]);

    if path.is_file():
        fixGradients(path)
    elif path.is_dir():
        svgs = path.rglob("*.svg")
        for svg in svgs:
            fixGradients(svg)
    else:
        print("Error! invalid path")
        sys.exit(1)
