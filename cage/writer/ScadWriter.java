package cage.writer;

import cage.CaGeResult;
import cage.writer.scad.ScadType;

/**
 * A CaGeWriter which outputs the graph as a SCAD file.
 * 
 * @author nvcleemp
 */
public class ScadWriter extends CaGeWriter {
    
    private ScadType type;

    @Override
    public String getFormatName() {
        return "SCAD";
    }

    @Override
    public String getFileExtension() {
        return "scad";
    }

    @Override
    public void outputResult(CaGeResult result) {
        out(type.processResult(result));
    }
    
    public void setType(ScadType type){
        this.type = type;
    }
}
