#target illustrator

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

    function flatten(item) {
        if (item.hidden || item.locked)
            return;

        doc.selection = null;
        item.selected = true;

        var actionName = "Flatten Transparency";
        var actionSetName = "Default Actions";

        try {
            app.doScript(actionName, actionSetName);
        } catch (error) {}

        doc.selection = null;
    }

    function recurseItems(items) {
        if (!items || items.length === 0)
            return;

        // Recurse through each item and apply the crop.
        for (var b = items.length - 1; b>=0; b--) {
            var item = items[b];

            if (item.hidden || item.locked)
                continue;

            if (item.typename === "GroupItem" && item.pageItems && item.pageItems.length > 0) { // we have a group
                recurseItems(item.pageItems);
                continue;
            }

            if (item.hasOwnProperty("blendingMode") && item.blendingMode !== BlendModes.NORMAL) {
                var parent = item.parent;
                if (parent.typename === "GroupItem") {
                    unlockAndCleanGroup(parent);
                    flatten(parent);
                    return; // we return here since we've now flattened the parent of the items[], so we can move onwards...
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

    //doc.selection = null;
    app.userInteractionLevel = oldInteractionLevel;

    alert("Transparency flattened!");
}

main();