package cage.decoration;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.stream.Stream;

/**
 * Class to represent a decoration.
 * 
 * @author nvcleemp
 */
public class Decoration {
    
    private final List<DecorationVertex> border = new ArrayList<>();
    private final List<DecorationVertex> vertices = new ArrayList<>();
    private final Map<DecorationVertex, List<DecorationVertex>> neighbours = new HashMap<>();

    public Decoration(List<DecorationVertex> vertices, List<DecorationVertex> border, Map<DecorationVertex, List<DecorationVertex>> neighbours) {
        this.vertices.addAll(vertices);
        this.border.addAll(border);
        for (DecorationVertex v : vertices) {
            this.neighbours.put(v, new ArrayList<>(neighbours.get(v)));
        }
    }
    
    public Iterable<DecorationVertex> getBorder(){
        return () ->  new Iterator<DecorationVertex>() {
                
                private int pos = -1;

                @Override
                public boolean hasNext() {
                    return true;
                }

                @Override
                public DecorationVertex next() {
                    pos = (pos+1)%border.size();
                    return border.get(pos);
                }
            };
    }
    
    public int getVertexCount(){
        return vertices.size();
    }
    
    public Stream<DecorationVertex> getVerticesAsStream(){
        return vertices.stream();
    }
    
    public Stream<DecorationVertex> getNeighboursAsStream(DecorationVertex v){
        return neighbours.get(v).stream();
    }
}
