package cage;

import java.awt.GridLayout;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JTabbedPane;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * <code>GeneratorPanel</code> that combines several <code>GeneratorPanel</code>s
 * on a <code>JTabbedPane</code>.
 *
 * @author nvcleemp
 */
public abstract class CombinedGeneratorPanel extends GeneratorPanel {

    protected JTabbedPane pane = new JTabbedPane();
    private boolean notShown = true; //set to false when this panel is shown for the first time
    
    private ChangeListener tabbedPaneListener = new ChangeListener() {

        public void stateChanged(ChangeEvent e) {
            //this object should only listen to pane
            if(!notShown && e.getSource().equals(pane)){
                getNextButton().setEnabled(true); //TODO: provide method for this in GeneratorPanel
                ((GeneratorPanel) pane.getSelectedComponent()).showing();
            }
        }
    };

    public CombinedGeneratorPanel() {
        pane.addChangeListener(tabbedPaneListener);
        setLayout(new GridLayout(1,1));
    }

    /**
     * Adds <tt>panel</tt> to the tabbed pane, sets a border for it and then
     * calls all of the {@link lisken.uitoolbox.WizardAwareComponent} methods.
     *
     * @param title the title to be displayed in this tab
     * @param panel the component to be displayed when this tab is clicked
     */
    protected void addTab(String title, GeneratorPanel panel) {
        panel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
        panel.setPreviousButton(getPreviousButton());
        panel.setNextButton(getNextButton());
        panel.setFinishButton(getFinishButton());
        panel.setCancelButton(getCancelButton());
        panel.setExitButton(getExitButton());
        pane.addTab(title, panel);
    }

    @Override
    public void setCancelButton(JButton cancelButton) {
        super.setCancelButton(cancelButton);
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setCancelButton(cancelButton);
        }
    }

    @Override
    public void setExitButton(JButton exitButton) {
        super.setExitButton(exitButton);
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setExitButton(exitButton);
        }
    }

    @Override
    public void setFinishButton(JButton finishButton) {
        super.setFinishButton(finishButton);
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setFinishButton(finishButton);
        }
    }

    @Override
    public void setNextButton(JButton nextButton) {
        super.setNextButton(nextButton);
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setNextButton(nextButton);
        }
    }

    @Override
    public void setPreviousButton(JButton previousButton) {
        super.setPreviousButton(previousButton);
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setPreviousButton(previousButton);
        }
    }

    /**
     * If there currently is a <code>GeneratorPanel</code> selected on the
     * <code>JTabbedPane</code>, calls the <code>showing()</code> method of
     * this panel.
     */
    public void showing() {
        notShown = false;
        if(pane.getSelectedComponent()!=null)
            ((GeneratorPanel) pane.getSelectedComponent()).showing();
    }

    /**
     * If there currently is a <code>GeneratorPanel</code> selected on the
     * <code>JTabbedPane</code>, calls the <code>getGeneratorInfo()</code> method of
     * this panel and returns that <code>GeneratorInfo</code>.
     *
     * @return The <code>GeneratorInfo</code> of the currently selected generator
     *         or <tt>null</tt> if no generator is selected.
     */
    public GeneratorInfo getGeneratorInfo() {
        if(pane.getSelectedComponent()!=null)
            return ((GeneratorPanel) pane.getSelectedComponent()).getGeneratorInfo();
        else
            return null;
    }
    
}
