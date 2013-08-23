package cage.viewer.twoview;

import cage.CaGe;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author nvcleemp
 */
public class BatchTwoViewModel {
    
    private File folder;
    private String fileNameTemplate;
    private TwoViewSavers saver;

    public BatchTwoViewModel() {
        folder = new File(CaGe.config.getProperty("CaGe.Generators.RunDir"));
        saver = TwoViewSavers.SVG;
    }

    public String getFileNameTemplate() {
        return fileNameTemplate;
    }

    public void setFileNameTemplate(String fileNameTemplate) {
        if(fileNameTemplate!=null){
            fileNameTemplate = fileNameTemplate.trim();
        }
        if(this.fileNameTemplate == null ?
                fileNameTemplate != null :
                !this.fileNameTemplate.equals(fileNameTemplate)){
            this.fileNameTemplate = fileNameTemplate;
            fireFileNameTemplateChanged();
        }
    }

    public File getFolder() {
        return folder;
    }

    public void setFolder(File folder) {
        if(this.folder == null ?
                folder != null :
                !this.folder.equals(folder)){
            this.folder = folder;
            fireFolderChanged();
        }
    }

    public TwoViewSavers getSaver() {
        return saver;
    }

    public void setSaver(TwoViewSavers saver) {
        if(this.saver != saver){
            TwoViewSavers oldSaver = this.saver;
            this.saver = saver;
            fireSaverChanged(oldSaver, saver);
            if(fileNameTemplate.endsWith(oldSaver.getExtension())){
                fileNameTemplate = 
                        fileNameTemplate.substring(0, fileNameTemplate.length() 
                            - oldSaver.getExtension().length())
                        + saver.getExtension();
                fireFileNameTemplateChanged();
            }
        }
    }
    
    private List<BatchTwoViewConfigurationListener> listeners = 
            new ArrayList<>();
    
    public void addBatchTwoViewConfigurationListener(BatchTwoViewConfigurationListener l){
        listeners.add(l);
    }
    
    public void removeBatchTwoViewConfigurationListener(BatchTwoViewConfigurationListener l){
        listeners.remove(l);
    }
    
    private void fireFileNameTemplateChanged(){
        for (BatchTwoViewConfigurationListener l : listeners) {
            l.fileNameTemplateChanged();
        }
    }
    
    private void fireFolderChanged(){
        for (BatchTwoViewConfigurationListener l : listeners) {
            l.folderChanged();
        }
    }
    
    private void fireSaverChanged(TwoViewSavers oldSaver, TwoViewSavers newSaver){
        for (BatchTwoViewConfigurationListener l : listeners) {
            l.saverChanged(oldSaver, newSaver);
        }
    }
}
