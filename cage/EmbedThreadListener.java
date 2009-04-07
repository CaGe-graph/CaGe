package cage;

public interface EmbedThreadListener {

    void showEmbeddingException(Exception e, String context, String embedDiagnostics);

    void embeddingFinished();
}

