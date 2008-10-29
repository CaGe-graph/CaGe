
package cage;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import javax.swing.Timer;
import javax.swing.event.EventListenerList;



public class CaGeTimer extends Timer
 implements ActionListener, PropertyChangeListener
{
  boolean running;
  CaGeRunner runner;

  public CaGeTimer(CaGeRunner runner, int delay)
  {
    super(delay, new DummyActionListener());
    listenerList = new EventListenerList();
    this.runner = runner;
    addActionListener(this);
    runner.addPropertyChangeListener(this);
  }

  public void start()
  {
    running = true;
    super.start();
  }

  public void stop()
  {
    super.stop();
    running = false;
  }

  public void propertyChange(PropertyChangeEvent e)
  {
    if (! running) return;
    if (e.getSource() == runner) {
      if (e.getPropertyName().charAt(0) == 'g') {
        this.restart();
      }
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    if (e.getSource() == this) {
      runner.fireGraphNoChanged();
    }
  }
}


class DummyActionListener implements ActionListener
{
  public void actionPerformed(ActionEvent e)
  {
  }
}

