package cage.decoration.io;

import cage.decoration.Decoration;
import cage.decoration.DecorationVertex;
import cage.decoration.FacetType;
import cage.decoration.VertexPosition;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.stream.Collectors;

/**
 * Reader for the chamber file format of Gunnar Brinkmann.
 * 
 * @author nvcleemp
 */
public class ChamberFileReader implements DecorationReader{
    
    private final Scanner scanner;

    public ChamberFileReader(InputStream is) {
        scanner = new Scanner(is);
    }
    
    private int[][] readPlanarCode(){
        int order = scanner.nextInt();
        int[][] type1Graph = new int[order][];
        int[] temp = new int[order];
        for (int v = 0; v < type1Graph.length; v++) {
            int i = 0;
            temp[i] = scanner.nextInt() - 1;
            while(temp[i]!=-1){
                i++;
                temp[i] = scanner.nextInt() - 1;
            }
            type1Graph[v] = new int[i];
            System.arraycopy(temp, 0, type1Graph[v], 0, i);
        }
        return type1Graph;
    }
    
    private Decoration readDecoration(){
        if(!scanner.hasNext()){
            return null;
        }
        
        //read planar graph
        int[][] type1Graph = readPlanarCode();
        
        FacetType[] types = new FacetType[type1Graph.length];
        VertexPosition[] positions = new VertexPosition[type1Graph.length];
        
        //first make everything a vertex and internal
        for (int i = 0; i < types.length; i++) {
            types[i] = FacetType.VERTEX;
            positions[i] = VertexPosition.INTERNAL;
        }
        
        //read border
        List<Integer> border = new ArrayList<>();
        while(scanner.hasNextInt()){
            int vertex = scanner.nextInt() - 1;
            String type = scanner.next();
            String position = scanner.next();
            
            if(vertex==-1){
                //end of graph
                break;
            }
            
            if("E".equals(type)){
                types[vertex] = FacetType.EDGE;
            } else if("F".equals(type)){
                types[vertex] = FacetType.FACE;
            } // V is already set
            
            if("I".equals(position)){
                positions[vertex] = null; //we'll handle this later
            } else if("0".equals(position)){
                positions[vertex] = VertexPosition.CORNER_VERTEX;
            } else if("1".equals(position)){
                positions[vertex] = VertexPosition.CORNER_EDGE;
            } else { //2
                positions[vertex] = VertexPosition.CORNER_FACE;
            }
            
            border.add(vertex);
        }
        
        //set remaining positions for border
        int pos = 0;
        while(positions[border.get(pos)]!=VertexPosition.CORNER_VERTEX) pos++;
        
        pos = (pos+1) % border.size();
        
        while(positions[border.get(pos)]!=VertexPosition.CORNER_EDGE) {
            positions[border.get(pos)] = VertexPosition.EDGE_FACE;
            pos = (pos+1) % border.size();
        }
        
        pos = (pos+1) % border.size();
        
        while(positions[border.get(pos)]!=VertexPosition.CORNER_FACE) {
            positions[border.get(pos)] = VertexPosition.EDGE_VERTEX;
            pos = (pos+1) % border.size();
        }
        
        pos = (pos+1) % border.size();
        
        while(positions[border.get(pos)]!=VertexPosition.CORNER_VERTEX) {
            positions[border.get(pos)] = VertexPosition.EDGE_EDGE;
            pos = (pos+1) % border.size();
        }
        
        List<DecorationVertex> vertices = new ArrayList<>();
        for (int i = 0; i < type1Graph.length; i++) {
            vertices.add(new DecorationVertex(i, positions[i], types[i]));
        }
        
        Map<DecorationVertex, List<DecorationVertex>> neighbours = new HashMap<>();
        for (int i = 0; i < type1Graph.length; i++) {
            neighbours.put(vertices.get(i), Arrays.stream(type1Graph[i]).mapToObj(vertices::get).collect(Collectors.toList()));
        }
        
        return new Decoration(vertices, border.stream().map(vertices::get).collect(Collectors.toList()), neighbours);
    }

    @Override
    public Decoration getNext() {
        return readDecoration();
    }
    
}
