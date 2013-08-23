package cage.background;

import cage.CaGe;
import cage.CaGePipe;
import cage.CaGeResult;
import cage.GeneratorInfo;
import cage.writer.CaGeWriter;

import java.util.ArrayList;
import java.util.List;

import lisken.systoolbox.Systoolbox;

/**
 * Default implementation of BackgroundRunner that takes a list of CaGeWriter's and
 * writes the graphs to file or pipe using these writers.
 */
public class DefaultBackgroundRunner extends AbstractBackgroundRunner {
    
    //counter used to give each DefaultBackgroundRunner a unique name.
    private static int threadCount = 0;
    
    private CaGeWriter[] writer;
    private List<CaGeWriter> writers;
    private List<String> writeDests;

    public DefaultBackgroundRunner(CaGePipe generator, GeneratorInfo generatorInfo,
            boolean doEmbed2D, boolean doEmbed3D,
            List<CaGeWriter> writers, List<String> writeDests) {
        super("BackgroundRunner " + ++threadCount, generator, generatorInfo, doEmbed2D, doEmbed3D);
        //TODO: does this really has to be the same Vector or could we copy the writers in a List?
        this.writers = new ArrayList<>(writers);
        this.writeDests = new ArrayList<>(writeDests);
        writer = new CaGeWriter[writers.size()];
        
        writer = writers.toArray(writer);
    }
    
    public String getInfoText() {
        infoText.append("generator:\t ").append(generatorInfo.getGeneratorName()).append("\n");
        if (CaGe.expertMode) {
            infoText.append("  command:\t ").append(Systoolbox.makeCmdLine(generatorInfo.getGenerator())).append("\n");
        }
        infoText.append("\noutput:\n");
        for(int i = 0; i < writers.size(); i++){
            int dimension = writers.get(i).getDimension();
            infoText.append("  ");
            infoText.append(dimension <= 0 ? "adj" : dimension + "D");
            infoText.append(" >\t ").append(writeDests.get(i)).append("\n");
        }
        return infoText.toString();
    }
    
    protected void cleanUp(){
        for (int i = 0; i < writer.length; ++i) {
            writer[i].stop();
        }
        //set the array to null to be sure that no further output is done
        writer = null;
    }

    protected void embeddingMade(CaGeResult result) {
        for (int i = 0; i < writer.length; ++i) {
            try {
                writer[i].outputResult(result);
                writer[i].throwLastIOException();
            } catch (Exception ex) {
                fireExceptionOccurred(ex);
                end();
            }
        }
    }
}

