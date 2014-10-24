package com.coapie.piebox;

import android.app.Application;
import android.content.res.Configuration;

public class PieBoxApplication extends Application {
	@Override
	public void onConfigurationChanged(Configuration newConfig){
		super.onConfigurationChanged(newConfig);
	}
	
	@Override
	public void onCreate(){
		super.onCreate();
	}
	
	@Override
	public void onLowMemory(){
		super.onLowMemory();
	}
	
	@Override
	public void onTerminate(){
		super.onTerminate();
	}
	
	@Override
	public void onTrimMemory(int level){
		super.onTrimMemory(level);
	}
}
