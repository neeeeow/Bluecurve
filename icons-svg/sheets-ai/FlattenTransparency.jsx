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

    var layers = doc.layers;
    for (var j = layers.length - 1; j >= 0; j--) {
        var layer = layers[j];

        if (!layer.visible || layer.locked)
            continue;

        if (layer.pageItems && layer.pageItems.length > 0) {
            var items = layer.pageItems;
            for (var k = items.length - 1; k>=0; k--) {
                var item = items[k];
                if (item.hidden || item.locked)
                    continue;

                if (item.typename === "GroupItem") { // Only apply flatten transparency to a group
                    unlockAndCleanGroup(item);

                    doc.selection = null;
                    item.selected = true;

                    var actionName = "Flatten Transparency";
                    var actionSetName = "Default Actions";

                    try {
                        app.doScript(actionName, actionSetName);
                    } catch (error) {}

                    doc.selection = null;
                }   
            }
        }
    }

    doc.selection = null;
    app.userInteractionLevel = oldInteractionLevel;

    alert("Transparency flattened!");
}

main();