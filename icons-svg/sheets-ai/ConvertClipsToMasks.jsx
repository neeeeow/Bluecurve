#target illustrator

function main() {
    if (app.documents.length === 0) {
        alert("No valid document");
        return;
    }

    var doc = app.activeDocument;

    // disable popups to not get in the way while converting
    var oldInteractionLevel = app.userInteractionLevel;
    app.userInteractionLevel = UserInteractionLevel.DONTDISPLAYALERTS;

    doc.selection = null; // clear selection

    function applyMask(clipGroup) {
        // First clean up the group and remove any hidden items
        function unlockAndCleanGroup(group) {
            for (var i = group.pageItems.length - 1; i >= 0; i--) {
                var child = group.pageItems[i];
                if (child.locked) {
                    child.locked = false;
                }
                if (child.hidden) {
                    child.remove(); // Safest for clean SVG output
                } else if (child.typename === "GroupItem") {
                    unlockAndCleanGroup(child);
                }
            }
        }
        unlockAndCleanGroup(clipGroup);
        if (clipGroup.pageItems.length === 0)
            return;
        clipGroup.clipping = false;

        // Create a dummy path as a sibling to our clip group, to avoid opacity mask bugs
        var dummy = clipGroup.parent.pathItems.add();
        dummy.filled = false;
        dummy.stroked = false;

        // Define our white fill color
        var whiteColor = new RGBColor();
        whiteColor.red = 255;
        whiteColor.green = 255;
        whiteColor.blue = 255;

        var clipPath = clipGroup.pageItems[0]; // It's pretty much always the first one, it's safe to assume this.
        clipPath.clipping = false;

        if (clipPath.filled === true || clipPath.typename === "CompoundPathItem") {
            var origClipPath = clipPath.duplicate();
            origClipPath.stroked = false;
            origClipPath.move(clipGroup.pageItems[clipGroup.pageItems.length - 1], ElementPlacement.PLACEAFTER);
            if (origClipPath.typename === "CompoundPathItem") {
                for (var j = 0; j < origClipPath.pathItems.length; j++) {
                    var subPath = origClipPath.pathItems[j];
                    subPath.clipping = false;
                    subPath.stroked = false;
                }
            }
        }

        if ((clipPath.stroked === true && clipPath.strokeWidth > 0) || clipPath.typename === "CompoundPathItem") {
            var origClipStroke = clipPath.duplicate();
            origClipStroke.filled = false;
            origClipStroke.move(clipGroup, ElementPlacement.PLACEATBEGINNING);
            if (origClipStroke.typename === "CompoundPathItem") {
                for (var k = 0; k < origClipStroke.pathItems.length; k++) {
                    var subPath = origClipStroke.pathItems[k];
                    subPath.clipping = false;
                    subPath.filled = false;
                }
            }
        }
        
        clipPath.filled = true;
        clipPath.fillColor = whiteColor;
        if (clipPath.stroked === true && clipPath.strokeWidth > 0)
            clipPath.strokeColor = whiteColor;

        if (clipPath.typename === "CompoundPathItem") {
            for (var a = 0; a < clipPath.pathItems.length; a++) {
                var subPath = clipPath.pathItems[a];
                subPath.clipping = false;
                subPath.filled = true;
                subPath.fillColor = whiteColor;
            }
        }

        // Move the path *above* the group, so we can make the opacity mask
        clipPath.move(clipGroup, ElementPlacement.PLACEBEFORE);

        doc.selection = null;
        clipPath.selected = true;
        clipGroup.selected = true;

        app.redraw();
        
        // Create the opacity mask via a custom action
        var actionName = "Create opacity mask";
        var actionSetName = "Default Actions";

        try {
            app.doScript(actionName, actionSetName);
        } catch (error) {}

        doc.selection = null;

        if (dummy)
            dummy.remove();
    }

    function recurseItems(items) {
        if (!items || items.length === 0)
            return;

        // Recurse through each item and apply the crop.
        for (var b = items.length - 1; b>=0; b--) {
            var item = items[b];

            if (item.hidden || item.locked)
                continue;

            if (item.typename === "GroupItem") { // we have a group
                if (item.pageItems && item.pageItems.length > 0)
                    recurseItems(item.pageItems);

                if (item.clipped) { // it's a clip group
                    applyMask(item);
                }
            }
        }
    }

    function traverseLayers(layers) {
        for (var c = layers.length - 1; c >= 0; c--) {
            var layer = layers[c];

            if (!layer.visible || layer.locked)
                continue;

            if (layer.layers && layer.layers.length > 0) {
                traverseLayers(layer.layers);
            }

            if (layer.pageItems && layer.pageItems.length > 0) {
                recurseItems(layer.pageItems);
            }
        }
    }

    // Execute the recursion
    traverseLayers(doc.layers);

    doc.selection = null;
    app.userInteractionLevel = oldInteractionLevel;

    alert("Cropping completed");
}


main()