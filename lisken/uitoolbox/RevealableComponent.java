package lisken.uitoolbox;

import java.awt.Graphics;
import java.awt.GridLayout;
import javax.swing.JComponent;
import javax.swing.JPanel;

/**
 * This class can wrap a component to make it revealable. This component completely
 * delegates to another component, but only shows that component when it is revealed.
 * 
 * @author nvcleemp
 */
public class RevealableComponent<C extends JComponent> extends JPanel{
    
    private C wrappedComponent;
    private boolean revealed;

    public RevealableComponent(C wrappedComponent, boolean revealed) {
        super(new GridLayout(1, 1));
        this.wrappedComponent = wrappedComponent;
        this.revealed = revealed;
        add(wrappedComponent);
    }

    public void setRevealed(boolean revealed) {
        if(this.revealed != revealed){
            this.revealed = revealed;
            repaint();
        }
    }

    public boolean isRevealed() {
        return revealed;
    }

    public C getWrappedComponent() {
        return wrappedComponent;
    }

    @Override
    protected void paintChildren(Graphics g) {
        if(revealed){
            super.paintChildren(g);
        }
    }
    
    
}
