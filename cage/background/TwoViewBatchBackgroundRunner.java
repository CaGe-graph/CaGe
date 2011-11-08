package cage.background;

import cage.CaGe;
import cage.CaGePipe;
import cage.CaGeResult;
import cage.GeneratorInfo;
import cage.viewer.twoview.BatchTwoViewModel;
import cage.viewer.twoview.TwoViewModel;
import cage.viewer.twoview.TwoViewSaver;

import java.io.File;

import lisken.systoolbox.Systoolbox;

/**
 * Implementation of BackgroundRunner that exports all graphs as images to files
 * according to a file name template it receives.
 * 
 * @author nvcleemp
 */
public class TwoViewBatchBackgroundRunner extends AbstractBackgroundRunner {
    
    private static int batchRunnerCount = 0;
    
    private final TwoViewModel model;
    private final TwoViewSaver saver;
    private final File folder;
    private final String fileNameTemplate;
    
    /**
     * 
     * @param generator
     * @param generatorInfo
     * @param batchTwoViewModel 
     */
    public TwoViewBatchBackgroundRunner(CaGePipe generator, GeneratorInfo generatorInfo,
            BatchTwoViewModel batchTwoViewModel) {
        super(String.format("TwoView Batch runner %d", ++batchRunnerCount), generator, generatorInfo, true, false);
        
        //create a TwoViewModel to handle the results
        model = new TwoViewModel();
        
        //construct the saver and copy the folder and the file name template
        saver = batchTwoViewModel.getSaver().getSaver(model);
        folder = batchTwoViewModel.getFolder();
        fileNameTemplate = batchTwoViewModel.getFileNameTemplate();
        
        //build the info text once
        buildInfoText();
    }
    
    private void buildInfoText(){
        infoText.append("Generator:\t ").append(
                    generatorInfo.getGeneratorName())
                .append("\n");
        if (CaGe.expertMode) {
            infoText.append("Command:\t ").append(
                    Systoolbox.makeCmdLine(
                        generatorInfo.getGenerator()))
                    .append("\n");
            infoText.append("Embedder:\t ").append(
                    Systoolbox.makeCmdLine(
                        generatorInfo.getEmbedder().getEmbed2DNew()))
                    .append("\n");
        }
        infoText.append("File template:\t ").append(
                    new File(folder, fileNameTemplate).getPath())
                .append("\n");
        
    }

    @Override
    protected void embeddingMade(CaGeResult result) {
        model.setResult(result);
        String fileName = String.format(fileNameTemplate, result.getGraphNo());
        File file = new File(folder, fileName);
        saver.saveFile(file);
    }

    public String getInfoText() {
        /*
         * just return the content of infoText. We added our data to this 
         * StringBuffer during construction and any error that occured was
         * appended to it.
         */
        return infoText.toString();
    }

    @Override
    protected void cleanUp() {
        //do nothing
    }
    
}
