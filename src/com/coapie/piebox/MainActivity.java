package com.coapie.piebox;

import com.jeremyfeinstein.slidingmenu.lib.SlidingMenu;
import com.jeremyfeinstein.slidingmenu.lib.app.SlidingActivity;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;


public class MainActivity extends SlidingActivity {

	private Fragment mContent;
	private int mContentIndex;
	
	public static final int HOME_FRAGMENT_INDEX 	= 0;
	public static final int SHARE_FRAGMENT_INDEX 	= 1;
	
	private IDaemonsHandle mDaemons = null;
	
	public IDaemonsHandle getDaemonsHandle(){
		return mDaemons;
	}

	private ServiceConnection mConnection = new ServiceConnection () {
		@Override
		public void onServiceConnected(ComponentName className, IBinder iService) {
			mDaemons = IDaemonsHandle.Stub.asInterface(iService);
		}
		
		@Override
		public void onServiceDisconnected(ComponentName className) {
			mDaemons = null;
		}
	};

	@Override
	public void onStart() {
		super.onStart();
		
		boolean rc = bindService(new Intent(IDaemonsHandle.class.getName()), mConnection, BIND_AUTO_CREATE);
		if (rc == false){
			Log.v("PieBox", "fail to bind native daemons service");
			return ;
		}
		
		Log.v("PieBox", "bind service success");
	}
	
	@Override
	public void onStop() {
		super.onStop();
		
		unbindService(mConnection);
	}  
		
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		mContentIndex = 0;

		if(savedInstanceState != null){
			mContent = getSupportFragmentManager().getFragment(savedInstanceState, "mContent");
		}
		
		if(mContent == null){
			mContent = new HomeFragment();
		}
		
		initSlidingMenu();
	}
	
	private void initSlidingMenu(){	
		setContentView(R.layout.content_frame);
		getSupportFragmentManager().beginTransaction()
		.replace(R.id.content_frame, mContent)
		.commit();  

		setBehindContentView(R.layout.menu_frame);
		getSupportFragmentManager().beginTransaction()
		.replace(R.id.menu_frame, new SlidingFragment())  
		.commit();  
		
		getActionBar().setHomeButtonEnabled(true);
		setSlidingActionBarEnabled(false);
     
		SlidingMenu menu = getSlidingMenu();
		menu.setMode(SlidingMenu.LEFT);
  	  	menu.setTouchModeAbove(SlidingMenu.TOUCHMODE_FULLSCREEN);
  	  	menu.setShadowWidthRes(R.dimen.slidingfragment_shadow_width);
  	  	menu.setShadowDrawable(R.drawable.sliding_shadow);
  	  	menu.setBehindOffsetRes(R.dimen.slidingfragment_width);
  	  	menu.setBehindScrollScale(0.0f); 
  	  	menu.setFadeDegree(0.35f);
		
  	  	showContent();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.home, menu);
		return true;
	}

	public boolean onOptionsItemSelected(MenuItem item) {  
	    switch(item.getItemId()){ 
	    case android.R.id.home:
	    	toggle();
	    default: 
	        break; 
	    } 
	    return super.onOptionsItemSelected(item); 
	} 
	

	@Override
	public void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
		getSupportFragmentManager().putFragment(outState, "mContent", mContent);
	}

	public void switchContent(int index) {
		if(mContentIndex == index){
			toggle();
			return;
		}
		
		switch(index){
		case HOME_FRAGMENT_INDEX:
			mContent = new HomeFragment();
			break;
			
		case SHARE_FRAGMENT_INDEX:
			mContent = new ShareFragment();
			break;
			
		default:
			toggle();
			return ;
		}
		mContentIndex = index;
		
		getSupportFragmentManager()
		.beginTransaction()
		.replace(R.id.content_frame, mContent)
		.commit();
		
		Handler h = new Handler();
		h.postDelayed(new Runnable() {
			public void run() {
				showContent();
			}
		}, 50);
	}
}
