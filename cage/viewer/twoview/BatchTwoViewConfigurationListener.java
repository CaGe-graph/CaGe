package cage.viewer.twoview;

/**
 *
 * @author nvcleemp
 */
public interface BatchTwoViewConfigurationListener {
    void fileNameTemplateChanged();
    void folderChanged();
    void saverChanged(TwoViewSavers oldSaver, TwoViewSavers newSaver);
}
