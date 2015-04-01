package cage.writer;

import cage.CaGeResult;
import cage.EdgeIterator;
import cage.ElementRule;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;

import java.util.HashMap;
import java.util.Map;


/**
 * A CaGeWriter which outputs the graph as a Spartan input file.
 * 
 * @author nvcleemp
 */
public class SpinputWriter extends CaGeWriter {
    
    private ElementRule elementRule;
    
    private boolean useSingleElementRule = false;
    private int singleElementNumber = 0;
    private String singleElementName = "";
    
    private float scalingFactor = 1.0f; 
    
    private final Map<String, Integer> atomNumber = new HashMap<>();

    public SpinputWriter() {
        //we only give the values for the elements currently used in CaGe
        atomNumber.put("H", 1);
        atomNumber.put("O", 8);
        atomNumber.put("C", 6);
        atomNumber.put("Si", 14);
        atomNumber.put("N", 7);
        atomNumber.put("S", 16);
        atomNumber.put("I", 53);
    }

    @Override
    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        super.setGeneratorInfo(generatorInfo);
        elementRule = generatorInfo.getElementRule();
    }

    @Override
    public String getFormatName() {
        return "spinput";
    }

    @Override
    public String getFileExtension() {
        return "spinput";
    }

    @Override
    public void outputResult(CaGeResult result) {
        EmbeddableGraph graph = result.getGraph();
        float[][] coordinate = graph.get3DCoordinates();
        
        StringBuilder sb = new StringBuilder("\n\n0 1\n");
        
        for (int i = 0; i < graph.getSize(); i++) {
            sb.append("\t").append(getElementNumber(graph, i+1));
            for (int j = 0; j < 3; j++) {
                sb.append("\t").append(coordinate[i][j]*scalingFactor);
            }
            sb.append("\n");
        }
        sb.append("ENDCART\nATOMLABELS\n");
        for (int i = 0; i < graph.getSize(); i++) {
            sb.append('"').append(getElementName(graph, i+1));
            sb.append(i+1).append('"').append("\n");
        }
        sb.append("ENDATOMLABELS\nHESSIAN\n");
        for (int i = 1; i <= graph.getSize(); i++) {
            sb.append("\t0");
            if(i%12==0){
                sb.append("\n");
            }
        }
        if(graph.getSize()%12!=0){
            sb.append("\n");
        }
        
        for (int i = 1; i <= graph.getSize(); i++) {
            EdgeIterator edgeIterator = graph.getEdgeIterator(i);
            while(edgeIterator.hasNext()) {
                int to = edgeIterator.nextEdge();
                if(i < to){
                    sb.append("\t").append(i).append("\t").append(to).append("\t1\n");
                }
            }
        }
        
        sb.append("ENDHESS\n");
        
        out(sb.toString());
    }
    
    private String getElementName(EmbeddableGraph graph, int vertex){
        if(useSingleElementRule){
            return singleElementName;
        } else {
            return elementRule.getElement(graph, vertex);
        }
    }
    
    private int getElementNumber(EmbeddableGraph graph, int vertex){
        if(useSingleElementRule){
            return singleElementNumber;
        } else {
            return atomNumber.get(elementRule.getElement(graph, vertex));
        }
    }

    public void setUseSingleElementRule(boolean useSingleElementRule) {
        this.useSingleElementRule = useSingleElementRule;
    }

    public void setSingleElement(int singleElementNumber, String singleElementName) {
        this.singleElementNumber = singleElementNumber;
        this.singleElementName = singleElementName;
    }

    public void setScalingFactor(float scalingFactor) {
        this.scalingFactor = scalingFactor;
    }
    
}
