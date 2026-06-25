#target illustrator

function loadXML() {
    var file = File.openDialog("Select .icontheme file:", "*.icontheme");
    if (file == null) {
        alert("Error: invalid file!");
        return null;
    }

    if (file.open("r")) {
        var content = file.read();
        file.close();

        try {
            var data = new XML(content);
            return data;
        } catch (e) {
            alert("Error parsing file: " + e.message);
            return null;
        }
    }
}

function main() {
    if (app.documents.length === 0) {
        alert("Please open a document first.");
        return;
    }

    var doc = app.activeDocument;

    var userInput = Window.prompt("Enter icon size:", "", "Icon exporter");
    var cellSize;

    if (userInput != null) {
        cellSize = parseInt(userInput, 10);
        if (isNaN(cellSize)) {
            alert("Error: invalid input!");
            return;
        }
    } else {
        alert("Error: invalid input!");
        return;
    }

    var data = loadXML();
    if (data == null) {
        alert("Error: invalid data!");
        return;
    }

    var exportFolder = Folder.selectDialog("Select output folder:");
    if (!exportFolder)
        return; 

    // disable popups to not get in the way while converting
    var oldInteractionLevel = app.userInteractionLevel;
    app.userInteractionLevel = UserInteractionLevel.DONTDISPLAYALERTS;

    var folderSetting = app.preferences.getIntegerPreference('plugin/SmartExportUI/CreateFoldersPreference');
    app.preferences.setIntegerPreference('plugin/SmartExportUI/CreateFoldersPreference', 0);

    // get document dimensions
    var abBounds = doc.artboards[doc.artboards.getActiveArtboardIndex()].artboardRect;
    var abLeft = abBounds[0];
    var abTop = abBounds[1];

    var rows = data.layout.row;

    for (var r = 0; r < rows.length(); r++) {
        var row = rows[r];
        var icons = row.icon;

        for (var c = 0; c < icons.length(); c++) {
            var icon = icons[c];
            var iconName = icon.@name.toString();

            // Cell dimensions
            var cellLeft = abLeft + (c * cellSize);
            var cellRight = cellLeft + cellSize;
            var cellTop = abTop - (r * cellSize);
            var cellBottom = cellTop - cellSize;

            var itemsToKeep = [];
            var itemsToDelete = [];

            // Iterate through each item 
            for (var i = 0; i < doc.pageItems.length; i++) {
                var item = doc.pageItems[i];

                // only care about top level items
                if (item.parent.typename !== "Document" && item.parent.typename !== "Layer")
                    continue;

                if (item.hidden) { // Auto delete any hidden items
                    item.hidden = false; // show the item before deleting it
                    itemsToDelete.push(item);
                    continue;
                }

                if (item.locked)
                    item.locked = false;

                // Get geometric bounding box
                var bounds = item.geometricBounds;
                var itemLeft = bounds[0];
                var itemTop = bounds[1];
                var itemRight = bounds[2];
                var itemBottom = bounds[3];

                // Calculate geometric centre
                var centerX = itemLeft + ((itemRight - itemLeft) / 2);
                var centerY = itemTop - ((itemTop - itemBottom) / 2);

                // If the geometric centre lies within the 48px area, we keep it, if not delete it.
                if (centerX >= cellLeft && centerX < cellRight && centerY <= cellTop && centerY > cellBottom) {
                    itemsToKeep.push(item);
                } else {
                    itemsToDelete.push(item);
                }
            }

            if (itemsToKeep.length === 0)
                continue;

            for (var d = itemsToDelete.length - 1; d >= 0; d--) {
                itemsToDelete[d].remove();
            }

            // Temporarily resize the artboard to exactly fit this 48x48px cell for clean SVG cropping
            var originalArtboardRect = doc.artboards[doc.artboards.getActiveArtboardIndex()].artboardRect;
            doc.artboards[doc.artboards.getActiveArtboardIndex()].artboardRect = [cellLeft, cellTop, cellRight, cellBottom];

            // Rename the artboard for the file output
            doc.artboards[doc.artboards.getActiveArtboardIndex()].name = iconName;

            // Set options and export
            var opts = new ExportForScreensOptionsWebOptimizedSVG();
            opts.svgResponsive       = false;
            opts.fontType            = SVGFontType.OUTLINEFONT;
            opts.coordinatePrecision = 7;
            opts.cssProperties       = SVGCSSPropertyLocation.PRESENTATIONATTRIBUTES;
            doc.exportForScreens(
                new File(exportFolder),
                ExportForScreensType.SE_SVG,
                opts
            );

            app.undo();

        }

    }

    // Restore settings
    app.preferences.setIntegerPreference('plugin/SmartExportUI/CreateFoldersPreference', folderSetting);
    app.userInteractionLevel = oldInteractionLevel;

    alert("SVGs exported!");

}

main();