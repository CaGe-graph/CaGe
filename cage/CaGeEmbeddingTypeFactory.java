package cage;

/**
 * Implementation of <code>EmbeddingTypeFactory</code> which offers access to
 * the default embedding types available in CaGe.
 *
 * @author nvcleemp
 */
public class CaGeEmbeddingTypeFactory implements EmbeddingTypeFactory{

    @Override
    public String[] getEmbeddingTypes() {
        String[] types = new String[Type.values().length];
        for (int i = 0; i < types.length; i++) {
            types[i] = Type.values()[i].getTypeName();
        }
        return types;
    }

    @Override
    public Embedder getEmbedderFor(String type) {
        int i = 0;
        while(i < Type.values().length && !Type.values()[i].getTypeName().equals(type)) i++;
        if(i==Type.values().length)
            return null;
        else
            return Type.values()[i].getEmbedder();
    }

    private static enum Type {
        RATHER_FLAT("rather flat"){
            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-ip"}};

            @Override
            public Embedder getEmbedder(){
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        },
        RATHER_SPHERICAL("rather spherical"){
            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-is"}};

            @Override
            public Embedder getEmbedder(){
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        },
        RATHER_TUBULAR("rather tubular"){
            private String[][] embed2D = {{"embed"}};
            private String[][] embed3D = {{"embed", "-d3", "-it"}};

            @Override
            public Embedder getEmbedder(){
                return EmbedFactory.createEmbedder(true, embed2D, embed3D);
            }
        };

        private String typeName;

        Type(String typeName) {
            this.typeName = typeName;
        }

        public abstract Embedder getEmbedder();

        public String getTypeName() {
            return typeName;
        }
    }
}
