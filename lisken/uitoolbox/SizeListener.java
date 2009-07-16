package lisken.uitoolbox;

/**
 * An object that can be notified when some other object's size changes
 */
interface SizeListener {

    /**
     * Called when the size of the component that is listened to changes.
     */
    public void sizing();
}
