package org.solarus.editor.entities;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Rectangle;

import org.solarus.editor.*;

/**
 * A map entity fully defined by scripts.
 */
public class CustomEntity extends MapEntity {

    /**
     * Description of the default image representing this kind of entity.
     */
    public static final EntityImageDescription[] generalImageDescriptions = {
        new EntityImageDescription("entity_custom.png", 0, 0, 32, 32)
    };

    /**
     * Resizable image of this entity type.
     */
    private static Image resizableImage;

    /**
     * Unitary size of a custom entity.
     */
    private static final Dimension unitarySize = new Dimension(8, 8);

    /**
     * Creates a new custom entity.
     * @param map The map.
     */
    public CustomEntity(Map map) throws MapException {
        super(map, 16, 16);
    }

    /**
     * Returns whether or not the entity is currently resizable.
     * @return true if the entity is resizable.
     */
    public boolean isResizable() {
        return true;
    }

    /**
     * Returns the minimum size of the entity (for a resizable entity).
     * When the entity is resized, its new size must be a multiple of this minimum size.
     * @return (8, 8)
     */
    public Dimension getUnitarySize() {
        return unitarySize;
    }

    /**
     * Declares all properties specific to the current entity type and sets
     * their initial values.
     */
    public void createProperties() throws MapException {
        createStringProperty("model", true, null);
    }

    /**
     * Checks the specific properties.
     * @throws MapException if a property is not valid
     */
    public void checkProperties() throws MapException {

        String model = getStringProperty("model");
        Resource entityResource = Project.getResource(ResourceType.ENTITY);
        if (model != null
                && !entityResource.exists(model)) {
            throw new MapException("Unknown custom entity model: '" + model + "'");
        }
    }

    /**
     * Draws the entity on the map editor.
     * @param g graphic context
     * @param zoom zoom of the image (for example, 1: unchanged, 2: zoom of 200%)
     * @param showTransparency true to make transparent pixels,
     * false to replace them by a background color
     */
    public void paint(Graphics g, double zoom, boolean showTransparency) {

        if (resizableImage == null) {
            resizableImage = Project.getEditorImageOrEmpty("entity_custom_resizable.png");
        }

        Rectangle positionInMap = getPositionInMap();
        int x = (int) (positionInMap.x * zoom);
        int y = (int) (positionInMap.y * zoom);
        int w = (int) (positionInMap.width * zoom);
        int h = (int) (positionInMap.height * zoom);

        g.setColor(new Color(224, 108, 72));
        g.fillRect(x, y, w, h);

        int dx1 = (int) ((positionInMap.x + positionInMap.width / 2 - 8) * zoom);
        int dy1 = (int) ((positionInMap.y + positionInMap.height / 2 - 8) * zoom);
        int dx2 = (int) (dx1 + 16 * zoom);
        int dy2 = (int) (dy1 + 16 * zoom);

        int sx1 = 0;
        int sx2 = sx1 + 32;

        g.drawImage(resizableImage, dx1, dy1, dx2, dy2, sx1, 0, sx2, 32, null);

        drawEntityOutline(g, zoom, new Color(184, 96, 96));
    }
}
