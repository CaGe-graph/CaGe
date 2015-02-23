package cage.writer;

import cage.CaGeResult;
import cage.EdgeIterator;
import cage.EmbeddableGraph;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Set;

/**
 * A CaGeWriter which outputs the graph as a OFF file.
 * 
 * @author nvcleemp
 */
public class OFFWriter extends CaGeWriter {

    @Override
    public String getFormatName() {
        return "OFF";
    }

    @Override
    public String getFileExtension() {
        return "off";
    }

    @Override
    public void outputResult(CaGeResult result) {        
        Edge[] edges = asPlaneGraph(result.getGraph());
        Edge[] faces = makeDual(edges);
        
        StringBuilder sb = new StringBuilder("OFF\n");
        sb
            .append(edges.length - 1).append(" ")
            .append(faces.length).append(" ")
            .append(edges.length + faces.length - 3).append("\n");
        
        for (int i = 1; i < edges.length; i++) {
            sb
                .append(result.getGraph().get3DCoordinates(i)[0]).append(" ")
                .append(result.getGraph().get3DCoordinates(i)[1]).append(" ")
                .append(result.getGraph().get3DCoordinates(i)[2]).append("\n");
        }
        
        for (Edge facestart : faces) {
            sb.append(facestart.faceSizeRight);
            Edge e, end;
            e = end = facestart;
            do {
                sb.append(" ").append(e.from - 1);
                e = e.inverse.previous;
            } while (e != end);
            sb.append("\n");
        }
        
        out(sb.toString());
    }
    
    private static Edge[] asPlaneGraph(EmbeddableGraph g){
        Edge[] edges = new Edge[g.getSize() + 1];
        Edge[][] inverseHelper = new Edge[g.getSize() + 1][g.getSize() + 1];
        
        for (int i = 1; i <= g.getSize(); ++i) {
            EdgeIterator edgeIterator = g.getEdgeIterator(i);
            Edge e, previousEdge;
            previousEdge = e = edges[i] = new Edge();
            e.from = i;
            try {
                e.to = edgeIterator.nextEdge();
                if(e.from > e.to){
                    Edge inverse = inverseHelper[e.to][e.from];
                    inverse.inverse = e;
                    e.inverse = inverse;
                } else {
                    inverseHelper[e.from][e.to] = e;
                }
                
                while(edgeIterator.hasNext()) {
                    e = new Edge();
                    e.from = i;
                    e.to = edgeIterator.nextEdge();
                    previousEdge.next = e;
                    e.previous = previousEdge;
                    if(e.from > e.to){
                        Edge inverse = inverseHelper[e.to][e.from];
                        inverse.inverse = e;
                        e.inverse = inverse;
                    } else {
                        inverseHelper[e.from][e.to] = e;
                    }
                    previousEdge = e;
                }
            } catch (NoSuchElementException ex) {
            }
            previousEdge.next = edges[i];
            edges[i].previous = previousEdge;
        }
        
        return edges;
    }
    
    private static Edge[] makeDual(Edge[] edges){
        Set<Edge> handled = new HashSet<>();
        List<Edge> faceStart = new ArrayList<>();
        
        Edge e, end, ef, efEnd;
        for (int i = 1; i < edges.length; i++) {
            e = end = edges[i];
            do {
                if(!handled.contains(e)){
                    faceStart.add(e);
                    ef = efEnd = e;
                    int size = 0;
                    do {
                        handled.add(ef);
                        ef = ef.inverse.previous;
                        size++;
                    } while (ef != efEnd);
                    e.faceSizeRight = size;
                }
                e = e.next;
            } while (e != end);
        }
        
        return faceStart.toArray(new Edge[faceStart.size()]);
    }
    
    /** The EmbeddableGraph doesn't expose enough of the graph, so we need to
     *  rebuild it, so we know what the faces are.
     */
    private static class Edge {
        int from;
        int to;
        Edge next;
        Edge previous;
        Edge inverse;
        
        int faceSizeRight; //only set if edge is start edge for a face
    }
    
}
