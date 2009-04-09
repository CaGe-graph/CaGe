package cage;

/**
 * Implementation of <code>EmbeddingTypeFactory</code> which offers access to
 * the default embedding types available in CaGe.
 *
 * @author nvcleemp
 */
public class CaGeEmbeddingTypeFactory implements EmbeddingTypeFactory{

    private static final Type[] TYPES;

    static {
        TYPES = new Type[3];
        TYPES[0] = new Type("Planar") {

            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-ip"}};

            public Embedder getEmbedder() {
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        };
        TYPES[1] = new Type("Polyhedral") {

            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-is"}};

            public Embedder getEmbedder() {
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        };
        TYPES[2] = new Type("Tubular") {

            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-it"}};

            public Embedder getEmbedder() {
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        };
    }

    public String[] getEmbeddingTypes() {
        String[] types = new String[TYPES.length];
        for (int i = 0; i < types.length; i++) {
            types[i] = TYPES[i].getTypeName();
        }
        return types;
    }

    public Embedder getEmbedderFor(String type) {
        int i = 0;
        while(i < TYPES.length && !TYPES[i].getTypeName().equals(type)) i++;
        if(i==TYPES.length)
            return null;
        else
            return TYPES[i].getEmbedder();
    }


    //TODO: when upgrading to Java 1.5 this should use enums
    private static abstract class Type {
        private String typeName;

        public Type(String typeName) {
            this.typeName = typeName;
        }

        public abstract Embedder getEmbedder();

        public String getTypeName() {
            return typeName;
        }
    }
}
