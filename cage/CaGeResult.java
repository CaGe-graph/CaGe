
package cage;


public class CaGeResult
{
  public EmbeddableGraph graph;
  public int graphNo;
  public boolean savedAdj = false;
  public boolean saved2D = false;
  public boolean saved3D = false;
  public int saved2DPS = 0;
  public boolean reembed2DMade = false;
  public boolean foldnetMade = false;

  public CaGeResult(EmbeddableGraph graph, int graphNo)
  {
    this.graph    = graph;
    this.graphNo  = graphNo;
  }
}

