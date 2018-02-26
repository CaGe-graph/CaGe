package cage.decoration;

/**
 * Enum to represent the different positions of a vertex in a decoration.
 * @author nvcleemp
 */
public enum VertexPosition {
    INTERNAL,
    EDGE_VERTEX(FacetType.VERTEX){

        @Override
        public boolean isEdge() {
            return true;
        }

        @Override
        public boolean isAdjacentCorner(VertexPosition p) {
            return p!=null && p.isCorner() && !p.equals(CORNER_VERTEX);
        }
        
    },
    EDGE_EDGE(FacetType.EDGE){

        @Override
        public boolean isEdge() {
            return true;
        }

        @Override
        public boolean isAdjacentCorner(VertexPosition p) {
            return p!=null && p.isCorner() && !p.equals(CORNER_EDGE);
        }
        
    },
    EDGE_FACE(FacetType.FACE){

        @Override
        public boolean isEdge() {
            return true;
        }

        @Override
        public boolean isAdjacentCorner(VertexPosition p) {
            return p!=null && p.isCorner() && !p.equals(CORNER_FACE);
        }
        
    },
    CORNER_VERTEX{

        @Override
        public boolean isCorner() {
            return true;
        }
        
    },
    CORNER_EDGE{

        @Override
        public FacetType getReflectingEdge(VertexPosition source) {
            switch(source){
                case CORNER_FACE:
                case EDGE_VERTEX:
                    return FacetType.FACE;
                case CORNER_VERTEX:
                case EDGE_FACE:
                    return FacetType.VERTEX;
                default:
                    throw new UnsupportedOperationException();
            }
        }

        @Override
        public boolean isCorner() {
            return true;
        }
        
    },
    CORNER_FACE{

        @Override
        public boolean isCorner() {
            return true;
        }
        
    };
    
    private final FacetType reflectingEdge;

    private VertexPosition() {
        this(null);
    }

    private VertexPosition(FacetType reflectingEdge) {
        this.reflectingEdge = reflectingEdge;
    }
    
    public FacetType getReflectingEdge(VertexPosition source){
        if(reflectingEdge==null)
            throw new UnsupportedOperationException();
        else
            return reflectingEdge;
    }
    
    public boolean isCorner(){
        return false;
    }
    
    public boolean isEdge(){
        return false;
    }
    
    public boolean isAdjacentCorner(VertexPosition p){
        return false;
    }
}
