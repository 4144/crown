package crown.android;

import android.content.res.AssetManager;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
		System.loadLibrary("simple");
	}
	
//	// Device functions

	// AssetManager functions
	public static native void initAssetManager(AssetManager assetManager);

	// InputManager functions
	public static native void pushEvent(int type, int a, int b, int c, int d);

//	// Renderer functions
//	public static native void beginFrame();
//	public static native void endFrame();

	public static native void init();
	public static native void frame();
	public static native void shutdown();
}
