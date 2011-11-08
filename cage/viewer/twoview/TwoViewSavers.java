package cage.viewer.twoview;

/**
 * Enum which can be used to get a TwoViewSaver.
 * 
 * @author nvcleemp
 */
public enum TwoViewSavers {
    SVG(".svg"){
        public SvgTwoViewSaver getSaver(TwoViewModel model){
            return new SvgTwoViewSaver(model);
        }
    },
    TIKZ(".tikz"){
        public TikZTwoViewSaver getSaver(TwoViewModel model){
            return new TikZTwoViewSaver(model);
        }
    },
    PNG(".png"){
        public PngTwoViewSaver getSaver(TwoViewModel model){
            return new PngTwoViewSaver(model);
        }
    };
    
    private String extension;
    
    private TwoViewSavers(String extension){
        this.extension = extension;
    }
    
    public String getExtension(){
        return extension;
    }
    
    public abstract TwoViewSaver getSaver(TwoViewModel model);
}
