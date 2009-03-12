package cage;

import java.beans.PropertyChangeListener;

public interface CaGeRunner {

    void addPropertyChangeListener(PropertyChangeListener listener);

    void removePropertyChangeListener(PropertyChangeListener listener);

    void fireGraphNoChanged();

    void fireRunningChanged();

    void fireExceptionOccurred(Exception e);

    int getGraphNo();
}

