package cage.viewer.twoview;

import cage.CaGe;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.UItoolbox;

/**
 *
 * @author nvcleemp
 */
public class PostScriptTwoViewDevice implements TwoViewDevice {

    private TwoViewPainter painter;
    private TwoViewModel model;
    private OutputStream savePostScriptStream;
    private Map<String, Integer> pageNumbers = new HashMap<String, Integer>();

    public PostScriptTwoViewDevice(TwoViewModel model) {
        this.model = model;
        painter = new TwoViewPainter(this);
    }

    /*
     * Saves the current graph to the file with the given name. Returns true
     * when succesful and false otherwise.
     */
    public boolean savePostScript(String fileName, String infoString) {
        //first we check whether we have a page number for this file
        //if this is the case we already wrote to this file during this session
        //in that case we will try to append to the file
        Integer pageNumber = pageNumbers.get(fileName);
        boolean append = (pageNumber != null);

        try {
            savePostScriptStream = Systoolbox.createOutputStream(
                    fileName, CaGe.config.getProperty("CaGe.Generators.RunDir"), append);
        } catch (Exception ex) {
            UItoolbox.showTextInfo(append ? "Error opening file" : "Error creating file",
                    Systoolbox.getStackTrace(ex));
            savePostScriptStream = null;
            return false;
        }
        if (savePostScriptStream == null) {
            //we were unable to open an outputstream to the file
            //aborting...
            return false;
        }

        if (!append) {
            try {
                InputStream prolog = new BufferedInputStream(ClassLoader.getSystemResource("cage/viewer/TwoViewProlog.ps").openStream());
                int c;
                while ((c = prolog.read()) >= 0) {
                    savePostScriptStream.write(c);
                }
                prolog.close();
            } catch (IOException ex1) {
                UItoolbox.showTextInfo("Error reading prolog",
                        Systoolbox.getStackTrace(ex1));
                try {
                    savePostScriptStream.close();
                } catch (IOException ex2) {
                } finally {
                    //something went wrong while outputting prolog section to ps
                    //aborting...
                    savePostScriptStream = null; //set field to null
                    return false;
                }
            }
        }

        float factor = 1;
        float edgeWidth = model.getEdgeWidth() * factor * 2;
        float vertexRadius = model.getVertexSize() * factor;

        //create a new painter, set it to A4 and give it the current graph
        painter = new TwoViewPainter(this);
        painter.setPaintArea(
                42.520 + vertexRadius, 553.391 - vertexRadius,
                42.520 + vertexRadius, 658.493 - vertexRadius);
        // A4 page, 1.5 cm margin each side, another 5 cm clear on top
        painter.setGraph(model.getResult().getGraph());

        //move one page ahead
        pageNumber = pageNumber == null ? 1 : pageNumber + 1;

        //write page number and bounding box as comment in PostScript file
        savePS("\n%%Page: " + model.getResult().getGraphNo() + " " + pageNumber.intValue() + "\n");
        FloatingPoint[] box = painter.getBoundingBox();
        savePS("%%BoundingBox: " + (float) (box[0].x - vertexRadius) + " " + (float) (box[0].y - vertexRadius) + " " + (float) (box[1].x + vertexRadius) + " " + (float) (box[1].y + vertexRadius) + "\n");

        savePS("\ngsave\n\n\n\n");
        //if a info string is provided we write it to the PostScript file
        if (infoString != null) {
            String info = Systoolbox.replace(infoString, " - ", " \\320 ");
            savePS("/Helvetica findfont 20 scalefont setfont\n");
            savePS("297.9554 672.6665 (" + info + ") center_text\n");
        }

        //write the current settings for the drawing
        savePS("/edge_width " + edgeWidth + " def\n");
        savePS("/edge_gray " + model.getEdgeBrightness() + " def\n\n");
        savePS("/vertex_radius " + vertexRadius + " def\n");
        savePS("/vertex_linewidth " + (vertexRadius / 6) + " def\n");
        savePS("/vertex_color_1 { " + CaGe.config.getProperty("TwoView.VertexColor1") + " } bind def\n");
        savePS("/vertex_color_2 { " + CaGe.config.getProperty("TwoView.VertexColor2") + " } bind def\n");
        savePS("/vertex_number_color { " + CaGe.config.getProperty("TwoView.VertexNumberColor") + " } bind def\n\n");

        //output the graph
        painter.paintGraph();

        //finalize this page
        savePS("\n\n\ngrestore\n\nshowpage\n");

        //close file and clean up
        try {
            savePostScriptStream.close();
            pageNumbers.put(fileName, pageNumber);
        } catch (IOException ex) {
            UItoolbox.showTextInfo("Error closing file",
                    Systoolbox.getStackTrace(ex));
        }
        savePostScriptStream = null; //set field to null
        model.getResult().incrementSaved2DPS();

        return true;
    }

    public void finishPostScriptFiles() {
        for (String fileName : pageNumbers.keySet()) {
            try {
                savePostScriptStream = Systoolbox.createOutputStream(
                        fileName, CaGe.config.getProperty("CaGe.Generators.RunDir"), true);
                savePS("\n\n%%Pages: " + pageNumbers.get(fileName) + "\n");
                savePS("%%EOF\n\n");
                savePostScriptStream.close();
            } catch (Exception ex) {
                UItoolbox.showTextInfo("Error finishing file '" + fileName + "'",
                        Systoolbox.getStackTrace(ex));
            }
        }
        pageNumbers.clear();
        savePostScriptStream = null;
    }

    public void beginGraph() {
        int graphSize = painter.getGraphSize();
        for (int i = 1; i <= graphSize; ++i) {
            FloatingPoint p = painter.getCoordinatePoint(i);
            savePS("/v" + i + " { " + p.x + " " + p.y + " } bind def\n");
        }
    }

    public void beginEdges() {
        savePS("\n\nbegin_edges\n\n");
    }

    public void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2, boolean useSpecialColour) {
        savePS("v" + v1 + " " + "v" + v2 + " edge\n");
    }

    public void beginVertices() {
        savePS("\n\nbegin_vertices\n\n");
        if (model.getShowNumbers()) {
            savePS("(" + painter.getGraphSize() + ") set_size\n\n");
        }
    }

    public void paintVertex(double x, double y, int number) {
        savePS("v" + number + " vertex\n");
        if (model.getShowNumbers()) {
            savePS("v" + number + " (" + number + ") vertex_number\n");
        }
    }

    /**
     * If there is a PS stream, this method writes <tt>portion</tt> to
     * that stream and otherwise returns. If this method fails to write to the
     * streams it will try to close the stream and set it to <tt>null</tt>.
     *
     * @param portion The text that should be written to the PS stream.
     */
    private void savePS(String portion) {
        if (savePostScriptStream == null) {
            return;
        }
        try {
            savePostScriptStream.write(portion.getBytes());
        } catch (IOException ex1) {
            UItoolbox.showTextInfo("Error saving PostScript",
                    Systoolbox.getStackTrace(ex1));
            try {
                savePostScriptStream.close();
            } catch (IOException ex2) {
            } finally {
                savePostScriptStream = null;
            }
        }
    }
}
