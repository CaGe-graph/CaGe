package cage.viewer.jmol;

import cage.EdgeIterator;
import cage.EmbeddableGraph;

import cage.GeneratorInfo;
import java.io.BufferedReader;
import java.io.InputStream;
import java.util.Hashtable;

import org.jmol.api.JmolAdapter;
import org.jmol.api.JmolFileReaderInterface;

/**
 *
 * @author nvcleemp
 */
public class CaGeJmolAdapter extends JmolAdapter{

    /*
     * Some notes: CaGe starts numbering its vertices from 1 to n (and not 0 to n-1).
     * For the vertices and their coordinates, requesting vertex 0 will return 0 for all
     * values. But for the edge iterator this will cause an invalid memory access.
     */

    private EmbeddableGraph clientFile;
    private GeneratorInfo generatorInfo;

    public CaGeJmolAdapter() {
        super("CaGeJmolAdapter");
    }

    public Object openBufferedReader(String name, String type, BufferedReader bufferedReader, Hashtable htParams) {
        return clientFile;
    }

    public Object openBufferedReader(String name, BufferedReader bufferedReader) {
        return clientFile;
    }

    public Object openBufferedReader(String name, BufferedReader bufferedReader, Hashtable htParams) {
        return clientFile;
    }

    public Object openBufferedReader(String name, String type, BufferedReader bufferedReader) {
        return clientFile;
    }

    public Object openBufferedReaders(JmolFileReaderInterface fileReader, String[] names, String[] types, Hashtable[] htParams) {
        return clientFile;
    }

    public Object openDOMReader(Object DOMNode) {
        return clientFile;
    }

    public Object openZipFiles(InputStream is, String fileName, String[] zipDirectory, Hashtable htParams, boolean asBufferedReader) {
        return clientFile;
    }

    public EmbeddableGraph getGraph() {
        return clientFile;
    }

    public void setGraph(EmbeddableGraph graph) {
        this.clientFile = graph;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
    }

    public int getEstimatedAtomCount(Object clientFile) {
        if(!(clientFile instanceof EmbeddableGraph))
            throw new RuntimeException("CaGeJmolAdpater used with wrong clientFile.");
        EmbeddableGraph graph = (EmbeddableGraph)clientFile;
        return graph.getSize();
    }

    public JmolAdapter.AtomIterator getAtomIterator(Object clientFile) {
        if(!(clientFile instanceof EmbeddableGraph))
            throw new RuntimeException("CaGeJmolAdpater used with wrong clientFile.");
        EmbeddableGraph graph = (EmbeddableGraph)clientFile;
        JmolAdapter.AtomIterator it = new MyAtomIterator(graph);
        return it;
    }

    public JmolAdapter.BondIterator getBondIterator(Object clientFile) {
        if(!(clientFile instanceof EmbeddableGraph))
            throw new RuntimeException("CaGeJmolAdpater used with wrong clientFile.");
        EmbeddableGraph graph = (EmbeddableGraph)clientFile;
        JmolAdapter.BondIterator it = new MyBondIterator(graph);
        return it;
    }


    private class MyAtomIterator extends JmolAdapter.AtomIterator {

        private EmbeddableGraph graph;
        private int position = 0;

        public MyAtomIterator(EmbeddableGraph graph) {
            this.graph = graph;
        }

        public boolean hasNext() {
            position++;
            return (position <= graph.getSize());
        }

        public Object getUniqueID() {
            return Integer.valueOf(position);
        }

        public float getX() {
            return graph.get3DCoordinates(position)[0];
        }

        public float getY() {
            return graph.get3DCoordinates(position)[1];
        }

        public float getZ() {
            return graph.get3DCoordinates(position)[2];
        }

        public String getElementSymbol() {
            return generatorInfo.getElementRule().getElement(graph, position);
        }

    }

    private class MyBondIterator extends JmolAdapter.BondIterator {

        private EmbeddableGraph graph;
        private int position = -1;
        private int[][] edges;

        public MyBondIterator(EmbeddableGraph graph) {
            this.graph = graph;
            int edgesSize = 0;
            for (int i = 1; i <= graph.getSize(); i++) {
                edgesSize += graph.getValency(i);
            }
            edgesSize /= 2;
            edges = new int[edgesSize][2];
            int j = 0;
            for (int i = 1; i <= graph.getSize(); i++) {
                EdgeIterator it = graph.getEdgeIterator(i);
                while (it.hasNext()) {
                    int to = it.nextEdge();
                    if(to > i){
                        edges[j][0]=i;
                        edges[j][1]=to;
                        j++;
                    }
                }
            }
        }

        public boolean hasNext() {
            position++;
            return position < edges.length;
        }

        public Object getAtomUniqueID1() {
            return Integer.valueOf(edges[position][0]);
        }

        public Object getAtomUniqueID2() {
            return Integer.valueOf(edges[position][1]);
        }

        public int getEncodedOrder() {
            return 1;
        }

    }

}
