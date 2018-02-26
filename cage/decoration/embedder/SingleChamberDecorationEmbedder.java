package cage.decoration.embedder;

import cage.decoration.Decoration;
import cage.decoration.DecorationGraph;
import cage.decoration.DecorationVertex;
import static cage.decoration.DoubleVectorUtility.dotProduct;
import static cage.decoration.DoubleVectorUtility.scalarProduct;
import static cage.decoration.DoubleVectorUtility.sum;
import cage.decoration.EmbeddedDecorationGraph;
import cage.decoration.FacetType;
import cage.decoration.Neighbour;
import cage.decoration.VertexPosition;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * An embedder for a decoration applied to a tiling containing a single chamber type.
 * 
 * @author nvcleemp
 */
public class SingleChamberDecorationEmbedder implements DecorationEmbedder{
    
    private final GeometricChamber chamber;
    private Decoration decoration;
    
    private DecorationGraph dg;
    private double[][] coordinates;
    private VertexPosition[] positionsForDg;
    
    private int tutteStepCount = 100;
    private int springStepCount = 100;
    private double dampening = 0.1;

    public SingleChamberDecorationEmbedder(GeometricChamber chamber) {
        this.chamber = chamber;
    }
    
    @Override
    public SingleChamberDecorationEmbedder setDecoration(Decoration decoration){
        this.decoration = decoration;
        return this;
    }
    
    public SingleChamberDecorationEmbedder setTutteStepCount(int tutteStepCount){
        this.tutteStepCount = tutteStepCount;
        return this;
    }

    public SingleChamberDecorationEmbedder setSpringStepCount(int springStepCount) {
        this.springStepCount = springStepCount;
        return this;
    }

    public SingleChamberDecorationEmbedder setDampening(double dampening) {
        this.dampening = dampening;
        return this;
    }
    
    public SingleChamberDecorationEmbedder initialise(){
        double[][] coordinatesAll = new double[decoration.getVertexCount()][2];
        Iterator<DecorationVertex> border = decoration.getBorder().iterator();
        //careful: this is an infinite iterator, so we assume the decoration is well-formed
        DecorationVertex current = border.next();
        while(!VertexPosition.CORNER_VERTEX.equals(current.getPosition())){
            current = border.next();
        }
        System.arraycopy(chamber.getCoordinatesForCorner(FacetType.VERTEX), 0, coordinatesAll[current.getId()], 0, 2);
        List<DecorationVertex> edgeVertices = new ArrayList<>();
        current = border.next();
        while(!VertexPosition.CORNER_EDGE.equals(current.getPosition())){
            edgeVertices.add(current);
            current = border.next();
        }
        System.arraycopy(chamber.getCoordinatesForCorner(FacetType.EDGE), 0, coordinatesAll[current.getId()], 0, 2);
        //set coordinates for vertices between V and E
        for (int i = 0; i < edgeVertices.size(); i++) {
            double factor = (i + 1.0)/(edgeVertices.size()+1);
            double[] coordinatesV = chamber.getCoordinatesForCorner(FacetType.VERTEX);
            double[] coordinatesE = chamber.getCoordinatesForCorner(FacetType.EDGE);
            coordinatesAll[edgeVertices.get(i).getId()][0] = (1-factor)*coordinatesV[0] + factor*coordinatesE[0];
            coordinatesAll[edgeVertices.get(i).getId()][1] = (1-factor)*coordinatesV[1] + factor*coordinatesE[1];
        }
        
        edgeVertices.clear();
        current = border.next();
        while(!VertexPosition.CORNER_FACE.equals(current.getPosition())){
            edgeVertices.add(current);
            current = border.next();
        }
        System.arraycopy(chamber.getCoordinatesForCorner(FacetType.FACE), 0, coordinatesAll[current.getId()], 0, 2);
        //set coordinates for vertices between E and F
        for (int i = 0; i < edgeVertices.size(); i++) {
            double factor = (i + 1.0)/(edgeVertices.size()+1);
            double[] coordinatesE = chamber.getCoordinatesForCorner(FacetType.EDGE);
            double[] coordinatesF = chamber.getCoordinatesForCorner(FacetType.FACE);
            coordinatesAll[edgeVertices.get(i).getId()][0] = (1-factor)*coordinatesE[0] + factor*coordinatesF[0];
            coordinatesAll[edgeVertices.get(i).getId()][1] = (1-factor)*coordinatesE[1] + factor*coordinatesF[1];
        }
        
        edgeVertices.clear();
        current = border.next();
        while(!VertexPosition.CORNER_VERTEX.equals(current.getPosition())){
            edgeVertices.add(current);
            current = border.next();
        }
        //set coordinates for vertices between F and V
        for (int i = 0; i < edgeVertices.size(); i++) {
            double factor = (i + 1.0)/(edgeVertices.size()+1);
            double[] coordinatesF = chamber.getCoordinatesForCorner(FacetType.FACE);
            double[] coordinatesV = chamber.getCoordinatesForCorner(FacetType.VERTEX);
            coordinatesAll[edgeVertices.get(i).getId()][0] = (1-factor)*coordinatesF[0] + factor*coordinatesV[0];
            coordinatesAll[edgeVertices.get(i).getId()][1] = (1-factor)*coordinatesF[1] + factor*coordinatesV[1];
        }

        final double centerX = Stream.of(FacetType.values()).mapToDouble(t -> chamber.getCoordinatesForCorner(t)[0]).average().getAsDouble();
        final double centerY = Stream.of(FacetType.values()).mapToDouble(t -> chamber.getCoordinatesForCorner(t)[1]).average().getAsDouble();
        decoration.getVerticesAsStream().filter(v -> VertexPosition.INTERNAL.equals(v.getPosition())).forEach(v -> {
            coordinatesAll[v.getId()][0] = centerX;
            coordinatesAll[v.getId()][1] = centerY;
        });
        
        double[][] coordinatesNew = new double[decoration.getVertexCount()][2];
        
        for (int i = 0; i < tutteStepCount; i++) {
            decoration.getVerticesAsStream().forEach((v) -> {
                if(VertexPosition.INTERNAL.equals(v.getPosition())){
                    coordinatesNew[v.getId()][0] = decoration
                            .getNeighboursAsStream(v)
                            .mapToDouble(n -> coordinatesAll[n.getId()][0])
                            .average().getAsDouble();
                    coordinatesNew[v.getId()][1] = decoration
                            .getNeighboursAsStream(v)
                            .mapToDouble(n -> coordinatesAll[n.getId()][1])
                            .average().getAsDouble();
                } else {
                    System.arraycopy(coordinatesAll[v.getId()], 0, coordinatesNew[v.getId()], 0, 2);
                }
            });
            for (int j = 0; j < coordinatesAll.length; j++) {
                System.arraycopy(coordinatesNew[j], 0, coordinatesAll[j], 0, 2);
            }
        }
        
        //create decoration graph and store coordinates
        int[] original2new = new int[decoration.getVertexCount()];
        for (int i = 0; i < original2new.length; i++) {
            original2new[i] = -1;
        }
        List<DecorationVertex> realVertices = decoration.getVerticesAsStream()
                .filter(v -> FacetType.VERTEX.equals(v.getType()))
                .collect(Collectors.toList());
        for (int i = 0; i < realVertices.size(); i++) {
            original2new[realVertices.get(i).getId()] = i;
        }
        Neighbour[][] neighbours = new Neighbour[realVertices.size()][];
        for (int i = 0; i < neighbours.length; i++) {
            final int iFinal = i;
            final DecorationVertex source = realVertices.get(i);
            neighbours[i] = decoration.getNeighboursAsStream(source)
                    .filter(v -> !FacetType.FACE.equals(v.getType()))
                    .map(v -> {
                        if(FacetType.VERTEX.equals(v.getType())){
                            return new Neighbour(original2new[v.getId()], null);
                        } else if(v.getPosition().isEdge() && FacetType.EDGE.equals(v.getType()) && (source.getPosition().equals(v.getPosition()) || v.getPosition().isAdjacentCorner(source.getPosition()))) {
                            DecorationVertex target = decoration.getNeighboursAsStream(v).filter(u -> FacetType.VERTEX.equals(u.getType()) && !u.equals(source)).findAny().get();
                            return new Neighbour(original2new[target.getId()], null);
                        } else {
                            return new Neighbour(
                                    iFinal,
                                    v.getPosition().getReflectingEdge(realVertices.get(iFinal).getPosition()));
                        }
                    }).toArray(Neighbour[]::new);
        }
        
        dg = new DecorationGraph(realVertices.size(), neighbours);
        
        positionsForDg = new VertexPosition[realVertices.size()];
        for (int i = 0; i < realVertices.size(); i++) {
            positionsForDg[i] = realVertices.get(i).getPosition();
        }
        
        coordinates = new double[realVertices.size()][2];
        for (int i = 0; i < realVertices.size(); i++) {
            System.arraycopy(coordinatesAll[realVertices.get(i).getId()], 0, coordinates[i], 0, 2);
        }
        
        return this;
    }
    
    public SingleChamberDecorationEmbedder step(){
        step_impl();
        return this;
    }
    
    public SingleChamberDecorationEmbedder steps(int count){
        for (int i = 0; i < count; i++) {
            step_impl();
        }
        return this;
    }
    
    private void step_impl(){
        //compute forces for each vertex
        double[][] forces = new double[dg.getOrder()][2];
        for (int i = 0; i < dg.getOrder(); i++) {
            forces[i][0] = forces[i][1] = 0.0;
            if(!positionsForDg[i].isCorner()){
                for (Neighbour n : dg.getNeighboursFor(i)) {
                    if(n.getType()==null){
                        double factor = 
                                (coordinates[n.getVertex()][0] - coordinates[i][0])*(coordinates[n.getVertex()][0] - coordinates[i][0]) +
                                (coordinates[n.getVertex()][1] - coordinates[i][1])*(coordinates[n.getVertex()][1] - coordinates[i][1]);
                        forces[i][0] += factor*(coordinates[n.getVertex()][0] - coordinates[i][0]);
                        forces[i][1] += factor*(coordinates[n.getVertex()][1] - coordinates[i][1]);
                    } else {
                        double[] coords = chamber.getReflectionOppositeOf(n.getType()).apply(coordinates[n.getVertex()]);
                        double factor = 
                                (coords[0] - coordinates[i][0])*(coords[0] - coordinates[i][0]) +
                                (coords[1] - coordinates[i][1])*(coords[1] - coordinates[i][1]);
                        forces[i][0] += factor*(coords[0] - coordinates[i][0]);
                        forces[i][1] += factor*(coords[1] - coordinates[i][1]);
                    }
                }
                if(!VertexPosition.INTERNAL.equals(positionsForDg[i])){
                    //vertex lies on an edge, so we replace the force by its component along the edge
                    double[] edgeVector = chamber.getVectorForEdge(positionsForDg[i].getReflectingEdge(null));
                    double[] edgeComponent = scalarProduct(edgeVector, dotProduct(edgeVector, forces[i])/dotProduct(edgeVector, edgeVector));
                    forces[i][0] = edgeComponent[0];
                    forces[i][1] = edgeComponent[1];
                }
                //apply dampening to force
                forces[i][0] *= dampening;
                forces[i][1] *= dampening;
            }
        }
        
        //apply constraints to forces
        for (int i = 0; i < dg.getOrder(); i++) {
            int count = 0;
            double[] bc = chamber.getBarycentricCoordinatesFor(sum(coordinates[i], forces[i]));
            //check each component of bc is between 0 and 1.
            while(count < 10 && Stream.of(bc[0], bc[1], bc[2]).anyMatch(v -> v < 0.0 || v > 1.0)){
                count++;
                forces[i][0] /= 2;
                forces[i][1] /= 2;
                bc = chamber.getBarycentricCoordinatesFor(sum(coordinates[i], forces[i]));
            }
            if(count == 10){
                forces[i][0] = forces[i][1] = 0.0;
            }
        }
        
        //move vertices
        for (int i = 0; i < dg.getOrder(); i++) {
            coordinates[i][0] += forces[i][0];
            coordinates[i][1] += forces[i][1];
        }
    }
    
    @Override
    public SingleChamberDecorationEmbedder run(){
        initialise();
        steps(springStepCount);
        return this;
    }

    public SingleChamberDecorationEmbedder printCoordinates(){
        for (double[] coords : coordinates) {
            for (double coord : coords) {
                System.out.println(coord + " ");
            }
            System.out.println();
        }
        return this;
    }
    
    @Override
    public EmbeddedDecorationGraph export(){
        //translate coordinates to barycentric coordinates
        double[][] barycentricCoordinates = new double[dg.getOrder()][3];
        for (int i = 0; i < barycentricCoordinates.length; i++) {
            double[] bc = chamber.getBarycentricCoordinatesFor(coordinates[i]);
            System.arraycopy(bc, 0, barycentricCoordinates[i], 0, 3);
        }
        
        EmbeddedDecorationGraph edg = new EmbeddedDecorationGraph(dg, barycentricCoordinates);
        return edg;
    }
}
