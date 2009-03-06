package cage.utility;

import java.awt.Component;
import java.util.ArrayList;
import java.util.List;

/**
 * Class that groups a set of <code>Component</code>s in a logical group. Actions
 * such as <tt>setVisible</tt> and <tt>setEnabled</tt>
 * 
 * @author nvcleemp
 */
public class ComponentLogicalGroup {

    private List components = new ArrayList();

    /**
     * Constructs an empty <code>ComponentLogicalGroup</code> object.
     */
    public ComponentLogicalGroup(){
        //empty constructor
    }

    /**
     * Adds the specified component to this group.
     *
     * @param c the component to be added to this group
     * @return <tt>true</tt> if <tt>c</tt> was not null
     */
    public boolean addComponent(Component c){
        return c!=null && components.add(c);
    }

    /**
     * Removes the specified component from this group.
     *
     * @param c the component to be removed from this group
     * @return <tt>true</tt> if this group contained the specified component.
     */
    public boolean removeComponent(Component c){
        return components.remove(c);
    }

    /**
     * Shows or hides the components in this group depending on
     * the value of parameter <code>visible</code>.
     * @param visible  if <code>true</code>, shows the components
     * in this group; otherwise, hides them
     */
    public void setVisible(boolean visible){
        for(int i=0; i<components.size(); i++){
            ((Component)components.get(i)).setVisible(visible);
        }
    }

    /**
     * Enables or disables the components in this group depending on
     * the value of parameter <code>enabled</code>.
     * @param enabled  if <code>true</code>, enables the components
     * in this group; otherwise, disables them
     */
    public void setEnabled(boolean enabled){
        for(int i=0; i<components.size(); i++){
            ((Component)components.get(i)).setEnabled(enabled);
        }
    }

}
